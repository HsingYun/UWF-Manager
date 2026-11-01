// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace uwf::app {

// nullopt data means the value exists but exceeds the preview limit.
// An empty vector instead represents an observed empty value.
struct RegistryValueInfo {
  std::string name;                          // Empty selects the default value.
  uint32_t type;                             // Win32 REG_* type code.
  std::optional<std::vector<uint8_t>> data;  // Unmodified preview bytes; string values normally use UTF-16LE.
};

}  // namespace uwf::app
