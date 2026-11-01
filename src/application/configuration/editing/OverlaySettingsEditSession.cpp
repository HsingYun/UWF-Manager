// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "application/configuration/editing/OverlaySettingsEditSession.h"

#include <QPointer>
#include <QSignalBlocker>
#include <algorithm>
#include <stdexcept>
#include <utility>

namespace uwf::app {

OverlaySettingsEditSession::OverlaySettingsEditSession(std::shared_ptr<OverlayPolicyEditor> policy, std::shared_ptr<OverlayMaintenanceEditor> maintenance)
    : m_policy(std::move(policy)),
      m_destinationMaintenance(std::move(maintenance)),
      m_maintenance(m_destinationMaintenance ? OverlayMaintenanceEditor::editDraft(m_destinationMaintenance) : nullptr) {
  if (!m_policy) throw std::invalid_argument("Overlay settings editing requires a policy editor");
  if (m_destinationMaintenance) m_policy->setMaintenanceEditor(m_destinationMaintenance);
  const auto source = m_policy->beginEdit();
  resetPolicyEdit(source, source.intent);
  connect(m_policy.get(), &OverlayPolicyEditor::stateChanged, this, &OverlaySettingsEditSession::rebasePolicy);
  if (m_maintenance) connect(m_maintenance.get(), &OverlayMaintenanceEditor::stateChanged, this, &OverlaySettingsEditSession::stateChanged);
}

OverlayPolicyEdit OverlaySettingsEditSession::policyEdit() const { return policyCandidate().edit; }
bool OverlaySettingsEditSession::policyEdited() const { return m_intent != m_sourceIntent; }
bool OverlaySettingsEditSession::edited() const { return policyEdited() || (m_maintenance && m_maintenance->draftEdited()); }
int OverlaySettingsEditSession::pendingCount() const {
  return m_policy->pendingCount(policyEdit()) + (m_maintenance ? static_cast<int>(m_maintenance->draft().count()) : 0);
}
OverlaySettingsEditSession::PolicyCandidate OverlaySettingsEditSession::policyCandidate() const {
  auto merged = m_policy->beginEdit();
  bool conflicted = policyEdited() && m_generation != merged.generation;
  if (conflicted) merged.generation = m_generation;
  for (const auto field : overlayPolicyFields) {
    const auto origin = OverlayPolicyEditor::value(m_sourceIntent, field);
    const auto edited = OverlayPolicyEditor::value(m_intent, field);
    if (origin == edited) continue;
    const auto current = OverlayPolicyEditor::value(merged.intent, field);
    if (current != origin && current != edited) conflicted = true;
    OverlayPolicyEditor::setValue(merged.intent, field, edited);
  }
  const auto maintenance = m_maintenance ? m_maintenance->draft() : merged.maintenance;
  return {m_policy->projectEdit(std::move(merged), maintenance), conflicted};
}
std::vector<OverlaySettingsEditSession::Conflict> OverlaySettingsEditSession::conflicts() const {
  const auto candidate = policyCandidate();
  std::vector<Conflict> result;
  const auto policyIssues = m_policy->conflicts(candidate.edit);
  for (const auto issue : policyIssues) result.emplace_back(issue);
  if (candidate.conflicted && std::find(policyIssues.begin(), policyIssues.end(), OverlayPolicyIssue::StaleEdit) == policyIssues.end())
    result.emplace_back(OverlayPolicyIssue::StaleEdit);
  if (m_maintenance) {
    const auto restriction = m_maintenance->draftCommitRestriction();
    if (!restriction.empty()) result.emplace_back(restriction);
  }
  return result;
}
bool OverlaySettingsEditSession::canCommit() const {
  if (!edited() || (m_maintenance && (m_maintenance->busy() || !m_maintenance->canCommitDraft()))) return false;
  const auto candidate = policyCandidate();
  return !candidate.conflicted && m_policy->conflicts(candidate.edit).empty() && (!policyEdited() || m_policy->canCommit(candidate.edit));
}
void OverlaySettingsEditSession::setPolicyValue(const OverlayPolicyField field, const bool enabled) {
  syncPolicy();
  const auto current = policyEdit();
  if (m_policy->disabledReason(current, field) != OverlayPolicyIssue::None) return;
  const auto previous = OverlayPolicyEditor::value(current.values, field);
  if (previous == enabled) {
    // Explicitly choosing an already displayed value still carries intent.
    clearPolicyEditBase(field);
  } else if (!OverlayPolicyEditor::value(m_editBase.values, field).has_value()) {
    OverlayPolicyEditor::setValue(m_editBase.values, field, previous);
    OverlayPolicyEditor::setValue(m_editBase.intent, field, OverlayPolicyEditor::value(current.intent, field));
  }
  OverlayPolicyEditor::setValue(m_intent, field, enabled);
  if (previous != enabled && OverlayPolicyEditor::value(m_editBase.values, field) == enabled) {
    auto inherited = current;
    inherited.intent = {};
    inherited = m_policy->projectEdit(std::move(inherited), current.maintenance);
    if (OverlayPolicyEditor::value(inherited.values, field) == enabled) {
      OverlayPolicyEditor::setValue(m_intent, field, OverlayPolicyEditor::value(m_editBase.intent, field));
      clearPolicyEditBase(field);
    }
  }
  emit stateChanged();
}
void OverlaySettingsEditSession::revert() {
  if (m_maintenance) {
    const QSignalBlocker blocker(m_maintenance.get());
    m_maintenance->discardDraft();
  }
  resetPolicyEdit(m_policy->beginEdit(), {});
  emit stateChanged();
}
void OverlaySettingsEditSession::resetPolicyEdit(const OverlayPolicyEdit& source, OverlayPolicyDelta intent) {
  m_generation = source.generation;
  m_sourceIntent = source.intent;
  m_intent = std::move(intent);
  m_editBase = {};
}
void OverlaySettingsEditSession::clearPolicyEditBase(const OverlayPolicyField field) {
  OverlayPolicyEditor::setValue(m_editBase.values, field, std::nullopt);
  OverlayPolicyEditor::setValue(m_editBase.intent, field, std::nullopt);
}
void OverlaySettingsEditSession::syncPolicy() {
  const auto current = m_policy->beginEdit();
  if (current.generation != m_generation) {
    if (!policyEdited()) resetPolicyEdit(current, current.intent);
    return;
  }
  for (const auto field : overlayPolicyFields) {
    const auto origin = OverlayPolicyEditor::value(m_sourceIntent, field);
    const auto shared = OverlayPolicyEditor::value(current.intent, field);
    if (shared == origin) continue;
    const auto edited = OverlayPolicyEditor::value(m_intent, field);
    if (edited == origin || edited == shared) {
      OverlayPolicyEditor::setValue(m_intent, field, shared);
      OverlayPolicyEditor::setValue(m_sourceIntent, field, shared);
      clearPolicyEditBase(field);
    }
  }
}
void OverlaySettingsEditSession::rebasePolicy() {
  syncPolicy();
  emit stateChanged();
}
bool OverlaySettingsEditSession::commit() {
  if (!canCommit()) return false;
  const auto policy = m_policy;
  const auto maintenance = m_maintenance;
  const auto destination = m_destinationMaintenance;
  const auto merged = policyCandidate().edit;
  const bool policyChanged = merged.intent != policy->beginEdit().intent;
  const QPointer self(this);
  {
    // Validate both drafts before mutation and publish only their complete state.
    const auto policyBatch = policy->beginBatch();
    const auto maintenanceBatch = destination ? std::optional{destination->beginBatch()} : std::nullopt;
    const QSignalBlocker editBlock(this);
    if (policyChanged && !policy->commit(merged)) return false;
    if (maintenance && !maintenance->commitDraft()) return false;
    const auto source = policy->beginEdit();
    resetPolicyEdit(source, source.intent);
  }
  if (self) emit stateChanged();
  return true;
}

}  // namespace uwf::app

#include "moc_OverlaySettingsEditSession.cpp"
