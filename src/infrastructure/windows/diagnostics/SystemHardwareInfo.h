// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include "application/diagnostics/contracts/SystemInformation.h"

namespace uwf::windows {

[[nodiscard]] const app::SystemHardwareInfo& systemHardwareInfo();
[[nodiscard]] uint32_t systemTotalRamMb();

}  // namespace uwf::windows
