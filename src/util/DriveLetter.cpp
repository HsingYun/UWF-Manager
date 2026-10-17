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
#include "DriveLetter.h"

#include <windows.h>

#include <cctype>
#include <string_view>
#include <system_error>

#include "StringUtil.h"

namespace uwf::drive {

namespace {

bool isAsciiLetter(const char value) {
  const auto character = static_cast<unsigned char>(value);
  return (character >= static_cast<unsigned char>('A') && character <= static_cast<unsigned char>('Z')) ||
         (character >= static_cast<unsigned char>('a') && character <= static_cast<unsigned char>('z'));
}

std::string normalizedLetter(const char letter) { return {static_cast<char>(std::toupper(static_cast<unsigned char>(letter))), ':'}; }

// 去掉开头的扩展长度前缀 "\\?\" 或 "\\.\"（恰好这 4 个字符）；没有则原样返回。
// 注意 "\\?\UNC\server\share" 去前缀后剩 "UNC\server\share"，后续按"无盘符"
// 自然落空，符合预期，无需特判。
std::string stripExtendedPrefix(const std::string& s) {
  if (s.size() >= 4 && s[0] == '\\' && s[1] == '\\' && (s[2] == '?' || s[2] == '.') && s[3] == '\\') {
    return s.substr(4);
  }
  return s;
}

// s 是否以 prefix 开头（ASCII 大小写不敏感）。
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
  const std::wstring volume = utf8ToWide(volumeGuidPath);
  std::wstring paths(MAX_PATH, L'\0');
  // 装入点可能在查询期间增加；有限次扩容后仍不稳定就报告失败，避免无限重试。
  constexpr int kMaximumMountQueryAttempts = 3;
  for (int attempt = 0; attempt < kMaximumMountQueryAttempts; ++attempt) {
    DWORD length = 0;
    if (!GetVolumePathNamesForVolumeNameW(volume.c_str(), paths.data(), static_cast<DWORD>(paths.size()), &length)) {
      const DWORD error = GetLastError();
      if (error == ERROR_FILE_NOT_FOUND || error == ERROR_PATH_NOT_FOUND) return {};
      if (error != ERROR_MORE_DATA) throw std::system_error(static_cast<int>(error), std::system_category(), "read volume path names");
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
      if (path.size() == 3 && path[1] == L':' && path[2] == L'\\' && path[0] <= 0x7f && isAsciiLetter(static_cast<char>(path[0]))) {
        letters.push_back(normalizedLetter(static_cast<char>(path[0])));
      }
      offset = end + 1;
    }
    throw std::runtime_error("volume path query returned a malformed MULTI_SZ");
  }
  throw std::system_error(ERROR_MORE_DATA, std::system_category(), "volume mount points kept changing during query");
}

std::string normalize(const std::string& raw) {
  const std::string s = stripExtendedPrefix(trim(raw));
  if (s.empty() || !isAsciiLetter(s.front())) return {};
  // 裸字母或单字母加冒号/路径才是 Windows 盘符。普通相对路径不能因为以
  // 字母开头就被误识别成盘符。
  if (s.size() == 1 || (s.size() >= 2 && s[1] == ':')) return normalizedLetter(s.front());
  return {};
}

PathSplit split(const std::string& path) {
  // 路径不 trim——前后空白对文件路径是有意义的，且盘符/扩展前缀本就在串首。
  const std::string body = stripExtendedPrefix(path);
  if (body.size() >= 2 && isAsciiLetter(body[0]) && body[1] == ':') {
    return {normalizedLetter(body[0]), body.substr(2)};
  }
  return {std::string{}, body};  // 无字面盘符前缀
}

std::string fromPath(const std::string& path) {
  // 普通路径 / 扩展长度路径（含 "\\?\C:\\x"）——字面拆分即可拿到盘符。
  if (const PathSplit s = split(path); !s.letter.empty()) return s.letter;

  // 卷 GUID 路径 "\\?\Volume{GUID}\\..."——字符串里没有盘符，查 Win32 API。
  const std::string body = stripExtendedPrefix(path);
  if (startsWithCI(body, "Volume{")) {
    const std::string volumeName = volumeNameFromGuidBody(body);
    if (volumeName.empty()) throw DriveLetterResolutionError("malformed volume GUID path: " + path);
    const auto letters = mountedLetters(volumeName);
    if (letters.empty()) throw DriveLetterResolutionError("could not resolve volume to a drive letter: " + volumeName);
    return letters.front();
  }

  // 其余（UNC 路径、卷内相对路径等）本就没有盘符——返回空串，不算错误。
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
