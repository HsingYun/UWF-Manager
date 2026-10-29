// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "RegistryPath.h"

#include "StringUtil.h"

namespace uwf::regkey {

std::string normalize(const std::string& key) {
  std::string path = trim(key);
  while (!path.empty() && path.back() == '\\') path.pop_back();
  if (path.empty()) return {};

  const size_t firstSlash = path.find('\\');
  const std::string hive = (firstSlash == std::string::npos) ? path : path.substr(0, firstSlash);
  const std::string rest = (firstSlash == std::string::npos) ? std::string{} : path.substr(firstSlash);

  const std::string hiveUpper = toUpperAscii(hive);
  for (const auto& h : kRegistryHiveNames) {
    if (hiveUpper == h.shortForm || hiveUpper == h.longForm) return std::string(h.longForm) + rest;
  }
  return path;  // hive 不认识——原样返回（已 trim、去尾反斜杠）
}

}  // namespace uwf::regkey
