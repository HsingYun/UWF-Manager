// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "SystemCheck.h"

#include <windows.h>

#include <filesystem>
#include <string>
#include <string_view>

#include "shared/Log.h"
#include "shared/StringUtil.h"
#include "windows/resources/UniqueHandle.h"
#include "windows/system/UnicodeConversion.h"
#include "windows/system/WindowsCompatibility.h"
#include "windows/system/WindowsVersion.h"

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
  // 进程的提权级别在创建时即固定——UAC 提权是另起新进程，运行中的进程无法
  // 就地改变自身主令牌的提权状态。故查一次缓存即可，无需每次重新查令牌。
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
  // uwfmgr.exe 的存在性只作为参考记录一条日志——本程序经 WMI 操作 UWF，
  // 并不调用这个命令行工具，找不到它不影响功能，故不再拦截启动。
  if (uwfmgrPath().empty()) {
    UWF_LOG_D("system") << "uwfmgr executable not found: capabilityProbe=wmi";
  }
  r.status = CheckStatus::Ok;
  return r;
}

}  // namespace uwf::windows
