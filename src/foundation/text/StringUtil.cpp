// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "foundation/text/StringUtil.h"

namespace uwf {
namespace {
template <typename Character>
Character lowerAscii(const Character value) {
  return value >= 'A' && value <= 'Z' ? static_cast<Character>(value - 'A' + 'a') : value;
}
template <typename Character>
bool equalAscii(const std::basic_string_view<Character> left, const std::basic_string_view<Character> right) {
  if (left.size() != right.size()) return false;
  for (std::size_t index = 0; index < left.size(); ++index)
    if (lowerAscii(left[index]) != lowerAscii(right[index])) return false;
  return true;
}
}  // namespace

bool isAsciiWhitespace(const char value) { return value == ' ' || value == '\t' || value == '\n' || value == '\r' || value == '\f' || value == '\v'; }

std::string toLowerAscii(std::string s) {
  for (char& c : s) {
    c = lowerAscii(c);
  }
  return s;
}
std::u16string toLowerAscii(std::u16string s) {
  for (char16_t& c : s) c = lowerAscii(c);
  return s;
}

std::string toUpperAscii(std::string s) {
  for (char& c : s) {
    if (c >= 'a' && c <= 'z') c = static_cast<char>(c - 'a' + 'A');
  }
  return s;
}

bool equalAsciiInsensitive(const std::string_view left, const std::string_view right) { return equalAscii(left, right); }
bool equalAsciiInsensitive(const std::u16string_view left, const std::u16string_view right) { return equalAscii(left, right); }

std::string trim(std::string s) {
  size_t begin = 0;
  size_t end = s.size();
  while (begin < end && isAsciiWhitespace(s[begin])) ++begin;
  while (end > begin && isAsciiWhitespace(s[end - 1])) --end;
  return s.substr(begin, end - begin);
}

}  // namespace uwf
