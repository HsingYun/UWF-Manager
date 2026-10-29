// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include "app/commit/CommitBatchExecution.h"
#include "presentation/commit/CommitReportViewModel.h"
namespace uwf::presentation {
class CommitBatchViewModel final {
 public:
  explicit CommitBatchViewModel(const app::CommitBatchRequest& batch) : m_batch(batch) {}
  void run(const app::CommitBatchProgress& progress);
  [[nodiscard]] const QList<CommitReportRow>& rows() const { return m_rows; }
  [[nodiscard]] int untouched() const { return m_untouched; }

 private:
  const app::CommitBatchRequest& m_batch;
  QList<CommitReportRow> m_rows;
  int m_untouched = 0;
};
}  // namespace uwf::presentation
