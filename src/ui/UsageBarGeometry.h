/*
 * Copyright (c) 2026 HsingYun (iakext@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

#include <QPainterPath>
#include <QtGlobal>
#include <algorithm>
#include <cmath>
#include <numbers>

namespace uwf::ui {

// 非零占用至少显示整条的 2%，避免真实宽度不足一个像素时完全不可见。
// 若存在阈值 / 上限，则视觉兜底不能超过其中任意一项的 50%，避免让很小的
// 实际占用看起来已经接近某条配置线。传 0 表示对应限制不存在。
[[nodiscard]] inline qreal visibleUsedWidth(const qreal naturalWidth, const qreal fullWidth, const qreal warningWidth = 0, const qreal criticalWidth = 0,
                                            const qreal maximumWidth = 0) {
  if (naturalWidth <= 0 || fullWidth <= 0) return 0;

  qreal hintWidth = fullWidth * 0.02;
  if (warningWidth > 0) hintWidth = std::min(hintWidth, warningWidth * 0.5);
  if (criticalWidth > 0) hintWidth = std::min(hintWidth, criticalWidth * 0.5);
  if (maximumWidth > 0) hintWidth = std::min(hintWidth, maximumWidth * 0.5);
  return std::clamp(std::max(naturalWidth, hintWidth), qreal{0}, fullWidth);
}

// 从 bounds 左侧到 usedWidth 生成闭合填充面；右缘用只向外扩展的正弦波装饰，
// 不会侵蚀真实进度或最小可见宽度。0% / 100% 由调用方单独处理。
[[nodiscard]] inline QPainterPath waveProgressPath(const QRectF& bounds, const qreal usedWidth, const qreal phase, const qreal excursion = 1.5) {
  const qreal waveX = bounds.left() + std::clamp(usedWidth, qreal{0}, bounds.width());
  const qreal waveCycle = 2.0 * std::numbers::pi_v<qreal>;
  const auto waveOffset = [excursion](const qreal p) { return excursion * (std::sin(p) + 1.0) * 0.5; };

  constexpr qreal kWaveStepY = 2.0;
  QPainterPath path;
  path.moveTo(bounds.left(), bounds.top());
  path.lineTo(waveX + waveOffset(phase), bounds.top());
  for (qreal y = bounds.top() + kWaveStepY; y < bounds.bottom(); y += kWaveStepY) {
    const qreal verticalPhase = (y - bounds.top()) / bounds.height() * waveCycle;
    path.lineTo(waveX + waveOffset(phase + verticalPhase), y);
  }
  path.lineTo(waveX + waveOffset(phase + waveCycle), bounds.bottom());
  path.lineTo(bounds.left(), bounds.bottom());
  path.closeSubpath();
  return path;
}

}  // namespace uwf::ui
