// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <functional>

#include "application/commit/contracts/CommitResult.h"

namespace uwf::app {
struct CommitBatchRequest {
  int total = 0;
  CommitOperation operation = CommitOperation::Persist;
  std::function<QString(int)> displayOf;
  std::function<CommitTargetResult(int, const QString&)> execute;
};
}  // namespace uwf::app
