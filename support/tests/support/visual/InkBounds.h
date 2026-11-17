// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QColor>
#include <QHash>
#include <QImage>
#include <QRect>
#include <QVector>
#include <algorithm>

namespace uwf::test {
namespace ink_bounds {
template <class IsInk>
QVector<QRect> horizontalRuns(const QImage& pixels, const QPoint& origin, const int minimumGap, IsInk isInk) {
  struct Column {
    int top = -1;
    int bottom = -1;
  };
  QVector<Column> columns(pixels.width());
  // Scan contiguous rows once; each column only needs its first and last ink pixel.
  for (int y = 0; y < pixels.height(); ++y) {
    const auto* row = reinterpret_cast<const QRgb*>(pixels.constScanLine(y));
    for (int x = 0; x < pixels.width(); ++x) {
      if (!isInk(row[x])) continue;
      auto& column = columns[x];
      if (column.top < 0) column.top = y;
      column.bottom = y;
    }
  }
  QVector<QRect> runs;
  for (int x = 0; x < columns.size(); ++x) {
    const auto& ink = columns[x];
    if (ink.top < 0) continue;
    const QRect column(origin.x() + x, origin.y() + ink.top, 1, ink.bottom - ink.top + 1);
    if (runs.isEmpty() || column.left() - runs.back().right() - 1 >= minimumGap)
      runs.append(column);
    else
      runs.back() = runs.back().united(column);
  }
  return runs;
}
}  // namespace ink_bounds

inline QVector<QRect> horizontalInkRuns(const QImage& image, const QColor& foreground, const QColor& background, const QRect& area, const int minimumGap) {
  const QRect scan = area.intersected(image.rect());
  const double red = foreground.red() - background.red();
  const double green = foreground.green() - background.green();
  const double blue = foreground.blue() - background.blue();
  const double contrast = red * red + green * green + blue * blue;
  if (scan.isEmpty() || contrast == 0) return {};
  // Convert only the sampled region. ARGB32 exposes the same unpremultiplied
  // channels as pixelColor(), including partially transparent source images.
  const auto pixels = image.copy(scan).convertToFormat(QImage::Format_ARGB32);
  return ink_bounds::horizontalRuns(pixels, scan.topLeft(), minimumGap, [&](const QRgb pixel) {
    const double coverage =
        ((qRed(pixel) - background.red()) * red + (qGreen(pixel) - background.green()) * green + (qBlue(pixel) - background.blue()) * blue) / contrast;
    return coverage >= .12;
  });
}

// Infer the composited background and retain edges with at least 12% of the
// strongest RGB contrast. Used when a production control has several ink colors.
inline QVector<QRect> contrastingInkRuns(const QImage& image, const QRect& area, const int minimumGap) {
  const auto scan = area.intersected(image.rect());
  if (scan.isEmpty()) return {};
  auto pixels = image.copy(scan);
  if (pixels.format() != QImage::Format_RGB32 && pixels.format() != QImage::Format_ARGB32 && pixels.format() != QImage::Format_ARGB32_Premultiplied)
    pixels = pixels.convertToFormat(QImage::Format_ARGB32);
  QHash<QRgb, int> colors;
  QRgb background = 0;
  int maximumCount = 0;
  for (int y = 0; y < pixels.height(); ++y) {
    const auto* row = reinterpret_cast<const QRgb*>(pixels.constScanLine(y));
    for (int x = 0; x < pixels.width(); ++x) {
      const int count = ++colors[row[x]];
      if (count > maximumCount) {
        maximumCount = count;
        background = row[x];
      }
    }
  }
  const auto contrast = [background](const QRgb pixel) {
    const int red = qRed(pixel) - qRed(background), green = qGreen(pixel) - qGreen(background), blue = qBlue(pixel) - qBlue(background);
    return red * red + green * green + blue * blue;
  };
  int maximumContrast = 0;
  for (int y = 0; y < pixels.height(); ++y) {
    const auto* row = reinterpret_cast<const QRgb*>(pixels.constScanLine(y));
    for (int x = 0; x < pixels.width(); ++x) maximumContrast = std::max(maximumContrast, contrast(row[x]));
  }
  if (maximumContrast < 64) return {};
  return ink_bounds::horizontalRuns(pixels, scan.topLeft(), minimumGap, [&](const QRgb pixel) { return contrast(pixel) >= maximumContrast * .12 * .12; });
}
}  // namespace uwf::test
