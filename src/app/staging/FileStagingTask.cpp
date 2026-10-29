// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "FileStagingTask.h"

#include <stdexcept>
#include <utility>

#include "app/uwf/UwfFilter.h"
#include "shared/Log.h"

namespace uwf::app {
FileStagingTask::FileStagingTask(wmi::WmiOperations& session, FileStagingStore& store, const UwfCapability capability,
                                 std::shared_ptr<const CommitTraversalSource> source)
    : m_store(&store), m_session(session.threadAccessor()), m_capability(capability), m_source(std::move(source)) {}
FileStagingTask::FileStagingTask(wmi::WmiOperations& session, QList<FileStagingEntry> entries, const UwfCapability capability,
                                 std::shared_ptr<const CommitTraversalSource> source)
    : m_entries(std::move(entries)), m_session(session.threadAccessor()), m_capability(capability), m_source(std::move(source)) {}
FileStagingTask::~FileStagingTask() { m_stop.request_stop(); }

bool FileStagingTask::pollPreparation() {
  if (m_failure) std::rethrow_exception(m_failure);
  try {
    if (preparationFinished()) return true;
    if (m_state == State::NotStarted) {
      if (m_entries) {
        m_sourceEntries = std::move(m_entries);
        m_entries.reset();
      } else {
        if (!m_store) throw std::logic_error("file staging store is unavailable");
        m_sourceEntries = m_store->load();
      }
      if (m_sourceEntries->isEmpty() || m_capability != UwfCapability::Available) {
        m_result.emplace();
        m_result->skippedEntries = static_cast<std::size_t>(m_sourceEntries->size());
        m_state = State::Completed;
        return true;
      }
      m_preparation = launchBackground(
          [entries = *m_sourceEntries, session = m_session, source = m_source, discovered = m_discovered](const std::stop_token stop) {
            Preparation prepared;
            if (stop.stop_requested()) return prepared;
            auto& transport = session();
            if (!provider::UwfFilter(transport).read().currentEnabled) {
              prepared.terminal.skippedEntries = static_cast<std::size_t>(entries.size());
              return prepared;
            }
            if (stop.stop_requested()) return prepared;
            FileStagingCommitter committer(transport, source);
            auto scan = FileStagingCommitter::scan(committer.prepareScan(entries), stop, discovered.get());
            prepared.operation = std::make_shared<FileStagingCommitOperation>(committer.beginCommit(committer.prepare(std::move(scan))));
            return prepared;
          },
          m_stop.get_token());
      m_state = State::Preparing;
      return false;
    }
    if (!m_preparation.isFinished()) return false;
    auto prepared = takeBackgroundResult(m_preparation);
    m_operation = std::move(prepared.operation);
    if (!m_operation || m_operation->finished()) {
      m_result = m_operation ? m_operation->result() : std::move(prepared.terminal);
      m_state = State::Completed;
    } else {
      m_progress->value.totalFiles = m_operation->totalFiles();
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
std::size_t FileStagingTask::discoveredDuringScan() const { return m_discovered->load(std::memory_order_relaxed); }
std::size_t FileStagingTask::totalFiles() const {
  if (!preparationFinished()) throw std::logic_error("file staging preparation is incomplete");
  const std::scoped_lock lock(m_progress->mutex);
  return m_progress->value.totalFiles;
}
const std::optional<QList<FileStagingEntry>>& FileStagingTask::sourceEntries() const { return m_sourceEntries; }

FileStagingTaskProgress FileStagingTask::advance() {
  if (m_failure) std::rethrow_exception(m_failure);
  try {
    if (!preparationFinished()) throw std::logic_error("file staging preparation is incomplete");
    if (finished()) throw std::logic_error("cannot advance a completed file staging task");
    if (m_state == State::Ready) {
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
