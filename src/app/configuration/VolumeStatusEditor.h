// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QObject>

#include "app/configuration/ConfigurationDraft.h"
#include "app/configuration/ConfigurationState.h"

namespace uwf::app {

class VolumeStatusEditor final : public QObject {
  Q_OBJECT
 public:
  explicit VolumeStatusEditor(QObject* parent = nullptr);
  VolumeStatusEditor(std::shared_ptr<app::VolumeConfigurationState> state, QObject* parent = nullptr);
  [[nodiscard]] std::optional<bool> currentProtection() const { return m_currentProtection; }
  void reset(const core::VolumeRecord* current, const core::VolumeRecord* next, bool available = true);
  void setUnsupported();
  void setControlsEnabled(bool enabled);
  [[nodiscard]] bool controlsEnabled() const { return m_controlsEnabled && m_draft.hasVolume; }
  [[nodiscard]] const app::VolumeConfigurationDraft& draft() const { return m_draft; }
  [[nodiscard]] std::optional<bool> pendingProtection() const { return m_draft.pendingProtection(); }
  [[nodiscard]] std::optional<bool> pendingBinding() const { return m_draft.pendingBinding(); }
  bool editProtection(bool value);
  bool editBinding(bool value);

 signals:
  void availabilityChanged();
  void stateChanged();
  void pendingChanged();

 private:
  std::shared_ptr<app::VolumeConfigurationState> m_state;
  app::VolumeConfigurationDraft& m_draft;
  bool m_controlsEnabled = true;
  std::optional<bool> m_currentProtection;
};

}  // namespace uwf::app
