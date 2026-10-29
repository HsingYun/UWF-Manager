// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <QElapsedTimer>
#include <QObject>
#include <QTimer>
#include <memory>
#include <optional>

#include "app/staging/FileStagingCoordinator.h"
#include "app/staging/FileStagingTask.h"
namespace uwf::presentation {
class FileStagingProgressViewModel final : public QObject, public app::FileStagingCommandOperation {
  Q_OBJECT
 public:
  enum class Phase { Preparing, Committing, Finished };
  struct State {
    Phase phase = Phase::Preparing;
    std::size_t processed = 0;
    std::size_t total = 0;
    QString currentPath;
    bool hasCommitProgress = false;
    app::ApplicationCommandOutcome outcome = app::ApplicationCommandOutcome::Succeeded;
    QString detail;
  };
  FileStagingProgressViewModel(std::unique_ptr<app::FileStagingTask> task, app::FileStagingCoordinator::Completion completion,
                               app::FileStagingCoordinator::Progress progress);
  [[nodiscard]] const State& state() const { return m_state; }
  void start() override;
  void disposeAfterCallback() override;
 signals:
  void presentationRequested();
  void stateChanged();
  void presentationFinished();

 private:
  void advance();
  void finishAfterMinimumDuration(app::FileStagingBatchResult result);
  void complete();
  std::unique_ptr<app::FileStagingTask> m_task;
  app::FileStagingCoordinator::Completion m_completion;
  app::FileStagingCoordinator::Progress m_progressObserver;
  State m_state;
  QTimer m_timer;
  QElapsedTimer m_visibleElapsed;
  std::optional<app::FileStagingBatchResult> m_finalResult;
  std::size_t m_lastReportedProgress = 0;
};
}  // namespace uwf::presentation
