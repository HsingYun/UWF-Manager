// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <optional>
#include <vector>

#include "application/overlay/contracts/OverlayPolicy.h"
#include "domain/uwf/model/UwfModel.h"

namespace uwf::app {

struct ApplicationState {
  std::vector<core::VolumeInfo> volumes;
  core::UwfSnapshot snapshot;
  OverlayPolicySnapshot overlayPolicy{};
  // This runtime fact is independent of OverlayFlags. Execution prechecks use
  // the shared UWF state rather than querying through this presentation view.
  std::optional<bool> hormEnabled{};
  // Configuration-only publication must not replace a newer live usage sample.
  bool overlayUsageSampled = true;
};

}  // namespace uwf::app
