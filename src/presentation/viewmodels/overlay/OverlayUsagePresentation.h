// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QDateTime>

#include "application/overlay/contracts/OverlayUsage.h"
#include "presentation/viewmodels/common/PresentationText.h"

namespace uwf::presentation {
[[nodiscard]] inline QString overlayUsageFreshnessText(const app::OverlayUsageState& state) {
  const auto& observation = app::overlayUsageObservation(state);
  if (!observation.lastSuccess)
    return observation.error ? PresentationText::tr("Overlay usage refresh failed.\n%1").arg(QString::fromStdString(*observation.error)) : QString{};
  const auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(observation.lastSuccess->time_since_epoch()).count();
  const auto time = QDateTime::fromMSecsSinceEpoch(milliseconds).toLocalTime().toString(Qt::ISODate);
  if (!observation.error) return PresentationText::tr("Last successful overlay usage observation: %1").arg(time);
  return PresentationText::tr("Overlay usage refresh failed. Showing the last successful observation from %1.\n%2")
      .arg(time, QString::fromStdString(*observation.error));
}
}  // namespace uwf::presentation
