// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include "ExclusionListViewModel.h"
#include "VolumeStatusViewModel.h"
#include "app/configuration/VolumeConfigurationEditor.h"
namespace uwf::presentation {
[[nodiscard]] QString volumeSupportText(core::VolumeSupport support, const std::string& fileSystem);
class VolumeConfigurationViewModel final : public QObject {
  Q_OBJECT
 public:
  using SnapshotState = app::VolumeConfigurationEditor::SnapshotState;
  struct CommitAvailability {
    bool files = false;
    bool registry = false;
    bool overlay = false;
    QString overlayHint, commitHint;
  };
  VolumeConfigurationViewModel(core::VolumeInfo volume, bool hostsRegistry);
  explicit VolumeConfigurationViewModel(std::shared_ptr<app::VolumeConfigurationEditor> editor);
  [[nodiscard]] const std::shared_ptr<app::VolumeConfigurationEditor>& editor() const { return m_editor; }
  [[nodiscard]] const core::VolumeInfo& volume() const { return m_editor->volume(); }
  [[nodiscard]] QString driveLetter() const { return m_editor->driveLetter(); }
  [[nodiscard]] const std::shared_ptr<VolumeStatusViewModel>& status() const { return m_status; }
  [[nodiscard]] const std::shared_ptr<ExclusionListViewModel>& files() const { return m_files; }
  [[nodiscard]] const std::shared_ptr<ExclusionListViewModel>& registry() const { return m_registry; }
  [[nodiscard]] app::FileStagingConflictPolicy& fileConflicts() const { return m_editor->fileConflicts(); }
  [[nodiscard]] const SnapshotState& snapshotState() const { return m_editor->snapshotState(); }
  [[nodiscard]] bool supportsProtection() const { return core::supportsVolumeProtection(volume().support); }
  [[nodiscard]] bool supportsFileOperations() const { return core::supportsFileOverlayOperations(volume().support); }
  [[nodiscard]] QString capacityText() const;
  [[nodiscard]] CommitAvailability commitAvailability() const;
  void applySnapshot(const core::UwfSnapshot& snapshot) { m_editor->applySnapshot(snapshot); }
  bool importProtect(bool value) { return m_editor->importProtect(value); }
  app::ExclusionImportOutcome importAddFileExclusion(const QString& path) { return m_editor->importAddFileExclusion(path); }
  app::ExclusionImportOutcome importRemoveFileExclusion(const QString& path) { return m_editor->importRemoveFileExclusion(path); }
 signals:
  void stateChanged();
  void pendingChanged();

 private:
  std::shared_ptr<app::VolumeConfigurationEditor> m_editor;
  std::shared_ptr<VolumeStatusViewModel> m_status;
  std::shared_ptr<ExclusionListViewModel> m_files;
  std::shared_ptr<ExclusionListViewModel> m_registry;
};
}  // namespace uwf::presentation
