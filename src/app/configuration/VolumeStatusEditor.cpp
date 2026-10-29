// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "VolumeStatusEditor.h"

namespace uwf::app {

VolumeStatusEditor::VolumeStatusEditor(QObject* parent) : VolumeStatusEditor(std::make_shared<app::VolumeConfigurationState>(), parent) {}

VolumeStatusEditor::VolumeStatusEditor(std::shared_ptr<app::VolumeConfigurationState> state, QObject* parent)
    : QObject(parent), m_state(std::move(state)), m_draft(m_state->draft) {}

void VolumeStatusEditor::setControlsEnabled(bool enabled) {
  if (m_controlsEnabled == enabled) return;
  m_controlsEnabled = enabled;
  emit availabilityChanged();
}
void VolumeStatusEditor::reset(const core::VolumeRecord* current, const core::VolumeRecord* next, const bool available) {
  m_currentProtection = available && current ? std::optional<bool>(current->isProtected) : std::nullopt;
  m_controlsEnabled = true;
  m_draft.supported = true;
  m_draft.hasVolume = current || next;
  m_draft.baselineProtected = next ? next->isProtected : (current ? current->isProtected : false);
  const auto* binding = next ? next : current;
  m_draft.baselineBindByVolumeName = binding ? !binding->bindByDriveLetter : false;
  // Missing records leave the disabled editors' last displayed values intact.
  if (m_draft.hasVolume) {
    m_draft.protectedVolume = m_draft.baselineProtected;
    m_draft.bindByVolumeName = m_draft.baselineBindByVolumeName;
  }
  emit stateChanged();
  emit availabilityChanged();
}

void VolumeStatusEditor::setUnsupported() {
  if (!m_draft.supported && !m_draft.hasVolume) return;
  m_currentProtection.reset();
  m_draft.supported = false;
  m_draft.hasVolume = false;
  emit stateChanged();
  emit availabilityChanged();
}

bool VolumeStatusEditor::editProtection(const bool value) {
  if (!m_draft.supported || !m_draft.hasVolume || m_draft.protectedVolume == value) return false;
  m_draft.protectedVolume = value;
  emit stateChanged();
  emit pendingChanged();
  return true;
}

bool VolumeStatusEditor::editBinding(const bool value) {
  if (!m_draft.supported || !m_draft.hasVolume || m_draft.bindByVolumeName == value) return false;
  m_draft.bindByVolumeName = value;
  emit stateChanged();
  emit pendingChanged();
  return true;
}

}  // namespace uwf::app

#include "moc_VolumeStatusEditor.cpp"
