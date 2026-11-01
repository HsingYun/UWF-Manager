// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <chrono>
namespace uwf::presentation::timing {
inline constexpr auto kMinimumProgressDisplayDuration = std::chrono::seconds{1};
inline constexpr auto kProgressRevealDelay = std::chrono::milliseconds{200};
inline constexpr auto kProgressUpdateInterval = std::chrono::milliseconds{50};
inline constexpr auto kOverlayUsageRefreshInterval = std::chrono::seconds{5};
}  // namespace uwf::presentation::timing
