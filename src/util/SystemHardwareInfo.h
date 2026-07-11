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

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace uwf {

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

// 硬件在进程生命周期内视为不变，首次调用后缓存。CPU/GPU/总内存来自 Win32，
// 内存条和插槽来自 root\CIMV2 的 SMBIOS-backed WMI 类；查询失败时保留已取得
// 的字段，其余字段为 0/空，不把展示或产品策略混入数据层。
[[nodiscard]] const SystemHardwareInfo& systemHardwareInfo();

// SMBIOS Type 17 Memory Type 数值到稳定英文硬件名称；未知值返回空。
[[nodiscard]] std::string_view physicalMemoryTypeName(SmbiosMemoryType type);

}  // namespace uwf
