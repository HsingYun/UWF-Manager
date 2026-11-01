// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include "application/overlay/contracts/OverlayMaintenance.h"
#include "application/overlay/contracts/OverlayPolicy.h"
#include "domain/uwf/model/UwfModel.h"

namespace uwf::app {
// Protect=true creates a missing Next registration. Unprotect and a Current
// registration alone do not establish a target for Next configuration edits.
[[nodiscard]] constexpr bool hasPlannedVolumeConfiguration(bool nextRegistered, bool protectionRequested) { return nextRegistered || protectionRequested; }

// Semantic application changes; transport selection belongs to the store adapter.
struct ConfigurationChanges {
  core::PendingChanges settings{};
  OverlayPolicyDelta overlayPolicy{};
  OverlayMaintenanceChanges maintenance{};
  [[nodiscard]] bool empty() const { return settings.empty() && overlayPolicy.empty() && maintenance.empty(); }
  [[nodiscard]] std::size_t count() const {
    return settings.count() + static_cast<std::size_t>(overlayPolicy.passthrough.has_value()) + static_cast<std::size_t>(overlayPolicy.persistent.has_value()) +
           static_cast<std::size_t>(overlayPolicy.readOnlyMedia.has_value()) + static_cast<std::size_t>(overlayPolicy.reset.has_value()) + maintenance.count();
  }
};
}  // namespace uwf::app
