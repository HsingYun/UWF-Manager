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
#include <string_view>

#include "../core/Config.h"
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
  result.revision = regkey::readDword(kCurrentVersion, "UBR");
  result.editionId = regkey::readString(kCurrentVersion, "EditionID");
  result.productName = correctedProductName(regkey::readString(kCurrentVersion, "ProductName"), result.family);
  result.displayVersion = regkey::readString(kCurrentVersion, "DisplayVersion");
  if (result.displayVersion.empty()) result.displayVersion = regkey::readString(kCurrentVersion, "ReleaseId");
  result.longTermServicing = isLtscEdition(result.editionId);
  return result;
}

}  // namespace

const WindowsVersionInfo& windowsVersionInfo() {
  static const WindowsVersionInfo version = queryWindowsVersion();
  return version;
}

}  // namespace uwf
