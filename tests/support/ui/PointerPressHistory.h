// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <cstdint>
#include <limits>
#include <optional>

namespace uwf::test {
// Opaque identities supplied by the desktop driver. Neither identity owns a
// window; the driver retires the pair when its QObject/native surface retires.
struct PointerWindowIdentity {
  std::uintptr_t object = 0;
  std::uintptr_t native = 0;
  bool operator==(const PointerWindowIdentity&) const = default;
};
struct PointerPoint {
  int x = 0, y = 0;
};
struct SinglePressPolicy {
  std::uint64_t intervalMs = 0;
  int halfWidth = 0, halfHeight = 0;
};

// The default mouseClick/press API means an independent press, not an implicit
// double click. Keep only the last real press, scoped to its window lifetime.
class PointerPressHistory final {
 public:
  void record(PointerWindowIdentity window, std::uint32_t button, PointerPoint point, std::uint64_t timeMs) { m_press = Press{window, button, point, timeMs}; }
  void retire(PointerWindowIdentity window) {
    if (m_press && m_press->window == window) m_press.reset();
  }
  [[nodiscard]] std::uint64_t remainingDelay(PointerWindowIdentity window, std::uint32_t button, PointerPoint point, std::uint64_t nowMs,
                                             SinglePressPolicy policy) const {
    if (!m_press || m_press->window != window || m_press->button != button || nowMs < m_press->timeMs || policy.halfWidth < 0 || policy.halfHeight < 0)
      return 0;
    const auto dx = static_cast<std::int64_t>(point.x) - m_press->point.x;
    const auto dy = static_cast<std::int64_t>(point.y) - m_press->point.y;
    if (dx < -static_cast<std::int64_t>(policy.halfWidth) || dx > policy.halfWidth || dy < -static_cast<std::int64_t>(policy.halfHeight) ||
        dy > policy.halfHeight)
      return 0;
    const auto elapsed = nowMs - m_press->timeMs;
    // A press exactly on the platform boundary may still be a double click.
    if (elapsed > policy.intervalMs) return 0;
    const auto remaining = policy.intervalMs - elapsed;
    return remaining == std::numeric_limits<std::uint64_t>::max() ? remaining : remaining + 1;
  }

 private:
  struct Press {
    PointerWindowIdentity window;
    std::uint32_t button;
    PointerPoint point;
    std::uint64_t timeMs;
  };
  std::optional<Press> m_press;
};
}  // namespace uwf::test
