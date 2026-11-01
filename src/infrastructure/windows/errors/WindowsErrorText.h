// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QString>
#include <cstdint>

namespace uwf::windows {
[[nodiscard]] QString windowsErrorText(uint32_t code);
}  // namespace uwf::windows
