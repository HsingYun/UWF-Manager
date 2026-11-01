// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QList>
#include <functional>
#include <string>

#include "application/configuration/contracts/ConfigurationEditing.h"
#include "application/configuration/importing/ImportReport.h"

namespace uwf::app {
class ExclusionEditor;
class OverlayPolicyEditor;
using DriveResolver = std::function<std::string(const std::string&)>;
using AllVolumeProtectionImporter = std::function<ExclusionImportOutcome(bool)>;
// Runtime maintenance has its own Current capability/admission rules. This
// callback stages an explicit intent; importing never executes the command.
using MaintenanceCommandImporter = std::function<ExclusionImportOutcome(const UwfmgrCommand&)>;

[[nodiscard]] QList<ImportResult> applyImportCommands(const QList<app::UwfmgrCommand>& commands, GlobalConfigurationEditing* global,
                                                      const VolumeConfigurationEditors& volumes, ExclusionEditor* registry, const DriveResolver& resolveDrive,
                                                      OverlayPolicyEditor* policy = nullptr, const AllVolumeProtectionImporter& stageAll = {},
                                                      const MaintenanceCommandImporter& stageMaintenance = {});

}  // namespace uwf::app
