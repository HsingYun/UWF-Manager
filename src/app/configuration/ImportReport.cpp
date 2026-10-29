// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "ImportReport.h"

#include "app/common/ApplicationText.h"

namespace uwf::app {

QString parseErrorMessage(app::ParseError e, const QString& errorContext) {
  switch (e) {
    case app::ParseError::None:
    case app::ParseError::Comment:
      return {};  // 非错误状态无须说明
    case app::ParseError::Incomplete:
      return ApplicationText::tr("Incomplete uwfmgr command");
    case app::ParseError::MissingSizeArg:
      return ApplicationText::tr("Missing size argument (MB)");
    case app::ParseError::InvalidSize:
      return ApplicationText::tr("Size must be a non-negative integer in MB");
    case app::ParseError::MissingTypeArg:
      return ApplicationText::tr("Missing overlay type argument (RAM or Disk)");
    case app::ParseError::UnknownType:
      return ApplicationText::tr("Unknown overlay type %1 (expected RAM or Disk)").arg(errorContext);
    case app::ParseError::MissingVolumeArg:
      return ApplicationText::tr("Missing volume argument (e.g. C:)");
    case app::ParseError::InvalidVolume:
      return ApplicationText::tr("Volume must be a drive letter such as C:");
    case app::ParseError::MissingPathArg:
      return ApplicationText::tr("Missing path argument");
    case app::ParseError::MissingRegistryKeyArg:
      return ApplicationText::tr("Missing registry key argument");
    case app::ParseError::MalformedQuoting:
      return ApplicationText::tr("Unclosed double quote in command");
    case app::ParseError::UnexpectedArgument:
      return ApplicationText::tr("Unexpected extra argument: %1").arg(errorContext);
    case app::ParseError::Unsupported:
      return ApplicationText::tr("Unsupported uwfmgr command (cannot be mapped to a UI action)");
  }
  return {};
}

}  // namespace uwf::app
