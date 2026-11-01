// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <cstdint>
#include <string>

namespace uwf {

// Use binary units: one decimal place for KB/MB, two for GB/TB.
[[nodiscard]] std::string formatBytes(uint64_t bytes);

// Scale binary MB to compact units; exact multiples use integer values.
[[nodiscard]] std::string formatMb(uint32_t mb);

}  // namespace uwf
