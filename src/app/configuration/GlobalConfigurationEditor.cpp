// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "GlobalConfigurationEditor.h"

#include <algorithm>

namespace uwf::app {
namespace {
constexpr int kEditorMaximum = std::numeric_limits<int>::max();
uint32_t editorValue(const uint32_t value) { return std::min(value, static_cast<uint32_t>(kEditorMaximum)); }
uint32_t bounded(const uint32_t value, const GlobalConfigurationEditor::Range range) {
  return std::clamp(value, static_cast<uint32_t>(range.minimum), static_cast<uint32_t>(range.maximum));
}
int warningThresholdMaximum(const uint32_t criticalThreshold) {
  return criticalThreshold == 0 || criticalThreshold > static_cast<uint32_t>(kEditorMaximum) ? kEditorMaximum
                                                                                             : app::strictThresholdUpper(static_cast<int>(criticalThreshold));
}
}  // namespace

GlobalConfigurationEditor::GlobalConfigurationEditor(const uint32_t totalRamMb, QObject* parent)
    : GlobalConfigurationEditor(std::make_shared<app::GlobalConfigurationState>(), totalRamMb, parent) {}

GlobalConfigurationEditor::GlobalConfigurationEditor(std::shared_ptr<app::GlobalConfigurationState> state, const uint32_t totalRamMb, QObject* parent)
    : QObject(parent), m_state(std::move(state)), m_draft(m_state->draft), m_totalRamMb(totalRamMb), m_available(m_state->available) {}

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
void GlobalConfigurationEditor::reset(const core::SessionSnapshot& next) {
  m_draft.baselineFilter = next.filter;
  m_draft.baselineOverlay = next.overlay;
  m_draft.filterEnabled = next.filter.enabled;
  m_draft.overlay = next.overlay;
  updateRanges();
  setAvailable(true);
  emit stateChanged();
}

void GlobalConfigurationEditor::setAvailable(const bool available) {
  if (m_available == available) return;
  m_available = available;
  emit availabilityChanged();
}

void GlobalConfigurationEditor::updateRanges() {
  m_rangesNormalized = true;
  const auto bounds = app::overlaySizeBounds(m_draft.overlay.type, m_totalRamMb, kEditorMaximum);
  const int maximumSize = static_cast<int>(editorValue(m_draft.overlay.maximumSizeMb));
  // Reading a system baseline must not turn editor limits into pending writes.
  m_maximumRange = {std::min(bounds.minimum, maximumSize), std::max(bounds.maximum, maximumSize)};
  m_criticalRange = {0, kEditorMaximum};
  m_warningRange = {0, warningThresholdMaximum(m_draft.overlay.criticalThresholdMb)};
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
  emit stateChanged();
  emit pendingChanged();
}

void GlobalConfigurationEditor::editFilterEnabled(const bool value) {
  m_draft.filterEnabled = value;
  publishEdit();
}

void GlobalConfigurationEditor::editOverlayType(const core::OverlayType value) {
  m_draft.overlay.type = value;
  normalize();
  publishEdit();
}

void GlobalConfigurationEditor::editMaximumSize(const int value) {
  m_draft.overlay.maximumSizeMb = static_cast<uint32_t>(std::max(0, value));
  normalize();
  publishEdit();
}

void GlobalConfigurationEditor::editWarningThreshold(const int value) {
  m_draft.overlay.warningThresholdMb = static_cast<uint32_t>(std::max(0, value));
  normalize();
  publishEdit();
}

void GlobalConfigurationEditor::editCriticalThreshold(const int value) {
  m_draft.overlay.criticalThresholdMb = static_cast<uint32_t>(std::max(0, value));
  normalize();
  publishEdit();
}

std::optional<bool> GlobalConfigurationEditor::pendingFilterEnabled() const { return m_available ? m_draft.pendingFilterEnabled() : std::nullopt; }

core::OverlayConfigDelta GlobalConfigurationEditor::pendingOverlay() const { return m_available ? m_draft.pendingOverlay() : core::OverlayConfigDelta{}; }

bool GlobalConfigurationEditor::importFilterEnabled(const bool value) {
  if (!m_available || m_draft.filterEnabled == value) return false;
  editFilterEnabled(value);
  return true;
}

bool GlobalConfigurationEditor::importOverlayType(const core::OverlayType value) {
  if (!m_available || m_draft.overlay.type == value) return false;
  if (value == core::OverlayType::RAM || value == core::OverlayType::Disk) m_draft.overlay.type = value;
  publishEdit();
  return true;
}

bool GlobalConfigurationEditor::importNumber(const uint32_t value, uint32_t& target, Range& range) {
  const auto imported = editorValue(value);
  if (!m_available || target == value || target == imported) return false;
  range = {0, kEditorMaximum};
  m_rangesNormalized = false;
  target = imported;
  publishEdit();
  return true;
}

bool GlobalConfigurationEditor::importOverlayMaxMb(const uint32_t value) { return importNumber(value, m_draft.overlay.maximumSizeMb, m_maximumRange); }
bool GlobalConfigurationEditor::importOverlayWarnMb(const uint32_t value) { return importNumber(value, m_draft.overlay.warningThresholdMb, m_warningRange); }
bool GlobalConfigurationEditor::importOverlayCritMb(const uint32_t value) { return importNumber(value, m_draft.overlay.criticalThresholdMb, m_criticalRange); }

void GlobalConfigurationEditor::finishImport() {
  if (!m_available) return;
  normalize();
  publishEdit();
}

}  // namespace uwf::app

#include "moc_GlobalConfigurationEditor.cpp"
