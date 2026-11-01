// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QObject>
#include <cstdint>
#include <memory>
#include <variant>
#include <vector>

#include "application/configuration/editing/OverlayMaintenanceEditor.h"
#include "application/configuration/overlay/OverlayPolicyEditor.h"

namespace uwf::app {

// Isolated settings edits merge into the shared drafts only when committed.
class OverlaySettingsEditSession final : public QObject {
  Q_OBJECT
 public:
  using Conflict = std::variant<OverlayPolicyIssue, OverlayMaintenanceEditor::Restriction>;
  OverlaySettingsEditSession(std::shared_ptr<OverlayPolicyEditor> policy, std::shared_ptr<OverlayMaintenanceEditor> maintenance = {});
  [[nodiscard]] OverlayPolicyEdit policyEdit() const;
  [[nodiscard]] const std::shared_ptr<OverlayMaintenanceEditor>& maintenance() const { return m_maintenance; }
  [[nodiscard]] bool edited() const;
  [[nodiscard]] int pendingCount() const;
  [[nodiscard]] bool canCommit() const;
  [[nodiscard]] std::vector<Conflict> conflicts() const;
  void setPolicyValue(OverlayPolicyField field, bool enabled);
  void revert();
  bool commit();

 signals:
  void stateChanged();

 private:
  struct PolicyCandidate {
    OverlayPolicyEdit edit;
    bool conflicted = false;
  };
  struct PolicyEditBase {
    OverlayPolicyDelta values;
    OverlayPolicyDelta intent;
  };
  [[nodiscard]] bool policyEdited() const;
  [[nodiscard]] PolicyCandidate policyCandidate() const;
  void resetPolicyEdit(const OverlayPolicyEdit& source, OverlayPolicyDelta intent);
  void clearPolicyEditBase(OverlayPolicyField field);
  void syncPolicy();
  void rebasePolicy();
  std::shared_ptr<OverlayPolicyEditor> m_policy;
  std::shared_ptr<OverlayMaintenanceEditor> m_destinationMaintenance;
  std::shared_ptr<OverlayMaintenanceEditor> m_maintenance;
  std::uint64_t m_generation = 0;
  OverlayPolicyDelta m_sourceIntent;
  OverlayPolicyDelta m_intent;
  // Each field starts a local edit at its first toggle. This interaction base
  // is separate from the shared intent used for three-way merging.
  PolicyEditBase m_editBase;
};

}  // namespace uwf::app
