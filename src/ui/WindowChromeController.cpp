/*
 * Copyright (c) 2026 HsingYun (iakext@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "WindowChromeController.h"

#include <dwmapi.h>
#include <windows.h>

#include <QApplication>
#include <QCoreApplication>
#include <QEvent>
#include <QMainWindow>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QStyle>
#include <QStyleOption>
#include <QSvgRenderer>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>

#include "ThemeManager.h"

namespace uwf::ui {

namespace {

constexpr DWORD kDwmAttrUseImmersiveDarkMode = 20;
constexpr DWORD kDwmAttrWindowCornerPreference = 33;
constexpr DWORD kDwmAttrBorderColor = 34;
constexpr DWORD kDwmAttrCaptionColor = 35;
constexpr DWORD kDwmAttrTextColor = 36;
constexpr int kDwmCornerRound = 2;

COLORREF colorRef(const QColor& c) { return RGB(c.red(), c.green(), c.blue()); }

template <typename T>
void setDwmAttr(HWND hwnd, DWORD attr, const T& value) {
  (void)DwmSetWindowAttribute(hwnd, attr, &value, static_cast<DWORD>(sizeof(value)));
}

bool isToolbarDragTarget(QObject* obj) {
  auto* w = qobject_cast<QWidget*>(obj);
  if (!w || qobject_cast<QToolButton*>(w)) return false;
  for (QObject* p = w; p; p = p->parent()) {
    if (qobject_cast<QToolButton*>(p) || qobject_cast<QMenu*>(p)) return false;
    if (auto* tb = qobject_cast<QToolBar*>(p); tb && tb->objectName() == "mainToolbar") return true;
  }
  return false;
}

class ToolbarExtIcon final : public QObject {
 public:
  using QObject::QObject;

 protected:
  bool eventFilter(QObject* obj, QEvent* ev) override {
    if (ev->type() != QEvent::Paint) return QObject::eventFilter(obj, ev);
    auto* w = qobject_cast<QWidget*>(obj);
    if (!w) return false;
    QPainter p(w);
    QStyleOption opt;
    opt.initFrom(w);
    w->style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, w);
    static QSvgRenderer svg{QStringLiteral(":/icons/arrow_right.svg")};
    constexpr qreal kSide = 14.0;
    svg.render(&p, QRectF((w->width() - kSide) / 2.0, (w->height() - kSide) / 2.0, kSide, kSide));
    return true;
  }
};

}  // namespace

WindowChromeController::WindowChromeController(QMainWindow* window, QObject* parent) : QObject(parent), m_window(window) { qApp->installEventFilter(this); }

void WindowChromeController::applyTitleBarTheme() const {
  const auto hwnd = reinterpret_cast<HWND>(m_window->winId());
  if (!hwnd) return;

  auto& tm = ThemeManager::instance();
  const BOOL dark = tm.current() == Theme::Dark ? TRUE : FALSE;
  setDwmAttr(hwnd, kDwmAttrUseImmersiveDarkMode, dark);

  const int corner = kDwmCornerRound;
  setDwmAttr(hwnd, kDwmAttrWindowCornerPreference, corner);

  const COLORREF caption = colorRef(tm.color(Sem::Surface));
  const COLORREF text = colorRef(tm.color(Sem::Fg));
  const COLORREF border = colorRef(tm.color(Sem::Border));
  setDwmAttr(hwnd, kDwmAttrBorderColor, border);
  setDwmAttr(hwnd, kDwmAttrCaptionColor, caption);
  setDwmAttr(hwnd, kDwmAttrTextColor, text);
}

void WindowChromeController::raiseToFront(const bool contentInitialized) const {
  const bool revealFromHidden = !m_window->isVisible() && contentInitialized;
  if (revealFromHidden) m_window->setWindowOpacity(0.0);
  if (m_window->isMinimized())
    m_window->showNormal();
  else
    m_window->show();
  m_window->raise();
  m_window->activateWindow();
  if (revealFromHidden) {
    QCoreApplication::sendPostedEvents(nullptr, QEvent::LayoutRequest);
    m_window->repaint();
    QTimer::singleShot(0, m_window, [window = m_window]() {
      window->setWindowOpacity(1.0);
      window->raise();
      window->activateWindow();
    });
  }
}

void WindowChromeController::decorateToolbar(QToolBar* toolbar) const {
  if (auto* ext = toolbar->findChild<QToolButton*>(QStringLiteral("qt_toolbar_ext_button"))) {
    ext->installEventFilter(new ToolbarExtIcon(ext));
  }
}

bool WindowChromeController::eventFilter(QObject* obj, QEvent* ev) {
  if (ev->type() == QEvent::MouseButtonPress && isToolbarDragTarget(obj)) {
    auto* me = static_cast<QMouseEvent*>(ev);
    if (me->button() == Qt::LeftButton) {
      const QPoint globalPos = me->globalPosition().toPoint();
      // startSystemMove() leaves Qt's toolbar hover tracking stale after a
      // click on Windows.  Let the native caption loop own this interaction;
      // unlike a delayed startSystemMove(), it also preserves drag-to-move.
      ReleaseCapture();
      SendMessageW(reinterpret_cast<HWND>(m_window->winId()), WM_NCLBUTTONDOWN, HTCAPTION,
                   MAKELPARAM(globalPos.x(), globalPos.y()));
      return true;
    }
  }
  return QObject::eventFilter(obj, ev);
}

}  // namespace uwf::ui
