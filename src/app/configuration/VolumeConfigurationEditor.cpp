// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "VolumeConfigurationEditor.h"

#include "ConfigurationEditSession.h"

namespace uwf::app {
namespace {
const core::VolumeRecord* findVolume(const core::SessionSnapshot& snapshot, const QString& driveLetter) {
  for (const auto& volume : snapshot.volumes) {
    if (QString::fromStdString(volume.driveLetter).toUpper() == driveLetter.toUpper()) return &volume;
  }
  return nullptr;
}

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
}  // namespace

VolumeConfigurationEditor::VolumeConfigurationEditor(core::VolumeInfo volume, const bool hostsRegistry, app::ConfigurationEditSession* session)
    : m_volume(std::move(volume)) {
  m_state = session ? session->addVolume(m_volume) : std::make_shared<app::VolumeConfigurationState>();
  m_state->driveLetter = m_volume.driveLetter;
  m_state->supported = core::supportsVolumeProtection(m_volume.support);
  m_status = std::make_shared<VolumeStatusEditor>(m_state);
  m_files = std::make_shared<ExclusionEditor>(ExclusionEditor::Kind::File, std::shared_ptr<app::ExclusionDraft>(m_state, &m_state->files), &m_state->conflicts);
  m_files->setDriveLetter(driveLetter());
  if (!m_state->supported) m_status->setUnsupported();
  m_files->setReadOnly(!core::supportsFileOverlayOperations(m_volume.support));
  if (hostsRegistry) {
    m_registry = session ? session->registryEditor() : std::make_shared<ExclusionEditor>(ExclusionEditor::Kind::Registry);
    connect(m_registry.get(), &ExclusionEditor::pendingChanged, this, &VolumeConfigurationEditor::pendingChanged);
  }
  connect(m_files.get(), &ExclusionEditor::pendingChanged, this, &VolumeConfigurationEditor::pendingChanged);
  connect(m_status.get(), &VolumeStatusEditor::pendingChanged, this, &VolumeConfigurationEditor::pendingChanged);
}

void VolumeConfigurationEditor::applySnapshot(const core::UwfSnapshot& snapshot) {
  m_snapshot.available = snapshot.uwfAvailable;
  m_snapshot.unavailableReason = QString::fromStdString(snapshot.unavailableReason);
  m_snapshot.editable = snapshot.uwfAvailable && snapshot.elevated;
  m_snapshot.filterEnabled = snapshot.current.filter.enabled;
  const bool supported = core::supportsVolumeProtection(m_volume.support);
  const auto* current = supported ? findVolume(snapshot.current, driveLetter()) : nullptr;
  const auto* next = supported ? findVolume(snapshot.next, driveLetter()) : nullptr;
  // Cached records remain the editors' baseline, but an unavailable provider
  // cannot establish the volume's currently observed protection state.
  m_snapshot.current = snapshot.uwfAvailable && current ? std::optional<core::VolumeRecord>(*current) : std::nullopt;
  m_snapshot.currentProtected = current && current->isProtected;
  if (supported) {
    m_status->reset(current, next, snapshot.uwfAvailable);
    m_status->setControlsEnabled(m_snapshot.editable);
    m_files->setBaseline(fileExclusions(snapshot.current, current), fileExclusions(snapshot.next, next));
  } else {
    m_status->setUnsupported();
    m_files->setReadOnly(true);
  }
  if (m_registry) {
    m_registry->setBaseline(strings(snapshot.current.registryExclusions), strings(snapshot.next.registryExclusions));
    m_registry->setPersistBaseline(snapshot.current.persistDomainSecretKey, snapshot.next.persistDomainSecretKey, snapshot.current.persistTSCAL,
                                   snapshot.next.persistTSCAL);
    m_registry->setReadOnly(!m_snapshot.editable || !m_snapshot.filterEnabled);
  }
  if (supported) {
    m_files->setReadOnly(!m_snapshot.editable || !core::supportsFileOverlayOperations(m_volume.support));
    const auto* displayed = current ? current : next;
    if (displayed && !displayed->volumeName.empty() && m_volume.volumeName.empty()) m_volume.volumeName = displayed->volumeName;
  }
  emit stateChanged();
}

bool VolumeConfigurationEditor::importProtect(const bool value) { return core::supportsVolumeProtection(m_volume.support) && m_status->editProtection(value); }

app::ExclusionImportOutcome VolumeConfigurationEditor::importAddFileExclusion(const QString& path) {
  return core::supportsFileOverlayOperations(m_volume.support) ? m_files->importAdd(path) : app::ExclusionImportOutcome::RejectedNotOnVolume;
}
app::ExclusionImportOutcome VolumeConfigurationEditor::importRemoveFileExclusion(const QString& path) {
  return core::supportsFileOverlayOperations(m_volume.support) ? m_files->importRemove(path) : app::ExclusionImportOutcome::RejectedNotOnVolume;
}

}  // namespace uwf::app

#include "moc_VolumeConfigurationEditor.cpp"
