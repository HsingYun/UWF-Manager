// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <array>
#include <string_view>

namespace uwf::windows {

// Windows 10 and 11 clients share major version 10; build 22000 starts Windows 11.
inline constexpr int kWindows11MinBuildNumber = 22000;

// Some Windows 11 registry ProductName values still identify Windows 10.
inline constexpr std::string_view kProductNameWin10Token = "Windows 10";
inline constexpr std::string_view kProductNameWin11Token = "Windows 11";

// Registry display/edition metadata; RtlGetVersion determines family and build.
inline constexpr std::string_view kRegPathWindowsCurrentVersion = "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion";

// Lowercase substrings used for the startup edition-support check.
inline constexpr std::array<std::string_view, 4> kSupportedEditionKeywords = {
    "enterprise",
    "education",
    "iotenterprise",
    "enterpriseg",
};

// Exact lowercase LTSC/LTSB edition IDs whose ProductName may omit that label.
inline constexpr std::array<std::string_view, 4> kLtscEditionIds = {
    "enterprises",
    "enterprisesn",
    "iotenterprises",
    "iotenterprisesn",
};

}  // namespace uwf::windows
