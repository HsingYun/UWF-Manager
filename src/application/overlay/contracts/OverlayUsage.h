// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <chrono>
#include <optional>
#include <string>
#include <utility>
#include <variant>

#include "domain/uwf/model/UwfModel.h"

namespace uwf::app {

struct OverlayUsageObservation {
  std::optional<std::chrono::system_clock::time_point> lastSuccess;
  std::optional<std::string> error;
  [[nodiscard]] bool stale() const { return error.has_value(); }
};
struct OverlayUsageUnavailable : OverlayUsageObservation {};
struct OverlayUsageDisabled : OverlayUsageObservation {};

struct OverlayUsageEnabled : OverlayUsageObservation {
  constexpr OverlayUsageEnabled(const core::OverlayRuntime usageRuntime, const core::OverlayConfig overlayConfig) noexcept
      : runtime(usageRuntime), config(overlayConfig) {}

  core::OverlayRuntime runtime;
  core::OverlayConfig config;
};

// Retain the last successful sample alongside observation time and read failure.
// Enabled guarantees a complete sample, not freshness.
using OverlayUsageState = std::variant<OverlayUsageUnavailable, OverlayUsageDisabled, OverlayUsageEnabled>;

[[nodiscard]] inline const OverlayUsageObservation& overlayUsageObservation(const OverlayUsageState& state) {
  return std::visit([](const auto& value) -> const OverlayUsageObservation& { return value; }, state);
}
inline void observeOverlayUsageSuccess(OverlayUsageState& state, const std::chrono::system_clock::time_point time) {
  std::visit(
      [time](auto& value) {
        value.lastSuccess = time;
        value.error.reset();
      },
      state);
}
inline void observeOverlayUsageFailure(OverlayUsageState& state, std::string error) {
  std::visit([&error](auto& value) { value.error = std::move(error); }, state);
}

}  // namespace uwf::app
