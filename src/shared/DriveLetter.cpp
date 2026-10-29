// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "DriveLetter.h"

#include <cctype>

#include "StringUtil.h"

namespace uwf::drive {

namespace {
bool isAsciiLetter(const char value) {
  const auto character = static_cast<unsigned char>(value);
  return (character >= static_cast<unsigned char>('A') && character <= static_cast<unsigned char>('Z')) ||
         (character >= static_cast<unsigned char>('a') && character <= static_cast<unsigned char>('z'));
}

std::string normalizedLetter(const char letter) { return {static_cast<char>(std::toupper(static_cast<unsigned char>(letter))), ':'}; }

// 仅移除扩展长度前缀；UNC 路径仍按无盘符处理。
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

}  // namespace uwf::drive
