// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "MainWindowViewModel.h"

#include <QScopedValueRollback>
#include <algorithm>
#include <chrono>
#include <exception>
#include <utility>

#include "app/api/ConfigurationPlan.h"
#include "presentation/common/PresentationText.h"
#include "shared/Log.h"
namespace uwf::presentation {
MainWindowViewModel::MainWindowViewModel(app::ApplicationStateSource& source, app::UwfCapability capability, std::shared_ptr<app::FileStagingDraft> staging,
                                         app::DriveResolver resolveDrive)
    : m_resolveDrive(std::move(resolveDrive)), m_stateSource(source), m_uwfCapability(capability), m_staging(std::move(staging)) {
  if (m_staging) connect(m_staging.get(), &app::FileStagingDraft::changed, this, &MainWindowViewModel::refreshStagingConstraints);
  m_enhancedModeStatusRetry.setSingleShot(true);
  connect(&m_enhancedModeStatusRetry, &QTimer::timeout, this, &MainWindowViewModel::refreshEnhancedModeStatus);
}
std::size_t MainWindowViewModel::pendingChangeCount() const {
  if (!m_configuration || !m_state.hasCommittedState()) return 0;
  return app::planConfiguration(m_configuration->pendingChanges(), m_state.snapshot(), m_staging ? m_staging->changes() : std::nullopt).count();
}
void MainWindowViewModel::publishPendingChanged() {
  if (!m_resettingDrafts) emit pendingChanged();
}
void MainWindowViewModel::refreshStagingConstraints() {
  {
    const QScopedValueRollback resetting(m_resettingDrafts, true);
    if (m_configuration) m_configuration->refreshStagingConstraints();
  }
  publishPendingChanged();
}
std::vector<MainWindowViewModel::VolumePage> MainWindowViewModel::volumePages(const QString& systemDrive) const {
  const auto& volumes = m_state.volumes();
  std::optional<std::size_t> registryHost;
  for (std::size_t i = 0; i < volumes.size(); ++i)
    if (!systemDrive.isEmpty() && QString::fromStdString(volumes[i].driveLetter).compare(systemDrive, Qt::CaseInsensitive) == 0) {
      registryHost = i;
      break;
    }
  if (!registryHost)
    for (std::size_t i = 0; i < volumes.size(); ++i)
      if (core::supportsVolumeProtection(volumes[i].support)) {
        registryHost = i;
        break;
      }
  if (!registryHost && !volumes.empty()) registryHost = 0;
  std::vector<VolumePage> result;
  result.reserve(volumes.size());
  for (std::size_t i = 0; i < volumes.size(); ++i) {
    const auto& volume = volumes[i];
    VolumePage page;
    page.volume = volume;
    page.hostsRegistry = registryHost && i == *registryHost;
    page.hasStaging = m_uwfCapability == app::UwfCapability::Available && (core::supportsFileOverlayOperations(volume.support) || page.hostsRegistry);
    const auto letter = QString::fromStdString(volume.driveLetter);
    if (m_configuration) page.model = m_configuration->volume(letter);
    page.system = !systemDrive.isEmpty() && letter.compare(systemDrive, Qt::CaseInsensitive) == 0;
    const bool protectable = core::supportsVolumeProtection(volume.support);
    page.icon = !protectable  ? QStringLiteral(":/icons/disk_off.svg")
                : page.system ? QStringLiteral(":/icons/disk_system.svg")
                              : QStringLiteral(":/icons/disk.svg");
    page.title = volume.label.empty() ? letter : QStringLiteral("%1 (%2)").arg(QString::fromStdString(volume.label), letter);
    const auto extra = page.hostsRegistry ? PresentationText::tr(" (Also manages global registry exclusions.)") : QString{};
    page.tooltip = protectable && volume.support != core::VolumeSupport::FileSystemLimited
                       ? PresentationText::tr("Switch to protection settings, file exclusions, and file staging for volume %1.%2").arg(letter, extra)
                       : volumeSupportText(volume.support, volume.fileSystem) + extra;
    result.push_back(std::move(page));
  }
  return result;
}
void MainWindowViewModel::initializeConfiguration(const uint32_t totalRamMb, QString systemDrive) {
  if (m_configuration) return;
  m_systemDrive = std::move(systemDrive);
  m_configuration = std::make_shared<ConfigurationViewModel>(
      totalRamMb, [staging = m_staging] { return staging && !staging->load().isEmpty(); }, m_systemDrive, m_resolveDrive);
  connect(m_configuration.get(), &ConfigurationViewModel::pendingChanged, this, &MainWindowViewModel::publishPendingChanged);
  resetVolumeEditors();
  publishPendingChanged();
}
void MainWindowViewModel::resetVolumeEditors() {
  if (!m_configuration || !m_state.hasCommittedState()) return;
  m_configuration->clearVolumes();
  for (const auto& page : volumePages(m_systemDrive)) m_configuration->addVolume(page.volume, page.hostsRegistry);
  m_configuration->resetSnapshot(m_state.snapshot());
  m_configuration->global()->setControlsEnabled(configurationWritesAllowed());
}
bool MainWindowViewModel::discardPendingChanges() {
  if (applyingConfiguration()) return false;
  {
    const QScopedValueRollback resetting(m_resettingDrafts, true);
    // Complete the fallible read before resetting any configuration editor.
    // If it fails, both drafts retain their edits and reconciliation remains required.
    if (m_staging && (m_staging->changes() || m_staging->reconciliationRequired())) m_staging->reload();
    if (m_configuration && m_state.hasCommittedState()) {
      m_configuration->resetSnapshot(m_state.snapshot());
      m_configuration->global()->setControlsEnabled(configurationWritesAllowed());
    }
  }
  publishPendingChanged();
  return true;
}
bool MainWindowViewModel::configurationWritesAllowed() const {
  return !applyingConfiguration() && !refreshing() && !m_externalOperationActive && m_state.configurationWritesAllowed();
}

bool MainWindowViewModel::powerActionsAllowed() const {
  if (!m_state.hasCommittedState() || applyingConfiguration()) return false;
  const auto& snapshot = m_state.snapshot();
  return snapshot.elevated && snapshot.uwfAvailable;
}
void MainWindowViewModel::publishInteractionAvailability() {
  const QPointer guard(this);
  if (m_applyPlan) m_applyPlan->setExecutionAllowed(!refreshing() && !m_externalOperationActive && m_state.configurationWritesAllowed());
  if (!guard) return;
  if (m_configuration) m_configuration->global()->setControlsEnabled(configurationWritesAllowed());
  if (guard) emit interactionAvailabilityChanged();
}
void MainWindowViewModel::setExternalOperationActive(const bool active) {
  if (m_externalOperationActive == active) return;
  m_externalOperationActive = active;
  publishInteractionAvailability();
}
void MainWindowViewModel::trackApplyPlan(std::shared_ptr<ApplyPlanViewModel> plan) {
  if (applyingConfiguration()) return;
  if (m_applyPlan) disconnect(m_applyPlan.get(), nullptr, this, nullptr);
  m_applyPlan = std::move(plan);
  connect(m_applyPlan.get(), &ApplyPlanViewModel::applyingChanged, this, [this] {
    const QPointer guard(this);
    emit configurationApplyingChanged(applyingConfiguration());
    if (guard) publishInteractionAvailability();
  });
  connect(m_applyPlan.get(), &ApplyPlanViewModel::reconciliationRequired, this, [this] {
    const QPointer guard(this);
    requireReconciliation();
    if (guard) QTimer::singleShot(0, this, &MainWindowViewModel::refresh);
  });
  publishInteractionAvailability();
}
void MainWindowViewModel::requireReconciliation() {
  m_state.requireReconciliation();
  publishInteractionAvailability();
}
void MainWindowViewModel::bindEnhancedMode(app::EnhancedModeSession& session) {
  unbindEnhancedMode();
  m_enhancedMode = &session;
  connect(m_enhancedMode, &app::EnhancedModeSession::statusChanged, this, &MainWindowViewModel::applyEnhancedModeStatus);
  connect(m_enhancedMode, &app::EnhancedModeSession::refreshRequested, this, &MainWindowViewModel::refreshEnhancedModeStatus);
  connect(m_enhancedMode, &app::EnhancedModeSession::agentConnectionStateChanged, this, [this](const app::EnhancedModeAgentState state) {
    m_enhancedModeStatus.agentState = state;
    emit enhancedModeAppearanceChanged();
  });
}
void MainWindowViewModel::unbindEnhancedMode() {
  m_enhancedModeStatusRetry.stop();
  m_enhancedModeStatusRetryDelayMs = kEnhancedModeStatusRetryInitialMs;
  if (m_enhancedMode) disconnect(m_enhancedMode, nullptr, this, nullptr);
  m_enhancedMode = nullptr;
}
void MainWindowViewModel::applyEnhancedModeStatus(const app::EnhancedModeStatus& status) {
  m_enhancedModeStatusRetry.stop();
  m_enhancedModeStatusRetryDelayMs = kEnhancedModeStatusRetryInitialMs;
  m_enhancedModeStatus = status;
  emit enhancedModeStatusChanged(status);
}
void MainWindowViewModel::refresh() {
  if (applyingConfiguration()) return;
  UWF_LOG_D("ui") << "state refresh started";
  const auto t0 = std::chrono::steady_clock::now();
  app::ApplicationStateSource::ReadOperation read;
  try {
    read = m_stateSource.readOperation(m_uwfCapability);
  } catch (...) {
    const auto failure = std::current_exception();
    read = [failure](std::stop_token) -> app::ApplicationState { std::rethrow_exception(failure); };
  }
  m_refresh.start(std::move(read), [this, t0](AsyncResult<app::ApplicationState> result) {
    const auto* staging = m_staging.get();
    std::optional<std::string> failure;
    if (const auto* error = std::get_if<std::exception_ptr>(&result)) {
      try {
        std::rethrow_exception(*error);
      } catch (const std::exception& error) {
        failure = error.what();
      } catch (...) {
        failure = "unknown application state read failure";
      }
      UWF_LOG_W("ui") << "state refresh failed: committedState=retained error=" << *failure;
    }
    {
      const QScopedValueRollback resetting(m_resettingDrafts, true);
      if (staging && (staging->reconciliationRequired() || !staging->changes())) {
        try {
          if (staging->reconciliationRequired())
            m_staging->reconcile();
          else
            m_staging->reload();
        } catch (const std::exception& error) {
          UWF_LOG_W("staging") << "file staging draft reload failed: " << error.what();
        }
      }
      if (!failure) {
        m_state.commit(std::get<app::ApplicationState>(std::move(result)));
        if (staging && staging->reconciliationRequired()) m_state.requireReconciliation();
        resetVolumeEditors();
      }
    }
    const QPointer guard(this);
    publishInteractionAvailability();
    if (!guard) return;
    publishPendingChanged();
    if (!guard) return;
    if (failure) {
      if (m_configuration && !m_state.hasCommittedState()) m_configuration->global()->setAvailable(false);
      if (guard) emit refreshFailed(*failure);
      return;
    }
    const auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t0).count();
    UWF_LOG_I("ui") << "state refresh completed: disks=" << m_state.volumes().size() << " uwfAvailable=" << m_state.snapshot().uwfAvailable
                    << " currentVolumes=" << m_state.snapshot().current.volumes.size() << " nextVolumes=" << m_state.snapshot().next.volumes.size()
                    << " elapsedMs=" << elapsedMs;
    emit stateCommitted();
  });
  publishInteractionAvailability();
}
void MainWindowViewModel::refreshEnhancedModeStatus() {
  if (!m_enhancedMode) return;
  try {
    m_enhancedMode->refresh();
  } catch (const std::exception& error) {
    recordEnhancedModeStatusFailure(QString::fromUtf8(error.what()));
  } catch (...) {
    recordEnhancedModeStatusFailure(PresentationText::tr("The enhanced mode status read failed because of an unknown error."));
  }
}
void MainWindowViewModel::recordEnhancedModeStatusFailure(const QString& detail) {
  // SCM 查询失败时保留已确认状态和代理所有权，降级展示并重试。
  m_enhancedModeStatus.state = app::EnhancedModeState::RepairRequired;
  m_enhancedModeStatus.detail = detail;
  if (!m_enhancedModeStatusRetry.isActive()) {
    m_enhancedModeStatusRetry.start(m_enhancedModeStatusRetryDelayMs);
    m_enhancedModeStatusRetryDelayMs = std::min(m_enhancedModeStatusRetryDelayMs * 2, kEnhancedModeStatusRetryMaximumMs);
  }
  emit enhancedModeAppearanceChanged();
}
}  // namespace uwf::presentation

#include "moc_MainWindowViewModel.cpp"
