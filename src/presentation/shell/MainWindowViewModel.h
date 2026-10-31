// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <QObject>
#include <QPointer>
#include <QTimer>

#include "app/enhanced/EnhancedModeSession.h"
#include "app/staging/FileStagingDraft.h"
#include "app/state/ApplicationStateSession.h"
#include "presentation/common/AsyncTask.h"
#include "presentation/configuration/ApplyPlanViewModel.h"
#include "presentation/configuration/ConfigurationViewModel.h"

namespace uwf::presentation {
class MainWindowViewModel final : public QObject {
  Q_OBJECT
 public:
  struct VolumePage {
    core::VolumeInfo volume;
    bool hostsRegistry = false;
    bool hasStaging = false;
    bool system = false;
    QString title, tooltip, icon;
    std::shared_ptr<VolumeConfigurationViewModel> model;
  };
  MainWindowViewModel(app::ApplicationStateSource& source, app::UwfCapability capability, std::shared_ptr<app::FileStagingDraft> staging = {},
                       app::DriveResolver resolveDrive = {});
  [[nodiscard]] const app::ApplicationStateSession& state() const { return m_state; }
  [[nodiscard]] const std::shared_ptr<ConfigurationViewModel>& configuration() const { return m_configuration; }
  [[nodiscard]] std::size_t pendingChangeCount() const;
  [[nodiscard]] std::vector<VolumePage> volumePages(const QString& systemDrive) const;
  void initializeConfiguration(uint32_t totalRamMb, QString systemDrive = {});
  void refresh();
  [[nodiscard]] bool refreshing() const { return m_refresh.isRunning(); }
  [[nodiscard]] bool applyingConfiguration() const { return m_applyPlan && m_applyPlan->applying(); }
  [[nodiscard]] bool configurationWritesAllowed() const;
  [[nodiscard]] bool exitAllowed() const { return !applyingConfiguration() && !m_externalOperationActive; }
  [[nodiscard]] bool powerActionsAllowed() const;
  bool discardPendingChanges();
  void trackApplyPlan(std::shared_ptr<ApplyPlanViewModel> plan);
  void setExternalOperationActive(bool active);
  void requireReconciliation();
  void bindEnhancedMode(app::EnhancedModeSession& session);
  void unbindEnhancedMode();
  void refreshEnhancedModeStatus();
  [[nodiscard]] const app::EnhancedModeStatus& enhancedModeStatus() const { return m_enhancedModeStatus; }
 signals:
  void stateCommitted();
  void pendingChanged();
  void refreshFailed(const std::string& reason);
  void interactionAvailabilityChanged();
  void configurationApplyingChanged(bool applying);
  void enhancedModeStatusChanged(const uwf::app::EnhancedModeStatus& status);
  void enhancedModeAppearanceChanged();

 private:
  void resetVolumeEditors();
  void refreshStagingConstraints();
  void publishPendingChanged();
  void publishInteractionAvailability();
  QString m_systemDrive;
  app::DriveResolver m_resolveDrive;
  void applyEnhancedModeStatus(const app::EnhancedModeStatus& status);
  void recordEnhancedModeStatusFailure(const QString& detail);
  app::ApplicationStateSource& m_stateSource;
  app::UwfCapability m_uwfCapability;
  app::ApplicationStateSession m_state;
  AsyncTask m_refresh;
  std::shared_ptr<ApplyPlanViewModel> m_applyPlan;
  bool m_externalOperationActive = false;
  std::shared_ptr<ConfigurationViewModel> m_configuration;
  std::shared_ptr<app::FileStagingDraft> m_staging;
  bool m_resettingDrafts = false;
  QPointer<app::EnhancedModeSession> m_enhancedMode;
  app::EnhancedModeStatus m_enhancedModeStatus;
  QTimer m_enhancedModeStatusRetry;
  static constexpr int kEnhancedModeStatusRetryInitialMs = 1000;
  static constexpr int kEnhancedModeStatusRetryMaximumMs = 30'000;
  int m_enhancedModeStatusRetryDelayMs = kEnhancedModeStatusRetryInitialMs;
};
}  // namespace uwf::presentation
