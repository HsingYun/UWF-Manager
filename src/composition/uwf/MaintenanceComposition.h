// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include "application/uwf/contracts/UwfMaintenanceRepository.h"

namespace uwf::composition {
[[nodiscard]] std::uint32_t systemCurrentControlSet();
[[nodiscard]] std::shared_ptr<app::UwfMaintenanceRepository> systemMaintenanceRepository();
}  // namespace uwf::composition
