// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "ConfigurationDraft.h"

#include <algorithm>

#include "core/UwfConstraints.h"

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
  if (!supported || !hasVolume) return std::nullopt;
  return protectedVolume == baselineProtected ? std::nullopt : std::optional<bool>(protectedVolume);
}

std::optional<bool> VolumeConfigurationDraft::pendingBinding() const {
  if (!supported || !hasVolume) return std::nullopt;
  return bindByVolumeName == baselineBindByVolumeName ? std::nullopt : std::optional<bool>(bindByVolumeName);
}

OverlaySizeBounds overlaySizeBounds(const core::OverlayType type, const uint32_t totalRamMb, const int editorMaximum) {
  const bool isRam = type == core::OverlayType::RAM;
  const int ceiling = (isRam && totalRamMb > 0) ? static_cast<int>(std::min<uint32_t>(totalRamMb, static_cast<uint32_t>(editorMaximum))) : editorMaximum;
  return {isRam ? 0 : static_cast<int>(core::kDiskOverlayMinSizeMb), ceiling};
}

int strictThresholdUpper(const int parentValue) { return parentValue <= 1 ? 0 : parentValue - 1; }

core::PendingChanges ConfigurationDraft::pendingChanges() const {
  core::PendingChanges changes;
  changes.setFilterEnabled = filterEnabled;
  changes.setOverlay = overlay;
  changes.addRegistryExclusions = registry.registryAdded;
  changes.removeRegistryExclusions = registry.registryRemoved;
  changes.setPersistDomainSecretKey = registry.persistDomainSecretKey;
  changes.setPersistTSCAL = registry.persistTSCAL;
  for (const auto& volume : volumes) {
    if (!volume.supported) continue;
    const auto& dl = volume.driveLetter;
    if (volume.protectedVolume) changes.volumeProtect[dl] = *volume.protectedVolume;
    if (volume.bindByVolumeName) changes.volumeBindByVolumeName[dl] = *volume.bindByVolumeName;
    if (!volume.fileAdded.empty()) {
      auto& added = changes.addFileExclusions[dl];
      added.insert(added.end(), volume.fileAdded.begin(), volume.fileAdded.end());
    }
    if (!volume.fileRemoved.empty()) {
      auto& removed = changes.removeFileExclusions[dl];
      removed.insert(removed.end(), volume.fileRemoved.begin(), volume.fileRemoved.end());
    }
  }
  return changes;
}

}  // namespace uwf::app
