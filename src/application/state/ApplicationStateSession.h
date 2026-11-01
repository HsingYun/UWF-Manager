// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include "application/state/contracts/ApplicationState.h"

namespace uwf::app {

class ApplicationStateSession final {
 public:
  void commit(ApplicationState state);
  void requireReconciliation();
  [[nodiscard]] bool hasCommittedState() const { return m_committed; }
  [[nodiscard]] bool reconciliationRequired() const { return m_reconciliationRequired; }
  [[nodiscard]] bool configurationWritesAllowed() const;
  [[nodiscard]] const std::vector<core::VolumeInfo>& volumes() const { return m_state.volumes; }
  [[nodiscard]] const core::UwfSnapshot& snapshot() const { return m_state.snapshot; }
  [[nodiscard]] const OverlayPolicySnapshot& overlayPolicy() const { return m_state.overlayPolicy; }
  [[nodiscard]] std::optional<bool> hormEnabled() const { return m_state.hormEnabled; }
  [[nodiscard]] bool overlayUsageSampled() const { return m_state.overlayUsageSampled; }

 private:
  ApplicationState m_state;
  bool m_committed = false;
  bool m_reconciliationRequired = false;
};

}  // namespace uwf::app
