// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "RegistryValueFormat.h"

#include <format>
namespace uwf::app {
std::string valueTypeName(uint32_t type) {
  switch (type) {
    case 0u:
      return "REG_NONE";
    case 1u:
      return "REG_SZ";
    case 2u:
      return "REG_EXPAND_SZ";
    case 3u:
      return "REG_BINARY";
    case 4u:
      return "REG_DWORD";
    case 5u:
      return "REG_DWORD_BIG_ENDIAN";
    case 6u:
      return "REG_LINK";
    case 7u:
      return "REG_MULTI_SZ";
    case 8u:
      return "REG_RESOURCE_LIST";
    case 9u:
      return "REG_FULL_RESOURCE_DESCRIPTOR";
    case 10u:
      return "REG_RESOURCE_REQUIREMENTS_LIST";
    case 11u:
      return "REG_QWORD";
    default:
      return std::format("UNKNOWN({})", type);
  }
}

}  // namespace uwf::app
