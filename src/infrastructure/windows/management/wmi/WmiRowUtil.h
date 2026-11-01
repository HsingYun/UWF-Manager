// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

// Required fields reject missing or malformed values rather than using defaults.

#include <string>
#include <vector>

#include "infrastructure/windows/management/wmi/WmiException.h"
#include "infrastructure/windows/management/wmi/WmiOperations.h"

namespace uwf::windows::wmi::row {

// Empty SAFEARRAY and explicit VT_NULL outputs both represent an empty list.
// Missing fields or invalid items fail the entire conversion.
template <class T, class ParseItem>
std::vector<T> readArrayOutput(const WmiMethodOutput& output, const char* arrayKey, ParseItem parseItem) {
  std::vector<T> out;
  const auto it = output.arrays.find(arrayKey);
  if (it == output.arrays.end()) {
    const auto nullIt = output.values.find(arrayKey);
    if (nullIt != output.values.end() && !nullIt->second.isValid()) return out;
    throw WmiProtocolError("decode method output", std::string("required output array is missing: ") + arrayKey);
  }
  out.reserve(it->second.size());
  for (const auto& item : it->second) {
    out.push_back(parseItem(item));
  }
  return out;
}

enum class EmptyString { Allow, Reject };

bool requireBool(const WmiRow& r, const std::string& key);
int32_t requireInt(const WmiRow& r, const std::string& key);
uint32_t requireUInt(const WmiRow& r, const std::string& key);
uint64_t requireUInt64(const WmiRow& r, const std::string& key);
std::string requireString(const WmiRow& r, const std::string& key, EmptyString empty = EmptyString::Allow);
// Accepts expanded fields or __MOF text; missing, mistyped or empty strings throw.
std::string requireEmbeddedString(const WmiRow& r, const std::string& propName);
// MOF UInt64 values accept bare or quoted decimal digits, including zero.
uint64_t requireEmbeddedUInt64(const WmiRow& r, const std::string& propName);

// Parses a flat MOF property list with an optional instance wrapper. Property
// names compare without ASCII case sensitivity; duplicates or malformed syntax
// throw. A missing string property returns an empty string.
std::string extractFromMof(const std::string& mof, const std::string& propName);

}  // namespace uwf::windows::wmi::row
