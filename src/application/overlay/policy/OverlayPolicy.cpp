// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "application/overlay/contracts/OverlayPolicy.h"

namespace uwf::app {
namespace {
std::optional<bool> persistentDisk(const OverlayPolicySession& session) {
  if (!session.overlayType.value || !session.flags.value) return std::nullopt;
  if (*session.overlayType.value != 0 && *session.overlayType.value != 1) return std::nullopt;
  return *session.overlayType.value == 1 && session.flags.value->persistent();
}

}  // namespace

std::optional<PersistentReset> OverlayPolicySnapshot::resetMode() const {
  if (!resetPersistentState.value) return std::nullopt;
  switch (*resetPersistentState.value) {
    case 0:
      return PersistentReset::Preserve;
    case 1:
      return PersistentReset::Clear;
    case 255:
      return PersistentReset::UseSavedMode;
    default:
      return std::nullopt;
  }
}

std::optional<bool> OverlayPolicySnapshot::currentPersistentDisk() const { return persistentDisk(current); }

bool OverlayPolicySnapshot::preservesOverlayData() const {
  return transportAvailable && current.filterEnabled.status == PolicyObservationStatus::Available && current.filterEnabled.value == true &&
         next.filterEnabled.status == PolicyObservationStatus::Available && next.filterEnabled.value == true &&
         current.overlayType.status == PolicyObservationStatus::Available && current.flags.status == PolicyObservationStatus::Available &&
         next.overlayType.status == PolicyObservationStatus::Available && next.flags.status == PolicyObservationStatus::Available &&
         currentPersistentDisk() == true && persistentDisk(next) == true && !current.flags.value->readOnlyMedia() && !next.flags.value->readOnlyMedia() &&
         resetPersistentState.status == PolicyObservationStatus::Available && resetMode() == PersistentReset::Preserve;
}

std::uint32_t OverlayPolicyDelta::applyFlags(std::uint32_t baseline) const {
  const auto merge = [&](std::uint32_t bit, const std::optional<bool> value) {
    if (value) baseline = *value ? baseline | bit : baseline & ~bit;
  };
  merge(OverlayFlags::Passthrough, passthrough);
  merge(OverlayFlags::Persistent, persistent);
  merge(OverlayFlags::ReadOnlyMedia, readOnlyMedia);
  return baseline;
}
bool OverlayPolicyDelta::empty() const { return !passthrough && !persistent && !readOnlyMedia && !reset; }

}  // namespace uwf::app
