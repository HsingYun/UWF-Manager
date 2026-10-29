// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QColor>
#include <QImage>
#include <QRect>
#include <QVector>

namespace uwf::test {
inline QVector<QRect> horizontalInkRuns(const QImage& image, const QColor& foreground, const QColor& background, const QRect& area,
                                       const int minimumGap) {
  const QRect scan = area.intersected(image.rect());
  const double red = foreground.red() - background.red();
  const double green = foreground.green() - background.green();
  const double blue = foreground.blue() - background.blue();
  const double contrast = red * red + green * green + blue * blue;
  QVector<QRect> runs;
  if (contrast == 0) return runs;
  for (int x = scan.left(); x <= scan.right(); ++x) {
    QRect column;
    for (int y = scan.top(); y <= scan.bottom(); ++y) {
      const QColor pixel = image.pixelColor(x, y);
      // Include antialiased edges, including thin glyphs with no opaque pixels.
      const double coverage =
          ((pixel.red() - background.red()) * red + (pixel.green() - background.green()) * green + (pixel.blue() - background.blue()) * blue) / contrast;
      if (coverage >= .12) column = column.united(QRect(x, y, 1, 1));
    }
    if (column.isEmpty()) continue;
    if (runs.isEmpty() || x - runs.back().right() - 1 >= minimumGap)
      runs.append(column);
    else
      runs.back() = runs.back().united(column);
  }
  return runs;
}
}  // namespace uwf::test
