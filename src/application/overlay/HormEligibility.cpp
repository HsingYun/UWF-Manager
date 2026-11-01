// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "application/overlay/HormEligibility.h"

#include "domain/uwf/model/UwfModel.h"

namespace uwf::app {
HormEligibility evaluateHormEligibility(const OverlayMaintenanceSnapshot& snapshot, const bool enabled) {
  std::optional<bool> rom;
  if (snapshot.currentOverlayType && snapshot.currentFlags)
    rom = core::overlayTypeFromValue(snapshot.currentOverlayType) == core::OverlayType::RAM && snapshot.currentFlags->raw == OverlayFlags::ReadOnlyMedia;
  HormEligibility result{{snapshot.filterEnabled, rom, snapshot.hibernationAvailable, snapshot.allVolumesProtected}};
  const auto reject = [&](const HormIssue issue, const std::optional<HormRequirement> requirement = {}) {
    result.issue = issue;
    result.blockingRequirement = requirement;
    return result;
  };
  if (!snapshot.hormCapability.canAttempt()) return reject(HormIssue::Unsupported);
  if (!snapshot.hormEnabled) return reject(HormIssue::StateUnknown);
  // Disabling and already-matching requests must not inherit enable checks.
  if (!enabled || *snapshot.hormEnabled) return result;
  // An inactive filter has no control device or active-volume observations.
  // Report the known prerequisite before requiring those runtime facts.
  if (snapshot.filterEnabled == false) return reject(HormIssue::FilterDisabled, HormRequirement::FilterEnabled);
  if (snapshot.overlayCommit == true) return reject(HormIssue::FullCommitActive);
  if (snapshot.romDisposition && *snapshot.romDisposition != 0) return reject(HormIssue::ProtectionStateChanged);
  if (snapshot.currentOverlayType && core::overlayTypeFromValue(snapshot.currentOverlayType) != core::OverlayType::RAM)
    return reject(HormIssue::OverlayNotRam, HormRequirement::ReadOnlyMedia);
  if (snapshot.currentFlags && snapshot.currentFlags->raw != OverlayFlags::ReadOnlyMedia)
    return reject(HormIssue::ConfigurationIncompatible, HormRequirement::ReadOnlyMedia);
  if (snapshot.hibernationAvailable == false) return reject(HormIssue::HibernationUnavailable, HormRequirement::Hibernation);
  if (snapshot.allVolumesProtected == false) return reject(HormIssue::VolumesUnprotected, HormRequirement::VolumeProtection);
  if (!snapshot.romDisposition) return reject(HormIssue::PrerequisitesUnknown);
  for (std::size_t index = 0; index != result.requirements.size(); ++index)
    if (!result.requirements[index]) return reject(HormIssue::PrerequisitesUnknown, static_cast<HormRequirement>(index));
  return result;
}
}  // namespace uwf::app
