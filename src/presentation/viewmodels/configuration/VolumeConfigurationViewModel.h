// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <cstdint>

#include "application/configuration/volumes/VolumeConfigurationEditor.h"
#include "presentation/viewmodels/configuration/ExclusionListViewModel.h"
#include "presentation/viewmodels/configuration/OverlayMaintenanceViewModel.h"
#include "presentation/viewmodels/configuration/VolumeStatusViewModel.h"

namespace uwf::presentation {
[[nodiscard]] QString volumeSupportText(core::VolumeSupport support, const std::string& fileSystem);
[[nodiscard]] QString volumeRegistrationText(const core::VolumeInfo& volume);
class VolumeConfigurationViewModel final : public QObject {
  Q_OBJECT
 public:
  using SnapshotState = app::VolumeConfigurationEditor::SnapshotState;
  struct CommitAvailability {
    bool files = false;
    bool registry = false;
    bool overlay = false;
    QString overlayHint, commitHint;
    OverlayMaintenanceViewModel::FullCommitState fullCommit;
    bool removeFullCommitDraft = false;
    bool menu = false;
  };
  VolumeConfigurationViewModel(core::VolumeInfo volume, bool hostsRegistry);
  explicit VolumeConfigurationViewModel(std::shared_ptr<app::VolumeConfigurationEditor> editor);
  [[nodiscard]] const std::shared_ptr<app::VolumeConfigurationEditor>& editor() const { return m_editor; }
  [[nodiscard]] const core::VolumeInfo& volume() const { return m_editor->volume(); }
  [[nodiscard]] QString driveLetter() const { return m_editor->driveLetter(); }
  [[nodiscard]] QString volumeIdentity() const { return m_editor->volumeIdentity(); }
  [[nodiscard]] const std::shared_ptr<VolumeStatusViewModel>& status() const { return m_status; }
  [[nodiscard]] const std::shared_ptr<ExclusionListViewModel>& files() const { return m_files; }
  [[nodiscard]] const std::shared_ptr<ExclusionListViewModel>& registry() const { return m_registry; }
  [[nodiscard]] app::FileStagingConflictPolicy& fileConflicts() const { return m_editor->fileConflicts(); }
  [[nodiscard]] const SnapshotState& snapshotState() const { return m_editor->snapshotState(); }
  [[nodiscard]] bool supportsProtection() const { return m_editor->supportsProtection(); }
  [[nodiscard]] bool supportsFileOperations() const { return !driveLetter().isEmpty() && core::supportsFileOverlayOperations(volume().support); }
  [[nodiscard]] QString capacityText() const;
  [[nodiscard]] CommitAvailability commitAvailability(const OverlayMaintenanceViewModel* maintenance = nullptr, bool diagnostic = false) const;
  void setCommitContext(bool allowed, QString restriction = {});
  [[nodiscard]] bool currentReadOnlyMediaActive() const;
  [[nodiscard]] bool currentHormActive() const;
  [[nodiscard]] QString runtimeNotice() const;
  [[nodiscard]] OverlayMaintenanceViewModel::RuntimeNotice runtimePresentation(const OverlayMaintenanceViewModel* maintenance = nullptr) const;
  void applySnapshot(const core::UwfSnapshot& snapshot, const std::optional<bool> hormEnabled = {},
                     const std::optional<std::uint32_t> currentOverlayFlags = {}) {
    m_editor->applySnapshot(snapshot, hormEnabled, currentOverlayFlags);
  }
  bool importProtect(bool value) { return m_editor->importProtect(value); }
  app::ExclusionImportOutcome importAddFileExclusion(const QString& path) { return m_editor->importAddFileExclusion(path); }
  app::ExclusionImportOutcome importRemoveFileExclusion(const QString& path) { return m_editor->importRemoveFileExclusion(path); }
 signals:
  void stateChanged();
  void pendingChanged();

 private:
  void updateRuntimePresentation();
  std::shared_ptr<app::VolumeConfigurationEditor> m_editor;
  std::shared_ptr<VolumeStatusViewModel> m_status;
  std::shared_ptr<ExclusionListViewModel> m_files;
  std::shared_ptr<ExclusionListViewModel> m_registry;
  bool m_commitsAllowed = true;
  QString m_commitRestriction;
  std::uint64_t m_presentationGeneration = 0;
};
}  // namespace uwf::presentation
