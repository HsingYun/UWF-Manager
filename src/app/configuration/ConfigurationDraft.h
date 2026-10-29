// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <optional>
#include <string>
#include <vector>

#include "core/UwfModel.h"

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
  bool baselineBindByVolumeName = false;
  bool protectedVolume = false;
  bool bindByVolumeName = false;
  bool supported = true;
  bool hasVolume = false;

  [[nodiscard]] std::optional<bool> pendingProtection() const;
  [[nodiscard]] std::optional<bool> pendingBinding() const;
};

struct VolumeConfigurationChanges {
  std::string driveLetter;
  bool supported = false;
  std::optional<bool> protectedVolume;
  std::optional<bool> bindByVolumeName;
  std::vector<std::string> fileAdded;
  std::vector<std::string> fileRemoved;
};

struct RegistryConfigurationChanges {
  std::vector<std::string> registryAdded;
  std::vector<std::string> registryRemoved;
  std::optional<bool> persistDomainSecretKey;
  std::optional<bool> persistTSCAL;
};

struct ConfigurationDraft {
  std::optional<bool> filterEnabled;
  core::OverlayConfigDelta overlay;
  RegistryConfigurationChanges registry;
  std::vector<VolumeConfigurationChanges> volumes;

  [[nodiscard]] core::PendingChanges pendingChanges() const;
};

struct OverlaySizeBounds {
  int minimum;
  int maximum;
};
[[nodiscard]] OverlaySizeBounds overlaySizeBounds(core::OverlayType type, uint32_t totalRamMb, int editorMaximum);
[[nodiscard]] int strictThresholdUpper(int parentValue);

}  // namespace uwf::app
