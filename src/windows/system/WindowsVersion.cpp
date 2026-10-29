// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "WindowsVersion.h"

#include <windows.h>

#include <algorithm>
#include <exception>
#include <functional>
#include <string_view>
#include <utility>

#include "app/ports/RegistryKey.h"
#include "shared/Log.h"
#include "shared/StringUtil.h"
#include "windows/system/WindowsCompatibility.h"

namespace uwf::windows {

namespace {

constexpr app::WindowsFamily classifyFamily(const std::uint32_t major, const std::uint32_t build, const bool workstation) {
  if (!workstation) return app::WindowsFamily::WindowsServer;
  if (major != 10) return app::WindowsFamily::Unknown;
  return build >= static_cast<std::uint32_t>(windows::kWindows11MinBuildNumber) ? app::WindowsFamily::Windows11 : app::WindowsFamily::Windows10;
}

static_assert(classifyFamily(10, 19045, true) == app::WindowsFamily::Windows10);
static_assert(classifyFamily(10, 22000, true) == app::WindowsFamily::Windows11);
static_assert(classifyFamily(10, 28000, true) == app::WindowsFamily::Windows11);
static_assert(classifyFamily(10, 26100, false) == app::WindowsFamily::WindowsServer);
static_assert(classifyFamily(11, 30000, true) == app::WindowsFamily::Unknown);

bool isLtscEdition(const std::string& editionId) {
  const std::string normalized = toLowerAscii(editionId);
  return std::ranges::any_of(windows::kLtscEditionIds, [&normalized](const std::string_view id) { return normalized == id; });
}

std::string correctedProductName(std::string productName, const app::WindowsFamily family) {
  if (productName.empty()) {
    switch (family) {
      case app::WindowsFamily::Windows10:
        return "Windows 10";
      case app::WindowsFamily::Windows11:
        return "Windows 11";
      case app::WindowsFamily::WindowsServer:
        return "Windows Server";
      case app::WindowsFamily::Unknown:
        return "Windows";
    }
  }
  if (family != app::WindowsFamily::Windows11) return productName;
  const auto pos = productName.find(windows::kProductNameWin10Token);
  if (pos != std::string::npos) productName.replace(pos, windows::kProductNameWin10Token.size(), windows::kProductNameWin11Token);
  return productName;
}

template <typename T, typename Read>
T readVersionMetadata(const std::string_view valueName, T fallback, Read&& read) {
  try {
    auto value = std::invoke(std::forward<Read>(read));
    return value ? std::move(*value) : fallback;
  } catch (const std::exception& error) {
    // CurrentVersion 下的这些字段只用于展示和官方支持清单判断。它们读取失败
    // 不能阻止后续 Embedded namespace 能力探测，否则自行安装的 UWF provider
    // 会被无关的系统元数据故障屏蔽。
    UWF_LOG_W("system") << "Windows version metadata unavailable: value=" << valueName << " error=" << error.what();
    return fallback;
  }
}

app::WindowsVersionInfo queryWindowsVersion() {
  app::WindowsVersionInfo result;

  using RtlGetVersionFn = LONG(WINAPI*)(OSVERSIONINFOW*);
  const HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
  const auto rtlGetVersion = ntdll ? reinterpret_cast<RtlGetVersionFn>(GetProcAddress(ntdll, "RtlGetVersion")) : nullptr;
  OSVERSIONINFOEXW native{};
  native.dwOSVersionInfoSize = sizeof(native);
  if (rtlGetVersion && rtlGetVersion(reinterpret_cast<OSVERSIONINFOW*>(&native)) == 0) {
    result.family = classifyFamily(native.dwMajorVersion, native.dwBuildNumber, native.wProductType == VER_NT_WORKSTATION);
    result.major = native.dwMajorVersion;
    result.minor = native.dwMinorVersion;
    result.build = native.dwBuildNumber;
  }

  constexpr std::string_view kCurrentVersion = windows::kRegPathWindowsCurrentVersion;
  result.revision = readVersionMetadata("UBR", std::uint32_t{0}, [=] { return regkey::readDword(kCurrentVersion, "UBR"); });
  result.editionId = readVersionMetadata("EditionID", std::string{}, [=] { return regkey::readString(kCurrentVersion, "EditionID"); });
  result.productName = correctedProductName(
      readVersionMetadata("ProductName", std::string{}, [=] { return regkey::readString(kCurrentVersion, "ProductName"); }), result.family);
  result.displayVersion = readVersionMetadata("DisplayVersion", std::string{}, [=] { return regkey::readString(kCurrentVersion, "DisplayVersion"); });
  if (result.displayVersion.empty()) {
    result.displayVersion = readVersionMetadata("ReleaseId", std::string{}, [=] { return regkey::readString(kCurrentVersion, "ReleaseId"); });
  }
  result.longTermServicing = isLtscEdition(result.editionId);
  return result;
}

}  // namespace

const app::WindowsVersionInfo& windowsVersionInfo() {
  static const app::WindowsVersionInfo version = queryWindowsVersion();
  return version;
}

}  // namespace uwf::windows
