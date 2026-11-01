// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QString>

#include "application/configuration/commands/UwfmgrCommand.h"

namespace uwf::app {

struct ImportResult {
  enum class Status { Success, Duplicate, Failed, Unsupported };
  Status status = Status::Failed;
  int lineNo = 0;
  QString lineText;
  QString detail;
};

// Localize the parse error while preserving the original offending argument.
QString parseErrorMessage(app::ParseError e, const QString& errorContext);

}  // namespace uwf::app
