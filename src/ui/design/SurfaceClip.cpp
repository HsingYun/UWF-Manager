// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "SurfaceClip.h"

#include <QPainter>
#include <QPainterPath>
#include <algorithm>

#include "DesignSystem.h"

namespace uwf::ui {
SurfaceClip::SurfaceClip(QObject* parent) : QGraphicsEffect(parent) {}
void SurfaceClip::setFrame(const Qt::Edges edges, const int radius) {
  m_edges = edges;
  if (m_radius != radius) m_mask = {};
  m_radius = radius;
  update();
}
void SurfaceClip::draw(QPainter* painter) {
  QPoint offset;
  const auto source = sourcePixmap(Qt::LogicalCoordinates, &offset, NoPad);
  if (source.isNull()) return;
  const qreal scale = source.devicePixelRatio();
  const QRectF bounds(QPointF(), source.deviceIndependentSize());
  if (m_mask.size() != source.size() || !qFuzzyCompare(m_mask.devicePixelRatio(), scale)) {
    m_mask = QImage(source.size(), QImage::Format_ARGB32_Premultiplied);
    m_mask.setDevicePixelRatio(scale);
    m_mask.fill(Qt::transparent);
    QPainter mask(&m_mask);
    mask.setRenderHint(QPainter::Antialiasing);
    mask.setPen(Qt::NoPen);
    mask.setBrush(Qt::white);
    mask.drawRoundedRect(bounds, m_radius, m_radius);
  }
  auto content = source.toImage().convertToFormat(QImage::Format_ARGB32_Premultiplied);
  content.setDevicePixelRatio(scale);
  {
    QPainter composite(&content);
    composite.setRenderHint(QPainter::Antialiasing);
    const auto line = DesignSystem::instance().color(design::Color::Line);
    if (m_edges == (Qt::LeftEdge | Qt::TopEdge | Qt::RightEdge | Qt::BottomEdge)) {
      QPainterPath frame;
      frame.addRect(bounds);
      frame.addRoundedRect(bounds.adjusted(1, 1, -1, -1), std::max(0, m_radius - 1), std::max(0, m_radius - 1));
      frame.setFillRule(Qt::OddEvenFill);
      composite.fillPath(frame, line);
    } else {
      if (m_edges.testFlag(Qt::LeftEdge)) composite.fillRect(QRectF(0, 0, 1, bounds.height()), line);
      if (m_edges.testFlag(Qt::TopEdge)) composite.fillRect(QRectF(0, 0, bounds.width(), 1), line);
      if (m_edges.testFlag(Qt::RightEdge)) composite.fillRect(QRectF(bounds.width() - 1, 0, 1, bounds.height()), line);
      if (m_edges.testFlag(Qt::BottomEdge)) composite.fillRect(QRectF(0, bounds.height() - 1, bounds.width(), 1), line);
    }
    composite.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    composite.drawImage(QPointF(), m_mask);
  }
  painter->drawImage(offset, content);
}
}  // namespace uwf::ui
