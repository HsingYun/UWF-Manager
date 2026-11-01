// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include "application/overlay/contracts/OverlayPolicy.h"

namespace uwf::app {
class OverlayPolicyActions {
 public:
  virtual ~OverlayPolicyActions() = default;
  // Published global observations only; missing fields remain unknown.
  [[nodiscard]] virtual OverlayPolicySnapshot snapshot() const { return {}; }
  [[nodiscard]] virtual OverlayPolicyResult apply(const OverlayPolicyUpdate& update) = 0;
};
}  // namespace uwf::app
