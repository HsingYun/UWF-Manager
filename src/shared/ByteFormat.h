// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

// 字节及 WMI MB 数值的统一显示格式，不依赖 Qt。

#include <cstdint>
#include <string>

namespace uwf {

// 字节使用不超过当前值的最大单位；KB/MB 保留一位小数，GB/TB 保留两位。
[[nodiscard]] std::string formatBytes(uint64_t bytes);

// MB 值选择紧凑单位；可整除时显示整数，否则保留两位小数。
[[nodiscard]] std::string formatMb(uint32_t mb);

}  // namespace uwf
