// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "application/configuration/editing/ConfigurationDraft.h"

#include <algorithm>

#include "domain/uwf/contracts/Types.h"
#include "domain/uwf/policies/UwfConstraints.h"

namespace uwf::app {

std::optional<bool> GlobalConfigurationDraft::pendingFilterEnabled() const {
  return filterEnabled == baselineFilter.enabled ? std::nullopt : std::optional<bool>(filterEnabled);
}

core::OverlayConfigDelta GlobalConfigurationDraft::pendingOverlay() const {
  core::OverlayConfigDelta d;
  const auto& b = baselineOverlay;
  const auto t = overlay.type;
  if (t != b.type) d.type = t;
  if (overlay.maximumSizeMb != b.maximumSizeMb) d.maximumSizeMb = overlay.maximumSizeMb;
  if (overlay.warningThresholdMb != b.warningThresholdMb) d.warningThresholdMb = overlay.warningThresholdMb;
  if (overlay.criticalThresholdMb != b.criticalThresholdMb) d.criticalThresholdMb = overlay.criticalThresholdMb;
  return d;
}

std::optional<bool> VolumeConfigurationDraft::pendingProtection() const {
  if (!supported || !hasTarget) return std::nullopt;
  return protectedVolume == bulkProtection.value_or(baselineProtected) ? std::nullopt : std::optional<bool>(protectedVolume);
}

core::VolumeBinding VolumeConfigurationDraft::creationBinding() const {
  return compat::parseVolumeId(protectionTarget) ? core::VolumeBinding::VolumeId : core::VolumeBinding::DriveLetter;
}
std::optional<core::VolumeBinding> VolumeConfigurationDraft::pendingBinding() const {
  if (!supported || !hasTarget || !hasPlannedConfiguration()) return std::nullopt;
  const auto inherited = hasConfiguration ? baselineBinding : bulkProtection == true ? core::VolumeBinding::VolumeId : creationBinding();
  return binding == inherited ? std::nullopt : std::optional<core::VolumeBinding>(binding);
}

OverlaySizeBounds overlaySizeBounds(const core::OverlayType type, const uint32_t totalRamMb, const uint32_t editorMaximum) {
  const bool isRam = type == core::OverlayType::RAM;
  const auto ceiling = (isRam && totalRamMb > 0) ? std::min(totalRamMb, editorMaximum) : editorMaximum;
  return {core::minimumOverlaySizeMb(type), ceiling};
}

uint32_t strictThresholdUpper(const uint32_t parentValue) { return parentValue <= 1 ? 0 : parentValue - 1; }

}  // namespace uwf::app
