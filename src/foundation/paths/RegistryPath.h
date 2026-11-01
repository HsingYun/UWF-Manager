// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <string>
#include <string_view>

namespace uwf::regkey {

// Expand only a recognized predefined hive. Preserve the key suffix verbatim;
// configuration-list spellings and object-path normalization are separate.
[[nodiscard]] std::string expandHive(const std::string& key);
[[nodiscard]] std::u16string expandHive(std::u16string_view key);

struct RegistryHiveName {
  const char* shortForm;
  const char* longForm;
};
inline constexpr RegistryHiveName kRegistryHiveNames[] = {
    {"HKLM", "HKEY_LOCAL_MACHINE"}, {"HKCU", "HKEY_CURRENT_USER"}, {"HKCR", "HKEY_CLASSES_ROOT"}, {"HKU", "HKEY_USERS"}, {"HKCC", "HKEY_CURRENT_CONFIG"},
};

}  // namespace uwf::regkey
