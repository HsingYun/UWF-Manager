// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <string>
#include <string_view>

namespace uwf::windows {

// Strict UTF-8/UTF-16 conversion at Win32 boundaries. Empty input remains empty;
// invalid encoding, oversized input and conversion failures throw.
[[nodiscard]] std::wstring utf8ToWide(std::string_view utf8);

[[nodiscard]] std::string wideToUtf8(std::wstring_view wide);

// Reads a NUL-terminated string; nullptr produces an empty string.
[[nodiscard]] std::string wideToUtf8(const wchar_t* wide);

}  // namespace uwf::windows
