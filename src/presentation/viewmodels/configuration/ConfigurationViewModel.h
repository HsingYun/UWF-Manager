// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <cstdint>
#include <functional>
#include <optional>

#include "application/configuration/editing/ConfigurationEditSession.h"
#include "presentation/viewmodels/configuration/GlobalConfigurationViewModel.h"
#include "presentation/viewmodels/configuration/OverlayAdvancedViewModel.h"
#include "presentation/viewmodels/configuration/VolumeConfigurationViewModel.h"

namespace uwf::presentation {

class ConfigurationViewModel final : public QObject {
  Q_OBJECT
 public:
  explicit ConfigurationViewModel(uint32_t totalRamMb, std::function<bool()> stagingData = {}, QString systemDrive = {}, app::DriveResolver resolveDrive = {},
                                  std::shared_ptr<const app::FileBrowser> files = {}, QObject* parent = nullptr,
                                  std::shared_ptr<app::OverlayMaintenanceEditor> maintenance = {});
  [[nodiscard]] const std::shared_ptr<GlobalConfigurationViewModel>& global() const { return m_global; }
  [[nodiscard]] const std::shared_ptr<OverlayAdvancedViewModel>& overlayPolicy() const { return m_overlayPolicy; }
  [[nodiscard]] const std::shared_ptr<app::OverlayMaintenanceEditor>& maintenanceEditor() const { return m_edit.maintenanceEditor(); }
  void reconcileApplied(const app::ConfigurationChanges& requested, const app::ConfigurationApplyResult& result) { m_edit.reconcileApplied(requested, result); }
  std::shared_ptr<VolumeConfigurationViewModel> addVolume(const core::VolumeInfo& volume, bool hostsRegistry);
  void clearVolumes();
  void resetSnapshot(const core::UwfSnapshot& snapshot, const app::OverlayPolicySnapshot& policy = {}, std::optional<bool> hormEnabled = {});
  void discardChanges(const core::UwfSnapshot& snapshot, const app::OverlayPolicySnapshot& policy = {}, std::optional<bool> hormEnabled = {});
  void setEditingAllowed(bool allowed) { m_edit.setEditingAllowed(allowed); }
  void setCommitContext(bool allowed, QString restriction = {});
  void refreshStagingConstraints();
  void setStagedEntries(QList<app::FileStagingEntry> entries);
  [[nodiscard]] std::shared_ptr<VolumeConfigurationViewModel> volume(const QString& driveLetter) const;
  [[nodiscard]] core::PendingChanges pendingChanges() const;
  [[nodiscard]] app::ConfigurationChanges pendingConfiguration() const { return m_edit.pendingConfiguration(); }
  [[nodiscard]] QList<app::ImportResult> importCommands(const QList<app::UwfmgrCommand>& commands);

 signals:
  void pendingChanged();

 private:
  void updateDraft(std::function<void()> work);
  app::ConfigurationEditSession m_edit;
  std::shared_ptr<GlobalConfigurationViewModel> m_global;
  std::shared_ptr<OverlayAdvancedViewModel> m_overlayPolicy;
  std::vector<std::shared_ptr<VolumeConfigurationViewModel>> m_volumes;
  QList<app::FileStagingEntry> m_stagedEntries;
  bool m_commitsAllowed = true;
  QString m_commitRestriction;
  std::uint64_t m_volumeGeneration = 0;
};

}  // namespace uwf::presentation
