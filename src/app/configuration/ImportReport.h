// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QString>

#include "UwfmgrCommands.h"

namespace uwf::app {

struct ImportResult {
  enum class Status { Success, Duplicate, Failed, Unsupported };
  Status status = Status::Failed;
  int lineNo = 0;
  QString lineText;
  QString detail;
};

// 生成解析错误的本地化说明；errorContext 保留导致错误的原始值。
QString parseErrorMessage(app::ParseError e, const QString& errorContext);

}  // namespace uwf::app
