// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <string>

namespace uwf::regkey {

struct RegistryHiveName {
  const char* shortForm;
  const char* longForm;
};
inline constexpr RegistryHiveName kRegistryHiveNames[] = {
    {"HKLM", "HKEY_LOCAL_MACHINE"}, {"HKCU", "HKEY_CURRENT_USER"}, {"HKCR", "HKEY_CLASSES_ROOT"}, {"HKU", "HKEY_USERS"}, {"HKCC", "HKEY_CURRENT_CONFIG"},
};

std::string normalize(const std::string& key);

}  // namespace uwf::regkey
