// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include "application/overlay/contracts/OverlayPolicy.h"

namespace uwf::app {
class OverlayPolicyDraft final {
 public:
  void reset(OverlayPolicySnapshot baseline);
  void edit(const OverlayPolicyDelta& changes);
  void replace(OverlayPolicyDelta intent);
  void discard() { m_intent = {}; }
  [[nodiscard]] const OverlayPolicySnapshot& baseline() const { return m_baseline; }
  [[nodiscard]] const OverlayPolicyDelta& intent() const { return m_intent; }

 private:
  OverlayPolicySnapshot m_baseline;
  // Explicit choices survive structural changes even when equal to the
  // observed flags. The composite editor derives executable changes later.
  OverlayPolicyDelta m_intent;
};
}  // namespace uwf::app
