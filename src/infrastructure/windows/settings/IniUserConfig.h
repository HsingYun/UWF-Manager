// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include "application/settings/UserConfig.h"

namespace uwf::windows {
// applicationDirectory must be absolute. Reads UWF.ini without modifying it;
// missing or unreadable storage uses defaults.
[[nodiscard]] app::UserConfig readIniUserConfig(const QString& applicationDirectory);
}  // namespace uwf::windows
