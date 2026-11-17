// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <vector>

#include "application/overlay/contracts/OverlayPolicy.h"

namespace uwf::app {
struct ConfigurationPlan;
enum class OverlayPolicyIssue {
  None,
  AdministratorRequired,
  WritesBlocked,
  HormActive,
  CurrentFilterMustBeOff,
  FlagsUnavailable,
  ResetUnsupported,
  ResetUnknown,
  CurrentPersistentRequired,
  NextPersistentRequired,
  PersistentNeedsDisk,
  MediaNeedsRam,
  StaleEdit
};

struct OverlayPolicyConflict {
  OverlayPolicyIssue issue;
  OverlayPolicyFailure failure;
};
struct OverlayPolicyValidation {
  std::vector<OverlayPolicyConflict> conflicts;
  [[nodiscard]] bool valid() const { return conflicts.empty(); }
  void requireValid() const;
};

[[nodiscard]] OverlayPolicyDelta overlayPolicyValues(const OverlayPolicySnapshot& baseline, const OverlayPolicyDelta& delta = {});
[[nodiscard]] OverlayPolicyDelta overlayPolicyChanges(const OverlayPolicySnapshot& baseline, const OverlayPolicyDelta& values);
// Explicit Passthrough/Persistent commands require Current OFF before any
// same-value decision; ROM retains the product rule for an actual value change.
[[nodiscard]] bool overlayOptionsRequireCurrentFilterOff(const OverlayPolicySnapshot& baseline, const OverlayPolicyDelta& values);
// A missing current-session observation cannot satisfy a filter-off prerequisite.
[[nodiscard]] OverlayPolicyIssue overlayCurrentFilterOffIssue(const OverlayPolicySnapshot& baseline);
// Reset and cancellation both require a running persistent disk overlay.
[[nodiscard]] OverlayPolicyIssue overlayResetCurrentIssue(const OverlayPolicySnapshot& baseline);
// Product rules, shared by editing, review and execution. These are not
// additional claims about undocumented native provider preconditions.
// Admission and presentation consume the same rule result. Each conflict keeps
// its precise prerequisite and provider diagnostics instead of reinterpreting
// a presentation issue when execution starts.
[[nodiscard]] OverlayPolicyValidation validateOverlayPolicyPlan(const ConfigurationPlan& plan);
}  // namespace uwf::app
