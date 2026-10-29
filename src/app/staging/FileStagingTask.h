// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <atomic>
#include <exception>
#include <memory>
#include <mutex>
#include <optional>

#include "FileStagingCommitter.h"
#include "app/api/UwfCapability.h"
#include "app/common/BackgroundWork.h"
#include "app/ports/FileStagingStore.h"

namespace uwf::app {
struct FileStagingTaskProgress {
  std::size_t processedFiles = 0;
  std::size_t totalFiles = 0;
  QString currentPath;
};

// Shared by safe power, command batches and the service. Preparation reads one
// saved-list snapshot, then queries and traverses on a worker. Writes begin only
// at the first advance(), after the power workflow has obtained confirmation.
// Workers own their operation and progress; destroying the task requests stop
// without waiting for filesystem or provider calls or touching a dead receiver.
class FileStagingTask final {
 public:
  FileStagingTask(wmi::WmiOperations& session, FileStagingStore& store, UwfCapability capability, std::shared_ptr<const CommitTraversalSource> source);
  FileStagingTask(wmi::WmiOperations& session, QList<FileStagingEntry> entries, UwfCapability capability, std::shared_ptr<const CommitTraversalSource> source);
  ~FileStagingTask();
  FileStagingTask(const FileStagingTask&) = delete;
  FileStagingTask& operator=(const FileStagingTask&) = delete;

  [[nodiscard]] bool pollPreparation();
  [[nodiscard]] bool preparationFinished() const;
  [[nodiscard]] bool finished() const;
  // A failed task retains its original exception and owns its single batch log.
  [[nodiscard]] bool failed() const;
  [[nodiscard]] std::size_t discoveredDuringScan() const;
  [[nodiscard]] std::size_t totalFiles() const;
  [[nodiscard]] const std::optional<QList<FileStagingEntry>>& sourceEntries() const;
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
    FileStagingCommitResult terminal;
  };
  struct Progress {
    std::mutex mutex;
    FileStagingTaskProgress value;
  };
  FileStagingStore* m_store = nullptr;
  std::optional<QList<FileStagingEntry>> m_entries;
  std::function<wmi::WmiOperations&()> m_session;
  UwfCapability m_capability;
  std::shared_ptr<const CommitTraversalSource> m_source;
  State m_state = State::NotStarted;
  std::optional<QList<FileStagingEntry>> m_sourceEntries;
  std::shared_ptr<std::atomic_size_t> m_discovered = std::make_shared<std::atomic_size_t>(0);
  std::shared_ptr<Progress> m_progress = std::make_shared<Progress>();
  std::stop_source m_stop;
  QFuture<BackgroundResult<Preparation>> m_preparation;
  QFuture<BackgroundResult<FileStagingCommitResult>> m_commit;
  std::shared_ptr<FileStagingCommitOperation> m_operation;
  std::optional<FileStagingCommitResult> m_result;
  std::exception_ptr m_failure;
};
}  // namespace uwf::app
