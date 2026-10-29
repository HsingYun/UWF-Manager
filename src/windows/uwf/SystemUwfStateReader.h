// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <vector>

#include "app/api/UwfCapability.h"
#include "core/UwfModel.h"

namespace uwf::windows {

[[nodiscard]] app::UwfCapability probeUwfCapability();
[[nodiscard]] core::UwfSnapshot readSnapshot(app::UwfCapability capability);
[[nodiscard]] std::vector<core::VolumeInfo> enumerateVolumes();

}  // namespace uwf::windows
