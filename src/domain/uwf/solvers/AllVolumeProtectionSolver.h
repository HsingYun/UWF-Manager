// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include "domain/uwf/contracts/AllVolumeProtection.h"
#include "domain/uwf/policies/ConfigurationModePolicy.h"

namespace uwf::compat {
struct AllVolumeProtectionFacts {
  std::optional<bool> administrator;
  std::optional<bool> hormEnabled;
  AllVolumeProtectionObservation observation;
  core::ConfigurationModeState mode{};
};
enum class AllVolumeProtectionResolution { Ready, ReadRequired, Rejected };
struct AllVolumeProtectionEvaluation {
  AllVolumeProtectionResolution resolution = AllVolumeProtectionResolution::ReadRequired;
  std::optional<Failure> failure;
};
[[nodiscard]] std::optional<bool> solveGetAllProtected(const GetAllProtected&, const AllVolumeProtectionObservation& facts);
[[nodiscard]] std::optional<bool> solveGetAllUnprotected(const GetAllUnprotected&, const AllVolumeProtectionObservation& facts);
[[nodiscard]] AllVolumeProtectionEvaluation solveProtectAll(const ProtectAll&, const AllVolumeProtectionFacts& facts);
[[nodiscard]] AllVolumeProtectionEvaluation solveUnprotectAll(const UnprotectAll&, const AllVolumeProtectionFacts& facts);
}  // namespace uwf::compat
