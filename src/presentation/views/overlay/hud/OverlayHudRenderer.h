// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QColor>
#include <QRectF>
#include <QString>

#include "domain/uwf/model/UwfModel.h"
#include "presentation/views/overlay/hud/OverlayHudPalette.h"

class QPainter;

namespace uwf::ui {

[[nodiscard]] QString overlayUsageText(const core::OverlayRuntime& runtime);

// Shared HUD painting; callers supply geometry, palette and usage availability.
void paintOverlayHud(QPainter& painter, const QRectF& bounds, qreal radius, const QColor& surfaceColor, const OverlayHudPalette& colors,
                     const core::OverlayRuntime& runtime, bool showUsage, qreal wavePhase);

}  // namespace uwf::ui
