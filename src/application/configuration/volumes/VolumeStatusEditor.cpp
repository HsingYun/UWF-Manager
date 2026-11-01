// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "application/configuration/volumes/VolumeStatusEditor.h"

#include <QPointer>
#include <utility>

namespace uwf::app {

VolumeStatusEditor::VolumeStatusEditor(QObject* parent) : VolumeStatusEditor(std::make_shared<app::VolumeConfigurationState>(), parent) {}

VolumeStatusEditor::VolumeStatusEditor(std::shared_ptr<app::VolumeConfigurationState> state, QObject* parent)
    : QObject(parent), m_state(std::move(state)), m_draft(m_state->draft) {}

void VolumeStatusEditor::setControlsEnabled(bool enabled) {
  if (m_controlsEnabled == enabled) return;
  m_controlsEnabled = enabled;
  emit availabilityChanged();
}
void VolumeStatusEditor::reset(const core::VolumeRecord* current, const core::VolumeRecord* next, const bool available, const core::VolumeInfo* physical) {
  m_currentProtection = available && current ? std::optional<bool>(current->isProtected) : std::nullopt;
  m_registrationEligible = physical && core::canRegisterVolume(*physical);
  m_draft.supported = true;
  m_draft.hasTarget = available && (current || next || m_registrationEligible);
  // Current membership establishes a target and a current observation, never
  // a Next baseline: removing a registration leaves Current unchanged.
  m_draft.hasConfiguration = next != nullptr;
  m_draft.baselineProtected = next ? next->isProtected : false;
  m_draft.baselineBinding = next ? next->binding : m_state->identityOnly ? core::VolumeBinding::VolumeId : core::VolumeBinding::DriveLetter;
  // A missing Next registration starts with the page default. An explicit
  // import retains its own creation selector when protection is staged.
  m_draft.protectedVolume = m_draft.baselineProtected;
  m_draft.binding = m_draft.baselineBinding;
  m_draft.bulkProtection.reset();
  const QPointer self(this);
  emit stateChanged();
  if (self) emit availabilityChanged();
}

void VolumeStatusEditor::setUnsupported() {
  if (!m_draft.supported && !m_draft.hasTarget) return;
  m_currentProtection.reset();
  m_registrationEligible = false;
  m_draft.supported = false;
  m_draft.hasTarget = false;
  m_draft.hasConfiguration = false;
  const QPointer self(this);
  emit stateChanged();
  if (self) emit availabilityChanged();
}

bool VolumeStatusEditor::editProtection(const bool value, std::optional<std::string> creationTarget) {
  if (!m_draft.supported || !controlsEnabled() || m_draft.protectedVolume == value) return false;
  const auto bindingWasEnabled = bindingEnabled();
  if (value && !m_draft.hasPlannedConfiguration() && creationTarget) {
    m_draft.protectionTarget = std::move(*creationTarget);
    m_draft.binding = m_draft.creationBinding();
  }
  m_draft.protectedVolume = value;
  const QPointer self(this);
  emit stateChanged();
  if (!self) return true;
  if (bindingEnabled() != bindingWasEnabled) emit availabilityChanged();
  if (self) emit pendingChanged();
  return true;
}

void VolumeStatusEditor::setBulkProtection(const std::optional<bool> value, const bool resetOverrides) {
  if (m_draft.bulkProtection == value && !resetOverrides) return;
  const auto previous = m_draft.bulkProtection;
  const bool preserveBinding = m_draft.pendingBinding().has_value() || (!m_draft.hasConfiguration && previous != true && m_draft.protectedVolume);
  m_draft.bulkProtection = value;
  if (!m_draft.hasConfiguration && !preserveBinding) {
    if (value == true && previous != true)
      m_draft.binding = core::VolumeBinding::VolumeId;
    else if (value != true && previous == true && m_draft.binding == core::VolumeBinding::VolumeId)
      m_draft.binding = m_draft.creationBinding();
  }
  if (value)
    m_draft.protectedVolume = *value;
  else if (previous == m_draft.protectedVolume)
    m_draft.protectedVolume = m_draft.baselineProtected;
  const QPointer self(this);
  emit stateChanged();
  if (!self) return;
  emit availabilityChanged();
  if (self) emit pendingChanged();
}

bool VolumeStatusEditor::editBinding(const core::VolumeBinding value) {
  if (!bindingEnabled() || m_draft.binding == value) return false;
  m_draft.binding = value;
  const QPointer self(this);
  emit stateChanged();
  if (self) emit pendingChanged();
  return true;
}

}  // namespace uwf::app

#include "moc_VolumeStatusEditor.cpp"
