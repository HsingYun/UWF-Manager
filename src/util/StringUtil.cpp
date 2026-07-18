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
#include "StringUtil.h"

#include <windows.h>

#include <limits>
#include <stdexcept>
#include <system_error>

namespace uwf {

namespace {

bool isAsciiWhitespace(const char value) { return value == ' ' || value == '\t' || value == '\n' || value == '\r' || value == '\f' || value == '\v'; }

}  // namespace

std::string toLowerAscii(std::string s) {
  for (char& c : s) {
    if (c >= 'A' && c <= 'Z') c = static_cast<char>(c - 'A' + 'a');
  }
  return s;
}

std::string toUpperAscii(std::string s) {
  for (char& c : s) {
    if (c >= 'a' && c <= 'z') c = static_cast<char>(c - 'a' + 'A');
  }
  return s;
}

std::string trim(std::string s) {
  size_t begin = 0;
  size_t end = s.size();
  while (begin < end && isAsciiWhitespace(s[begin])) ++begin;
  while (end > begin && isAsciiWhitespace(s[end - 1])) --end;
  return s.substr(begin, end - begin);
}

std::wstring utf8ToWide(std::string_view utf8) {
  if (utf8.empty()) return {};
  if (utf8.size() > static_cast<size_t>(std::numeric_limits<int>::max())) throw std::length_error("UTF-8 input is too large to convert");
  const int inputLength = static_cast<int>(utf8.size());
  const int wideLen = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8.data(), inputLength, nullptr, 0);
  if (wideLen <= 0) throw std::system_error(static_cast<int>(GetLastError()), std::system_category(), "convert UTF-8 to UTF-16");
  std::wstring out(static_cast<size_t>(wideLen), L'\0');
  const int converted = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8.data(), inputLength, out.data(), wideLen);
  if (converted == 0) throw std::system_error(static_cast<int>(GetLastError()), std::system_category(), "convert UTF-8 to UTF-16");
  if (converted != wideLen) throw std::runtime_error("UTF-8 to UTF-16 conversion returned an inconsistent length");
  return out;
}

std::string wideToUtf8(std::wstring_view wide) {
  if (wide.empty()) return {};
  if (wide.size() > static_cast<size_t>(std::numeric_limits<int>::max())) throw std::length_error("UTF-16 input is too large to convert");
  const int inputLength = static_cast<int>(wide.size());
  const int byteLen = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, wide.data(), inputLength, nullptr, 0, nullptr, nullptr);
  if (byteLen <= 0) throw std::system_error(static_cast<int>(GetLastError()), std::system_category(), "convert UTF-16 to UTF-8");
  std::string out(static_cast<size_t>(byteLen), '\0');
  const int converted = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, wide.data(), inputLength, out.data(), byteLen, nullptr, nullptr);
  if (converted == 0) throw std::system_error(static_cast<int>(GetLastError()), std::system_category(), "convert UTF-16 to UTF-8");
  if (converted != byteLen) throw std::runtime_error("UTF-16 to UTF-8 conversion returned an inconsistent length");
  return out;
}

std::string wideToUtf8(const wchar_t* wide) {
  if (!wide) return {};
  return wideToUtf8(std::wstring_view(wide));
}

}  // namespace uwf
