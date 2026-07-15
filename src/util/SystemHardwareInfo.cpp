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
#include "SystemHardwareInfo.h"

#include <dxgi1_2.h>
#include <windows.h>

#include <limits>
#include <optional>
#include <string>

#include "../uwf/wmi/WmiClient.h"
#include "../uwf/wmi/WmiRowUtil.h"
#include "Log.h"
#include "RegistryKey.h"
#include "StringUtil.h"

namespace uwf {

namespace {

bool readNullableUInt(const WmiRow& row, const char* field, std::optional<std::uint32_t>& value, std::string* error) {
  value.reset();
  const auto it = row.find(field);
  if (it == row.end()) {
    if (error) *error = std::string("WMI field '") + field + "' is missing";
    return false;
  }
  if (!it->second.isValid()) return true;
  const auto decoded = rowutil::requireUInt(row, field, error);
  if (!decoded) return false;
  value = *decoded;
  return true;
}

template <typename T>
class ComPtr final {
 public:
  ComPtr() = default;
  ~ComPtr() {
    if (m_ptr) m_ptr->Release();
  }
  ComPtr(const ComPtr&) = delete;
  ComPtr& operator=(const ComPtr&) = delete;

  [[nodiscard]] T* get() const { return m_ptr; }
  [[nodiscard]] T** put() {
    if (m_ptr) {
      m_ptr->Release();
      m_ptr = nullptr;
    }
    return &m_ptr;
  }
  [[nodiscard]] T* operator->() const { return m_ptr; }

 private:
  T* m_ptr = nullptr;
};

std::string cpuModel() { return regkey::readString(R"(HKEY_LOCAL_MACHINE\HARDWARE\DESCRIPTION\System\CentralProcessor\0)", "ProcessorNameString"); }

std::wstring primaryDisplayDeviceName() {
  const HMONITOR monitor = MonitorFromPoint(POINT{0, 0}, MONITOR_DEFAULTTOPRIMARY);
  MONITORINFOEXW info{};
  info.cbSize = sizeof(info);
  if (!monitor || !GetMonitorInfoW(monitor, &info)) return {};
  return info.szDevice;
}

GraphicsAdapterInfo fallbackGraphicsAdapter(const std::wstring& primaryDeviceName) {
  DISPLAY_DEVICEW device{};
  device.cb = sizeof(device);
  GraphicsAdapterInfo firstAttached;
  for (DWORD index = 0; EnumDisplayDevicesW(nullptr, index, &device, 0); ++index) {
    if ((device.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP) != 0) {
      GraphicsAdapterInfo candidate{wideToUtf8(device.DeviceString), 0};
      if (firstAttached.name.empty()) firstAttached = candidate;
      if (!primaryDeviceName.empty() && primaryDeviceName == device.DeviceName) return candidate;
    }
    device = {};
    device.cb = sizeof(device);
  }
  return firstAttached;
}

bool adapterDrivesDisplay(IDXGIAdapter1* adapter, const std::wstring& displayDeviceName) {
  if (!adapter || displayDeviceName.empty()) return false;
  for (UINT index = 0;; ++index) {
    ComPtr<IDXGIOutput> output;
    const HRESULT enumerateResult = adapter->EnumOutputs(index, output.put());
    if (enumerateResult == DXGI_ERROR_NOT_FOUND) break;
    if (FAILED(enumerateResult)) break;

    DXGI_OUTPUT_DESC description{};
    if (SUCCEEDED(output->GetDesc(&description)) && description.AttachedToDesktop && displayDeviceName == description.DeviceName) return true;
  }
  return false;
}

GraphicsAdapterInfo graphicsAdapter() {
  const std::wstring primaryDeviceName = primaryDisplayDeviceName();
  ComPtr<IDXGIFactory1> factory;
  if (FAILED(CreateDXGIFactory1(IID_IDXGIFactory1, reinterpret_cast<void**>(factory.put())))) return fallbackGraphicsAdapter(primaryDeviceName);

  for (UINT index = 0;; ++index) {
    ComPtr<IDXGIAdapter1> adapter;
    const HRESULT enumerateResult = factory->EnumAdapters1(index, adapter.put());
    if (enumerateResult == DXGI_ERROR_NOT_FOUND) break;
    if (FAILED(enumerateResult)) break;

    DXGI_ADAPTER_DESC1 description{};
    if (FAILED(adapter->GetDesc1(&description)) || (description.Flags & static_cast<UINT>(DXGI_ADAPTER_FLAG_SOFTWARE | DXGI_ADAPTER_FLAG_REMOTE)) != 0) {
      continue;
    }

    if (adapterDrivesDisplay(adapter.get(), primaryDeviceName)) {
      return {wideToUtf8(description.Description), static_cast<std::uint64_t>(description.DedicatedVideoMemory)};
    }
  }
  return fallbackGraphicsAdapter(primaryDeviceName);
}

std::uint64_t visibleMemoryBytes() {
  MEMORYSTATUSEX memory{};
  memory.dwLength = sizeof(memory);
  return GlobalMemoryStatusEx(&memory) ? memory.ullTotalPhys : 0;
}

void queryPhysicalMemory(SystemHardwareInfo& info) {
  auto& session = cimv2WmiSession();
  std::string error;
  if (!session.ensureConnected(&error)) {
    UWF_LOG_W("hardware") << "physical memory WMI connection unavailable: " << error;
    return;
  }

  const auto modules = session.query("SELECT Capacity, ConfiguredClockSpeed, SMBIOSMemoryType FROM Win32_PhysicalMemory", &error);
  if (!error.empty()) {
    UWF_LOG_W("hardware") << "physical memory module query failed: " << error;
    return;
  }

  std::vector<PhysicalMemoryModuleInfo> decodedModules;
  decodedModules.reserve(modules.size());
  for (const WmiRow& row : modules) {
    const auto capacity = rowutil::requireUInt64(row, "Capacity", &error);
    std::optional<std::uint32_t> speed;
    std::optional<std::uint32_t> type;
    if (!capacity || !readNullableUInt(row, "ConfiguredClockSpeed", speed, &error) || !readNullableUInt(row, "SMBIOSMemoryType", type, &error)) {
      UWF_LOG_W("hardware") << "physical memory module response is incomplete: " << error;
      return;
    }
    PhysicalMemoryModuleInfo module;
    module.capacityBytes = *capacity;
    // WMI 沿用 ConfiguredClockSpeed 名称，但 SMBIOS 在现代 DDR 设备上返回的是
    // 有效传输速率（例如 DDR5-6000 返回 6000），展示统一使用 MT/s。
    module.configuredSpeedMtPerSecond = speed.value_or(0);
    module.memoryType = static_cast<SmbiosMemoryType>(type.value_or(0));
    if (module.capacityBytes != 0) decodedModules.push_back(module);
  }

  // 模块信息与物理阵列的插槽数是独立的展示字段。后续插槽查询失败
  // 不能把已经完整解码的模块信息一并丢掉。
  info.memoryModules = std::move(decodedModules);

  const auto arrays = session.query("SELECT MemoryDevices FROM Win32_PhysicalMemoryArray WHERE Use = 3", &error);
  if (!error.empty()) {
    UWF_LOG_W("hardware") << "physical memory array query failed: " << error;
    return;
  }
  std::uint64_t totalSlots = 0;
  bool allSlotCountsKnown = true;
  for (const WmiRow& row : arrays) {
    std::optional<std::uint32_t> slots;
    if (!readNullableUInt(row, "MemoryDevices", slots, &error)) {
      UWF_LOG_W("hardware") << "physical memory array response is invalid: " << error;
      return;
    }
    if (!slots) {
      allSlotCountsKnown = false;
      continue;
    }
    if (totalSlots > std::numeric_limits<std::uint32_t>::max() - *slots) {
      UWF_LOG_W("hardware") << "physical memory array response is invalid: " << (error.empty() ? "slot count overflow" : error);
      return;
    }
    totalSlots += *slots;
  }
  if (allSlotCountsKnown) info.totalMemorySlots = static_cast<std::uint32_t>(totalSlots);
}

SystemHardwareInfo querySystemHardwareInfo() {
  SystemHardwareInfo info;
  info.cpuModel = cpuModel();
  info.graphicsAdapter = graphicsAdapter();
  queryPhysicalMemory(info);
  std::uint64_t installedMemoryBytes = 0;
  for (const PhysicalMemoryModuleInfo& module : info.memoryModules) {
    if (module.capacityBytes > std::numeric_limits<std::uint64_t>::max() - installedMemoryBytes) {
      installedMemoryBytes = 0;
      break;
    }
    installedMemoryBytes += module.capacityBytes;
  }
  info.totalMemoryBytes = installedMemoryBytes != 0 ? installedMemoryBytes : visibleMemoryBytes();
  return info;
}

}  // namespace

const SystemHardwareInfo& systemHardwareInfo() {
  static const SystemHardwareInfo info = querySystemHardwareInfo();
  return info;
}

std::string_view physicalMemoryTypeName(const SmbiosMemoryType type) {
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

}  // namespace uwf
