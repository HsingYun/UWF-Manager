// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QObject>
#include <memory>
#include <optional>

#include "application/configuration/editing/GlobalConfigurationEditor.h"
#include "application/overlay/contracts/OverlayUsage.h"

namespace uwf::presentation {
class GlobalConfigurationViewModel final : public QObject {
  Q_OBJECT
 public:
  using Range = app::GlobalConfigurationEditor::Range;
  struct Usage {
    bool enabled = false;
    bool stale = false;
    QString freshnessText;
    QString valueText;
    QString percentageText;
    QString warningText;
    QString warningLegend;
    QString criticalLegend;
    core::OverlayUsageSeverity severity = core::OverlayUsageSeverity::Normal;
    uint32_t currentMb = 0;
    uint32_t warningMb = 0;
    uint32_t criticalMb = 0;
    uint64_t totalMb = 0;
  };
  explicit GlobalConfigurationViewModel(uint32_t totalRamMb = 0, QObject* parent = nullptr);
  GlobalConfigurationViewModel(std::shared_ptr<app::GlobalConfigurationState> state, uint32_t totalRamMb, QObject* parent = nullptr);
  explicit GlobalConfigurationViewModel(std::shared_ptr<app::GlobalConfigurationEditor> editor, QObject* parent = nullptr);
  [[nodiscard]] const std::shared_ptr<app::GlobalConfigurationEditor>& editor() const { return m_editor; }
  [[nodiscard]] bool available() const { return m_editor->available(); }
  [[nodiscard]] uint32_t totalRamMb() const { return m_editor->totalRamMb(); }
  [[nodiscard]] const app::GlobalConfigurationDraft& draft() const { return m_editor->draft(); }
  [[nodiscard]] Range maximumRange() const { return m_editor->maximumRange(); }
  [[nodiscard]] Range warningRange() const { return m_editor->warningRange(); }
  [[nodiscard]] Range criticalRange() const { return m_editor->criticalRange(); }
  [[nodiscard]] std::optional<bool> pendingFilterEnabled() const { return m_editor->pendingFilterEnabled(); }
  [[nodiscard]] core::OverlayConfigDelta pendingOverlay() const { return m_editor->pendingOverlay(); }
  void reset(const core::SessionSnapshot& next) { m_editor->reset(next); }
  void setControlsEnabled(bool enabled) { m_editor->setControlsEnabled(enabled); }
  [[nodiscard]] bool controlsEnabled() const { return m_editor->controlsEnabled(); }
  [[nodiscard]] bool thresholdEditingAllowed() const { return m_editor->thresholdEditingAllowed(); }
  [[nodiscard]] QString thresholdRestriction() const { return m_editor->thresholdRestriction(); }
  [[nodiscard]] bool overlayStorageEditingAllowed() const { return m_editor->overlayStorageEditingAllowed(); }
  [[nodiscard]] QString overlayStorageRestriction() const { return m_editor->overlayStorageRestriction(); }
  void setAvailable(bool available) { m_editor->setAvailable(available); }
  void editFilterEnabled(bool value) { m_editor->editFilterEnabled(value); }
  void editOverlayType(core::OverlayType value) { m_editor->editOverlayType(value); }
  void editMaximumSize(int64_t value) { m_editor->editMaximumSize(value); }
  void editWarningThreshold(int64_t value) { m_editor->editWarningThreshold(value); }
  void editCriticalThreshold(int64_t value) { m_editor->editCriticalThreshold(value); }
  app::GlobalEditResult importFilterEnabled(bool value) { return m_editor->importFilterEnabled(value); }
  app::GlobalEditResult importOverlayType(core::OverlayType value) { return m_editor->importOverlayType(value); }
  app::GlobalEditResult importOverlayMaxMb(uint32_t value) { return m_editor->importOverlayMaxMb(value); }
  app::GlobalEditResult importOverlayWarnMb(uint32_t value) { return m_editor->importOverlayWarnMb(value); }
  app::GlobalEditResult importOverlayCritMb(uint32_t value) { return m_editor->importOverlayCritMb(value); }
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
