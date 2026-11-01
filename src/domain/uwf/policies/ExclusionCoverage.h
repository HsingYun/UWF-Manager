// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace uwf::core {
// These facts are independent. The captured management precheck can acknowledge
// a commit without issuing a driver commit, although writes still enter the
// overlay. Configuration-list identity is a third, exact ASCII-case comparison.
struct ExclusionEffects {
  bool managementSkipsCommit = false;
  bool driverBypassesWrites = false;
  [[nodiscard]] bool preventsCommit() const { return managementSkipsCommit || driverBypassesWrites; }
};

// Inputs are a stored pattern and the actual object path, using backslashes and
// the same volume-relative/absolute form. Never clean or trim the pattern.
// IsFileExcluded first compares the original strings, then removes at most one
// trailing pattern separator for its ASCII descendant comparison. It does not
// remove a target separator. The driver preserves the pattern and uses ordinal
// prefix matching with a component boundary.
[[nodiscard]] ExclusionEffects fileExclusionEffects(std::u16string_view pattern, std::u16string_view target);

// Resolve hive aliases without normalizing suffixes. Management matching folds
// ASCII case and removes at most one trailing separator from each suffix.
// Driver matching preserves separators and uses host ordinal identity.
// Stored driver patterns replace a boundary-free CurrentControlSet prefix with
// ControlSet%03d; resolve the current set only when that translation affects coverage.
// Missing required control-set observations are errors, not false matches.
// Management commit skipping is independent of this environment fact.
[[nodiscard]] bool registryManagementSkipsCommit(std::u16string_view pattern, std::u16string_view target);
[[nodiscard]] bool registryExclusionNeedsCurrentControlSet(std::u16string_view pattern, std::u16string_view target);
// Actual object identity resolves the complete CurrentControlSet component.
// A different key such as CurrentControlSetExtra remains a distinct object;
// only stored driver patterns have the captured boundary-free replacement.
[[nodiscard]] std::u16string registryDriverObjectName(std::u16string_view key, std::uint32_t currentControlSet);
[[nodiscard]] ExclusionEffects registryExclusionEffects(std::u16string_view pattern, std::u16string_view target,
                                                        std::optional<std::uint32_t> currentControlSet = {});
}  // namespace uwf::core
