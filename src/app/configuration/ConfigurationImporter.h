// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QList>
#include <functional>
#include <string>

#include "ConfigurationEditing.h"
#include "ImportReport.h"

namespace uwf::app {
class ExclusionEditor;
using DriveResolver = std::function<std::string(const std::string&)>;

[[nodiscard]] QList<ImportResult> applyImportCommands(const QList<app::UwfmgrCommand>& commands, GlobalConfigurationEditing* global,
                                                      const VolumeConfigurationEditors& volumes, ExclusionEditor* registry,
                                                      const DriveResolver& resolveDrive);

}  // namespace uwf::app
