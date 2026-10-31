// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <memory>
#include <vector>

#include "ConfigurationDraft.h"
#include "ConfigurationImporter.h"
#include "ConfigurationState.h"

namespace uwf::app {

class GlobalConfigurationEditor;
class ExclusionEditor;
class VolumeConfigurationEditor;

// Shared state outlives replaceable views. Pending is derived, never stored.
class ConfigurationEditSession final {
 public:
  // Without a resolver, imports accept literal drive paths. The platform
  // composition supplies volume-GUID resolution when that capability exists.
  explicit ConfigurationEditSession(uint32_t totalRamMb = 0, QString systemDrive = {}, DriveResolver resolveDrive = {});
  [[nodiscard]] const std::shared_ptr<GlobalConfigurationEditor>& globalEditor() const { return m_globalEditor; }
  [[nodiscard]] const std::shared_ptr<ExclusionEditor>& registryEditor() const { return m_registryEditor; }
  void resetRegistry(const core::UwfSnapshot& snapshot);
  std::shared_ptr<VolumeConfigurationEditor> editVolume(const core::VolumeInfo& volume, bool hostsRegistry);
  [[nodiscard]] QList<ImportResult> importCommands(const QList<app::UwfmgrCommand>& commands);
  [[nodiscard]] const std::shared_ptr<GlobalConfigurationState>& global() const { return m_global; }
  [[nodiscard]] const std::vector<std::shared_ptr<VolumeConfigurationState>>& volumes() const { return m_volumes; }
  std::shared_ptr<VolumeConfigurationState> addVolume(const core::VolumeInfo& volume);
  void clearVolumes();
  [[nodiscard]] core::PendingChanges pendingChanges() const;

 private:
  std::shared_ptr<GlobalConfigurationState> m_global;
  std::vector<std::shared_ptr<VolumeConfigurationState>> m_volumes;
  std::shared_ptr<GlobalConfigurationEditor> m_globalEditor;
  std::shared_ptr<ExclusionEditor> m_registryEditor;
  std::vector<std::shared_ptr<VolumeConfigurationEditor>> m_volumeEditors;
  QString m_systemDrive;
  DriveResolver m_resolveDrive;
};

}  // namespace uwf::app
