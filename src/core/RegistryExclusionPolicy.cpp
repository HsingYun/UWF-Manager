// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "RegistryExclusionPolicy.h"

#include <string_view>

#include "UwfConstraints.h"
#include "shared/RegistryPath.h"
#include "shared/StringUtil.h"

namespace uwf::core {

RegExclusionClass classifyRegistryExclusion(const std::string& key) {
  const std::string upper = toUpperAscii(regkey::normalize(key));
  if (upper.empty()) return RegExclusionClass::OutOfScope;

  // $MACHINE.ACC 位于允许子树内，禁止规则须先于允许规则。
  if (std::string_view(upper) == core::kForbiddenRegistryKeyMachineAccount) return RegExclusionClass::MachineAccount;

  for (const std::string_view prefix : core::kAllowedRegistryRootPrefixes) {
    if (upper.size() > prefix.size() && upper.starts_with(prefix)) return RegExclusionClass::Allowed;
  }

  // 允许根本身及其祖先仅作为可展开的容器。
  const std::string withSep = upper + '\\';
  for (const std::string_view prefix : core::kAllowedRegistryRootPrefixes) {
    if (prefix.starts_with(withSep) || withSep.starts_with(prefix)) return RegExclusionClass::Container;
  }

  return RegExclusionClass::OutOfScope;
}

}  // namespace uwf::core
