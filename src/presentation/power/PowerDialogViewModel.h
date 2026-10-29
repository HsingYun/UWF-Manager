// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <QElapsedTimer>
#include <QObject>
#include <QTimer>

#include "PowerDialogState.h"
namespace uwf::presentation {

class PowerDialogViewModel final : public QObject {
  Q_OBJECT
 public:
  explicit PowerDialogViewModel(app::PowerActionDialogRequest request);
  [[nodiscard]] bool preparationRequired() const { return m_preparationRequired; }
  [[nodiscard]] std::size_t filesToCommit() const { return m_filesToCommit; }
  [[nodiscard]] bool hasStaging() const { return static_cast<bool>(m_advanceStaging); }
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
  std::function<app::PowerStagingProgress()> m_advanceStaging;
  QTimer m_preparationTimer;
  QElapsedTimer m_preparationElapsed;
  std::optional<app::PowerStagingPreparation> m_completedPreparation;
  QTimer m_stepTimer;
  bool m_directSelected = false;
  bool m_directConfirmationOpen = false;
  bool m_stagingStarted = false;
  bool m_failureCanContinue = false;
  int m_displayedProgress = 0;
};
}  // namespace uwf::presentation
