// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <exception>
#include <memory>
#include <mutex>
#include <optional>

#include "application/commit/contracts/CommitTraversalSource.h"
#include "application/execution/BackgroundWork.h"
#include "application/staging/execution/FileStagingCommitter.h"
#include "application/uwf/contracts/UwfCapability.h"
#include "application/uwf/contracts/UwfMaintenanceRepository.h"

namespace uwf::app {
struct FileStagingTaskProgress {
  std::size_t processedFiles = 0;
  std::size_t totalFiles = 0;
  QString currentPath;
};

// Prepare a saved-list snapshot on a worker; advance() authorizes execution.
// Workers own operation/progress state. Destruction requests cancellation
// without waiting for I/O or accessing the destroyed owner.
class FileStagingTask final {
 public:
  FileStagingTask(std::shared_ptr<UwfMaintenanceRepository> repository, QList<FileStagingEntry> entries, UwfCapability capability,
                  std::shared_ptr<const CommitTraversalSource> source);
  ~FileStagingTask();
  FileStagingTask(const FileStagingTask&) = delete;
  FileStagingTask& operator=(const FileStagingTask&) = delete;

  [[nodiscard]] bool pollPreparation();
  [[nodiscard]] bool preparationFinished() const;
  [[nodiscard]] bool finished() const;
  // A failed task retains its original exception and owns its single batch log.
  [[nodiscard]] bool failed() const;
  [[nodiscard]] std::size_t totalFiles() const;
  [[nodiscard]] AutomaticCommitSummary automaticCommitSummary() const;
  [[nodiscard]] const AutomaticCommitProposal& proposal() const;
  // Refresh admission and reuse the prepared discovery before final confirmation.
  // Never call after execution starts. New work is prepared, not dispatched.
  void revalidatePreparation();
  // Starts or polls execution; never blocks on a provider call.
  [[nodiscard]] FileStagingTaskProgress advance();
  [[nodiscard]] const FileStagingCommitResult& result() const;
  // Only non-UI callers may wait instead of polling (for example the service).
  void waitForWorker();

 private:
  void recordFailure(const char* phase) noexcept;
  enum class State { NotStarted, Preparing, Ready, Committing, Completed };
  struct Preparation {
    std::shared_ptr<FileStagingCommitOperation> operation;
    std::optional<FileStagingCommitPlan> plan;
    FileStagingCommitResult terminal;
  };
  struct Progress {
    std::mutex mutex;
    FileStagingTaskProgress value;
  };
  QList<FileStagingEntry> m_entries;
  std::shared_ptr<UwfMaintenanceRepository> m_repository;
  UwfCapability m_capability;
  std::shared_ptr<const CommitTraversalSource> m_source;
  State m_state = State::NotStarted;
  bool m_executionStarted = false;
  std::shared_ptr<Progress> m_progress = std::make_shared<Progress>();
  std::stop_source m_stop;
  QFuture<BackgroundResult<Preparation>> m_preparation;
  QFuture<BackgroundResult<FileStagingCommitResult>> m_commit;
  std::shared_ptr<FileStagingCommitOperation> m_operation;
  std::optional<FileStagingCommitPlan> m_plan;
  AutomaticCommitSummary m_automaticCommit{};
  AutomaticCommitProposal m_proposal{};
  std::optional<FileStagingCommitResult> m_result;
  std::exception_ptr m_failure;
};
}  // namespace uwf::app
