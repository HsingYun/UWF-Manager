#include "SystemCheck.h"

#include <windows.h>

#include <algorithm>
#include <filesystem>
#include <string>

#include "../util/Log.h"

namespace uwf {

namespace {

std::string wideToUtf8(const std::wstring& w) {
  if (w.empty()) return {};
  const int size = WideCharToMultiByte(CP_UTF8, 0, w.data(), static_cast<int>(w.size()), nullptr, 0, nullptr, nullptr);
  if (size <= 0) return {};
  std::string out(static_cast<size_t>(size), '\0');
  WideCharToMultiByte(CP_UTF8, 0, w.data(), static_cast<int>(w.size()), out.data(), size, nullptr, nullptr);
  return out;
}

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

std::string toLowerAscii(std::string s) {
  std::ranges::transform(s, s.begin(), [](const unsigned char c) { return std::tolower(c); });
  return s;
}

bool editionSupported(const std::string& editionId) {
  const std::string e = toLowerAscii(editionId);
  if (e.empty()) return false;
  return e.find("enterprise") != std::string::npos || e.find("education") != std::string::npos || e.find("iotenterprise") != std::string::npos ||
         e.find("enterpriseg") != std::string::npos;
}

}  // namespace

bool isElevated() {
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
  r.productName = readRegString(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", L"ProductName");

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
