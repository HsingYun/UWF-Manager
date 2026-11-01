// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include "domain/uwf/contracts/AllVolumeProtection.h"

namespace uwf::compat {
struct UwfStateFacts;

// Read-only predicates over a complete scope. Unknown members remain unknown,
// including when another member already differs from the requested protection.
[[nodiscard]] std::optional<bool> aggregateVolumeProtection(const AllVolumeProtectionObservation& observation, bool protectedVolume);
// Project the published physical protection scope or complete Next registration
// scope without reading a provider or changing shared state.
[[nodiscard]] AllVolumeProtectionObservation projectAllVolumeProtection(const UwfStateFacts& state, bool protect);
}  // namespace uwf::compat
