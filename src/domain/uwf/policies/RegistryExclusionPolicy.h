// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <string>
#include <string_view>

namespace uwf::core {

enum class RegExclusionClass {
  Allowed,
  MachineAccount,  // Managed through the dedicated persistence setting.
  Container,       // Browsable HKLM root or reserved key, not selectable.
  OutOfScope,
};

enum class RegistryExclusionError { None, MachineSubkeyRequired, EmbeddedNull, ComponentTooLong, TooManyComponents, Reserved };

// Validate the original HKLM subkey: at most 512 nonempty components of 255
// UTF-16 units and six exact reserved targets. Preserve separators and non-NUL
// characters, including controls. Machine-account routing is separate editor policy.
[[nodiscard]] RegistryExclusionError registryExclusionError(std::u16string_view key);

// Expand hive aliases and fold ASCII case without normalizing the key suffix.
[[nodiscard]] RegExclusionClass classifyRegistryExclusion(const std::string& key);

}  // namespace uwf::core
