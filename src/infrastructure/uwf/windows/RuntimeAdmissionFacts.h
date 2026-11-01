// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <string>
#include <variant>
#include <vector>

#include "domain/uwf/contracts/Backend.h"
#include "domain/uwf/contracts/BackendObservations.h"

namespace uwf::windows {
// Read-only Windows observations, independent of provider execution and routing.
[[nodiscard]] compat::Response readRomDisposition(const compat::Context& context);
[[nodiscard]] compat::VolumeAdmissionResult inspectVolumeAdmission(const std::string& volume, bool currentFilter, compat::VolumeAdmissionPurpose purpose,
                                                                   const compat::Context& context);
[[nodiscard]] compat::Response readHormVolumeProtection(const compat::Context& context);
struct HormVolumeProtectionDetails {
  bool allProtected = true;
  std::vector<std::string> unprotectedVolumes;
};
using HormVolumeProtectionDetailsResult = std::variant<HormVolumeProtectionDetails, compat::Failure>;
// Enumerates all blockers using the boolean observation's eligibility and mount
// checks. Any read failure prevents a complete result.
[[nodiscard]] HormVolumeProtectionDetailsResult readHormVolumeProtectionDetails(const compat::Context& context);
[[nodiscard]] compat::Response readCapturedSettingsAvailable(const compat::Context& context);
[[nodiscard]] compat::Response readShutdownPrivilegeAvailable(const compat::Context& context);
[[nodiscard]] compat::ServicingAdmissionResult readServicingAdmission(bool enabled, const compat::Context& context);
}  // namespace uwf::windows
