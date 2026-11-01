// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

// Current is the loaded session; Next takes effect after restart. *Mb fields use UWF MB.

#include <cstddef>
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "domain/uwf/policies/VolumeAdmissionPolicy.h"

namespace uwf::core {

enum class OverlayType : int {
  Unknown = -1,  // Unavailable or unrecognized observation.
  RAM = 0,
  Disk = 1,
};
[[nodiscard]] constexpr OverlayType overlayTypeFromValue(const std::int64_t value) {
  return value == static_cast<std::int64_t>(OverlayType::RAM)    ? OverlayType::RAM
         : value == static_cast<std::int64_t>(OverlayType::Disk) ? OverlayType::Disk
                                                                 : OverlayType::Unknown;
}
[[nodiscard]] constexpr OverlayType overlayTypeFromValue(const std::optional<std::int64_t> value) {
  return value ? overlayTypeFromValue(*value) : OverlayType::Unknown;
}

struct FilterState {
  bool enabled = false;
};

// Type and capacity are session-scoped; thresholds are global runtime settings.
struct OverlayConfig {
  OverlayType type = OverlayType::Unknown;
  uint32_t maximumSizeMb = 0;
  uint32_t warningThresholdMb = 0;
  uint32_t criticalThresholdMb = 0;
};

// nullopt preserves the observed field. Type/capacity edits require Current UWF
// disabled; threshold edits do not.
struct OverlayConfigDelta {
  std::optional<OverlayType> type{};
  std::optional<uint32_t> maximumSizeMb{};
  std::optional<uint32_t> warningThresholdMb{};
  std::optional<uint32_t> criticalThresholdMb{};

  [[nodiscard]] bool empty() const { return !type && !maximumSizeMb && !warningThresholdMb && !criticalThresholdMb; }
  [[nodiscard]] bool touchesOverlayConfig() const { return type.has_value() || maximumSizeMb.has_value(); }
};

// Read-only usage of the running overlay, separate from its configured capacity.
struct OverlayRuntime {
  uint32_t availableSpaceMb = 0;
  uint32_t currentConsumptionMb = 0;
  std::optional<std::uint32_t> romDisposition{};
  std::optional<bool> fullCommit{};
  std::optional<bool> hormEnabled{};
  std::optional<std::uint32_t> currentOverlayFlags{};
  std::optional<bool> systemVolumeProtected{};
};

enum class OverlayUsageSeverity { Normal, Warning, Critical };

[[nodiscard]] constexpr uint64_t overlayTotalMb(const OverlayRuntime& runtime) {
  return static_cast<uint64_t>(runtime.currentConsumptionMb) + runtime.availableSpaceMb;
}
[[nodiscard]] constexpr OverlayUsageSeverity overlayUsageSeverity(uint32_t usedMb, uint32_t warningMb, uint32_t criticalMb) {
  return criticalMb && usedMb >= criticalMb ? OverlayUsageSeverity::Critical
         : warningMb && usedMb >= warningMb ? OverlayUsageSeverity::Warning
                                            : OverlayUsageSeverity::Normal;
}

enum class VolumeBinding { DriveLetter, VolumeId };

struct VolumeRecord {
  std::string volumeName;   // Volume{GUID}, without the device-path prefix.
  std::string driveLetter;  // Uppercase drive letter plus colon, or empty.
  bool isProtected = false;
  VolumeBinding binding = VolumeBinding::DriveLetter;
  // Actual mount observed with this snapshot; known empty means unmounted.
  std::optional<std::string> physicalDriveLetter{};
};

struct SessionSnapshot {
  FilterState filter;
  OverlayConfig overlay;
  std::vector<VolumeRecord> volumes;
  // Original provider patterns, keyed by VolumeRecord::volumeName. An apparent
  // drive/GUID prefix inside a pattern is literal, not a separate volume address.
  std::map<std::string, std::vector<std::string>> fileExclusions;
  std::vector<std::string> registryExclusions;                     // Global across volumes.
  bool persistDomainSecretKey = false;
  bool persistTSCAL = false;
};

struct UwfSnapshot {
  SessionSnapshot current;
  SessionSnapshot next;
  OverlayRuntime runtime;
  bool uwfAvailable = false;  // Required UWF observations completed successfully.
  bool elevated = false;
  std::string unavailableReason;  // UTF-8 diagnostic when UWF is unavailable.
};

// Inventory classification; new registration also requires PhysicalVolumeFacts.
// Failed physical discovery does not remove access to existing registrations.
enum class VolumeSupport : int {
  Supported,  // NTFS/FAT/FAT32 support file exclusions and commits.
  NotFixedLocalDisk,
  ExceedsMaxSize,     // Exceeds kMaxProtectedVolumeBytes.
  FileSystemLimited,  // Protection only; no file exclusions or commits.
};

[[nodiscard]] constexpr bool supportsVolumeProtection(const VolumeSupport support) noexcept {
  return support == VolumeSupport::Supported || support == VolumeSupport::FileSystemLimited;
}

[[nodiscard]] constexpr bool supportsFileOverlayOperations(const VolumeSupport support) noexcept { return support == VolumeSupport::Supported; }

struct VolumeInfo {
  std::string driveLetter;  // Uppercase drive letter plus colon, or empty.
  std::string volumeName;
  std::string fileSystem;
  std::string label;
  uint64_t totalBytes = 0;
  uint64_t freeBytes = 0;
  VolumeSupport support = VolumeSupport::Supported;
  PhysicalVolumeFacts physical{};
};

[[nodiscard]] inline bool canRegisterVolume(const VolumeInfo& volume) {
  return supportsVolumeProtection(volume.support) && !volume.volumeName.empty() &&
         physicalVolumeAdmission(volume.physical) == PhysicalVolumeAdmission::Eligible;
}

void sortExclusions(std::vector<std::string>& items);
void sortSnapshot(UwfSnapshot& snapshot);

// Explicit unapplied intents; nullopt leaves a scalar unchanged. Bulk-operation
// exceptions may intentionally equal the original baseline.
struct PendingChanges {
  std::optional<bool> setFilterEnabled;
  OverlayConfigDelta setOverlay;
  std::map<std::string, bool> volumeProtect;                             // Key: drive letter or canonical volume GUID.
  std::map<std::string, VolumeBinding> volumeBindings;                   // Key: drive letter or canonical volume GUID.
  std::map<std::string, std::vector<std::string>> addFileExclusions;     // Key: drive letter or canonical volume GUID.
  std::map<std::string, std::vector<std::string>> removeFileExclusions;  // Key: drive letter or canonical volume GUID.
  std::vector<std::string> addRegistryExclusions;
  std::vector<std::string> removeRegistryExclusions;
  std::optional<bool> setPersistDomainSecretKey;
  std::optional<bool> setPersistTSCAL;

  [[nodiscard]] bool empty() const;
  // Count scalar edits and individual exclusion additions/removals.
  [[nodiscard]] std::size_t count() const;
  void clear();
};

}  // namespace uwf::core
