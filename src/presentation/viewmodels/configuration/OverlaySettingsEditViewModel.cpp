// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "presentation/viewmodels/configuration/OverlaySettingsEditViewModel.h"

#include <utility>

namespace uwf::presentation {
OverlaySettingsEditViewModel::OverlaySettingsEditViewModel(std::shared_ptr<OverlayAdvancedViewModel> policy,
                                                           std::shared_ptr<OverlayMaintenanceViewModel> maintenance)
    : m_policy(std::move(policy)), m_session(m_policy ? m_policy->editor() : nullptr, maintenance ? maintenance->editor() : nullptr) {
  if (maintenance)
    m_maintenance = std::make_shared<OverlayMaintenanceViewModel>(m_session.maintenance(), std::move(maintenance), m_policy->editor());
  connect(&m_session, &app::OverlaySettingsEditSession::stateChanged, this, &OverlaySettingsEditViewModel::stateChanged);
}
QStringList OverlaySettingsEditViewModel::conflicts() const {
  QStringList result;
  for (const auto& conflict : m_session.conflicts()) {
    if (const auto* policy = std::get_if<app::OverlayPolicyIssue>(&conflict))
      result.append(OverlayAdvancedViewModel::conflictText(*policy));
    else if (m_maintenance)
      result.append(m_maintenance->draftConflictText(std::get<app::OverlayMaintenanceEditor::Restriction>(conflict)));
  }
  result.removeDuplicates();
  return result;
}
}  // namespace uwf::presentation

#include "moc_OverlaySettingsEditViewModel.cpp"
