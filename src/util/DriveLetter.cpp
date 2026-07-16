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

#include <algorithm>
#include <cctype>
#include <iterator>
#include <system_error>

#include "StringUtil.h"

namespace uwf::drive {

namespace {

bool isAsciiLetter(const char value) {
  const auto character = static_cast<unsigned char>(value);
  return (character >= static_cast<unsigned char>('A') && character <= static_cast<unsigned char>('Z')) ||
         (character >= static_cast<unsigned char>('a') && character <= static_cast<unsigned char>('z'));
}

std::string normalizedLetter(const char letter) {
  return {static_cast<char>(std::toupper(static_cast<unsigned char>(letter))), ':'};
}

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

// 用 Win32 把卷名 "\\?\Volume{GUID}\\" 解析成挂载的盘符（"C:"）。
// 成功但没有盘符挂载点时返回空串；API 失败抛 std::system_error，不能把系统
// 故障与“该卷确实没有盘符”折叠成同一业务状态。
std::string resolveVolumeDriveLetter(const std::string& volumeName) {
  const std::wstring wvol = utf8ToWide(volumeName);

  // 先用 0 长缓冲探所需大小：必失败，且 GetLastError()==ERROR_MORE_DATA、
  // needed 被填好。其它错误保留原始 Win32 语义。
  DWORD needed = 0;
  if (GetVolumePathNamesForVolumeNameW(wvol.c_str(), nullptr, 0, &needed)) {
    throw std::runtime_error("volume path query unexpectedly succeeded without an output buffer");
  }
  const DWORD sizingError = GetLastError();
  if (sizingError != ERROR_MORE_DATA) {
    throw std::system_error(static_cast<int>(sizingError), std::system_category(), "measure volume path names");
  }
  if (needed == 0) throw std::runtime_error("volume path query returned an empty required size");

  std::wstring buf(static_cast<size_t>(needed), L'\0');
  DWORD got = 0;
  if (!GetVolumePathNamesForVolumeNameW(wvol.c_str(), buf.data(), needed, &got)) {
    throw std::system_error(static_cast<int>(GetLastError()), std::system_category(), "read volume path names");
  }

  // buf 是"多个以 NUL 结尾的挂载点路径 + 末尾再一个 NUL"。逐段扫描，取第一个
  // 形如 "C:\\" 的盘符根挂载点。目录挂载点不是盘符，不能截取其前缀。
  for (size_t i = 0; i < buf.size() && buf[i] != L'\0';) {
    const auto terminator = std::find(buf.cbegin() + static_cast<std::ptrdiff_t>(i), buf.cend(), L'\0');
    if (terminator == buf.cend()) throw std::runtime_error("volume path query returned a malformed MULTI_SZ");
    const size_t len = static_cast<size_t>(std::distance(buf.cbegin() + static_cast<std::ptrdiff_t>(i), terminator));
    if (len == 3 && buf[i + 1] == L':' && buf[i + 2] == L'\\' && buf[i] <= 0x7f && isAsciiLetter(static_cast<char>(buf[i]))) {
      return normalizedLetter(static_cast<char>(buf[i]));
    }
    i += len + 1;
  }
  return {};
}

}  // namespace

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
    const std::string dl = resolveVolumeDriveLetter(volumeName);
    if (dl.empty()) throw DriveLetterResolutionError("could not resolve volume to a drive letter: " + volumeName);
    return dl;
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
