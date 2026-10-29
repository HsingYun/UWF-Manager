// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QString>
#include <cstdint>
#include <memory>
#include <vector>

#include "ExclusionDraft.h"
#include "core/UwfModel.h"

namespace uwf::app {

class GlobalConfigurationEditing {
 public:
  virtual ~GlobalConfigurationEditing() = default;
  virtual bool importFilterEnabled(bool value) = 0;
  virtual bool importOverlayType(core::OverlayType type) = 0;
  virtual bool importOverlayMaxMb(uint32_t value) = 0;
  virtual bool importOverlayWarnMb(uint32_t value) = 0;
  virtual bool importOverlayCritMb(uint32_t value) = 0;
  virtual void finishImport() = 0;
};

class VolumeConfigurationEditing {
 public:
  virtual ~VolumeConfigurationEditing() = default;
  [[nodiscard]] virtual QString driveLetter() const = 0;
  virtual bool importProtect(bool value) = 0;
  virtual ExclusionImportOutcome importAddFileExclusion(const QString& path) = 0;
  virtual ExclusionImportOutcome importRemoveFileExclusion(const QString& path) = 0;
};

// Resolve each command independently and keep its target alive through callbacks.
using VolumeConfigurationEditors = std::vector<std::weak_ptr<VolumeConfigurationEditing>>;

}  // namespace uwf::app
