// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "application/enhanced/EnhancedModeManager.h"

#include <QPointer>

namespace uwf::app {
EnhancedModeManager::EnhancedModeManager(std::shared_ptr<EnhancedModeServiceControl> control, std::shared_ptr<UwfMaintenanceRepository> repository,
                                         const UwfCapability capability, QObject* parent)
    : EnhancedModeActions(parent), m_operations(std::make_shared<EnhancedModeOperations>(std::move(control), repository, capability)) {}

EnhancedModeStatus EnhancedModeManager::status() const {
  if (operationInProgress() && m_observed) {
    auto result = *m_observed;
    result.agentState = m_agentState;
    return result;
  }
  const auto operations = m_operations;
  const auto agentState = m_agentState;
  const auto generation = ++m_observationGeneration;
  const QPointer guard(this);
  auto result = operations->status();
  if (guard) {
    if (m_observationGeneration == generation)
      m_observed = result;
    else if (m_observed)
      result = *m_observed;
    result.agentState = m_agentState;
  } else
    result.agentState = agentState;
  return result;
}
void EnhancedModeManager::acceptChangeResult(EnhancedModeChangeResult& result) {
  ++m_observationGeneration;
  m_observed = result.status;
  result.status.agentState = m_agentState;
  const QPointer guard(this);
  emit statusChanged(result.status);
  if (!guard) return;
  // Session observers may start the agent synchronously. Preserve that newer
  // observation in the result delivered to the operation's initiating view.
  result.status.agentState = m_agentState;
}
EnhancedModeActions::ChangeWork EnhancedModeManager::prepareChange(const EnhancedModeAction action, const QString& description) {
  const auto operations = m_operations;
  const QString serviceDescription = description;
  if (!m_observed) static_cast<void>(status());
  return [operations, action, description = serviceDescription] {
    switch (action) {
      case EnhancedModeAction::Disable:
        return operations->disable();
      case EnhancedModeAction::Remove:
        return operations->remove();
      case EnhancedModeAction::Start:
        return operations->start();
      case EnhancedModeAction::Enable:
      case EnhancedModeAction::Repair:
        return operations->enable(description);
    }
    throw std::logic_error("invalid enhanced mode action");
  };
}
void EnhancedModeManager::setAgentState(const EnhancedModeAgentState state) {
  if (m_agentState == state) return;
  m_agentState = state;
  emit agentConnectionStateChanged(state);
}
}  // namespace uwf::app
#include "moc_EnhancedModeManager.cpp"
