// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "domain/uwf/solvers/AllVolumeProtectionSolver.h"

#include "domain/uwf/policies/AllVolumeProtectionProjection.h"

namespace uwf::compat {
namespace {
AllVolumeProtectionEvaluation solve(const AllVolumeProtectionFacts& facts, const bool desired) {
  const auto reject = [](const ErrorKind kind, const char* message) {
    return AllVolumeProtectionEvaluation{AllVolumeProtectionResolution::Rejected, Failure{kind, Effect::None, message, {}}};
  };
  if (facts.administrator == false) return reject(ErrorKind::AccessDenied, "Administrator access is required to change protection on all volumes");
  if (facts.hormEnabled == true) return reject(ErrorKind::AccessDenied, "Disable HORM and restart before changing protection on all volumes");
  const auto persistence = core::configurationWriteRestriction(facts.mode);
  if (persistence == core::ConfigurationWriteRestriction::RomVolatile)
    return reject(ErrorKind::AccessDenied, core::configurationWriteReason(persistence));
  if (!facts.administrator || !facts.hormEnabled || !facts.observation.scopeConfirmed) return {};
  if (desired && facts.observation.volumes.empty()) return reject(ErrorKind::Unavailable, "The complete volume protection scope is empty");
  for (const auto& volume : facts.observation.volumes) {
    if (!parseVolumeId(volume.volume)) return reject(ErrorKind::Protocol, "The complete volume protection scope contains an invalid identity");
  }
  return {AllVolumeProtectionResolution::Ready, {}};
}
}  // namespace
std::optional<bool> solveGetAllProtected(const GetAllProtected&, const AllVolumeProtectionObservation& facts) { return aggregateVolumeProtection(facts, true); }
std::optional<bool> solveGetAllUnprotected(const GetAllUnprotected&, const AllVolumeProtectionObservation& facts) {
  return aggregateVolumeProtection(facts, false);
}
AllVolumeProtectionEvaluation solveProtectAll(const ProtectAll&, const AllVolumeProtectionFacts& facts) { return solve(facts, true); }
AllVolumeProtectionEvaluation solveUnprotectAll(const UnprotectAll&, const AllVolumeProtectionFacts& facts) { return solve(facts, false); }
}  // namespace uwf::compat
