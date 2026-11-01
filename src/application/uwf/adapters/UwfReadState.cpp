// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "application/uwf/adapters/UwfReadState.h"
#include "domain/uwf/policies/ConfigurationModePolicy.h"

#include <algorithm>
#include <map>
#include <stdexcept>
#include <utility>

#include "application/uwf/adapters/UwfCall.h"
#include "domain/uwf/policies/IndividualCommitPolicy.h"
#include "domain/uwf/policies/VolumeIdentityPolicy.h"
#include "foundation/paths/DriveLetter.h"

namespace uwf::app {
namespace {
using PhysicalMounts = std::map<std::string, std::vector<const core::VolumeInfo*>>;
PhysicalMounts physicalMounts(const std::vector<core::VolumeInfo>& volumes) {
  PhysicalMounts mounts;
  for (const auto& physical : volumes) {
    const auto id = compat::parseVolumeId(physical.volumeName);
    if (id) mounts[*id].push_back(&physical);
  }
  return mounts;
}
void publishPhysicalMounts(compat::UwfStateFacts& state, const PhysicalMounts& mounts) {
  // This is a complete physical inventory: removed mounts must not retain
  // their previous drive letter in the newly published state.
  for (auto* session : {&state.current, &state.next})
    for (auto& [id, volume] : session->volumes) {
      (void)id;
      volume.physicalDriveLetter = compat::Fact<std::string>::known({});
    }
  for (const auto& [id, aliases] : mounts) {
    for (auto* scope : {&state.current, &state.next}) {
      auto& volume = scope->volumes[id];
      if (!volume.present.isKnown() && scope->volumesObserved) volume.present = compat::Fact<bool>::known(false);
      auto selected = aliases.front();
      for (const auto* alias : aliases) {
        if (selected->driveLetter.empty() && !alias->driveLetter.empty()) selected = alias;
        if (volume.driveLetter.value && !volume.driveLetter.value->empty() &&
            drive::normalize(*volume.driveLetter.value) == drive::normalize(alias->driveLetter)) {
          selected = alias;
          break;
        }
      }
      volume.physicalDriveLetter = compat::Fact<std::string>::known(selected->driveLetter);
    }
  }
}
void publishPhysicalState(compat::UwfStateFacts& state, const PhysicalMounts& mounts) {
  publishPhysicalMounts(state, mounts);
  for (const auto& [id, aliases] : mounts) {
    const auto& physical = *aliases.front();
    auto& target = state.targets[id];
    if (physical.physical.fixedMedia && physical.physical.supportedBus) {
      const bool fixed = *physical.physical.fixedMedia;
      const bool eligible = fixed && *physical.physical.supportedBus;
      // With Current UWF active, the driver's volume state is an additional
      // registration constraint. Host media/bus facts cannot override it.
      if (!eligible || state.current.filterEnabled.value == false) target.volumeEligibility = compat::Fact<compat::VolumeAdmission>::known({fixed, eligible});
    }
  }
}
}  // namespace
void publishUwfPhysicalState(compat::UwfStateFacts& state, const std::vector<core::VolumeInfo>& volumes) {
  publishPhysicalState(state, physicalMounts(volumes));
}
compat::Response refreshUwfStateWithPhysicalInventory(const compat::Uwf& system, const std::vector<core::VolumeInfo>& volumes, const compat::Context& context) {
  return compat::synchronizedUwf(system, [&] {
    const auto mounts = physicalMounts(volumes);
    auto before = system.state();
    publishPhysicalState(before, mounts);
    system.updateState(std::move(before));
    auto result = system.refreshState(context);
    // The first refresh discovers configured letters; projecting before it
    // would arbitrarily select the first alias and retain that wrong selection.
    auto state = system.state();
    // Admission belongs to the completed refresh, including an unknown value
    // after failed discovery. Only mount selection depends on the new letters.
    publishPhysicalMounts(state, mounts);
    system.updateState(std::move(state));
    return result;
  });
}
core::OverlayType overlayTypeFromUwf(const std::uint32_t type) {
  switch (static_cast<compat::OverlayType>(type)) {
    case compat::OverlayType::Ram:
      return core::OverlayType::RAM;
    case compat::OverlayType::Disk:
      return core::OverlayType::Disk;
  }
  throwUwfOperationError(compat::Error({compat::ErrorKind::Protocol, compat::Effect::None, "UWF returned an unsupported overlay type", {}}));
}

UwfCapability probeUwfCapability(const compat::Uwf& system) {
  const compat::Request request = compat::GetBoolean{compat::BooleanProperty::FilterEnabled};
  const auto capability = system.probe(request);
  if (capability.support == compat::Support::Unsupported) return UwfCapability::Unavailable;
  // An available entry point does not guarantee a successful snapshot read.
  return UwfCapability::Available;
}

core::VolumeRecord toVolumeRecord(const compat::Volume& volume) {
  return {.volumeName = "Volume{" + compat::normalizeVolumeId(volume.id) + '}',
          .driveLetter = volume.driveLetter,
          .isProtected = volume.protectedVolume,
          .binding = volume.bindByDriveLetter ? core::VolumeBinding::DriveLetter : core::VolumeBinding::VolumeId};
}

std::string toUwfFilePath(const std::string& path, const std::string& driveLetter) {
  const auto split = drive::split(path);
  if (!split.letter.empty() && split.letter != drive::normalize(driveLetter)) throw std::invalid_argument("The file path belongs to a different volume");
  // A volume-relative pattern can carry the extended-path marker as raw data.
  // Only a recognized drive prefix belongs to display/routing syntax.
  return split.letter.empty() ? path : split.rest;
}

core::UwfSnapshot publishedUwfSnapshot(const compat::UwfStateFacts& state, const UwfCapability capability, const bool elevated,
                                       const compat::OverlayUsage usage) {
  core::UwfSnapshot result;
  result.elevated = elevated;
  if (capability == UwfCapability::Unavailable) {
    result.unavailableReason = "UWF is not registered";
    return result;
  }
  const auto required = []<class T>(const compat::Fact<T>& field, const char* name) -> const T& {
    if (!field.value) throw OperationError(std::make_error_code(std::errc::state_not_recoverable), std::string("Published UWF state has no ") + name);
    return *field.value;
  };
  const auto maintenance = compat::individualCommitState(state);
  result.runtime = {.availableSpaceMb = usage.availableMb,
                    .currentConsumptionMb = usage.consumptionMb,
                    .romDisposition = maintenance.romDisposition,
                    .fullCommit = maintenance.fullCommit,
                    .hormEnabled = maintenance.hormEnabled,
                    .currentOverlayFlags = maintenance.overlayFlags,
                    .systemVolumeProtected = compat::configurationModeState(state).systemVolumeProtected};
  const auto warning = required(state.runtime.warningThreshold, "warning threshold");
  const auto critical = required(state.runtime.criticalThreshold, "critical threshold");
  for (const auto session : {compat::Session::Current, compat::Session::Next}) {
    const auto& source = session == compat::Session::Current ? state.current : state.next;
    auto& target = session == compat::Session::Current ? result.current : result.next;
    target.filter.enabled = required(source.filterEnabled, "filter state");
    target.overlay = {.type = overlayTypeFromUwf(required(source.overlayType, "overlay type")),
                      .maximumSizeMb = required(source.maximumOverlaySize, "overlay capacity"),
                      .warningThresholdMb = warning,
                      .criticalThresholdMb = critical};
    if (!source.volumesObserved) throw OperationError(std::make_error_code(std::errc::state_not_recoverable), "Published UWF volume inventory is incomplete");
    for (const auto& [id, volume] : source.volumes) {
      if (!required(volume.present, "volume membership")) continue;
      auto record = toVolumeRecord({id,
                                    required(volume.driveLetter, "configured drive letter"),
                                    required(volume.protectedVolume, "volume protection"),
                                    required(volume.bindByDriveLetter, "volume binding"),
                                    {}});
      record.physicalDriveLetter = volume.physicalDriveLetter.value;
      // Configured GUIDs remain distinct even when their saved letters overlap.
      // Only consumers select which record applies to a physical page.
      target.volumes.push_back(record);
      if (session == compat::Session::Current && !record.isProtected) continue;
      auto& paths = target.fileExclusions[record.volumeName];
      // Native, WMI and CSP expose the original per-volume patterns. A pattern
      // that resembles a complete GUID path is still literal provider data;
      // consumers add their own explicit address shell without losing it.
      for (const auto& path : required(volume.fileExclusions, "file exclusions"))
        if (!path.empty()) paths.push_back(path);
    }
    target.persistDomainSecretKey = required(source.persistDomainSecretKey, "domain persistence");
    target.persistTSCAL = required(source.persistTscal, "license persistence");
    for (const auto& key : required(source.registryExclusions, "registry exclusions"))
      if (!key.empty()) target.registryExclusions.push_back(key);
  }
  result.uwfAvailable = true;
  core::sortSnapshot(result);
  return result;
}

}  // namespace uwf::app
