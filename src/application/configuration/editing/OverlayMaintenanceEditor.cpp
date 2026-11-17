// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "application/configuration/editing/OverlayMaintenanceEditor.h"

#include <QPointer>
#include <algorithm>
#include <stdexcept>
#include <utility>

#include "application/overlay/HormEligibility.h"
#include "domain/uwf/model/UwfModel.h"
#include "domain/uwf/policies/ConfigurationModePolicy.h"

namespace uwf::app {
namespace {
template <class Work>
void maintenanceFields(Work work) {
  using Draft = OverlayMaintenanceChanges;
  work(&Draft::storageVolume);
  work(&Draft::hormEnabled);
  work(&Draft::allVolumesProtected);
  work(&Draft::fullCommit);
}
}  // namespace
OverlayMaintenanceEditor::Update::Update(OverlayMaintenanceEditor& editor) : EditorUpdate(editor) {}
bool OverlayMaintenanceEditor::Update::publishSnapshot(const std::function<bool()>& continuePublishing) {
  return publish(continuePublishing, [&] { emit m_editor->pendingChanged(); }, [&] { emit m_editor->stateChanged(); });
}

OverlayMaintenanceEditor::OverlayMaintenanceEditor(std::shared_ptr<OverlayMaintenanceEditor> source)
    : m_observation(source->m_observation), m_source(std::move(source)), m_origin(m_source->draft()), m_draft(m_origin) {
  connect(m_source.get(), &OverlayMaintenanceEditor::pendingChanged, this, &OverlayMaintenanceEditor::rebaseDraft);
  connect(m_source.get(), &OverlayMaintenanceEditor::stateChanged, this, &OverlayMaintenanceEditor::stateChanged);
  connect(m_source.get(), &OverlayMaintenanceEditor::busyChanged, this, &OverlayMaintenanceEditor::busyChanged);
  connect(m_source.get(), &OverlayMaintenanceEditor::refreshFinished, this, &OverlayMaintenanceEditor::refreshFinished);
}
std::shared_ptr<OverlayMaintenanceEditor> OverlayMaintenanceEditor::editDraft(std::shared_ptr<OverlayMaintenanceEditor> source) {
  if (!source) throw std::invalid_argument("A maintenance draft edit requires a source");
  return std::shared_ptr<OverlayMaintenanceEditor>(new OverlayMaintenanceEditor(std::move(source)));
}
OverlayMaintenanceEditor::BatchUpdate::BatchUpdate(OverlayMaintenanceEditor& editor) : m_editor(&editor) { ++editor.m_batchDepth; }
OverlayMaintenanceEditor::BatchUpdate::BatchUpdate(BatchUpdate&& other) noexcept : m_editor(std::exchange(other.m_editor, {})) {}
OverlayMaintenanceEditor::BatchUpdate::~BatchUpdate() {
  if (m_editor) m_editor->finishBatch();
}
void OverlayMaintenanceEditor::finishBatch() {
  if (--m_batchDepth == 0 && std::exchange(m_batchChanged, false)) notifyDraftChanged();
}
void OverlayMaintenanceEditor::notifyDraftChanged() {
  if (m_batchDepth > 0) {
    m_batchChanged = true;
    return;
  }
  const QPointer self(this);
  emit pendingChanged();
  if (self) emit stateChanged();
}
bool OverlayMaintenanceEditor::draftEdited() const { return m_source && m_draft != m_origin; }

bool OverlayMaintenanceEditor::draftConflicted() const {
  if (!m_source) return false;
  bool conflict = false;
  maintenanceFields([&](auto field) {
    if (m_draft.*field != m_origin.*field && m_source->draft().*field != m_origin.*field && m_source->draft().*field != m_draft.*field) conflict = true;
  });
  return conflict;
}

bool OverlayMaintenanceEditor::canCommitDraft() const {
  if (!m_source) return false;
  return draftCommitRestriction().empty();
}

OverlayMaintenanceEditor::Restriction OverlayMaintenanceEditor::draftCommitRestriction() const {
  if (!draftEdited()) return {};
  if (draftConflicted()) return Issue::DraftChanged;
  if (const auto reason = commonRestriction(); !reason.empty()) return reason;
  if (m_draft.storageVolume != m_origin.storageVolume && m_draft.storageVolume) {
    if (const auto reason = storageRestriction(); !reason.empty()) return reason;
    if (!storageVolumeEligible(*m_draft.storageVolume)) return Issue::StorageVolumeUnavailable;
  }
  if (m_draft.hormEnabled != m_origin.hormEnabled && m_draft.hormEnabled)
    if (const auto reason = hormRestriction(*m_draft.hormEnabled); !reason.empty()) return reason;
  if (m_draft.allVolumesProtected != m_origin.allVolumesProtected && m_draft.allVolumesProtected)
    if (const auto reason = allVolumesProtectionRestriction(*m_draft.allVolumesProtected); !reason.empty()) return reason;
  if (m_draft.fullCommit != m_origin.fullCommit && m_draft.fullCommit) return fullCommitRestriction();
  return {};
}

bool OverlayMaintenanceEditor::commitDraft() {
  if (!canCommitDraft()) return false;
  auto merged = m_source->draft();
  maintenanceFields([&](auto field) {
    if (m_draft.*field != m_origin.*field) merged.*field = m_draft.*field;
  });
  m_origin = merged;
  m_draft = merged;
  const auto source = m_source;
  source->replaceDraft(merged);
  return true;
}

void OverlayMaintenanceEditor::rebaseDraft() {
  const auto before = m_draft;
  maintenanceFields([&](auto field) {
    if (m_draft.*field == m_origin.*field) m_draft.*field = m_origin.*field = m_source->draft().*field;
  });
  publishDraft(before);
}

bool OverlayMaintenanceEditor::currentRomLoaded() const {
  return m_observation->snapshot.filterEnabled == true && m_observation->snapshot.currentFlags && m_observation->snapshot.currentFlags->readOnlyMedia();
}

bool OverlayMaintenanceEditor::swapRestartRequired() const {
  return m_observation->swapConfigured || (core::overlayTypeFromValue(m_observation->snapshot.nextOverlayType) == core::OverlayType::Disk &&
                                           std::ranges::any_of(m_observation->snapshot.volumes, [](const auto& volume) {
                                             return volume.nextMarker == 1 && volume.currentMarker && *volume.currentMarker != 1;
                                           }));
}

OverlayMaintenanceEditor::Restriction OverlayMaintenanceEditor::commonRestriction() const {
  if (m_observation->busy) return Issue::Busy;
  if (!m_observation->elevated) return Issue::AdministratorRequired;
  if (!m_observation->hasSnapshot) return Issue::SnapshotRequired;
  if (m_observation->writesBlocked) return Issue::WritesBlocked;
  return {};
}

OverlayMaintenanceEditor::Restriction OverlayMaintenanceEditor::fullCommitRestriction() const {
  if (m_observation->snapshot.overlayCommit == true) return Issue::FullCommitStarted;
  if (const auto reason = commonRestriction(); !reason.empty()) return reason;
  if (!m_observation->snapshot.fullCommitCapability.canAttempt()) return Issue::FullCommitUnsupported;
  if (!m_observation->snapshot.filterEnabled || !m_observation->snapshot.currentFlags) return Issue::ProtectionUnknown;
  if (!currentRomLoaded()) return Issue::FullCommitRequiresRom;
  if (m_observation->restartRequired || hormActive()) return Issue::FullCommitRequiresRestart;
  if (!m_observation->snapshot.hormEnabled || !m_observation->snapshot.overlayCommit) return Issue::CommitStateUnknown;
  if (!m_observation->snapshot.romDisposition) return Issue::CommitStateUnknown;
  if (*m_observation->snapshot.romDisposition != 0) return Issue::FullCommitRequiresRestart;
  return {};
}

OverlayMaintenanceEditor::Restriction OverlayMaintenanceEditor::storageRestriction() const {
  if (const auto reason = commonRestriction(); !reason.empty()) return reason;
  if (!m_observation->snapshot.swapCapability.canAttempt()) return Issue::StorageUnsupported;
  if (!m_observation->snapshot.filterEnabled) return Issue::FilterUnknown;
  if (*m_observation->snapshot.filterEnabled) return Issue::StorageRequiresFilterDisabled;
  if (m_observation->restartRequired || hormActive()) return Issue::StorageRequiresRestart;
  if (!m_observation->snapshot.hormEnabled) return Issue::HormUnknown;
  // Capacity may be edited in the same ordinary configuration draft. The
  // combined review validates the final Disk capacity before any execution.
  if (!m_observation->snapshot.volumesReadable || std::ranges::none_of(m_observation->snapshot.volumes, [](const auto& volume) { return volume.eligible; }))
    return Issue::StorageInventoryUnavailable;
  return {};
}

OverlayMaintenanceEditor::Restriction OverlayMaintenanceEditor::hormRestriction(const bool enabled) const {
  // Returning to the observed state only removes an in-memory intent.
  if (m_draft.hormEnabled && m_observation->snapshot.hormEnabled == enabled) return {};
  if (const auto reason = commonRestriction(); !reason.empty()) return reason;
  if (enabled && m_observation->restartRequired) return Issue::HormRequiresRestart;
  const auto eligibility = hormEligibility(enabled);
  return eligibility.allowed() ? Restriction{} : Restriction{Issue::HormRequirement, eligibility.issue};
}

OverlayMaintenanceEditor::Restriction OverlayMaintenanceEditor::allVolumesProtectionRestriction(const bool enabled) const {
  if (const auto reason = commonRestriction(); !reason.empty()) return reason;
  const auto& capability = enabled ? m_observation->snapshot.protectAllCapability : m_observation->snapshot.unprotectAllCapability;
  if (!capability.canAttempt()) return Issue::BulkProtectionUnsupported;
  if (!m_observation->snapshot.hormEnabled) return Issue::HormUnknown;
  if (hormActive()) return Issue::BulkProtectionRequiresRestart;
  const auto& state = m_observation->snapshot;
  const core::ConfigurationModeState mode{state.filterEnabled, state.systemVolumeProtected,
      {state.hormEnabled, state.currentFlags ? std::optional{state.currentFlags->raw} : std::nullopt, state.romDisposition, state.overlayCommit}};
  const auto persistence = core::configurationWriteRestriction(mode);
  if (persistence == core::ConfigurationWriteRestriction::RomVolatile) return Issue::ConfigurationNotPersistent;
  if (persistence == core::ConfigurationWriteRestriction::Unknown) return Issue::ProtectionUnknown;
  return {};
}

HormEligibility OverlayMaintenanceEditor::hormEligibility(const bool enabled) const { return evaluateHormEligibility(m_observation->snapshot, enabled); }

bool OverlayMaintenanceEditor::hormRestrictionCoveredByChecklist() const {
  return commonRestriction().empty() && !m_observation->restartRequired && hormEligibility(true).checklistCoversIssue();
}

void OverlayMaintenanceEditor::setInteractionContext(const bool elevated, const bool writesBlocked) {
  if (m_source) {
    m_source->setInteractionContext(elevated, writesBlocked);
    return;
  }
  if (m_observation->elevated == elevated && m_observation->writesBlocked == writesBlocked) return;
  m_observation->elevated = elevated;
  m_observation->writesBlocked = writesBlocked;
  emit stateChanged();
}

void OverlayMaintenanceEditor::setBusy(const bool busy) {
  if (m_observation->busy == busy) return;
  m_observation->busy = busy;
  emit busyChanged();
}

void OverlayMaintenanceEditor::publishDraft(const OverlayMaintenanceChanges& before) {
  if (m_draft == before) return;
  const QPointer self(this);
  if (m_draft.allVolumesProtected != before.allVolumesProtected) emit bulkProtectionChanged(true);
  if (!self) return;
  notifyDraftChanged();
}

bool OverlayMaintenanceEditor::stageFullCommit() {
  if (m_observation->snapshot.overlayCommit == true) return false;
  if (m_draft.fullCommit) return true;
  if (!canFullCommit()) return false;
  const auto before = m_draft;
  m_draft.fullCommit = true;
  publishDraft(before);
  return true;
}

bool OverlayMaintenanceEditor::unstageFullCommit() {
  const auto before = m_draft;
  m_draft.fullCommit = false;
  publishDraft(before);
  return before.fullCommit;
}

bool OverlayMaintenanceEditor::stageStorageLocation(const QString& volume) {
  if (!canCreateSwap() || !storageVolumeEligible(volume)) return false;
  const auto before = m_draft;
  m_draft.storageVolume = volume;
  publishDraft(before);
  return true;
}

bool OverlayMaintenanceEditor::storageVolumeEligible(const QString& volume) const {
  return std::ranges::any_of(snapshot().volumes, [&](const auto& item) { return item.id == volume && item.eligible; });
}

bool OverlayMaintenanceEditor::stageHorm(const bool enabled) {
  if (!canSetHorm(enabled)) return false;
  const auto before = m_draft;
  m_draft.hormEnabled = m_observation->snapshot.hormEnabled == enabled ? std::nullopt : std::optional<bool>{enabled};
  publishDraft(before);
  return true;
}

bool OverlayMaintenanceEditor::stageAllVolumesProtection(const bool enabled) {
  if (!allVolumesProtectionRestriction(enabled).empty()) return false;
  const auto before = m_draft;
  m_draft.allVolumesProtected = enabled;
  const QPointer self(this);
  if (before.allVolumesProtected == enabled) emit bulkProtectionChanged(true);
  if (self) publishDraft(before);
  return true;
}

void OverlayMaintenanceEditor::discardDraft() {
  if (m_source) m_origin = m_source->draft();
  replaceDraft({});
}

void OverlayMaintenanceEditor::replaceDraft(const OverlayMaintenanceChanges& draft) {
  const auto before = m_draft;
  if (before == draft) return;
  m_draft = draft;
  if (m_observation->snapshot.overlayCommit == true) m_draft.fullCommit = false;
  publishDraft(before);
}

bool OverlayMaintenanceEditor::storageMatches(const OverlayMaintenanceSnapshot& observed, const QString& volume) {
  return observed.volumesReadable && core::overlayTypeFromValue(observed.nextOverlayType) == core::OverlayType::Disk &&
         std::ranges::any_of(observed.volumes, [&](const auto& item) { return item.id == volume && item.nextMarker == 1; });
}

void OverlayMaintenanceEditor::adoptObservation(OverlayMaintenanceSnapshot observed, const bool reconcileFullCommit) {
  const auto oldHorm = m_observation->snapshot.hormEnabled;
  if (oldHorm == true && observed.hormEnabled == false) m_observation->restartRequired = true;
  if (observed.overlayCommit == true) m_observation->fullCommitObserved = true;
  if (m_observation->fullCommitObserved) observed.overlayCommit = true;
  m_observation->snapshot = std::move(observed);
  m_observation->hasSnapshot = true;
  // Full Commit's observed trigger must prevent any repeat. Storage markers
  // and the HORM flag are scalar observations, not complete workflow receipts:
  // they may match after a failed or only partially completed operation.
  if (reconcileFullCommit && m_draft.fullCommit && m_observation->snapshot.overlayCommit == true) m_draft.fullCommit = false;
}

void OverlayMaintenanceEditor::reconcileAppliedDraft(const OverlayMaintenanceChanges& confirmed, const OverlayMaintenanceSnapshot& observed) {
  if (m_source) {
    m_source->reconcileAppliedDraft(confirmed, observed);
    return;
  }
  // A dialog's earlier read must not overwrite the executor's newer readback.
  ++m_observationGeneration;
  const bool wasBusy = std::exchange(m_observation->busy, false);
  const auto before = m_draft;
  adoptObservation(observed, false);
  if (confirmed.storageVolume && storageMatches(observed, *confirmed.storageVolume)) {
    m_observation->swapConfigured = true;
    if (m_draft.storageVolume == confirmed.storageVolume) m_draft.storageVolume.reset();
  }
  if (confirmed.hormEnabled && observed.hormEnabled == confirmed.hormEnabled) {
    if (!*confirmed.hormEnabled) m_observation->restartRequired = true;
    if (m_draft.hormEnabled == confirmed.hormEnabled) m_draft.hormEnabled.reset();
  }
  if (confirmed.fullCommit && observed.overlayCommit == true && m_draft.fullCommit) m_draft.fullCommit = false;
  // Confirmation names a successful workflow receipt. Later single-volume
  // overrides can legitimately make the final aggregate differ from the bulk.
  if (confirmed.allVolumesProtected && m_draft.allVolumesProtected == confirmed.allVolumesProtected) m_draft.allVolumesProtected.reset();
  const QPointer self(this);
  if (wasBusy) emit busyChanged();
  if (!self) return;
  publishDraft(before);
  if (self) emit stateChanged();
}
std::optional<std::uint64_t> OverlayMaintenanceEditor::beginObservation() {
  if (m_source) return m_source->beginObservation();
  if (busy()) return {};
  const auto generation = ++m_observationGeneration;
  const QPointer self(this);
  setBusy(true);
  if (!self || generation != m_observationGeneration) return {};
  emit stateChanged();
  if (!self || generation != m_observationGeneration) return {};
  return generation;
}
void OverlayMaintenanceEditor::finishObservation(const std::uint64_t generation, OverlayMaintenanceSnapshot observed) {
  if (m_source) {
    m_source->finishObservation(generation, std::move(observed));
    return;
  }
  if (generation != m_observationGeneration) return;
  const bool succeeded = observed.readError.isEmpty() && (observed.filterEnabled.has_value() || observed.hormEnabled.has_value());
  const auto before = m_draft;
  const QPointer self(this);
  adoptObservation(std::move(observed), true);
  setBusy(false);
  if (!self) return;
  publishDraft(before);
  if (!self || generation != m_observationGeneration) return;
  emit stateChanged();
  if (self && generation == m_observationGeneration) emit refreshFinished(succeeded);
}
}  // namespace uwf::app
#include "moc_OverlayMaintenanceEditor.cpp"
