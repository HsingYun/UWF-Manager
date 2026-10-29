// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QList>

#include "ConfigurationEditing.h"
#include "ImportReport.h"

namespace uwf::app {
class ExclusionEditor;

[[nodiscard]] QList<ImportResult> applyImportCommands(const QList<app::UwfmgrCommand>& commands, GlobalConfigurationEditing* global,
                                                      const VolumeConfigurationEditors& volumes, ExclusionEditor* registry);

}  // namespace uwf::app
