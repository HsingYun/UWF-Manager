// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "ConfigurationViewModel.h"

#include <QSignalBlocker>

namespace uwf::presentation {

ConfigurationViewModel::ConfigurationViewModel(const uint32_t totalRamMb, std::function<bool()> stagingData, QString systemDrive,
                                               app::DriveResolver resolveDrive, QObject* parent)
    : QObject(parent), m_edit(totalRamMb, std::move(systemDrive), std::move(resolveDrive)),
      m_global(std::make_shared<GlobalConfigurationViewModel>(m_edit.globalEditor())) {
  m_edit.registryEditor()->setFileStagingDataProbe(std::move(stagingData));
  connect(m_global.get(), &GlobalConfigurationViewModel::pendingChanged, this, &ConfigurationViewModel::pendingChanged);
}

std::shared_ptr<VolumeConfigurationViewModel> ConfigurationViewModel::addVolume(const core::VolumeInfo& volume, const bool hostsRegistry) {
  auto model = std::make_shared<VolumeConfigurationViewModel>(m_edit.editVolume(volume, hostsRegistry));
  connect(model.get(), &VolumeConfigurationViewModel::pendingChanged, this, &ConfigurationViewModel::pendingChanged);
  m_volumes.push_back(model);
  return model;
}

void ConfigurationViewModel::clearVolumes() {
  for (const auto& volume : m_volumes) disconnect(volume.get(), nullptr, this, nullptr);
  m_volumes.clear();
  m_edit.clearVolumes();
}
void ConfigurationViewModel::resetSnapshot(const core::UwfSnapshot& snapshot) {
  {
    const QSignalBlocker resetting(this);
    m_global->applySnapshot(snapshot);
    for (const auto& volume : m_volumes) volume->applySnapshot(snapshot);
    m_edit.resetRegistry(snapshot);
  }
  emit pendingChanged();
}

std::shared_ptr<VolumeConfigurationViewModel> ConfigurationViewModel::volume(const QString& driveLetter) const {
  for (const auto& model : m_volumes)
    if (model->driveLetter().compare(driveLetter, Qt::CaseInsensitive) == 0) return model;
  return {};
}

void ConfigurationViewModel::refreshStagingConstraints() {
  {
    const QSignalBlocker refreshing(this);
    m_edit.registryEditor()->refreshFileStagingConstraints();
  }
  emit pendingChanged();
}

core::PendingChanges ConfigurationViewModel::pendingChanges() const { return m_edit.pendingChanges(); }

QList<app::ImportResult> ConfigurationViewModel::importCommands(const QList<app::UwfmgrCommand>& commands) { return m_edit.importCommands(commands); }

}  // namespace uwf::presentation

#include "moc_ConfigurationViewModel.cpp"
