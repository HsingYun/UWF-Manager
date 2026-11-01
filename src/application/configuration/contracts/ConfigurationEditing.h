// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QString>
#include <cstdint>
#include <memory>
#include <vector>

#include "application/configuration/contracts/ExclusionImportOutcome.h"
#include "domain/uwf/model/UwfModel.h"

namespace uwf::app {

enum class GlobalEditOutcome { Applied, Unchanged, Rejected };
struct GlobalEditResult {
  GlobalEditOutcome outcome;
  QString reason;
};

class GlobalConfigurationEditing {
 public:
  virtual ~GlobalConfigurationEditing() = default;
  [[nodiscard]] virtual bool available() const = 0;
  [[nodiscard]] virtual bool editingAllowed() const = 0;
  [[nodiscard]] virtual bool filterEnabled() const = 0;
  [[nodiscard]] virtual core::OverlayConfig overlayConfiguration() const = 0;
  virtual GlobalEditResult importFilterEnabled(bool value) = 0;
  virtual GlobalEditResult importOverlayType(core::OverlayType type) = 0;
  virtual GlobalEditResult importOverlayMaxMb(uint32_t value) = 0;
  virtual GlobalEditResult importOverlayWarnMb(uint32_t value) = 0;
  virtual GlobalEditResult importOverlayCritMb(uint32_t value) = 0;
  virtual void finishImport() = 0;
};

class VolumeConfigurationEditing {
 public:
  virtual ~VolumeConfigurationEditing() = default;
  [[nodiscard]] virtual QString driveLetter() const = 0;
  [[nodiscard]] virtual QString volumeIdentity() const = 0;
  [[nodiscard]] virtual QString configurationVolumeId() const = 0;
  [[nodiscard]] virtual bool protectionEditingAllowed() const = 0;
  [[nodiscard]] virtual bool editingAllowed() const = 0;
  // A new registration retains the command selector, including GUID creation semantics.
  virtual bool importProtect(bool value, const QString& target = {}) = 0;
  virtual ExclusionImportOutcome importAddFileExclusion(const QString& path) = 0;
  virtual ExclusionImportOutcome importRemoveFileExclusion(const QString& path) = 0;
};

// Resolve each command independently and keep its target alive through callbacks.
using VolumeConfigurationEditors = std::vector<std::weak_ptr<VolumeConfigurationEditing>>;

}  // namespace uwf::app
