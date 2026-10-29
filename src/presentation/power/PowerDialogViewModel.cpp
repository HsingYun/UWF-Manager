// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "PowerDialogViewModel.h"

#include <QPointer>
#include <algorithm>
#include <chrono>
#include <exception>
#include <limits>
#include <stdexcept>
#include <utility>

#include "presentation/common/PresentationText.h"
#include "presentation/common/PresentationTiming.h"
namespace uwf::presentation {
using namespace app;
namespace {
int progressValue(const std::size_t value) { return static_cast<int>(std::min(value, static_cast<std::size_t>(std::numeric_limits<int>::max()))); }
}  // namespace
PowerDialogViewModel::PowerDialogViewModel(PowerActionDialogRequest request)
    : m_request(std::move(request)), m_preparationRequired(static_cast<bool>(m_request.pollStagingPreparation)), m_lifecycle(m_preparationRequired) {
  if (m_request.pollStagingPreparation && m_request.completedStagingFailure) {
    throw std::invalid_argument("power dialog cannot prepare and reuse a completed staging result simultaneously");
  }
  if (m_request.mode == PowerActionDialogMode::Direct && (m_request.pollStagingPreparation || m_request.completedStagingFailure)) {
    throw std::invalid_argument("direct power dialog cannot prepare or reuse file staging");
  }
  m_preparationTimer.setSingleShot(true);
  m_preparationTimer.setTimerType(Qt::PreciseTimer);
  m_stepTimer.setSingleShot(true);
  connect(&m_preparationTimer, &QTimer::timeout, this, &PowerDialogViewModel::pollPreparation);
  connect(&m_stepTimer, &QTimer::timeout, this, &PowerDialogViewModel::advanceStaging);
}
void PowerDialogViewModel::start() {
  if (m_request.completedStagingFailure) {
    showFailure(
        {PowerStagingState::Failed, 0, 0, {}, std::move(m_request.completedStagingFailure->details), std::move(m_request.completedStagingFailure->result)});
  } else if (m_preparationRequired) {
    m_preparationTimer.start(timing::kProgressUpdateInterval);
  }
}
void PowerDialogViewModel::showFailure(const PowerStagingProgress& status) {
  m_preparationTimer.stop();
  m_stepTimer.stop();
  m_failureCanContinue = status.failureResult && !status.failureResult->succeeded();
  m_lifecycle.post(PowerDialogState::Event::FailureObserved);
  recordProgress(status);
  emit failureObserved(status);
}
void PowerDialogViewModel::recordProgress(const PowerStagingProgress& status) {
  const int maximum = std::max(1, progressValue(status.totalFiles));
  if (m_displayedProgress > maximum) m_displayedProgress = -1;
  const int value = progressValue(status.processedFiles);
  if (value <= maximum) m_displayedProgress = value;
}
void PowerDialogViewModel::advanceStaging() {
  PowerStagingProgress status;
  try {
    status = m_advanceStaging();
  } catch (const std::exception& error) {
    status = {PowerStagingState::Failed,
              static_cast<std::size_t>(std::max(0, m_displayedProgress)),
              m_filesToCommit,
              {},
              PresentationText::tr("Automatic file staging stopped unexpectedly:\n%1").arg(QString::fromUtf8(error.what()))};
  } catch (...) {
    status = {PowerStagingState::Failed,
              static_cast<std::size_t>(std::max(0, m_displayedProgress)),
              m_filesToCommit,
              {},
              PresentationText::tr("Automatic file staging stopped because of an unknown error.")};
  }

  recordProgress(status);
  const QPointer<PowerDialogViewModel> self(this);
  emit progressChanged(status);
  if (!self || m_lifecycle.terminal()) return;
  switch (status.state) {
    case PowerStagingState::InProgress:
      m_stepTimer.start(timing::kProgressUpdateInterval);
      break;
    case PowerStagingState::Completed:
      m_lifecycle.post(PowerDialogState::Event::StagingCompleted);
      emit accepted();
      break;
    case PowerStagingState::Failed:
      showFailure(status);
      break;
  }
}
void PowerDialogViewModel::pollPreparation() {
  if (m_directConfirmationOpen) return;
  if (!m_completedPreparation) {
    try {
      auto prepared = m_request.pollStagingPreparation();
      if (prepared) m_completedPreparation.emplace(std::move(*prepared));
    } catch (const std::exception& error) {
      m_completedPreparation.emplace(
          PowerStagingFailure{PresentationText::tr("Automatic file staging stopped unexpectedly:\n%1").arg(QString::fromUtf8(error.what()))});
    } catch (...) {
      m_completedPreparation.emplace(PowerStagingFailure{PresentationText::tr("Automatic file staging stopped because of an unknown error.")});
    }
  }

  // 首次轮询已经返回，事件循环接下来即可完成进度条绘制；从这里开始计算
  // 最短展示时间，不能把首次同步探测占用的时间算作用户已经看见的时间。
  if (!m_preparationElapsed.isValid()) m_preparationElapsed.start();

  if (!m_completedPreparation) {
    m_preparationTimer.start(timing::kProgressUpdateInterval);
    return;
  }

  // Keep the calculating state visible for the shared minimum duration.
  // Only result publication is delayed; polling and the event loop remain responsive.
  const auto elapsed = std::chrono::milliseconds{m_preparationElapsed.elapsed()};
  const auto remaining = timing::kMinimumProgressDisplayDuration - elapsed;
  if (remaining > std::chrono::milliseconds::zero()) {
    m_preparationTimer.start(remaining);
    return;
  }

  m_request.pollStagingPreparation = {};
  PowerStagingPreparation prepared = std::move(*m_completedPreparation);
  m_completedPreparation.reset();
  if (const auto* failure = std::get_if<PowerStagingFailure>(&prepared)) {
    showFailure({PowerStagingState::Failed, 0, 0, {}, failure->details, failure->result});
    return;
  }

  if (auto* commit = std::get_if<PowerStagingCommit>(&prepared)) {
    m_filesToCommit = commit->filesToCommit;
    m_advanceStaging = std::move(commit->advanceStaging);
  }

  m_lifecycle.post(PowerDialogState::Event::PreparationReady);
  emit preparationReady();
}
void PowerDialogViewModel::confirm() {
  switch (m_lifecycle.state()) {
    case PowerDialogState::State::FailureDecision:
      if (!m_failureCanContinue) return;
      m_lifecycle.post(PowerDialogState::Event::Confirm);
      emit accepted();
      return;
    case PowerDialogState::State::Confirmation:
      if (!m_advanceStaging) {
        m_lifecycle.post(PowerDialogState::Event::Confirm);
        emit accepted();
        return;
      }
      break;
    case PowerDialogState::State::Preparing:
    case PowerDialogState::State::Staging:
    case PowerDialogState::State::Confirmed:
    case PowerDialogState::State::Canceled:
    case PowerDialogState::State::Count:
      return;
  }

  m_lifecycle.post(PowerDialogState::Event::BeginStaging);
  m_stagingStarted = true;
  m_displayedProgress = 0;
  m_stepTimer.start(0);
  emit stagingBegan();
}
void PowerDialogViewModel::confirmDirect() {
  if (m_lifecycle.state() != PowerDialogState::State::Preparing && m_lifecycle.state() != PowerDialogState::State::Confirmation) return;
  m_preparationTimer.stop();
  m_stepTimer.stop();
  m_directSelected = true;
  emit accepted();
}
void PowerDialogViewModel::beginDirectConfirmation() {
  m_directConfirmationOpen = true;
  m_preparationTimer.stop();
}
void PowerDialogViewModel::cancelDirectConfirmation() {
  if (!m_directConfirmationOpen) return;
  m_directConfirmationOpen = false;
  if (m_lifecycle.state() == PowerDialogState::State::Preparing) m_preparationTimer.start(timing::kProgressUpdateInterval);
}
PowerActionDialogOutcome PowerDialogViewModel::finish(const bool accepted) {
  if (m_directSelected) return PowerActionDialogOutcome::DirectConfirmed;
  if (!m_lifecycle.terminal()) {
    const bool mayAccept = accepted && (m_lifecycle.state() != PowerDialogState::State::FailureDecision || m_failureCanContinue);
    m_lifecycle.post(mayAccept ? PowerDialogState::Event::Confirm : PowerDialogState::Event::Cancel);
  }
  const auto outcome = m_lifecycle.outcome();
  if (m_request.mode == PowerActionDialogMode::Direct && outcome == PowerActionDialogOutcome::Confirmed) return PowerActionDialogOutcome::DirectConfirmed;
  return outcome;
}
}  // namespace uwf::presentation

#include "moc_PowerDialogViewModel.cpp"
