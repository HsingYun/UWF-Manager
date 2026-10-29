// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "CommitBatchViewModel.h"
namespace uwf::presentation {
void CommitBatchViewModel::run(const app::CommitBatchProgress& progress) {
  m_rows.clear();
  auto observer = progress;
  observer.completed = [this, completed = progress.completed](const app::CommitTargetResult& result, const QString& display) {
    m_rows.append(makeCommitReportRow(result, display, m_batch.operation));
    if (completed) completed(result, display);
  };
  m_untouched = app::executeCommitBatch(m_batch, observer).untouched;
}
}  // namespace uwf::presentation
