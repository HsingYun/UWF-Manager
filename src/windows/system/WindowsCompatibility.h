// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <array>
#include <string_view>

namespace uwf::windows {

// Windows 11 正式客户端从 Build 22000 起；Win10 / Win11 共享 Major=10，
// 对 VER_NT_WORKSTATION 系统只能靠 RtlGetVersion 返回的 Build 区分。
inline constexpr int kWindows11MinBuildNumber = 22000;

// 部分 Windows 11 系统的 ProductName 仍含 Windows 10；仅在确认系统版本后修正展示名称。
inline constexpr std::string_view kProductNameWin10Token = "Windows 10";
inline constexpr std::string_view kProductNameWin11Token = "Windows 11";

// 注册表中 OS 展示信息（ProductName / EditionID / DisplayVersion / UBR）所在项
// 的完整键路径（含 hive）。系统家族与 Build 不从这里判断。
inline constexpr std::string_view kRegPathWindowsCurrentVersion = "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion";

// 支持 UWF 的 Windows 版本——EditionID 里只要（不分大小写）包含其中任一关键字
// 即视为受支持。小写存储，调用方先把 EditionID 转小写再做子串匹配。
inline constexpr std::array<std::string_view, 4> kSupportedEditionKeywords = {
    "enterprise",
    "education",
    "iotenterprise",
    "enterpriseg",
};

// LTSC / LTSB 变体的 EditionID（小写、精确匹配）。ProductName 不一定写出
// "LTSC" 字样，命中这些 EditionID 时需自行补上。
inline constexpr std::array<std::string_view, 4> kLtscEditionIds = {
    "enterprises",
    "enterprisesn",
    "iotenterprises",
    "iotenterprisesn",
};

}  // namespace uwf::windows
