// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <optional>
#include <string>

#include "application/configuration/contracts/ConfigurationChanges.h"
#include "domain/uwf/model/UwfModel.h"

namespace uwf::app {

struct GlobalConfigurationDraft {
  core::FilterState baselineFilter;
  core::OverlayConfig baselineOverlay;
  bool filterEnabled = false;
  core::OverlayConfig overlay;

  [[nodiscard]] std::optional<bool> pendingFilterEnabled() const;
  [[nodiscard]] core::OverlayConfigDelta pendingOverlay() const;
};

struct VolumeConfigurationDraft {
  bool baselineProtected = false;
  core::VolumeBinding baselineBinding = core::VolumeBinding::DriveLetter;
  bool protectedVolume = false;
  core::VolumeBinding binding = core::VolumeBinding::DriveLetter;
  bool supported = true;
  // An observed physical or configured identity, not a UWF registration flag.
  bool hasTarget = false;
  bool hasConfiguration = false;
  // The complete-volume command establishes the inherited target. A later
  // single-volume edit is compared with this target, including baseline no-ops.
  std::optional<bool> bulkProtection;
  // Existing configuration GUID, or the original selector for a pending creation.
  std::string protectionTarget;

  [[nodiscard]] bool hasPlannedConfiguration() const { return hasConfiguration || protectedVolume || bulkProtection == true; }
  [[nodiscard]] std::optional<bool> pendingProtection() const;
  [[nodiscard]] core::VolumeBinding creationBinding() const;
  [[nodiscard]] std::optional<core::VolumeBinding> pendingBinding() const;
};

struct OverlaySizeBounds {
  uint32_t minimum;
  uint32_t maximum;
};
[[nodiscard]] OverlaySizeBounds overlaySizeBounds(core::OverlayType type, uint32_t totalRamMb, uint32_t editorMaximum);
[[nodiscard]] uint32_t strictThresholdUpper(uint32_t parentValue);

}  // namespace uwf::app
