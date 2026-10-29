// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "SystemUwfStateReader.h"

#include "app/uwf/UwfStateReader.h"
#include "windows/system/SystemCheck.h"
#include "windows/wmi/WmiSession.h"

namespace uwf::windows {

app::UwfCapability probeUwfCapability() { return app::probeUwfCapability(embeddedWmiSession()); }

core::UwfSnapshot readSnapshot(const app::UwfCapability capability) {
  return app::readSnapshot(embeddedWmiSession(), capability, isElevated(), drive::mountedLetters);
}

std::vector<core::VolumeInfo> enumerateVolumes() { return app::enumerateVolumes(cimv2WmiSession()); }

}  // namespace uwf::windows
