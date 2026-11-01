// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "foundation/paths/DriveLetter.h"

#include <cctype>

#include "foundation/text/StringUtil.h"

namespace uwf::drive {

namespace {
bool isAsciiLetter(const char value) {
  const auto character = static_cast<unsigned char>(value);
  return (character >= static_cast<unsigned char>('A') && character <= static_cast<unsigned char>('Z')) ||
         (character >= static_cast<unsigned char>('a') && character <= static_cast<unsigned char>('z'));
}

std::string normalizedLetter(const char letter) { return {static_cast<char>(std::toupper(static_cast<unsigned char>(letter))), ':'}; }

std::string stripExtendedPrefix(const std::string& s) {
  if (s.size() >= 4 && s[0] == '\\' && s[1] == '\\' && (s[2] == '?' || s[2] == '.') && s[3] == '\\') {
    return s.substr(4);
  }
  return s;
}

}  // namespace

std::string normalize(const std::string& raw) {
  const std::string s = stripExtendedPrefix(trim(raw));
  if (s.empty() || !isAsciiLetter(s.front())) return {};
  // A leading letter alone does not make a relative filename drive-addressed.
  if (s.size() == 1 || (s.size() >= 2 && s[1] == ':')) return normalizedLetter(s.front());
  return {};
}

PathSplit split(const std::string& path) {
  // Whitespace is part of the selected object name.
  const std::string body = stripExtendedPrefix(path);
  if (body.size() >= 2 && isAsciiLetter(body[0]) && body[1] == ':') {
    return {normalizedLetter(body[0]), body.substr(2)};
  }
  return {std::string{}, body};
}

}  // namespace uwf::drive
