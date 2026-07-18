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
#include "WindowsVersion.h"

#include <windows.h>

#include <algorithm>
#include <exception>
#include <functional>
#include <string_view>
#include <utility>

#include "../core/Config.h"
#include "Log.h"
#include "RegistryKey.h"
#include "StringUtil.h"

namespace uwf {

namespace {

constexpr WindowsFamily classifyFamily(const std::uint32_t major, const std::uint32_t build, const bool workstation) {
  if (!workstation) return WindowsFamily::WindowsServer;
  if (major != 10) return WindowsFamily::Unknown;
  return build >= static_cast<std::uint32_t>(config::kWindows11MinBuildNumber) ? WindowsFamily::Windows11 : WindowsFamily::Windows10;
}

static_assert(classifyFamily(10, 19045, true) == WindowsFamily::Windows10);
static_assert(classifyFamily(10, 22000, true) == WindowsFamily::Windows11);
static_assert(classifyFamily(10, 28000, true) == WindowsFamily::Windows11);
static_assert(classifyFamily(10, 26100, false) == WindowsFamily::WindowsServer);
static_assert(classifyFamily(11, 30000, true) == WindowsFamily::Unknown);

bool isLtscEdition(const std::string& editionId) {
  const std::string normalized = toLowerAscii(editionId);
  return std::ranges::any_of(config::kLtscEditionIds, [&normalized](const std::string_view id) { return normalized == id; });
}

std::string correctedProductName(std::string productName, const WindowsFamily family) {
  if (productName.empty()) {
    switch (family) {
      case WindowsFamily::Windows10:
        return "Windows 10";
      case WindowsFamily::Windows11:
        return "Windows 11";
      case WindowsFamily::WindowsServer:
        return "Windows Server";
      case WindowsFamily::Unknown:
        return "Windows";
    }
  }
  if (family != WindowsFamily::Windows11) return productName;
  const auto pos = productName.find(config::kProductNameWin10Token);
  if (pos != std::string::npos) productName.replace(pos, config::kProductNameWin10Token.size(), config::kProductNameWin11Token);
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

WindowsVersionInfo queryWindowsVersion() {
  WindowsVersionInfo result;

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

  constexpr std::string_view kCurrentVersion = config::kRegPathWindowsCurrentVersion;
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

const WindowsVersionInfo& windowsVersionInfo() {
  static const WindowsVersionInfo version = queryWindowsVersion();
  return version;
}

}  // namespace uwf
