// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "EnhancedModeManager.h"

#include <QPointer>

namespace uwf::app {
EnhancedModeManager::EnhancedModeManager(std::shared_ptr<EnhancedModeServiceControl> control, wmi::WmiOperations& session, const UwfCapability capability,
                                         QObject* parent)
    : EnhancedModeActions(parent), m_operations(std::make_shared<EnhancedModeOperations>(std::move(control), session, capability)) {}

EnhancedModeStatus EnhancedModeManager::status() const {
  if (!operationInProgress() || !m_observed) m_observed = m_operations->status();
  auto result = *m_observed;
  result.agentState = m_agentState;
  return result;
}
void EnhancedModeManager::acceptChangeResult(EnhancedModeChangeResult& result) {
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
  if (!m_observed) m_observed = m_operations->status();
  return [operations = m_operations, action, description] {
    switch (action) {
      case EnhancedModeAction::Disable:
        return operations->disable();
      case EnhancedModeAction::Start:
        return operations->start();
      case EnhancedModeAction::Enable:
      case EnhancedModeAction::Repair:
        return operations->enable(description);
    }
    throw std::logic_error("invalid enhanced mode action");
  };
}
EnhancedModeChangeResult EnhancedModeManager::enable(const QString& description) {
  auto result = m_operations->enable(description);
  acceptChangeResult(result);
  return result;
}
EnhancedModeChangeResult EnhancedModeManager::start() {
  auto result = m_operations->start();
  acceptChangeResult(result);
  return result;
}
EnhancedModeChangeResult EnhancedModeManager::disable() {
  auto result = m_operations->disable();
  acceptChangeResult(result);
  return result;
}
void EnhancedModeManager::setAgentState(const EnhancedModeAgentState state) {
  if (m_agentState == state) return;
  m_agentState = state;
  emit agentConnectionStateChanged(state);
}
}  // namespace uwf::app
#include "moc_EnhancedModeManager.cpp"
