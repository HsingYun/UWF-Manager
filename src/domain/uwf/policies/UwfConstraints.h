// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <array>
#include <cstdint>
#include <string_view>

#include "domain/uwf/model/UwfModel.h"

namespace uwf::core {

inline constexpr uint64_t kMaxProtectedVolumeBytes = 16ULL << 40;

// File exclusions and commits require these file systems; compare uppercase names.
inline constexpr std::array<std::string_view, 3> kFullySupportedFileSystems = {
    "NTFS",
    "FAT",
    "FAT32",
};

// The CLI's explicit type/size commands require Disk sizes of at least 1024 MB.
inline constexpr uint32_t kDiskOverlayMinSizeMb = 1024;
// Native disk sizing sign-extends the DWORD before converting MB to bytes.
inline constexpr uint32_t kDiskOverlayMaxSizeMb = 0x7fffffffU;
inline constexpr uint32_t kRamOverlayMinSizeMb = 1;
[[nodiscard]] constexpr uint32_t minimumOverlaySizeMb(const OverlayType type) {
  return type == OverlayType::Disk ? kDiskOverlayMinSizeMb : kRamOverlayMinSizeMb;
}
// Invalid observed storage remains readable; edits must satisfy the writable domain.
[[nodiscard]] constexpr bool validOverlayTarget(const OverlayType type, const uint32_t sizeMb) {
  return (type == OverlayType::RAM || type == OverlayType::Disk) && sizeMb >= minimumOverlaySizeMb(type) &&
         (type != OverlayType::Disk || sizeMb <= kDiskOverlayMaxSizeMb);
}

// Uppercase, volume-relative paths reserved on every volume.

inline constexpr std::array<std::string_view, 3> kForbiddenVolumeRootFiles = {
    R"(\PAGEFILE.SYS)",
    R"(\SWAPFILE.SYS)",
    R"(\HIBERFIL.SYS)",
};

// Reserved directories on the system volume; individual descendants may be excluded.
inline constexpr std::string_view kForbiddenDirWindows = R"(\WINDOWS)";
inline constexpr std::string_view kForbiddenDirWindowsSystem32 = R"(\WINDOWS\SYSTEM32)";
inline constexpr std::string_view kForbiddenDirWindowsDrivers = R"(\WINDOWS\SYSTEM32\DRIVERS)";

// Reserved registry hives and boot-state files on the system volume.
inline constexpr std::array<std::string_view, 8> kForbiddenSystemFiles = {
    R"(\WINDOWS\SYSTEM32\CONFIG\DEFAULT)", R"(\WINDOWS\SYSTEM32\CONFIG\SAM)", R"(\WINDOWS\SYSTEM32\CONFIG\SECURITY)", R"(\WINDOWS\SYSTEM32\CONFIG\SOFTWARE)",
    R"(\WINDOWS\SYSTEM32\CONFIG\SYSTEM)",  R"(\WINDOWS\BOOTSTAT.DAT)",        R"(\EFI\MICROSOFT\BOOT\BOOTSTAT.DAT)",  R"(\BOOT\BOOTSTAT.DAT)",
};

// Reserved per-user hive: \USERS\<one component>\NTUSER.DAT.
inline constexpr std::string_view kUsersDirName = "USERS";
inline constexpr std::string_view kPerUserRegistryHive = "NTUSER.DAT";

// Add rejects the HKLM root and these exact targets, not their descendants.
inline constexpr std::string_view kRegistryExclusionHive = "HKEY_LOCAL_MACHINE";
inline constexpr std::array<std::string_view, 6> kReservedRegistryExclusionKeys = {
    R"(HKEY_LOCAL_MACHINE\SYSTEM)",   R"(HKEY_LOCAL_MACHINE\SYSTEM\CURRENTCONTROLSET)",
    R"(HKEY_LOCAL_MACHINE\SOFTWARE)", R"(HKEY_LOCAL_MACHINE\SAM)",
    R"(HKEY_LOCAL_MACHINE\SECURITY)", R"(HKEY_LOCAL_MACHINE\COMPONENTS)",
};

// The editor uses the dedicated persistence setting for this key; native Add does not reserve it.
inline constexpr std::string_view kRegistryKeyMachineAccount = R"(HKEY_LOCAL_MACHINE\SECURITY\POLICY\SECRETS\$MACHINE.ACC)";

}  // namespace uwf::core
