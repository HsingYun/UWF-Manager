// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "BusyIndicator.h"

#include <QPainter>

#include "ui/design/DesignSystem.h"
namespace uwf::ui {
BusyIndicator::BusyIndicator(QWidget* parent) : QWidget(parent) {
  setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  setFocusPolicy(Qt::NoFocus);
  m_rotation.setDuration(design::spinnerDurationMs);
  m_rotation.setStartValue(0.0);
  m_rotation.setEndValue(360.0);
  m_rotation.setLoopCount(-1);
  connect(&m_rotation, &QVariantAnimation::valueChanged, this, qOverload<>(&QWidget::update));
  connect(&DesignSystem::instance(), &DesignSystem::changed, this, qOverload<>(&QWidget::update));
}
QSize BusyIndicator::sizeHint() const { return {24, 24}; }
void BusyIndicator::setColorRole(const design::Color role) {
  m_color = role;
  update();
}
void BusyIndicator::setStrokeWidth(const qreal width) {
  m_strokeWidth = width;
  update();
}
void BusyIndicator::paintEvent(QPaintEvent*) {
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);
  const qreal side = std::min(width(), height());
  painter.translate(width() / 2.0, height() / 2.0);
  painter.scale(side / 24.0, side / 24.0);
  painter.rotate(m_rotation.currentValue().toReal());
  const QColor ink = DesignSystem::instance().color(m_color);
  painter.setPen(QPen(ink, m_strokeWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
  painter.setBrush(Qt::NoBrush);
  painter.setOpacity(.2);
  painter.drawEllipse(QRectF(-8, -8, 16, 16));
  painter.setOpacity(1);
  painter.drawArc(QRectF(-8, -8, 16, 16), 0, 90 * 16);
}
void BusyIndicator::showEvent(QShowEvent* event) {
  QWidget::showEvent(event);
  m_rotation.start();
}
void BusyIndicator::hideEvent(QHideEvent* event) {
  m_rotation.stop();
  QWidget::hideEvent(event);
}
}  // namespace uwf::ui
#include "moc_BusyIndicator.cpp"
