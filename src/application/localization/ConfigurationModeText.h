// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include "application/localization/ApplicationText.h"
#include "domain/uwf/policies/ConfigurationModePolicy.h"

namespace uwf::app {
[[nodiscard]] inline QString configurationModeRestrictionText(const core::ConfigurationWriteRestriction reason) {
  switch (reason) {
    case core::ConfigurationWriteRestriction::None:
      return {};
    case core::ConfigurationWriteRestriction::HormActive:
      return ApplicationText::tr("UWF settings cannot be changed while HORM is active.");
    case core::ConfigurationWriteRestriction::RomVolatile:
      return ApplicationText::tr(
          "Read-only ROM cannot persist ordinary UWF settings on the protected system volume. Disable UWF and restart before changing them.");
    case core::ConfigurationWriteRestriction::RomCommitUnavailable:
      return ApplicationText::tr("Threshold changes require a registry commit, which is unavailable in read-only ROM.");
    case core::ConfigurationWriteRestriction::Unknown:
      return ApplicationText::tr("Configuration persistence could not be confirmed. Refresh the system state before changing this setting.");
  }
  return {};
}
}  // namespace uwf::app
