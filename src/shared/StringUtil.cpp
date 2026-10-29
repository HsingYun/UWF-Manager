// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "StringUtil.h"

namespace uwf {

bool isAsciiWhitespace(const char value) { return value == ' ' || value == '\t' || value == '\n' || value == '\r' || value == '\f' || value == '\v'; }

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

}  // namespace uwf
