// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

namespace uwf::app {

// Skipping an automatic commit does not prove that data has reached the base
// volume. Explicit commits and list writes retain their own admission.
enum class AutomaticCommitDisposition {
  Unavailable,
  CheckDurability,
  AlreadyPersistent,
  SkipReadOnlyMedia,
  SkipRetainedOverlay,
  SkipHormActive,
  SkipHormTransition,
  SkipFullCommit,
};

[[nodiscard]] constexpr bool automaticFileCommitPaused(const AutomaticCommitDisposition disposition) {
  switch (disposition) {
    case AutomaticCommitDisposition::SkipReadOnlyMedia:
    case AutomaticCommitDisposition::SkipRetainedOverlay:
    case AutomaticCommitDisposition::SkipHormActive:
    case AutomaticCommitDisposition::SkipHormTransition:
    case AutomaticCommitDisposition::SkipFullCommit:
      return true;
    default:
      return false;
  }
}

// Shared by preparation, execution and presentation. Pausing an automatic
// action is distinct from certifying that its data is already persistent.
struct AutomaticCommitSummary {
  bool unavailable = false;
  bool readOnlyMediaPaused = false;
  bool retainedOverlayPaused = false;
  bool commitRequired = false;
  bool hormPaused = false;
  bool hormTransitionPaused = false;
  bool fullCommitPaused = false;
  bool alreadyPersistent = false;
  bool operator==(const AutomaticCommitSummary&) const = default;
  void include(const AutomaticCommitDisposition disposition) {
    switch (disposition) {
      case AutomaticCommitDisposition::Unavailable: unavailable = true; break;
      case AutomaticCommitDisposition::CheckDurability: commitRequired = true; break;
      case AutomaticCommitDisposition::AlreadyPersistent: alreadyPersistent = true; break;
      case AutomaticCommitDisposition::SkipReadOnlyMedia: readOnlyMediaPaused = true; break;
      case AutomaticCommitDisposition::SkipRetainedOverlay: retainedOverlayPaused = true; break;
      case AutomaticCommitDisposition::SkipHormActive: hormPaused = true; break;
      case AutomaticCommitDisposition::SkipHormTransition: hormTransitionPaused = true; break;
      case AutomaticCommitDisposition::SkipFullCommit: fullCommitPaused = true; break;
    }
  }
  [[nodiscard]] bool unsupported() const { return readOnlyMediaPaused || hormPaused || fullCommitPaused; }
  [[nodiscard]] bool paused() const { return unsupported() || retainedOverlayPaused || hormTransitionPaused; }
  [[nodiscard]] bool partiallyPaused() const { return paused() && commitRequired; }
};

}  // namespace uwf::app
