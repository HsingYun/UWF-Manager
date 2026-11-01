// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QObject>
#include <array>
#include <cstdint>
#include <memory>
#include <vector>

#include "application/configuration/editing/OverlayMaintenanceEditor.h"
#include "application/overlay/contracts/OverlayMaintenance.h"
#include "presentation/viewmodels/common/AsyncTask.h"

namespace uwf::app {
class OverlayPolicyEditor;
}
namespace uwf::presentation {
class OverlayMaintenanceViewModel final : public QObject {
  Q_OBJECT
 public:
  struct BlockedVolumeRow {
    QString volumeId;
    QString displayName;
  };
  enum class NoticeTone { Neutral, Accent, Warning };
  struct RuntimeNotice {
    QString heading;
    QString text;
    NoticeTone tone = NoticeTone::Neutral;
  };
  struct FullCommitState {
    QString label;
    QString hint;
    QString note;
    bool available = false;
    bool pending = false;
  };
  struct PanelState {
    QString storageStatus;
    QString hormStatus;
    QString protectionStatus;
    QString readError;
    QString protectReason;
    QString unprotectReason;
    bool busy = false;
    bool hormActive = false;
    bool protectEnabled = false;
    bool unprotectEnabled = false;
    bool manageProtectionEnabled = false;
  };
  enum class DialogFeedbackTone { Neutral, Warning, Success, Error };
  struct DialogState {
    QString title;
    QString submitLabel;
    QString cancelLabel;
    QString submitHint;
    QString feedback;
    QString feedbackHeading;
    DialogFeedbackTone feedbackTone = DialogFeedbackTone::Warning;
    bool busy = false;
    bool submitVisible = true;
    bool submitEnabled = false;
  };
  struct StorageVolumeRow {
    QString id;
    QString text;
    bool eligible = false;
  };
  struct StorageDialogState {
    DialogState dialog;
    std::vector<StorageVolumeRow> volumes;
    int selectedIndex = -1;
    QString filter;
    QString currentType;
    QString selectedVolume;
    QString nextType;
    QString capacity;
    QString previousHost;
    QString previousHostHint;
    bool selectionEnabled = false;
  };
  struct HormRequirementState {
    QString text;
    QString hint;
    std::optional<bool> met;
  };
  struct HormDialogState {
    DialogState dialog;
    std::array<HormRequirementState, 4> requirements;
    std::vector<BlockedVolumeRow> blockedVolumes;
    QString protectionText;
    QString protectionHint;
    bool disabling = false;
    bool blockersVisible = false;
    bool protectionVisible = false;
    bool protectionPendingVisible = false;
    bool protectEnabled = false;
  };
  [[nodiscard]] FullCommitState fullCommitState() const;
  [[nodiscard]] std::optional<RuntimeNotice> runtimeNotice() const;
  [[nodiscard]] PanelState panelState() const;
  [[nodiscard]] DialogState fullCommitDialogState() const;
  [[nodiscard]] StorageDialogState storageDialogState() const;
  [[nodiscard]] HormDialogState hormDialogState() const;
  void selectStorageVolume(QString volume);
  // Each dialog edits an isolated model. Submission commits only that draft
  // into its parent; a true result tells the view it may close.
  bool submitFullCommitDraft();
  bool submitStorageDraft();
  bool submitHormDraft();
  explicit OverlayMaintenanceViewModel(std::shared_ptr<app::OverlayMaintenanceActions> actions, QObject* parent = nullptr);
  OverlayMaintenanceViewModel(std::shared_ptr<app::OverlayMaintenanceActions> actions, std::shared_ptr<app::OverlayMaintenanceEditor> editor,
                              QObject* parent = nullptr);
  OverlayMaintenanceViewModel(std::shared_ptr<app::OverlayMaintenanceEditor> draft, std::shared_ptr<OverlayMaintenanceViewModel> source,
                              std::weak_ptr<app::OverlayPolicyEditor> policy);
  [[nodiscard]] const std::shared_ptr<app::OverlayMaintenanceEditor>& editor() const { return m_editor; }
  // An edit shares live observations, but owns an isolated draft. Nested edits
  // commit into their parent draft; destroying one never requires a rollback.
  [[nodiscard]] static std::shared_ptr<OverlayMaintenanceViewModel> editDraft(std::shared_ptr<OverlayMaintenanceViewModel> source);
  [[nodiscard]] bool draftEdited() const { return m_editor->draftEdited(); }
  [[nodiscard]] bool draftConflicted() const { return m_editor->draftConflicted(); }
  [[nodiscard]] QString draftCommitDisabledReason() const { return restrictionText(m_editor->draftCommitRestriction()); }
  [[nodiscard]] bool canCommitDraft() const { return m_editor->canCommitDraft(); }
  [[nodiscard]] QString draftConflictText(app::OverlayMaintenanceEditor::Restriction restriction) const { return restrictionText(restriction); }
  bool commitDraft() { return m_editor->commitDraft(); }
  [[nodiscard]] const app::OverlayMaintenanceSnapshot& snapshot() const { return m_editor->snapshot(); }
  [[nodiscard]] bool busy() const { return m_editor->busy(); }
  [[nodiscard]] bool hasSnapshot() const { return m_editor->hasSnapshot(); }
  [[nodiscard]] const app::OverlayMaintenanceChanges& draft() const { return m_editor->draft(); }
  [[nodiscard]] bool hasPendingChanges() const { return !draft().empty(); }
  [[nodiscard]] std::size_t pendingCount() const { return draft().count(); }
  [[nodiscard]] bool restartRequired() const { return m_editor->restartRequired(); }
  [[nodiscard]] bool swapRestartRequired() const { return m_editor->swapRestartRequired(); }
  [[nodiscard]] bool currentRomLoaded() const { return m_editor->currentRomLoaded(); }
  [[nodiscard]] bool hormActive() const { return snapshot().hormEnabled == true; }
  [[nodiscard]] QString fullCommitLabel() const;
  [[nodiscard]] static QString storageLocationText(const app::OverlayStorageLocation& location);
  [[nodiscard]] static QString storageLocationError(const app::OverlayStorageLocation& location);
  [[nodiscard]] QString currentOverlayTypeText() const;
  [[nodiscard]] QString storageTargetTypeText() const;
  [[nodiscard]] QString fullCommitDisabledReason() const { return restrictionText(m_editor->fullCommitRestriction()); }
  [[nodiscard]] QString hormDisabledReason(bool enabled) const { return restrictionText(m_editor->hormRestriction(enabled)); }
  [[nodiscard]] QString hibernationStateText() const;
  [[nodiscard]] std::vector<BlockedVolumeRow> hormBlockedVolumes() const;
  [[nodiscard]] app::HormEligibility hormEligibility(bool enabled) const { return m_editor->hormEligibility(enabled); }
  [[nodiscard]] bool hormRestrictionCoveredByChecklist() const { return m_editor->hormRestrictionCoveredByChecklist(); }
  [[nodiscard]] bool canFullCommit() const { return m_editor->canFullCommit(); }
  [[nodiscard]] bool canCreateSwap() const { return m_editor->canCreateSwap(); }
  [[nodiscard]] bool canSetHorm(bool enabled) const { return m_editor->canSetHorm(enabled); }
  void setInteractionContext(bool elevated, bool writesBlocked) { m_editor->setInteractionContext(elevated, writesBlocked); }
  void refresh();
  void adoptPublishedState();
  bool stageFullCommit() { return m_editor->stageFullCommit(); }
  bool unstageFullCommit() { return m_editor->unstageFullCommit(); }
  bool stageStorageLocation(const QString& volume) { return m_editor->stageStorageLocation(volume); }
  bool stageHorm(bool enabled) { return m_editor->stageHorm(enabled); }
  bool stageAllVolumesProtection(bool enabled) { return m_editor->stageAllVolumesProtection(enabled); }
  void discardDraft() { m_editor->discardDraft(); }
  void replaceDraft(const app::OverlayMaintenanceChanges& draft) { m_editor->replaceDraft(draft); }
  // Only executor steps with confirmed readback may be supplied here. A
  // setter acknowledgement alone does not establish any draft's postcondition.
  void reconcileAppliedDraft(const app::OverlayMaintenanceChanges& confirmed, const app::OverlayMaintenanceSnapshot& observed) {
    m_editor->reconcileAppliedDraft(confirmed, observed);
  }
 signals:
  void stateChanged();
  void busyChanged();
  void pendingChanged();
  void refreshFinished(bool succeeded);

 private:
  explicit OverlayMaintenanceViewModel(std::shared_ptr<OverlayMaintenanceViewModel> source);
  void bindEditor();
  [[nodiscard]] DialogState dialogState(const QString& reason, bool failed, bool success = false) const;
  [[nodiscard]] QString selectedStorageVolume() const;
  [[nodiscard]] QString restrictionText(app::OverlayMaintenanceEditor::Restriction restriction) const;
  std::shared_ptr<app::OverlayMaintenanceActions> m_actions;
  std::shared_ptr<app::OverlayMaintenanceEditor> m_editor;
  std::shared_ptr<OverlayMaintenanceViewModel> m_source;
  std::weak_ptr<app::OverlayPolicyEditor> m_policy;
  std::optional<QString> m_storageSelection;
  bool m_refreshFailed = false;
  bool m_hormDisabling = false;
  bool m_adoptPublishedPending = false;
  AsyncTask m_task;
};
}  // namespace uwf::presentation
