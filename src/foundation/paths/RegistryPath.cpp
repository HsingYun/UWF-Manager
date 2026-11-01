// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "foundation/paths/RegistryPath.h"

#include "foundation/text/StringUtil.h"

namespace uwf::regkey {

std::string expandHive(const std::string& path) {
  const size_t firstSlash = path.find('\\');
  const std::string hive = (firstSlash == std::string::npos) ? path : path.substr(0, firstSlash);
  const std::string rest = (firstSlash == std::string::npos) ? std::string{} : path.substr(firstSlash);

  const std::string hiveUpper = toUpperAscii(hive);
  for (const auto& h : kRegistryHiveNames) {
    if (hiveUpper == h.shortForm || hiveUpper == h.longForm) return std::string(h.longForm) + rest;
  }
  return path;
}

std::u16string expandHive(const std::u16string_view path) {
  const auto slash = path.find(u'\\');
  const auto hive = path.substr(0, slash);
  // Predefined hive names are ASCII. Reuse the same alias table without
  // transcoding or normalizing any code unit in the actual key suffix.
  std::string asciiHive;
  for (const auto unit : hive) {
    if (unit > u'\x7f') return std::u16string(path);
    asciiHive.push_back(static_cast<char>(unit));
  }
  const auto expanded = expandHive(asciiHive);
  std::u16string result(expanded.begin(), expanded.end());
  if (slash != std::u16string_view::npos) result.append(path.substr(slash));
  return result;
}

}  // namespace uwf::regkey
