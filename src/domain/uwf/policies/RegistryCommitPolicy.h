// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <string>
#include <string_view>

namespace uwf::core {
// Management target syntax/reservations only; existence, protection, and exclusions
// are separate. Values require HKLM. Whole-key deletion uses its own blacklist.
[[nodiscard]] bool isRegistryCommitTargetAllowed(const std::string& key, bool valueOperation);

// Reject CurrentControlSetExtra-style names that native prefix rewriting would
// redirect to a different ControlSetNNNExtra object. This does not validate permissions.
[[nodiscard]] bool preservesRegistryCommitTarget(std::string_view key);
}  // namespace uwf::core
