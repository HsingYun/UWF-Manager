// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace uwf::app {

enum class WindowsFamily {
  Unknown,
  Windows10,
  Windows11,
  WindowsServer,
};

struct WindowsVersionInfo {
  WindowsFamily family = WindowsFamily::Unknown;
  std::uint32_t major = 0;
  std::uint32_t minor = 0;
  std::uint32_t build = 0;
  std::uint32_t revision = 0;
  std::string productName;     // 已按真实家族修正的展示名
  std::string editionId;       // 注册表 EditionID
  std::string displayVersion;  // 例如 22H2 / 24H2；旧系统回退到 ReleaseId
  bool longTermServicing = false;
};

enum class SmbiosMemoryType : std::uint32_t {
  Unknown = 0,
  DDR = 20,
  DDR2 = 21,
  DDR3 = 24,
  DDR4 = 26,
  LPDDR = 27,
  LPDDR2 = 28,
  LPDDR3 = 29,
  LPDDR4 = 30,
  HBM = 32,
  HBM2 = 33,
  DDR5 = 34,
  LPDDR5 = 35,
  HBM3 = 36,
};

struct PhysicalMemoryModuleInfo {
  std::uint64_t capacityBytes = 0;
  std::uint32_t configuredSpeedMtPerSecond = 0;
  SmbiosMemoryType memoryType = SmbiosMemoryType::Unknown;
};

struct GraphicsAdapterInfo {
  std::string name;
  std::uint64_t dedicatedVideoMemoryBytes = 0;
};

struct SystemHardwareInfo {
  std::string cpuModel;
  GraphicsAdapterInfo graphicsAdapter;
  std::uint64_t totalMemoryBytes = 0;
  std::vector<PhysicalMemoryModuleInfo> memoryModules;
  std::uint32_t totalMemorySlots = 0;
};

inline constexpr std::string_view physicalMemoryTypeName(const SmbiosMemoryType type) {
  switch (type) {
    case SmbiosMemoryType::DDR:
      return "DDR";
    case SmbiosMemoryType::DDR2:
      return "DDR2";
    case SmbiosMemoryType::DDR3:
      return "DDR3";
    case SmbiosMemoryType::DDR4:
      return "DDR4";
    case SmbiosMemoryType::LPDDR:
      return "LPDDR";
    case SmbiosMemoryType::LPDDR2:
      return "LPDDR2";
    case SmbiosMemoryType::LPDDR3:
      return "LPDDR3";
    case SmbiosMemoryType::LPDDR4:
      return "LPDDR4";
    case SmbiosMemoryType::HBM:
      return "HBM";
    case SmbiosMemoryType::HBM2:
      return "HBM2";
    case SmbiosMemoryType::DDR5:
      return "DDR5";
    case SmbiosMemoryType::LPDDR5:
      return "LPDDR5";
    case SmbiosMemoryType::HBM3:
      return "HBM3";
    case SmbiosMemoryType::Unknown:
      return {};
  }
  return {};
}

}  // namespace uwf::app
