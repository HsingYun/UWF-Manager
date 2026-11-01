// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include "application/localization/ApplicationText.h"
#include "domain/uwf/model/UwfModel.h"

namespace uwf::app {

[[nodiscard]] inline QString overlayTypeText(const core::OverlayType type) {
  switch (type) {
    case core::OverlayType::RAM:
      return QStringLiteral("RAM");
    case core::OverlayType::Disk:
      return ApplicationText::tr("Disk");
    case core::OverlayType::Unknown:
      break;
  }
  return ApplicationText::tr("Unknown");
}

}  // namespace uwf::app
