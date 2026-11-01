// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "application/configuration/editing/GlobalConfigurationEditor.h"

#include <QPointer>
#include <algorithm>
#include <utility>

#include "application/localization/ApplicationText.h"
#include "application/localization/ConfigurationModeText.h"

namespace uwf::app {
namespace {
constexpr uint32_t kEditorMaximum = std::numeric_limits<uint32_t>::max();
uint32_t editedValue(const int64_t value) { return static_cast<uint32_t>(std::clamp<int64_t>(value, 0, kEditorMaximum)); }
uint32_t bounded(const uint32_t value, const GlobalConfigurationEditor::Range range) { return std::clamp(value, range.minimum, range.maximum); }
uint32_t warningThresholdMaximum(const uint32_t criticalThreshold) {
  return criticalThreshold == 0 ? kEditorMaximum : app::strictThresholdUpper(criticalThreshold);
}
}  // namespace

GlobalConfigurationEditor::GlobalConfigurationEditor(const uint32_t totalRamMb, QObject* parent)
    : GlobalConfigurationEditor(std::make_shared<app::GlobalConfigurationState>(), totalRamMb, parent) {}

GlobalConfigurationEditor::GlobalConfigurationEditor(std::shared_ptr<app::GlobalConfigurationState> state, const uint32_t totalRamMb, QObject* parent)
    : QObject(parent), m_state(std::move(state)), m_draft(m_state->draft), m_totalRamMb(totalRamMb), m_available(m_state->available) {}

GlobalConfigurationEditor::Update::Update(GlobalConfigurationEditor& editor)
    : EditorUpdate(editor), m_controlsEnabled(editor.controlsEnabled()), m_thresholdEditingAllowed(editor.thresholdEditingAllowed()) {}
bool GlobalConfigurationEditor::Update::publishSnapshot(const std::function<bool()>& continuePublishing) {
  return publish(continuePublishing, [&] { emit m_editor->stateChanged(); }, [&] { emit m_editor->availabilityChanged(); });
}
bool GlobalConfigurationEditor::Update::publishAvailability(const std::function<bool()>& continuePublishing) {
  if (!current(continuePublishing)) return false;
  if (m_controlsEnabled != m_editor->controlsEnabled() || m_thresholdEditingAllowed != m_editor->thresholdEditingAllowed())
    return publish(continuePublishing, [&] { emit m_editor->availabilityChanged(); });
  return current(continuePublishing);
}

void GlobalConfigurationEditor::setControlsEnabled(bool enabled) {
  if (m_controlsEnabled == enabled) return;
  m_controlsEnabled = enabled;
  emit availabilityChanged();
}
void GlobalConfigurationEditor::setOverlayConfigLocked(bool locked) {
  if (m_overlayConfigLocked == locked) return;
  m_overlayConfigLocked = locked;
  emit availabilityChanged();
}
void GlobalConfigurationEditor::setMode(core::ConfigurationModeState mode) {
  m_mode = std::move(mode);
  emit availabilityChanged();
}
QString GlobalConfigurationEditor::thresholdRestriction() const {
  return configurationModeRestrictionText(core::configurationWriteRestriction(m_mode, core::ConfigurationWrite::Threshold));
}
void GlobalConfigurationEditor::reset(const core::SessionSnapshot& next) {
  m_draft.baselineFilter = next.filter;
  m_draft.baselineOverlay = next.overlay;
  m_draft.filterEnabled = next.filter.enabled;
  m_draft.overlay = next.overlay;
  updateRanges();
  const QPointer self(this);
  setAvailable(true);
  if (self) emit stateChanged();
}

void GlobalConfigurationEditor::restoreChanges(const std::optional<bool> filterEnabled, const core::OverlayConfigDelta& overlay) {
  if (filterEnabled) m_draft.filterEnabled = *filterEnabled;
  if (overlay.type) m_draft.overlay.type = *overlay.type;
  if (overlay.maximumSizeMb) m_draft.overlay.maximumSizeMb = *overlay.maximumSizeMb;
  if (overlay.warningThresholdMb) m_draft.overlay.warningThresholdMb = *overlay.warningThresholdMb;
  if (overlay.criticalThresholdMb) m_draft.overlay.criticalThresholdMb = *overlay.criticalThresholdMb;
  updateRanges();
  publishEdit();
}

void GlobalConfigurationEditor::setAvailable(const bool available) {
  if (m_available == available) return;
  m_available = available;
  emit availabilityChanged();
}

void GlobalConfigurationEditor::updateRanges() {
  const auto bounds = app::overlaySizeBounds(m_draft.overlay.type, m_totalRamMb, kEditorMaximum);
  const auto maximumSize = m_draft.overlay.maximumSizeMb;
  // Reading a system baseline must not turn editor limits into pending writes.
  m_maximumRange = {std::min(bounds.minimum, maximumSize), std::max(bounds.maximum, maximumSize)};
  m_criticalRange = {0, kEditorMaximum};
  // Presentation must include the observed value even when the system's
  // baseline does not satisfy this editor's stricter relationship policy.
  m_warningRange = {0, std::max(warningThresholdMaximum(m_draft.overlay.criticalThresholdMb), m_draft.overlay.warningThresholdMb)};
}

void GlobalConfigurationEditor::normalize() {
  if (m_draft.overlay.type != m_draft.baselineOverlay.type || m_draft.overlay.maximumSizeMb != m_draft.baselineOverlay.maximumSizeMb) {
    const auto bounds = app::overlaySizeBounds(m_draft.overlay.type, m_totalRamMb, kEditorMaximum);
    m_draft.overlay.maximumSizeMb = bounded(m_draft.overlay.maximumSizeMb, {bounds.minimum, std::max(bounds.minimum, bounds.maximum)});
  }
  if (m_draft.overlay.warningThresholdMb != m_draft.baselineOverlay.warningThresholdMb ||
      m_draft.overlay.criticalThresholdMb != m_draft.baselineOverlay.criticalThresholdMb) {
    m_warningRange = {0, warningThresholdMaximum(m_draft.overlay.criticalThresholdMb)};
    m_draft.overlay.warningThresholdMb = bounded(m_draft.overlay.warningThresholdMb, m_warningRange);
  }
  updateRanges();
}

void GlobalConfigurationEditor::publishEdit() {
  const QPointer self(this);
  emit stateChanged();
  if (self) emit pendingChanged();
}

void GlobalConfigurationEditor::editFilterEnabled(const bool value) {
  if (!editingAllowed()) return;
  m_draft.filterEnabled = value;
  publishEdit();
}

void GlobalConfigurationEditor::editOverlayType(const core::OverlayType value) {
  if (!overlayStorageEditingAllowed()) return;
  m_draft.overlay.type = value;
  normalize();
  publishEdit();
}

void GlobalConfigurationEditor::editMaximumSize(const int64_t value) {
  if (!overlayStorageEditingAllowed()) return;
  m_draft.overlay.maximumSizeMb = editedValue(value);
  normalize();
  publishEdit();
}

void GlobalConfigurationEditor::editWarningThreshold(const int64_t value) {
  if (!thresholdEditingAllowed()) return;
  m_draft.overlay.warningThresholdMb = editedValue(value);
  normalize();
  publishEdit();
}

void GlobalConfigurationEditor::editCriticalThreshold(const int64_t value) {
  if (!thresholdEditingAllowed()) return;
  m_draft.overlay.criticalThresholdMb = editedValue(value);
  normalize();
  publishEdit();
}

std::optional<bool> GlobalConfigurationEditor::pendingFilterEnabled() const { return m_available ? m_draft.pendingFilterEnabled() : std::nullopt; }

core::OverlayConfigDelta GlobalConfigurationEditor::pendingOverlay() const { return m_available ? m_draft.pendingOverlay() : core::OverlayConfigDelta{}; }

QString GlobalConfigurationEditor::overlayStorageRestriction() const {
  if (!editingAllowed()) return ApplicationText::tr("Read-only");
  if (m_overlayConfigLocked)
    return ApplicationText::tr("To change the overlay type or maximum capacity:\n1. Disable UWF.\n2. Restart the device.\n3. Change this setting.");
  return {};
}

GlobalEditResult GlobalConfigurationEditor::importFilterEnabled(const bool value) {
  if (!editingAllowed()) return {GlobalEditOutcome::Rejected, ApplicationText::tr("Read-only")};
  if (m_draft.filterEnabled == value) return {GlobalEditOutcome::Unchanged, {}};
  editFilterEnabled(value);
  return {GlobalEditOutcome::Applied, {}};
}

GlobalEditResult GlobalConfigurationEditor::importOverlayType(const core::OverlayType value) {
  if (!editingAllowed()) return {GlobalEditOutcome::Rejected, ApplicationText::tr("Read-only")};
  if (value != core::OverlayType::RAM && value != core::OverlayType::Disk)
    return {GlobalEditOutcome::Rejected, ApplicationText::tr("Unknown overlay type %1 (expected RAM or Disk)").arg(static_cast<int>(value))};
  if (m_draft.overlay.type == value) return {GlobalEditOutcome::Unchanged, {}};
  if (!overlayStorageEditingAllowed()) return {GlobalEditOutcome::Rejected, overlayStorageRestriction()};
  m_draft.overlay.type = value;
  publishEdit();
  return {GlobalEditOutcome::Applied, {}};
}

GlobalEditResult GlobalConfigurationEditor::importNumber(const uint32_t value, uint32_t& target, Range& range) {
  if (!editingAllowed()) return {GlobalEditOutcome::Rejected, ApplicationText::tr("Read-only")};
  if (target == value) return {GlobalEditOutcome::Unchanged, {}};
  range = {0, kEditorMaximum};
  target = value;
  publishEdit();
  return {GlobalEditOutcome::Applied, {}};
}

GlobalEditResult GlobalConfigurationEditor::importOverlayMaxMb(const uint32_t value) {
  if (!editingAllowed()) return {GlobalEditOutcome::Rejected, ApplicationText::tr("Read-only")};
  if (m_draft.overlay.maximumSizeMb == value) return {GlobalEditOutcome::Unchanged, {}};
  if (!overlayStorageEditingAllowed()) return {GlobalEditOutcome::Rejected, overlayStorageRestriction()};
  return importNumber(value, m_draft.overlay.maximumSizeMb, m_maximumRange);
}
GlobalEditResult GlobalConfigurationEditor::importOverlayWarnMb(const uint32_t value) {
  if (!thresholdEditingAllowed()) return {GlobalEditOutcome::Rejected, thresholdRestriction()};
  return importNumber(value, m_draft.overlay.warningThresholdMb, m_warningRange);
}
GlobalEditResult GlobalConfigurationEditor::importOverlayCritMb(const uint32_t value) {
  if (!thresholdEditingAllowed()) return {GlobalEditOutcome::Rejected, thresholdRestriction()};
  return importNumber(value, m_draft.overlay.criticalThresholdMb, m_criticalRange);
}

void GlobalConfigurationEditor::finishImport() {
  if (!editingAllowed()) return;
  normalize();
  publishEdit();
}

}  // namespace uwf::app

#include "moc_GlobalConfigurationEditor.cpp"
