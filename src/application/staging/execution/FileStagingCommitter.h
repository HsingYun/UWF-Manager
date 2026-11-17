// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QList>
#include <QString>
#include <QStringList>
#include <cstddef>
#include <map>
#include <stop_token>
#include <vector>

#include "application/commit/contracts/CommitPlan.h"
#include "application/commit/contracts/CommitTraversalSource.h"
#include "application/staging/contracts/FileStagingTypes.h"
#include "application/uwf/contracts/UwfMaintenanceRepository.h"

namespace uwf::app {

class FileStagingCommitOperation;
class FileStagingCommitter;

// Resolve durability before scanning so already-persistent directories do not
// require recursive enumeration during shutdown.
class FileStagingScanPlan {
 private:
  friend class FileStagingCommitter;

  std::shared_ptr<const CommitTraversalSource> m_source;

  QList<CommitNode> m_entries;
  QList<CommitNode> m_pausedTargets;
  QList<CommitNode> m_persistentTargets;
  std::map<std::string, std::vector<std::string>> m_fileExclusions;
  QList<CommitNode> m_selectedTargets;
  std::vector<std::shared_ptr<UwfVolumeTarget>> m_volumes;
  std::size_t m_skippedEntries = 0;
  QList<FileStagingCommitFailure> m_failures;
};

// Scanning uses the prepared volume/exclusion snapshot and performs no UWF calls.
class FileStagingScan {
 private:
  friend class FileStagingCommitter;

  std::shared_ptr<const CommitTraversalSource> m_source;

  QList<CommitNode> m_files;
  QList<CommitNode> m_pausedTargets;
  QList<CommitNode> m_persistentTargets;
  std::map<std::string, std::vector<std::string>> m_fileExclusions;
  QList<CommitNode> m_selectedTargets;
  std::vector<std::shared_ptr<UwfVolumeTarget>> m_volumes;
  std::size_t m_skippedFiles = 0;
  std::size_t m_skippedEntries = 0;
  QList<FileStagingCommitFailure> m_failures;
};

// Freeze the discovered file set; later directory additions are excluded from
// this batch. Skipped entries and scan failures remain explicit results.
class FileStagingCommitPlan {
 public:
  [[nodiscard]] std::size_t fileCount() const { return static_cast<std::size_t>(m_files.size()); }

 private:
  friend class FileStagingCommitter;
  friend class FileStagingCommitOperation;

  std::shared_ptr<const CommitTraversalSource> m_source;

  QList<CommitNode> m_files;
  QList<CommitNode> m_pausedTargets;
  QList<CommitNode> m_persistentTargets;
  QList<CommitNode> m_selectedTargets;
  std::vector<std::shared_ptr<UwfVolumeTarget>> m_volumes;
  std::size_t m_discoveredFiles = 0;
  std::size_t m_skippedFiles = 0;
  std::size_t m_skippedEntries = 0;
  AutomaticCommitSummary m_automaticCommit{};
  std::map<std::string, AutomaticCommitDisposition> m_dispositions;
  QList<FileStagingCommitFailure> m_failures;
};

// Synchronous execution primitive. FileStagingTask runs it on a worker and
// publishes progress; callers must not execute provider calls on the UI thread.
class FileStagingCommitOperation {
 public:
  [[nodiscard]] bool finished() const { return m_nextFile >= m_files.size(); }
  [[nodiscard]] std::size_t totalFiles() const { return static_cast<std::size_t>(m_files.size()); }
  [[nodiscard]] std::size_t processedFiles() const { return static_cast<std::size_t>(m_nextFile); }
  [[nodiscard]] const FileStagingCommitResult& result() const { return m_result; }
  [[nodiscard]] const AutomaticCommitProposal& proposal() const { return m_proposal; }

  // Return the processed path; calling after finished() is a logic error.
  [[nodiscard]] QString advance();

 private:
  friend class FileStagingCommitter;

  FileStagingCommitOperation(const std::shared_ptr<UwfMaintenanceRepository>& repository, FileStagingCommitPlan plan);

  std::shared_ptr<UwfMaintenanceRepository> m_repository;
  std::shared_ptr<const CommitTraversalSource> m_source;
  QList<CommitNode> m_files;
  std::vector<std::shared_ptr<UwfVolumeTarget>> m_volumes;
  qsizetype m_nextFile = 0;
  FileStagingCommitResult m_result;
  AutomaticCommitProposal m_proposal{};
};

// Expand saved entries without following reparse points; deduplicate overlapping
// selections by object identity. The power workflow decides whether failures permit shutdown.
class FileStagingCommitter {
 public:
  FileStagingCommitter(std::shared_ptr<UwfMaintenanceRepository> repository, std::shared_ptr<const CommitTraversalSource> source);

  // Provider snapshot failures propagate to the batch owner, without target results.
  [[nodiscard]] FileStagingScanPlan prepareScan(const QList<app::FileStagingEntry>& entries) const;
  [[nodiscard]] static FileStagingScan scan(FileStagingScanPlan plan, std::stop_token stopToken = {});
  [[nodiscard]] FileStagingCommitPlan prepare(FileStagingScan scan) const;
  // Revalidate live durability before execution, including all-skipped plans.
  // Required-read/admission failures propagate to the batch owner.
  [[nodiscard]] FileStagingCommitOperation beginCommit(FileStagingCommitPlan plan) const;
  // Reuse scanned objects, discovering only scopes which were previously skipped
  // and now need a commit. The updated plan remains available for confirmation.
  [[nodiscard]] FileStagingCommitOperation revalidate(FileStagingCommitPlan& plan, std::stop_token stop = {}) const;

 private:
  std::shared_ptr<UwfMaintenanceRepository> m_repository;
  std::shared_ptr<const CommitTraversalSource> m_source;
};

}  // namespace uwf::app
