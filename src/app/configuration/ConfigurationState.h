// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include "ConfigurationDraft.h"
#include "ExclusionDraft.h"
#include "app/staging/FileStagingConflictPolicy.h"
namespace uwf::app {
struct GlobalConfigurationState {
  GlobalConfigurationState() { draft.overlay.type = core::OverlayType::RAM; }
  GlobalConfigurationDraft draft;
  ExclusionDraft registry;
  bool available = true;
};

struct VolumeConfigurationState {
  std::string driveLetter;
  VolumeConfigurationDraft draft;
  ExclusionDraft files;
  FileStagingConflictPolicy conflicts;
  bool supported = true;
};

}  // namespace uwf::app
