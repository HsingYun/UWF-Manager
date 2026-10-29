// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

// ASCII 大小写转换与首尾空白裁剪。

#include <string>
#include <string_view>

namespace uwf {

[[nodiscard]] bool isAsciiWhitespace(char value);

// ASCII 大小写折叠：仅 A-Z / a-z 受影响，非 ASCII 字节原样保留。
[[nodiscard]] std::string toLowerAscii(std::string s);
[[nodiscard]] std::string toUpperAscii(std::string s);

// 去掉首尾的 ASCII 空白字符，返回裁剪后的副本。
[[nodiscard]] std::string trim(std::string s);

}  // namespace uwf
