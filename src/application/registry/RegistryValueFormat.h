// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <cstdint>
#include <string>
namespace uwf::app {
// Return the Win32 constant name, or UNKNOWN(number); identifiers are not localized.
[[nodiscard]] std::string valueTypeName(uint32_t type);
}  // namespace uwf::app
