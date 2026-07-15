/*
 * Copyright (c) 2026 HsingYun (iakext@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

// ┌──────────────────────────────────────────────────────────────────────────┐
// │ uwf::config —— 全项目共用的「领域常量」集中地。                            │
// │                                                                          │
// │ 这里只收录 UWF 协议规则与 Windows 事实性常量——文件 / 注册表排除的黑白    │
// │ 名单、受支持的 Windows 版本关键字、WMI 命名空间、覆盖层尺寸下限、文件     │
// │ 系统支持判定等。散落在各层时容易各写一份、改一处漏一处，集中到此单一     │
// │ 来源。                                                                   │
// │                                                                          │
// │ 不收录：UI 表现值（像素 / 颜色 / 几何）、单位换算因子，以及单一调用方的   │
// │ 实现调优常量（日志环形缓冲容量、导入文件上限等）——那些与各自模块强      │
// │ 耦合，留在原处局部性更好。                                               │
// │                                                                          │
// │ 本文件 **不依赖 Qt、不包含 Windows API 头**，只有纯数据，可被任意层      │
// │ include。注意与同目录 UwfModel.h 区分：后者是数据「结构」，本文件是      │
// │ 「常量」。                                                               │
// └──────────────────────────────────────────────────────────────────────────┘

#include <array>
#include <cstdint>
#include <string_view>

namespace uwf::config {

// ── WMI 命名空间 ────────────────────────────────────────────────────────────
// thread_local WMI 上下文中两个固定 session 使用的 namespace。

// UWF 的 UWF_* 类所在命名空间。
inline constexpr const char* kWmiNamespaceEmbedded = "root\\standardcimv2\\embedded";
// 标准 Windows CIM 命名空间，Win32_*（逻辑磁盘 / 卷）所在。
inline constexpr const char* kWmiNamespaceCimv2 = "root\\cimv2";

// ── Windows 版本与版本号 ────────────────────────────────────────────────────

// Windows 11 正式客户端从 Build 22000 起；Win10 / Win11 共享 Major=10，
// 对 VER_NT_WORKSTATION 系统只能靠 RtlGetVersion 返回的 Build 区分。
inline constexpr int kWindows11MinBuildNumber = 22000;

// Windows 11 的注册表 ProductName 至今仍写作 "Windows 10 …"——微软从未更新，
// 需把名称里的旧 token 替换成新 token。
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

// ── 磁盘与文件系统 ──────────────────────────────────────────────────────────

// Win32_LogicalDisk.DriveType：3 = 固定本地磁盘，UWF 仅支持保护这一类。
inline constexpr int kDriveTypeFixedLocalDisk = 3;

// UWF 单个受保护卷的容量上限（MS Learn UWF "保护范围"段：单个受保护卷最大
// 16 TB）。按 16 TiB（= 16 × 2^40）解释——NTFS 默认簇大小的最大卷也是这个量级，
// UWF 作为 NTFS 之上的过滤驱动几乎肯定继承该上限。即便官方实际指的是十进制 TB
// （16 × 10^12），用 TiB 也只是稍严，不会放过不该放过的卷。
inline constexpr uint64_t kMaxProtectedVolumeBytes = 16ULL << 40;

// UWF 完整支持（可加文件排除 + 提交文件）的文件系统。大写存储，调用方先转
// 大写再比对；不在此列的固定盘（exFAT / ReFS 等）只能保护、不能加文件排除。
inline constexpr std::array<std::string_view, 3> kFullySupportedFileSystems = {
    "NTFS",
    "FAT",
    "FAT32",
};

// ── 覆盖层 ──────────────────────────────────────────────────────────────────

// 基于磁盘的覆盖层，UWF 要求最大大小至少 1024 MB（见 UWF_OverlayConfig 的
// SetMaximumSize / SetType 生效条件）；RAM 覆盖层无此下限。
inline constexpr uint32_t kDiskOverlayMinSizeMb = 1024;

// ── 文件排除黑名单 ──────────────────────────────────────────────────────────
// 全部为大写、卷内相对路径（以反斜杠开头、不含盘符）。调用方先把待校验路径
// 规整成同样形态再比对。UWF 文档明确禁止排除以下路径。

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

// ── 注册表排除黑 / 白名单 ───────────────────────────────────────────────────
// 全部为大写、长写 hive 形式（HKEY_LOCAL_MACHINE\…）。调用方先归一再比对。

// UWF 只允许排除以下 6 个顶层键的「子键」（顶层键本身不行，故前缀以反斜杠
// 结尾，匹配时其后还须至少有一个字符）。
inline constexpr std::array<std::string_view, 6> kAllowedRegistryRootPrefixes = {
    R"(HKEY_LOCAL_MACHINE\BCD00000000\)", R"(HKEY_LOCAL_MACHINE\SYSTEM\)",   R"(HKEY_LOCAL_MACHINE\SOFTWARE\)",
    R"(HKEY_LOCAL_MACHINE\SAM\)",         R"(HKEY_LOCAL_MACHINE\SECURITY\)", R"(HKEY_LOCAL_MACHINE\COMPONENTS\)",
};

// 域机器账户密钥——它在 SECURITY 下本可通过白名单，但 UWF 文档明确禁止排除，
// 须在白名单检查之前单独挡掉。
inline constexpr std::string_view kForbiddenRegistryKeyMachineAccount = R"(HKEY_LOCAL_MACHINE\SECURITY\POLICY\SECRETS\$MACHINE.ACC)";

// ── 注册表名字长度上限 ──────────────────────────────────────────────────────
// Windows 注册表键名上限 255 字符、值名上限 16383 字符（RegCreateKeyEx /
// RegSetValueEx 文档）。枚举子键 / 值名时按「上限 + 1（收尾 NUL）」开缓冲。
inline constexpr int kRegistryKeyNameBufChars = 256;
inline constexpr int kRegistryValueNameBufChars = 16384;

}  // namespace uwf::config
