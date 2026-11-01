// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "presentation/design/painting/RasterInkBounds.h"

#include <QImage>
#include <algorithm>

namespace uwf::ui {
QRect rasterInkBounds(const QImage& image) {
  const auto pixels = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
  // Ignore faint antialiasing fringes when locating the visible stroke edges.
  constexpr qreal minimumCoverage = .12;
  int left = pixels.width(), right = -1, first = pixels.height(), last = -1;
  for (int y = 0; y < pixels.height(); ++y) {
    const auto* row = reinterpret_cast<const QRgb*>(pixels.constScanLine(y));
    for (int x = 0; x < pixels.width(); ++x) {
      if (qAlpha(row[x]) / 255.0 < minimumCoverage) continue;
      left = std::min(left, x);
      right = std::max(right, x);
      first = std::min(first, y);
      last = y;
    }
  }
  return right < left ? QRect{} : QRect(left, first, right - left + 1, last - first + 1);
}
}  // namespace uwf::ui
