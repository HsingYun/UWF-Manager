// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include "application/uwf/contracts/UwfMaintenanceRepository.h"
#include "domain/uwf/contracts/Backend.h"

namespace uwf::app {
[[nodiscard]] std::shared_ptr<UwfMaintenanceRepository> uwfMaintenanceRepository(std::shared_ptr<compat::Uwf> system);
}  // namespace uwf::app
