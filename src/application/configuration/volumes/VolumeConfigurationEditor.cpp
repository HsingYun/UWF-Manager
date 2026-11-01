// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "application/configuration/volumes/VolumeConfigurationEditor.h"

#include <utility>

#include "application/configuration/editing/ConfigurationEditSession.h"
#include "application/uwf/contracts/UwfMaintenanceRepository.h"
#include "domain/uwf/contracts/Types.h"
#include "domain/uwf/policies/VolumeIdentityPolicy.h"
#include "foundation/paths/DriveLetter.h"

namespace uwf::app {
namespace {
QStringList strings(const std::vector<std::string>& values) {
  QStringList result;
  result.reserve(static_cast<int>(values.size()));
  for (const auto& value : values) result << QString::fromStdString(value);
  return result;
}

QStringList fileExclusions(const core::SessionSnapshot& snapshot, const core::VolumeRecord* volume) {
  if (!volume) return {};
  const auto found = snapshot.fileExclusions.find(volume->volumeName);
  return found == snapshot.fileExclusions.end() ? QStringList{} : strings(found->second);
}
void updateFileTarget(const std::shared_ptr<VolumeConfigurationState>& state, ExclusionEditor& files) {
  const auto& draft = state->draft;
  files.setFileAddTargetAvailable(draft.hasPlannedConfiguration());
}
}  // namespace

QString VolumeConfigurationEditor::volumeIdentity(const core::VolumeInfo& volume) {
  return volume.volumeName.empty() ? QString::fromStdString(volume.driveLetter).toUpper() : QString::fromStdString(core::volumeIdentity(volume.volumeName));
}

QString VolumeConfigurationEditor::configurationVolumeId() const {
  return m_snapshot.next ? QString::fromStdString(compat::parseVolumeId(m_snapshot.next->volumeName).value_or(std::string{})) : QString{};
}

VolumeConfigurationEditor::VolumeConfigurationEditor(core::VolumeInfo volume, const bool hostsRegistry, app::ConfigurationEditSession* session,
                                                     std::shared_ptr<const FileBrowser> files)
    : m_volume(std::move(volume)) {
  m_state = session ? session->addVolume(m_volume) : std::make_shared<app::VolumeConfigurationState>(m_volume);
  m_state->conflicts.setVolumeIdentity(driveLetter(), m_volume.volumeName);
  m_status = std::make_shared<VolumeStatusEditor>(m_state);
  // Provider-relative paths on an unlettered volume have no filesystem
  // address in this editor. Do not inspect the same spelling on another drive.
  if (m_state->identityOnly) files.reset();
  m_files = std::make_shared<ExclusionEditor>(ExclusionEditor::Kind::File, std::shared_ptr<app::ExclusionDraft>(m_state, &m_state->files), &m_state->conflicts,
                                              std::move(files));
  m_files->setDriveLetter(driveLetter());
  m_files->setFileVolumeIdentity(m_volume.volumeName);
  connect(m_status.get(), &VolumeStatusEditor::stateChanged, m_files.get(), [state = m_state, files = m_files.get()] { updateFileTarget(state, *files); });
  updateFileTarget(m_state, *m_files);
  if (!m_state->supported) m_status->setUnsupported();
  m_files->setReadOnly(m_state->identityOnly || !core::supportsFileOverlayOperations(m_volume.support));
  if (hostsRegistry && !m_volume.driveLetter.empty()) {
    m_ownsRegistry = session == nullptr;
    m_registry = session ? session->registryEditor() : std::make_shared<ExclusionEditor>(ExclusionEditor::Kind::Registry);
    if (m_ownsRegistry) connect(m_registry.get(), &ExclusionEditor::pendingChanged, this, &VolumeConfigurationEditor::pendingChanged);
  }
  connect(m_files.get(), &ExclusionEditor::pendingChanged, this, &VolumeConfigurationEditor::pendingChanged);
  connect(m_status.get(), &VolumeStatusEditor::pendingChanged, this, &VolumeConfigurationEditor::pendingChanged);
}

VolumeConfigurationEditor::Update::Update(VolumeConfigurationEditor& editor)
    : EditorUpdate(editor, {editor.m_status.get(), editor.m_files.get(), editor.m_ownsRegistry ? editor.m_registry.get() : nullptr}),
      m_status(editor.m_status),
      m_files(editor.m_files),
      m_registry(editor.m_ownsRegistry ? editor.m_registry : nullptr),
      m_supported(editor.m_volume.driveLetter.empty() || core::supportsVolumeProtection(editor.m_volume.support)),
      m_publishStatus(m_supported || m_status->draft().supported || m_status->draft().hasTarget),
      m_statusControlsBefore(m_status->controlsEnabled()),
      m_statusEditingBefore(m_status->editingAllowed()),
      m_filesReadOnlyBefore(m_files->readOnly()),
      m_filesCanAddBefore(m_files->canAdd()),
      m_registryReadOnlyBefore(m_registry && m_registry->readOnly()),
      m_editingAllowedBefore(editor.m_editingAllowed) {}
bool VolumeConfigurationEditor::Update::publishSnapshot(const std::function<bool()>& continuePublishing) {
  if (m_publishStatus &&
      (!publish(continuePublishing, [&] { emit m_status->stateChanged(); }) || !publish(continuePublishing, [&] { emit m_status->availabilityChanged(); })))
    return false;
  if (m_supported && (!publish(continuePublishing, [&] { emit m_files->baselineChanged(); }) || !publish(continuePublishing, [&] {
        emit m_files->stateChanged();
      }) || !publish(continuePublishing, [&] { emit m_files->pendingChanged(); })))
    return false;
  if (m_registry && (!publish(continuePublishing, [&] { emit m_registry->baselineChanged(); }) || !publish(continuePublishing, [&] {
        emit m_registry->stateChanged();
      }) || !publish(continuePublishing, [&] { emit m_registry->pendingChanged(); })))
    return false;
  if ((m_filesReadOnlyBefore != m_files->readOnly() || m_filesCanAddBefore != m_files->canAdd()) &&
      !publish(continuePublishing, [&] { emit m_files->editingAvailabilityChanged(); }))
    return false;
  if (m_registry && m_registryReadOnlyBefore != m_registry->readOnly() && !publish(continuePublishing, [&] { emit m_registry->editingAvailabilityChanged(); }))
    return false;
  return publish(continuePublishing, [&] { emit m_editor->stateChanged(); });
}
bool VolumeConfigurationEditor::Update::publishAvailability(const std::function<bool()>& continuePublishing) {
  if (!current(continuePublishing)) return false;
  bool changed = false;
  if (m_statusControlsBefore != m_status->controlsEnabled() || m_statusEditingBefore != m_status->editingAllowed()) {
    changed = true;
    if (!publish(continuePublishing, [&] { emit m_status->availabilityChanged(); })) return false;
  }
  if (m_filesReadOnlyBefore != m_files->readOnly() || m_filesCanAddBefore != m_files->canAdd()) {
    changed = true;
    if (!publish(continuePublishing, [&] { emit m_files->editingAvailabilityChanged(); })) return false;
  }
  if (m_registry && m_registryReadOnlyBefore != m_registry->readOnly()) {
    changed = true;
    if (!publish(continuePublishing, [&] { emit m_registry->editingAvailabilityChanged(); })) return false;
  }
  return changed || m_editingAllowedBefore != m_editor->m_editingAllowed ? publish(continuePublishing, [&] { emit m_editor->stateChanged(); })
                                                                         : current(continuePublishing);
}

void VolumeConfigurationEditor::setEditingAllowed(const bool allowed) {
  if (m_editingAllowed == allowed) return;
  auto update = beginUpdate();
  m_editingAllowed = allowed;
  updateEditingAvailability();
  update.resume();
  (void)update.publishAvailability();
}

void VolumeConfigurationEditor::updateEditingAvailability() {
  const bool allowed = m_editingAllowed && m_snapshot.editable && m_snapshot.configurationRestriction == core::ConfigurationWriteRestriction::None;
  m_status->setControlsEnabled(allowed);
  m_files->setReadOnly(m_state->identityOnly || !allowed || !core::supportsFileOverlayOperations(m_volume.support));
  if (m_registry && m_ownsRegistry) m_registry->setReadOnly(!allowed);
}

void VolumeConfigurationEditor::applySnapshot(const core::UwfSnapshot& snapshot, const std::optional<bool> hormEnabled,
                                              const std::optional<std::uint32_t> currentOverlayFlags) {
  auto update = beginUpdate();
  const bool supported = m_volume.driveLetter.empty() || core::supportsVolumeProtection(m_volume.support);
  {
    // The facade and all child editors form one observation. Complete their
    // state before notifying any receiver, which may replace or release them.
    m_snapshot.available = snapshot.uwfAvailable;
    m_snapshot.unavailableReason = QString::fromStdString(snapshot.unavailableReason);
    m_snapshot.elevated = snapshot.elevated;
    const auto observedHorm = hormEnabled ? hormEnabled : snapshot.runtime.hormEnabled;
    const auto observedFlags = currentOverlayFlags ? currentOverlayFlags : snapshot.runtime.currentOverlayFlags;
    m_snapshot.hormEnabled = observedHorm;
    m_snapshot.currentOverlayFlags = observedFlags;
    m_snapshot.romDisposition = snapshot.runtime.romDisposition;
    m_snapshot.fullCommit = snapshot.runtime.fullCommit;
    auto mode = core::configurationModeState(snapshot);
    mode.runtime.hormEnabled = observedHorm;
    mode.runtime.overlayFlags = observedFlags;
    m_snapshot.configurationRestriction = core::configurationWriteRestriction(mode);
    // Protection may target a stable volume identity. Host-path operations still
    // require a drive letter and retain their independent read-only admission.
    m_snapshot.editable = m_state->supported && snapshot.uwfAvailable && snapshot.elevated && hormAllowsChanges(observedHorm);
    m_snapshot.filterEnabled = snapshot.current.filter.enabled;
    const auto* current = supported ? core::findVolumeConfiguration(snapshot.current, m_volume) : nullptr;
    const auto* next = supported ? core::findVolumeConfiguration(snapshot.next, m_volume) : nullptr;
    // Cached records remain the editors' baseline, but an unavailable provider
    // cannot establish the volume's currently observed protection state.
    m_snapshot.current = snapshot.uwfAvailable && current ? std::optional<core::VolumeRecord>(*current) : std::nullopt;
    m_snapshot.currentProtected = current && current->isProtected;
    m_snapshot.next = snapshot.uwfAvailable && next ? std::optional<core::VolumeRecord>(*next) : std::nullopt;
    m_state->draft.protectionTarget = core::volumeConfigurationTarget(next, m_volume);
    if (supported) {
      m_status->reset(current, next, snapshot.uwfAvailable, &m_volume);
      const auto* owned = current ? current : next;
      m_files->setFileVolumeIdentity(owned ? owned->volumeName : m_volume.volumeName);
      m_files->setProviderBaseline(fileExclusions(snapshot.current, current), fileExclusions(snapshot.next, next));
    } else {
      m_status->setUnsupported();
      m_files->setReadOnly(true);
    }
    if (m_registry && m_ownsRegistry) {
      m_registry->setBaseline(strings(snapshot.current.registryExclusions), strings(snapshot.next.registryExclusions));
      m_registry->setPersistBaseline(snapshot.current.persistDomainSecretKey, snapshot.next.persistDomainSecretKey, snapshot.current.persistTSCAL,
                                     snapshot.next.persistTSCAL);
    }
    if (supported) {
      const auto* displayed = current ? current : next;
      if (displayed && !displayed->volumeName.empty() && m_volume.volumeName.empty()) m_volume.volumeName = displayed->volumeName;
    }
    m_state->conflicts.setVolumeIdentity(driveLetter(), m_volume.volumeName);
    updateFileTarget(m_state, *m_files);
    updateEditingAvailability();
  }
  update.resume();
  (void)update.publishSnapshot();
}

bool VolumeConfigurationEditor::importProtect(const bool value, const QString& target) {
  if (!m_state->supported) return false;
  std::optional<std::string> creation;
  if (!target.isEmpty()) {
    std::string selector;
    if (const auto id = compat::parseVolumeId(target.toStdString())) {
      const auto expected = m_snapshot.next ? compat::parseVolumeId(m_snapshot.next->volumeName) : compat::parseVolumeId(m_volume.volumeName);
      if (!expected || *id != *expected) return false;
      selector = *id;
    } else {
      selector = drive::normalize(target.toStdString());
      if (selector.empty() || selector != drive::normalize(m_volume.driveLetter)) return false;
    }
    if (value && !m_state->draft.hasPlannedConfiguration()) creation = std::move(selector);
  }
  return m_status->editProtection(value, std::move(creation));
}

void VolumeConfigurationEditor::setBulkProtection(const std::optional<bool> value, const bool resetOverrides) {
  const auto state = m_state;
  const auto files = m_files;
  const auto status = m_status;
  status->setBulkProtection(value, resetOverrides);
  // Session resets block child signals while committing the shared draft.
  updateFileTarget(state, *files);
}

void VolumeConfigurationEditor::restoreChanges(const std::optional<bool> protection, const std::optional<core::VolumeBinding> binding,
                                               const std::vector<std::string>& added, const std::vector<std::string>& removed) {
  auto update = beginUpdate();
  if ((protection || binding || !added.empty() || !removed.empty()) && compat::parseVolumeId(m_volume.volumeName)) m_state->draft.hasTarget = true;
  if (protection) m_state->draft.protectedVolume = *protection;
  if (binding) m_state->draft.binding = *binding;
  const auto displayPath = [&](const std::string& path) {
    const auto text = QString::fromStdString(path);
    const auto address = paths::resolveVolumeFilePath(text);
    return address && !m_volume.driveLetter.empty() && compat::parseVolumeId(m_volume.volumeName) == std::optional{address->volume}
               ? driveLetter() + address->relativePath
               : text;
  };
  for (const auto& path : added) (void)m_state->files.add(displayPath(path));
  for (const auto& path : removed) (void)m_state->files.remove(displayPath(path));
  updateFileTarget(m_state, *m_files);
  update.resume();
  (void)update.publishSnapshot();
}

app::ExclusionImportOutcome VolumeConfigurationEditor::importAddFileExclusion(const QString& path) {
  if (m_state->identityOnly) return app::ExclusionImportOutcome::RejectedReadOnly;
  if (!core::supportsFileOverlayOperations(m_volume.support)) return app::ExclusionImportOutcome::RejectedNotOnVolume;
  if (!editingAllowed()) return app::ExclusionImportOutcome::RejectedReadOnly;
  return m_files->importAdd(path);
}
app::ExclusionImportOutcome VolumeConfigurationEditor::importRemoveFileExclusion(const QString& path) {
  if (m_state->identityOnly) return app::ExclusionImportOutcome::RejectedReadOnly;
  if (!core::supportsFileOverlayOperations(m_volume.support)) return app::ExclusionImportOutcome::RejectedNotOnVolume;
  if (!editingAllowed()) return app::ExclusionImportOutcome::RejectedReadOnly;
  return m_files->importRemove(path);
}

}  // namespace uwf::app

#include "moc_VolumeConfigurationEditor.cpp"
