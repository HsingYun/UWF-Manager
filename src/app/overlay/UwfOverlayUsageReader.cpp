// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "UwfOverlayUsageReader.h"
namespace uwf::app {
OverlayUsageState UwfOverlayUsageReader::read() const {
  auto& session = m_session();
  const provider::UwfFilter filterReader(session);
  const provider::UwfOverlay overlayReader(session);
  const provider::UwfOverlayConfig configReader(session);
  OverlayUsageState candidate{OverlayUsageDisabled{}};
  const provider::FilterRow filter = filterReader.read();
  if (filter.currentEnabled) {
    const auto overlay = overlayReader.read();
    const auto config = configReader.read(provider::Session::Current);
    core::OverlayRuntime runtime;
    runtime.currentConsumptionMb = overlay.overlayConsumption;
    runtime.availableSpaceMb = overlay.availableSpace;
    core::OverlayConfig presentationConfig;
    presentationConfig.maximumSizeMb = config.maximumSize;
    presentationConfig.type = config.type == provider::OverlayType::RAM ? core::OverlayType::RAM : core::OverlayType::Disk;
    presentationConfig.warningThresholdMb = overlay.warningOverlayThreshold;
    presentationConfig.criticalThresholdMb = overlay.criticalOverlayThreshold;
    candidate = OverlayUsageEnabled{runtime, presentationConfig};
  }
  return candidate;
}
}  // namespace uwf::app
