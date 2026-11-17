// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "presentation/viewmodels/shell/MainWindowViewModel.h"

#include <QScopedValueRollback>
#include <QStringList>
#include <algorithm>
#include <chrono>
#include <exception>
#include <stdexcept>
#include <tuple>
#include <utility>

#include "application/configuration/contracts/ConfigurationPlan.h"
#include "application/diagnostics/DiagnosticReportService.h"
#include "application/staging/policy/AutomaticCommitPolicy.h"
#include "application/state/contracts/ApplicationStateSource.h"
#include "application/uwf/contracts/UwfMaintenanceRepository.h"
#include "domain/uwf/policies/ConfigurationModePolicy.h"
#include "domain/uwf/policies/IndividualCommitPolicy.h"
#include "domain/uwf/policies/VolumeIdentityPolicy.h"
#include "foundation/logging/Log.h"
#include "presentation/viewmodels/common/PresentationText.h"
#include "presentation/viewmodels/common/RuntimeWriteText.h"
#include "presentation/viewmodels/common/ViewModelFactory.h"

namespace uwf::presentation {
MainWindowViewModel::MainWindowViewModel(app::ApplicationStateSource& source, app::UwfCapability capability, std::shared_ptr<app::FileStagingDraft> staging,
                                         app::DriveResolver resolveDrive, std::shared_ptr<const app::FileBrowser> files)
    : m_resolveDrive(std::move(resolveDrive)), m_files(std::move(files)), m_stateSource(source), m_uwfCapability(capability), m_staging(std::move(staging)) {
  if (m_staging) connect(m_staging.get(), &app::FileStagingDraft::changed, this, &MainWindowViewModel::refreshStagingConstraints);
  if (m_staging) connect(m_staging.get(), &app::FileStagingDraft::busyChanged, this, &MainWindowViewModel::publishInteractionAvailability);
  m_enhancedModeStatusRetry.setSingleShot(true);
  connect(&m_enhancedModeStatusRetry, &QTimer::timeout, this, &MainWindowViewModel::refreshEnhancedModeStatus);
}
MainWindowViewModel::~MainWindowViewModel() {
  m_refresh.cancel();
  if (m_refreshingStaging && m_staging) m_staging->setBusy(false);
  if (m_power) disconnect(m_power.get(), nullptr, this, nullptr);
  m_power.reset();
  unbindEnhancedMode();
  m_ownedEnhancedMode.reset();
}
void MainWindowViewModel::initializeWorkflows(std::shared_ptr<ViewModelFactory> models, std::unique_ptr<app::EnhancedModeSession> enhancedMode,
                                              std::unique_ptr<app::PowerCoordinator> power) {
  if (m_models || !models || !power) throw std::invalid_argument("window workflows require one complete composition");
  const QPointer self(this);
  m_models = std::move(models);
  m_overlay = m_models->overlay();
  if (m_overlay) {
    connect(m_overlay.get(), &OverlayViewModel::usageChanged, this, [this](const app::OverlayUsageState& state) {
      if (m_configuration) m_configuration->global()->applyUsageState(state);
    });
    connect(this, &MainWindowViewModel::stateCommitted, m_overlay.get(),
            [this] { m_overlay->applySnapshot(m_state.snapshot(), m_state.overlayUsageSampled()); });
    if (m_state.hasCommittedState()) m_overlay->applySnapshot(m_state.snapshot(), m_state.overlayUsageSampled());
    if (!self) return;
  }
  m_overlayMaintenance = m_models->overlayMaintenance(m_maintenanceEditor);
  if (m_overlayMaintenance) {
    // Read progress belongs to the maintenance dialog. Only observed admission
    // changes affect the window; starting the same read must not redraw it.
    const auto admission = [this] { return std::tuple{m_overlayMaintenance->hasSnapshot(), runtimeRestriction(false), m_overlayMaintenance->hormActive()}; };
    connect(m_overlayMaintenance.get(), &OverlayMaintenanceViewModel::stateChanged, this, [this, admission, previous = admission()]() mutable {
      const auto current = admission();
      if (current == previous) return;
      previous = current;
      publishInteractionAvailability();
    });
    connect(this, &MainWindowViewModel::stateCommitted, m_overlayMaintenance.get(), &OverlayMaintenanceViewModel::adoptPublishedState);
    connect(m_overlayMaintenance.get(), &OverlayMaintenanceViewModel::pendingChanged, this, &MainWindowViewModel::publishInteractionAvailability);
  }
  m_ownedEnhancedMode = std::move(enhancedMode);
  m_power = std::move(power);
  connect(m_power.get(), &app::PowerCoordinator::activeChanged, this, &MainWindowViewModel::setExternalOperationActive);
  if (m_ownedEnhancedMode) bindEnhancedMode(*m_ownedEnhancedMode);
  m_power->setSuspended(operationActive());
  if (!self) return;
  setExternalOperationActive(m_power->active());
  if (self) refreshEnhancedModeStatus();
}
MainWindowViewModel::InteractionState MainWindowViewModel::interactionState() const {
  InteractionState view;
  view.refreshEnabled = !refreshing() && !operationActive();
  view.configurationEnabled = configurationWritesAllowed();
  view.reviewEnabled = configurationReviewAllowed();
  view.powerEnabled = powerActionsAllowed();
  view.powerHint = directRestartRestriction();
  view.exitEnabled = exitAllowed();
  view.accessKnown = m_state.hasCommittedState();
  view.elevated = view.accessKnown && m_state.snapshot().elevated;
  view.enhancedModeEnabled = hasEnhancedMode() && !m_refreshFailure && !operationActive() && (!view.accessKnown || view.elevated);
  view.volumePagesEnabled = !m_state.reconciliationRequired() && !refreshing() && !operationActive();
  view.configurationHint = configurationWriteRestriction();
  view.importHint = view.configurationHint.isEmpty()
                        ? PresentationText::tr("Import uwfmgr commands into the draft. Changes are written only after confirmation in Review and apply.")
                        : view.configurationHint;
  view.reviewHint = view.reviewEnabled ? PresentationText::tr("Review all pending changes and apply them in one batch.") : view.configurationHint;
  return view;
}
std::shared_ptr<FileStagingViewModel> MainWindowViewModel::fileStagingModel(const std::shared_ptr<VolumeConfigurationViewModel>& volume) const {
  if (!m_models || !m_staging || !volume || volume->driveLetter().isEmpty()) return {};
  app::FileStagingScope scope;
  for (const auto& page : volumePages(m_systemDrive)) {
    if (!page.hasStaging) continue;
    const auto drive = QString::fromStdString(page.volume.driveLetter);
    scope.managedDrives.append(drive);
    if (scope.fallbackDrive.isEmpty() || page.hostsRegistry) scope.fallbackDrive = drive;
  }
  auto model = m_models->fileStaging(volume->driveLetter(), *m_staging, &volume->fileConflicts(), scope);
  if (!model) return {};
  const auto disable = [target = model.get()] {
    auto availability = target->availability();
    availability.readOnly = true;
    target->setAvailability(availability);
  };
  // The staging editor borrows the draft and this volume's conflict policy.
  // Keep both alive for as long as this model can receive draft notifications.
  const auto update = [window = QPointer(this), volume, staging = m_staging, target = model.get(), disable] {
    static_cast<void>(staging);
    if (!window) {
      disable();
      return;
    }
    const auto& snapshot = window->state().snapshot();
    target->setAvailability({.readOnly = !window->configurationWritesAllowed(),
                             .enhancedModeAvailable = window->enhancedModeStatus().serviceContractSatisfied(),
                             .editingRestriction = window->stagingEditingRestriction(),
                             .automaticCommit =
                                 [&] {
                                   app::AutomaticCommitSummary summary;
                                   summary.include(window->automaticCommitDisposition(core::findVolumeConfiguration(snapshot.current, volume->volume()),
                                                                                      core::findVolumeConfiguration(snapshot.next, volume->volume())));
                                   return summary;
                                 }(),
                             .automaticCommitVolumes =
                                 [&] {
                                   FileStagingViewModel::AutomaticCommitVolumes volumes;
                                   volumes.unregistered = window->automaticCommitDisposition(nullptr, nullptr);
                                   for (const auto* session : {&snapshot.current, &snapshot.next})
                                     for (const auto& record : session->volumes) {
                                       const auto id = core::volumeIdentity(record.volumeName);
                                       if (id.empty() || volumes.dispositions.contains(id)) continue;
                                       volumes.dispositions.emplace(id, window->automaticCommitDisposition(core::findVolumeConfiguration(snapshot.current, id),
                                                                                                           core::findVolumeConfiguration(snapshot.next, id)));
                                     }
                                   return volumes;
                                 }(),
                             .additionsAllowed = volume->supportsFileOperations()});
  };
  connect(this, &MainWindowViewModel::interactionAvailabilityChanged, model.get(), update);
  connect(this, &MainWindowViewModel::enhancedModeStatusChanged, model.get(), update);
  connect(this, &MainWindowViewModel::enhancedModeAppearanceChanged, model.get(), update);
  connect(volume.get(), &VolumeConfigurationViewModel::stateChanged, model.get(), update);
  connect(this, &QObject::destroyed, model.get(), disable);
  connect(volume.get(), &QObject::destroyed, model.get(), disable);
  update();
  return model;
}
std::shared_ptr<ApplyPlanViewModel> MainWindowViewModel::reviewConfiguration() {
  if (!m_models || !configurationReviewAllowed() || !m_configuration) return {};
  auto changes = m_configuration->pendingConfiguration();
  auto plan = m_models->applyPlan(std::move(changes), m_state.snapshot(), m_state.overlayPolicy(), m_staging,
                                  m_overlayMaintenance && m_overlayMaintenance->hasSnapshot() ? std::optional{m_overlayMaintenance->snapshot()} : std::nullopt);
  trackApplyPlan(plan);
  return plan;
}
void MainWindowViewModel::requestReview() {
  if (!m_models || !configurationReviewAllowed() || !m_configuration) return;
  bool ready = true;
  if (m_staging) {
    try {
      ready = m_staging->cachedSnapshot().has_value();
    } catch (...) {
      ready = false;
    }
  }
  if (!ready)
    refreshImpl(false, ReadPurpose::Review);
  else
    publishReview();
}
void MainWindowViewModel::publishReview() {
  try {
    if (auto plan = reviewConfiguration()) emit reviewPrepared(std::move(plan));
  } catch (const std::exception& error) {
    emit reviewFailed(QString::fromUtf8(error.what()));
  } catch (...) {
    emit reviewFailed(PresentationText::tr("Unavailable"));
  }
}
std::shared_ptr<EnhancedModeViewModel> MainWindowViewModel::enhancedModeModel() const {
  if (!m_models || !m_enhancedMode || operationActive()) return {};
  auto model = m_models->enhancedMode(m_enhancedMode->actions());
  model->setWriteRestriction(stagingEditingRestriction());
  updateAutomaticCommitSummary(*model);
  const auto update = [self = QPointer(this), target = QPointer(model.get())] {
    if (!self || !target) return;
    target->setWriteRestriction(self->stagingEditingRestriction());
    if (self && target) self->updateAutomaticCommitSummary(*target);
  };
  connect(this, &MainWindowViewModel::interactionAvailabilityChanged, model.get(), update);
  connect(this, &MainWindowViewModel::pendingChanged, model.get(), update);
  return model;
}
void MainWindowViewModel::updateAutomaticCommitSummary(EnhancedModeViewModel& model) const {
  if (!m_state.hasCommittedState() || m_refreshFailure || m_state.reconciliationRequired() || !m_state.snapshot().uwfAvailable ||
      (m_staging && !m_staging->storageAvailable())) {
    model.setAutomaticCommitSummary({.unavailable = true});
    return;
  }
  const auto& snapshot = m_state.snapshot();
  app::AutomaticCommitSummary summary;
  const auto inspectVolume = [&](const core::VolumeRecord* current, const core::VolumeRecord* next) {
    summary.include(automaticCommitDisposition(current, next));
  };
  // The saved baseline drives shutdown, independently of unapplied list edits.
  // Its bindings were resolved when the list was observed; this projection does no I/O.
  const auto entries = m_staging ? m_staging->baseline() : QList<app::FileStagingEntry>{};
  if (!entries.isEmpty()) {
    for (const auto& entry : entries) {
      if (!entry.fileTarget) {
        summary.include(app::AutomaticCommitDisposition::Unavailable);
        continue;
      }
      inspectVolume(core::findVolumeConfiguration(snapshot.current, entry.fileTarget->volumeId),
                    core::findVolumeConfiguration(snapshot.next, entry.fileTarget->volumeId));
    }
  } else {
    // With no saved targets, describe the current protected volumes to keep the
    // feature's status meaningful before the user adds a staging entry.
    for (const auto& current : snapshot.current.volumes) inspectVolume(&current, core::findVolumeConfiguration(snapshot.next, current.volumeName));
  }
  model.setAutomaticCommitSummary(summary);
}
app::AutomaticCommitDisposition MainWindowViewModel::automaticCommitDisposition(const core::VolumeRecord* current, const core::VolumeRecord* next) const {
  if (!m_state.hasCommittedState() || m_refreshFailure || m_state.reconciliationRequired() || !m_state.snapshot().uwfAvailable)
    return app::AutomaticCommitDisposition::Unavailable;
  const auto& runtime = m_state.snapshot().runtime;
  return app::automaticFileCommitDisposition({m_state.overlayPolicy(), m_state.hormEnabled(), runtime.romDisposition, runtime.fullCommit},
                                             current && current->isProtected, next && next->isProtected);
}
std::shared_ptr<ImportViewModel> MainWindowViewModel::importModel() {
  if (!m_models || !m_configuration || !configurationWritesAllowed()) return {};
  const auto models = m_models;
  const QPointer self(this);
  auto model = models->importCommands();
  if (!self || !model) return {};
  model->setApplier([self](const QList<app::UwfmgrCommand>& commands) -> QList<app::ImportResult> {
    if (!self || !self->m_configuration) return {};
    const auto configuration = self->m_configuration;
    return configuration->importCommands(commands);
  });
  if (const auto maintenance = m_overlayMaintenance) {
    auto* importer = model.get();
    connect(maintenance.get(), &OverlayMaintenanceViewModel::busyChanged, importer,
            [source = maintenance.get(), importer] { importer->setPreparing(source->busy()); });
    importer->setPreparing(maintenance->busy());
    if (!self) return {};
    maintenance->refresh();
  }
  if (!self) return {};
  return model;
}
app::DiagnosticReportContext MainWindowViewModel::diagnosticContext(const bool compatibilityMode) const {
  return app::DiagnosticReportService::capture(m_uwfCapability, m_state.hasCommittedState() ? &m_state.snapshot() : nullptr,
                                               hasEnhancedMode() ? &m_enhancedModeStatus : nullptr, compatibilityMode);
}
void MainWindowViewModel::safeShutdown() {
  if (m_power && powerActionsAllowed()) m_power->safeShutdown();
}
void MainWindowViewModel::safeRestart() {
  if (m_power && powerActionsAllowed()) m_power->safeRestart();
}
void MainWindowViewModel::directRestart() {
  if (m_power && directRestartAllowed()) m_power->directRestart();
}
std::size_t MainWindowViewModel::pendingChangeCount() const {
  if (!m_configuration || !m_state.hasCommittedState()) return 0;
  auto changes = m_configuration->pendingConfiguration();
  return app::planConfiguration(changes.settings, m_state.snapshot(), m_staging ? m_staging->editSnapshot() : std::nullopt).count() +
         app::ConfigurationChanges{{}, changes.overlayPolicy, changes.maintenance}.count();
}
void MainWindowViewModel::publishPendingChanged() {
  if (!m_resettingDrafts) {
    if (m_overlayMaintenance)
      m_overlayMaintenance->setInteractionContext(m_state.hasCommittedState() && m_state.snapshot().elevated,
                                                  refreshing() || operationActive() || m_state.reconciliationRequired());
    emit pendingChanged();
  }
}
void MainWindowViewModel::refreshStagingConstraints() {
  {
    const QScopedValueRollback resetting(m_resettingDrafts, true);
    if (m_configuration) {
      if (m_staging) {
        try {
          const auto cached = m_staging->cachedSnapshot();
          if (cached) m_configuration->setStagedEntries(cached->entries);
        } catch (const std::exception& error) {
          UWF_LOG_W("staging") << "file staging constraints unavailable: " << error.what();
        }
      }
      m_configuration->refreshStagingConstraints();
    }
  }
  publishPendingChanged();
}
std::vector<MainWindowViewModel::VolumePage> MainWindowViewModel::volumePages(const QString& systemDrive) const {
  auto volumes = m_state.volumes();
  const auto identity = [](const auto& volume) { return app::VolumeConfigurationEditor::volumeIdentity(volume); };
  const auto& snapshot = m_state.snapshot();
  const auto hiddenProtected = [&](const core::VolumeInfo& volume) {
    const auto matches = [&](const auto& session) {
      return std::ranges::any_of(session.volumes, [&](const auto& record) {
        return record.isProtected && !record.volumeName.empty() &&
               identity(core::VolumeInfo{.driveLetter = {}, .volumeName = record.volumeName, .fileSystem = {}, .label = {}}) == identity(volume);
      });
    };
    return matches(snapshot.current) || matches(snapshot.next);
  };
  std::erase_if(volumes, [&](const auto& volume) { return volume.driveLetter.empty() && !hiddenProtected(volume); });
  // Logical disk inventory omits hidden partitions. UWF's Current and Next
  // memberships independently keep their protected identities discoverable.
  for (const auto* session : {&snapshot.current, &snapshot.next})
    for (const auto& record : session->volumes) {
      if (!core::volumeConfigurationAddress(record).empty() || !record.isProtected || record.volumeName.empty()) continue;
      core::VolumeInfo hidden{.driveLetter = {}, .volumeName = record.volumeName, .fileSystem = {}, .label = {}};
      if (std::ranges::none_of(volumes, [&](const auto& volume) { return identity(volume) == identity(hidden); })) volumes.push_back(std::move(hidden));
    }
  std::stable_sort(volumes.begin(), volumes.end(), [&](const auto& left, const auto& right) {
    if (left.driveLetter.empty() != right.driveLetter.empty()) return !left.driveLetter.empty();
    return left.driveLetter.empty() && identity(left) < identity(right);
  });
  std::optional<std::size_t> registryHost;
  for (std::size_t i = 0; i < volumes.size(); ++i)
    if (!systemDrive.isEmpty() && QString::fromStdString(volumes[i].driveLetter).compare(systemDrive, Qt::CaseInsensitive) == 0) {
      registryHost = i;
      break;
    }
  std::vector<VolumePage> result;
  result.reserve(volumes.size());
  for (std::size_t i = 0; i < volumes.size(); ++i) {
    const auto& volume = volumes[i];
    VolumePage page;
    page.volume = volume;
    page.identity = identity(volume);
    page.hostsRegistry = registryHost && i == *registryHost;
    page.hasStaging = !volume.driveLetter.empty() && m_uwfCapability == app::UwfCapability::Available &&
                      (core::supportsFileOverlayOperations(volume.support) || page.hostsRegistry);
    const auto letter = QString::fromStdString(volume.driveLetter);
    if (m_configuration) page.model = m_configuration->volume(page.identity);
    page.system = !letter.isEmpty() && !systemDrive.isEmpty() && letter.compare(systemDrive, Qt::CaseInsensitive) == 0;
    const bool registered = core::findVolumeConfiguration(snapshot.next, volume) != nullptr;
    const auto registrationNotice = registered ? QString{} : volumeRegistrationText(volume);
    const bool protectable = core::supportsVolumeProtection(volume.support) && registrationNotice.isEmpty();
    page.icon = !protectable  ? QStringLiteral(":/icons/disk_off.svg")
                : page.system ? QStringLiteral(":/icons/disk_system.svg")
                              : QStringLiteral(":/icons/disk.svg");
    page.title = letter;
    if (letter.isEmpty()) {
      page.title = PresentationText::tr("Partition");
      page.tooltip = page.title + u'\n' + QString::fromStdString(volume.volumeName);
      result.push_back(std::move(page));
      continue;
    }
    const auto extra = page.hostsRegistry ? PresentationText::tr(" (Also manages global registry exclusions.)") : QString{};
    page.tooltip = !registrationNotice.isEmpty() ? registrationNotice + extra
                   : protectable && volume.support != core::VolumeSupport::FileSystemLimited
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
      totalRamMb,
      [staging = m_staging] {
        if (!staging) return false;
        const auto cached = staging->cachedSnapshot();
        if (!cached)
          throw std::runtime_error(PresentationText::tr("The file staging list could not be read:\n%1").arg(PresentationText::tr("Unavailable")).toStdString());
        return !cached->entries.isEmpty();
      },
      m_systemDrive, m_resolveDrive, m_files, nullptr, m_maintenanceEditor);
  m_configuration->setEditingAllowed(configurationWritesAllowed());
  m_configuration->setCommitContext(maintenanceActionsAllowed(), maintenanceActionRestriction());
  connect(m_configuration.get(), &ConfigurationViewModel::pendingChanged, this, &MainWindowViewModel::publishPendingChanged);
  const QPointer self(this);
  resetConfiguration();
  if (!self) return;
  if (m_overlay && m_state.hasCommittedState()) m_configuration->global()->applyUsageState(m_overlay->usage());
  if (!self) return;
  publishInteractionAvailability();
  if (self) publishPendingChanged();
}
void MainWindowViewModel::resetConfiguration(const bool discardChanges) {
  if (!m_configuration || !m_state.hasCommittedState()) return;
  const auto configuration = m_configuration;
  const auto pages = volumePages(m_systemDrive);
  const auto snapshot = m_state.snapshot();
  const auto policy = m_state.overlayPolicy();
  const auto hormEnabled = m_state.hormEnabled();
  const QPointer guard(this);
  configuration->clearVolumes();
  if (!guard) return;
  for (const auto& page : pages) {
    configuration->addVolume(page.volume, page.hostsRegistry);
    if (!guard) return;
  }
  if (discardChanges)
    configuration->discardChanges(snapshot, policy, hormEnabled);
  else
    configuration->resetSnapshot(snapshot, policy, hormEnabled);
  if (guard) {
    refreshStagingConstraints();
    if (guard) configuration->setEditingAllowed(configurationWritesAllowed());
  }
}
bool MainWindowViewModel::discardPendingChanges() {
  if (operationActive()) return false;
  {
    const QScopedValueRollback resetting(m_resettingDrafts, true);
    // Discard restores the last observed baseline; only a refresh observes storage.
    // An unknown baseline must be reconciled before either draft is discarded.
    if (m_staging && (m_staging->changes() || m_staging->reconciliationRequired())) m_staging->discardChanges();
    if (m_configuration && m_state.hasCommittedState()) {
      m_configuration->discardChanges(m_state.snapshot(), m_state.overlayPolicy(), m_state.hormEnabled());
      m_configuration->setEditingAllowed(configurationWritesAllowed());
    }
  }
  publishPendingChanged();
  return true;
}
bool MainWindowViewModel::configurationWritesAllowed() const {
  return runtimeRestriction(false) == app::RuntimeWriteRestriction::None && !operationActive() && !refreshing() && m_state.configurationWritesAllowed();
}
bool MainWindowViewModel::configurationReviewAllowed() const {
  const bool ready = !operationActive() && !refreshing() && m_state.hasCommittedState() && m_state.snapshot().uwfAvailable && m_state.snapshot().elevated &&
                     !m_state.reconciliationRequired();
  if (!ready) return false;
  if (configurationWritesAllowed()) return true;
  if (!m_overlayMaintenance || !m_configuration) return false;
  const auto changes = m_configuration->pendingConfiguration();
  if (!changes.settings.empty() || !changes.overlayPolicy.empty()) return false;
  const auto& draft = changes.maintenance;
  return draft.hormEnabled == false && !draft.storageVolume && !draft.fullCommit && !draft.allVolumesProtected &&
         (m_overlayMaintenance->hormActive() || m_state.hormEnabled() == true);
}
app::RuntimeWriteRestriction MainWindowViewModel::runtimeRestriction(const bool includeApplicationHorm) const {
  if (m_overlayMaintenance) {
    if (m_overlayMaintenance->hormActive()) return app::RuntimeWriteRestriction::HormActive;
    if (m_overlayMaintenance->hasSnapshot() && !m_overlayMaintenance->snapshot().hormEnabled.has_value()) return app::RuntimeWriteRestriction::HormUnknown;
  }
  // Window-wide maintenance admission leaves application HORM to each target;
  // configuration and automatic saving also honor the application observation.
  if (includeApplicationHorm && !app::hormAllowsChanges(m_state.hormEnabled())) return app::RuntimeWriteRestriction::HormActive;
  return app::RuntimeWriteRestriction::None;
}
QString MainWindowViewModel::configurationWriteRestriction() const { return configurationWritesAllowed() ? QString{} : writeRestriction(true); }
QString MainWindowViewModel::runtimeModesText(const bool compatibilityMode) const {
  if (!m_state.hasCommittedState() || !m_state.snapshot().uwfAvailable) return {};
  QStringList modes;
  const auto& snapshot = m_state.snapshot();
  if (snapshot.current.filter.enabled) {
    const auto& observedFlags = m_state.overlayPolicy().current.flags;
    const auto flags =
        observedFlags.status == app::PolicyObservationStatus::Available && observedFlags.value ? std::optional{observedFlags.value->raw} : std::nullopt;
    if (m_state.hormEnabled() == true)
      modes.append(QStringLiteral("HORM"));
    else if (core::effectiveReadOnlyMedia({m_state.hormEnabled(), flags, snapshot.runtime.romDisposition, snapshot.runtime.fullCommit}) == true)
      modes.append(PresentationText::tr("Read-only ROM"));
    if (flags) {
      const app::OverlayFlags current{*flags};
      if (current.passthrough()) modes.append(PresentationText::tr("Freespace Passthrough"));
      if (current.persistent() && snapshot.current.overlay.type == core::OverlayType::Disk) modes.append(PresentationText::tr("Persistent Overlay"));
    }
  }
  if (compatibilityMode) modes.append(PresentationText::tr("Compatibility mode"));
  return modes.join(QStringLiteral(" · "));
}
bool MainWindowViewModel::maintenanceActionsAllowed() const {
  return !operationActive() && !refreshing() && m_state.hasCommittedState() && m_state.snapshot().uwfAvailable && m_state.snapshot().elevated &&
         !m_state.reconciliationRequired();
}
QString MainWindowViewModel::maintenanceActionRestriction() const { return maintenanceActionsAllowed() ? QString{} : writeRestriction(false); }
QString MainWindowViewModel::writeRestriction(const bool configuration) const {
  if (m_refreshFailure) return QString::fromStdString(*m_refreshFailure);
  const auto runtime = runtimeRestriction(configuration);
  if (runtime != app::RuntimeWriteRestriction::None) return runtimeWriteRestrictionText(runtime);
  if (operationActive() || refreshing()) return PresentationText::tr("Please wait for the operation to finish.");
  if (m_state.reconciliationRequired()) return PresentationText::tr("Some changes may already have been written. Refresh the system state before retrying.");
  if (!m_state.hasCommittedState() || !m_state.snapshot().uwfAvailable) return PresentationText::tr("UWF is currently unavailable");
  if (!m_state.snapshot().elevated) return PresentationText::tr("Administrator privileges are required; the overlay configuration is read-only.");
  return {};
}

bool MainWindowViewModel::directRestartAllowed() const {
  return m_state.hasCommittedState() && !m_refreshFailure && !operationActive() && m_state.snapshot().elevated && m_state.snapshot().uwfAvailable;
}
bool MainWindowViewModel::powerActionsAllowed() const { return directRestartAllowed(); }
QString MainWindowViewModel::directRestartRestriction() const {
  if (directRestartAllowed()) return {};
  if (m_refreshFailure) return QString::fromStdString(*m_refreshFailure);
  if (operationActive()) return PresentationText::tr("Please wait for the operation to finish.");
  if (!m_state.hasCommittedState() || !m_state.snapshot().uwfAvailable) return PresentationText::tr("UWF is currently unavailable");
  return PresentationText::tr("Administrator privileges are required.");
}
bool MainWindowViewModel::exitAllowed() const { return !operationActive(); }
app::RuntimeWriteRestriction MainWindowViewModel::stagingEditingRestriction() const {
  if (m_refreshFailure) return app::RuntimeWriteRestriction::StateUnavailable;
  const auto runtime = runtimeRestriction(true);
  if (runtime != app::RuntimeWriteRestriction::None) return runtime;
  if (!m_state.hasCommittedState() || !m_state.snapshot().uwfAvailable) return app::RuntimeWriteRestriction::None;
  switch (core::registryWriteDisposition(core::configurationModeState(m_state.snapshot()))) {
    case core::RegistryWriteDisposition::Direct:
    case core::RegistryWriteDisposition::RequiresCommit:
      return app::RuntimeWriteRestriction::None;
    case core::RegistryWriteDisposition::HormActive:
      return app::RuntimeWriteRestriction::HormActive;
    case core::RegistryWriteDisposition::ReadOnlyMediaActive:
      return app::RuntimeWriteRestriction::ReadOnlyMediaActive;
    case core::RegistryWriteDisposition::Unknown:
      return app::RuntimeWriteRestriction::StateUnavailable;
  }
  return app::RuntimeWriteRestriction::StateUnavailable;
}
void MainWindowViewModel::publishInteractionAvailability() {
  const QPointer guard(this);
  if (m_power) m_power->setSuspended(operationActive());
  if (m_overlayMaintenance)
    m_overlayMaintenance->setInteractionContext(m_state.hasCommittedState() && m_state.snapshot().elevated,
                                                refreshing() || operationActive() || m_state.reconciliationRequired());
  if (m_applyPlan) {
    const auto directRestriction = directRestartRestriction();
    m_applyPlan->setInteractionAvailability({.apply = configurationWritesAllowed() || (m_applyPlan->onlyDisablesHorm() && configurationReviewAllowed()),
                                             .safeRestart = powerActionsAllowed(),
                                             .directRestart = directRestartAllowed(),
                                             .safeRestartRestriction = directRestriction,
                                             .directRestartRestriction = directRestriction});
  }
  if (!guard) return;
  if (m_configuration) {
    m_configuration->setEditingAllowed(configurationWritesAllowed());
    if (!guard) return;
    m_configuration->setCommitContext(maintenanceActionsAllowed(), maintenanceActionRestriction());
  }
  if (guard) emit interactionAvailabilityChanged();
}
void MainWindowViewModel::setExternalOperationActive(const bool active) {
  if (m_externalOperationActive == active) return;
  m_externalOperationActive = active;
  publishInteractionAvailability();
}
void MainWindowViewModel::setManualCommitActive(const bool active) {
  if (m_manualCommitActive == active) return;
  m_manualCommitActive = active;
  publishInteractionAvailability();
}
void MainWindowViewModel::trackApplyPlan(std::shared_ptr<ApplyPlanViewModel> plan) {
  if (operationActive()) return;
  if (m_applyPlan) disconnect(m_applyPlan.get(), nullptr, this, nullptr);
  m_applyPlan = std::move(plan);
  connect(m_applyPlan.get(), &ApplyPlanViewModel::resultChanged, this, [this] {
    const QPointer guard(this);
    const auto completed = m_applyPlan;
    if (!completed) return;
    if (m_configuration) m_configuration->reconcileApplied(completed->configurationChanges(), completed->result());
    // Preserve the complete remaining target before any observation can replace
    // its baseline or rebuild the volume editors.
    if (guard && completed->result().reconciliationRequired()) QTimer::singleShot(0, this, &MainWindowViewModel::refreshPublishedState);
  });
  connect(m_applyPlan.get(), &ApplyPlanViewModel::applyingChanged, this, [this] {
    const QPointer guard(this);
    emit configurationApplyingChanged(applyingConfiguration());
    if (guard) publishInteractionAvailability();
  });
  connect(m_applyPlan.get(), &ApplyPlanViewModel::reconciliationRequired, this, [this] { requireReconciliation(); });
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
std::optional<QString> MainWindowViewModel::workspaceDiagnostic() const {
  // The latest read failure takes precedence over the retained snapshot's capability diagnostic.
  if (m_refreshFailure) return m_refreshFailure->empty() ? PresentationText::tr("UWF namespace is not available") : QString::fromStdString(*m_refreshFailure);
  if (m_state.hasCommittedState() && !m_state.snapshot().uwfAvailable) {
    const auto& reason = m_state.snapshot().unavailableReason;
    return reason.empty() ? PresentationText::tr("UWF namespace is not available") : QString::fromStdString(reason);
  }
  return std::nullopt;
}
void MainWindowViewModel::refresh() { refreshImpl(true); }
void MainWindowViewModel::refreshDiscardingChanges() { refreshImpl(true, ReadPurpose::Discard); }
void MainWindowViewModel::refreshPublishedState() { refreshImpl(false); }
void MainWindowViewModel::refreshImpl(const bool refreshConfiguration, const ReadPurpose purpose) {
  if (applyingConfiguration() || m_externalOperationActive || m_manualCommitActive || (m_staging && m_staging->busy() && !m_refreshingStaging)) return;
  m_refresh.cancel();
  const bool previouslyReadingStaging = m_refreshingStaging;
  UWF_LOG_D("ui") << "state refresh started";
  const auto t0 = std::chrono::steady_clock::now();
  const QPointer guard(this);
  const auto staging = m_staging;
  const auto stagingRevision = staging ? staging->revision() : 0;
  bool discardStaging = false;
  app::FileStagingDraft::ReadOperation stagingRead;
  if (staging) {
    const bool changed = staging->changes().has_value();
    // Only actual staging intent or an unknown write baseline makes discarding
    // depend on this read. Optional staging failures do not invalidate system facts.
    discardStaging = purpose == ReadPurpose::Discard && (changed || staging->reconciliationRequired());
    if (purpose != ReadPurpose::Observe || staging->reconciliationRequired() || !staging->storageAvailable() || !changed)
      stagingRead = staging->prepareRead({}, purpose == ReadPurpose::Review || (!discardStaging && (staging->reconciliationRequired() || changed)));
  }
  app::ApplicationStateSource::ReadOperation read;
  try {
    if (purpose != ReadPurpose::Review)
      read = refreshConfiguration ? m_stateSource.readOperation(m_uwfCapability) : m_stateSource.publishedOperation(m_uwfCapability);
  } catch (...) {
    const auto failure = std::current_exception();
    read = [failure](std::stop_token) -> app::ApplicationState { std::rethrow_exception(failure); };
  }
  if (!guard) return;
  struct RefreshResult {
    std::optional<app::ApplicationState> state;
    std::exception_ptr stateFailure;
    std::optional<app::FileStagingDraft::ReadResult> staging;
    std::exception_ptr stagingFailure;
  };
  m_refreshingStaging = static_cast<bool>(stagingRead);
  if (staging && (m_refreshingStaging || previouslyReadingStaging)) staging->setBusy(m_refreshingStaging);
  if (!guard) return;
  m_refresh.start(
      [read = std::move(read), stagingRead = std::move(stagingRead)](const std::stop_token stop) {
        RefreshResult result;
        if (read) {
          try {
            result.state = read(stop);
          } catch (...) {
            result.stateFailure = std::current_exception();
          }
        }
        if (stagingRead) {
          try {
            result.staging = stagingRead(stop);
          } catch (...) {
            result.stagingFailure = std::current_exception();
          }
        }
        return result;
      },
      [this, t0, staging, stagingRevision, purpose, discardStaging](AsyncResult<RefreshResult> completed) {
        RefreshResult result;
        if (auto* value = std::get_if<RefreshResult>(&completed))
          result = std::move(*value);
        else
          result.stateFailure = std::get<std::exception_ptr>(completed);
        std::optional<std::string> failure;
        if (result.stateFailure) {
          try {
            std::rethrow_exception(result.stateFailure);
          } catch (const std::exception& error) {
            failure = error.what();
          } catch (...) {
            failure = "unknown application state read failure";
          }
          UWF_LOG_W("ui") << "state refresh failed: committedState=retained error=" << *failure;
        }
        // Interaction changes and retries retain read failures until a replacement snapshot commits.
        if (failure) {
          m_refreshFailure = failure;
          m_state.requireReconciliation();
        }
        const QPointer guard(this);
        m_resettingDrafts = true;
        std::optional<QString> stagingFailure;
        if (staging) {
          if (result.stagingFailure) {
            QString detail;
            try {
              std::rethrow_exception(result.stagingFailure);
            } catch (const std::exception& error) {
              detail = QString::fromUtf8(error.what());
            } catch (...) {
              detail = PresentationText::tr("Unavailable");
            }
            stagingFailure = detail;
            staging->acceptReadFailure(stagingRevision, detail);
          } else if (result.staging && (!discardStaging || !failure)) {
            if (!staging->acceptRead(stagingRevision, std::move(*result.staging)))
              stagingFailure = PresentationText::tr("The file staging draft changed after this plan was opened. Review a new plan.");
          }
          if (!guard) return;
        }
        if (purpose != ReadPurpose::Review && !failure && (!discardStaging || !stagingFailure)) {
          m_state.commit(std::move(*result.state));
          m_refreshFailure.reset();
          if (staging && staging->reconciliationRequired()) m_state.requireReconciliation();
          resetConfiguration(purpose == ReadPurpose::Discard);
          if (!guard) return;
        }
        m_resettingDrafts = false;
        const bool releaseStaging = std::exchange(m_refreshingStaging, false);
        if (releaseStaging && staging) staging->setBusy(false);
        if (!guard) return;
        publishInteractionAvailability();
        if (!guard) return;
        publishPendingChanged();
        if (!guard) return;
        if (purpose == ReadPurpose::Review) {
          if (stagingFailure)
            emit reviewFailed(*stagingFailure);
          else if (failure)
            emit reviewFailed(QString::fromStdString(*failure));
          else
            publishReview();
          return;
        }
        if (discardStaging && stagingFailure) {
          emit refreshFailed(stagingFailure->toStdString());
          return;
        }
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
void MainWindowViewModel::retranslate() { publishInteractionAvailability(); }
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
  // SCM read failures retain confirmed status and agent ownership while scheduling a retry.
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
