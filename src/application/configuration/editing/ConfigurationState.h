// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include "application/configuration/editing/ConfigurationDraft.h"
#include "application/configuration/exclusions/ExclusionDraft.h"
#include "application/staging/editing/FileStagingConflictPolicy.h"
#include "domain/uwf/contracts/Types.h"

namespace uwf::app {
struct GlobalConfigurationState {
  GlobalConfigurationState() { draft.overlay.type = core::OverlayType::RAM; }
  GlobalConfigurationDraft draft;
  ExclusionDraft registry;
  bool available = true;
};

struct VolumeConfigurationState {
  VolumeConfigurationState() = default;
  explicit VolumeConfigurationState(const core::VolumeInfo& volume) : driveLetter(volume.driveLetter) {
    identityOnly = driveLetter.empty();
    draft.protectionTarget = identityOnly ? compat::parseVolumeId(volume.volumeName).value_or(std::string{}) : driveLetter;
    supported = identityOnly ? !draft.protectionTarget.empty() : core::supportsVolumeProtection(volume.support);
  }

  std::string driveLetter;
  VolumeConfigurationDraft draft;
  ExclusionDraft files;
  FileStagingConflictPolicy conflicts;
  bool supported = true;
  bool identityOnly = false;
};

}  // namespace uwf::app
