// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "FileStagingProgressViewModel.h"

#include <QPointer>
#include <chrono>
#include <exception>
#include <stdexcept>
#include <utility>

#include "FileStagingResultText.h"
#include "presentation/common/PresentationText.h"
#include "presentation/common/PresentationTiming.h"
#include "shared/Log.h"
namespace uwf::presentation {
namespace {
void notifyProgress(std::function<void(std::size_t, std::size_t)> progress, const std::size_t processed, const std::size_t total) noexcept {
  if (!progress) return;
  try {
    progress(processed, total);
  } catch (const std::exception& error) {
    UWF_LOG_E("staging") << "file staging progress observer failed: error=" << error.what();
  } catch (...) {
    UWF_LOG_E("staging") << "file staging progress observer failed: error=non-standard-exception";
  }
}

}  // namespace
FileStagingProgressViewModel::FileStagingProgressViewModel(std::unique_ptr<app::FileStagingTask> task, app::FileStagingCoordinator::Completion completion,
                                                           app::FileStagingCoordinator::Progress progress)
    : m_task(std::move(task)), m_completion(std::move(completion)), m_progressObserver(std::move(progress)) {
  if (!m_task) throw std::invalid_argument("file staging progress requires a task");
  m_timer.setSingleShot(true);
  m_timer.setTimerType(Qt::PreciseTimer);
  connect(&m_timer, &QTimer::timeout, this, &FileStagingProgressViewModel::advance);
}
void FileStagingProgressViewModel::start() {
  m_visibleElapsed.start();
  m_timer.start(0);
  emit presentationRequested();
}
void FileStagingProgressViewModel::disposeAfterCallback() { deleteLater(); }
void FileStagingProgressViewModel::advance() {
  const QPointer<FileStagingProgressViewModel> self(this);
  try {
    if (!m_task->preparationFinished()) {
      if (!m_task->pollPreparation()) {
        const std::size_t discovered = m_task->discoveredDuringScan();
        if (discovered != m_lastReportedProgress) {
          m_lastReportedProgress = discovered;
          notifyProgress(m_progressObserver, discovered, 0);
          if (!self) return;
        }
        m_timer.start(timing::kProgressUpdateInterval);
        return;
      }
      m_state.phase = Phase::Committing;
      m_state.total = m_task->totalFiles();
      m_state.processed = 0;
      emit stateChanged();
      if (!self) return;
    }
    if (!m_task->finished()) {
      const auto progress = m_task->advance();
      m_state.total = progress.totalFiles;
      m_state.processed = progress.processedFiles;
      m_state.currentPath = progress.currentPath;
      m_state.hasCommitProgress = true;
      emit stateChanged();
      if (!self) return;
      notifyProgress(m_progressObserver, progress.processedFiles, progress.totalFiles);
      if (!self) return;
      m_timer.start(timing::kProgressUpdateInterval);
      return;
    }
    app::FileStagingBatchResult batch{toApplicationCommandResult(m_task->result()), m_task->sourceEntries(), m_task->result()};
    finishAfterMinimumDuration(std::move(batch));
  } catch (const std::exception& error) {
    if (!self) return;
    if (!m_task->failed()) UWF_LOG_E("staging") << "file staging progress failed: error=" << error.what();
    finishAfterMinimumDuration({app::ApplicationCommandResult::infrastructureFailure(QString::fromUtf8(error.what())), m_task->sourceEntries()});
  } catch (...) {
    if (!self) return;
    if (!m_task->failed()) UWF_LOG_E("staging") << "file staging progress failed: error=non-standard-exception";
    finishAfterMinimumDuration(
        {app::ApplicationCommandResult::infrastructureFailure(PresentationText::tr("Automatic file staging stopped because of an unknown error.")),
         m_task->sourceEntries()});
  }
}
void FileStagingProgressViewModel::finishAfterMinimumDuration(app::FileStagingBatchResult result) {
  if (m_finalResult) return;
  m_finalResult = std::move(result);
  m_state.phase = Phase::Finished;
  m_state.outcome = m_finalResult->command.outcome;
  m_state.detail = m_finalResult->command.detail;
  const auto elapsed = std::chrono::milliseconds{m_visibleElapsed.elapsed()};
  const auto remaining = timing::kMinimumProgressDisplayDuration - elapsed;
  if (remaining > std::chrono::milliseconds::zero()) {
    m_timer.stop();
    QTimer::singleShot(remaining, this, [this] { complete(); });
    emit stateChanged();
    return;
  }
  const QPointer<FileStagingProgressViewModel> self(this);
  emit stateChanged();
  if (!self) return;
  complete();
}
void FileStagingProgressViewModel::complete() {
  if (!m_finalResult) return;
  auto completion = std::move(m_completion);
  auto result = std::move(*m_finalResult);
  m_finalResult.reset();
  emit presentationFinished();
  completion(result);
}
}  // namespace uwf::presentation
#include "moc_FileStagingProgressViewModel.cpp"
