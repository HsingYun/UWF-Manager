// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>

#include "application/configuration/contracts/ConfigurationEditing.h"
#include "application/configuration/editing/EditorUpdate.h"
#include "application/configuration/exclusions/ExclusionEditor.h"
#include "application/configuration/volumes/VolumeStatusEditor.h"
#include "domain/uwf/policies/ConfigurationModePolicy.h"

namespace uwf::app {
class ConfigurationEditSession;

class VolumeConfigurationEditor final : public QObject, public app::VolumeConfigurationEditing {
  Q_OBJECT
 public:
  struct SnapshotState {
    bool available = false;
    QString unavailableReason;
    bool editable = false;
    bool elevated = false;
    std::optional<bool> hormEnabled;
    std::optional<std::uint32_t> currentOverlayFlags;
    std::optional<std::uint32_t> romDisposition;
    std::optional<bool> fullCommit;
    bool filterEnabled = false;
    bool currentProtected = false;
    core::ConfigurationWriteRestriction configurationRestriction = core::ConfigurationWriteRestriction::None;
    std::optional<core::VolumeRecord> current, next;
  };

  VolumeConfigurationEditor(core::VolumeInfo volume, bool hostsRegistry, app::ConfigurationEditSession* session = nullptr,
                            std::shared_ptr<const FileBrowser> files = {});
  [[nodiscard]] const core::VolumeInfo& volume() const { return m_volume; }
  [[nodiscard]] static QString volumeIdentity(const core::VolumeInfo& volume);
  [[nodiscard]] QString volumeIdentity() const override { return volumeIdentity(m_volume); }
  [[nodiscard]] QString configurationVolumeId() const override;
  [[nodiscard]] QString driveLetter() const override { return QString::fromStdString(m_volume.driveLetter); }
  [[nodiscard]] bool editingAllowed() const override { return m_status->editingAllowed(); }
  // Manual data maintenance has a target-specific capability check. A locked
  // ordinary configuration hive must not act as a second, broader commit gate.
  [[nodiscard]] bool maintenanceEditingAllowed() const { return m_editingAllowed && m_snapshot.editable; }
  [[nodiscard]] bool protectionEditingAllowed() const override { return m_status->controlsEnabled(); }
  [[nodiscard]] bool supportsProtection() const { return m_state->supported; }
  [[nodiscard]] const std::shared_ptr<VolumeStatusEditor>& status() const { return m_status; }
  [[nodiscard]] const std::shared_ptr<ExclusionEditor>& files() const { return m_files; }
  [[nodiscard]] const std::shared_ptr<ExclusionEditor>& registry() const { return m_registry; }
  [[nodiscard]] app::FileStagingConflictPolicy& fileConflicts() const { return m_state->conflicts; }
  [[nodiscard]] const SnapshotState& snapshotState() const { return m_snapshot; }
  class Update : public EditorUpdate<VolumeConfigurationEditor> {
   public:
    explicit Update(VolumeConfigurationEditor& editor);
    Update(Update&& other) noexcept = default;
    Update(const Update&) = delete;
    Update& operator=(const Update&) = delete;
    [[nodiscard]] bool publishSnapshot(const std::function<bool()>& continuePublishing = {});
    [[nodiscard]] bool publishAvailability(const std::function<bool()>& continuePublishing = {});

   private:
    std::shared_ptr<VolumeStatusEditor> m_status;
    std::shared_ptr<ExclusionEditor> m_files;
    std::shared_ptr<ExclusionEditor> m_registry;
    bool m_supported;
    bool m_publishStatus;
    bool m_statusControlsBefore;
    bool m_statusEditingBefore;
    bool m_filesReadOnlyBefore;
    bool m_filesCanAddBefore;
    bool m_registryReadOnlyBefore;
    bool m_editingAllowedBefore;
  };
  // Resume all participants before publishing any part of a shared update.
  [[nodiscard]] Update beginUpdate() { return Update(*this); }
  void setEditingAllowed(bool allowed);
  void applySnapshot(const core::UwfSnapshot& snapshot, std::optional<bool> hormEnabled = {}, std::optional<std::uint32_t> currentOverlayFlags = {});
  void setBulkProtection(std::optional<bool> value, bool resetOverrides = false);
  void restoreChanges(std::optional<bool> protection, std::optional<core::VolumeBinding> binding, const std::vector<std::string>& added,
                      const std::vector<std::string>& removed);
  bool importProtect(bool value, const QString& target = {}) override;
  app::ExclusionImportOutcome importAddFileExclusion(const QString& path) override;
  app::ExclusionImportOutcome importRemoveFileExclusion(const QString& path) override;

 signals:
  void stateChanged();
  void pendingChanged();

 private:
  void updateEditingAvailability();
  core::VolumeInfo m_volume;
  std::shared_ptr<app::VolumeConfigurationState> m_state;
  std::shared_ptr<VolumeStatusEditor> m_status;
  std::shared_ptr<ExclusionEditor> m_files;
  std::shared_ptr<ExclusionEditor> m_registry;
  SnapshotState m_snapshot;
  bool m_editingAllowed = true;
  bool m_ownsRegistry = false;
};

}  // namespace uwf::app
