// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <cstdint>

#include "application/overlay/contracts/OverlayPolicy.h"

namespace uwf::app {

class OverlayPolicyOperations {
 public:
  virtual ~OverlayPolicyOperations() = default;
  // Published state only; this accessor never invokes a provider.
  [[nodiscard]] virtual OverlayPolicySnapshot snapshot() const = 0;
  // Option composition uses published flags; no implicit prerequisite/readback query.
  // None means already satisfied; Confirmed means applied. Failures throw
  // OverlayPolicyError; later matching reads cannot repair a failed command.
  [[nodiscard]] virtual WriteEffect setOption(OverlayOption option, bool enabled) = 0;
  // Only the two reset policy properties are writable by numeric value here.
  // The same confirmed-command contract applies to numeric writes.
  [[nodiscard]] virtual WriteEffect replace(OverlayProperty node, std::int64_t value) = 0;
};

}  // namespace uwf::app
