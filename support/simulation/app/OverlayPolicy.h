// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include "application/overlay/contracts/OverlayPolicy.h"

namespace uwf::simulation::application {
inline app::OverlayPolicySnapshot diskPersistentPolicy() {
  using app::PolicyObservationStatus;
  app::OverlayPolicySnapshot policy;
  policy.transportAvailable = true;
  policy.current.filterEnabled = {PolicyObservationStatus::Available, true};
  policy.next.filterEnabled = policy.current.filterEnabled;
  policy.current.overlayType = {PolicyObservationStatus::Available, 1};
  policy.next.overlayType = policy.current.overlayType;
  policy.current.flags = {PolicyObservationStatus::Available, app::OverlayFlags{3}};
  policy.next.flags = policy.current.flags;
  policy.resetPersistentState = {PolicyObservationStatus::Available, 0};
  return policy;
}
}  // namespace uwf::simulation::application
