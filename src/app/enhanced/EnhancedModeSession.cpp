// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "EnhancedModeSession.h"

#include <QTimer>
#include <stdexcept>
#include <utility>

namespace uwf::app {

EnhancedModeSession::EnhancedModeSession(std::unique_ptr<EnhancedModeActions> actions, EnhancedModeAgentConnection& agent)
    : m_actions(std::move(actions)), m_agent(&agent) {
  bind();
}

EnhancedModeSession::EnhancedModeSession(std::unique_ptr<EnhancedModeActions> actions, std::unique_ptr<EnhancedModeAgentConnection> agent)
    : m_actions(std::move(actions)), m_ownedAgent(std::move(agent)), m_agent(m_ownedAgent.get()) {
  bind();
}

void EnhancedModeSession::bind() {
  if (!m_actions || !m_agent) throw std::invalid_argument("enhanced mode requires service actions and an agent");
  connect(m_actions.get(), &EnhancedModeActions::statusChanged, this, &EnhancedModeSession::applyStatus);
  connect(m_actions.get(), &EnhancedModeActions::agentConnectionStateChanged, this, [this](const EnhancedModeAgentState state) {
    m_status.agentState = state;
    emit agentConnectionStateChanged(state);
  });
  connect(m_agent, &EnhancedModeAgentConnection::connectionStateChanged, this, [this](const bool connected) {
    if (!m_status.serviceContractSatisfied()) return;
    m_actions->setAgentState(connected ? EnhancedModeAgentState::Connected : EnhancedModeAgentState::Disconnected);
    // Recheck SCM outside the connection callback; authentication alone never
    // reads staging data or starts a commit.
    if (!connected) QTimer::singleShot(0, this, &EnhancedModeSession::refreshRequested);
  });
}

EnhancedModeSession::~EnhancedModeSession() {
  disconnect(m_agent, nullptr, this, nullptr);
  m_agent->stop();
}

void EnhancedModeSession::refresh() { applyStatus(m_actions->status()); }

void EnhancedModeSession::applyStatus(EnhancedModeStatus status) {
  // Only a complete SCM observation changes connection ownership. Failed reads
  // leave the last confirmed contract and its running connection intact.
  m_status = std::move(status);
  if (m_status.serviceContractSatisfied()) {
    if (!m_agent->running()) {
      m_actions->setAgentState(EnhancedModeAgentState::Connecting);
      m_agent->start();
      if (!m_agent->running()) m_actions->setAgentState(EnhancedModeAgentState::Disconnected);
    }
  } else {
    m_agent->stop();
    m_actions->setAgentState(EnhancedModeAgentState::Unobserved);
  }
  emit statusChanged(m_status);
}

PreshutdownCommitControl EnhancedModeSession::preshutdownControl() {
  return {[this] { return markPreshutdown(true); }, [this] { return markPreshutdown(false); }};
}

PreshutdownControlResult EnhancedModeSession::markPreshutdown(const bool handled) {
  if (!m_status.serviceContractSatisfied()) return PreshutdownControlResult::NotApplicable;
  const bool acknowledged = handled ? m_agent->markPreshutdownCommitHandled() : m_agent->requirePreshutdownCommit();
  return acknowledged ? PreshutdownControlResult::Acknowledged : PreshutdownControlResult::Unacknowledged;
}

}  // namespace uwf::app

#include "moc_EnhancedModeSession.cpp"
