// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QString>
#include <utility>

#include "app/api/FileStagingTypes.h"

namespace uwf::app {

[[nodiscard]] bool isAbsoluteLocalFileStagingPath(const QString& path);
[[nodiscard]] std::pair<FileStagingKind, QString> fileStagingEntryKey(const FileStagingEntry& entry);

}  // namespace uwf::app
