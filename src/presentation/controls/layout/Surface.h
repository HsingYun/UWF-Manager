// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <QWidget>

#include "presentation/design/theme/DesignSystem.h"
namespace uwf::ui {
class SurfaceClip;
class Surface : public QWidget {
 public:
  explicit Surface(design::Color fill = design::Color::Surface, QWidget* parent = nullptr);
  void setFill(design::Color fill);
  void setFillEnabled(bool enabled);
  void setBorder(Qt::Edges edges, int radius = 0);
  void setBorderColor(design::Color color, qreal opacity = 1.0);
  void setClipContent(bool clip);
  // Only translucent when the top-level native material is available/active.
  // A solid fallback uses the original fill automatically.
  void setBackdropOpacity(int percent);

 protected:
  void paintEvent(QPaintEvent*) override;

 private:
  design::Color m_fill;
  bool m_fillEnabled = true;
  Qt::Edges m_edges;
  design::Color m_borderColor = design::Color::Line;
  qreal m_borderOpacity = 1.0;
  int m_radius = 0;
  SurfaceClip* m_clip = nullptr;
  int m_backdropOpacity = 100;
};
}  // namespace uwf::ui
