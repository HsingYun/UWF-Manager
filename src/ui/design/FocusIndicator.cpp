// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "FocusIndicator.h"

#include <QAbstractButton>
#include <QAbstractSpinBox>
#include <QApplication>
#include <QComboBox>
#include <QFocusFrame>
#include <QKeyEvent>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QTextEdit>
namespace uwf::ui {
namespace {
bool isTextEditingControl(const QWidget* control) {
  for (; control; control = control->parentWidget()) {
    if (qobject_cast<const QLineEdit*>(control) || qobject_cast<const QTextEdit*>(control) || qobject_cast<const QPlainTextEdit*>(control) ||
        qobject_cast<const QAbstractSpinBox*>(control))
      return true;
    if (const auto* combo = qobject_cast<const QComboBox*>(control); combo && combo->isEditable()) return true;
    if (control->isWindow()) break;
  }
  return false;
}
}  // namespace
void FocusIndicator::install() {
  if (!qApp->findChild<QObject*>(QStringLiteral("designFocusIndicator"), Qt::FindDirectChildrenOnly)) new FocusIndicator();
}
bool FocusIndicator::keyboardNavigation() {
  const auto* indicator = qApp->findChild<QObject*>(QStringLiteral("designFocusIndicator"), Qt::FindDirectChildrenOnly);
  return indicator && static_cast<const FocusIndicator*>(indicator)->m_keyboard;
}
FocusIndicator::FocusIndicator() : QObject(qApp) {
  setObjectName(QStringLiteral("designFocusIndicator"));
  qApp->installEventFilter(this);
  connect(qApp, &QApplication::focusChanged, this, [this] { updateIndicator(); });
}
void FocusIndicator::setKeyboardNavigation(const bool keyboard) {
  if (m_keyboard == keyboard) return;
  m_keyboard = keyboard;
  updateIndicator();
}
void FocusIndicator::updateIndicator() {
  auto* target = QApplication::focusWidget();
  if (target && qobject_cast<QLineEdit*>(target) &&
      (qobject_cast<QAbstractSpinBox*>(target->parentWidget()) || qobject_cast<QComboBox*>(target->parentWidget())))
    target = target->parentWidget();
  const bool supported = target && (qobject_cast<QAbstractButton*>(target) || qobject_cast<QComboBox*>(target) || qobject_cast<QAbstractSpinBox*>(target) ||
                                    qobject_cast<QLineEdit*>(target));
  if (!m_keyboard || !supported || !target->isVisible() || !target->isEnabled()) {
    if (m_frame) {
      m_frame->setWidget(nullptr);
      m_frame->hide();
    }
    return;
  }
  if (!m_frame) m_frame = new QFocusFrame(target->window());
  m_frame->setWidget(target);
  m_frame->show();
}
bool FocusIndicator::eventFilter(QObject*, QEvent* event) {
  if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::TouchBegin) setKeyboardNavigation(false);
  if (event->type() == QEvent::KeyPress) {
    const int key = static_cast<QKeyEvent*>(event)->key();
    const bool traversal = key == Qt::Key_Tab || key == Qt::Key_Backtab;
    const bool editing = isTextEditingControl(QApplication::focusWidget());
    if (traversal || (!editing && key != Qt::Key_Shift && key != Qt::Key_Control && key != Qt::Key_Alt && key != Qt::Key_Meta))
      setKeyboardNavigation(true);
  }

  return false;
}
}  // namespace uwf::ui
