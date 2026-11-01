// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <cstdint>
#include <limits>
#include <map>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "domain/uwf/contracts/BackendObservations.h"
#include "domain/uwf/contracts/Requests.h"

namespace uwf::compat {
template <class T>
struct Fact {
  std::optional<T> value;
  [[nodiscard]] static Fact known(T observed) {
    Fact result;
    result.value.emplace(std::move(observed));
    return result;
  }
  [[nodiscard]] static Fact unknown() { return {}; }
  [[nodiscard]] bool isKnown() const { return value.has_value(); }
};

struct BitFacts {
  std::uint32_t knownMask = 0;
  std::uint32_t valueBits = 0;
  [[nodiscard]] static constexpr BitFacts known(const std::uint32_t value) { return {std::numeric_limits<std::uint32_t>::max(), value}; }
  [[nodiscard]] constexpr bool valid() const { return (valueBits & ~knownMask) == 0; }
  [[nodiscard]] constexpr std::optional<bool> hasAny(const std::uint32_t mask) const {
    if (!valid()) return std::nullopt;
    if ((valueBits & mask) != 0) return true;
    if ((knownMask & mask) == mask) return false;
    return std::nullopt;
  }
  [[nodiscard]] constexpr std::optional<bool> equals(const std::uint32_t value) const {
    if (!valid()) return std::nullopt;
    if (((valueBits ^ value) & knownMask) != 0) return false;
    if (knownMask == std::numeric_limits<std::uint32_t>::max()) return true;
    return std::nullopt;
  }
};

struct VolumeStateFacts {
  Fact<bool> present, protectedVolume, bindByDriveLetter;
  Fact<std::string> driveLetter;
  // Actual host mount, distinct from UWF's configured drive metadata.
  // Known empty means no drive-letter mount.
  Fact<std::string> physicalDriveLetter;
  Fact<std::uint32_t> swapfileMarker;
  Fact<std::vector<std::string>> fileExclusions;
};
struct SessionStateFacts {
  Fact<bool> filterEnabled, servicingEnabled, persistDomainSecretKey, persistTscal;
  Fact<std::uint32_t> overlayType, maximumOverlaySize, savedMode;
  BitFacts overlayFlags;
  std::map<std::string, VolumeStateFacts> volumes;
  bool volumesObserved = false;
  Fact<std::vector<std::string>> registryExclusions;
};
// Shared facts describe configuration or runtime state observable by production
// readers. Private rollback headers and final native safety checks remain inside
// their entry points. Overlay usage is sampled separately by GetOverlayUsage;
// data commits do not require a configuration refresh to maintain a usage cache.
struct RuntimeStateFacts {
  Fact<bool> horm, shutdownPending, controlDevicePresent, hiberFile, hormVolumesProtected;
  Fact<bool> fullCommit, shutdownPrivilegeAvailable;
  Fact<std::uint32_t> currentControlSet;
  BitFacts romDisposition;
  Fact<std::uint32_t> resetState, warningThreshold, criticalThreshold, servicingStatus;
  // Known(nullopt) is confirmed absence; an unknown observation is different.
  Fact<std::optional<bool>> savedServicingFilter;
  Fact<std::vector<std::string>> protectScope;
  // Unprotect scope is derived from the complete Next volume inventory.
};
struct TargetStateFacts {
  Fact<VolumeAdmission> volumeEligibility;
  Fact<FileMaintenanceFacts> fileTarget;
  // Aggregate paging-file validation is private to the native maintenance call,
  // rather than an independently observable property of this exact target.
  Fact<bool> persistentFileViewReady;
};
struct ObservationContext {
  bool applicable = true;
};
struct UwfStateFacts {
  SessionStateFacts current, next;
  RuntimeStateFacts runtime;
  // Only a completed global refresh owns this marker. Other state publishers
  // retain it so a partial observation cannot make an unhealthy snapshot ready.
  std::optional<Failure> refreshFailure;
  Fact<bool> administrator, capturedSettingsAvailable;
  Fact<std::string> systemVolumeId;
  // Keys are canonical volume GUIDs. Per-file facts describe only requestTarget.
  std::map<std::string, TargetStateFacts> targets;
  std::optional<Request> requestTarget;
  ObservationContext observation;
};
}  // namespace uwf::compat
