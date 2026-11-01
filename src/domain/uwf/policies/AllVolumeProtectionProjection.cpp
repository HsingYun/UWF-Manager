// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "domain/uwf/policies/AllVolumeProtectionProjection.h"

#include <algorithm>
#include <utility>

#include "domain/uwf/contracts/UwfStateFacts.h"

namespace uwf::compat {
std::optional<bool> aggregateVolumeProtection(const AllVolumeProtectionObservation& observation, const bool protectedVolume) {
  if (!observation.scopeConfirmed || observation.failure || (protectedVolume && observation.volumes.empty())) return {};
  if (std::ranges::any_of(observation.volumes, [](const auto& volume) { return !volume.nextProtected.has_value() || volume.failure.has_value(); })) return {};
  return std::ranges::all_of(observation.volumes, [protectedVolume](const auto& volume) { return *volume.nextProtected == protectedVolume; });
}

AllVolumeProtectionObservation projectAllVolumeProtection(const UwfStateFacts& state, const bool protect) {
  AllVolumeProtectionObservation result;
  std::optional<std::vector<std::string>> scope;
  if (protect) {
    scope = state.runtime.protectScope.value;
  } else if (state.next.volumesObserved) {
    scope.emplace();
    for (const auto& [id, volume] : state.next.volumes) {
      if (!volume.present.value) {
        result.failure = Failure{ErrorKind::Unavailable, Effect::None, "The global Next volume membership is unavailable", {}};
        return result;
      }
      if (*volume.present.value) scope->push_back(id);
    }
  }
  if (!scope) {
    result.failure = Failure{ErrorKind::Unavailable, Effect::None, "The global complete volume protection scope is unavailable", {}};
    return result;
  }
  result.scopeConfirmed = true;
  for (const auto& target : *scope) {
    const auto identity = parseVolumeId(target);
    if (!identity) {
      result.scopeConfirmed = false;
      result.failure = Failure{ErrorKind::Protocol, Effect::None, "The global volume protection scope contains an invalid identity", {}};
      return result;
    }
    AllVolumeProtectionEntry volume{*identity, {}, {}};
    const auto found = state.next.volumes.find(*identity);
    if ((found == state.next.volumes.end() && state.next.volumesObserved) || (found != state.next.volumes.end() && found->second.present.value == false))
      volume.nextProtected = false;
    else if (found != state.next.volumes.end() && found->second.present.value == true)
      volume.nextProtected = found->second.protectedVolume.value;
    if (!volume.nextProtected) {
      volume.failure = Failure{ErrorKind::Unavailable, Effect::None, "The global Next volume protection is unavailable", {}};
      if (!result.failure)
        result.failure = volume.failure;
      else
        result.failure->diagnostics.push_back({{}, "AllVolumeProtection", 0, volume.failure->message});
    }
    result.volumes.push_back(std::move(volume));
  }
  result.value = aggregateVolumeProtection(result, protect);
  return result;
}
}  // namespace uwf::compat
