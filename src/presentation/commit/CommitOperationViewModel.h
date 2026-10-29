// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <QObject>
#include <QTimer>

#include "app/api/CommitPreparation.h"
#include "app/ports/TextFileWriter.h"
#include "presentation/commit/CommitReportViewModel.h"
#include "presentation/common/AsyncTask.h"
#include "presentation/common/LoadingGate.h"

namespace uwf::presentation {
struct CommitOperationText {
  QString title, heading, description, count, detail, note, icon;
  QString primary, retry, close;
  bool primaryVisible = false, primaryEnabled = false, retryVisible = false;
  bool busy = false, activePath = false, writing = false, cancelEnabled = true, showReport = false;
  bool destructive = false, exportEnabled = false, warning = false, success = false, error = false;
};
class CommitOperationViewModel final : public QObject {
  Q_OBJECT
 public:
  enum class Phase { Idle, Preparing, Partial, Ready, Empty, Failed, Committing, Canceling, Complete, Canceled };
  CommitOperationViewModel(app::CommitRequest request, std::shared_ptr<app::CommitBackend> backend, QObject* parent = nullptr);
  ~CommitOperationViewModel() override;
  void start();
  void retry();
  void continuePartial();
  void commit();
  void proceed();
  [[nodiscard]] bool cancel();
  void exportRecords(const QString& path, app::TextFileWriter writer);
  [[nodiscard]] Phase phase() const { return m_phase; }
  [[nodiscard]] const app::CommitRequest& request() const { return m_request; }
  [[nodiscard]] const app::CommitTraversalProgress& progress() const { return m_progress; }
  [[nodiscard]] const app::CommitPlan& plan() const { return m_plan; }
  [[nodiscard]] const QList<CommitReportRow>& rows() const { return m_rows; }
  [[nodiscard]] int total() const { return static_cast<int>(m_targets.size()); }
  [[nodiscard]] int processed() const { return m_processed; }
  [[nodiscard]] int untouched() const { return m_untouched; }
  [[nodiscard]] CommitCompletion completion() const;
  [[nodiscard]] bool partialAccepted() const { return m_partialAccepted; }
  [[nodiscard]] CommitOperationText text() const;
 signals:
  void stateChanged();
  void exportFinished(const QString& error);

 private:
  struct LiveProgress;
  void prepare();
  app::CommitRequest m_request;
  std::shared_ptr<app::CommitBackend> m_backend;
  app::CommitPlan m_plan;
  app::CommitTraversalProgress m_progress;
  QList<app::CommitNode> m_targets;
  QList<CommitReportRow> m_rows;
  QString m_error;
  Phase m_phase = Phase::Idle;
  AsyncTask m_task{this};
  LoadingGate m_loading{this};
  QTimer m_pulse{this};
  std::shared_ptr<LiveProgress> m_live;
  int m_processed = 0;
  int m_untouched = 0;
  CommitCounts m_counts;
  bool m_partialAccepted = false;
  bool m_executionStarted = false;
};
}  // namespace uwf::presentation
