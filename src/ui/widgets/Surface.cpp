// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "Surface.h"

#include <QPainter>

#include "ui/design/ControlShape.h"
#include "ui/design/SurfaceClip.h"
namespace uwf::ui {
Surface::Surface(const design::Color fill, QWidget* parent) : QWidget(parent), m_fill(fill) {
  connect(&DesignSystem::instance(), &DesignSystem::changed, this, qOverload<>(&QWidget::update));
}
void Surface::setFill(const design::Color fill) {
  m_fill = fill;
  update();
}
void Surface::setBackdropOpacity(const int percent) {
  m_backdropOpacity = qBound(0, percent, 100);
  update();
}
void Surface::setBorder(const Qt::Edges edges, const int radius) {
  m_edges = edges;
  m_radius = radius;
  if (m_clip) m_clip->setFrame(edges, radius);
  update();
}
void Surface::setClipContent(const bool clip) {
  if (!m_clip && clip) {
    m_clip = new SurfaceClip(this);
    setGraphicsEffect(m_clip);
  }
  if (m_clip) {
    m_clip->setFrame(m_edges, m_radius);
    m_clip->setEnabled(clip);
  }
  update();
}
void Surface::paintEvent(QPaintEvent*) {
  QPainter painter(this);
  const auto& designSystem = DesignSystem::instance();
  auto fill = designSystem.color(m_fill);
  if (window()->property("systemBackdropActive").toBool()) fill.setAlpha(qRound(255.0 * m_backdropOpacity / 100));
  if (fill.alpha() == 0 && m_backdropOpacity == 0) {
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(rect(), Qt::transparent);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
  }
  if (m_clip && m_clip->isEnabled()) {
    painter.fillRect(rect(), fill);
    return;
  }
  painter.setRenderHint(QPainter::Antialiasing);
  painter.setBrush(fill);
  painter.setPen(Qt::NoPen);
  painter.drawRoundedRect(QRectF(rect()), m_radius, m_radius);
  painter.setPen(designSystem.color(design::Color::Line));
  if (m_radius > 0 && m_edges == (Qt::LeftEdge | Qt::TopEdge | Qt::RightEdge | Qt::BottomEdge)) {
    paintControlShape(painter, QRectF(rect()), m_radius, {}, designSystem.color(design::Color::Line));
  } else {
    if (m_edges.testFlag(Qt::LeftEdge)) painter.drawLine(0, 0, 0, height() - 1);
    if (m_edges.testFlag(Qt::TopEdge)) painter.drawLine(0, 0, width() - 1, 0);
    if (m_edges.testFlag(Qt::RightEdge)) painter.drawLine(width() - 1, 0, width() - 1, height() - 1);
    if (m_edges.testFlag(Qt::BottomEdge)) painter.drawLine(0, height() - 1, width() - 1, height() - 1);
  }
}
}  // namespace uwf::ui
