// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "GlobalConfigurationViewModel.h"

#include <QLocale>

#include "presentation/common/PresentationText.h"

namespace uwf::presentation {
GlobalConfigurationViewModel::GlobalConfigurationViewModel(uint32_t ram, QObject* parent)
    : GlobalConfigurationViewModel(std::make_shared<app::GlobalConfigurationEditor>(ram), parent) {}
GlobalConfigurationViewModel::GlobalConfigurationViewModel(std::shared_ptr<app::GlobalConfigurationState> state, uint32_t ram, QObject* parent)
    : GlobalConfigurationViewModel(std::make_shared<app::GlobalConfigurationEditor>(std::move(state), ram), parent) {}
GlobalConfigurationViewModel::GlobalConfigurationViewModel(std::shared_ptr<app::GlobalConfigurationEditor> editor, QObject* parent)
    : QObject(parent), m_editor(std::move(editor)) {
  connect(m_editor.get(), &app::GlobalConfigurationEditor::availabilityChanged, this, &GlobalConfigurationViewModel::availabilityChanged);
  connect(m_editor.get(), &app::GlobalConfigurationEditor::availabilityChanged, this, &GlobalConfigurationViewModel::usageChanged);
  connect(m_editor.get(), &app::GlobalConfigurationEditor::stateChanged, this, &GlobalConfigurationViewModel::stateChanged);
  connect(m_editor.get(), &app::GlobalConfigurationEditor::pendingChanged, this, &GlobalConfigurationViewModel::pendingChanged);
}
void GlobalConfigurationViewModel::setCurrentSession(const core::SessionSnapshot& current, const core::OverlayRuntime& runtime) {
  m_currentFilterEnabled = current.filter.enabled;
  if (current.filter.enabled)
    applyUsageState(app::OverlayUsageEnabled{runtime, current.overlay});
  else
    applyUsageState(app::OverlayUsageDisabled{});
}
void GlobalConfigurationViewModel::setData(const core::SessionSnapshot& current, const core::SessionSnapshot& next, const core::OverlayRuntime& runtime) {
  setCurrentSession(current, runtime);
  setControlsEnabled(true);
  setOverlayConfigLocked(current.filter.enabled);
  reset(next);
}
void GlobalConfigurationViewModel::applySnapshot(const core::UwfSnapshot& snapshot) {
  if (snapshot.uwfAvailable) setData(snapshot.current, snapshot.next, snapshot.runtime);
  setAvailable(snapshot.uwfAvailable);
  setControlsEnabled(snapshot.uwfAvailable && snapshot.elevated);
}
void GlobalConfigurationViewModel::applyUsageState(const app::OverlayUsageState& state) {
  m_usage = state;
  emit usageChanged();
}
GlobalConfigurationViewModel::Usage GlobalConfigurationViewModel::usage() const {
  Usage result;
  const auto* enabled = std::get_if<app::OverlayUsageEnabled>(&m_usage);
  result.enabled = available() && enabled;
  if (!result.enabled) {
    const bool disabled = available() && std::holds_alternative<app::OverlayUsageDisabled>(m_usage);
    result.valueText = disabled ? PresentationText::tr("Filter is disabled") : PresentationText::tr("Usage unavailable");
    result.percentageText = QStringLiteral("—");
    result.warningLegend = disabled ? PresentationText::tr("Enable the filter and restart to display live usage")
                                    : PresentationText::tr("Read the configuration to display live usage");
    return result;
  }
  const QLocale numbers(QLocale::English, QLocale::UnitedStates);
  const auto total = core::overlayTotalMb(enabled->runtime);
  const auto used = enabled->runtime.currentConsumptionMb;
  result.currentMb = used;
  result.warningMb = enabled->config.warningThresholdMb;
  result.criticalMb = enabled->config.criticalThresholdMb;
  result.maximumMb = enabled->config.maximumSizeMb;
  result.valueText = QStringLiteral("%1 / %2 MB").arg(numbers.toString(used), numbers.toString(static_cast<qulonglong>(total)));
  result.percentageText = total ? QStringLiteral("%1%").arg(qRound(static_cast<double>(used) * 100 / static_cast<double>(total))) : QStringLiteral("—");
  result.severity = core::overlayUsageSeverity(used, result.warningMb, result.criticalMb);
  if (result.severity == core::OverlayUsageSeverity::Critical)
    result.warningText = PresentationText::tr("Overlay critical threshold reached. Save changes and safely restart.");
  else if (result.severity == core::OverlayUsageSeverity::Warning)
    result.warningText = PresentationText::tr("Overlay warning threshold reached. Save changes and safely restart.");
  result.warningLegend = PresentationText::tr("Warning %1 MB").arg(numbers.toString(result.warningMb));
  result.criticalLegend = PresentationText::tr("Critical %1 MB").arg(numbers.toString(result.criticalMb));
  return result;
}
}  // namespace uwf::presentation

#include "moc_GlobalConfigurationViewModel.cpp"
