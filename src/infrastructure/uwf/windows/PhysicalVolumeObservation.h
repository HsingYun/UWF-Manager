// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <stop_token>
#include <string>

#include "domain/uwf/policies/VolumeAdmissionPolicy.h"

namespace uwf::windows {
// UWF's read-only media/bus observations back inventory admission and the
// execution-time recheck. Failures throw compat::Error; callers decide whether
// to publish Unknown for one inventory item or reject the requested operation.
[[nodiscard]] core::PhysicalVolumeFacts readPhysicalVolumeFacts(const std::string& volume, bool includeBus = true, std::stop_token stop = {});
}  // namespace uwf::windows
