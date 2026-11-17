// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <vector>

#include "application/configuration/contracts/ConfigurationPlan.h"
#include "application/configuration/editing/EditorUpdate.h"
#include "application/configuration/overlay/OverlayPolicyConstraints.h"
#include "application/configuration/overlay/OverlayPolicyDraft.h"
#include "domain/uwf/model/UwfModel.h"

namespace uwf::app {
class OverlayMaintenanceEditor;

enum class OverlayPolicyField { Passthrough, Persistent, Reset, ReadOnlyMedia };
inline constexpr std::array overlayPolicyFields{OverlayPolicyField::Passthrough, OverlayPolicyField::Persistent, OverlayPolicyField::Reset,
                                                OverlayPolicyField::ReadOnlyMedia};
struct OverlayPolicyEdit {
  std::uint64_t generation = 0;
  // Derived presentation; only intent is merged back into the shared draft.
  OverlayPolicyDelta values;
  OverlayPolicyDelta intent;
  // A modal transaction projects its private maintenance choices without
  // changing the shared editor's context.
  OverlayMaintenanceChanges maintenance;
};

// Editing is independent of transport and translation. Modal transactions use
// commit(); command batches use importChanges(). Both honor the same admission.
class OverlayPolicyEditor final : public QObject {
  Q_OBJECT
 public:
  explicit OverlayPolicyEditor(QObject* parent = nullptr) : QObject(parent) {}
  // Cross-editor updates are silent until the owner resumes and publishes them.
  class Update : public EditorUpdate<OverlayPolicyEditor> {
   public:
    explicit Update(OverlayPolicyEditor& editor);
    Update(Update&& other) noexcept = default;
    Update(const Update&) = delete;
    Update& operator=(const Update&) = delete;
    [[nodiscard]] bool publishSnapshot(const std::function<bool()>& continuePublishing = {});
    [[nodiscard]] bool publishAvailability(const std::function<bool()>& continuePublishing = {});

   private:
    bool m_editingAllowed;
  };
  [[nodiscard]] Update beginUpdate() { return Update(*this); }
  void reset(OverlayPolicySnapshot policy, const core::UwfSnapshot& snapshot, std::optional<bool> hormEnabled = {});
  void restoreChanges(const OverlayPolicyDelta& changes);
  void setOverlayChanges(core::OverlayConfigDelta changes);
  void setMaintenanceEditor(std::shared_ptr<OverlayMaintenanceEditor> maintenance);
  void setControlsEnabled(bool enabled);
  [[nodiscard]] OverlayPolicyDelta pendingChanges() const;
  [[nodiscard]] const OverlayPolicySnapshot& baseline() const { return m_draft.baseline(); }
  [[nodiscard]] bool available() const { return baseline().transportAvailable; }
  [[nodiscard]] bool readOnly() const { return !m_snapshot.elevated; }
  [[nodiscard]] bool editingAllowed() const;
  [[nodiscard]] core::OverlayType targetType() const { return targetType(beginEdit()); }
  [[nodiscard]] core::OverlayType targetType(const OverlayPolicyEdit& edit) const;
  [[nodiscard]] core::OverlayConfig targetOverlay(const OverlayPolicyEdit& edit) const;
  [[nodiscard]] OverlayPolicyEdit beginEdit(bool baselineOnly = false) const;
  [[nodiscard]] OverlayPolicyEdit projectEdit(OverlayPolicyEdit edit, const OverlayMaintenanceChanges& maintenance) const;
  [[nodiscard]] bool changed(const OverlayPolicyEdit& edit, OverlayPolicyField field) const;
  [[nodiscard]] std::optional<bool> current(OverlayPolicyField field) const;
  [[nodiscard]] static std::optional<bool> value(const OverlayPolicyDelta& values, OverlayPolicyField field);
  static void setValue(OverlayPolicyDelta& values, OverlayPolicyField field, std::optional<bool> value);
  static void setValue(OverlayPolicyEdit& edit, OverlayPolicyField field, bool value);
  [[nodiscard]] int pendingCount(const OverlayPolicyEdit& edit) const;
  [[nodiscard]] OverlayPolicyIssue disabledReason(const OverlayPolicyEdit& edit, OverlayPolicyField field) const;
  [[nodiscard]] std::vector<OverlayPolicyIssue> conflicts(const OverlayPolicyEdit& edit) const;
  [[nodiscard]] bool canCommit(const OverlayPolicyEdit& edit) const;
  bool commit(const OverlayPolicyEdit& edit);
  class BatchUpdate {
   public:
    explicit BatchUpdate(OverlayPolicyEditor& editor);
    BatchUpdate(BatchUpdate&& other) noexcept;
    BatchUpdate(const BatchUpdate&) = delete;
    BatchUpdate& operator=(const BatchUpdate&) = delete;
    ~BatchUpdate();

   private:
    QPointer<OverlayPolicyEditor> m_editor;
  };
  [[nodiscard]] BatchUpdate beginBatch() { return BatchUpdate(*this); }
  // Import stages a batch whose cross-field policy is validated by the final
  // configuration plan. Admission and required observations still apply here.
  bool importChanges(const OverlayPolicyDelta& changes);

 signals:
  void stateChanged();
  void pendingChanged();

 private:
  void notifyDraftChanged();
  void finishBatch();
  [[nodiscard]] ConfigurationPlan plan(const OverlayPolicyEdit& edit, bool includeIntent = true) const;
  [[nodiscard]] OverlayPolicySnapshot structuralBaseline(const OverlayPolicyEdit& edit) const;
  [[nodiscard]] OverlayPolicyDelta pendingChanges(const OverlayPolicyEdit& edit) const;
  OverlayPolicyDraft m_draft;
  core::UwfSnapshot m_snapshot;
  std::optional<bool> m_hormEnabled;
  core::OverlayConfigDelta m_overlayChanges;
  std::shared_ptr<OverlayMaintenanceEditor> m_maintenance;
  std::uint64_t m_generation = 0;
  bool m_controlsEnabled = true;
  int m_batchDepth = 0;
  bool m_batchChanged = false;
};
}  // namespace uwf::app
