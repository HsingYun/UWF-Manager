// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "app/ports/VolumePaths.h"

#include <windows.h>

#include <cctype>
#include <string_view>

#include "windows/system/UnicodeConversion.h"
#include "windows/system/WindowsSystemError.h"

namespace uwf::drive {

namespace {

bool startsWithCI(const std::string& s, const char* prefix) {
  for (size_t i = 0; prefix[i] != '\0'; ++i) {
    if (i >= s.size()) return false;
    if (std::tolower(static_cast<unsigned char>(s[i])) != std::tolower(static_cast<unsigned char>(prefix[i]))) return false;
  }
  return true;
}

// body 形如 "Volume{GUID}\\..."（扩展前缀已剥）。还原出 Win32 API 需要的卷名
// "\\?\Volume{GUID}\\"（必须以反斜杠结尾）。'}' 缺失或其后不是 '\' 时返回空串。
std::string volumeNameFromGuidBody(const std::string& body) {
  const size_t brace = body.find('}');
  if (brace == std::string::npos) return {};
  if (brace + 1 >= body.size() || body[brace + 1] != '\\') return {};
  return R"(\\?\)" + body.substr(0, brace + 2);  // 含结尾反斜杠
}

}  // namespace

std::vector<std::string> mountedLetters(const std::string& volumeGuidPath) {
  const std::wstring volume = windows::utf8ToWide(volumeGuidPath);
  std::wstring paths(MAX_PATH, L'\0');
  // 装入点可能在查询期间增加；有限次扩容后仍不稳定就报告失败，避免无限重试。
  constexpr int kMaximumMountQueryAttempts = 3;
  for (int attempt = 0; attempt < kMaximumMountQueryAttempts; ++attempt) {
    DWORD length = 0;
    if (!GetVolumePathNamesForVolumeNameW(volume.c_str(), paths.data(), static_cast<DWORD>(paths.size()), &length)) {
      const DWORD error = GetLastError();
      if (error == ERROR_FILE_NOT_FOUND || error == ERROR_PATH_NOT_FOUND) return {};
      if (error != ERROR_MORE_DATA) throw windows::WindowsSystemError(error, "read volume path names");
      if (length <= paths.size()) throw std::runtime_error("volume path query returned an inconsistent required size");
      paths.resize(length);
      continue;
    }
    if (length == 0 || length > paths.size()) throw std::runtime_error("volume path query returned an invalid result size");
    const std::wstring_view result(paths.data(), length);
    std::vector<std::string> letters;
    for (size_t offset = 0; offset < result.size();) {
      if (result[offset] == L'\0') return letters;
      const auto end = result.find(L'\0', offset);
      if (end == std::wstring_view::npos) break;
      const auto path = result.substr(offset, end - offset);
      if (path.size() == 3 && path[1] == L':' && path[2] == L'\\' && path[0] <= 0x7f) {
        const auto letter = normalize(std::string{static_cast<char>(path[0]), ':'});
        if (!letter.empty()) letters.push_back(letter);
      }
      offset = end + 1;
    }
    throw std::runtime_error("volume path query returned a malformed MULTI_SZ");
  }
  throw windows::WindowsSystemError(ERROR_MORE_DATA, "volume mount points kept changing during query");
}

std::string fromPath(const std::string& path) {
  const PathSplit parts = split(path);
  if (!parts.letter.empty()) return parts.letter;

  // 卷 GUID 路径 "\\?\Volume{GUID}\\..."——字符串里没有盘符，查 Win32 API。
  const std::string& body = parts.rest;
  if (startsWithCI(body, "Volume{")) {
    const std::string volumeName = volumeNameFromGuidBody(body);
    if (volumeName.empty()) throw DriveLetterResolutionError("malformed volume GUID path: " + path);
    const auto letters = mountedLetters(volumeName);
    if (letters.empty()) throw DriveLetterResolutionError("could not resolve volume to a drive letter: " + volumeName);
    return letters.front();
  }

  return {};
}

std::string systemLetter() {
  // GetWindowsDirectoryA 返回 "C:\\Windows" 形式；盘符部分必为 ASCII。
  char buf[MAX_PATH] = {};
  const UINT n = GetWindowsDirectoryA(buf, MAX_PATH);
  if (n == 0 || n >= MAX_PATH) return {};  // 取不到或缓冲区不足——系统盘未知，不臆测 "C:"
  try {
    return fromPath(std::string(buf, n));
  } catch (const DriveLetterResolutionError&) {
    return {};
  }
}

}  // namespace uwf::drive
