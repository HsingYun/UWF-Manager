// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

namespace uwf::app {
// Result of one configuration import edit; independent of its draft storage.
enum class ExclusionImportOutcome {
  Applied,                // The pending edit changed.
  NoOp,                   // The requested edit already holds.
  RejectedNotOnVolume,    // File path does not belong to this volume.
  RejectedForbidden,      // UWF forbids this exclusion.
  RejectedConflict,       // The path intersects staged files on this volume.
  RejectedMissingVolume,  // No Next registration or planned Protect creation.
  RejectedRequired,       // Staging still requires this registry exclusion.
  RejectedReadOnly,       // The current editing admission does not allow changes.
};
}  // namespace uwf::app
