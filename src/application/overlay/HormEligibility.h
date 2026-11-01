// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include "application/overlay/contracts/HormEligibilityTypes.h"
#include "application/overlay/contracts/OverlayMaintenance.h"

namespace uwf::app {
// One decision for reviewed plans, fresh execution admission and presentation.
// UI ownership/busy/elevation restrictions remain with the editing context.
[[nodiscard]] HormEligibility evaluateHormEligibility(const OverlayMaintenanceSnapshot& snapshot, bool enabled);
}  // namespace uwf::app
