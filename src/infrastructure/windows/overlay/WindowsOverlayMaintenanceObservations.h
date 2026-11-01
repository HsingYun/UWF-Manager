// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <memory>

#include "application/overlay/contracts/OverlayMaintenance.h"
#include "application/volumes/contracts/VolumeRepository.h"

namespace uwf::windows {
// Mount paths label runtime blockers; eligibility comes from native observations.
[[nodiscard]] app::OverlayMaintenanceReadOnlyPorts windowsOverlayMaintenanceObservations(std::shared_ptr<const app::VolumeRepository> volumes);
}  // namespace uwf::windows
