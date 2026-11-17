// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QString>
#include <stdexcept>

#include "application/state/contracts/ApplicationState.h"
#include "application/state/contracts/ApplicationStateSource.h"

namespace uwf::test::application {
inline core::UwfSnapshot configurationSnapshot(const core::SessionSnapshot& current, const core::SessionSnapshot& next,
                                               const core::OverlayRuntime& runtime = {}) {
  core::UwfSnapshot result;
  result.uwfAvailable = true;
  result.elevated = true;
  result.current = current;
  result.next = next;
  // Widget fixtures built here describe mounted records. Tests for moved or
  // offline volumes supply their physical observation explicitly.
  for (auto* session : {&result.current, &result.next})
    for (auto& volume : session->volumes)
      if (!volume.physicalDriveLetter) volume.physicalDriveLetter = volume.driveLetter;
  result.runtime = runtime;
  if (!result.runtime.romDisposition) result.runtime.romDisposition = 0;
  if (!result.runtime.fullCommit) result.runtime.fullCommit = false;
  if (!result.runtime.hormEnabled) result.runtime.hormEnabled = false;
  if (!result.runtime.currentOverlayFlags) result.runtime.currentOverlayFlags = 0;
  if (!result.runtime.systemVolumeProtected) {
    result.runtime.systemVolumeProtected = false;
    for (const auto& volume : result.current.volumes)
      if (volume.driveLetter == "C:") result.runtime.systemVolumeProtected = volume.isProtected;
  }
  return result;
}

class MutableApplicationStateSource final : public app::ApplicationStateSource {
 public:
  std::vector<core::VolumeInfo> volumes;
  core::UwfSnapshot snapshot;
  app::OverlayPolicySnapshot policy;
  std::optional<bool> hormEnabled{};
  QString failure;
  int reads = 0;

  ReadOperation readOperation(uwf::app::UwfCapability) override {
    ++reads;
    return [candidate = app::ApplicationState{volumes, snapshot, policy, hormEnabled}, error = failure](std::stop_token) {
      if (!error.isEmpty()) throw std::runtime_error(error.toStdString());
      return candidate;
    };
  }
};

}  // namespace uwf::test::application
