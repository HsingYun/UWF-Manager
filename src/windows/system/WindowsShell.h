// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QString>

namespace uwf::windows::shell {

void revealInExplorer(const QString& path);
[[nodiscard]] QString extractDriveLetter(const QString& path);

}  // namespace uwf::windows::shell
