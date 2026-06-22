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
#include <cwchar>
#include <cwctype>

#include "StringUtil.h"

namespace uwf::drive {

namespace {

// s 开头连续字母的数量。
size_t leadingLetters(const std::string& s) {
  size_t n = 0;
  while (n < s.size() && std::isalpha(static_cast<unsigned char>(s[n]))) ++n;
  return n;
}

// 把字母段转大写、拼上结尾冒号，得到规范盘符（"C:" / "CC:"）。
std::string toUpperWithColon(const std::string& letters) { return toUpperAscii(letters) + ':'; }

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

// 用 Win32 把卷名 "\\?\Volume{GUID}\\" 解析成挂载的盘符（"C:" / "CC:"）。
// 该卷没有盘符挂载点（仅挂在文件夹上 / 未挂载）或 API 失败 → 返回空串。
std::string resolveVolumeDriveLetter(const std::string& volumeName) {
  const std::wstring wvol = utf8ToWide(volumeName);
  if (wvol.empty()) return {};

  // 先用 0 长缓冲探所需大小：必失败，且 GetLastError()==ERROR_MORE_DATA、
  // needed 被填好。任何其它结果都当解析不出盘符处理。
  DWORD needed = 0;
  if (GetVolumePathNamesForVolumeNameW(wvol.c_str(), nullptr, 0, &needed)) return {};
  if (GetLastError() != ERROR_MORE_DATA || needed == 0) return {};

  std::wstring buf(static_cast<size_t>(needed), L'\0');
  DWORD got = 0;
  if (!GetVolumePathNamesForVolumeNameW(wvol.c_str(), buf.data(), needed, &got)) return {};

  // buf 是"多个以 NUL 结尾的挂载点路径 + 末尾再一个 NUL"。逐段扫描，取第一个
  // 形如 "<字母...>:\\" 的盘符根挂载点。
  for (size_t i = 0; i < buf.size() && buf[i] != L'\0';) {
    const size_t len = std::wcslen(&buf[i]);
    if (len >= 3 && buf[i + len - 1] == L'\\' && buf[i + len - 2] == L':') {
      bool allLetters = true;
      for (size_t k = 0; k + 2 < len; ++k)
        if (!std::iswalpha(static_cast<wint_t>(buf[i + k]))) {
          allLetters = false;
          break;
        }
      if (allLetters) {
        std::string out;
        out.reserve(len - 1);
        for (size_t k = 0; k + 2 < len; ++k) out += static_cast<char>(std::towupper(static_cast<wint_t>(buf[i + k])));
        out += ':';
        return out;
      }
    }
    i += len + 1;
  }
  return {};
}

}  // namespace

std::string normalize(const std::string& raw) {
  const std::string s = stripExtendedPrefix(trim(raw));
  const size_t letters = leadingLetters(s);
  if (letters == 0) return {};
  // 字母段后面：要么直接到结尾（裸盘符 "C" / "CC"），要么紧跟 ':'（"C:" 或
  // "C:\\path"——冒号后的路径残留忽略）。其它（数字、'{' 等）都不是盘符。
  if (letters == s.size() || s[letters] == ':') return toUpperWithColon(s.substr(0, letters));
  return {};
}

PathSplit split(const std::string& path) {
  // 路径不 trim——前后空白对文件路径是有意义的，且盘符/扩展前缀本就在串首。
  const std::string body = stripExtendedPrefix(path);
  const size_t letters = leadingLetters(body);
  if (letters > 0 && letters < body.size() && body[letters] == ':') {
    return {toUpperWithColon(body.substr(0, letters)), body.substr(letters + 1)};
  }
  return {std::string{}, body};  // 无字面盘符前缀
}

std::string fromPath(const std::string& path, std::string* error) {
  if (error) error->clear();

  // 普通路径 / 扩展长度路径（含 "\\?\C:\\x"）——字面拆分即可拿到盘符。
  if (const PathSplit s = split(path); !s.letter.empty()) return s.letter;

  // 卷 GUID 路径 "\\?\Volume{GUID}\\..."——字符串里没有盘符，查 Win32 API。
  const std::string body = stripExtendedPrefix(path);
  if (startsWithCI(body, "Volume{")) {
    const std::string volumeName = volumeNameFromGuidBody(body);
    if (volumeName.empty()) {
      if (error) *error = "malformed volume GUID path: " + path;
      return {};
    }
    // 该卷只挂在文件夹上 / 未挂载 / API 失败——都归为"解析不出盘符"。
    const std::string dl = resolveVolumeDriveLetter(volumeName);
    if (dl.empty() && error) *error = "could not resolve volume to a drive letter: " + volumeName;
    return dl;
  }

  // 其余（UNC 路径、卷内相对路径等）本就没有盘符——返回空串，不算错误。
  return {};
}

std::string systemLetter() {
  // GetWindowsDirectoryA 返回 "C:\\Windows" 形式；盘符部分必为 ASCII。
  char buf[MAX_PATH] = {};
  const UINT n = GetWindowsDirectoryA(buf, MAX_PATH);
  if (n == 0 || n > MAX_PATH) return {};  // 取不到——系统盘未知，不臆测 "C:"
  return fromPath(std::string(buf, n));
}

}  // namespace uwf::drive
