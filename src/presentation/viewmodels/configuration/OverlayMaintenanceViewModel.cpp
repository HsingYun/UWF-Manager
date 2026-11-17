// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "presentation/viewmodels/configuration/OverlayMaintenanceViewModel.h"

#include <QPointer>
#include <algorithm>
#include <exception>
#include <stdexcept>
#include <utility>

#include "application/configuration/overlay/OverlayPolicyEditor.h"
#include "application/localization/HormText.h"
#include "application/localization/ConfigurationModeText.h"
#include "domain/uwf/policies/ConfigurationModePolicy.h"
#include "application/localization/OverlayTypeText.h"
#include "presentation/viewmodels/common/PresentationText.h"
#include "presentation/viewmodels/configuration/MaintenanceValidationText.h"

namespace uwf::presentation {
OverlayMaintenanceViewModel::OverlayMaintenanceViewModel(std::shared_ptr<app::OverlayMaintenanceActions> actions, QObject* parent)
    : OverlayMaintenanceViewModel(std::move(actions), std::make_shared<app::OverlayMaintenanceEditor>(), parent) {}
OverlayMaintenanceViewModel::OverlayMaintenanceViewModel(std::shared_ptr<app::OverlayMaintenanceActions> actions,
                                                         std::shared_ptr<app::OverlayMaintenanceEditor> editor, QObject* parent)
    : QObject(parent), m_actions(std::move(actions)), m_editor(std::move(editor)), m_task(this) {
  if (!m_actions || !m_editor) throw std::invalid_argument("Overlay maintenance requires actions and an editor");
  bindEditor();
}
OverlayMaintenanceViewModel::OverlayMaintenanceViewModel(std::shared_ptr<OverlayMaintenanceViewModel> source)
    : OverlayMaintenanceViewModel(app::OverlayMaintenanceEditor::editDraft(source->editor()), source, source->m_policy) {}
OverlayMaintenanceViewModel::OverlayMaintenanceViewModel(std::shared_ptr<app::OverlayMaintenanceEditor> draft,
                                                         std::shared_ptr<OverlayMaintenanceViewModel> source,
                                                         std::weak_ptr<app::OverlayPolicyEditor> policy)
    : m_editor(std::move(draft)), m_source(std::move(source)), m_policy(std::move(policy)), m_task(this) {
  if (!m_editor || !m_source) throw std::invalid_argument("A maintenance draft presentation requires an editor and observation source");
  bindEditor();
}
void OverlayMaintenanceViewModel::bindEditor() {
  m_hormDisabling = draft().hormEnabled.value_or(hormActive());
  if (const auto policy = m_policy.lock())
    connect(policy.get(), &app::OverlayPolicyEditor::stateChanged, this, &OverlayMaintenanceViewModel::stateChanged);
  connect(m_editor.get(), &app::OverlayMaintenanceEditor::stateChanged, this, &OverlayMaintenanceViewModel::stateChanged);
  connect(m_editor.get(), &app::OverlayMaintenanceEditor::pendingChanged, this, &OverlayMaintenanceViewModel::pendingChanged);
  connect(m_editor.get(), &app::OverlayMaintenanceEditor::busyChanged, this, &OverlayMaintenanceViewModel::busyChanged);
  connect(m_editor.get(), &app::OverlayMaintenanceEditor::refreshFinished, this, [this](const bool succeeded) {
    m_refreshFailed = !succeeded;
    if (snapshot().hormEnabled) m_hormDisabling = draft().hormEnabled.value_or(hormActive());
    const QPointer self(this);
    emit stateChanged();
    if (self) emit refreshFinished(succeeded);
  });
}
std::shared_ptr<OverlayMaintenanceViewModel> OverlayMaintenanceViewModel::editDraft(std::shared_ptr<OverlayMaintenanceViewModel> source) {
  if (!source) throw std::invalid_argument("A maintenance draft edit requires a source");
  return std::shared_ptr<OverlayMaintenanceViewModel>(new OverlayMaintenanceViewModel(std::move(source)));
}
QString OverlayMaintenanceViewModel::restrictionText(const app::OverlayMaintenanceEditor::Restriction restriction) const {
  using Issue = app::OverlayMaintenanceEditor::Issue;
  switch (restriction.issue) {
    case Issue::None:
      return {};
    case Issue::DraftChanged:
      return PresentationText::tr("The configuration has changed. Close and reopen this dialog before editing.");
    case Issue::Busy:
      return PresentationText::tr("An operation is in progress. Please wait.");
    case Issue::AdministratorRequired:
      return PresentationText::tr("Administrator privileges are required.");
    case Issue::SnapshotRequired:
      return PresentationText::tr("Read the current UWF state first.");
    case Issue::WritesBlocked:
      return PresentationText::tr("Configuration changes are temporarily unavailable.");
    case Issue::FullCommitStarted:
      return PresentationText::tr("Full volume commit mode is already enabled. Restart before using it again.");
    case Issue::FullCommitUnsupported:
      return PresentationText::tr("Full volume commit is not available on this system.");
    case Issue::ProtectionUnknown:
      return PresentationText::tr("The current protection mode could not be read. Refresh the state.");
    case Issue::FullCommitRequiresRom:
      return PresentationText::tr("Full volume commit is available only while Read-Only Media mode is active.");
    case Issue::FullCommitRequiresRestart:
      return PresentationText::tr("Disable HORM and restart before using full volume commit.");
    case Issue::CommitStateUnknown:
      return PresentationText::tr("The commit or HORM state is unknown. Refresh before continuing.");
    case Issue::StorageUnsupported:
      return PresentationText::tr("Configuring the storage location is not available on this system.");
    case Issue::FilterUnknown:
      return PresentationText::tr("The current UWF state could not be read. Refresh before continuing.");
    case Issue::StorageRequiresFilterDisabled:
      return PresentationText::tr("Disable UWF and restart before configuring the storage location.");
    case Issue::StorageRequiresRestart:
      return PresentationText::tr("Disable HORM and restart before configuring the storage location.");
    case Issue::HormUnknown:
      return PresentationText::tr("The HORM state is unknown. Refresh before continuing.");
    case Issue::StorageInventoryUnavailable:
      return PresentationText::tr("No suitable storage volume is available. Refresh the volume list.");
    case Issue::HormRequiresRestart:
      return PresentationText::tr("Restart and read the current state before enabling HORM again.");
    case Issue::BulkProtectionUnsupported:
      return PresentationText::tr("Bulk volume protection is not available on this system.");
    case Issue::BulkProtectionRequiresRestart:
      return PresentationText::tr("Disable HORM and restart before changing volume protection.");
    case Issue::ConfigurationNotPersistent:
      return app::configurationModeRestrictionText(core::ConfigurationWriteRestriction::RomVolatile);
    case Issue::StorageVolumeUnavailable:
      return maintenanceValidationText(app::MaintenancePlanIssue::StorageVolumeUnavailable);
    case Issue::HormRequirement:
      return restriction.horm == app::HormIssue::HibernationUnavailable ? hibernationStateText() : app::hormIssueText(restriction.horm);
  }
  return {};
}
QString OverlayMaintenanceViewModel::fullCommitLabel() const {
  if (snapshot().overlayCommit == true) return PresentationText::tr("Full volume commit mode (enabled until restart)");
  if (draft().fullCommit) return PresentationText::tr("Full volume commit (pending apply)");
  return PresentationText::tr("Full volume commit");
}

QString OverlayMaintenanceViewModel::storageLocationText(const app::OverlayStorageLocation& location) {
  const auto name = location.driveLetter.isEmpty() ? location.volumeId.value_or(QString{}) : location.driveLetter;
  switch (location.origin) {
    case app::OverlayStorageOrigin::NotUsed:
      return PresentationText::tr("Not using a Disk overlay");
    case app::OverlayStorageOrigin::SystemDriveDefault:
      return name.isEmpty() ? PresentationText::tr("Unknown") : PresentationText::tr("System drive default (%1)").arg(name);
    case app::OverlayStorageOrigin::ConfiguredMarker:
      return name.isEmpty() ? PresentationText::tr("Unknown") : name;
    default:
      return PresentationText::tr("Unknown");
  }
}

QString OverlayMaintenanceViewModel::storageLocationError(const app::OverlayStorageLocation& location) {
  QString reason;
  switch (location.issue) {
    case app::OverlayStorageIssue::None:
      break;
    case app::OverlayStorageIssue::ObservationUnavailable:
      reason = PresentationText::tr("The configured storage location could not be read.");
      break;
    case app::OverlayStorageIssue::OverlayTypeUnknown:
      reason = PresentationText::tr("The Disk overlay type could not be established.");
      break;
    case app::OverlayStorageIssue::MarkerUnavailable:
      reason = PresentationText::tr("A configured storage marker could not be read.");
      break;
    case app::OverlayStorageIssue::MultipleHosts:
      reason = PresentationText::tr("More than one storage volume has a nonzero marker.");
      break;
    case app::OverlayStorageIssue::SystemDriveUnavailable:
      reason = PresentationText::tr("The system drive could not be identified.");
      break;
  }
  if (!reason.isEmpty() && !location.diagnostics.isEmpty()) reason += u'\n';
  return reason + location.diagnostics;
}

QString OverlayMaintenanceViewModel::currentOverlayTypeText() const { return app::overlayTypeText(core::overlayTypeFromValue(snapshot().currentOverlayType)); }

QString OverlayMaintenanceViewModel::storageTargetTypeText() const {
  const auto disk = app::overlayTypeText(core::OverlayType::Disk);
  return core::overlayTypeFromValue(snapshot().nextOverlayType) == core::OverlayType::Disk
             ? disk
             : PresentationText::tr("%1 → %2").arg(app::overlayTypeText(core::overlayTypeFromValue(snapshot().nextOverlayType)), disk);
}

QString OverlayMaintenanceViewModel::hibernationStateText() const {
  if (!snapshot().hibernationCapabilities) return PresentationText::tr("Unable to read");
  const auto& capabilities = *snapshot().hibernationCapabilities;
  if (!capabilities.systemS4) return PresentationText::tr("Hibernation is not supported");
  if (!capabilities.hiberFilePresent || capabilities.fileType == app::HibernationFileType::None) return PresentationText::tr("Hibernation is not enabled");
  if (capabilities.fileType == app::HibernationFileType::Reduced) return PresentationText::tr("Fast startup only");
  if (capabilities.fileType != app::HibernationFileType::Full) return PresentationText::tr("Unable to read");
  return PresentationText::tr("Met");
}

std::vector<OverlayMaintenanceViewModel::BlockedVolumeRow> OverlayMaintenanceViewModel::hormBlockedVolumes() const {
  std::vector<BlockedVolumeRow> rows;
  rows.reserve(snapshot().blockedVolumes.size());
  for (const auto& volume : snapshot().blockedVolumes)
    rows.push_back({volume.volumeId, volume.name.isEmpty() ? PresentationText::tr("Partition") : volume.name});
  return rows;
}
OverlayMaintenanceViewModel::FullCommitState OverlayMaintenanceViewModel::fullCommitState() const {
  FullCommitState state;
  state.label = fullCommitLabel();
  state.pending = draft().fullCommit && snapshot().overlayCommit != true;
  state.available = state.pending || canFullCommit();
  state.hint =
      state.available ? PresentationText::tr("Save changes on all protected volumes. Review the effects before continuing.") : fullCommitDisabledReason();
  const auto reason = fullCommitDisabledReason();
  state.note = reason.isEmpty() ? PresentationText::tr("Save changes on all protected volumes") : reason;
  return state;
}
std::optional<OverlayMaintenanceViewModel::RuntimeNotice> OverlayMaintenanceViewModel::runtimeNotice() const {
  if (snapshot().overlayCommit == true)
    return RuntimeNotice{fullCommitLabel(),
                         PresentationText::tr("Full volume commit mode remains enabled until restart. New changes are saved directly. "
                                              "Individual commits remain unavailable."),
                         NoticeTone::Accent};
  if (draft().fullCommit)
    return RuntimeNotice{fullCommitLabel(), PresentationText::tr("Full volume commit is pending. It will start only after you confirm Review and apply."),
                         NoticeTone::Accent};
  if (restartRequired() || (snapshot().hormEnabled == false && snapshot().romDisposition &&
                            (*snapshot().romDisposition & compat::disposition::horm) != 0))
    return RuntimeNotice{{},
                         PresentationText::tr("HORM is disabled. Automatic staged-file commits are paused until restart. Manual operations follow the current UWF capabilities."),
                         NoticeTone::Warning};
  if (hormActive())
    return RuntimeNotice{
        QStringLiteral("HORM · ") + PresentationText::tr("Enabled"),
        PresentationText::tr("HORM is active. UWF settings and individual commits are unavailable. Disable HORM and restart before changing them.")};
  return {};
}
OverlayMaintenanceViewModel::PanelState OverlayMaintenanceViewModel::panelState() const {
  PanelState state;
  const auto& observed = snapshot();
  const auto& pending = draft();
  const auto displayVolume = [&observed](const QString& id) {
    for (const auto& volume : observed.volumes)
      if (volume.id == id && !volume.driveLetter.isEmpty()) return volume.driveLetter;
    return id;
  };
  state.busy = busy();
  state.hormActive = hormActive();
  state.storageStatus = PresentationText::tr("Current: %1").arg(storageLocationText(observed.currentStorage));
  state.storageStatus += u'\n' + PresentationText::tr("Next startup: %1").arg(storageLocationText(observed.nextStorage));
  if (pending.storageVolume) state.storageStatus += u'\n' + PresentationText::tr("Draft: %1 · apply required").arg(displayVolume(*pending.storageVolume));
  if (state.busy) state.storageStatus = PresentationText::tr("Reading state…");
  const auto currentHorm = state.busy                              ? PresentationText::tr("Reading state…")
                           : !observed.hormCapability.canAttempt() ? PresentationText::tr("Not supported")
                           : !observed.hormEnabled                 ? PresentationText::tr("State unknown")
                           : *observed.hormEnabled                 ? PresentationText::tr("Enabled")
                           : restartRequired()                     ? PresentationText::tr("Disabled · restart required")
                                                                   : PresentationText::tr("Not enabled");
  state.hormStatus = PresentationText::tr("Current: %1").arg(currentHorm);
  if (pending.hormEnabled)
    state.hormStatus +=
        u'\n' +
        PresentationText::tr("Draft: %1 · apply required").arg(*pending.hormEnabled ? PresentationText::tr("Enabled") : PresentationText::tr("Disabled"));
  const auto protectRestriction = m_editor->allVolumesProtectionRestriction(true);
  const auto unprotectRestriction = m_editor->allVolumesProtectionRestriction(false);
  state.protectReason = restrictionText(protectRestriction);
  state.unprotectReason = restrictionText(unprotectRestriction);
  state.protectEnabled = protectRestriction.empty() && pending.allVolumesProtected != true;
  state.unprotectEnabled = unprotectRestriction.empty() && pending.allVolumesProtected != false;
  state.manageProtectionEnabled = !state.busy && (protectRestriction.empty() || unprotectRestriction.empty());
  state.protectionStatus = !observed.allVolumesNextProtected || !observed.allVolumesNextUnprotected ? PresentationText::tr("State unknown")
                           : observed.allVolumesNextProtected == true   ? PresentationText::tr("Next startup: all volumes protected")
                           : observed.allVolumesNextUnprotected == true ? PresentationText::tr("Next startup: all volumes unprotected")
                                                                        : PresentationText::tr("Next startup: individual volume settings");
  if (pending.allVolumesProtected)
    state.protectionStatus += u'\n' + PresentationText::tr("Draft: %1 · apply required")
                                          .arg(*pending.allVolumesProtected ? PresentationText::tr("Protect all") : PresentationText::tr("Unprotect all"));
  if (state.busy) state.protectionStatus = PresentationText::tr("Reading state…");
  state.readError = observed.readError;
  return state;
}
OverlayMaintenanceViewModel::DialogState OverlayMaintenanceViewModel::dialogState(const QString& reason, const bool failed, const bool success) const {
  DialogState state;
  state.busy = busy();
  state.cancelLabel = PresentationText::tr("Cancel");
  state.feedbackTone = failed ? DialogFeedbackTone::Error : success ? DialogFeedbackTone::Success : DialogFeedbackTone::Warning;
  state.feedbackHeading = failed ? PresentationText::tr("The result could not be confirmed") : QString{};
  state.feedback = failed ? PresentationText::tr("The current UWF state could not be read. Refresh before continuing.") : reason;
  if (draftEdited() && !canCommitDraft() && !failed) state.feedback = draftCommitDisabledReason();
  return state;
}
OverlayMaintenanceViewModel::DialogState OverlayMaintenanceViewModel::fullCommitDialogState() const {
  auto state = dialogState(fullCommitDisabledReason(), m_refreshFailed);
  const bool modeEnabled = snapshot().overlayCommit == true;
  const bool reread = m_refreshFailed || !hasSnapshot() || !snapshot().overlayCommit.has_value();
  state.title = modeEnabled ? fullCommitLabel() : PresentationText::tr("Add full volume commit to the draft?");
  if (modeEnabled) {
    state.feedbackTone = DialogFeedbackTone::Neutral;
    state.cancelLabel = PresentationText::tr("Close");
  }
  state.submitVisible = !modeEnabled;
  state.submitLabel = state.busy ? PresentationText::tr("Processing…")
                      : reread   ? PresentationText::tr("Read state again")
                                 : PresentationText::tr("Add to draft");
  state.submitEnabled = !state.busy && !modeEnabled && !draftConflicted() && (reread ? snapshot().fullCommitCapability.canAttempt() : canFullCommit());
  return state;
}
QString OverlayMaintenanceViewModel::selectedStorageVolume() const {
  if (m_storageSelection) return *m_storageSelection;
  const auto& observed = snapshot();
  auto preferred = draft().storageVolume.value_or(observed.nextStorage.volumeId.value_or(observed.systemVolumeId.value_or(QString{})));
  if (preferred.isEmpty() && !observed.systemDrive.isEmpty())
    for (const auto& volume : observed.volumes)
      if (volume.driveLetter.compare(observed.systemDrive, Qt::CaseInsensitive) == 0) preferred = volume.id;
  return preferred;
}
OverlayMaintenanceViewModel::StorageDialogState OverlayMaintenanceViewModel::storageDialogState() const {
  StorageDialogState state;
  const auto& observed = snapshot();
  const auto preferred = selectedStorageVolume();
  std::vector<const app::OverlaySwapVolume*> ordered;
  ordered.reserve(observed.volumes.size());
  for (const auto& volume : observed.volumes) ordered.push_back(&volume);
  const auto location = [](const auto& volume) { return volume.driveLetter.isEmpty() ? volume.id : volume.driveLetter; };
  std::ranges::sort(ordered,
                    [&location](const auto* first, const auto* second) { return QString::localeAwareCompare(location(*first), location(*second)) < 0; });
  QString selected;
  for (const auto* volume : ordered) {
    const auto address = location(*volume);
    const auto name = volume->label.isEmpty() ? address : PresentationText::tr("%1 (%2)").arg(volume->label, address);
    constexpr double bytesPerGiB = 1073741824.0;
    const auto available = PresentationText::tr("%1 GB available").arg(QString::number(static_cast<double>(volume->freeBytes) / bytesPerGiB, 'f', 0));
    if (volume->id == preferred && volume->eligible) {
      state.selectedIndex = static_cast<int>(state.volumes.size());
      selected = address;
    }
    state.volumes.push_back({volume->id, PresentationText::tr("%1 · %2").arg(name, available), volume->eligible});
  }
  const auto restriction = m_editor->storageRestriction();
  const auto reason = restrictionText(restriction);
  const bool configured = !m_storageSelection && !m_refreshFailed && swapRestartRequired();
  const bool reread = m_refreshFailed || !hasSnapshot() || !observed.volumesReadable;
  const auto success = PresentationText::tr("Configured; takes effect after restarting. The overlay from this startup will not be moved to the new location.");
  state.dialog = dialogState(configured ? success : reason, m_refreshFailed, configured);
  state.dialog.title = PresentationText::tr("Configure storage location");
  if (configured) {
    state.dialog.feedbackHeading = PresentationText::tr("Configured; takes effect after restarting");
    state.dialog.cancelLabel = PresentationText::tr("Close");
  }
  state.dialog.submitLabel = state.dialog.busy                     ? PresentationText::tr("Processing…")
                             : reread                              ? PresentationText::tr("Read state again")
                             : configured || draft().storageVolume ? PresentationText::tr("Save draft")
                                                                   : PresentationText::tr("Add to draft");
  state.dialog.submitEnabled =
      !state.dialog.busy && !draftConflicted() && (reread ? observed.swapCapability.canAttempt() : restriction.empty() && state.selectedIndex >= 0);
  state.selectionEnabled = !state.dialog.busy && !m_refreshFailed && restriction.empty();
  state.filter = !observed.filterEnabled   ? PresentationText::tr("Unknown")
                 : *observed.filterEnabled ? PresentationText::tr("Enabled")
                                           : PresentationText::tr("Disabled");
  state.currentType = currentOverlayTypeText();
  state.selectedVolume = selected.isEmpty() ? PresentationText::tr("Not selected") : selected;
  state.nextType = storageTargetTypeText();
  auto capacity = observed.maximumOverlaySize;
  if (const auto policy = m_policy.lock()) {
    auto maintenance = draft();
    if (!preferred.isEmpty()) maintenance.storageVolume = preferred;
    capacity = policy->targetOverlay(policy->projectEdit(policy->beginEdit(), maintenance)).maximumSizeMb;
  }
  state.capacity = capacity ? PresentationText::tr("%1 MB").arg(*capacity) : PresentationText::tr("Unknown");
  const auto previous = storageLocationText(observed.nextStorage);
  state.previousHost = observed.nextStorage.volumeId && !selected.isEmpty() && *observed.nextStorage.volumeId != preferred
                           ? PresentationText::tr("%1 (will be replaced)").arg(previous)
                           : previous;
  state.previousHostHint = storageLocationError(observed.nextStorage);
  return state;
}
OverlayMaintenanceViewModel::HormDialogState OverlayMaintenanceViewModel::hormDialogState() const {
  HormDialogState state;
  const auto& observed = snapshot();
  const bool failed = hasSnapshot() && !observed.hormEnabled.has_value();
  const bool unknown = !hasSnapshot() || !observed.hormEnabled.has_value();
  state.disabling = m_hormDisabling;
  const bool target = !state.disabling;
  const auto restriction = m_editor->hormRestriction(target);
  const auto reason = restrictionText(restriction);
  // The checklist describes observations; the primary restriction also gives
  // the next action (for example enabling UWF and restarting). Keep it visible.
  state.dialog = dialogState(reason, failed);
  state.dialog.title = state.disabling ? PresentationText::tr("Disable saved-state startup?") : PresentationText::tr("Enable saved-state startup?");
  const auto eligibility = hormEligibility(target);
  for (std::size_t index = 0; index < state.requirements.size(); ++index) {
    auto& requirement = state.requirements[index];
    requirement.met = state.dialog.busy ? std::nullopt : eligibility.requirements[index];
    requirement.text = state.dialog.busy  ? PresentationText::tr("Reading state…")
                       : !requirement.met ? PresentationText::tr("Unable to read")
                       : *requirement.met ? PresentationText::tr("Met")
                                          : PresentationText::tr("Not met");
  }
  state.requirements[2].hint = observed.hibernationError;
  if (!state.dialog.busy && observed.hibernationAvailable == false) state.requirements[2].text = hibernationStateText();
  state.requirements[3].hint = observed.allVolumesProtectedError;
  state.blockedVolumes = hormBlockedVolumes();
  state.blockersVisible = !state.dialog.busy && !state.blockedVolumes.empty();
  const bool pendingProtection = draft().allVolumesProtected == true;
  const bool nextProtected = observed.allVolumesNextProtected == true && observed.allVolumesProtected != true;
  state.protectionVisible = target && !state.dialog.busy && observed.allVolumesProtected == false;
  state.protectionPendingVisible = state.protectionVisible && (pendingProtection || nextProtected);
  state.protectionText = pendingProtection ? PresentationText::tr("Protection for all volumes is pending apply")
                                           : PresentationText::tr("Volume protection takes effect after restarting");
  const auto protectionRestriction = m_editor->allVolumesProtectionRestriction(true);
  state.protectionHint = restrictionText(protectionRestriction);
  state.protectEnabled = !pendingProtection && !nextProtected && protectionRestriction.empty();
  state.dialog.submitLabel = state.dialog.busy             ? PresentationText::tr("Processing…")
                             : pendingProtection && target ? PresentationText::tr("OK")
                             : draft().hormEnabled         ? PresentationText::tr("Save draft")
                                                           : PresentationText::tr("Add to draft");
  state.dialog.submitHint = reason;
  state.dialog.submitEnabled =
      !state.dialog.busy && !draftConflicted() && ((pendingProtection && target && canCommitDraft()) || (!unknown && !failed && restriction.empty()));
  return state;
}
void OverlayMaintenanceViewModel::selectStorageVolume(QString volume) {
  if (m_storageSelection == volume) return;
  m_storageSelection = std::move(volume);
  emit stateChanged();
}
bool OverlayMaintenanceViewModel::submitFullCommitDraft() {
  if (!fullCommitDialogState().submitEnabled) return false;
  if (m_refreshFailed || !hasSnapshot() || !snapshot().overlayCommit.has_value()) {
    refresh();
    return false;
  }
  const auto editor = m_editor;
  return editor->stageFullCommit() && editor->commitDraft();
}
bool OverlayMaintenanceViewModel::submitStorageDraft() {
  const auto state = storageDialogState();
  if (!state.dialog.submitEnabled) return false;
  if (m_refreshFailed || !hasSnapshot() || !snapshot().volumesReadable) {
    refresh();
    return false;
  }
  const auto editor = m_editor;
  return editor->stageStorageLocation(state.volumes.at(static_cast<std::size_t>(state.selectedIndex)).id) && editor->commitDraft();
}
bool OverlayMaintenanceViewModel::submitHormDraft() {
  if (!hormDialogState().dialog.submitEnabled) return false;
  const auto editor = m_editor;
  if (!m_hormDisabling && draft().allVolumesProtected == true) return editor->commitDraft();
  return editor->stageHorm(!m_hormDisabling) && editor->commitDraft();
}
void OverlayMaintenanceViewModel::adoptPublishedState() {
  if (m_source) {
    m_source->adoptPublishedState();
    return;
  }
  if (busy()) {
    m_adoptPublishedPending = true;
    return;
  }
  const auto observed = m_actions->snapshot();
  if (!observed) return;
  if (const auto generation = m_editor->beginObservation()) m_editor->finishObservation(*generation, *observed);
}
void OverlayMaintenanceViewModel::refresh() {
  if (m_source) {
    m_source->refresh();
    return;
  }
  const auto editor = m_editor;
  const auto actions = m_actions;
  const QPointer self(this);
  const auto generation = editor->beginObservation();
  if (!self || !generation) return;
  m_task.start([actions](const std::stop_token stop) { return actions->observe(stop); },
               [self, editor, actions, generation = *generation](AsyncResult<app::OverlayMaintenanceSnapshot> response) {
                 app::OverlayMaintenanceSnapshot observed;
                 if (const auto failure = std::get_if<std::exception_ptr>(&response)) {
                   try {
                     std::rethrow_exception(*failure);
                   } catch (const std::exception& error) {
                     observed.readError = QString::fromUtf8(error.what());
                   } catch (...) {
                     observed.readError = PresentationText::tr("The operation failed with an unknown error.");
                   }
                 } else
                   observed = std::get<app::OverlayMaintenanceSnapshot>(std::move(response));
                 // The worker's host details may finish after an ordinary
                 // configuration write. The current published projection owns
                 // configuration values; the returned payload cannot roll them
                 // back in the maintenance editor.
                 bool adopted = false;
                 try {
                   if (auto published = actions->snapshot()) {
                     if (!observed.readError.isEmpty() && !published->readError.contains(observed.readError)) {
                       if (!published->readError.isEmpty()) published->readError += u'\n';
                       published->readError += observed.readError;
                     }
                     observed = std::move(*published);
                     adopted = true;
                   }
                 } catch (const std::exception& error) {
                   if (!observed.readError.isEmpty()) observed.readError += u'\n';
                   observed.readError += QString::fromUtf8(error.what());
                 }
                 const bool pending = self && std::exchange(self->m_adoptPublishedPending, false);
                 editor->finishObservation(generation, std::move(observed));
                 if (self && pending && !adopted && !self->busy()) self->adoptPublishedState();
               });
}
}  // namespace uwf::presentation
#include "moc_OverlayMaintenanceViewModel.cpp"
