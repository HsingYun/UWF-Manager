// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include "application/volumes/contracts/AllVolumeProtection.h"
#include "domain/uwf/contracts/AllVolumeProtection.h"

namespace uwf::app {
[[nodiscard]] std::shared_ptr<compat::AllVolumeProtection> composeAllVolumeProtection(std::shared_ptr<compat::Uwf> system,
                                                                                      compat::AllVolumeProtectionEnvironment environment);
[[nodiscard]] std::shared_ptr<AllVolumeProtectionActions> allVolumeProtectionActions(std::shared_ptr<compat::AllVolumeProtection> operations);
}  // namespace uwf::app
