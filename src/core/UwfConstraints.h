// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

// 共享 UWF 协议与 Windows 领域常量；不依赖 Qt 或平台头。
// UI 表现值和单模块调优常量保留在所属模块。

#include <array>
#include <cstdint>
#include <string_view>

namespace uwf::core {

// Win32_LogicalDisk.DriveType：3 = 固定本地磁盘，UWF 仅支持保护这一类。
inline constexpr int kDriveTypeFixedLocalDisk = 3;

// 卷容量检查使用 16 TiB 上限。
inline constexpr uint64_t kMaxProtectedVolumeBytes = 16ULL << 40;

// UWF 完整支持（可加文件排除 + 提交文件）的文件系统。大写存储，调用方先转
// 大写再比对；不在此列的固定盘（exFAT / ReFS 等）只能保护、不能加文件排除。
inline constexpr std::array<std::string_view, 3> kFullySupportedFileSystems = {
    "NTFS",
    "FAT",
    "FAT32",
};

// 基于磁盘的覆盖层，UWF 要求最大大小至少 1024 MB（见 UWF_OverlayConfig 的
// SetMaximumSize / SetType 生效条件）；RAM 覆盖层无此下限。
inline constexpr uint32_t kDiskOverlayMinSizeMb = 1024;

// 排除路径使用大写卷内相对形式，不含盘符。

// 任何卷都禁止排除的卷根系统文件（分页 / 交换 / 休眠文件）——UWF 自身依赖它们。
inline constexpr std::array<std::string_view, 3> kForbiddenVolumeRootFiles = {
    R"(\PAGEFILE.SYS)",
    R"(\SWAPFILE.SYS)",
    R"(\HIBERFIL.SYS)",
};

// 系统卷上禁止整体排除的关键目录（目录内的具体文件仍可排除）。三条各自对应
// 一句不同的用户提示，故分开命名而非数组。
inline constexpr std::string_view kForbiddenDirWindows = R"(\WINDOWS)";
inline constexpr std::string_view kForbiddenDirWindowsSystem32 = R"(\WINDOWS\SYSTEM32)";
inline constexpr std::string_view kForbiddenDirWindowsDrivers = R"(\WINDOWS\SYSTEM32\DRIVERS)";

// 系统卷上禁止排除的关键系统文件（注册表蜂巢 + 引导统计文件）。
inline constexpr std::array<std::string_view, 8> kForbiddenSystemFiles = {
    R"(\WINDOWS\SYSTEM32\CONFIG\DEFAULT)", R"(\WINDOWS\SYSTEM32\CONFIG\SAM)", R"(\WINDOWS\SYSTEM32\CONFIG\SECURITY)", R"(\WINDOWS\SYSTEM32\CONFIG\SOFTWARE)",
    R"(\WINDOWS\SYSTEM32\CONFIG\SYSTEM)",  R"(\WINDOWS\BOOTSTAT.DAT)",        R"(\EFI\MICROSOFT\BOOT\BOOTSTAT.DAT)",  R"(\BOOT\BOOTSTAT.DAT)",
};

// 每用户注册表蜂巢路径 \USERS\<用户名>\NTUSER.DAT 的两个固定段名（大写）。
// 用户名任意但只有一层，校验时按这两个段名匹配三段式路径。
inline constexpr std::string_view kUsersDirName = "USERS";
inline constexpr std::string_view kPerUserRegistryHive = "NTUSER.DAT";

// 注册表规则使用大写、完整 hive 名。

// UWF 只允许排除以下 6 个顶层键的「子键」（顶层键本身不行，故前缀以反斜杠
// 结尾，匹配时其后还须至少有一个字符）。
inline constexpr std::array<std::string_view, 6> kAllowedRegistryRootPrefixes = {
    R"(HKEY_LOCAL_MACHINE\BCD00000000\)", R"(HKEY_LOCAL_MACHINE\SYSTEM\)",   R"(HKEY_LOCAL_MACHINE\SOFTWARE\)",
    R"(HKEY_LOCAL_MACHINE\SAM\)",         R"(HKEY_LOCAL_MACHINE\SECURITY\)", R"(HKEY_LOCAL_MACHINE\COMPONENTS\)",
};

// $MACHINE.ACC 位于允许的 SECURITY 子树内，须优先执行禁止规则。
inline constexpr std::string_view kForbiddenRegistryKeyMachineAccount = R"(HKEY_LOCAL_MACHINE\SECURITY\POLICY\SECRETS\$MACHINE.ACC)";

}  // namespace uwf::core
