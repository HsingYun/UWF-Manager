// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <map>

#include "domain/uwf/contracts/Backend.h"

namespace uwf::windows {
struct VolumeProtectionInventory {
  std::vector<std::string> scope;
  std::map<std::string, compat::VolumeAdmission> admissions;
};
[[nodiscard]] VolumeProtectionInventory readVolumeProtectionInventory(bool currentFilter, const compat::Context& context = {});

// ProtectAll scope: eligible volumes from complete Windows enumeration, plus
// the system partition's stable GUID. Observation is read-only.
[[nodiscard]] std::vector<std::string> readAllVolumeProtectionScope(bool currentFilter, const compat::Context& context = {});
}  // namespace uwf::windows
