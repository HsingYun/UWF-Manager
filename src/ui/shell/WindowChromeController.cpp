// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "WindowChromeController.h"

#include <dwmapi.h>
#include <windows.h>

#include <QAbstractButton>
#include <QApplication>
#include <QCoreApplication>
#include <QEvent>
#include <QLayout>
#include <QMainWindow>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QPointer>
#include <QScreen>
#include <QStyle>
#include <QStyleOption>
#include <QSvgRenderer>
#include <QTimer>
#include <QWindow>

#include "design/DesignTokens.h"
#include "ui/appearance/WindowAppearance.h"

namespace uwf::ui {

namespace {

bool isWindowDragTarget(QObject* object) {
  auto* widget = qobject_cast<QWidget*>(object);
  for (auto* current = widget; current; current = current->parentWidget()) {
    if (qobject_cast<QAbstractButton*>(current) || qobject_cast<QMenu*>(current)) return false;
    if (current->property("windowDragSurface").toBool()) return true;
  }
  return false;
}

}  // namespace

WindowChromeController::WindowChromeController(QMainWindow* window, QObject* parent) : QObject(parent), m_window(window) {
  m_window->setProperty("nativeCaptionCommands", true);
  qApp->installEventFilter(this);
}

void WindowChromeController::applyTitleBarTheme() const { applyWindowAppearance(m_window); }

QRect WindowChromeController::startupGeometry(const QRect& available, const QMargins& frameMargins, const QSize& minimumClientSize) {
  if (available.isEmpty()) return {};
  const QSize frame(frameMargins.left() + frameMargins.right(), frameMargins.top() + frameMargins.bottom());
  const QSize availableClient = (available.size() - frame).expandedTo(QSize(1, 1));
  // 保留工作区四周的留白；小屏优先容纳控件，但窗口外框始终受工作区约束。
  const QSize preferred(design::mainWidth, design::mainHeight);
  const QSize comfortable = (QSize(qRound(available.width() * 0.9), qRound(available.height() * 0.9)) - frame).expandedTo(QSize(1, 1));
  const QSize minimum = minimumClientSize.expandedTo(QSize(1, 1)).boundedTo(availableClient);
  const QSize client = preferred.boundedTo(comfortable).expandedTo(minimum).boundedTo(availableClient);
  const QSize outer = client + frame;
  return {available.topLeft() +
              QPoint((available.width() - outer.width()) / 2 + frameMargins.left(), (available.height() - outer.height()) / 2 + frameMargins.top()),
          client};
}

void WindowChromeController::initializeGeometry() const {
  if (m_window->isMaximized() || m_window->isFullScreen()) return;
  auto* screen = m_window->screen();
  auto* handle = m_window->windowHandle();
  if (!screen || !handle) return;
  const auto margins = handle->frameMargins();
  const QRect available = screen->availableGeometry();
  const QSize maximum = available.size() - QSize(margins.left() + margins.right(), margins.top() + margins.bottom());
  const auto minimumAtWidth = [&](const int width) {
    QSize minimum = m_window->minimumSizeHint().expandedTo(m_window->minimumSize());
    if (auto* central = m_window->centralWidget(); central && central->layout()) {
      const QSize chrome = m_window->size() - central->size();
      const int contentHeight = central->layout()->totalMinimumHeightForWidth(qMax(1, width - chrome.width()));
      if (contentHeight >= 0) minimum.setHeight(qMax(minimum.height(), contentHeight + chrome.height()));
    }
    return minimum;
  };
  auto geometry = startupGeometry(available, margins, m_window->minimumSizeHint().expandedTo(m_window->minimumSize()));
  auto minimum = minimumAtWidth(geometry.width());
  if (minimum.height() > maximum.height()) {
    minimum = minimumAtWidth(maximum.width());
    minimum.setWidth(maximum.width());
  }
  geometry = startupGeometry(available, margins, minimum);
  if (geometry.isEmpty()) return;
  // Qt 已提交的布局最小尺寸也不能迫使原生窗口超出小屏的可用区域。
  m_window->setMinimumSize(m_window->minimumSize().boundedTo(geometry.size()));
  m_window->setGeometry(geometry);
}

void WindowChromeController::raiseToFront() const {
  if (m_window->isMinimized())
    m_window->showNormal();
  else
    m_window->show();
  m_window->raise();
  m_window->activateWindow();
  refreshWindowBackdrop(m_window);
}

bool WindowChromeController::nativeEvent(void* message, qintptr* result) {
  const auto* native = static_cast<const MSG*>(message);
  if (handleWindowResize(m_window, message, result) || handleWindowMenu(m_window, message, result)) return true;
  if (native->message == WM_CANCELMODE || native->message == WM_CAPTURECHANGED) {
    if (m_pressedCaption) m_pressedCaption->setDown(false);
    m_pressedCaption.clear();
  }
  if (m_pressedCaption && (native->message == WM_MOUSEMOVE || native->message == WM_LBUTTONUP)) {
    const qreal scale = m_window->devicePixelRatioF();
    const QPoint point(qRound(static_cast<short>(LOWORD(native->lParam)) / scale), qRound(static_cast<short>(HIWORD(native->lParam)) / scale));
    const bool inside = m_pressedCaption->rect().contains(m_pressedCaption->mapFrom(m_window, point));
    m_pressedCaption->setDown(inside);
    m_pressedCaption->setProperty("nativeCaptionHover", inside);
    if (native->message == WM_LBUTTONUP) {
      const QPointer<QAbstractButton> button = m_pressedCaption;
      m_pressedCaption.clear();
      button->setDown(false);
      button->setProperty("nativeCaptionHover", false);
      ReleaseCapture();
      // The Qt button routes to WM_SYSCOMMAND. Track the custom rectangle here:
      // DefWindowProc's press loop assumes system caption-button geometry.
      if (inside && button && button->isEnabled()) button->click();
    }
    *result = 0;
    return true;
  }
  if (native->message == WM_NCHITTEST) {
    POINT point{static_cast<short>(LOWORD(native->lParam)), static_cast<short>(HIWORD(native->lParam))};
    if (ScreenToClient(native->hwnd, &point)) {
      const qreal scale = m_window->devicePixelRatioF();
      auto* target = m_window->childAt(QPoint(qRound(point.x / scale), qRound(point.y / scale)));
      for (auto* current = target; current && current != m_window; current = current->parentWidget()) {
        const int hit = current->property("nativeCaptionHit").toInt();
        if (hit && current->isEnabled()) {
          *result = hit;
          return true;
        }
      }
      if (isWindowDragTarget(target)) {
        *result = HTCAPTION;
        return true;
      }
    }
  }
  if (native->message == WM_NCMOUSEMOVE || native->message == WM_NCMOUSEHOVER || native->message == WM_NCMOUSELEAVE) {
    for (const auto buttons = m_window->findChildren<QAbstractButton*>(); auto* button : buttons) {
      const int hit = button->property("nativeCaptionHit").toInt();
      if (!hit) continue;
      button->setProperty("nativeCaptionHover", native->message != WM_NCMOUSELEAVE && native->wParam == static_cast<WPARAM>(hit));
      button->update();
    }
    if (native->message == WM_NCMOUSEMOVE) {
      TRACKMOUSEEVENT tracking{sizeof(TRACKMOUSEEVENT), TME_HOVER | TME_LEAVE | TME_NONCLIENT, native->hwnd, HOVER_DEFAULT};
      TrackMouseEvent(&tracking);
    }
    // DWM owns caption hover and the system Snap Layouts flyout, including the
    // hover/leave messages that Qt's frameless platform path may consume.
    LRESULT nativeResult = 0;
    if (!DwmDefWindowProc(native->hwnd, native->message, native->wParam, native->lParam, &nativeResult))
      nativeResult = DefWindowProcW(native->hwnd, native->message, native->wParam, native->lParam);
    *result = nativeResult;
    return true;
  }
  if ((native->message == WM_NCLBUTTONDOWN || native->message == WM_NCLBUTTONDBLCLK) &&
      (native->wParam == HTMINBUTTON || native->wParam == HTMAXBUTTON || native->wParam == HTCLOSE)) {
    QPointer<QAbstractButton> button;
    for (const auto candidates = m_window->findChildren<QAbstractButton*>(); auto* candidate : candidates)
      if (candidate->property("nativeCaptionHit").toInt() == static_cast<int>(native->wParam)) button = candidate;
    if (button) {
      m_pressedCaption = button;
      button->setDown(true);
      SetCapture(native->hwnd);
    }
    *result = 0;
    return true;
  }
  return false;
}

bool WindowChromeController::eventFilter(QObject* obj, QEvent* ev) {
  const auto* widget = qobject_cast<QWidget*>(obj);
  if (!widget || widget->window() != m_window) return QObject::eventFilter(obj, ev);
  if (obj == m_window && ev->type() == QEvent::WindowStateChange) {
    for (const auto buttons = m_window->findChildren<QAbstractButton*>(); auto* button : buttons)
      if (button->property("nativeCaptionHit").isValid()) button->update();
  }
  if (ev->type() == QEvent::MouseButtonDblClick && isWindowDragTarget(obj)) {
    const auto* mouse = static_cast<QMouseEvent*>(ev);
    if (mouse->button() == Qt::LeftButton) {
      SendMessageW(reinterpret_cast<HWND>(m_window->winId()), WM_SYSCOMMAND, m_window->isMaximized() ? SC_RESTORE : SC_MAXIMIZE, 0);
      return true;
    }
  }
  if (ev->type() == QEvent::MouseButtonPress && isWindowDragTarget(obj)) {
    auto* me = static_cast<QMouseEvent*>(ev);
    if (me->button() == Qt::LeftButton) {
      const QPoint globalPos = me->globalPosition().toPoint();
      // startSystemMove() leaves Qt's toolbar hover tracking stale after a
      // click on Windows.  Let the native caption loop own this interaction;
      // unlike a delayed startSystemMove(), it also preserves drag-to-move.
      startWindowDrag(m_window, globalPos);
      return true;
    }
  }
  return QObject::eventFilter(obj, ev);
}

}  // namespace uwf::ui
