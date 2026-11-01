// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "presentation/viewmodels/configuration/ConfigurationViewModel.h"

#include <QPointer>
#include <utility>

namespace uwf::presentation {

ConfigurationViewModel::ConfigurationViewModel(const uint32_t totalRamMb, std::function<bool()> stagingData, QString systemDrive,
                                               app::DriveResolver resolveDrive, std::shared_ptr<const app::FileBrowser> files, QObject* parent,
                                               std::shared_ptr<app::OverlayMaintenanceEditor> maintenance)
    : QObject(parent),
      m_edit(totalRamMb, std::move(systemDrive), std::move(resolveDrive), std::move(files), std::move(maintenance)),
      m_global(std::make_shared<GlobalConfigurationViewModel>(m_edit.globalEditor())),
      m_overlayPolicy(std::make_shared<OverlayAdvancedViewModel>(m_edit.overlayPolicyEditor())) {
  m_edit.registryEditor()->setFileStagingDataProbe(std::move(stagingData));
  connect(&m_edit, &app::ConfigurationEditSession::pendingChanged, this, &ConfigurationViewModel::pendingChanged);
}

std::shared_ptr<VolumeConfigurationViewModel> ConfigurationViewModel::addVolume(const core::VolumeInfo& volume, const bool hostsRegistry) {
  auto model = std::make_shared<VolumeConfigurationViewModel>(m_edit.editVolume(volume, hostsRegistry));
  model->fileConflicts().setStagedEntries(m_stagedEntries);
  model->setCommitContext(m_commitsAllowed, m_commitRestriction);
  m_volumes.push_back(model);
  ++m_volumeGeneration;
  return model;
}

void ConfigurationViewModel::clearVolumes() {
  ++m_volumeGeneration;
  const auto volumes = std::exchange(m_volumes, {});
  for (const auto& volume : volumes) disconnect(volume.get(), nullptr, this, nullptr);
  m_edit.clearVolumes();
}
void ConfigurationViewModel::resetSnapshot(const core::UwfSnapshot& snapshot, const app::OverlayPolicySnapshot& policy, const std::optional<bool> hormEnabled) {
  updateDraft([self = QPointer(this), edit = &m_edit, global = m_global, snapshot, policy, hormEnabled] {
    edit->resetSnapshot(snapshot, policy, hormEnabled);
    if (self && snapshot.uwfAvailable) global->setCurrentSession(snapshot.current, snapshot.runtime);
  });
}

void ConfigurationViewModel::discardChanges(const core::UwfSnapshot& snapshot, const app::OverlayPolicySnapshot& policy,
                                            const std::optional<bool> hormEnabled) {
  updateDraft([self = QPointer(this), edit = &m_edit, global = m_global, snapshot, policy, hormEnabled] {
    edit->discardChanges(snapshot, policy, hormEnabled);
    if (self && snapshot.uwfAvailable) global->setCurrentSession(snapshot.current, snapshot.runtime);
  });
}

void ConfigurationViewModel::setCommitContext(const bool allowed, QString restriction) {
  m_commitsAllowed = allowed;
  m_commitRestriction = std::move(restriction);
  const auto volumes = m_volumes;
  const auto generation = m_volumeGeneration;
  const auto reason = m_commitRestriction;
  const QPointer self(this);
  for (const auto& volume : volumes) {
    if (!self || generation != m_volumeGeneration) return;
    volume->setCommitContext(allowed, reason);
  }
}

std::shared_ptr<VolumeConfigurationViewModel> ConfigurationViewModel::volume(const QString& driveLetter) const {
  if (driveLetter.isEmpty()) return {};
  for (const auto& model : m_volumes)
    if (model->volumeIdentity().compare(driveLetter, Qt::CaseInsensitive) == 0 ||
        (!model->driveLetter().isEmpty() && model->driveLetter().compare(driveLetter, Qt::CaseInsensitive) == 0))
      return model;
  return {};
}

void ConfigurationViewModel::refreshStagingConstraints() {
  updateDraft([registry = m_edit.registryEditor()] { registry->refreshFileStagingConstraints(); });
}
void ConfigurationViewModel::setStagedEntries(QList<app::FileStagingEntry> entries) {
  m_stagedEntries = std::move(entries);
  const auto volumes = m_volumes;
  const auto generation = m_volumeGeneration;
  const QPointer self(this);
  for (const auto& volume : volumes) {
    if (!self || generation != m_volumeGeneration) return;
    volume->fileConflicts().setStagedEntries(m_stagedEntries);
  }
}

void ConfigurationViewModel::updateDraft(std::function<void()> work) {
  const QPointer self(this);
  const bool previouslyBlocked = blockSignals(true);
  try {
    work();
  } catch (...) {
    if (self) blockSignals(previouslyBlocked);
    throw;
  }
  if (!self) return;
  blockSignals(previouslyBlocked);
  emit pendingChanged();
}

core::PendingChanges ConfigurationViewModel::pendingChanges() const { return m_edit.pendingChanges(); }

QList<app::ImportResult> ConfigurationViewModel::importCommands(const QList<app::UwfmgrCommand>& commands) { return m_edit.importCommands(commands); }

}  // namespace uwf::presentation

#include "moc_ConfigurationViewModel.cpp"
