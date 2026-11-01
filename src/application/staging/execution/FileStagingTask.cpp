// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "application/staging/execution/FileStagingTask.h"

#include <stdexcept>
#include <utility>

#include "application/commit/contracts/CommitTraversalSource.h"
#include "foundation/logging/Log.h"

namespace uwf::app {
FileStagingTask::FileStagingTask(std::shared_ptr<UwfMaintenanceRepository> repository, QList<FileStagingEntry> entries, const UwfCapability capability,
                                 std::shared_ptr<const CommitTraversalSource> source)
    : m_entries(std::move(entries)), m_repository(std::move(repository)), m_capability(capability), m_source(std::move(source)) {
  if (!m_repository) throw std::invalid_argument("file staging requires a repository");
}
FileStagingTask::~FileStagingTask() { m_stop.request_stop(); }

bool FileStagingTask::pollPreparation() {
  if (m_failure) std::rethrow_exception(m_failure);
  try {
    if (preparationFinished()) return true;
    if (m_state == State::NotStarted) {
      if (m_entries.isEmpty()) {
        m_result.emplace();
        m_result->skippedEntries = static_cast<std::size_t>(m_entries.size());
        m_state = State::Completed;
        return true;
      }
      if (m_capability != UwfCapability::Available) throw std::runtime_error("The automatic file commit capability is unavailable");
      m_preparation = launchBackground(
          [entries = std::move(m_entries), repository = m_repository, source = m_source](const std::stop_token stop) {
            Preparation prepared;
            if (stop.stop_requested()) return prepared;
            FileStagingCommitter committer(repository, source);
            auto scan = FileStagingCommitter::scan(committer.prepareScan(entries), stop);
            auto plan = committer.prepare(std::move(scan));
            prepared.operation = std::make_shared<FileStagingCommitOperation>(committer.revalidate(plan, stop));
            prepared.plan = std::move(plan);
            return prepared;
          },
          m_stop.get_token());
      m_state = State::Preparing;
      return false;
    }
    if (!m_preparation.isFinished()) return false;
    auto prepared = takeBackgroundResult(m_preparation);
    m_operation = std::move(prepared.operation);
    m_plan = std::move(prepared.plan);
    m_progress->value = {0, m_operation ? m_operation->totalFiles() : 0, {}};
    if (m_operation) {
      m_automaticCommit = m_operation->result().automaticCommit;
      m_proposal = m_operation->proposal();
    }
    if (!m_operation || m_operation->finished()) {
      m_result = m_operation ? m_operation->result() : std::move(prepared.terminal);
      m_state = State::Completed;
    } else {
      m_state = State::Ready;
    }
    return true;
  } catch (...) {
    recordFailure("preparation");
    throw;
  }
}

bool FileStagingTask::preparationFinished() const { return m_state == State::Ready || m_state == State::Committing || m_state == State::Completed; }
bool FileStagingTask::finished() const { return m_state == State::Completed; }
bool FileStagingTask::failed() const { return static_cast<bool>(m_failure); }
std::size_t FileStagingTask::totalFiles() const {
  if (!preparationFinished()) throw std::logic_error("file staging preparation is incomplete");
  const std::scoped_lock lock(m_progress->mutex);
  return m_progress->value.totalFiles;
}
AutomaticCommitSummary FileStagingTask::automaticCommitSummary() const {
  if (!preparationFinished()) throw std::logic_error("file staging preparation is incomplete");
  if (m_result) return m_result->automaticCommit;
  return m_automaticCommit;
}
const AutomaticCommitProposal& FileStagingTask::proposal() const {
  if (!preparationFinished()) throw std::logic_error("file staging preparation is incomplete");
  return m_proposal;
}
void FileStagingTask::revalidatePreparation() {
  if (m_failure) std::rethrow_exception(m_failure);
  if (m_executionStarted || (m_state != State::Ready && m_state != State::Completed))
    throw std::logic_error("staging preparation cannot be revalidated during execution");
  if (!m_plan) return;  // The saved list was empty; no skip or write was certified.
  m_preparation = launchBackground(
      [repository = m_repository, source = m_source, plan = std::move(*m_plan)](const std::stop_token stop) mutable {
        Preparation prepared;
        if (stop.stop_requested()) return prepared;
        repository->refreshAutomaticCommitState();
        FileStagingCommitter committer(repository, source);
        prepared.operation = std::make_shared<FileStagingCommitOperation>(committer.revalidate(plan, stop));
        prepared.plan = std::move(plan);
        return prepared;
      }, m_stop.get_token());
  m_result.reset();
  m_operation.reset();
  m_state = State::Preparing;
}

FileStagingTaskProgress FileStagingTask::advance() {
  if (m_failure) std::rethrow_exception(m_failure);
  try {
    if (!preparationFinished()) throw std::logic_error("file staging preparation is incomplete");
    if (finished()) throw std::logic_error("cannot advance a completed file staging task");
    if (m_state == State::Ready) {
      m_executionStarted = true;
      m_commit = launchBackground(
          [operation = std::move(m_operation), progress = m_progress](const std::stop_token stop) {
            while (!operation->finished() && !stop.stop_requested()) {
              const auto path = operation->advance();
              const std::scoped_lock lock(progress->mutex);
              progress->value = {operation->processedFiles(), operation->totalFiles(), path};
            }
            return operation->result();
          },
          m_stop.get_token());
      m_state = State::Committing;
    }
    if (m_commit.isFinished()) {
      m_result = takeBackgroundResult(m_commit);
      m_state = State::Completed;
    }
    const std::scoped_lock lock(m_progress->mutex);
    return m_progress->value;
  } catch (...) {
    recordFailure("commit");
    throw;
  }
}
const FileStagingCommitResult& FileStagingTask::result() const {
  if (!finished() || !m_result) throw std::logic_error("file staging task result is not final");
  return *m_result;
}
void FileStagingTask::waitForWorker() {
  if (m_state == State::Preparing) m_preparation.waitForFinished();
  if (m_state == State::Committing) m_commit.waitForFinished();
}
void FileStagingTask::recordFailure(const char* phase) noexcept {
  if (m_failure) return;
  m_failure = std::current_exception();
  try {
    std::rethrow_exception(m_failure);
  } catch (const std::exception& error) {
    UWF_LOG_E("staging") << "file staging batch failed: phase=" << phase << " error=" << error.what();
  } catch (...) {
    UWF_LOG_E("staging") << "file staging batch failed: phase=" << phase << " error=non-standard-exception";
  }
}
}  // namespace uwf::app
