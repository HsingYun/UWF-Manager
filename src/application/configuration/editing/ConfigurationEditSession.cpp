// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "application/configuration/editing/ConfigurationEditSession.h"

#include <algorithm>
#include <cstdint>
#include <deque>
#include <set>
#include <utility>

#include "application/configuration/contracts/ConfigurationActions.h"
#include "application/configuration/editing/GlobalConfigurationEditor.h"
#include "application/configuration/volumes/VolumeConfigurationEditor.h"
#include "application/uwf/contracts/UwfMaintenanceRepository.h"
#include "domain/uwf/contracts/Types.h"
#include "domain/uwf/policies/VolumeIdentityPolicy.h"
#include "foundation/paths/DriveLetter.h"

namespace uwf::app {
namespace {
std::vector<std::string> sortedPaths(const QSet<QString>& paths) {
  auto sorted = paths.values();
  std::ranges::sort(sorted, [](const QString& first, const QString& second) { return first.compare(second, Qt::CaseInsensitive) < 0; });
  sorted.removeDuplicates();
  std::vector<std::string> result;
  result.reserve(static_cast<std::size_t>(sorted.size()));
  for (const auto& path : std::as_const(sorted)) result.push_back(path.toStdString());
  return result;
}
std::vector<std::string> fileOperationPaths(const ExclusionDraft& draft, const QSet<QString>& selections, const std::string& driveLetter) {
  std::vector<std::string> result;
  result.reserve(static_cast<std::size_t>(selections.size()));
  for (const auto& path : selections) {
    if (driveLetter.empty()) {
      const auto address = draft.fileAddress(path);
      if (address && !address->volume.empty()) result.push_back(paths::volumeFilePath(*address).toStdString());
    } else {
      result.push_back(driveLetter + draft.filePattern(path).toStdString());
    }
  }
  return result;
}
QStringList strings(const std::vector<std::string>& paths) {
  QStringList result;
  for (const auto& path : paths) result.append(QString::fromStdString(path));
  return result;
}
using Editors = std::vector<std::shared_ptr<VolumeConfigurationEditor>>;

ConfigurationChanges unfinishedChanges(ConfigurationChanges requested, const ConfigurationApplyResult& result) {
  using Kind = ConfigurationOperationKind;
  std::set<std::pair<Kind, std::string>> completed;
  for (const auto& step : result.steps)
    if (step.operation && step.outcome == ConfigurationStepOutcome::Succeeded) completed.emplace(step.operation->kind, step.operation->target);
  const auto succeeded = [&](const Kind kind, const std::string& target = {}) { return completed.contains({kind, target}); };
  const auto scalar = [&](auto& field, const Kind kind) {
    if (succeeded(kind)) field.reset();
  };
  auto& settings = requested.settings;
  scalar(settings.setFilterEnabled, Kind::Filter);
  scalar(settings.setOverlay.type, Kind::OverlayType);
  scalar(settings.setOverlay.maximumSizeMb, Kind::OverlaySize);
  scalar(settings.setOverlay.warningThresholdMb, Kind::WarningThreshold);
  scalar(settings.setOverlay.criticalThresholdMb, Kind::CriticalThreshold);
  scalar(settings.setPersistDomainSecretKey, Kind::DomainPersistence);
  scalar(settings.setPersistTSCAL, Kind::LicensePersistence);
  scalar(requested.overlayPolicy.passthrough, Kind::OverlayPassthrough);
  scalar(requested.overlayPolicy.persistent, Kind::OverlayPersistent);
  scalar(requested.overlayPolicy.readOnlyMedia, Kind::OverlayReadOnlyMedia);
  scalar(requested.overlayPolicy.reset, Kind::ResetPersistentState);
  std::erase_if(settings.volumeProtect, [&](const auto& item) { return succeeded(Kind::VolumeProtection, item.first); });
  std::erase_if(settings.volumeBindings, [&](const auto& item) { return succeeded(Kind::VolumeBinding, item.first); });
  for (auto* groups : {&settings.addFileExclusions, &settings.removeFileExclusions}) {
    const auto kind = groups == &settings.addFileExclusions ? Kind::AddFileExclusion : Kind::RemoveFileExclusion;
    for (auto& [target, paths] : *groups) std::erase_if(paths, [&](const auto& path) { return succeeded(kind, target + path); });
    std::erase_if(*groups, [](const auto& group) { return group.second.empty(); });
  }
  std::erase_if(settings.addRegistryExclusions, [&](const auto& path) { return succeeded(Kind::AddRegistryExclusion, path); });
  std::erase_if(settings.removeRegistryExclusions, [&](const auto& path) { return succeeded(Kind::RemoveRegistryExclusion, path); });
  return requested;
}

void stabilizeVolumeTargets(core::PendingChanges& changes, const Editors& editors) {
  const auto editorFor = [&](const std::string& target) -> std::shared_ptr<VolumeConfigurationEditor> {
    const auto id = compat::parseVolumeId(target);
    for (const auto& editor : editors) {
      if (id ? compat::parseVolumeId(editor->volume().volumeName) == id : drive::normalize(editor->volume().driveLetter) == drive::normalize(target))
        return editor;
    }
    return {};
  };
  const auto identity = [&](const std::string& target) {
    const auto editor = editorFor(target);
    return editor ? compat::parseVolumeId(editor->volume().volumeName).value_or(target) : compat::parseVolumeId(target).value_or(target);
  };
  // A drive selector also chooses the default binding of a newly created
  // registration. Keep that part of the reviewed target when its owner is
  // stabilized to a GUID and may later appear under another letter.
  for (const auto& [target, protection] : changes.volumeProtect) {
    const auto editor = editorFor(target);
    if (protection && editor && !editor->status()->draft().hasConfiguration && !changes.volumeBindings.contains(target))
      changes.volumeBindings.emplace(target, editor->status()->draft().binding);
  }
  const auto scalars = [&](auto& values) {
    auto stable = values;
    stable.clear();
    for (const auto& [target, value] : values) stable.insert_or_assign(identity(target), value);
    values = std::move(stable);
  };
  scalars(changes.volumeProtect);
  scalars(changes.volumeBindings);
  for (auto* groups : {&changes.addFileExclusions, &changes.removeFileExclusions}) {
    auto stable = *groups;
    stable.clear();
    for (const auto& [target, selections] : *groups) {
      const auto owner = identity(target);
      const auto editor = editorFor(target);
      auto& stored = stable[owner];
      for (const auto& path : selections) {
        const auto address = editor ? editor->files()->draft().fileAddress(QString::fromStdString(path)) : std::nullopt;
        stored.push_back(address && !address->volume.empty() ? paths::volumeFilePath(*address).toStdString() : path);
      }
      core::sortExclusions(stored);
    }
    *groups = std::move(stable);
  }
}
}  // namespace

struct ConfigurationEditSession::Reset {
  enum class Mode { Refresh, Discard };
  Mode mode;
  core::UwfSnapshot snapshot;
  OverlayPolicySnapshot policy;
  std::optional<bool> hormEnabled;
};
struct ConfigurationEditSession::State {
  std::shared_ptr<GlobalConfigurationState> global;
  std::vector<std::shared_ptr<VolumeConfigurationState>> volumes;
  std::shared_ptr<GlobalConfigurationEditor> globalEditor;
  std::shared_ptr<ExclusionEditor> registryEditor;
  Editors volumeEditors, identityEditors;
  QPointer<ConfigurationEditSession> owner;
  [[nodiscard]] Editors editors() const {
    auto result = volumeEditors;
    result.insert(result.end(), identityEditors.begin(), identityEditors.end());
    return result;
  }
  QString systemDrive;
  DriveResolver resolveDrive;
  std::shared_ptr<const FileBrowser> files;
  std::shared_ptr<OverlayPolicyEditor> policy = std::make_shared<OverlayPolicyEditor>();
  std::shared_ptr<OverlayMaintenanceEditor> maintenance;
  // A single write-to-observation transition. Drafts own edits again after the
  // next successful reset; rebuilding their replaceable volume views cannot
  // consume an unfinished reviewed target.
  std::optional<ConfigurationChanges> pendingRebase;
  std::deque<Reset> resets;
  // Replacing volumes invalidates only their captured notification recipients.
  // A newer observation also supersedes the global/registry/policy snapshot.
  std::uint64_t volumeGeneration = 0;
  std::uint64_t observationGeneration = 0;
  bool active = true;
  bool editingAllowed = true;
  bool snapshotEditable = true;
  core::ConfigurationWriteRestriction configurationRestriction = core::ConfigurationWriteRestriction::None;
  bool resetting = false;
};

// All configuration partitions share one commit/publication boundary. Each
// editor supplies only its own notifications through EditorUpdate.
struct ConfigurationEditSession::Publication {
  explicit Publication(std::shared_ptr<State> owner)
      : state(std::move(owner)),
        volumeGeneration(state->volumeGeneration),
        observationGeneration(state->observationGeneration),
        global(state->globalEditor),
        policy(state->policy),
        registry(state->registryEditor),
        volumes(state->editors()),
        globalUpdate(global->beginUpdate()),
        policyUpdate(policy->beginUpdate()),
        registryUpdate(registry->beginUpdate()),
        maintenanceUpdate(state->maintenance->beginUpdate()) {
    volumeUpdates.reserve(volumes.size());
    for (const auto& volume : volumes) volumeUpdates.push_back(volume->beginUpdate());
  }
  void resume() {
    globalUpdate.resume();
    policyUpdate.resume();
    registryUpdate.resume();
    maintenanceUpdate.resume();
    for (auto& update : volumeUpdates) update.resume();
  }
  void publishAvailability() {
    const auto active = [&] { return state->active; };
    const auto current = [&] { return state->active && state->volumeGeneration == volumeGeneration; };
    if (!globalUpdate.publishAvailability(active) || !policyUpdate.publishAvailability(active) || !registryUpdate.publishAvailability(active)) return;
    for (auto& update : volumeUpdates)
      if (!update.publishAvailability(current)) break;
  }
  bool publishSnapshot(bool maintenanceChanged) {
    const auto current = [&] { return state->active && state->volumeGeneration == volumeGeneration; };
    const auto observed = [&] { return state->active && state->observationGeneration == observationGeneration; };
    for (auto& update : volumeUpdates)
      if (!update.publishSnapshot(current)) break;
    if (!registryUpdate.publishSnapshot(observed) || !policyUpdate.publishSnapshot(observed) || !globalUpdate.publishSnapshot(observed)) return false;
    return !maintenanceChanged || maintenanceUpdate.publishSnapshot(observed);
  }
  std::shared_ptr<State> state;
  std::uint64_t volumeGeneration, observationGeneration;
  std::shared_ptr<GlobalConfigurationEditor> global;
  std::shared_ptr<OverlayPolicyEditor> policy;
  std::shared_ptr<ExclusionEditor> registry;
  Editors volumes;
  GlobalConfigurationEditor::Update globalUpdate;
  OverlayPolicyEditor::Update policyUpdate;
  ExclusionEditor::Update registryUpdate;
  OverlayMaintenanceEditor::Update maintenanceUpdate;
  std::vector<VolumeConfigurationEditor::Update> volumeUpdates;
};

ConfigurationEditSession::ConfigurationEditSession(uint32_t totalRamMb, QString systemDrive, DriveResolver resolveDrive,
                                                   std::shared_ptr<const FileBrowser> files, std::shared_ptr<OverlayMaintenanceEditor> maintenance)
    : m_state(std::make_shared<State>()) {
  const auto state = m_state;
  state->owner = this;
  state->maintenance = maintenance ? std::move(maintenance) : std::make_shared<OverlayMaintenanceEditor>();
  state->global = std::make_shared<GlobalConfigurationState>();
  state->globalEditor = std::make_shared<GlobalConfigurationEditor>(state->global, totalRamMb);
  state->registryEditor =
      std::make_shared<ExclusionEditor>(ExclusionEditor::Kind::Registry, std::shared_ptr<ExclusionDraft>(state->global, &state->global->registry));
  state->systemDrive = std::move(systemDrive);
  state->resolveDrive = resolveDrive ? std::move(resolveDrive) : DriveResolver{[](const std::string& path) { return drive::split(path).letter; }};
  state->files = std::move(files);
  auto* global = state->globalEditor.get();
  auto* policy = state->policy.get();
  policy->setMaintenanceEditor(state->maintenance);
  QObject::connect(global, &GlobalConfigurationEditor::stateChanged, policy, [global, policy] { policy->setOverlayChanges(global->pendingOverlay()); });
  QObject::connect(global, &GlobalConfigurationEditor::availabilityChanged, policy,
                   [global, policy] { policy->setControlsEnabled(global->controlsEnabled()); });
  policy->setControlsEnabled(global->controlsEnabled());
  connect(global, &GlobalConfigurationEditor::pendingChanged, this, &ConfigurationEditSession::pendingChanged);
  connect(policy, &OverlayPolicyEditor::pendingChanged, this, &ConfigurationEditSession::pendingChanged);
  connect(state->registryEditor.get(), &ExclusionEditor::pendingChanged, this, &ConfigurationEditSession::pendingChanged);
  connect(state->maintenance.get(), &OverlayMaintenanceEditor::pendingChanged, this, &ConfigurationEditSession::pendingChanged);
  QObject::connect(state->maintenance.get(), &OverlayMaintenanceEditor::bulkProtectionChanged, state->maintenance.get(),
                   [weak = std::weak_ptr<State>(state)](const bool resetOverrides) {
                     if (const auto current = weak.lock()) updateBulkProtection(current, resetOverrides);
                   });
}
ConfigurationEditSession::~ConfigurationEditSession() {
  const auto state = m_state;
  state->active = false;
  ++state->volumeGeneration;
  state->resets.clear();
  // An in-progress reset may release its retained state after facade destruction,
  // but must not restore editing. Accepted imports retain their own editor lifetime.
  if (state->resetting) updateEditingAvailability(state, false);
}
const std::shared_ptr<GlobalConfigurationEditor>& ConfigurationEditSession::globalEditor() const { return m_state->globalEditor; }
const std::shared_ptr<ExclusionEditor>& ConfigurationEditSession::registryEditor() const { return m_state->registryEditor; }
const std::shared_ptr<GlobalConfigurationState>& ConfigurationEditSession::global() const { return m_state->global; }
const std::vector<std::shared_ptr<VolumeConfigurationState>>& ConfigurationEditSession::volumes() const { return m_state->volumes; }
const std::shared_ptr<OverlayPolicyEditor>& ConfigurationEditSession::overlayPolicyEditor() const { return m_state->policy; }
const std::shared_ptr<OverlayMaintenanceEditor>& ConfigurationEditSession::maintenanceEditor() const { return m_state->maintenance; }
ConfigurationChanges ConfigurationEditSession::pendingConfiguration() const {
  if (m_state->pendingRebase) {
    auto changes = *m_state->pendingRebase;
    changes.maintenance = m_state->maintenance->draft();
    return changes;
  }
  return {pendingChanges(), m_state->policy->pendingChanges(), m_state->maintenance->draft()};
}
void ConfigurationEditSession::reconcileApplied(const ConfigurationChanges& requestedChanges, const ConfigurationApplyResult& result) {
  const auto state = m_state;
  if (result.reconciliationRequired()) {
    auto remaining = unfinishedChanges(requestedChanges, result);
    stabilizeVolumeTargets(remaining.settings, state->editors());
    state->pendingRebase = std::move(remaining);
  }
  if (!result.observedOverlayMaintenance) return;
  const auto& requested = requestedChanges.maintenance;
  OverlayMaintenanceChanges confirmed;
  const auto bulkConfirmed = [&](const bool enabled) {
    const auto matches = [enabled](const OverlayMaintenanceSnapshot& value) {
      return enabled ? value.allVolumesNextProtected == true : value.allVolumesNextUnprotected == true;
    };
    return matches(*result.observedOverlayMaintenance) || std::ranges::any_of(result.steps, [&](const auto& receipt) {
             return receipt.outcome == ConfigurationStepOutcome::Succeeded && receipt.maintenance && matches(receipt.maintenance->snapshot);
           });
  };
  for (const auto& step : result.steps) {
    if (!step.operation || step.outcome != ConfigurationStepOutcome::Succeeded) continue;
    switch (step.operation->kind) {
      case ConfigurationOperationKind::OverlayStorage:
        confirmed.storageVolume = requested.storageVolume;
        break;
      case ConfigurationOperationKind::Horm:
        confirmed.hormEnabled = requested.hormEnabled;
        break;
      case ConfigurationOperationKind::FullCommit:
        confirmed.fullCommit = requested.fullCommit;
        break;
      case ConfigurationOperationKind::ProtectAllVolumes:
        if (bulkConfirmed(true)) confirmed.allVolumesProtected = requested.allVolumesProtected;
        break;
      case ConfigurationOperationKind::UnprotectAllVolumes:
        if (bulkConfirmed(false)) confirmed.allVolumesProtected = requested.allVolumesProtected;
        break;
      default:
        break;
    }
  }
  m_state->maintenance->reconcileAppliedDraft(confirmed, *result.observedOverlayMaintenance);
  if (state->pendingRebase) state->pendingRebase->maintenance = state->maintenance->draft();
}

void ConfigurationEditSession::updateBulkProtection(std::shared_ptr<State> state, const bool resetOverrides) {
  const auto bulk = state->maintenance->draft().allVolumesProtected;
  const auto snapshot = state->maintenance->snapshot();
  const auto editors = state->editors();
  const auto generation = state->volumeGeneration;
  for (const auto& editor : editors) {
    if (!state->active || state->volumeGeneration != generation) break;
    const auto identity = compat::parseVolumeId(editor->volume().volumeName);
    const auto& scope = bulk == true ? snapshot.protectTargetVolumes : snapshot.unprotectTargetVolumes;
    const bool confirmedScope = bulk == true ? snapshot.protectScopeConfirmed : snapshot.unprotectScopeConfirmed;
    const bool included =
        confirmedScope ? identity && std::ranges::any_of(scope, [&](const auto& target) { return compat::parseVolumeId(target.toStdString()) == identity; })
        : bulk == true ? core::canRegisterVolume(editor->volume())
                       : editor->status()->draft().hasConfiguration;
    editor->setBulkProtection(bulk && included ? bulk : std::nullopt, resetOverrides);
  }
}

void ConfigurationEditSession::setEditingAllowed(const bool allowed) {
  const auto state = m_state;
  state->editingAllowed = allowed;
  updateEditingAvailability(state);
}
void ConfigurationEditSession::updateEditingAvailability(std::shared_ptr<State> state, const bool notify) {
  const bool allowed = state->active && state->editingAllowed && state->snapshotEditable && !state->resetting;
  Publication update(state);
  update.global->setControlsEnabled(allowed);
  update.policy->setControlsEnabled(update.global->controlsEnabled());
  const bool ordinaryAllowed = allowed && state->configurationRestriction == core::ConfigurationWriteRestriction::None;
  update.registry->setReadOnly(!ordinaryAllowed);
  for (const auto& volume : update.volumes) volume->setEditingAllowed(allowed);
  update.resume();
  if (notify) update.publishAvailability();
}

void ConfigurationEditSession::resetSnapshot(const core::UwfSnapshot& snapshot, const OverlayPolicySnapshot& policy, const std::optional<bool> hormEnabled) {
  enqueueReset(m_state, {Reset::Mode::Refresh, snapshot, policy, hormEnabled});
}
void ConfigurationEditSession::discardChanges(const core::UwfSnapshot& snapshot, const OverlayPolicySnapshot& policy, const std::optional<bool> hormEnabled) {
  enqueueReset(m_state, {Reset::Mode::Discard, snapshot, policy, hormEnabled});
}
void ConfigurationEditSession::enqueueReset(std::shared_ptr<State> state, Reset reset) {
  if (!state->active) return;
  state->resets.push_back(std::move(reset));
  ++state->volumeGeneration;
  ++state->observationGeneration;
  if (state->resetting) return;
  // Notifications may destroy the facade, replace volumes, or enqueue another
  // reset. Retain the state and publish only the current generation.
  state->resetting = true;
  while (state->active && !state->resets.empty()) {
    auto request = std::move(state->resets.front());
    state->resets.pop_front();
    if (request.mode == Reset::Mode::Discard) state->pendingRebase.reset();
    const auto rebase = state->pendingRebase;
    const auto volumeGeneration = state->volumeGeneration;
    const auto current = [&] { return state->active && state->volumeGeneration == volumeGeneration; };
    updateEditingAvailability(state);
    if (!current()) continue;
    for (const auto& editor : state->identityEditors) QObject::disconnect(editor.get(), nullptr, state->owner.data(), nullptr);
    state->identityEditors.clear();
    for (const auto& record : request.snapshot.next.volumes) {
      const bool represented = std::ranges::any_of(
          state->volumeEditors, [&](const auto& editor) { return core::findVolumeConfiguration(request.snapshot.next, editor->volume()) == &record; });
      if (represented) continue;
      core::VolumeInfo identity{.driveLetter = {}, .volumeName = record.volumeName, .fileSystem = {}, .label = {}};
      auto editor = std::make_shared<VolumeConfigurationEditor>(std::move(identity), false);
      editor->setEditingAllowed(false);
      QObject::connect(editor.get(), &VolumeConfigurationEditor::pendingChanged, state->owner.data(), &ConfigurationEditSession::pendingChanged);
      state->identityEditors.push_back(std::move(editor));
    }
    if (rebase) {
      const auto ensureIdentity = [&](const std::string& target) {
        const auto id = compat::parseVolumeId(target);
        if (!id) return;
        const auto editors = state->editors();
        if (std::ranges::any_of(editors, [&](const auto& editor) { return compat::parseVolumeId(editor->volume().volumeName) == id; })) return;
        core::VolumeInfo identity{.driveLetter = {}, .volumeName = "Volume{" + *id + '}', .fileSystem = {}, .label = {}};
        auto editor = std::make_shared<VolumeConfigurationEditor>(std::move(identity), false);
        editor->setEditingAllowed(false);
        QObject::connect(editor.get(), &VolumeConfigurationEditor::pendingChanged, state->owner.data(), &ConfigurationEditSession::pendingChanged);
        state->identityEditors.push_back(std::move(editor));
      };
      for (const auto& [target, value] : rebase->settings.volumeProtect) {
        (void)value;
        ensureIdentity(target);
      }
      for (const auto& [target, value] : rebase->settings.volumeBindings) {
        (void)value;
        ensureIdentity(target);
      }
      for (const auto* groups : {&rebase->settings.addFileExclusions, &rebase->settings.removeFileExclusions})
        for (const auto& [target, selections] : *groups) {
          (void)selections;
          ensureIdentity(target);
        }
    }
    Publication update(state);
    const auto& global = update.global;
    const auto& policy = update.policy;
    const auto& registry = update.registry;
    const auto& volumes = update.volumes;
    const bool discardMaintenance = request.mode == Reset::Mode::Discard && !state->maintenance->draft().empty();
    {
      auto snapshot = request.snapshot;
      const auto observedHorm = request.hormEnabled ? request.hormEnabled : snapshot.runtime.hormEnabled;
      state->snapshotEditable = snapshot.uwfAvailable && snapshot.elevated && hormAllowsChanges(observedHorm);
      if (snapshot.uwfAvailable) global->reset(snapshot.next);
      global->setAvailable(snapshot.uwfAvailable);
      global->setOverlayConfigLocked(snapshot.uwfAvailable && snapshot.current.filter.enabled);
      const auto& flags = request.policy.current.flags;
      const auto currentFlags =
          flags.status == PolicyObservationStatus::Available && flags.value ? std::optional<std::uint32_t>{flags.value->raw} : std::nullopt;
      auto mode = core::configurationModeState(snapshot);
      if (observedHorm) mode.runtime.hormEnabled = observedHorm;
      if (currentFlags) mode.runtime.overlayFlags = currentFlags;
      if (!mode.systemVolumeProtected && !state->systemDrive.isEmpty()) {
        const auto* system = core::findVolumeConfiguration(snapshot.current, state->systemDrive.toStdString());
        if (system) mode.systemVolumeProtected = system->isProtected;
      }
      // Every editor receives the same resolved SYSTEM observation. Missing
      // mount information remains unknown rather than implying no protection.
      snapshot.runtime.systemVolumeProtected = mode.systemVolumeProtected;
      state->configurationRestriction = core::configurationWriteRestriction(mode);
      global->setMode(mode);
      for (const auto& volume : volumes) volume->applySnapshot(snapshot, observedHorm, currentFlags);
      policy->reset(request.policy, snapshot, observedHorm);
      if (discardMaintenance) state->maintenance->discardDraft();
      updateBulkProtection(state);
      registry->setBaseline(strings(snapshot.current.registryExclusions), strings(snapshot.next.registryExclusions));
      registry->setPersistBaseline(snapshot.current.persistDomainSecretKey, snapshot.next.persistDomainSecretKey, snapshot.current.persistTSCAL,
                                   snapshot.next.persistTSCAL);
      if (rebase) {
        const auto& changes = rebase->settings;
        global->restoreChanges(changes.setFilterEnabled, changes.setOverlay);
        policy->setOverlayChanges(global->pendingOverlay());
        policy->restoreChanges(rebase->overlayPolicy);
        auto& draft = state->global->registry;
        for (const auto& path : changes.addRegistryExclusions) (void)draft.add(QString::fromStdString(path));
        for (const auto& path : changes.removeRegistryExclusions) (void)draft.remove(QString::fromStdString(path));
        if (changes.setPersistDomainSecretKey && *changes.setPersistDomainSecretKey != draft.domainSecretKey.baseNext)
          draft.domainSecretKey.pendingNext = changes.setPersistDomainSecretKey;
        if (changes.setPersistTSCAL && *changes.setPersistTSCAL != draft.tscal.baseNext) draft.tscal.pendingNext = changes.setPersistTSCAL;
        for (const auto& volume : volumes) {
          const auto id = compat::parseVolumeId(volume->volume().volumeName);
          const auto target = id.value_or(volume->volume().driveLetter);
          const auto protection = changes.volumeProtect.find(target);
          const auto binding = changes.volumeBindings.find(target);
          const auto added = changes.addFileExclusions.find(target);
          const auto removed = changes.removeFileExclusions.find(target);
          volume->restoreChanges(protection == changes.volumeProtect.end() ? std::nullopt : std::optional{protection->second},
                                 binding == changes.volumeBindings.end() ? std::nullopt : std::optional{binding->second},
                                 added == changes.addFileExclusions.end() ? std::vector<std::string>{} : added->second,
                                 removed == changes.removeFileExclusions.end() ? std::vector<std::string>{} : removed->second);
        }
        // An unavailable namespace does not establish a new UWF baseline.
        if (snapshot.uwfAvailable) state->pendingRebase.reset();
      }
      registry->refreshFileStagingConstraints();
      updateEditingAvailability(state, false);
    }
    update.resume();
    if (!update.publishSnapshot(discardMaintenance)) continue;
  }
  state->resetting = false;
  if (state->active) updateEditingAvailability(state);
}

std::shared_ptr<VolumeConfigurationEditor> ConfigurationEditSession::editVolume(const core::VolumeInfo& volume, bool hostsRegistry) {
  const auto state = m_state;
  auto editor = std::make_shared<VolumeConfigurationEditor>(volume, hostsRegistry, this, state->files);
  editor->files()->setSystemDriveLetter(state->systemDrive);
  editor->setEditingAllowed(state->active && state->editingAllowed && state->snapshotEditable && !state->resetting);
  connect(editor.get(), &VolumeConfigurationEditor::pendingChanged, this, &ConfigurationEditSession::pendingChanged);
  state->volumeEditors.push_back(editor);
  updateBulkProtection(state);
  return editor;
}
QList<ImportResult> ConfigurationEditSession::importCommands(const QList<app::UwfmgrCommand>& commands) {
  VolumeConfigurationEditors editors;
  const auto global = m_state->globalEditor;
  const auto registry = m_state->registryEditor;
  const auto policy = m_state->policy;
  const auto resolveDrive = m_state->resolveDrive;
  auto registryBatch = registry->beginBatch();
  std::vector<ExclusionEditor::BatchUpdate> fileBatches;
  {
    const auto targets = m_state->editors();
    editors.reserve(targets.size());
    fileBatches.reserve(targets.size());
    for (const auto& editor : targets) {
      editors.emplace_back(editor);
      fileBatches.push_back(editor->files()->beginBatch());
    }
  }
  const auto maintenance = m_state->maintenance;
  const auto maintenanceBatch = maintenance->beginBatch();
  return applyImportCommands(
      commands, global.get(), editors, registry.get(), resolveDrive, policy.get(),
      [maintenance, state = m_state](const bool enabled) {
        const auto before = maintenance->draft().allVolumesProtected;
        const auto targets = state->editors();
        const bool overrides = std::ranges::any_of(targets, [](const auto& editor) {
          const auto& draft = editor->status()->draft();
          return draft.bulkProtection && draft.pendingProtection().has_value();
        });
        if (!maintenance->stageAllVolumesProtection(enabled)) return ExclusionImportOutcome::RejectedReadOnly;
        return before == enabled && !overrides ? ExclusionImportOutcome::NoOp : ExclusionImportOutcome::Applied;
      },
      [maintenance, state = m_state](const UwfmgrCommand& command) {
        // HORM can lock configuration while its own disable operation remains
        // available. Window/session admission still applies to every intent.
        if (!state->active || !state->editingAllowed || state->resetting) return ExclusionImportOutcome::RejectedReadOnly;
        const auto before = maintenance->draft();
        bool accepted = false;
        switch (command.kind) {
          case UwfmgrKind::FilterEnableHorm:
          case UwfmgrKind::FilterDisableHorm:
            accepted = command.args.empty() && maintenance->stageHorm(command.kind == UwfmgrKind::FilterEnableHorm);
            break;
          case UwfmgrKind::OverlayCommit:
            accepted = command.args.empty() && maintenance->stageFullCommit();
            break;
          case UwfmgrKind::VolumeCreateSwapfile: {
            if (command.args.size() != 1 || !maintenance->canCreateSwap()) return ExclusionImportOutcome::RejectedReadOnly;
            const auto identity = compat::parseVolumeId(command.args.front());
            const auto letter = drive::normalize(command.args.front());
            std::optional<QString> target;
            // Resolve against observed physical inventory only; saved Next
            // drive aliases cannot select a runtime storage host.
            for (const auto& volume : maintenance->snapshot().volumes) {
              const auto candidate = compat::parseVolumeId(volume.id.toStdString());
              if (!candidate || (identity ? candidate != identity : letter.empty() || drive::normalize(volume.driveLetter.toStdString()) != letter)) continue;
              if (target && compat::parseVolumeId(target->toStdString()) != candidate) return ExclusionImportOutcome::RejectedMissingVolume;
              target = volume.id;
            }
            if (!target) return ExclusionImportOutcome::RejectedMissingVolume;
            accepted = maintenance->stageStorageLocation(*target);
            break;
          }
          default:
            return ExclusionImportOutcome::RejectedReadOnly;
        }
        if (!accepted) return ExclusionImportOutcome::RejectedReadOnly;
        return before == maintenance->draft() ? ExclusionImportOutcome::NoOp : ExclusionImportOutcome::Applied;
      });
}
std::shared_ptr<VolumeConfigurationState> ConfigurationEditSession::addVolume(const core::VolumeInfo& volume) {
  const auto state = m_state;
  auto result = std::make_shared<VolumeConfigurationState>(volume);
  ++state->volumeGeneration;
  state->volumes.push_back(result);
  return result;
}
void ConfigurationEditSession::clearVolumes() {
  const auto state = m_state;
  ++state->volumeGeneration;
  const auto volumeGeneration = state->volumeGeneration;
  auto volumes = state->editors();
  state->volumeEditors.clear();
  state->identityEditors.clear();
  for (const auto& volume : volumes) QObject::disconnect(volume.get(), nullptr, this, nullptr);
  state->volumes.clear();
  std::vector<VolumeConfigurationEditor::Update> updates;
  updates.reserve(volumes.size());
  for (const auto& volume : volumes) updates.push_back(volume->beginUpdate());
  for (const auto& volume : volumes) volume->setEditingAllowed(false);
  for (auto& update : updates) update.resume();
  const auto current = [&] { return state->active && state->volumeGeneration == volumeGeneration; };
  for (auto& update : updates)
    if (!update.publishAvailability(current)) break;
}

core::PendingChanges ConfigurationEditSession::pendingChanges() const {
  const auto state = m_state;
  if (state->pendingRebase) return state->pendingRebase->settings;
  core::PendingChanges changes;
  if (state->global->available) {
    changes.setFilterEnabled = state->global->draft.pendingFilterEnabled();
    changes.setOverlay = state->global->draft.pendingOverlay();
    changes.addRegistryExclusions = sortedPaths(state->global->registry.added());
    changes.removeRegistryExclusions = sortedPaths(state->global->registry.removed());
    changes.setPersistDomainSecretKey = state->global->registry.domainSecretKey.pendingNext;
    changes.setPersistTSCAL = state->global->registry.tscal.pendingNext;
  }
  const auto appendVolume = [&](const VolumeConfigurationDraft& draft, const ExclusionDraft& files, const std::string& letter) {
    if (draft.protectionTarget.empty()) return;
    if (const auto protection = draft.pendingProtection()) changes.volumeProtect[draft.protectionTarget] = *protection;
    if (const auto binding = draft.pendingBinding()) changes.volumeBindings[draft.protectionTarget] = *binding;
    if (const auto paths = fileOperationPaths(files, files.added(), letter); !paths.empty()) {
      auto& added = changes.addFileExclusions[draft.protectionTarget];
      added.insert(added.end(), paths.begin(), paths.end());
    }
    if (const auto paths = fileOperationPaths(files, files.removed(), letter); !paths.empty()) {
      auto& removed = changes.removeFileExclusions[draft.protectionTarget];
      removed.insert(removed.end(), paths.begin(), paths.end());
    }
  };
  for (const auto& volume : state->volumes)
    if (volume->supported) appendVolume(volume->draft, volume->files, volume->driveLetter);
  for (const auto& editor : state->identityEditors) appendVolume(editor->status()->draft(), editor->files()->draft(), {});
  // Group after all editors have contributed: both address aliases and repeated
  // projections of one volume must dispatch each exact provider pattern once.
  for (auto* groups : {&changes.addFileExclusions, &changes.removeFileExclusions})
    for (auto& [volume, paths] : *groups) {
      (void)volume;
      core::sortExclusions(paths);
    }
  return changes;
}
}  // namespace uwf::app

#include "moc_ConfigurationEditSession.cpp"
