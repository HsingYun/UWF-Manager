// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <memory>
#include <optional>
#include <vector>

#include "application/configuration/contracts/ConfigurationChanges.h"
#include "application/configuration/editing/ConfigurationState.h"
#include "application/configuration/editing/OverlayMaintenanceEditor.h"
#include "application/configuration/importing/ConfigurationImporter.h"
#include "application/configuration/overlay/OverlayPolicyEditor.h"

namespace uwf::app {

class GlobalConfigurationEditor;
class ExclusionEditor;
class VolumeConfigurationEditor;
class FileBrowser;
struct ConfigurationApplyResult;

// Shared drafts outlive replaceable views; pending changes are derived from those drafts.
class ConfigurationEditSession final : public QObject {
  Q_OBJECT
 public:
  // Without a resolver, imports accept literal drive paths. The platform
  // composition supplies volume-GUID resolution when that capability exists.
  explicit ConfigurationEditSession(uint32_t totalRamMb = 0, QString systemDrive = {}, DriveResolver resolveDrive = {},
                                    std::shared_ptr<const FileBrowser> files = {}, std::shared_ptr<OverlayMaintenanceEditor> maintenance = {});
  ~ConfigurationEditSession() override;
  ConfigurationEditSession(const ConfigurationEditSession&) = delete;
  ConfigurationEditSession& operator=(const ConfigurationEditSession&) = delete;
  [[nodiscard]] const std::shared_ptr<GlobalConfigurationEditor>& globalEditor() const;
  [[nodiscard]] const std::shared_ptr<ExclusionEditor>& registryEditor() const;
  // Window-wide admission is shared by every editor, including editors added
  // or reset while a refresh, execution or reconciliation is in progress.
  void setEditingAllowed(bool allowed);
  void resetSnapshot(const core::UwfSnapshot& snapshot, const OverlayPolicySnapshot& policy = {}, std::optional<bool> hormEnabled = {});
  void discardChanges(const core::UwfSnapshot& snapshot, const OverlayPolicySnapshot& policy = {}, std::optional<bool> hormEnabled = {});
  std::shared_ptr<VolumeConfigurationEditor> editVolume(const core::VolumeInfo& volume, bool hostsRegistry);
  [[nodiscard]] QList<ImportResult> importCommands(const QList<app::UwfmgrCommand>& commands);
  [[nodiscard]] const std::shared_ptr<GlobalConfigurationState>& global() const;
  [[nodiscard]] const std::vector<std::shared_ptr<VolumeConfigurationState>>& volumes() const;
  std::shared_ptr<VolumeConfigurationState> addVolume(const core::VolumeInfo& volume);
  void clearVolumes();
  [[nodiscard]] core::PendingChanges pendingChanges() const;
  [[nodiscard]] ConfigurationChanges pendingConfiguration() const;
  [[nodiscard]] const std::shared_ptr<OverlayPolicyEditor>& overlayPolicyEditor() const;
  [[nodiscard]] const std::shared_ptr<OverlayMaintenanceEditor>& maintenanceEditor() const;
  void reconcileApplied(const ConfigurationChanges& requested, const ConfigurationApplyResult& result);

 signals:
  void pendingChanged();

 private:
  struct State;
  struct Reset;
  struct Publication;
  static void updateEditingAvailability(std::shared_ptr<State> state, bool notify = true);
  static void enqueueReset(std::shared_ptr<State> state, Reset reset);
  static void updateBulkProtection(std::shared_ptr<State> state, bool resetOverrides = false);
  std::shared_ptr<State> m_state;
};

}  // namespace uwf::app
