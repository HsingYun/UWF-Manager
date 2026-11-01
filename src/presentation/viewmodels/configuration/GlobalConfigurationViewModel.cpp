// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "presentation/viewmodels/configuration/GlobalConfigurationViewModel.h"

#include <QLocale>

#include "presentation/viewmodels/common/PresentationText.h"
#include "presentation/viewmodels/overlay/OverlayUsagePresentation.h"

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
void GlobalConfigurationViewModel::applyUsageState(const app::OverlayUsageState& state) {
  m_usage = state;
  emit usageChanged();
}
GlobalConfigurationViewModel::Usage GlobalConfigurationViewModel::usage() const {
  Usage result;
  result.stale = app::overlayUsageObservation(m_usage).stale();
  result.freshnessText = overlayUsageFreshnessText(m_usage);
  const auto* enabled = std::get_if<app::OverlayUsageEnabled>(&m_usage);
  result.enabled = available() && enabled;
  if (!result.enabled) {
    const bool disabled = available() && std::holds_alternative<app::OverlayUsageDisabled>(m_usage);
    result.valueText = disabled ? PresentationText::tr("Filter is disabled") : PresentationText::tr("Usage unavailable");
    result.percentageText = QStringLiteral("—");
    result.warningLegend = disabled ? PresentationText::tr("Enable the filter and restart to display live usage")
                                    : PresentationText::tr("Read the configuration to display live usage");
    if (result.stale) result.valueText.prepend(QStringLiteral("~ "));
    if (result.stale) result.warningText = result.freshnessText;
    return result;
  }
  const QLocale numbers(QLocale::English, QLocale::UnitedStates);
  const auto total = core::overlayTotalMb(enabled->runtime);
  const auto used = enabled->runtime.currentConsumptionMb;
  result.currentMb = used;
  result.warningMb = enabled->config.warningThresholdMb;
  result.criticalMb = enabled->config.criticalThresholdMb;
  result.totalMb = total;
  result.valueText = QStringLiteral("%1 / %2 MB").arg(numbers.toString(used), numbers.toString(static_cast<qulonglong>(total)));
  result.percentageText = total ? QStringLiteral("%1%").arg(qRound(static_cast<double>(used) * 100 / static_cast<double>(total))) : QStringLiteral("—");
  result.severity = core::overlayUsageSeverity(used, result.warningMb, result.criticalMb);
  if (result.severity == core::OverlayUsageSeverity::Critical)
    result.warningText = PresentationText::tr("Overlay critical threshold reached. Save changes and safely restart.");
  else if (result.severity == core::OverlayUsageSeverity::Warning)
    result.warningText = PresentationText::tr("Overlay warning threshold reached. Save changes and safely restart.");
  result.warningLegend = PresentationText::tr("Warning %1 MB").arg(numbers.toString(result.warningMb));
  result.criticalLegend = PresentationText::tr("Critical %1 MB").arg(numbers.toString(result.criticalMb));
  if (result.stale) {
    result.valueText.prepend(QStringLiteral("~ "));
    result.percentageText.prepend(QStringLiteral("~ "));
    result.warningText = result.freshnessText + (result.warningText.isEmpty() ? QString{} : u'\n' + result.warningText);
  }
  return result;
}
}  // namespace uwf::presentation

#include "moc_GlobalConfigurationViewModel.cpp"
