// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "infrastructure/windows/compatibility/SystemCheck.h"

#include <windows.h>

#include <filesystem>
#include <string>

#include "foundation/logging/Log.h"
#include "foundation/text/StringUtil.h"
#include "infrastructure/windows/resources/UniqueHandle.h"
#include "infrastructure/windows/text/UnicodeConversion.h"
#include "infrastructure/windows/compatibility/WindowsCompatibility.h"
#include "infrastructure/windows/compatibility/WindowsVersion.h"

namespace uwf::windows {

namespace {

std::string envVar(const wchar_t* name) {
  const DWORD size = GetEnvironmentVariableW(name, nullptr, 0);
  if (size == 0) return {};
  std::wstring buf(size, L'\0');
  const DWORD got = GetEnvironmentVariableW(name, buf.data(), size);
  if (got == 0 || got >= size) return {};
  buf.resize(got);
  return wideToUtf8(buf);
}

bool editionSupported(const std::string& editionId) {
  const std::string e = toLowerAscii(editionId);
  if (e.empty()) return false;
  for (const auto keyword : windows::kSupportedEditionKeywords) {
    if (e.find(keyword) != std::string::npos) return true;
  }
  return false;
}

bool windowsFamilySupported(const app::WindowsFamily family) { return family == app::WindowsFamily::Windows10 || family == app::WindowsFamily::Windows11; }

}  // namespace

bool isElevated() {
  // UI elevation launches a new process, so this observation is cached.
  static const bool cached = [] {
    BOOL elevated = FALSE;
    HANDLE rawToken = nullptr;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &rawToken)) {
      const windows::UniqueHandle token(rawToken);
      TOKEN_ELEVATION elevation{};
      DWORD size = 0;
      if (GetTokenInformation(token.get(), TokenElevation, &elevation, sizeof(elevation), &size)) {
        elevated = elevation.TokenIsElevated ? TRUE : FALSE;
      }
    }
    return elevated == TRUE;
  }();
  return cached;
}

std::string uwfmgrPath() {
  std::string root = envVar(L"SystemRoot");
  if (root.empty()) root = "C:/Windows";
  const std::string path = root + "/System32/uwfmgr.exe";
  std::error_code ec;
  return std::filesystem::exists(path, ec) ? path : std::string{};
}

SystemCheckResult runSystemChecks() {
  SystemCheckResult r;
  const app::WindowsVersionInfo& version = windowsVersionInfo();
  r.editionId = version.editionId;
  r.productName = version.productName;

  if (!windowsFamilySupported(version.family) || !editionSupported(r.editionId)) {
    r.status = CheckStatus::UnsupportedSystem;
    return r;
  }
  // Tool presence is diagnostic; the UWF adapters probe actual capabilities.
  if (uwfmgrPath().empty()) {
    UWF_LOG_D("system") << "uwfmgr executable not found";
  }
  r.status = CheckStatus::Ok;
  return r;
}

}  // namespace uwf::windows
