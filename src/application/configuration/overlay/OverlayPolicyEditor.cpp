// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "application/configuration/overlay/OverlayPolicyEditor.h"

#include <QPointer>
#include <stdexcept>
#include <utility>

#include "application/configuration/editing/OverlayMaintenanceEditor.h"
#include "application/uwf/contracts/UwfMaintenanceRepository.h"

namespace uwf::app {
namespace {
template <typename T>
bool known(const PolicyObservation<T>& field) {
  return field.status == PolicyObservationStatus::Available && field.value.has_value();
}
std::optional<bool> flag(const PolicyObservation<OverlayFlags>& observation, const OverlayPolicyField field) {
  if (!known(observation)) return std::nullopt;
  switch (field) {
    case OverlayPolicyField::Passthrough:
      return observation.value->passthrough();
    case OverlayPolicyField::Persistent:
      return observation.value->persistent();
    case OverlayPolicyField::ReadOnlyMedia:
      return observation.value->readOnlyMedia();
    default:
      return std::nullopt;
  }
}
}  // namespace

OverlayPolicyEditor::Update::Update(OverlayPolicyEditor& editor) : EditorUpdate(editor), m_editingAllowed(editor.editingAllowed()) {}
bool OverlayPolicyEditor::Update::publishSnapshot(const std::function<bool()>& continuePublishing) {
  return publish(continuePublishing, [&] { emit m_editor->stateChanged(); }, [&] { emit m_editor->pendingChanged(); });
}
bool OverlayPolicyEditor::Update::publishAvailability(const std::function<bool()>& continuePublishing) {
  if (!current(continuePublishing)) return false;
  if (m_editingAllowed != m_editor->editingAllowed()) return publish(continuePublishing, [&] { emit m_editor->stateChanged(); });
  return current(continuePublishing);
}

void OverlayPolicyEditor::reset(OverlayPolicySnapshot policy, const core::UwfSnapshot& snapshot, const std::optional<bool> hormEnabled) {
  ++m_generation;
  m_draft.reset(std::move(policy));
  m_snapshot = snapshot;
  m_hormEnabled = hormEnabled;
  m_overlayChanges = {};
  notifyDraftChanged();
}
void OverlayPolicyEditor::restoreChanges(const OverlayPolicyDelta& changes) {
  m_draft.replace(changes);
  notifyDraftChanged();
}
void OverlayPolicyEditor::setOverlayChanges(core::OverlayConfigDelta changes) {
  m_overlayChanges = std::move(changes);
  emit stateChanged();
}
void OverlayPolicyEditor::setMaintenanceEditor(std::shared_ptr<OverlayMaintenanceEditor> maintenance) {
  if (m_maintenance == maintenance) return;
  if (m_maintenance) throw std::invalid_argument("Overlay policy already belongs to a maintenance edit context");
  m_maintenance = std::move(maintenance);
  if (m_maintenance) connect(m_maintenance.get(), &OverlayMaintenanceEditor::stateChanged, this, &OverlayPolicyEditor::stateChanged);
  emit stateChanged();
}
void OverlayPolicyEditor::setControlsEnabled(const bool enabled) {
  if (m_controlsEnabled == enabled) return;
  m_controlsEnabled = enabled;
  emit stateChanged();
}
void OverlayPolicyEditor::notifyDraftChanged() {
  if (m_batchDepth > 0) {
    m_batchChanged = true;
    return;
  }
  const QPointer guard(this);
  emit stateChanged();
  if (guard) emit pendingChanged();
}
OverlayPolicyEditor::BatchUpdate::BatchUpdate(OverlayPolicyEditor& editor) : m_editor(&editor) { ++editor.m_batchDepth; }
OverlayPolicyEditor::BatchUpdate::BatchUpdate(BatchUpdate&& other) noexcept : m_editor(std::exchange(other.m_editor, {})) {}
OverlayPolicyEditor::BatchUpdate::~BatchUpdate() {
  if (m_editor) m_editor->finishBatch();
}
void OverlayPolicyEditor::finishBatch() {
  if (--m_batchDepth == 0 && std::exchange(m_batchChanged, false)) notifyDraftChanged();
}

std::optional<bool> OverlayPolicyEditor::value(const OverlayPolicyDelta& values, const OverlayPolicyField field) {
  switch (field) {
    case OverlayPolicyField::Passthrough:
      return values.passthrough;
    case OverlayPolicyField::Persistent:
      return values.persistent;
    case OverlayPolicyField::ReadOnlyMedia:
      return values.readOnlyMedia;
    case OverlayPolicyField::Reset:
      if (values.reset == PersistentReset::Clear) return true;
      if (values.reset == PersistentReset::Preserve) return false;
      return std::nullopt;
  }
  return std::nullopt;
}
void OverlayPolicyEditor::setValue(OverlayPolicyDelta& values, const OverlayPolicyField field, const std::optional<bool> enabled) {
  switch (field) {
    case OverlayPolicyField::Passthrough:
      values.passthrough = enabled;
      break;
    case OverlayPolicyField::Persistent:
      values.persistent = enabled;
      break;
    case OverlayPolicyField::ReadOnlyMedia:
      values.readOnlyMedia = enabled;
      break;
    case OverlayPolicyField::Reset:
      values.reset = enabled ? std::optional{*enabled ? PersistentReset::Clear : PersistentReset::Preserve} : std::nullopt;
      break;
  }
}
void OverlayPolicyEditor::setValue(OverlayPolicyEdit& edit, const OverlayPolicyField field, const bool enabled) {
  setValue(edit.intent, field, enabled);
  setValue(edit.values, field, enabled);
}
ConfigurationPlan OverlayPolicyEditor::plan(const OverlayPolicyEdit& edit, const bool includeIntent) const {
  ConfigurationPlan result;
  result.snapshot = m_snapshot;
  result.policySnapshot = baseline();
  result.hasPolicyContext = baseline().transportAvailable;
  result.changes.setOverlay = m_overlayChanges;
  result.maintenance = edit.maintenance;
  if (m_maintenance && m_maintenance->hasSnapshot()) result.maintenanceSnapshot = m_maintenance->snapshot();
  if (includeIntent) result.overlayPolicy = pendingChanges(edit);
  return result;
}
OverlayPolicySnapshot OverlayPolicyEditor::structuralBaseline(const OverlayPolicyEdit& edit) const {
  const auto context = plan(edit, false);
  auto projected = configurationOverlayBaseline(context);
  const auto target = configurationOverlayTarget(context);
  projected.next.overlayType = {PolicyObservationStatus::Available, static_cast<std::int64_t>(target.overlay.type)};
  if (target.flags) projected.next.flags = {PolicyObservationStatus::Available, target.flags};
  return projected;
}
OverlayPolicyDelta OverlayPolicyEditor::pendingChanges(const OverlayPolicyEdit& edit) const {
  return overlayPolicyChanges(structuralBaseline(edit), edit.intent);
}
OverlayPolicyDelta OverlayPolicyEditor::pendingChanges() const { return pendingChanges(beginEdit()); }
core::OverlayType OverlayPolicyEditor::targetType(const OverlayPolicyEdit& edit) const { return targetOverlay(edit).type; }
core::OverlayConfig OverlayPolicyEditor::targetOverlay(const OverlayPolicyEdit& edit) const { return configurationOverlayTarget(plan(edit)).overlay; }
OverlayPolicyEdit OverlayPolicyEditor::projectEdit(OverlayPolicyEdit edit, const OverlayMaintenanceChanges& maintenance) const {
  edit.maintenance = maintenance;
  edit.values = overlayPolicyValues(structuralBaseline(edit), edit.intent);
  return edit;
}
OverlayPolicyEdit OverlayPolicyEditor::beginEdit(const bool baselineOnly) const {
  OverlayPolicyEdit edit;
  edit.generation = m_generation;
  if (!baselineOnly) edit.intent = m_draft.intent();
  return projectEdit(std::move(edit), m_maintenance ? m_maintenance->draft() : OverlayMaintenanceChanges{});
}
std::optional<bool> OverlayPolicyEditor::current(const OverlayPolicyField field) const {
  if (field == OverlayPolicyField::Reset) {
    // A reset request affects the next startup, not current overlay contents.
    return known(baseline().resetPersistentState) && baseline().resetMode() && baseline().resetMode() != PersistentReset::UseSavedMode ? std::optional{false}
                                                                                                                                       : std::nullopt;
  }
  return flag(baseline().current.flags, field);
}
int OverlayPolicyEditor::pendingCount(const OverlayPolicyEdit& edit) const {
  const auto changes = pendingChanges(edit);
  int count = 0;
  for (const auto field : overlayPolicyFields)
    if (value(changes, field)) ++count;
  return count;
}
bool OverlayPolicyEditor::changed(const OverlayPolicyEdit& edit, const OverlayPolicyField field) const {
  return value(pendingChanges(edit), field).has_value();
}
OverlayPolicyIssue OverlayPolicyEditor::disabledReason(const OverlayPolicyEdit& edit, const OverlayPolicyField field) const {
  if (!m_snapshot.uwfAvailable) return OverlayPolicyIssue::FlagsUnavailable;
  if (readOnly()) return OverlayPolicyIssue::AdministratorRequired;
  if (!hormAllowsChanges(m_hormEnabled)) return OverlayPolicyIssue::HormActive;
  if (!m_controlsEnabled) return OverlayPolicyIssue::WritesBlocked;
  if (edit.generation != m_generation) return OverlayPolicyIssue::StaleEdit;
  const auto enabled = value(edit.values, field);
  const auto type = targetType(edit);
  if (field == OverlayPolicyField::Reset) {
    if (baseline().resetPersistentState.status == PolicyObservationStatus::Unsupported) return OverlayPolicyIssue::ResetUnsupported;
    if (!value(beginEdit(true).values, field)) return OverlayPolicyIssue::ResetUnknown;
    const auto issue = overlayResetCurrentIssue(baseline());
    if (issue != OverlayPolicyIssue::None) return issue;
    if (enabled == true) return OverlayPolicyIssue::None;
    if (enabled != true && (type != core::OverlayType::Disk || edit.values.persistent != true)) return OverlayPolicyIssue::NextPersistentRequired;
  } else {
    if (!enabled) return OverlayPolicyIssue::FlagsUnavailable;
    const auto issue = overlayCurrentFilterOffIssue(baseline());
    if (issue != OverlayPolicyIssue::None) return issue;
    if (field == OverlayPolicyField::Persistent) {
      if (*enabled && edit.values.reset == PersistentReset::Clear) return OverlayPolicyIssue::NextPersistentRequired;
      if (!*enabled && type != core::OverlayType::Disk) return OverlayPolicyIssue::PersistentNeedsDisk;
    }
    if (field == OverlayPolicyField::ReadOnlyMedia && !*enabled) {
      if (type != core::OverlayType::RAM) return OverlayPolicyIssue::MediaNeedsRam;
    }
  }
  return OverlayPolicyIssue::None;
}
std::vector<OverlayPolicyIssue> OverlayPolicyEditor::conflicts(const OverlayPolicyEdit& edit) const {
  if (edit.generation != m_generation) return {OverlayPolicyIssue::StaleEdit};
  std::vector<OverlayPolicyIssue> result;
  for (const auto& conflict : validateOverlayPolicyPlan(plan(edit)).conflicts) result.push_back(conflict.issue);
  return result;
}
bool OverlayPolicyEditor::canCommit(const OverlayPolicyEdit& edit) const {
  if (!editingAllowed() || !conflicts(edit).empty()) return false;
  for (const auto field : overlayPolicyFields)
    if (value(edit.intent, field) && !value(overlayPolicyValues(baseline()), field)) return false;
  return true;
}
bool OverlayPolicyEditor::commit(const OverlayPolicyEdit& edit) {
  if (!canCommit(edit)) return false;
  const auto previous = m_draft.intent();
  m_draft.replace(edit.intent);
  if (previous != m_draft.intent()) notifyDraftChanged();
  return true;
}
bool OverlayPolicyEditor::editingAllowed() const {
  return available() && m_snapshot.uwfAvailable && !readOnly() && m_controlsEnabled && hormAllowsChanges(m_hormEnabled);
}
bool OverlayPolicyEditor::importChanges(const OverlayPolicyDelta& changes) {
  if (!editingAllowed()) return false;
  if ((changes.passthrough || changes.persistent || changes.readOnlyMedia) && !known(baseline().next.flags)) return false;
  if (changes.reset && !known(baseline().resetPersistentState)) return false;
  const auto previous = m_draft.intent();
  m_draft.edit(changes);
  if (previous != m_draft.intent()) notifyDraftChanged();
  return true;
}
}  // namespace uwf::app
#include "moc_OverlayPolicyEditor.cpp"
