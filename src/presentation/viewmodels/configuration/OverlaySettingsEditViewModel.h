// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include "application/configuration/editing/OverlaySettingsEditSession.h"
#include "presentation/viewmodels/configuration/OverlayAdvancedViewModel.h"
#include "presentation/viewmodels/configuration/OverlayMaintenanceViewModel.h"

namespace uwf::presentation {
// One window owns one edit transaction. Policy and maintenance stay private
// until Save; child dialogs commit into this transaction, never global state.
class OverlaySettingsEditViewModel final : public QObject {
  Q_OBJECT
 public:
  OverlaySettingsEditViewModel(std::shared_ptr<OverlayAdvancedViewModel> policy, std::shared_ptr<OverlayMaintenanceViewModel> maintenance = {});
  [[nodiscard]] OverlayAdvancedViewModel::Edit policyEdit() const { return m_session.policyEdit(); }
  [[nodiscard]] const std::shared_ptr<OverlayAdvancedViewModel>& policyModel() const { return m_policy; }
  [[nodiscard]] const std::shared_ptr<OverlayMaintenanceViewModel>& maintenance() const { return m_maintenance; }
  [[nodiscard]] bool edited() const { return m_session.edited(); }
  [[nodiscard]] int pendingCount() const { return m_session.pendingCount(); }
  [[nodiscard]] bool canCommit() const { return m_session.canCommit(); }
  [[nodiscard]] QStringList conflicts() const;
  void setPolicyValue(OverlayAdvancedViewModel::Field field, bool enabled) { m_session.setPolicyValue(field, enabled); }
  void revert() { m_session.revert(); }
  bool commit() { return m_session.commit(); }

 signals:
  void stateChanged();

 private:
  std::shared_ptr<OverlayAdvancedViewModel> m_policy;
  app::OverlaySettingsEditSession m_session;
  std::shared_ptr<OverlayMaintenanceViewModel> m_maintenance;
};
}  // namespace uwf::presentation
