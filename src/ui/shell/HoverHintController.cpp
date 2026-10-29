// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "HoverHintController.h"

#include <QAbstractItemView>
#include <QAction>
#include <QApplication>
#include <QCursor>
#include <QDialog>
#include <QHoverEvent>
#include <QMenu>
#include <QMouseEvent>
#include <QTabBar>
#include <QWidget>

#include "design/DesignTokens.h"

namespace uwf::ui {
namespace {
QPoint eventPosition(QWidget* widget, QEvent* event) {
  if (const auto* mouse = dynamic_cast<QMouseEvent*>(event)) return mouse->position().toPoint();
  if (const auto* hover = dynamic_cast<QHoverEvent*>(event)) return hover->position().toPoint();
  return widget->mapFromGlobal(QCursor::pos());
}
}  // namespace
HoverHintController::HoverHintController(QWidget* rootWindow, QObject* parent) : QObject(parent), m_rootWindow(rootWindow) {
  m_delay.setSingleShot(true);
  m_delay.setTimerType(Qt::PreciseTimer);
  m_delay.setInterval(design::statusHoverDelayMs);
  connect(&m_delay, &QTimer::timeout, this, [this] {
    if (m_target) m_target->showHover(m_source ? m_hint : QString{});
  });
  qApp->installEventFilter(this);
}
void HoverHintController::setTarget(TransientLabel* target) {
  clearHint();
  m_target = target;
}
void HoverHintController::clearHint() {
  m_delay.stop();
  m_source.clear();
  m_hint.clear();
  if (m_target) m_target->showHover({});
}
void HoverHintController::queueHint(QWidget* source, const QString& text) {
  // Mouse events can bubble from a hinted child to an unhinted ancestor.
  if (text.isEmpty() && m_source && source != m_source && source->isAncestorOf(m_source)) return;
  if (m_source == source && m_hint == text) return;
  m_source = source;
  m_hint = text;
  m_delay.start();
}
bool HoverHintController::eventFilter(QObject* object, QEvent* event) {
  if (!m_target || !m_rootWindow) return false;
  auto* widget = qobject_cast<QWidget*>(object);
  if (!widget) return false;
  const auto type = event->type();
  // Dialogs retain their own tooltips and never publish a pending main hint.
  bool owned = false;
  for (QObject* ancestor = object; ancestor; ancestor = ancestor->parent()) {
    if (ancestor == m_rootWindow) owned = true;
    if (qobject_cast<QDialog*>(ancestor)) {
      if (type == QEvent::Show) clearHint();
      return false;
    }
  }
  auto* menu = qobject_cast<QMenu*>(widget);
  if (widget->window() != m_rootWindow) {
    if (!menu || !owned) return false;
  }
  if (type == QEvent::ToolTip) return true;
  if (type == QEvent::MouseButtonPress) {
    clearHint();
    return false;
  }
  if (type == QEvent::Leave || type == QEvent::HoverLeave || type == QEvent::Hide) {
    if (m_source && (widget == m_source || widget->isAncestorOf(m_source))) {
      m_delay.stop();
      m_source.clear();
      m_hint.clear();
      m_delay.start();
    }
    return false;
  }
  if (type != QEvent::Enter && type != QEvent::HoverEnter && type != QEvent::MouseMove && type != QEvent::HoverMove) return false;
  const auto position = eventPosition(widget, event);
  QString hint;
  if (menu) {
    const auto* action = menu->actionAt(position);
    if (action && action->toolTip() != action->text()) hint = action->toolTip();
  } else {
    if (auto* bar = qobject_cast<QTabBar*>(widget)) hint = bar->tabToolTip(bar->tabAt(position));
    if (auto* view = qobject_cast<QAbstractItemView*>(widget->parentWidget()); view && view->viewport() == widget)
      hint = view->indexAt(position).data(Qt::ToolTipRole).toString();
    if (hint.isEmpty()) {
      auto* ancestor = widget;
      while (ancestor && ancestor->toolTip().isEmpty() && ancestor != m_rootWindow) ancestor = ancestor->parentWidget();
      if (ancestor) hint = ancestor->toolTip();
    }
  }
  queueHint(widget, hint);
  return false;
}
}  // namespace uwf::ui
