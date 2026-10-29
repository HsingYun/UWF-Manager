// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QObject>
#include <memory>

#include "app/api/OverlayUsage.h"
#include "app/configuration/GlobalConfigurationEditor.h"

namespace uwf::presentation {
class GlobalConfigurationViewModel final : public QObject {
  Q_OBJECT
 public:
  using Range = app::GlobalConfigurationEditor::Range;
  struct Usage {
    bool enabled = false;
    QString valueText;
    QString percentageText;
    QString warningText;
    QString warningLegend;
    QString criticalLegend;
    core::OverlayUsageSeverity severity = core::OverlayUsageSeverity::Normal;
    uint32_t currentMb = 0;
    uint32_t warningMb = 0;
    uint32_t criticalMb = 0;
    uint32_t maximumMb = 0;
  };
  explicit GlobalConfigurationViewModel(uint32_t totalRamMb = 0, QObject* parent = nullptr);
  GlobalConfigurationViewModel(std::shared_ptr<app::GlobalConfigurationState> state, uint32_t totalRamMb, QObject* parent = nullptr);
  explicit GlobalConfigurationViewModel(std::shared_ptr<app::GlobalConfigurationEditor> editor, QObject* parent = nullptr);
  [[nodiscard]] const std::shared_ptr<app::GlobalConfigurationEditor>& editor() const { return m_editor; }
  [[nodiscard]] bool available() const { return m_editor->available(); }
  [[nodiscard]] uint32_t totalRamMb() const { return m_editor->totalRamMb(); }
  [[nodiscard]] bool rangesNormalized() const { return m_editor->rangesNormalized(); }
  [[nodiscard]] const app::GlobalConfigurationDraft& draft() const { return m_editor->draft(); }
  [[nodiscard]] Range maximumRange() const { return m_editor->maximumRange(); }
  [[nodiscard]] Range warningRange() const { return m_editor->warningRange(); }
  [[nodiscard]] Range criticalRange() const { return m_editor->criticalRange(); }
  [[nodiscard]] std::optional<bool> pendingFilterEnabled() const { return m_editor->pendingFilterEnabled(); }
  [[nodiscard]] core::OverlayConfigDelta pendingOverlay() const { return m_editor->pendingOverlay(); }
  void reset(const core::SessionSnapshot& next) { m_editor->reset(next); }
  void setData(const core::SessionSnapshot& current, const core::SessionSnapshot& next, const core::OverlayRuntime& runtime);
  void applySnapshot(const core::UwfSnapshot& snapshot);
  void setControlsEnabled(bool enabled) { m_editor->setControlsEnabled(enabled); }
  void setOverlayConfigLocked(bool locked) { m_editor->setOverlayConfigLocked(locked); }
  [[nodiscard]] bool controlsEnabled() const { return m_editor->controlsEnabled(); }
  [[nodiscard]] bool overlayConfigLocked() const { return m_editor->overlayConfigLocked(); }
  void setAvailable(bool available) { m_editor->setAvailable(available); }
  void editFilterEnabled(bool value) { m_editor->editFilterEnabled(value); }
  void editOverlayType(core::OverlayType value) { m_editor->editOverlayType(value); }
  void editMaximumSize(int value) { m_editor->editMaximumSize(value); }
  void editWarningThreshold(int value) { m_editor->editWarningThreshold(value); }
  void editCriticalThreshold(int value) { m_editor->editCriticalThreshold(value); }
  bool importFilterEnabled(bool value) { return m_editor->importFilterEnabled(value); }
  bool importOverlayType(core::OverlayType value) { return m_editor->importOverlayType(value); }
  bool importOverlayMaxMb(uint32_t value) { return m_editor->importOverlayMaxMb(value); }
  bool importOverlayWarnMb(uint32_t value) { return m_editor->importOverlayWarnMb(value); }
  bool importOverlayCritMb(uint32_t value) { return m_editor->importOverlayCritMb(value); }
  void finishImport() { m_editor->finishImport(); }
  void setCurrentSession(const core::SessionSnapshot& current, const core::OverlayRuntime& runtime);
  // Live monitoring does not replace the full-session configuration or its draft.
  void applyUsageState(const app::OverlayUsageState& state);
  [[nodiscard]] Usage usage() const;
  [[nodiscard]] bool currentFilterEnabled() const { return m_currentFilterEnabled; }
 signals:
  void availabilityChanged();
  void stateChanged();
  void pendingChanged();
  void usageChanged();

 private:
  std::shared_ptr<app::GlobalConfigurationEditor> m_editor;
  bool m_currentFilterEnabled = false;
  app::OverlayUsageState m_usage{app::OverlayUsageUnavailable{}};
};
}  // namespace uwf::presentation
