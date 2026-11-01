// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <QElapsedTimer>
#include <QObject>
#include <QTimer>
#include <chrono>

#include "presentation/viewmodels/power/PowerDialogState.h"
#include "presentation/viewmodels/common/AutomaticCommitText.h"
namespace uwf::presentation {

class PowerDialogViewModel final : public QObject {
  Q_OBJECT
 public:
  explicit PowerDialogViewModel(app::PowerActionDialogRequest request);
  PowerDialogViewModel(app::PowerActionDialogRequest request, std::chrono::milliseconds minimumPreparationDuration);
  [[nodiscard]] bool preparationRequired() const { return m_preparationRequired; }
  [[nodiscard]] std::size_t filesToCommit() const { return m_filesToCommit; }
  [[nodiscard]] bool hasStaging() const { return static_cast<bool>(m_advanceStaging); }
  [[nodiscard]] const app::AutomaticCommitSummary& automaticCommitSummary() const { return m_automaticCommit; }
  [[nodiscard]] AutomaticCommitPresentation automaticCommitPresentation() const;
  [[nodiscard]] bool stagingStarted() const { return m_stagingStarted; }
  [[nodiscard]] bool canContinueAfterStagingFailure() const { return m_failureCanContinue; }
  [[nodiscard]] PowerDialogState::State state() const { return m_lifecycle.state(); }
  void start();
  void confirm();
  void confirmDirect();
  void beginDirectConfirmation();
  void cancelDirectConfirmation();
  [[nodiscard]] app::PowerActionDialogOutcome finish(bool accepted);
 signals:
  void failureObserved(const uwf::app::PowerStagingProgress& status);
  void progressChanged(const uwf::app::PowerStagingProgress& status);
  void preparationReady();
  void stagingBegan();
  void preparationBegan();
  void accepted();

 private:
  void pollPreparation();
  void advanceStaging();
  void showFailure(const app::PowerStagingProgress& status);
  void recordProgress(const app::PowerStagingProgress& status);
  app::PowerActionDialogRequest m_request;
  bool m_preparationRequired;
  PowerDialogState m_lifecycle;
  std::size_t m_filesToCommit = 0;
  app::AutomaticCommitSummary m_automaticCommit{};
  std::function<app::PowerStagingProgress()> m_advanceStaging;
  QTimer m_preparationTimer;
  QElapsedTimer m_preparationElapsed;
  std::chrono::milliseconds m_minimumPreparationDuration;
  std::optional<app::PowerStagingPreparation> m_completedPreparation;
  QTimer m_stepTimer;
  bool m_directSelected = false;
  bool m_directConfirmationOpen = false;
  bool m_preparationPolling = false;
  bool m_stagingStarted = false;
  bool m_failureCanContinue = false;
  bool m_revalidating = false;
  bool m_finalPreparationChecked = false;
  app::AutomaticCommitSummary m_reviewedAutomaticCommit{};
  std::size_t m_reviewedFileCount = 0;
  bool m_reviewedHasStaging = false;
  app::AutomaticCommitProposal m_proposal{};
  app::AutomaticCommitProposal m_reviewedProposal{};
  int m_displayedProgress = 0;
};
}  // namespace uwf::presentation
