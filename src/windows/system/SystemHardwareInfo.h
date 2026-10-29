// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include "app/api/SystemInformation.h"

namespace uwf::windows {

[[nodiscard]] const app::SystemHardwareInfo& systemHardwareInfo();
[[nodiscard]] uint32_t systemTotalRamMb();

}  // namespace uwf::windows
