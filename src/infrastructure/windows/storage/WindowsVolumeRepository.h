// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <functional>
#include <memory>

#include "application/volumes/contracts/VolumeRepository.h"
#include "infrastructure/windows/management/wmi/WmiOperations.h"

namespace uwf::windows {
using PhysicalVolumeReader = std::function<core::PhysicalVolumeFacts(const std::string&)>;
// Explicit transports stay isolated unless a fallback is supplied.
[[nodiscard]] std::shared_ptr<app::VolumeRepository> windowsVolumeRepository(wmi::WmiOperations& inventory,
                                                                             std::shared_ptr<const app::VolumeRepository> fallback = {},
                                                                             PhysicalVolumeReader physical = {});
[[nodiscard]] std::shared_ptr<app::VolumeRepository> win32VolumeRepository();
[[nodiscard]] std::shared_ptr<app::VolumeRepository> systemVolumeRepository();
}  // namespace uwf::windows
