// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "presentation/viewmodels/common/RuntimeWriteText.h"

#include "presentation/viewmodels/common/PresentationText.h"

namespace uwf::presentation {
QString runtimeWriteRestrictionText(const app::RuntimeWriteRestriction restriction) {
  switch (restriction) {
    case app::RuntimeWriteRestriction::None:
      return {};
    case app::RuntimeWriteRestriction::HormRestartRequired:
      return PresentationText::tr("HORM is disabled. Restart the computer before changing UWF settings.");
    case app::RuntimeWriteRestriction::HormActive:
      return PresentationText::tr("UWF settings cannot be changed while HORM is active.");
    case app::RuntimeWriteRestriction::HormUnknown:
      return PresentationText::tr("The HORM state is unknown. Read the state again before continuing.");
    case app::RuntimeWriteRestriction::ReadOnlyMediaActive:
      return PresentationText::tr("Read-only media mode does not support individual commits or automatic saving of staged files.");
    case app::RuntimeWriteRestriction::StateUnavailable:
      return PresentationText::tr("UWF is currently unavailable");
  }
  return {};
}
}  // namespace uwf::presentation
