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
#include "OverlayHudRenderer.h"

#include <QPainter>
#include <QPainterPath>
#include <format>

#include "UsageBarGeometry.h"

namespace uwf::ui {

uint64_t overlayTotalMb(const core::OverlayRuntime& runtime) { return static_cast<uint64_t>(runtime.currentConsumptionMb) + runtime.availableSpaceMb; }

QString overlayUsageText(const core::OverlayRuntime& runtime) {
  const uint64_t totalMb = overlayTotalMb(runtime);
  const double percent = totalMb == 0 ? 0.0 : static_cast<double>(runtime.currentConsumptionMb) * 100.0 / static_cast<double>(totalMb);
  return QString::fromStdString(std::format("{:.1f}% {}/{} MB", percent, runtime.currentConsumptionMb, totalMb));
}

void paintOverlayHud(QPainter& painter, const QRectF& bounds, const qreal radius, const QColor& surfaceColor, const OverlayHudPalette& colors,
                     const core::OverlayRuntime& runtime, const bool showUsage, const qreal wavePhase) {
  QPainterPath surfacePath;
  surfacePath.addRoundedRect(bounds, radius, radius);
  painter.fillPath(surfacePath, surfaceColor);

  const uint64_t totalMb = overlayTotalMb(runtime);
  if (showUsage && totalMb > 0 && runtime.currentConsumptionMb > 0) {
    if (static_cast<uint64_t>(runtime.currentConsumptionMb) >= totalMb) {
      painter.fillPath(surfacePath, colors.progressFill);
    } else {
      const qreal ratio = static_cast<qreal>(runtime.currentConsumptionMb) / static_cast<qreal>(totalMb);
      const qreal usedWidth = visibleUsedWidth(bounds.width() * ratio, bounds.width());
      const QPainterPath progressPath = waveProgressPath(bounds, usedWidth, wavePhase);
      painter.save();
      painter.setClipPath(surfacePath);
      painter.fillPath(progressPath, colors.progressFill);
      painter.restore();
    }
  }

  painter.setPen(QPen(colors.border, 1.0));
  painter.drawPath(surfacePath);
}

}  // namespace uwf::ui
