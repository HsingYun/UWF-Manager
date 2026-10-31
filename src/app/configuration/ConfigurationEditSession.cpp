// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "ConfigurationEditSession.h"

#include <algorithm>
#include <utility>

#include "GlobalConfigurationEditor.h"
#include "VolumeConfigurationEditor.h"
#include "shared/DriveLetter.h"

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
}  // namespace

ConfigurationEditSession::ConfigurationEditSession(uint32_t totalRamMb, QString systemDrive, DriveResolver resolveDrive)
    : m_global(std::make_shared<GlobalConfigurationState>()),
      m_globalEditor(std::make_shared<GlobalConfigurationEditor>(m_global, totalRamMb)),
      m_registryEditor(std::make_shared<ExclusionEditor>(ExclusionEditor::Kind::Registry, std::shared_ptr<ExclusionDraft>(m_global, &m_global->registry))),
      m_systemDrive(std::move(systemDrive)),
      m_resolveDrive(resolveDrive ? std::move(resolveDrive) : DriveResolver{[](const std::string& path) { return drive::split(path).letter; }}) {}

void ConfigurationEditSession::resetRegistry(const core::UwfSnapshot& snapshot) {
  const auto strings = [](const std::vector<std::string>& paths) {
    QStringList result;
    for (const auto& path : paths) result.append(QString::fromStdString(path));
    return result;
  };
  m_registryEditor->setBaseline(strings(snapshot.current.registryExclusions), strings(snapshot.next.registryExclusions));
  m_registryEditor->setPersistBaseline(snapshot.current.persistDomainSecretKey, snapshot.next.persistDomainSecretKey, snapshot.current.persistTSCAL,
                                       snapshot.next.persistTSCAL);
  m_registryEditor->setReadOnly(!snapshot.uwfAvailable || !snapshot.elevated || !snapshot.current.filter.enabled);
  m_registryEditor->refreshFileStagingConstraints();
}
std::shared_ptr<VolumeConfigurationEditor> ConfigurationEditSession::editVolume(const core::VolumeInfo& volume, bool hostsRegistry) {
  auto editor = std::make_shared<VolumeConfigurationEditor>(volume, hostsRegistry, this);
  editor->files()->setSystemDriveLetter(m_systemDrive);
  m_volumeEditors.push_back(editor);
  return editor;
}
QList<ImportResult> ConfigurationEditSession::importCommands(const QList<app::UwfmgrCommand>& commands) {
  VolumeConfigurationEditors editors;
  editors.reserve(m_volumeEditors.size());
  for (const auto& editor : m_volumeEditors) editors.emplace_back(editor);
  const auto global = m_globalEditor;
  const auto registry = m_registryEditor;
  const auto resolveDrive = m_resolveDrive;
  return applyImportCommands(commands, global.get(), editors, registry.get(), resolveDrive);
}

std::shared_ptr<VolumeConfigurationState> ConfigurationEditSession::addVolume(const core::VolumeInfo& volume) {
  auto state = std::make_shared<VolumeConfigurationState>();
  state->driveLetter = volume.driveLetter;
  state->supported = core::supportsVolumeProtection(volume.support);
  m_volumes.push_back(state);
  return state;
}

void ConfigurationEditSession::clearVolumes() {
  m_volumeEditors.clear();
  m_volumes.clear();
}

core::PendingChanges ConfigurationEditSession::pendingChanges() const {
  ConfigurationDraft changes;
  if (m_global->available) {
    changes.filterEnabled = m_global->draft.pendingFilterEnabled();
    changes.overlay = m_global->draft.pendingOverlay();
    changes.registry = {sortedPaths(m_global->registry.added), sortedPaths(m_global->registry.removed), m_global->registry.domainSecretKey.pendingNext,
                        m_global->registry.tscal.pendingNext};
  }
  for (const auto& volume : m_volumes) {
    if (!volume->supported) continue;
    VolumeConfigurationChanges entry;
    entry.driveLetter = volume->driveLetter;
    entry.supported = true;
    entry.protectedVolume = volume->draft.pendingProtection();
    entry.bindByVolumeName = volume->draft.pendingBinding();
    entry.fileAdded = sortedPaths(volume->files.added);
    entry.fileRemoved = sortedPaths(volume->files.removed);
    changes.volumes.push_back(std::move(entry));
  }
  return changes.pendingChanges();
}

}  // namespace uwf::app
