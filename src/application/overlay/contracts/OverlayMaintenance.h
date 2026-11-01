// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QString>
#include <cstdint>
#include <functional>
#include <optional>
#include <stop_token>
#include <vector>

#include "application/operations/contracts/OperationError.h"
#include "application/overlay/contracts/MaintenanceValidation.h"
#include "application/overlay/contracts/OverlayPolicy.h"
#include "application/volumes/contracts/AllVolumeProtection.h"

namespace uwf::app {
// Explicit draft intents. Configuration phases order complete-volume protection
// before its per-volume exceptions. Storage participates in the overlay
// transition plan; remaining runtime maintenance follows configuration writes.
struct OverlayMaintenanceChanges {
  std::optional<QString> storageVolume{};
  std::optional<bool> hormEnabled{};
  bool fullCommit = false;
  std::optional<bool> allVolumesProtected{};
  [[nodiscard]] bool empty() const { return !storageVolume && !hormEnabled && !fullCommit && !allVolumesProtected; }
  [[nodiscard]] std::size_t count() const {
    return static_cast<std::size_t>(storageVolume.has_value()) + static_cast<std::size_t>(hormEnabled.has_value()) + static_cast<std::size_t>(fullCommit) +
           static_cast<std::size_t>(allVolumesProtected.has_value());
  }
  bool operator==(const OverlayMaintenanceChanges&) const = default;
};
enum class OverlayMaintenanceSupport { Unknown, Supported, Unsupported, Unavailable };
struct OverlayMaintenanceCapability {
  OverlayMaintenanceSupport support = OverlayMaintenanceSupport::Unknown;
  QString reason;
  [[nodiscard]] bool canAttempt() const { return support == OverlayMaintenanceSupport::Supported || support == OverlayMaintenanceSupport::Unknown; }
};
struct OverlaySwapVolume {
  QString id;
  QString driveLetter;
  QString label;
  std::uint64_t freeBytes = 0;
  bool eligible = true;
  std::optional<std::uint32_t> currentMarker;
  std::optional<std::uint32_t> nextMarker;
};
enum class OverlayStorageOrigin { Unknown, ConfiguredMarker, SystemDriveDefault, NotUsed };
enum class OverlayStorageIssue { None, ObservationUnavailable, OverlayTypeUnknown, MarkerUnavailable, MultipleHosts, SystemDriveUnavailable };
// A configuration-derived location, never proof that a physical swapfile was
// created, opened, or migrated. A candidate in the picker is not an observation.
struct OverlayStorageLocation {
  OverlayStorageOrigin origin = OverlayStorageOrigin::Unknown;
  std::optional<QString> volumeId;
  QString driveLetter;
  QString diagnostics;
  OverlayStorageIssue issue = OverlayStorageIssue::None;
};
struct OverlayHormBlockedVolume {
  QString volumeId;
  QString name;
  bool operator==(const OverlayHormBlockedVolume&) const = default;
};
struct OverlayHormVolumeProtection {
  bool allProtected = false;
  std::vector<OverlayHormBlockedVolume> blockedVolumes{};
  QString detailError{};
};
enum class HibernationFileType { Unknown, None, Reduced, Full };
struct OverlayHibernationCapabilities {
  bool systemS4 = false;
  bool hiberFilePresent = false;
  HibernationFileType fileType = HibernationFileType::Unknown;
  [[nodiscard]] bool available() const { return systemS4 && hiberFilePresent && fileType == HibernationFileType::Full; }
};
// Explicit read-only host facts. Missing functions leave the corresponding
// observation unknown; an application adapter never resolves Windows services.
struct OverlayMaintenanceReadOnlyPorts {
  std::function<OverlayHormVolumeProtection(std::stop_token)> hormVolumeProtection{};
  std::function<OverlayHibernationCapabilities(std::stop_token)> hibernationCapabilities{};
};
// Missing observations stay unknown. Host markers are not byte/MB sizes.
struct OverlayMaintenanceSnapshot {
  std::optional<bool> filterEnabled;
  std::optional<bool> systemVolumeProtected;
  std::optional<bool> overlayCommit;
  std::optional<bool> hormEnabled;
  std::optional<std::int64_t> currentOverlayType;
  std::optional<std::int64_t> nextOverlayType;
  std::optional<std::uint32_t> maximumOverlaySize;
  std::optional<OverlayFlags> currentFlags;
  std::optional<OverlayFlags> nextFlags;
  std::vector<OverlaySwapVolume> volumes;
  bool volumesReadable = false;
  OverlayMaintenanceCapability fullCommitCapability;
  OverlayMaintenanceCapability swapCapability;
  OverlayMaintenanceCapability hormCapability;
  // Public HORM prerequisite observations may not exist on a complete provider.
  std::optional<bool> hibernationAvailable;
  std::optional<bool> allVolumesProtected;
  QString readError;
  std::optional<std::uint32_t> romDisposition;
  QString hibernationError, allVolumesProtectedError;
  QString systemDrive;
  std::optional<QString> systemVolumeId;
  QString systemDriveError;
  OverlayStorageLocation currentStorage, nextStorage;
  std::vector<OverlayHormBlockedVolume> blockedVolumes{};
  std::optional<OverlayHibernationCapabilities> hibernationCapabilities{};
  std::optional<bool> allVolumesNextProtected;
  std::optional<bool> allVolumesNextUnprotected;
  OverlayMaintenanceCapability protectAllCapability;
  QString protectAllError;
  OverlayMaintenanceCapability unprotectAllCapability;
  QString unprotectAllError;
  // Complete registered Next identities from the independent batch reader.
  // The ordinary settings snapshot may filter stale drive-letter aliases.
  bool unprotectScopeConfirmed = false;
  std::vector<QString> unprotectTargetVolumes{};
  bool protectScopeConfirmed = false;
  std::vector<QString> protectTargetVolumes{};
};
struct OverlayMaintenanceResult {
  OverlayMaintenanceSnapshot snapshot;
  QString error;
  std::optional<OperationFailure> failure;
  WriteEffect effect = WriteEffect::None;
  // Confirmed completion (receipt or already satisfied); not an independent readback.
  bool completionConfirmed = false;
  std::vector<AllVolumeProtectionItemResult> allVolumeItems{};
  std::optional<MaintenanceValidationIssue> validationIssue{};
  [[nodiscard]] bool succeeded() const { return error.isEmpty() && !failure && !validationIssue && completionConfirmed; }
};
class OverlayMaintenanceActions {
 public:
  virtual ~OverlayMaintenanceActions() = default;
  [[nodiscard]] virtual OverlayMaintenanceSnapshot observe(std::stop_token stop = {}) = 0;
  // Project shared in-memory observations without refreshing the host or UWF.
  [[nodiscard]] virtual std::optional<OverlayMaintenanceSnapshot> snapshot() const { return {}; }
  [[nodiscard]] virtual OverlayMaintenanceResult fullCommit(std::stop_token stop = {}) = 0;
  [[nodiscard]] virtual OverlayMaintenanceResult createSwap(const QString& volume, std::stop_token stop = {}) = 0;
  [[nodiscard]] virtual OverlayMaintenanceResult setHorm(bool enabled, std::stop_token stop = {}) = 0;
  [[nodiscard]] virtual OverlayMaintenanceResult setAllVolumesProtected(bool enabled, std::stop_token stop = {}) {
    (void)enabled;
    (void)stop;
    OverlayMaintenanceResult result;
    result.validationIssue = MaintenancePlanIssue::BatchProtectionUnsupported;
    result.failure = OperationFailure{OperationFailureKind::Unsupported, WriteEffect::None, {}};
    return result;
  }
};
}  // namespace uwf::app
