// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QObject>
#include <QPointer>
#include <QTimer>
#include <optional>
#include <string>

#include "application/diagnostics/contracts/DiagnosticReportTypes.h"
#include "application/enhanced/EnhancedModeSession.h"
#include "application/power/PowerCoordinator.h"
#include "application/staging/contracts/AutomaticCommitDisposition.h"
#include "application/staging/editing/FileStagingDraft.h"
#include "application/state/ApplicationStateSession.h"
#include "application/state/contracts/ApplicationStateSource.h"
#include "application/uwf/contracts/RuntimeWriteRestriction.h"
#include "presentation/viewmodels/common/AsyncTask.h"
#include "presentation/viewmodels/configuration/ApplyPlanViewModel.h"
#include "presentation/viewmodels/configuration/ConfigurationViewModel.h"

namespace uwf::presentation {
class ViewModelFactory;
class FileStagingViewModel;
class EnhancedModeViewModel;
class ImportViewModel;
class OverlayMaintenanceViewModel;
class OverlayViewModel;
class MainWindowViewModel final : public QObject {
  Q_OBJECT
 public:
  struct VolumePage {
    core::VolumeInfo volume;
    bool hostsRegistry = false;
    bool hasStaging = false;
    bool system = false;
    QString title, tooltip, icon;
    QString identity;
    std::shared_ptr<VolumeConfigurationViewModel> model;
  };
  struct InteractionState {
    bool refreshEnabled = false;
    bool configurationEnabled = false;
    bool reviewEnabled = false;
    bool powerEnabled = false;
    bool exitEnabled = true;
    bool enhancedModeEnabled = false;
    bool volumePagesEnabled = false;
    bool accessKnown = false;
    bool elevated = false;
    QString configurationHint, importHint, reviewHint, powerHint;
  };
  MainWindowViewModel(app::ApplicationStateSource& source, app::UwfCapability capability, std::shared_ptr<app::FileStagingDraft> staging = {},
                      app::DriveResolver resolveDrive = {}, std::shared_ptr<const app::FileBrowser> files = {});
  ~MainWindowViewModel() override;
  // Composition transfers workflow ownership once; the window model owns their coordination.
  void initializeWorkflows(std::shared_ptr<ViewModelFactory> models, std::unique_ptr<app::EnhancedModeSession> enhancedMode,
                           std::unique_ptr<app::PowerCoordinator> power);
  [[nodiscard]] InteractionState interactionState() const;
  [[nodiscard]] std::shared_ptr<FileStagingViewModel> fileStagingModel(const std::shared_ptr<VolumeConfigurationViewModel>& volume) const;
  [[nodiscard]] std::shared_ptr<ApplyPlanViewModel> reviewConfiguration();
  void requestReview();
  [[nodiscard]] std::shared_ptr<ImportViewModel> importModel();
  [[nodiscard]] std::shared_ptr<EnhancedModeViewModel> enhancedModeModel() const;
  [[nodiscard]] const std::shared_ptr<OverlayViewModel>& overlay() const { return m_overlay; }
  [[nodiscard]] const std::shared_ptr<OverlayMaintenanceViewModel>& overlayMaintenance() const { return m_overlayMaintenance; }
  // Editing the staging list still requires its registry data to be durable.
  // Automatic execution separately decides whether each file should be skipped.
  [[nodiscard]] app::RuntimeWriteRestriction stagingEditingRestriction() const;
  [[nodiscard]] QString runtimeModesText(bool compatibilityMode) const;
  [[nodiscard]] bool hasEnhancedMode() const { return !m_enhancedMode.isNull(); }
  [[nodiscard]] app::DiagnosticReportContext diagnosticContext(bool compatibilityMode) const;
  void safeShutdown();
  void safeRestart();
  void directRestart();
  [[nodiscard]] const app::ApplicationStateSession& state() const { return m_state; }
  [[nodiscard]] const std::shared_ptr<ConfigurationViewModel>& configuration() const { return m_configuration; }
  [[nodiscard]] std::size_t pendingChangeCount() const;
  [[nodiscard]] std::vector<VolumePage> volumePages(const QString& systemDrive) const;
  void initializeConfiguration(uint32_t totalRamMb, QString systemDrive = {});
  void refresh();
  void refreshDiscardingChanges();
  void retranslate();
  [[nodiscard]] bool refreshing() const { return m_refresh.isRunning(); }
  // All workspace views render the latest completed read from this owner.
  [[nodiscard]] std::optional<QString> workspaceDiagnostic() const;
  [[nodiscard]] bool applyingConfiguration() const { return m_applyPlan && m_applyPlan->applying(); }
  [[nodiscard]] bool configurationWritesAllowed() const;
  [[nodiscard]] bool configurationReviewAllowed() const;
  [[nodiscard]] QString configurationWriteRestriction() const;
  // Window-wide interaction admission only. The selected volume and the live
  // execution boundary decide protection, HORM and ROM requirements per target.
  [[nodiscard]] bool maintenanceActionsAllowed() const;
  [[nodiscard]] QString maintenanceActionRestriction() const;
  [[nodiscard]] bool exitAllowed() const;
  [[nodiscard]] bool powerActionsAllowed() const;
  [[nodiscard]] bool directRestartAllowed() const;
  bool discardPendingChanges();
  void trackApplyPlan(std::shared_ptr<ApplyPlanViewModel> plan);
  void setExternalOperationActive(bool active);
  void setManualCommitActive(bool active);
  void requireReconciliation();
  void bindEnhancedMode(app::EnhancedModeSession& session);
  void unbindEnhancedMode();
  void refreshEnhancedModeStatus();
  [[nodiscard]] const app::EnhancedModeStatus& enhancedModeStatus() const { return m_enhancedModeStatus; }
 signals:
  void stateCommitted();
  void pendingChanged();
  void refreshFailed(const std::string& reason);
  void reviewPrepared(std::shared_ptr<uwf::presentation::ApplyPlanViewModel> plan);
  void reviewFailed(const QString& reason);
  void interactionAvailabilityChanged();
  void configurationApplyingChanged(bool applying);
  void enhancedModeStatusChanged(const uwf::app::EnhancedModeStatus& status);
  void enhancedModeAppearanceChanged();

 private:
  [[nodiscard]] bool operationActive() const {
    return applyingConfiguration() || m_externalOperationActive || m_manualCommitActive || (m_staging && m_staging->busy());
  }
  [[nodiscard]] app::RuntimeWriteRestriction runtimeRestriction(bool includeApplicationHorm) const;
  [[nodiscard]] app::AutomaticCommitDisposition automaticCommitDisposition(const core::VolumeRecord* current, const core::VolumeRecord* next) const;
  [[nodiscard]] QString writeRestriction(bool configuration) const;
  [[nodiscard]] QString directRestartRestriction() const;
  void refreshPublishedState();
  enum class ReadPurpose { Observe, Discard, Review };
  void refreshImpl(bool refreshConfiguration, ReadPurpose purpose = ReadPurpose::Observe);
  void publishReview();
  void resetConfiguration(bool discardChanges = false);
  void refreshStagingConstraints();
  void updateAutomaticCommitSummary(EnhancedModeViewModel& model) const;
  void publishPendingChanged();
  void publishInteractionAvailability();
  QString m_systemDrive;
  app::DriveResolver m_resolveDrive;
  std::shared_ptr<const app::FileBrowser> m_files;
  void applyEnhancedModeStatus(const app::EnhancedModeStatus& status);
  void recordEnhancedModeStatusFailure(const QString& detail);
  app::ApplicationStateSource& m_stateSource;
  app::UwfCapability m_uwfCapability;
  app::ApplicationStateSession m_state;
  AsyncTask m_refresh;
  bool m_refreshingStaging = false;
  std::optional<std::string> m_refreshFailure;
  std::shared_ptr<ApplyPlanViewModel> m_applyPlan;
  bool m_externalOperationActive = false;
  bool m_manualCommitActive = false;
  std::shared_ptr<app::OverlayMaintenanceEditor> m_maintenanceEditor = std::make_shared<app::OverlayMaintenanceEditor>();
  std::shared_ptr<ConfigurationViewModel> m_configuration;
  std::shared_ptr<app::FileStagingDraft> m_staging;
  std::shared_ptr<ViewModelFactory> m_models;
  std::shared_ptr<OverlayViewModel> m_overlay;
  std::shared_ptr<OverlayMaintenanceViewModel> m_overlayMaintenance;
  // Power releases its borrowed enhanced-mode controls before the session.
  std::unique_ptr<app::EnhancedModeSession> m_ownedEnhancedMode;
  std::unique_ptr<app::PowerCoordinator> m_power;
  bool m_resettingDrafts = false;
  QPointer<app::EnhancedModeSession> m_enhancedMode;
  app::EnhancedModeStatus m_enhancedModeStatus;
  QTimer m_enhancedModeStatusRetry;
  static constexpr int kEnhancedModeStatusRetryInitialMs = 1000;
  static constexpr int kEnhancedModeStatusRetryMaximumMs = 30'000;
  int m_enhancedModeStatusRetryDelayMs = kEnhancedModeStatusRetryInitialMs;
};
}  // namespace uwf::presentation
