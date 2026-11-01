// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QObject>

#include "application/configuration/editing/ConfigurationDraft.h"
#include "application/configuration/editing/ConfigurationState.h"

namespace uwf::app {

class VolumeStatusEditor final : public QObject {
  Q_OBJECT
 public:
  explicit VolumeStatusEditor(QObject* parent = nullptr);
  VolumeStatusEditor(std::shared_ptr<app::VolumeConfigurationState> state, QObject* parent = nullptr);
  [[nodiscard]] std::optional<bool> currentProtection() const { return m_currentProtection; }
  void reset(const core::VolumeRecord* current, const core::VolumeRecord* next, bool available = true, const core::VolumeInfo* physical = nullptr);
  void setUnsupported();
  void setControlsEnabled(bool enabled);
  [[nodiscard]] bool editingAllowed() const {
    return m_controlsEnabled && (!m_state->identityOnly || !m_state->draft.protectionTarget.empty()) && m_draft.hasTarget;
  }
  [[nodiscard]] bool controlsEnabled() const { return editingAllowed() && (m_draft.hasConfiguration || m_registrationEligible); }
  [[nodiscard]] bool bindingEnabled() const { return !m_state->identityOnly && controlsEnabled() && m_draft.hasPlannedConfiguration(); }
  [[nodiscard]] const app::VolumeConfigurationDraft& draft() const { return m_draft; }
  [[nodiscard]] std::optional<bool> pendingProtection() const { return m_draft.pendingProtection(); }
  [[nodiscard]] std::optional<core::VolumeBinding> pendingBinding() const { return m_draft.pendingBinding(); }
  bool editProtection(bool value, std::optional<std::string> creationTarget = {});
  void setBulkProtection(std::optional<bool> value, bool resetOverrides = false);
  bool editBinding(core::VolumeBinding value);

 signals:
  void availabilityChanged();
  void stateChanged();
  void pendingChanged();

 private:
  std::shared_ptr<app::VolumeConfigurationState> m_state;
  app::VolumeConfigurationDraft& m_draft;
  bool m_controlsEnabled = true;
  std::optional<bool> m_currentProtection;
  bool m_registrationEligible = false;
};

}  // namespace uwf::app
