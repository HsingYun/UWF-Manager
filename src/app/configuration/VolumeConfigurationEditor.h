// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include "ExclusionEditor.h"
#include "VolumeStatusEditor.h"
#include "app/configuration/ConfigurationEditing.h"

namespace uwf::app {
class ConfigurationEditSession;

class VolumeConfigurationEditor final : public QObject, public app::VolumeConfigurationEditing {
  Q_OBJECT
 public:
  struct SnapshotState {
    bool available = false;
    QString unavailableReason;
    bool editable = false;
    bool filterEnabled = false;
    bool currentProtected = false;
    std::optional<core::VolumeRecord> current;
  };

  VolumeConfigurationEditor(core::VolumeInfo volume, bool hostsRegistry, app::ConfigurationEditSession* session = nullptr);
  [[nodiscard]] const core::VolumeInfo& volume() const { return m_volume; }
  [[nodiscard]] QString driveLetter() const override { return QString::fromStdString(m_volume.driveLetter); }
  [[nodiscard]] const std::shared_ptr<VolumeStatusEditor>& status() const { return m_status; }
  [[nodiscard]] const std::shared_ptr<ExclusionEditor>& files() const { return m_files; }
  [[nodiscard]] const std::shared_ptr<ExclusionEditor>& registry() const { return m_registry; }
  [[nodiscard]] app::FileStagingConflictPolicy& fileConflicts() const { return m_state->conflicts; }
  [[nodiscard]] const SnapshotState& snapshotState() const { return m_snapshot; }
  void applySnapshot(const core::UwfSnapshot& snapshot);
  bool importProtect(bool value) override;
  app::ExclusionImportOutcome importAddFileExclusion(const QString& path) override;
  app::ExclusionImportOutcome importRemoveFileExclusion(const QString& path) override;

 signals:
  void stateChanged();
  void pendingChanged();

 private:
  core::VolumeInfo m_volume;
  std::shared_ptr<app::VolumeConfigurationState> m_state;
  std::shared_ptr<VolumeStatusEditor> m_status;
  std::shared_ptr<ExclusionEditor> m_files;
  std::shared_ptr<ExclusionEditor> m_registry;
  SnapshotState m_snapshot;
};

}  // namespace uwf::app
