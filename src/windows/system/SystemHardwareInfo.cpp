// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "SystemHardwareInfo.h"

#include <dxgi1_2.h>
#include <windows.h>

#include <limits>
#include <optional>
#include <string>

#include "app/ports/RegistryKey.h"
#include "app/wmi/WmiException.h"
#include "app/wmi/WmiRowUtil.h"
#include "shared/Log.h"
#include "shared/StringUtil.h"
#include "windows/resources/ComPtr.h"
#include "windows/system/UnicodeConversion.h"
#include "windows/wmi/WmiSession.h"

namespace uwf::windows {

uint32_t systemTotalRamMb() {
  MEMORYSTATUSEX state{};
  state.dwLength = sizeof(state);
  if (GlobalMemoryStatusEx(&state)) return static_cast<uint32_t>(state.ullTotalPhys / (1024ULL * 1024ULL));
  return 0;
}

namespace {

std::optional<std::uint32_t> readNullableUInt(const app::wmi::WmiRow& row, const char* field) {
  const auto it = row.find(field);
  if (it == row.end()) throw app::wmi::WmiDecodeError("decode physical memory information", std::string("field '") + field + "' is missing");
  if (!it->second.isValid()) return std::nullopt;
  return app::wmi::row::requireUInt(row, field);
}

std::string cpuModel() {
  try {
    return regkey::readString(R"(HKEY_LOCAL_MACHINE\HARDWARE\DESCRIPTION\System\CentralProcessor\0)", "ProcessorNameString").value_or(std::string{});
  } catch (const std::exception& error) {
    UWF_LOG_D("hardware") << "CPU model unavailable: error=" << error.what();
    return {};
  }
}

std::wstring primaryDisplayDeviceName() {
  const HMONITOR monitor = MonitorFromPoint(POINT{0, 0}, MONITOR_DEFAULTTOPRIMARY);
  MONITORINFOEXW info{};
  info.cbSize = sizeof(info);
  if (!monitor || !GetMonitorInfoW(monitor, &info)) return {};
  return info.szDevice;
}

app::GraphicsAdapterInfo fallbackGraphicsAdapter(const std::wstring& primaryDeviceName) {
  DISPLAY_DEVICEW device{};
  device.cb = sizeof(device);
  app::GraphicsAdapterInfo firstAttached;
  for (DWORD index = 0; EnumDisplayDevicesW(nullptr, index, &device, 0); ++index) {
    if ((device.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP) != 0) {
      app::GraphicsAdapterInfo candidate{wideToUtf8(device.DeviceString), 0};
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
    windows::ComPtr<IDXGIOutput> output;
    const HRESULT enumerateResult = adapter->EnumOutputs(index, output.put());
    if (enumerateResult == DXGI_ERROR_NOT_FOUND) break;
    if (FAILED(enumerateResult)) break;

    DXGI_OUTPUT_DESC description{};
    if (SUCCEEDED(output->GetDesc(&description)) && description.AttachedToDesktop && displayDeviceName == description.DeviceName) return true;
  }
  return false;
}

app::GraphicsAdapterInfo graphicsAdapter() {
  const std::wstring primaryDeviceName = primaryDisplayDeviceName();
  windows::ComPtr<IDXGIFactory1> factory;
  if (FAILED(CreateDXGIFactory1(IID_IDXGIFactory1, factory.putVoid()))) return fallbackGraphicsAdapter(primaryDeviceName);

  for (UINT index = 0;; ++index) {
    windows::ComPtr<IDXGIAdapter1> adapter;
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

void queryPhysicalMemoryModules(app::SystemHardwareInfo& info) {
  auto& session = cimv2WmiSession();
  std::vector<app::PhysicalMemoryModuleInfo> decodedModules;
  try {
    const auto modules = session.query("SELECT Capacity, ConfiguredClockSpeed, SMBIOSMemoryType FROM Win32_PhysicalMemory");
    decodedModules.reserve(modules.size());
    for (const app::wmi::WmiRow& row : modules) {
      const auto capacity = app::wmi::row::requireUInt64(row, "Capacity");
      const auto speed = readNullableUInt(row, "ConfiguredClockSpeed");
      const auto type = readNullableUInt(row, "SMBIOSMemoryType");
      app::PhysicalMemoryModuleInfo module;
      module.capacityBytes = capacity;
      // WMI 沿用 ConfiguredClockSpeed 名称，但 SMBIOS 在现代 DDR 设备上返回的是
      // 有效传输速率（例如 DDR5-6000 返回 6000），展示统一使用 MT/s。
      module.configuredSpeedMtPerSecond = speed.value_or(0);
      module.memoryType = static_cast<app::SmbiosMemoryType>(type.value_or(0));
      if (module.capacityBytes != 0) decodedModules.push_back(module);
    }
  } catch (const std::exception& error) {
    UWF_LOG_D("hardware") << "memory module information unavailable: error=" << error.what();
    return;
  }

  // 只有整份模块查询与解码成功后才提交，避免展示部分模块。
  info.memoryModules = std::move(decodedModules);
}

void queryPhysicalMemorySlots(app::SystemHardwareInfo& info) {
  try {
    const auto arrays = cimv2WmiSession().query("SELECT MemoryDevices FROM Win32_PhysicalMemoryArray WHERE Use = 3");
    std::uint64_t totalSlots = 0;
    bool allSlotCountsKnown = true;
    for (const app::wmi::WmiRow& row : arrays) {
      const auto slots = readNullableUInt(row, "MemoryDevices");
      if (!slots) {
        allSlotCountsKnown = false;
        continue;
      }
      if (totalSlots > std::numeric_limits<std::uint32_t>::max() - *slots) {
        throw app::wmi::WmiDecodeError("decode physical memory information", "slot count overflow");
      }
      totalSlots += *slots;
    }
    if (allSlotCountsKnown) info.totalMemorySlots = static_cast<std::uint32_t>(totalSlots);
  } catch (const std::exception& error) {
    UWF_LOG_D("hardware") << "memory slot information unavailable: error=" << error.what();
  }
}

app::SystemHardwareInfo querySystemHardwareInfo() {
  app::SystemHardwareInfo info;
  info.cpuModel = cpuModel();
  try {
    info.graphicsAdapter = graphicsAdapter();
  } catch (const std::exception& error) {
    UWF_LOG_D("hardware") << "graphics adapter information unavailable: error=" << error.what();
  }
  queryPhysicalMemoryModules(info);
  queryPhysicalMemorySlots(info);
  std::uint64_t installedMemoryBytes = 0;
  bool installedMemoryOverflow = false;
  for (const app::PhysicalMemoryModuleInfo& module : info.memoryModules) {
    if (module.capacityBytes > std::numeric_limits<std::uint64_t>::max() - installedMemoryBytes) {
      installedMemoryOverflow = true;
      break;
    }
    installedMemoryBytes += module.capacityBytes;
  }
  info.totalMemoryBytes = !installedMemoryOverflow && installedMemoryBytes != 0 ? installedMemoryBytes : visibleMemoryBytes();
  return info;
}

}  // namespace

const app::SystemHardwareInfo& systemHardwareInfo() {
  static const app::SystemHardwareInfo info = querySystemHardwareInfo();
  return info;
}

}  // namespace uwf::windows
