// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "UnicodeConversion.h"

#include <windows.h>

#include <limits>
#include <stdexcept>

#include "app/ports/TextEncoding.h"
#include "windows/system/WindowsSystemError.h"

namespace uwf::windows {

std::wstring utf8ToWide(std::string_view utf8) {
  if (utf8.empty()) return {};
  if (utf8.size() > static_cast<size_t>(std::numeric_limits<int>::max())) throw std::length_error("UTF-8 input is too large to convert");
  const int inputLength = static_cast<int>(utf8.size());
  const int wideLen = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8.data(), inputLength, nullptr, 0);
  if (wideLen <= 0) throw WindowsSystemError(GetLastError(), "convert UTF-8 to UTF-16");
  std::wstring out(static_cast<size_t>(wideLen), L'\0');
  const int converted = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8.data(), inputLength, out.data(), wideLen);
  if (converted == 0) throw WindowsSystemError(GetLastError(), "convert UTF-8 to UTF-16");
  if (converted != wideLen) throw std::runtime_error("UTF-8 to UTF-16 conversion returned an inconsistent length");
  return out;
}

std::string wideToUtf8(std::wstring_view wide) {
  if (wide.empty()) return {};
  if (wide.size() > static_cast<size_t>(std::numeric_limits<int>::max())) throw std::length_error("UTF-16 input is too large to convert");
  const int inputLength = static_cast<int>(wide.size());
  const int byteLen = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, wide.data(), inputLength, nullptr, 0, nullptr, nullptr);
  if (byteLen <= 0) throw WindowsSystemError(GetLastError(), "convert UTF-16 to UTF-8");
  std::string out(static_cast<size_t>(byteLen), '\0');
  const int converted = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, wide.data(), inputLength, out.data(), byteLen, nullptr, nullptr);
  if (converted == 0) throw WindowsSystemError(GetLastError(), "convert UTF-16 to UTF-8");
  if (converted != byteLen) throw std::runtime_error("UTF-16 to UTF-8 conversion returned an inconsistent length");
  return out;
}

std::string wideToUtf8(const wchar_t* wide) {
  if (!wide) return {};
  return wideToUtf8(std::wstring_view(wide));
}

}  // namespace uwf::windows

namespace uwf::app {

QString decodeUtf8Text(const std::string_view text) { return QString::fromStdWString(windows::utf8ToWide(text)); }

}  // namespace uwf::app
