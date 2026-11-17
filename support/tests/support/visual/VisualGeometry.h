// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QColor>
#include <QImage>
#include <QRect>
#include <QRectF>
#include <QtMath>
#include <cmath>
#include <limits>
#include <utility>

#include "support/visual/InkBounds.h"

namespace uwf::test {
// Alignment allows one tenth of the reference glyph or control height.
inline constexpr qreal kVisualAlignmentRatio = .10;
// Channel differences are measured on the normalized [0, 1] color scale.
inline constexpr qreal kVisualColorRatio = .05;
// A semantic background must dominate at least four fifths of its sample area.
inline constexpr qreal kVisualMinimumCoverage = .80;
// Sample the interior rather than rounded edges and their antialiasing.
inline constexpr qreal kVisualRegionInset = .15;

inline QRectF logicalBounds(const QRect& raster, const qreal dpr) {
  if (raster.isEmpty() || !(dpr > 0)) return {};
  return {raster.x() / dpr, raster.y() / dpr, raster.width() / dpr, raster.height() / dpr};
}
inline QRect pixelCenterRegion(const QRectF& region, const qreal dpr) {
  if (region.isEmpty() || !(dpr > 0)) return {};
  // Outward rounding can import a neighboring border at fractional DPI.
  return {QPoint(qCeil(region.left() * dpr - .5), qCeil(region.top() * dpr - .5)),
          QPoint(qCeil(region.right() * dpr - .5) - 1, qCeil(region.bottom() * dpr - .5) - 1)};
}
inline bool centersAligned(const QRectF& actual, const QRectF& reference, const Qt::Orientation orientation = Qt::Vertical) {
  if (actual.isEmpty() || reference.isEmpty()) return false;
  const qreal difference = orientation == Qt::Vertical ? actual.center().y() - reference.center().y() : actual.center().x() - reference.center().x();
  return std::abs(difference) / reference.height() <= kVisualAlignmentRatio;
}
inline bool bottomsAligned(const QRectF& actual, const QRectF& reference) {
  return !actual.isEmpty() && !reference.isEmpty() && std::abs(actual.bottom() - reference.bottom()) / reference.height() <= kVisualAlignmentRatio;
}
inline bool similarSize(const QRectF& actual, const QRectF& reference) {
  return !actual.isEmpty() && !reference.isEmpty() && std::abs(actual.width() - reference.width()) / reference.width() <= kVisualAlignmentRatio &&
         std::abs(actual.height() - reference.height()) / reference.height() <= kVisualAlignmentRatio;
}
inline QRectF interiorRegion(const QRectF& bounds) {
  return bounds.adjusted(bounds.width() * kVisualRegionInset, bounds.height() * kVisualRegionInset, -bounds.width() * kVisualRegionInset,
                         -bounds.height() * kVisualRegionInset);
}
inline qreal colorCoverage(const QImage& image, const QColor& expected, const QRectF& logicalRegion) {
  const qreal dpr = image.devicePixelRatio();
  if (image.isNull() || logicalRegion.isEmpty() || !(dpr > 0)) return std::numeric_limits<qreal>::quiet_NaN();
  const QRect raster = QRectF(logicalRegion.x() * dpr, logicalRegion.y() * dpr, logicalRegion.width() * dpr, logicalRegion.height() * dpr)
                           .toAlignedRect()
                           .intersected(image.rect());
  if (raster.isEmpty()) return std::numeric_limits<qreal>::quiet_NaN();
  qreal matching = 0, sampled = 0;
  for (int y = raster.top(); y <= raster.bottom(); ++y)
    for (int x = raster.left(); x <= raster.right(); ++x) {
      if (!logicalRegion.contains(QPointF((x + .5) / dpr, (y + .5) / dpr))) continue;
      const auto pixel = image.pixelColor(x, y);
      if (std::abs(pixel.redF() - expected.redF()) <= kVisualColorRatio && std::abs(pixel.greenF() - expected.greenF()) <= kVisualColorRatio &&
          std::abs(pixel.blueF() - expected.blueF()) <= kVisualColorRatio && std::abs(pixel.alphaF() - expected.alphaF()) <= kVisualColorRatio)
        ++matching;
      ++sampled;
    }
  return sampled > 0 ? matching / sampled : std::numeric_limits<qreal>::quiet_NaN();
}
inline QColor regionColor(const QImage& image, const QRectF& logicalRegion) {
  const qreal dpr = image.devicePixelRatio();
  if (image.isNull() || logicalRegion.isEmpty() || !(dpr > 0)) return {};
  const QRect raster = QRectF(logicalRegion.x() * dpr, logicalRegion.y() * dpr, logicalRegion.width() * dpr, logicalRegion.height() * dpr)
                           .toAlignedRect()
                           .intersected(image.rect());
  if (raster.isEmpty()) return {};
  qreal red = 0, green = 0, blue = 0, alpha = 0, sampled = 0;
  for (int y = raster.top(); y <= raster.bottom(); ++y)
    for (int x = raster.left(); x <= raster.right(); ++x) {
      if (!logicalRegion.contains(QPointF((x + .5) / dpr, (y + .5) / dpr))) continue;
      const auto pixel = image.pixelColor(x, y);
      red += pixel.redF();
      green += pixel.greenF();
      blue += pixel.blueF();
      alpha += pixel.alphaF();
      ++sampled;
    }
  if (sampled == 0) return {};
  return QColor::fromRgbF(static_cast<float>(red / sampled), static_cast<float>(green / sampled), static_cast<float>(blue / sampled),
                          static_cast<float>(alpha / sampled));
}
inline QRectF visibleInkBounds(const QImage& image, const QColor& ink, const QColor& background, const QRectF& logicalRegion) {
  const qreal dpr = image.devicePixelRatio();
  if (image.isNull() || logicalRegion.isEmpty() || !(dpr > 0)) return {};
  const auto raster = pixelCenterRegion(logicalRegion, dpr);
  QRect bounds;
  for (auto runs = horizontalInkRuns(image, ink, background, raster, 1); const auto& run : std::as_const(runs)) bounds = bounds.united(run);
  return logicalBounds(bounds, dpr);
}
inline qreal changedCoverage(const QImage& before, const QImage& after, const QRectF& logicalRegion) {
  if (before.isNull() || after.isNull() || logicalRegion.isEmpty()) return std::numeric_limits<qreal>::quiet_NaN();
  const qreal dpr = before.devicePixelRatio();
  const QRect raster = QRectF(logicalRegion.x() * dpr, logicalRegion.y() * dpr, logicalRegion.width() * dpr, logicalRegion.height() * dpr)
                           .toAlignedRect()
                           .intersected(before.rect());
  if (raster.isEmpty()) return std::numeric_limits<qreal>::quiet_NaN();
  qreal changed = 0, sampled = 0;
  for (int y = raster.top(); y <= raster.bottom(); ++y)
    for (int x = raster.left(); x <= raster.right(); ++x) {
      if (!logicalRegion.contains(QPointF((x + .5) / dpr, (y + .5) / dpr))) continue;
      const QPoint point(qFloor((x + .5) / dpr * after.devicePixelRatio()), qFloor((y + .5) / dpr * after.devicePixelRatio()));
      if (!after.rect().contains(point)) continue;
      const auto first = before.pixelColor(x, y);
      const auto second = after.pixelColor(point);
      if (std::abs(first.redF() - second.redF()) > kVisualColorRatio || std::abs(first.greenF() - second.greenF()) > kVisualColorRatio ||
          std::abs(first.blueF() - second.blueF()) > kVisualColorRatio || std::abs(first.alphaF() - second.alphaF()) > kVisualColorRatio)
        ++changed;
      ++sampled;
    }
  return sampled > 0 ? changed / sampled : std::numeric_limits<qreal>::quiet_NaN();
}
}  // namespace uwf::test
