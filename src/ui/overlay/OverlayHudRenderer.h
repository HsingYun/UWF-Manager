// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QColor>
#include <QRectF>
#include <QString>
#include <cstdint>

#include "OverlayHudPalette.h"
#include "core/UwfModel.h"

class QPainter;

namespace uwf::ui {

[[nodiscard]] QString overlayUsageText(const core::OverlayRuntime& runtime);

// 浮窗与任务栏共用的 HUD 表面绘制：底色、进度、水波、圆角裁剪和边框均在
// 此处完成。调用方只决定几何、表面颜色、圆角和是否展示有效用量。
void paintOverlayHud(QPainter& painter, const QRectF& bounds, qreal radius, const QColor& surfaceColor, const OverlayHudPalette& colors,
                     const core::OverlayRuntime& runtime, bool showUsage, qreal wavePhase);

}  // namespace uwf::ui
