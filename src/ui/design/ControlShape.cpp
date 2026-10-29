// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "ControlShape.h"

#include <QPainter>
#include <QPainterPath>
#include <algorithm>
namespace uwf::ui {
void paintControlShape(QPainter& painter, const QRectF& rect, const qreal radius, const QColor& fill, const QColor& border, const QColor& bottomBorder) {
  if (rect.isEmpty()) return;
  painter.save();
  painter.setRenderHint(QPainter::Antialiasing);
  const qreal corner = std::clamp(radius, 0.0, std::min(rect.width(), rect.height()) / 2);
  QPainterPath outside;
  outside.addRoundedRect(rect, corner, corner);
  if (fill.isValid() && fill.alpha()) painter.fillPath(outside, fill);
  if ((border.isValid() && border.alpha()) || (bottomBorder.isValid() && bottomBorder.alpha())) {
    QPainterPath inside;
    const qreal innerCorner = std::max(0.0, corner - 1);
    inside.addRoundedRect(rect.adjusted(1, 1, -1, -1), innerCorner, innerCorner);
    const auto edge = outside.subtracted(inside);
    painter.fillPath(edge, border);
    if (bottomBorder.isValid()) {
      painter.setClipRect(QRectF(rect.left(), rect.bottom() - 1, rect.width(), 1));
      painter.fillPath(edge, bottomBorder);
    }
  }
  painter.restore();
}
}  // namespace uwf::ui
