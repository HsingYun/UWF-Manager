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

#include <QColor>
#include <QRectF>
#include <QString>
#include <cstdint>

#include "../core/UwfModel.h"
#include "OverlayHudPalette.h"

class QPainter;

namespace uwf::ui {

[[nodiscard]] uint64_t overlayTotalMb(const core::OverlayRuntime& runtime);
[[nodiscard]] QString overlayUsageText(const core::OverlayRuntime& runtime);

// 浮窗与任务栏共用的 HUD 表面绘制：底色、进度、水波、圆角裁剪和边框均在
// 此处完成。调用方只决定几何、表面颜色、圆角和是否展示有效用量。
void paintOverlayHud(QPainter& painter, const QRectF& bounds, qreal radius, const QColor& surfaceColor, const OverlayHudPalette& colors,
                     const core::OverlayRuntime& runtime, bool showUsage, qreal wavePhase);

}  // namespace uwf::ui
