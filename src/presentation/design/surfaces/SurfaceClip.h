// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QGraphicsEffect>
#include <QImage>

#include "design/DesignTokens.h"

namespace uwf::ui {
class SurfaceClip final : public QGraphicsEffect {
 public:
  explicit SurfaceClip(QObject* parent);
  void setFrame(Qt::Edges edges, int radius);
  void setBorderColor(design::Color color, qreal opacity = 1.0);

 protected:
  void draw(QPainter* painter) override;

 private:
  Qt::Edges m_edges;
  int m_radius = 0;
  design::Color m_borderColor = design::Color::Line;
  qreal m_borderOpacity = 1.0;
  QImage m_mask;
};
}  // namespace uwf::ui
