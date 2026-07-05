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
#include "OverlayFloatingWidget.h"

#include <windows.h>

#include <QAction>
#include <QContextMenuEvent>
#include <QCursor>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QHideEvent>
#include <QLabel>
#include <QMenu>
#include <QMouseEvent>
#include <QMoveEvent>
#include <QPainter>
#include <QPainterPath>
#include <QScreen>
#include <QShowEvent>
#include <QStyle>
#include <QVBoxLayout>
#include <algorithm>
#include <format>

#include "I18n.h"
#include "ThemeManager.h"

namespace uwf::ui {

namespace {

constexpr int kWindowH = 44;
constexpr int kMinWindowW = 96;
constexpr int kHandleWindowSide = 32;
constexpr int kHandleSide = 24;
constexpr int kHandleRightInset = 4;
constexpr int kContentLeftMargin = 14;
constexpr int kContentVPadding = 10;
constexpr int kTextHandleGap = 8;
constexpr int kRadius = 8;
constexpr float kSurfaceAlpha = 0.46f;
constexpr float kHandleFillAlpha = 0.12f;

QString formatUsageText(const uint32_t usedMb, const uint32_t totalMb) {
  const double pct = totalMb == 0 ? 0.0 : static_cast<double>(usedMb) * 100.0 / static_cast<double>(totalMb);
  return QString::fromStdString(std::format("{:.1f}% {}/{} MB", pct, usedMb, totalMb));
}

QRect primaryDesktopGeometry() {
  if (QScreen* screen = QGuiApplication::primaryScreen()) return screen->availableGeometry();
  return {0, 0, 1280, 720};
}

QRect cursorDesktopGeometry() {
  if (QScreen* screen = QGuiApplication::screenAt(QCursor::pos())) return screen->availableGeometry();
  if (QScreen* screen = QGuiApplication::primaryScreen()) return screen->availableGeometry();
  return {0, 0, 1280, 720};
}

QPoint clampedTopLeft(QPoint p, const QSize size) {
  const QRect g = cursorDesktopGeometry();
  p.setX(std::clamp(p.x(), g.left(), std::max(g.left(), g.right() - size.width() + 1)));
  p.setY(std::clamp(p.y(), g.top(), std::max(g.top(), g.bottom() - size.height() + 1)));
  return p;
}

QRect handleRectForSize(const QSize size) {
  return QRect(size.width() - kHandleRightInset - kHandleWindowSide, (size.height() - kHandleWindowSide) / 2, kHandleWindowSide, kHandleWindowSide);
}

int contentRightMargin() { return kHandleRightInset + kHandleWindowSide + kTextHandleGap; }

void makeWindowMouseTransparent(HWND hwnd) {
  if (!hwnd) return;
  const LONG_PTR ex = GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
  const LONG_PTR next = ex | WS_EX_LAYERED | WS_EX_TRANSPARENT;
  if (next == ex) return;
  SetWindowLongPtrW(hwnd, GWL_EXSTYLE, next);
  SetWindowPos(hwnd, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
}

}  // namespace

class OverlayMoveHandle final : public QWidget {
 public:
  explicit OverlayMoveHandle(OverlayFloatingWidget* owner)
      : QWidget(owner, Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::NoDropShadowWindowHint), m_owner(owner) {
    setObjectName("overlayFloatingHandle");
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAttribute(Qt::WA_ShowWithoutActivating, true);
    setFixedSize(kHandleWindowSide, kHandleWindowSide);
    setCursor(Qt::SizeAllCursor);
  }

 protected:
  void contextMenuEvent(QContextMenuEvent* ev) override {
    if (m_owner) m_owner->popupContextMenuAt(ev->globalPos());
  }

  void mousePressEvent(QMouseEvent* ev) override {
    if (m_owner && ev->button() == Qt::LeftButton) {
      m_dragging = true;
      m_dragOffset = ev->globalPosition().toPoint() - m_owner->frameGeometry().topLeft();
      setCursor(Qt::ClosedHandCursor);
      ev->accept();
      return;
    }
    QWidget::mousePressEvent(ev);
  }

  void mouseMoveEvent(QMouseEvent* ev) override {
    if (m_owner && m_dragging) {
      m_owner->moveByHandleDrag(ev->globalPosition().toPoint(), m_dragOffset);
      ev->accept();
      return;
    }
    QWidget::mouseMoveEvent(ev);
  }

  void mouseReleaseEvent(QMouseEvent* ev) override {
    if (ev->button() == Qt::LeftButton && m_dragging) {
      m_dragging = false;
      setCursor(Qt::SizeAllCursor);
      ev->accept();
      return;
    }
    QWidget::mouseReleaseEvent(ev);
  }

  void paintEvent(QPaintEvent*) override {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const auto& tm = ThemeManager::instance();
    const QRectF h((width() - kHandleSide) / 2.0, (height() - kHandleSide) / 2.0, kHandleSide, kHandleSide);
    QPainterPath handlePath;
    handlePath.addRoundedRect(h.adjusted(2.5, 2.5, -2.5, -2.5), 5, 5);
    QColor handleFill = tm.color(Sem::FgMuted);
    handleFill.setAlphaF(kHandleFillAlpha);
    p.fillPath(handlePath, handleFill);
    p.setPen(QPen(tm.color(Sem::Border), 1.0));
    p.drawPath(handlePath);

    const QColor muted = tm.color(Sem::FgMuted);
    const QPointF c = h.center();
    p.setPen(QPen(muted, 1.6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    p.drawLine(QPointF(c.x() - 6, c.y()), QPointF(c.x() + 6, c.y()));
    p.drawLine(QPointF(c.x(), c.y() - 6), QPointF(c.x(), c.y() + 6));
    p.drawLine(QPointF(c.x() - 6, c.y()), QPointF(c.x() - 3.5, c.y() - 2.5));
    p.drawLine(QPointF(c.x() - 6, c.y()), QPointF(c.x() - 3.5, c.y() + 2.5));
    p.drawLine(QPointF(c.x() + 6, c.y()), QPointF(c.x() + 3.5, c.y() - 2.5));
    p.drawLine(QPointF(c.x() + 6, c.y()), QPointF(c.x() + 3.5, c.y() + 2.5));
    p.drawLine(QPointF(c.x(), c.y() - 6), QPointF(c.x() - 2.5, c.y() - 3.5));
    p.drawLine(QPointF(c.x(), c.y() - 6), QPointF(c.x() + 2.5, c.y() - 3.5));
    p.drawLine(QPointF(c.x(), c.y() + 6), QPointF(c.x() - 2.5, c.y() + 3.5));
    p.drawLine(QPointF(c.x(), c.y() + 6), QPointF(c.x() + 2.5, c.y() + 3.5));
  }

 private:
  OverlayFloatingWidget* m_owner = nullptr;
  bool m_dragging = false;
  QPoint m_dragOffset;
};

OverlayFloatingWidget::OverlayFloatingWidget(QWidget* parent)
    : QWidget(parent, Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::NoDropShadowWindowHint) {
  setObjectName("overlayFloatingWidget");
  setAttribute(Qt::WA_TranslucentBackground, true);
  setFixedHeight(kWindowH);
  setMinimumWidth(kMinWindowW);
  setCursor(Qt::ArrowCursor);
  m_handle = new OverlayMoveHandle(this);

  auto* outer = new QVBoxLayout(this);
  outer->setContentsMargins(kContentLeftMargin, kContentVPadding, contentRightMargin(), kContentVPadding);
  outer->setSpacing(0);

  auto* head = new QHBoxLayout();
  head->setContentsMargins(0, 0, 0, 0);
  head->setSpacing(0);

  m_usage = new QLabel(QStringLiteral("—"), this);
  m_usage->setObjectName("overlayFloatUsage");
  m_usage->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  m_usage->setTextInteractionFlags(Qt::NoTextInteraction);
  m_usage->setAttribute(Qt::WA_TransparentForMouseEvents, true);
  head->addWidget(m_usage, 0);
  outer->addLayout(head);

  connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [this](Theme) { applyTheme(); });
  applyTheme();
  refreshText();
}

void OverlayFloatingWidget::popupContextMenuAt(const QPoint& globalPos) {
  QMenu menu(this);
  QAction* showMainAct = menu.addAction(I18n::tr("Show main window"));
  QAction* closeAct = menu.addAction(I18n::tr("Close floating window"));
  menu.addSeparator();
  QAction* exitAct = menu.addAction(I18n::tr("Exit application"));
  QAction* picked = menu.exec(globalPos);
  if (picked == showMainAct) {
    emit showMainWindowRequested();
  } else if (picked == closeAct) {
    emit closeFloatingWindowRequested();
  } else if (picked == exitAct) {
    emit exitApplicationRequested();
  }
}

void OverlayFloatingWidget::setOverlayConfig(const core::OverlayConfig& cfg) {
  (void)cfg;
  refreshText();
}

void OverlayFloatingWidget::updateUsage(const core::OverlayRuntime& runtime) {
  m_runtime = runtime;
  m_hasRuntime = true;
  m_unavailable = false;
  refreshText();
}

void OverlayFloatingWidget::setUnavailable(const QString& reason) {
  (void)reason;
  m_unavailable = true;
  m_hasRuntime = false;
  refreshText();
}

void OverlayFloatingWidget::setFilterEnabled(const bool enabled) {
  m_filterEnabled = enabled;
  refreshText();
}

void OverlayFloatingWidget::showEvent(QShowEvent* ev) {
  QWidget::showEvent(ev);
  if (!m_positionInitialized) {
    moveToDefaultPosition();
    m_positionInitialized = true;
  }
  makeWindowMouseTransparent(reinterpret_cast<HWND>(winId()));
  syncHandleGeometry();
  if (m_handle) m_handle->show();
}

void OverlayFloatingWidget::hideEvent(QHideEvent* ev) {
  if (m_handle) m_handle->hide();
  QWidget::hideEvent(ev);
}

void OverlayFloatingWidget::moveEvent(QMoveEvent* ev) {
  QWidget::moveEvent(ev);
  syncHandleGeometry();
}

void OverlayFloatingWidget::paintEvent(QPaintEvent*) {
  QPainter p(this);
  p.setRenderHint(QPainter::Antialiasing, true);

  const auto& tm = ThemeManager::instance();
  const QRectF r = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);
  QPainterPath path;
  path.addRoundedRect(r, kRadius, kRadius);
  QColor bg = tm.color(Sem::Surface);
  bg.setAlphaF(kSurfaceAlpha);
  p.fillPath(path, bg);
  p.setPen(QPen(tm.color(Sem::Border), 1.0));
  p.drawPath(path);
}

void OverlayFloatingWidget::applyTheme() {
  const auto& tm = ThemeManager::instance();
  const QString fg = tm.color(Sem::Fg).name();
  setStyleSheet(QStringLiteral("#overlayFloatingWidget { background: transparent; }"
                               "#overlayFloatUsage { color: %1; font-weight: 600; }")
                    .arg(fg));
  if (m_handle) m_handle->update();
  update();
  refreshText();
}

void OverlayFloatingWidget::refreshText() {
  if (m_unavailable) {
    if (m_usage) m_usage->setText(QStringLiteral("—"));
    resizeToContent();
    return;
  }

  if (!m_filterEnabled) {
    if (m_usage) m_usage->setText(QStringLiteral("—"));
    resizeToContent();
    return;
  }

  if (!m_hasRuntime) {
    if (m_usage) m_usage->setText(QStringLiteral("—"));
    resizeToContent();
    return;
  }

  const uint32_t totalMb = m_runtime.availableSpaceMb + m_runtime.currentConsumptionMb;
  if (m_usage) m_usage->setText(formatUsageText(m_runtime.currentConsumptionMb, totalMb));
  resizeToContent();
}

void OverlayFloatingWidget::resizeToContent() {
  if (!m_usage) return;

  const int textW = m_usage->sizeHint().width();
  const int desiredW = std::max(kMinWindowW, kContentLeftMargin + textW + contentRightMargin());
  if (desiredW == width() && height() == kWindowH) return;

  const int oldRight = geometry().right();
  resize(desiredW, kWindowH);
  if (isVisible() && m_positionInitialized) {
    move(clampedTopLeft(QPoint(oldRight - width() + 1, y()), size()));
  }
  syncHandleGeometry();
}

void OverlayFloatingWidget::moveToDefaultPosition() {
  const QRect g = primaryDesktopGeometry();
  move(QPoint(g.right() - width() + 1, g.bottom() - height() + 1));
}

void OverlayFloatingWidget::syncHandleGeometry() {
  if (!m_handle) return;
  const QRect r = handleRectForSize(size());
  m_handle->move(frameGeometry().topLeft() + r.topLeft());
}

void OverlayFloatingWidget::moveByHandleDrag(const QPoint& globalPos, const QPoint& dragOffset) {
  move(clampedTopLeft(globalPos - dragOffset, size()));
  syncHandleGeometry();
}

}  // namespace uwf::ui
