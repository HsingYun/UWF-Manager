// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "domain/uwf/policies/RegistryCommitPolicy.h"

#include <algorithm>
#include <array>
#include <iterator>
#include <string_view>
#include <utility>

#include "foundation/paths/RegistryPath.h"
#include "foundation/text/StringUtil.h"

namespace uwf::core {
bool preservesRegistryCommitTarget(const std::string_view key) {
  const auto separator = key.find('\\');
  if (separator == std::string_view::npos) return true;
  const auto hive = key.substr(0, separator);
  if (!equalAsciiInsensitive(hive, "HKLM") && !equalAsciiInsensitive(hive, "HKEY_LOCAL_MACHINE")) return true;
  constexpr std::string_view marker = "SYSTEM\\CurrentControlSet";
  const auto relative = key.substr(separator + 1);
  return relative.size() <= marker.size() || !equalAsciiInsensitive(relative.substr(0, marker.size()), marker) || relative[marker.size()] == '\\';
}

bool isRegistryCommitTargetAllowed(const std::string& key, const bool valueOperation) {
  const auto separator = key.find('\\');
  auto hive = toUpperAscii(key.substr(0, separator));
  const auto name =
      std::ranges::find_if(regkey::kRegistryHiveNames, [&](const auto& candidate) { return hive == candidate.longForm || hive == candidate.shortForm; });
  if (name == std::end(regkey::kRegistryHiveNames)) return false;
  hive = name->longForm;
  if (valueOperation) return hive == "HKEY_LOCAL_MACHINE";

  auto relative = separator == std::string::npos ? std::string{} : key.substr(separator + 1);
  if (!relative.empty() && relative.back() == '\\') relative.pop_back();
  constexpr std::array<std::pair<std::string_view, std::string_view>, 10> reserved{{{"HKEY_LOCAL_MACHINE", "SYSTEM"},
                                                                                    {"HKEY_LOCAL_MACHINE", "SOFTWARE"},
                                                                                    {"HKEY_LOCAL_MACHINE", "SAM"},
                                                                                    {"HKEY_LOCAL_MACHINE", "SECURITY"},
                                                                                    {"HKEY_LOCAL_MACHINE", "COMPONENTS"},
                                                                                    {"HKEY_CLASSES_ROOT", ""},
                                                                                    {"HKEY_CURRENT_CONFIG", ""},
                                                                                    {"HKEY_CURRENT_USER", ""},
                                                                                    {"HKEY_LOCAL_MACHINE", ""},
                                                                                    {"HKEY_USERS", ""}}};
  // ValidateRegistryKeyValueToBeCommitted compares the requested relative
  // name's length with _wcsnicmp. Thus even a shorter prefix of a reserved name
  // is rejected; descendants such as SYSTEM\CurrentControlSet are distinct.
  return std::ranges::none_of(
      reserved, [&](const auto& target) { return hive == target.first && equalAsciiInsensitive(relative, target.second.substr(0, relative.size())); });
}
}  // namespace uwf::core
