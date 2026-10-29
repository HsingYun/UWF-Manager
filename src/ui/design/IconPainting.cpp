// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "IconPainting.h"

#include <QPainter>
#include <QPainterPath>
#include <algorithm>
namespace uwf::ui {
namespace {
qreal centerOffset(const QRectF& bounds, const QRectF& ink) { return bounds.center().y() - ink.center().y(); }
}  // namespace
void paintIcon(QPainter& painter, const QIcon& icon, const QRectF& bounds, const QColor& ink) {
  if (icon.isNull() || bounds.isEmpty()) return;
  auto bitmap = icon.pixmap(bounds.size().toSize(), painter.device()->devicePixelRatioF(), QIcon::Normal, QIcon::Off);
  if (ink.isValid()) {
    QPainter tint(&bitmap);
    tint.setCompositionMode(QPainter::CompositionMode_SourceIn);
    tint.fillRect(bitmap.rect(), ink);
  }
  const auto pixels = bitmap.toImage().convertToFormat(QImage::Format_ARGB32_Premultiplied);
  int first = pixels.height();
  int last = -1;
  for (int y = 0; y < pixels.height(); ++y) {
    const auto* row = reinterpret_cast<const QRgb*>(pixels.constScanLine(y));
    for (int x = 0; x < pixels.width(); ++x) {
      if (qAlpha(row[x]) == 0) continue;
      first = std::min(first, y);
      last = std::max(last, y);
      break;
    }
  }
  QRectF target = bounds;
  if (last >= first) {
    const qreal scale = bounds.height() / pixels.height();
    target.translate(0, centerOffset(bounds, QRectF(bounds.left(), bounds.top() + first * scale, bounds.width(), (last - first + 1) * scale)));
  }
  painter.save();
  painter.setRenderHint(QPainter::SmoothPixmapTransform);
  painter.drawPixmap(target, bitmap, QRectF(bitmap.rect()));
  painter.restore();
}
void paintIconPath(QPainter& painter, const QPainterPath& path, const QRectF& bounds, const QColor& ink, const qreal strokeWidth) {
  if (path.isEmpty() || bounds.isEmpty()) return;
  painter.save();
  painter.setRenderHint(QPainter::Antialiasing);
  painter.setPen(QPen(ink, strokeWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
  painter.setBrush(Qt::NoBrush);
  painter.translate(0, centerOffset(bounds, path.boundingRect()));
  painter.drawPath(path);
  painter.restore();
}
}  // namespace uwf::ui
