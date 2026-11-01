// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <algorithm>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "application/uwf/UwfRequestPrecheck.h"
#include "application/uwf/UwfStateUpdates.h"
#include "support/uwf/FakeUwfBackend.h"

namespace uwf::test::state_consistency {
using namespace uwf::compat;
using Fake = uwf::test::FakeUwfBackend;
inline constexpr auto volumeC = "11111111-2222-3333-4444-555555555555";
inline constexpr auto volumeD = "66666666-7777-8888-9999-aaaaaaaaaaaa";

inline Fake::State initialState() {
  Fake::State state;
  state.current.overlayType = static_cast<std::uint32_t>(OverlayType::Disk);
  state.current.maximumOverlaySize = 4096;
  state.current.warningThreshold = 400;
  state.current.criticalThreshold = 700;
  state.current.overlayFlags = 0x80000002u;
  state.current.volumes.emplace(volumeC, Fake::VolumeConfiguration{.volume = {volumeC, "C:", true, false, false}, .fileExclusions = {"\\Existing"}});
  state.current.volumes.emplace(volumeD, Fake::VolumeConfiguration{.volume = {volumeD, "D:", false, true, false}, .fileExclusions = {"\\Other"}});
  state.current.registryExclusions = {"HKEY_LOCAL_MACHINE\\SOFTWARE\\Existing"};
  state.next = state.current;
  state.runtime.systemVolumeId = volumeC;
  state.runtime.controlDevicePresent = true;
  state.runtime.physicalVolumes = {{volumeC, {true, true}}, {volumeD, {true, true}}};
  state.runtime.fileMaintenancePrefixes = {{volumeC, "C:"}, {volumeD, "D:"}};
  return state;
}

inline UwfStateFacts observedFacts(const Fake& backend) { return backend.state(); }

class Comparison {
 public:
  std::string difference;
  void fact(const Fact<std::vector<std::string>>& actual, const Fact<std::vector<std::string>>& expected, const std::string& name) {
    if (!difference.empty() || !expected.value) return;
    if (!actual.value) {
      difference = name;
      return;
    }
    // These collections describe membership, with duplicate entries and original
    // spelling preserved. Getter ordering is tested by CollectionContractTests.
    auto actualEntries = *actual.value;
    auto expectedEntries = *expected.value;
    std::ranges::sort(actualEntries);
    std::ranges::sort(expectedEntries);
    if (actualEntries != expectedEntries) difference = name;
  }
  template <class T>
  void fact(const Fact<T>& actual, const Fact<T>& expected, const std::string& name) {
    if (difference.empty() && expected.value && actual.value != expected.value) difference = name;
  }
  void bits(const BitFacts& actual, const BitFacts& expected, const std::string& name) {
    if (difference.empty() && (!actual.valid() || (actual.knownMask & expected.knownMask) != expected.knownMask ||
                               ((actual.valueBits ^ expected.valueBits) & expected.knownMask) != 0))
      difference = name;
  }
  void session(const SessionStateFacts& actual, const SessionStateFacts& expected, const std::string& prefix) {
    fact(actual.filterEnabled, expected.filterEnabled, prefix + ".filterEnabled");
    fact(actual.servicingEnabled, expected.servicingEnabled, prefix + ".servicingEnabled");
    fact(actual.persistDomainSecretKey, expected.persistDomainSecretKey, prefix + ".persistDomainSecretKey");
    fact(actual.persistTscal, expected.persistTscal, prefix + ".persistTscal");
    fact(actual.overlayType, expected.overlayType, prefix + ".overlayType");
    fact(actual.maximumOverlaySize, expected.maximumOverlaySize, prefix + ".maximumOverlaySize");
    fact(actual.savedMode, expected.savedMode, prefix + ".savedMode");
    fact(actual.registryExclusions, expected.registryExclusions, prefix + ".registryExclusions");
    bits(actual.overlayFlags, expected.overlayFlags, prefix + ".overlayFlags");
    if (difference.empty() && expected.volumesObserved && !actual.volumesObserved) difference = prefix + ".volumesObserved";
    for (const auto& [id, volume] : expected.volumes) {
      const auto found = actual.volumes.find(id);
      if (found == actual.volumes.end()) {
        if (difference.empty() && (volume.present.value != false || !actual.volumesObserved || volume.physicalDriveLetter.value))
          difference = prefix + ".volumes[" + id + "]";
        continue;
      }
      const auto name = prefix + ".volumes[" + id + "]";
      fact(found->second.present, volume.present, name + ".present");
      fact(found->second.protectedVolume, volume.protectedVolume, name + ".protectedVolume");
      fact(found->second.bindByDriveLetter, volume.bindByDriveLetter, name + ".bindByDriveLetter");
      fact(found->second.driveLetter, volume.driveLetter, name + ".driveLetter");
      fact(found->second.physicalDriveLetter, volume.physicalDriveLetter, name + ".physicalDriveLetter");
      fact(found->second.swapfileMarker, volume.swapfileMarker, name + ".swapfileMarker");
      fact(found->second.fileExclusions, volume.fileExclusions, name + ".fileExclusions");
    }
    for (const auto& [id, volume] : actual.volumes) {
      const auto found = expected.volumes.find(id);
      const bool absent = found == expected.volumes.end() || found->second.present.value == false;
      if (difference.empty() && expected.volumesObserved && absent && volume.present.value == true) difference = prefix + ".unexpectedVolume[" + id + "]";
    }
  }
  void state(const UwfStateFacts& actual, const UwfStateFacts& expected) {
    session(actual.current, expected.current, "Current");
    session(actual.next, expected.next, "Next");
    fact(actual.administrator, expected.administrator, "administrator");
    fact(actual.systemVolumeId, expected.systemVolumeId, "systemVolumeId");
    fact(actual.capturedSettingsAvailable, expected.capturedSettingsAvailable, "capturedSettingsAvailable");
#define COMPARE_RUNTIME(field) fact(actual.runtime.field, expected.runtime.field, "Runtime." #field)
    COMPARE_RUNTIME(horm);
    COMPARE_RUNTIME(shutdownPending);
    COMPARE_RUNTIME(controlDevicePresent);
    COMPARE_RUNTIME(hiberFile);
    COMPARE_RUNTIME(hormVolumesProtected);
    COMPARE_RUNTIME(fullCommit);
    COMPARE_RUNTIME(shutdownPrivilegeAvailable);
    COMPARE_RUNTIME(currentControlSet);
    COMPARE_RUNTIME(resetState);
    COMPARE_RUNTIME(warningThreshold);
    COMPARE_RUNTIME(criticalThreshold);
    COMPARE_RUNTIME(servicingStatus);
    COMPARE_RUNTIME(savedServicingFilter);
    COMPARE_RUNTIME(protectScope);
#undef COMPARE_RUNTIME
    bits(actual.runtime.romDisposition, expected.runtime.romDisposition, "Runtime.romDisposition");
    for (const auto& [id, target] : expected.targets) {
      if (!target.volumeEligibility.value) continue;
      const auto found = actual.targets.find(id);
      if (found == actual.targets.end()) {
        if (difference.empty()) difference = "physicalTarget[" + id + "]";
      } else {
        fact(found->second.volumeEligibility, target.volumeEligibility, "physicalTarget[" + id + "].eligibility");
      }
    }
  }
};

struct RefreshCount {
  int value = 0;
};
struct Fixture {
  std::shared_ptr<Fake> native;
  std::shared_ptr<RefreshCount> refreshes = std::make_shared<RefreshCount>();
  app::PrecheckedUwf system;
  explicit Fixture(Fake::State initial)
      : native(std::make_shared<Fake>(std::move(initial))),
        system(native->rawBackend(), {}, true, [backend = native, count = refreshes](const Uwf& source, const Context& context) {
          ++count->value;
          auto result = app::refreshUwfState(source, context);
          auto state = source.state();
          const auto host = observedFacts(*backend);
          // Do not copy Current/Next or getter-backed fields: the basic refresh
          // and primitive receipts must publish those themselves.
          state.administrator = host.administrator;
          state.systemVolumeId = host.systemVolumeId;
          state.capturedSettingsAvailable = host.capturedSettingsAvailable;
          state.runtime.controlDevicePresent = host.runtime.controlDevicePresent;
          state.runtime.romDisposition = host.runtime.romDisposition;
          state.runtime.hiberFile = host.runtime.hiberFile;
          state.runtime.hormVolumesProtected = host.runtime.hormVolumesProtected;
          state.runtime.shutdownPrivilegeAvailable = host.runtime.shutdownPrivilegeAvailable;
          state.runtime.currentControlSet = host.runtime.currentControlSet;
          state.runtime.servicingStatus = host.runtime.servicingStatus;
          state.runtime.savedServicingFilter = host.runtime.savedServicingFilter;
          source.updateState(std::move(state));
          return result;
        }) {
    system.updateState(observedFacts(*native));
    native->clearTrace();
  }
  std::string difference() const {
    Comparison comparison;
    comparison.state(system.state(), observedFacts(*native));
    return comparison.difference;
  }
  std::vector<Request> writes() const {
    std::vector<Request> requests;
    for (const auto& entry : native->trace())
      if (isWrite(entry.request)) requests.push_back(entry.request);
    return requests;
  }
};
}  // namespace uwf::test::state_consistency
