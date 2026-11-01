// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <QObject>
#include <QPointer>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>

#include "application/configuration/editing/EditorUpdate.h"
#include "application/overlay/HormEligibility.h"
#include "application/overlay/contracts/OverlayMaintenance.h"
namespace uwf::app {
class OverlayMaintenanceEditor final : public QObject {
  Q_OBJECT
 public:
  enum class Issue {
    None,
    DraftChanged,
    Busy,
    AdministratorRequired,
    SnapshotRequired,
    WritesBlocked,
    FullCommitStarted,
    FullCommitUnsupported,
    ProtectionUnknown,
    FullCommitRequiresRom,
    FullCommitRequiresRestart,
    CommitStateUnknown,
    StorageUnsupported,
    FilterUnknown,
    StorageRequiresFilterDisabled,
    StorageRequiresRestart,
    HormUnknown,
    StorageInventoryUnavailable,
    HormRequiresRestart,
    BulkProtectionUnsupported,
    BulkProtectionRequiresRestart,
    ConfigurationNotPersistent,
    StorageVolumeUnavailable,
    HormRequirement
  };
  struct Restriction {
    Issue issue = Issue::None;
    HormIssue horm = HormIssue::None;
    Restriction(Issue value = Issue::None, HormIssue requirement = HormIssue::None) : issue(value), horm(requirement) {}
    [[nodiscard]] bool empty() const { return issue == Issue::None; }
  };
  explicit OverlayMaintenanceEditor(QObject* parent = nullptr) : QObject(parent) {}
  // Explicit publication keeps bulk-protection effects inside the outer update.
  class Update : public EditorUpdate<OverlayMaintenanceEditor> {
   public:
    explicit Update(OverlayMaintenanceEditor& editor);
    Update(Update&& other) noexcept = default;
    Update(const Update&) = delete;
    Update& operator=(const Update&) = delete;
    [[nodiscard]] bool publishSnapshot(const std::function<bool()>& continuePublishing = {});
  };
  [[nodiscard]] Update beginUpdate() { return Update(*this); }
  [[nodiscard]] static std::shared_ptr<OverlayMaintenanceEditor> editDraft(std::shared_ptr<OverlayMaintenanceEditor> source);
  [[nodiscard]] const OverlayMaintenanceSnapshot& snapshot() const { return m_observation->snapshot; }
  [[nodiscard]] bool busy() const { return m_observation->busy; }
  [[nodiscard]] bool hasSnapshot() const { return m_observation->hasSnapshot; }
  [[nodiscard]] const OverlayMaintenanceChanges& draft() const { return m_draft; }
  [[nodiscard]] bool restartRequired() const { return m_observation->restartRequired; }
  [[nodiscard]] bool swapRestartRequired() const;
  [[nodiscard]] bool currentRomLoaded() const;
  [[nodiscard]] bool hormActive() const { return snapshot().hormEnabled == true; }
  [[nodiscard]] bool draftEdited() const;
  [[nodiscard]] bool draftConflicted() const;
  [[nodiscard]] Restriction draftCommitRestriction() const;
  [[nodiscard]] bool canCommitDraft() const;
  bool commitDraft();
  [[nodiscard]] Restriction fullCommitRestriction() const;
  [[nodiscard]] Restriction storageRestriction() const;
  [[nodiscard]] Restriction hormRestriction(bool enabled) const;
  [[nodiscard]] Restriction allVolumesProtectionRestriction(bool enabled) const;
  [[nodiscard]] HormEligibility hormEligibility(bool enabled) const;
  [[nodiscard]] bool hormRestrictionCoveredByChecklist() const;
  [[nodiscard]] bool canFullCommit() const { return fullCommitRestriction().empty(); }
  [[nodiscard]] bool canCreateSwap() const { return storageRestriction().empty(); }
  [[nodiscard]] bool canSetHorm(bool enabled) const { return hormRestriction(enabled).empty(); }
  void setInteractionContext(bool elevated, bool writesBlocked);
  bool stageFullCommit();
  bool unstageFullCommit();
  bool stageStorageLocation(const QString& volume);
  bool stageHorm(bool enabled);
  bool stageAllVolumesProtection(bool enabled);
  void discardDraft();
  void replaceDraft(const OverlayMaintenanceChanges& draft);
  void reconcileAppliedDraft(const OverlayMaintenanceChanges& confirmed, const OverlayMaintenanceSnapshot& observed);
  [[nodiscard]] std::optional<std::uint64_t> beginObservation();
  void finishObservation(std::uint64_t generation, OverlayMaintenanceSnapshot observed);
  class BatchUpdate {
   public:
    explicit BatchUpdate(OverlayMaintenanceEditor& editor);
    BatchUpdate(BatchUpdate&& other) noexcept;
    BatchUpdate(const BatchUpdate&) = delete;
    BatchUpdate& operator=(const BatchUpdate&) = delete;
    ~BatchUpdate();

   private:
    QPointer<OverlayMaintenanceEditor> m_editor;
  };
  [[nodiscard]] BatchUpdate beginBatch() { return BatchUpdate(*this); }
 signals:
  void stateChanged();
  void busyChanged();
  void pendingChanged();
  // Semantic target changes are immediate, also inside notification batches.
  void bulkProtectionChanged(bool resetOverrides);
  void refreshFinished(bool succeeded);

 private:
  struct Observation {
    OverlayMaintenanceSnapshot snapshot;
    bool hasSnapshot = false;
    bool busy = false;
    bool elevated = false;
    bool writesBlocked = false;
    bool restartRequired = false;
    bool fullCommitObserved = false;
    bool swapConfigured = false;
  };
  explicit OverlayMaintenanceEditor(std::shared_ptr<OverlayMaintenanceEditor> source);
  void rebaseDraft();
  [[nodiscard]] Restriction commonRestriction() const;
  [[nodiscard]] bool storageVolumeEligible(const QString& volume) const;
  [[nodiscard]] static bool storageMatches(const OverlayMaintenanceSnapshot& observed, const QString& volume);
  void adoptObservation(OverlayMaintenanceSnapshot observed, bool reconcileFullCommit);
  void publishDraft(const OverlayMaintenanceChanges& before);
  void setBusy(bool busy);
  void notifyDraftChanged();
  void finishBatch();
  std::shared_ptr<Observation> m_observation = std::make_shared<Observation>();
  std::shared_ptr<OverlayMaintenanceEditor> m_source;
  OverlayMaintenanceChanges m_origin;
  OverlayMaintenanceChanges m_draft;
  std::uint64_t m_observationGeneration = 0;
  int m_batchDepth = 0;
  bool m_batchChanged = false;
};
}  // namespace uwf::app
