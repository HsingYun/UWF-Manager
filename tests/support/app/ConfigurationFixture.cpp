// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "ConfigurationFixture.h"

namespace uwf::test::application {

core::UwfSnapshot editableSnapshot() {
  core::UwfSnapshot snapshot;
  snapshot.uwfAvailable = true;
  snapshot.elevated = true;
  snapshot.current.filter.enabled = true;
  snapshot.next.filter.enabled = true;
  snapshot.current.overlay = {core::OverlayType::RAM, 4096, 2048, 3072};
  snapshot.next.overlay = {core::OverlayType::Disk, 8192, 4096, 6144};
  snapshot.runtime = {3072, 1024};
  snapshot.current.volumes.push_back({"Volume{c}", "C:", true, true});
  snapshot.next.volumes.push_back({"Volume{c}", "C:", true, true});
  snapshot.current.volumes.push_back({"Volume{d}", "D:", false, true});
  snapshot.next.volumes.push_back({"Volume{d}", "D:", false, true});
  snapshot.next.fileExclusions["Volume{c}"] = {"C:\\Existing"};
  snapshot.next.registryExclusions = {"HKEY_LOCAL_MACHINE\\SOFTWARE\\Existing"};
  return snapshot;
}

}  // namespace uwf::test::application
