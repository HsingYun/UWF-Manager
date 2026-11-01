// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include "application/commit/CommitBatchExecution.h"
#include "application/commit/contracts/CommitBatchRequest.h"
#include "presentation/viewmodels/commit/CommitReportViewModel.h"
namespace uwf::presentation {
class CommitBatchViewModel final {
 public:
  // The caller keeps both references alive throughout run(); targets follow batch order.
  CommitBatchViewModel(const app::CommitBatchRequest& batch, const QList<app::CommitNode>& targets) : m_batch(batch), m_targets(targets) {}
  void run(const app::CommitBatchProgress& progress);
  [[nodiscard]] const QList<CommitReportRow>& rows() const { return m_rows; }
  [[nodiscard]] int untouched() const { return m_untouched; }

 private:
  const app::CommitBatchRequest& m_batch;
  const QList<app::CommitNode>& m_targets;
  QList<CommitReportRow> m_rows;
  int m_untouched = 0;
};
}  // namespace uwf::presentation
