// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "CommitBatchExecution.h"
namespace uwf::app {
CommitBatchOutcome executeCommitBatch(const CommitBatchRequest& batch, const CommitBatchProgress& progress) {
  CommitBatchOutcome outcome;
  for (int index = 0; index < batch.total; ++index) {
    if (progress.continueAt && !progress.continueAt(index)) {
      outcome.untouched = batch.total - outcome.processed;
      break;
    }
    const QString display = batch.displayOf(index);
    if (progress.targetChanged) progress.targetChanged(index, display);
    auto result = batch.execute(index, display);
    if (progress.completed) progress.completed(result, display);
    ++outcome.processed;
  }
  return outcome;
}
}  // namespace uwf::app
