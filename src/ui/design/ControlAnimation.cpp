// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "ControlAnimation.h"

#include <QAbstractButton>
#include <QComboBox>
#include <QEvent>
#include <QProgressBar>
#include <QWidget>

#include "DesignSystem.h"

namespace uwf::ui {
void ControlAnimation::attach(QWidget* widget) {
  if (!widget->findChild<ControlAnimation*>(QStringLiteral("designAnimation"), Qt::FindDirectChildrenOnly)) new ControlAnimation(widget);
}
ControlAnimation::ControlAnimation(QWidget* widget) : QObject(widget), m_widget(widget) {
  setObjectName(QStringLiteral("designAnimation"));
  widget->setProperty("designHover", 0.0);
  widget->setProperty("designPress", 0.0);
  widget->setAttribute(Qt::WA_Hover);
  widget->installEventFilter(this);
  m_busy.setDuration(design::busyDurationMs);
  m_busy.setStartValue(0.0);
  m_busy.setEndValue(1.0);
  m_busy.setLoopCount(-1);
  QEasingCurve busyEasing(QEasingCurve::BezierSpline);
  busyEasing.addCubicBezierSegment(QPointF(.42, 0), QPointF(.58, 1), QPointF(1, 1));
  m_busy.setEasingCurve(busyEasing);
  connect(&m_busy, &QVariantAnimation::valueChanged, widget, [widget](const QVariant& value) {
    widget->setProperty("designBusyPhase", value);
    widget->update();
  });
  m_hover.setDuration(design::hoverDurationMs);
  QEasingCurve easing(QEasingCurve::BezierSpline);
  easing.addCubicBezierSegment(QPointF(0, 0), QPointF(.58, 1), QPointF(1, 1));
  m_hover.setEasingCurve(easing);
  m_press.setDuration(design::hoverDurationMs);
  m_press.setEasingCurve(easing);
  m_progress.setDuration(design::progressDurationMs);
  m_progress.setEasingCurve(QEasingCurve::Linear);
  if (auto* progress = qobject_cast<QProgressBar*>(widget)) {
    m_progressMinimum = progress->minimum();
    m_progressMaximum = progress->maximum();
    widget->setProperty("designProgressValue", progress->value());
    connect(progress, &QProgressBar::valueChanged, this, [this, progress](const int value) {
      const qreal before = m_widget->property("designProgressValue").toReal();
      m_progress.stop();
      if (!progress->isVisible() || value < progress->minimum() || progress->maximum() <= progress->minimum()) {
        m_widget->setProperty("designProgressValue", value);
      } else {
        m_progress.setStartValue(before);
        m_progress.setEndValue(static_cast<qreal>(value));
        m_progress.start();
      }
    });
    connect(&m_progress, &QVariantAnimation::valueChanged, widget, [widget](const QVariant& value) {
      widget->setProperty("designProgressValue", value);
      widget->update();
    });
  }
  connect(&m_press, &QVariantAnimation::valueChanged, widget, [widget](const QVariant& value) {
    widget->setProperty("designPress", value);
    widget->update();
  });
  if (auto* button = qobject_cast<QAbstractButton*>(widget)) {
    connect(button, &QAbstractButton::pressed, this, [this] { setPressed(true); });
    connect(button, &QAbstractButton::released, this, [this] { setPressed(false); });
  }
  connect(&m_hover, &QVariantAnimation::valueChanged, widget, [widget](const QVariant& value) {
    widget->setProperty("designHover", value);
    widget->update();
  });
}
qreal ControlAnimation::hover(const QWidget* widget, const bool hovered) {
  if (!widget) return hovered ? 1.0 : 0.0;
  if (auto* animation = widget->findChild<ControlAnimation*>(QStringLiteral("designAnimation"), Qt::FindDirectChildrenOnly);
      animation && widget->isVisible() && widget->isEnabled() && animation->m_hoverTarget != hovered)
    animation->setHovered(hovered);
  const auto value = widget->property("designHover");
  return value.isValid() ? value.toReal() : (hovered ? 1.0 : 0.0);
}
qreal ControlAnimation::press(const QWidget* widget, const bool pressed) {
  if (!widget) return pressed ? 1.0 : 0.0;
  if (auto* animation = widget->findChild<ControlAnimation*>(QStringLiteral("designAnimation"), Qt::FindDirectChildrenOnly);
      animation && widget->isVisible() && widget->isEnabled() && animation->m_pressTarget != pressed)
    animation->setPressed(pressed);
  const auto value = widget->property("designPress");
  return value.isValid() ? value.toReal() : (pressed ? 1.0 : 0.0);
}
void ControlAnimation::setPressed(const bool pressed) {
  if (m_pressTarget == pressed) return;
  const auto current = m_widget->property("designPress");
  m_pressTarget = pressed;
  m_press.stop();
  m_press.setDuration(pressed ? design::pressDurationMs : design::hoverDurationMs);
  m_press.setStartValue(current.isValid() ? current.toReal() : (pressed ? 0.0 : 1.0));
  m_press.setEndValue(pressed ? 1.0 : 0.0);
  m_press.start();
}
void ControlAnimation::setHovered(const bool hovered) {
  if (m_hoverTarget == hovered) return;
  const auto current = m_widget->property("designHover");
  m_hoverTarget = hovered;
  m_hover.stop();
  m_hover.setStartValue(current.isValid() ? current.toReal() : (hovered ? 0.0 : 1.0));
  m_hover.setEndValue(hovered ? 1.0 : 0.0);
  m_hover.start();
}
bool ControlAnimation::eventFilter(QObject* object, QEvent* event) {
  if (object != m_widget) return false;
  if (auto* progress = qobject_cast<QProgressBar*>(m_widget); progress && (event->type() == QEvent::Paint || event->type() == QEvent::Hide)) {
    if (event->type() == QEvent::Hide || m_progressMinimum != progress->minimum() || m_progressMaximum != progress->maximum()) {
      m_progress.stop();
      m_widget->setProperty("designProgressValue", progress->value());
      m_progressMinimum = progress->minimum();
      m_progressMaximum = progress->maximum();
    }
    const bool running = progress->isVisible() && progress->minimum() == 0 && progress->maximum() == 0;
    if (running && m_busy.state() != QAbstractAnimation::Running)
      m_busy.start();
    else if (!running)
      m_busy.stop();
  }
  if (event->type() == QEvent::Enter || event->type() == QEvent::HoverEnter) setHovered(true);
  if (event->type() == QEvent::Leave || event->type() == QEvent::HoverLeave) setHovered(false);
  if (event->type() == QEvent::FocusOut || event->type() == QEvent::Hide || event->type() == QEvent::EnabledChange) {
    if (!m_widget->isVisible() || !m_widget->isEnabled()) {
      m_hoverTarget = false;
      m_pressTarget = false;
      m_hover.stop();
      m_press.stop();
      m_widget->setProperty("designHover", 0.0);
      m_widget->setProperty("designPress", 0.0);
    }
  }
  return false;
}
}  // namespace uwf::ui
#include "moc_ControlAnimation.cpp"
