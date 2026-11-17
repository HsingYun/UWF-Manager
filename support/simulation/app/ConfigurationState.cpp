// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "ConfigurationState.h"

namespace uwf::simulation::application {

core::UwfSnapshot editableSnapshot() {
  core::UwfSnapshot snapshot;
  snapshot.uwfAvailable = true;
  snapshot.elevated = true;
  snapshot.current.filter.enabled = true;
  snapshot.next.filter.enabled = true;
  snapshot.current.overlay = {core::OverlayType::RAM, 4096, 2048, 3072};
  snapshot.next.overlay = {core::OverlayType::Disk, 8192, 4096, 6144};
  snapshot.runtime = {3072, 1024, 0, false};
  snapshot.runtime.hormEnabled = false;
  snapshot.runtime.currentOverlayFlags = 0;
  snapshot.runtime.systemVolumeProtected = true;
  snapshot.current.volumes.push_back({fixtureVolumeCName, "C:", true, core::VolumeBinding::DriveLetter, std::string("C:")});
  snapshot.next.volumes.push_back({fixtureVolumeCName, "C:", true, core::VolumeBinding::DriveLetter, std::string("C:")});
  snapshot.current.volumes.push_back({fixtureVolumeDName, "D:", false, core::VolumeBinding::DriveLetter, std::string("D:")});
  snapshot.next.volumes.push_back({fixtureVolumeDName, "D:", false, core::VolumeBinding::DriveLetter, std::string("D:")});
  snapshot.next.fileExclusions[fixtureVolumeCName] = {"\\Existing"};
  snapshot.next.registryExclusions = {"HKEY_LOCAL_MACHINE\\SOFTWARE\\Existing"};
  return snapshot;
}

}  // namespace uwf::simulation::application
