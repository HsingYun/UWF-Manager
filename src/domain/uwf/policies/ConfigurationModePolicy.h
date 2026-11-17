// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <optional>
#include <type_traits>

#include "domain/uwf/contracts/UwfStateFacts.h"
#include "domain/uwf/model/UwfModel.h"
#include "domain/uwf/policies/IndividualCommitPolicy.h"

namespace uwf::core {
// Ordinary setters update the SYSTEM configuration hive. Filter and HORM
// transitions have their own persistence paths and must not inherit this gate.
enum class ConfigurationWrite { Ordinary, Threshold };
enum class ConfigurationWriteRestriction { None, HormActive, RomVolatile, RomCommitUnavailable, Unknown };
struct ConfigurationModeState {
  std::optional<bool> filterEnabled;
  std::optional<bool> systemVolumeProtected;
  IndividualCommitState runtime;
};
// Admission for a new host registry mutation, before it is performed. FullCommit
// makes future writes direct; it never proves historical overlay data durable.
enum class RegistryWriteDisposition { Direct, RequiresCommit, HormActive, ReadOnlyMediaActive, Unknown };
[[nodiscard]] constexpr RegistryWriteDisposition registryWriteDisposition(const ConfigurationModeState& state) {
  if (state.filterEnabled == false || state.systemVolumeProtected == false) return RegistryWriteDisposition::Direct;
  if (state.filterEnabled != true || state.systemVolumeProtected != true || !state.runtime.hormEnabled) return RegistryWriteDisposition::Unknown;
  if (*state.runtime.hormEnabled) return RegistryWriteDisposition::HormActive;
  if (state.runtime.fullCommit == true || (state.runtime.romDisposition && (*state.runtime.romDisposition & compat::disposition::fullCommit) != 0))
    return RegistryWriteDisposition::Direct;
  const auto rom = effectiveReadOnlyMedia(state.runtime);
  if (!rom) return RegistryWriteDisposition::Unknown;
  return *rom ? RegistryWriteDisposition::ReadOnlyMediaActive : RegistryWriteDisposition::RequiresCommit;
}
[[nodiscard]] constexpr ConfigurationWriteRestriction configurationWriteRestriction(const ConfigurationModeState& state,
                                                                                    const ConfigurationWrite operation = ConfigurationWrite::Ordinary) {
  if (state.runtime.hormEnabled == true) return ConfigurationWriteRestriction::HormActive;
  if (state.filterEnabled == false || state.systemVolumeProtected == false) return ConfigurationWriteRestriction::None;
  if (!state.filterEnabled) return ConfigurationWriteRestriction::Unknown;
  const auto rom = effectiveReadOnlyMedia(state.runtime);
  if (rom == false) return ConfigurationWriteRestriction::None;
  if (!rom || !state.systemVolumeProtected) return ConfigurationWriteRestriction::Unknown;
  if (operation == ConfigurationWrite::Threshold) return ConfigurationWriteRestriction::RomCommitUnavailable;
  // FullCommit's supported future-write path is direct. Its trigger is not a
  // receipt proving that every historical overlay cluster reached storage.
  if (state.runtime.fullCommit == true || (state.runtime.romDisposition && (*state.runtime.romDisposition & compat::disposition::fullCommit) != 0))
    return ConfigurationWriteRestriction::None;
  return ConfigurationWriteRestriction::RomVolatile;
}
[[nodiscard]] constexpr const char* configurationWriteReason(const ConfigurationWriteRestriction reason) {
  switch (reason) {
    case ConfigurationWriteRestriction::None:
      return "";
    case ConfigurationWriteRestriction::HormActive:
      return "UWF settings cannot be changed while HORM is active.";
    case ConfigurationWriteRestriction::RomVolatile:
      return "Read-only ROM cannot persist ordinary UWF settings on the protected system volume. Disable UWF and restart before changing them.";
    case ConfigurationWriteRestriction::RomCommitUnavailable:
      return "Threshold changes require a registry commit, which is unavailable in read-only ROM.";
    case ConfigurationWriteRestriction::Unknown:
      return "Configuration persistence could not be confirmed. Refresh the system state before changing this setting.";
  }
  return "";
}
[[nodiscard]] inline ConfigurationModeState configurationModeState(const UwfSnapshot& snapshot) {
  return {snapshot.current.filter.enabled,
          snapshot.runtime.systemVolumeProtected,
          {snapshot.runtime.hormEnabled, snapshot.runtime.currentOverlayFlags, snapshot.runtime.romDisposition, snapshot.runtime.fullCommit}};
}
}  // namespace uwf::core
namespace uwf::compat {
[[nodiscard]] inline std::optional<core::ConfigurationWrite> configurationWrite(const Request& request) {
  return std::visit(
      [](const auto& operation) -> std::optional<core::ConfigurationWrite> {
        using T = std::decay_t<decltype(operation)>;
        if constexpr (std::is_same_v<T, SetVolume> || std::is_same_v<T, AddVolume> || std::is_same_v<T, RemoveVolume> || std::is_same_v<T, EditFileExclusion> ||
                      std::is_same_v<T, EditRegistryExclusion>)
          return core::ConfigurationWrite::Ordinary;
        else if constexpr (std::is_same_v<T, SetBoolean>) {
          if (operation.property == BooleanProperty::PersistDomainSecretKey || operation.property == BooleanProperty::PersistTscal)
            return core::ConfigurationWrite::Ordinary;
        } else if constexpr (std::is_same_v<T, SetNumber>) {
          if (operation.property == NumberProperty::WarningThreshold || operation.property == NumberProperty::CriticalThreshold)
            return core::ConfigurationWrite::Threshold;
          if (operation.property == NumberProperty::ResetPersistentStateSavedMode) return core::ConfigurationWrite::Ordinary;
        }
        return {};
      },
      request);
}
[[nodiscard]] inline core::ConfigurationModeState configurationModeState(const UwfStateFacts& state) {
  std::optional<bool> protectedSystem;
  if (state.systemVolumeId.value) {
    const auto found = state.current.volumes.find(normalizeVolumeId(*state.systemVolumeId.value));
    if (found != state.current.volumes.end()) {
      if (found->second.present.value == false)
        protectedSystem = false;
      else if (found->second.present.value == true)
        protectedSystem = found->second.protectedVolume.value;
    } else if (state.current.volumesObserved)
      protectedSystem = false;
  }
  return {state.current.filterEnabled.value, protectedSystem, individualCommitState(state)};
}
}  // namespace uwf::compat
