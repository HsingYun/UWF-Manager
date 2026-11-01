// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <cstdint>
#include <optional>

#include "application/overlay/contracts/OverlayPolicy.h"
#include "application/staging/contracts/AutomaticCommitDisposition.h"
#include "domain/uwf/policies/IndividualCommitPolicy.h"

namespace uwf::app {

struct AutomaticCommitContext {
  OverlayPolicySnapshot overlay;
  std::optional<bool> hormEnabled;
  std::optional<std::uint32_t> romDisposition;
  std::optional<bool> fullCommit;
};

[[nodiscard]] inline AutomaticCommitDisposition automaticFileCommitDisposition(const AutomaticCommitContext& context,
                                                                               const std::optional<bool> currentProtected,
                                                                               const std::optional<bool> nextProtected) {
  const auto& overlay = context.overlay;
  if (!overlay.transportAvailable) return AutomaticCommitDisposition::Unavailable;
  const auto currentFilter = overlay.current.filterEnabled.status == PolicyObservationStatus::Available ? overlay.current.filterEnabled.value : std::nullopt;
  if (currentFilter == false) return AutomaticCommitDisposition::AlreadyPersistent;
  if (currentFilter != true) return AutomaticCommitDisposition::Unavailable;
  if (currentProtected == false) return AutomaticCommitDisposition::AlreadyPersistent;
  // Known unsupported modes pause automation before traversal. Unknown facts
  // remain failures; they never become a successful skipped operation.
  if (currentProtected != true || !context.hormEnabled) return AutomaticCommitDisposition::Unavailable;
  if (context.hormEnabled == true) return AutomaticCommitDisposition::SkipHormActive;
  if (context.fullCommit == true ||
      (context.romDisposition && (*context.romDisposition & compat::disposition::fullCommit) != 0))
    return AutomaticCommitDisposition::SkipFullCommit;
  // DisableHORM retains the HORM disposition bit until a fresh driver boot.
  // Manual commits keep their actual driver admission, but automation must not
  // silently resume during this maintenance transition (including service shutdown).
  if (context.romDisposition && (*context.romDisposition & compat::disposition::horm) != 0)
    return AutomaticCommitDisposition::SkipHormTransition;
  const auto flags = overlay.current.flags.status == PolicyObservationStatus::Available && overlay.current.flags.value
                         ? std::optional{overlay.current.flags.value->raw}
                         : std::nullopt;
  const auto readOnlyMedia = core::effectiveReadOnlyMedia({context.hormEnabled, flags, context.romDisposition, context.fullCommit});
  if (readOnlyMedia == true) return AutomaticCommitDisposition::SkipReadOnlyMedia;
  // Only retention depends on Next. Exiting ROM at the next startup cannot
  // make a Current ROM commit possible.
  if (nextProtected == true && overlay.preservesOverlayData()) return AutomaticCommitDisposition::SkipRetainedOverlay;
  if (!readOnlyMedia) return AutomaticCommitDisposition::Unavailable;
  return AutomaticCommitDisposition::CheckDurability;
}

}  // namespace uwf::app
