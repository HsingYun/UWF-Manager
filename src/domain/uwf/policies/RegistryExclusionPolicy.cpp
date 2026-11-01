// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "domain/uwf/policies/RegistryExclusionPolicy.h"

#include <algorithm>
#include <cstddef>

#include "domain/uwf/policies/UwfConstraints.h"
#include "foundation/paths/RegistryPath.h"
#include "foundation/text/StringUtil.h"

namespace uwf::core {

RegistryExclusionError registryExclusionError(const std::u16string_view key) {
  if (key.find(u'\0') != std::u16string_view::npos) return RegistryExclusionError::EmbeddedNull;
  const auto expanded = regkey::expandHive(key);
  constexpr std::u16string_view prefix = u"HKEY_LOCAL_MACHINE\\";
  if (expanded.size() < prefix.size() || !equalAsciiInsensitive(std::u16string_view(expanded).substr(0, prefix.size()), prefix))
    return RegistryExclusionError::MachineSubkeyRequired;
  const auto relative = std::u16string_view(expanded).substr(prefix.size());
  constexpr std::size_t componentLimit = 255;
  constexpr std::size_t componentCountLimit = 512;
  std::size_t components = 0;
  for (std::size_t offset = 0; offset < relative.size();) {
    if (relative[offset] == u'\\') {
      ++offset;
      continue;
    }
    const auto separator = relative.find(u'\\', offset);
    const auto end = separator == std::u16string_view::npos ? relative.size() : separator;
    if (end - offset > componentLimit) return RegistryExclusionError::ComponentTooLong;
    if (++components > componentCountLimit) return RegistryExclusionError::TooManyComponents;
    offset = end;
  }
  if (components == 0) return RegistryExclusionError::MachineSubkeyRequired;
  for (const auto reserved : kReservedRegistryExclusionKeys) {
    const std::u16string name(reserved.begin(), reserved.end());
    if (equalAsciiInsensitive(expanded, name)) return RegistryExclusionError::Reserved;
  }
  return RegistryExclusionError::None;
}

RegExclusionClass classifyRegistryExclusion(const std::string& key) {
  const std::string upper = toUpperAscii(regkey::expandHive(key));
  if (upper.empty()) return RegExclusionClass::OutOfScope;

  if (upper == kRegistryExclusionHive) return RegExclusionClass::Container;
  const std::string prefix = std::string(kRegistryExclusionHive) + '\\';
  if (upper == prefix) return RegExclusionClass::Container;
  if (!upper.starts_with(prefix)) return RegExclusionClass::OutOfScope;

  // This editor routes the machine account secret to its dedicated persistence
  // setting. The native Add contract does not reserve this target.
  if (upper == kRegistryKeyMachineAccount) return RegExclusionClass::MachineAccount;
  if (std::ranges::find(kReservedRegistryExclusionKeys, upper) != kReservedRegistryExclusionKeys.end()) return RegExclusionClass::Container;
  return RegExclusionClass::Allowed;
}

}  // namespace uwf::core
