// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QObject>
#include <memory>

#include "app/configuration/VolumeStatusEditor.h"

namespace uwf::presentation {
class VolumeStatusViewModel final : public QObject {
  Q_OBJECT
 public:
  explicit VolumeStatusViewModel(QObject* parent = nullptr);
  explicit VolumeStatusViewModel(std::shared_ptr<app::VolumeStatusEditor> editor, QObject* parent = nullptr);
  [[nodiscard]] QString currentProtectionText() const;
  [[nodiscard]] bool currentProtected() const { return m_editor->currentProtection().value_or(false); }
  void reset(const core::VolumeRecord* current, const core::VolumeRecord* next) { m_editor->reset(current, next); }
  void setControlsEnabled(bool enabled) { m_editor->setControlsEnabled(enabled); }
  [[nodiscard]] bool controlsEnabled() const { return m_editor->controlsEnabled(); }
  void setUnsupported() { m_editor->setUnsupported(); }
  [[nodiscard]] const app::VolumeConfigurationDraft& draft() const { return m_editor->draft(); }
  [[nodiscard]] std::optional<bool> pendingProtection() const { return m_editor->pendingProtection(); }
  [[nodiscard]] std::optional<bool> pendingBinding() const { return m_editor->pendingBinding(); }
  bool editProtection(bool value) { return m_editor->editProtection(value); }
  bool editBinding(bool value) { return m_editor->editBinding(value); }
 signals:
  void availabilityChanged();
  void stateChanged();
  void pendingChanged();

 private:
  std::shared_ptr<app::VolumeStatusEditor> m_editor;
};
}  // namespace uwf::presentation
