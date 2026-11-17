// SPDX-License-Identifier: GPL-3.0-or-later
#include "presentation/views/shell/WindowChromeController.h"

#include <QApplication>
#include <QEvent>
#include <QMainWindow>
#include <QMouseEvent>
#include <QScreen>

#include "presentation/appearance/WindowAppearance.h"

namespace uwf::ui {
WindowChromeController::WindowChromeController(QMainWindow* window, QObject* parent) : QObject(parent), m_window(window) { qApp->installEventFilter(this); }
void WindowChromeController::applyTitleBarTheme() const {}
QRect WindowChromeController::startupGeometry(const QRect& available, const QMargins&, const QSize& minimum) {
  const auto size = QSize(1280, 800).expandedTo(minimum).boundedTo(available.size());
  return {available.center() - QPoint(size.width() / 2, size.height() / 2), size};
}
void WindowChromeController::initializeGeometry() const {
  if (m_window->screen()) m_window->setGeometry(startupGeometry(m_window->screen()->availableGeometry(), {}, m_window->minimumSizeHint()));
}
void WindowChromeController::raiseToFront() const {
  m_window->showNormal();
  m_window->raise();
  m_window->activateWindow();
}
bool WindowChromeController::nativeEvent(void*, qintptr*) { return false; }
bool WindowChromeController::eventFilter(QObject* object, QEvent* event) {
  auto* widget = qobject_cast<QWidget*>(object);
  if (widget && widget->window() == m_window && widget->property("windowDragSurface").toBool()) {
    if (event->type() == QEvent::MouseButtonPress) {
      auto* mouse = static_cast<QMouseEvent*>(event);
      if (mouse->button() == Qt::LeftButton) startWindowDrag(m_window, mouse->globalPosition().toPoint());
    } else if (event->type() == QEvent::MouseButtonDblClick && static_cast<QMouseEvent*>(event)->button() == Qt::LeftButton) {
      m_window->isMaximized() ? m_window->showNormal() : m_window->showMaximized();
      return true;
    }
  }
  return false;
}

}  // namespace uwf::ui
