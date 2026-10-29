// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QObject>
#include <limits>

#include "app/configuration/ConfigurationDraft.h"
#include "app/configuration/ConfigurationEditing.h"
#include "app/configuration/ConfigurationState.h"

namespace uwf::app {

class GlobalConfigurationEditor final : public QObject, public app::GlobalConfigurationEditing {
  Q_OBJECT
 public:
  struct Range {
    int minimum = 0;
    int maximum = std::numeric_limits<int>::max();
  };

  explicit GlobalConfigurationEditor(uint32_t totalRamMb = 0, QObject* parent = nullptr);
  GlobalConfigurationEditor(std::shared_ptr<app::GlobalConfigurationState> state, uint32_t totalRamMb, QObject* parent = nullptr);

  void reset(const core::SessionSnapshot& next);
  void setAvailable(bool available);
  void setControlsEnabled(bool enabled);
  void setOverlayConfigLocked(bool locked);
  [[nodiscard]] bool controlsEnabled() const { return m_available && m_controlsEnabled; }
  [[nodiscard]] bool overlayConfigLocked() const { return m_overlayConfigLocked; }
  [[nodiscard]] bool available() const { return m_available; }
  [[nodiscard]] uint32_t totalRamMb() const { return m_totalRamMb; }
  [[nodiscard]] bool rangesNormalized() const { return m_rangesNormalized; }
  [[nodiscard]] const app::GlobalConfigurationDraft& draft() const { return m_draft; }
  [[nodiscard]] Range maximumRange() const { return m_maximumRange; }
  [[nodiscard]] Range warningRange() const { return m_warningRange; }
  [[nodiscard]] Range criticalRange() const { return m_criticalRange; }
  [[nodiscard]] std::optional<bool> pendingFilterEnabled() const;
  [[nodiscard]] core::OverlayConfigDelta pendingOverlay() const;

  void editFilterEnabled(bool value);
  void editOverlayType(core::OverlayType value);
  void editMaximumSize(int value);
  void editWarningThreshold(int value);
  void editCriticalThreshold(int value);

  bool importFilterEnabled(bool value) override;
  bool importOverlayType(core::OverlayType value) override;
  bool importOverlayMaxMb(uint32_t value) override;
  bool importOverlayWarnMb(uint32_t value) override;
  bool importOverlayCritMb(uint32_t value) override;
  void finishImport() override;

 signals:
  void availabilityChanged();
  void stateChanged();
  void pendingChanged();

 private:
  void updateRanges();
  void normalize();
  void publishEdit();
  bool importNumber(uint32_t value, uint32_t& target, Range& range);

  std::shared_ptr<app::GlobalConfigurationState> m_state;
  app::GlobalConfigurationDraft& m_draft;
  Range m_maximumRange;
  Range m_warningRange;
  Range m_criticalRange;
  uint32_t m_totalRamMb;
  bool& m_available;
  bool m_rangesNormalized = false;
  bool m_controlsEnabled = true;
  bool m_overlayConfigLocked = false;
};

}  // namespace uwf::app
