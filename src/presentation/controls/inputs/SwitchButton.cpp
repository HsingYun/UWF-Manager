// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "presentation/controls/inputs/SwitchButton.h"

#include <QMouseEvent>
#include <QPainter>
#include <QScopedValueRollback>
#include <algorithm>

#include "presentation/design/controls/ControlAnimation.h"
#include "presentation/design/controls/ControlShape.h"
#include "presentation/design/theme/DesignSystem.h"
namespace uwf::ui {
SwitchButton::SwitchButton(QWidget* parent) : QAbstractButton(parent) {
  setCheckable(true);
  setCursor(Qt::PointingHandCursor);
  setFocusPolicy(Qt::StrongFocus);
  setFixedSize(SwitchButton::sizeHint());
  setProperty("designRadius", design::switchHeight / 2.0);
  ControlAnimation::attach(this);
  m_position.setDuration(design::switchDurationMs);
  QEasingCurve easing(QEasingCurve::OutBack);
  easing.setOvershoot(1.1);
  m_position.setEasingCurve(easing);
  connect(&m_position, &QVariantAnimation::valueChanged, this, [this](const QVariant& value) {
    m_checked = value.toReal();
    update();
  });

  connect(&DesignSystem::instance(), &DesignSystem::changed, this, qOverload<>(&QWidget::update));
}
void SwitchButton::checkStateSet() {
  QAbstractButton::checkStateSet();
  if (!m_userChange) updatePosition(!signalsBlocked() && isVisible());
}
void SwitchButton::nextCheckState() {
  const QScopedValueRollback changing(m_userChange, true);
  QAbstractButton::nextCheckState();
  updatePosition(isVisible() && !signalsBlocked());
}
void SwitchButton::updatePosition(const bool animate) {
  const qreal target = isChecked() ? 1.0 : 0.0;
  if (animate && target == m_target && m_position.state() == QAbstractAnimation::Running) return;
  m_target = target;
  m_position.stop();
  if (!animate || m_checked == target) {
    m_checked = target;
    update();
    return;
  }
  m_position.setStartValue(m_checked);
  m_position.setEndValue(target);
  m_position.start();
}
QSize SwitchButton::sizeHint() const { return {design::switchWidth, design::switchHeight}; }
void SwitchButton::setUnknown(const bool unknown) {
  if (m_unknown == unknown) return;
  m_unknown = unknown;
  update();
}
void SwitchButton::mousePressEvent(QMouseEvent* event) {
  if (event->button() == Qt::LeftButton && isEnabled()) {
    m_pressPosition = event->position();
    m_dragOrigin = isChecked() ? 1.0 : 0.0;
    m_pointerActive = true;
  }
  QAbstractButton::mousePressEvent(event);
}
void SwitchButton::mouseMoveEvent(QMouseEvent* event) {
  if (!m_pointerActive || !isEnabled()) {
    QAbstractButton::mouseMoveEvent(event);
    return;
  }
  const qreal delta = event->position().x() - m_pressPosition.x();
  if (!m_dragging && qAbs(delta) < 3) return;
  m_dragging = true;
  m_position.stop();
  const qreal raw = m_dragOrigin + delta / design::switchTravel;
  const qreal bounded = std::clamp(raw, 0.0, 1.0);
  m_checked = bounded < .15 ? bounded * .45 : bounded > .85 ? 1 - (1 - bounded) * .45 : bounded;
  if (raw < 0) m_checked = -std::min(.08, -raw * .15);
  if (raw > 1) m_checked = 1 + std::min(.08, (raw - 1) * .15);
  m_stretch = std::min(3.0, qAbs(delta) * .08);
  setDown(true);
  update();
  event->accept();
}
void SwitchButton::mouseReleaseEvent(QMouseEvent* event) {
  m_pointerActive = false;
  if (!m_dragging || event->button() != Qt::LeftButton) {
    QAbstractButton::mouseReleaseEvent(event);
    return;
  }
  const bool target = m_checked >= .5;
  const bool changed = target != isChecked();
  m_dragging = false;
  m_stretch = 0;
  setDown(false);
  {
    const QScopedValueRollback changing(m_userChange, true);
    setChecked(target);
  }
  updatePosition(isVisible() && !signalsBlocked());
  emit released();
  if (changed) emit clicked(isChecked());
  event->accept();
}
bool SwitchButton::event(QEvent* event) {
  if (m_pointerActive && (event->type() == QEvent::UngrabMouse || event->type() == QEvent::Hide || (event->type() == QEvent::EnabledChange && !isEnabled()))) {
    m_pointerActive = m_dragging = false;
    m_stretch = 0;
    setDown(false);
    updatePosition(false);
  }
  if (event->type() == QEvent::EnabledChange) {
    setCursor(isEnabled() ? Qt::PointingHandCursor : Qt::ArrowCursor);
    // A disabled switch conveys the observed value, not an intermediate
    // animation frame that might resemble the opposite value.
    if (!isEnabled()) updatePosition(false);
  }
  return QAbstractButton::event(event);
}
void SwitchButton::paintEvent(QPaintEvent*) {
  using design::Color;
  auto& theme = DesignSystem::instance();
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);
  if (m_unknown) {
    paintControlShape(painter, rect(), height() / 2.0, !isEnabled() ? theme.color(Color::SurfaceSoft) : QColor(Qt::transparent),
                      theme.color(!isEnabled() ? Color::Line : Color::Faint));
    painter.setPen(QPen(theme.color(!isEnabled() ? Color::Faint : Color::Muted), 1.5, Qt::SolidLine, Qt::RoundCap));
    painter.drawLine(QPointF(width() / 2.0 - 4, height() / 2.0), QPointF(width() / 2.0 + 4, height() / 2.0));
    return;
  }
  const bool on = m_checked >= .5;
  const auto ink = [&](const QColor& color) {
    if (!underMouse() || !isEnabled()) return color;
    return QColor(std::min(255, qRound(color.red() * 1.08)), std::min(255, qRound(color.green() * 1.08)), std::min(255, qRound(color.blue() * 1.08)),
                  color.alpha());
  };
  QColor track = isEnabled() ? DesignSystem::mix(theme.color(Color::SurfaceSoft), ink(theme.color(Color::Accent)), std::clamp(m_checked, 0.0, 1.0))
                             : theme.color(on ? Color::Line : Color::SurfaceSoft);
  auto border = ink(theme.color(isEnabled() ? on ? Color::Accent : Color::Faint : Color::Line));
  auto thumbColor = ink(theme.color(isEnabled() ? on ? Color::AccentText : m_offThumbColor : on ? Color::Surface : Color::Line));
  if (!isEnabled()) {
    if (on) {
      constexpr qreal disabledTrackTint = .22;
      track = DesignSystem::mix(theme.color(Color::Surface), theme.color(Color::Accent), disabledTrackTint);
      border = Qt::transparent;
    }
    thumbColor = theme.color(on ? Color::Accent : Color::Faint);
    constexpr float disabledOnThumbOpacity = .85F, disabledOffThumbOpacity = .6F;
    thumbColor.setAlphaF(on ? disabledOnThumbOpacity : disabledOffThumbOpacity);
  }
  paintControlShape(painter, rect(), height() / 2.0, track, border);
  painter.setPen(Qt::NoPen);
  painter.setBrush(thumbColor);
  const qreal thumb = height() - 2.0 * design::switchOffset;
  const qreal travel = width() - 2.0 * design::switchOffset - thumb;
  painter.drawEllipse(QRectF(design::switchOffset + travel * m_checked - m_stretch / 2, (height() - thumb) / 2.0, thumb + m_stretch, thumb));
}
}  // namespace uwf::ui
#include "moc_SwitchButton.cpp"
