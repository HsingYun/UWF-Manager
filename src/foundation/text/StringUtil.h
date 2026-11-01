// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <string>
#include <string_view>

namespace uwf {

[[nodiscard]] bool isAsciiWhitespace(char value);

// Only ASCII letters change; non-ASCII bytes are preserved.
[[nodiscard]] std::string toLowerAscii(std::string s);
[[nodiscard]] std::u16string toLowerAscii(std::u16string s);
[[nodiscard]] std::string toUpperAscii(std::string s);
// Locale-independent ASCII comparison; non-ASCII bytes must match exactly.
[[nodiscard]] bool equalAsciiInsensitive(std::string_view left, std::string_view right);
[[nodiscard]] bool equalAsciiInsensitive(std::u16string_view left, std::u16string_view right);

// Trim ASCII whitespace only.
[[nodiscard]] std::string trim(std::string s);

}  // namespace uwf
