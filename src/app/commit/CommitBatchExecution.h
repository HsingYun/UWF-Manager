// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include "app/api/CommitInteraction.h"
namespace uwf::app {
struct CommitBatchProgress {
  std::function<bool(int)> continueAt;
  std::function<void(int, const QString&)> targetChanged;
  std::function<void(const CommitTargetResult&, const QString&)> completed;
};
struct CommitBatchOutcome {
  int processed = 0;
  int untouched = 0;
};
[[nodiscard]] CommitBatchOutcome executeCommitBatch(const CommitBatchRequest& batch, const CommitBatchProgress& progress);
}  // namespace uwf::app
