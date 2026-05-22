#include "SystemCheck.h"

#include <windows.h>

#include <algorithm>
#include <charconv>
#include <filesystem>
#include <string>
#include <string_view>

#include "../core/Config.h"
#include "../util/Log.h"
#include "../util/StringUtil.h"

namespace uwf {

namespace {

std::string readRegString(HKEY root, const wchar_t* subKey, const wchar_t* value) {
  HKEY key = nullptr;
  if (RegOpenKeyExW(root, subKey, 0, KEY_READ | KEY_WOW64_64KEY, &key) != ERROR_SUCCESS) return {};
  DWORD type = 0;
  DWORD bytes = 0;
  LSTATUS st = RegQueryValueExW(key, value, nullptr, &type, nullptr, &bytes);
  if (st != ERROR_SUCCESS || (type != REG_SZ && type != REG_EXPAND_SZ) || bytes == 0) {
    RegCloseKey(key);
    return {};
  }
  std::wstring buf(bytes / sizeof(wchar_t), L'\0');
  st = RegQueryValueExW(key, value, nullptr, &type, reinterpret_cast<LPBYTE>(buf.data()), &bytes);
  RegCloseKey(key);
  if (st != ERROR_SUCCESS) return {};
  while (!buf.empty() && buf.back() == L'\0') buf.pop_back();
  return wideToUtf8(buf);
}

std::string envVar(const wchar_t* name) {
  const DWORD size = GetEnvironmentVariableW(name, nullptr, 0);
  if (size == 0) return {};
  std::wstring buf(size, L'\0');
  const DWORD got = GetEnvironmentVariableW(name, buf.data(), size);
  if (got == 0 || got >= size) return {};
  buf.resize(got);
  return wideToUtf8(buf);
}

// Windows 11 上注册表的 ProductName 仍写着 "Windows 10"——微软从未更新过这个
// 值。用 CurrentBuildNumber 兜底：>= 22000 即 Windows 11，把名称里的
// "Windows 10" 改写成 "Windows 11"；解析不出构建号或非 Win11 时原样返回。
std::string correctWin11Name(std::string productName) {
  const std::string build = readRegString(HKEY_LOCAL_MACHINE, config::kRegPathWindowsCurrentVersion, L"CurrentBuildNumber");
  int buildNum = 0;
  std::from_chars(build.data(), build.data() + build.size(), buildNum);
  if (buildNum < config::kWindows11MinBuildNumber) return productName;
  const auto pos = productName.find(config::kProductNameWin10Token);
  if (pos != std::string::npos) productName.replace(pos, config::kProductNameWin10Token.size(), config::kProductNameWin11Token);
  return productName;
}

bool editionSupported(const std::string& editionId) {
  const std::string e = toLowerAscii(editionId);
  if (e.empty()) return false;
  for (const auto keyword : config::kSupportedEditionKeywords) {
    if (e.find(keyword) != std::string::npos) return true;
  }
  return false;
}

}  // namespace

bool isElevated() {
  // 进程的提权级别在创建时即固定——UAC 提权是另起新进程，运行中的进程无法
  // 就地改变自身主令牌的提权状态。故查一次缓存即可，无需每次重新查令牌。
  static const bool cached = [] {
    BOOL elevated = FALSE;
    HANDLE token = nullptr;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token)) {
      TOKEN_ELEVATION elevation{};
      DWORD size = 0;
      if (GetTokenInformation(token, TokenElevation, &elevation, sizeof(elevation), &size)) {
        elevated = elevation.TokenIsElevated ? TRUE : FALSE;
      }
      CloseHandle(token);
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
  r.editionId = readRegString(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", L"EditionID");
  r.productName = correctWin11Name(readRegString(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", L"ProductName"));

  if (!editionSupported(r.editionId)) {
    r.status = CheckStatus::UnsupportedEdition;
    return r;
  }
  // uwfmgr.exe 的存在性只作为参考记录一条日志——本程序经 WMI 操作 UWF，
  // 并不调用这个命令行工具，找不到它不影响功能，故不再拦截启动。
  if (uwfmgrPath().empty()) {
    UWF_LOG_W("syscheck") << "uwfmgr.exe not found under System32; UWF feature may not be installed (continuing anyway)";
  }
  r.status = CheckStatus::Ok;
  return r;
}

}  // namespace uwf
