// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QList>
#include <QString>
#include <string>
#include <vector>

#include "application/staging/contracts/FileStagingTypes.h"

namespace uwf::windows {

// REG_MULTI_SZ records use F| or D| followed by the literal full path.
[[nodiscard]] std::vector<std::string> encodeFileStagingEntries(const QList<app::FileStagingEntry>& entries);
[[nodiscard]] QList<app::FileStagingEntry> decodeFileStagingEntries(const std::vector<std::string>& records);

}  // namespace uwf::windows
