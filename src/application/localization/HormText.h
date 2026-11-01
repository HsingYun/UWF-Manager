// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include "application/localization/ApplicationText.h"
#include "application/overlay/contracts/HormEligibilityTypes.h"

namespace uwf::app {
[[nodiscard]] inline QString hormIssueText(const HormIssue issue) {
  switch (issue) {
    case HormIssue::None:
      return {};
    case HormIssue::Unsupported:
      return ApplicationText::tr("HORM is not available on this system.");
    case HormIssue::StateUnknown:
      return ApplicationText::tr("The HORM state is unknown. Refresh before continuing.");
    case HormIssue::FullCommitActive:
      return ApplicationText::tr("Full volume commit has started. Restart before enabling HORM.");
    case HormIssue::ProtectionStateChanged:
      return ApplicationText::tr("The protection state changed during this startup. Restart before enabling HORM.");
    case HormIssue::PrerequisitesUnknown:
      return ApplicationText::tr("HORM prerequisites could not be verified. Read the state again before continuing.");
    case HormIssue::FilterDisabled:
      return ApplicationText::tr("Enable UWF and restart before enabling HORM.");
    case HormIssue::OverlayNotRam:
      return ApplicationText::tr("HORM requires the current RAM overlay.");
    case HormIssue::ConfigurationIncompatible:
      return ApplicationText::tr("HORM requires the current RAM Read-Only Media configuration.");
    case HormIssue::HibernationUnavailable:
      return ApplicationText::tr("Full hibernation is not available on this computer");
    case HormIssue::VolumesUnprotected:
      return ApplicationText::tr("A mounted fixed disk failed the protection check. Review the listed disks before enabling HORM.");
  }
  return {};
}
}  // namespace uwf::app
