// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <cstdint>
#include <functional>
#include <limits>
#include <optional>

#include "application/configuration/contracts/ConfigurationEditing.h"
#include "application/configuration/editing/ConfigurationDraft.h"
#include "application/configuration/editing/ConfigurationState.h"
#include "application/configuration/editing/EditorUpdate.h"
#include "domain/uwf/policies/ConfigurationModePolicy.h"

namespace uwf::app {

class GlobalConfigurationEditor final : public QObject, public app::GlobalConfigurationEditing {
  Q_OBJECT
 public:
  struct Range {
    uint32_t minimum = 0;
    uint32_t maximum = std::numeric_limits<uint32_t>::max();
  };

  explicit GlobalConfigurationEditor(uint32_t totalRamMb = 0, QObject* parent = nullptr);
  GlobalConfigurationEditor(std::shared_ptr<app::GlobalConfigurationState> state, uint32_t totalRamMb, QObject* parent = nullptr);

  // Cross-editor updates resume every participant before explicit publication.
  class Update : public EditorUpdate<GlobalConfigurationEditor> {
   public:
    explicit Update(GlobalConfigurationEditor& editor);
    Update(Update&& other) noexcept = default;
    Update(const Update&) = delete;
    Update& operator=(const Update&) = delete;
    [[nodiscard]] bool publishSnapshot(const std::function<bool()>& continuePublishing = {});
    [[nodiscard]] bool publishAvailability(const std::function<bool()>& continuePublishing = {});

   private:
    bool m_controlsEnabled;
    bool m_thresholdEditingAllowed;
  };
  [[nodiscard]] Update beginUpdate() { return Update(*this); }

  void reset(const core::SessionSnapshot& next);
  // Rebase an already reviewed target without treating it as a new UI edit.
  void restoreChanges(std::optional<bool> filterEnabled, const core::OverlayConfigDelta& overlay);
  void setAvailable(bool available);
  void setControlsEnabled(bool enabled);
  void setOverlayConfigLocked(bool locked);
  void setMode(core::ConfigurationModeState mode);
  [[nodiscard]] bool controlsEnabled() const { return m_available && m_controlsEnabled; }
  [[nodiscard]] bool editingAllowed() const override { return controlsEnabled(); }
  [[nodiscard]] bool overlayStorageEditingAllowed() const { return editingAllowed() && !m_overlayConfigLocked; }
  [[nodiscard]] bool thresholdEditingAllowed() const {
    return editingAllowed() && core::configurationWriteRestriction(m_mode, core::ConfigurationWrite::Threshold) == core::ConfigurationWriteRestriction::None;
  }
  [[nodiscard]] QString thresholdRestriction() const;
  [[nodiscard]] QString overlayStorageRestriction() const;
  [[nodiscard]] bool available() const override { return m_available; }
  [[nodiscard]] bool filterEnabled() const override { return m_draft.filterEnabled; }
  [[nodiscard]] core::OverlayConfig overlayConfiguration() const override { return m_draft.overlay; }
  [[nodiscard]] uint32_t totalRamMb() const { return m_totalRamMb; }
  [[nodiscard]] const app::GlobalConfigurationDraft& draft() const { return m_draft; }
  [[nodiscard]] Range maximumRange() const { return m_maximumRange; }
  [[nodiscard]] Range warningRange() const { return m_warningRange; }
  [[nodiscard]] Range criticalRange() const { return m_criticalRange; }
  [[nodiscard]] std::optional<bool> pendingFilterEnabled() const;
  [[nodiscard]] core::OverlayConfigDelta pendingOverlay() const;

  void editFilterEnabled(bool value);
  void editOverlayType(core::OverlayType value);
  void editMaximumSize(int64_t value);
  void editWarningThreshold(int64_t value);
  void editCriticalThreshold(int64_t value);

  GlobalEditResult importFilterEnabled(bool value) override;
  GlobalEditResult importOverlayType(core::OverlayType value) override;
  GlobalEditResult importOverlayMaxMb(uint32_t value) override;
  GlobalEditResult importOverlayWarnMb(uint32_t value) override;
  GlobalEditResult importOverlayCritMb(uint32_t value) override;
  void finishImport() override;

 signals:
  void availabilityChanged();
  void stateChanged();
  void pendingChanged();

 private:
  void updateRanges();
  void normalize();
  void publishEdit();
  GlobalEditResult importNumber(uint32_t value, uint32_t& target, Range& range);

  std::shared_ptr<app::GlobalConfigurationState> m_state;
  app::GlobalConfigurationDraft& m_draft;
  Range m_maximumRange;
  Range m_warningRange;
  Range m_criticalRange;
  uint32_t m_totalRamMb;
  bool& m_available;
  bool m_controlsEnabled = true;
  bool m_overlayConfigLocked = false;
  core::ConfigurationModeState m_mode{false, {}, {}};
};

}  // namespace uwf::app
