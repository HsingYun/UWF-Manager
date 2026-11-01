// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QPainterPath>
#include <QtGlobal>
#include <algorithm>
#include <cmath>
#include <numbers>

namespace uwf::ui {

// Nonzero usage has a 2% visibility floor, capped at half each positive marker width.
// Zero marker widths impose no cap; measured usage is never reduced by the floor.
[[nodiscard]] inline qreal visibleUsedWidth(const qreal naturalWidth, const qreal fullWidth, const qreal warningWidth = 0, const qreal criticalWidth = 0,
                                            const qreal maximumWidth = 0) {
  if (naturalWidth <= 0 || fullWidth <= 0) return 0;

  qreal hintWidth = fullWidth * 0.02;
  if (warningWidth > 0) hintWidth = std::min(hintWidth, warningWidth * 0.5);
  if (criticalWidth > 0) hintWidth = std::min(hintWidth, criticalWidth * 0.5);
  if (maximumWidth > 0) hintWidth = std::min(hintWidth, maximumWidth * 0.5);
  return std::clamp(std::max(naturalWidth, hintWidth), qreal{0}, fullWidth);
}

// The wave extends outward from usedWidth without reducing the measured fill.
// Callers handle the empty and full states separately.
[[nodiscard]] inline QPainterPath waveProgressPath(const QRectF& bounds, const qreal usedWidth, const qreal phase, const qreal excursion = 1.5) {
  const qreal waveX = bounds.left() + std::clamp(usedWidth, qreal{0}, bounds.width());
  const qreal waveCycle = 2.0 * std::numbers::pi_v<qreal>;
  const auto waveOffset = [excursion](const qreal p) { return excursion * (std::sin(p) + 1.0) * 0.5; };

  constexpr int kWaveStepY = 2;
  QPainterPath path;
  path.moveTo(bounds.left(), bounds.top());
  path.lineTo(waveX + waveOffset(phase), bounds.top());
  for (int yOffset = kWaveStepY; static_cast<qreal>(yOffset) < bounds.height(); yOffset += kWaveStepY) {
    const qreal y = bounds.top() + static_cast<qreal>(yOffset);
    const qreal verticalPhase = (y - bounds.top()) / bounds.height() * waveCycle;
    path.lineTo(waveX + waveOffset(phase + verticalPhase), y);
  }
  path.lineTo(waveX + waveOffset(phase + waveCycle), bounds.bottom());
  path.lineTo(bounds.left(), bounds.bottom());
  path.closeSubpath();
  return path;
}

}  // namespace uwf::ui
