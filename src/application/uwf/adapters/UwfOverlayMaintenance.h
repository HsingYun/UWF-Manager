// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <memory>

#include "application/overlay/contracts/OverlayMaintenance.h"
#include "application/volumes/contracts/AllVolumeProtection.h"
#include "application/volumes/contracts/VolumeRepository.h"
#include "domain/uwf/contracts/Backend.h"

namespace uwf::app {
[[nodiscard]] std::shared_ptr<OverlayMaintenanceActions> uwfOverlayMaintenance(std::shared_ptr<compat::Uwf> system,
                                                                               std::shared_ptr<const VolumeRepository> volumes = {},
                                                                               OverlayMaintenanceReadOnlyPorts observations = {},
                                                                               std::shared_ptr<AllVolumeProtectionActions> allVolumeProtection = {});
}  // namespace uwf::app
