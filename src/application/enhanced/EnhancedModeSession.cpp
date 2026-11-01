// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "application/enhanced/EnhancedModeSession.h"

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
    const QPointer self(this);
    const auto actions = m_actions;
    actions->setAgentState(connected ? EnhancedModeAgentState::Connected : EnhancedModeAgentState::Disconnected);
    if (!self) return;
    // Recheck SCM outside the connection callback; authentication alone never
    // reads staging data or starts a commit.
    if (!connected) QTimer::singleShot(0, this, &EnhancedModeSession::refreshRequested);
  });
}

EnhancedModeSession::~EnhancedModeSession() {
  if (m_agent) {
    disconnect(m_agent, nullptr, this, nullptr);
    m_agent->stop();
  }
}

void EnhancedModeSession::refresh() {
  const QPointer self(this);
  const auto generation = m_observationGeneration;
  const auto actions = m_actions;
  auto status = actions->status();
  if (self && m_observationGeneration == generation) applyStatus(std::move(status));
}

void EnhancedModeSession::applyStatus(EnhancedModeStatus status) {
  // Only a complete SCM observation changes connection ownership. Failed reads
  // leave the last confirmed contract and its running connection intact.
  m_status = std::move(status);
  const auto generation = ++m_observationGeneration;
  const QPointer self(this);
  const auto current = [&] { return self && self->m_observationGeneration == generation; };
  const auto actions = m_actions;
  const auto ownedAgent = m_ownedAgent;
  const auto agent = m_agent;
  if (!agent) return;
  if (m_status.serviceContractSatisfied()) {
    const bool running = agent->running();
    if (!current() || !agent) return;
    if (!running) {
      actions->setAgentState(EnhancedModeAgentState::Connecting);
      if (!current() || !agent) return;
      agent->start();
      if (!current() || !agent) return;
      const bool started = agent->running();
      if (!current()) return;
      if (!started) actions->setAgentState(EnhancedModeAgentState::Disconnected);
      if (!current()) return;
    }
  } else {
    agent->stop();
    if (!current()) return;
    actions->setAgentState(EnhancedModeAgentState::Unobserved);
    if (!current()) return;
  }
  const auto observed = m_status;
  emit statusChanged(observed);
}

PreshutdownCommitControl EnhancedModeSession::preshutdownControl() {
  const QPointer self(this);
  return {[self] { return self ? self->markPreshutdown(true) : PreshutdownControlResult::NotApplicable; },
          [self] { return self ? self->markPreshutdown(false) : PreshutdownControlResult::NotApplicable; }};
}

PreshutdownControlResult EnhancedModeSession::markPreshutdown(const bool handled) {
  if (!m_status.serviceContractSatisfied()) return PreshutdownControlResult::NotApplicable;
  const auto ownedAgent = m_ownedAgent;
  const auto agent = m_agent;
  if (!agent) return PreshutdownControlResult::NotApplicable;
  const bool acknowledged = handled ? agent->markPreshutdownCommitHandled() : agent->requirePreshutdownCommit();
  return acknowledged ? PreshutdownControlResult::Acknowledged : PreshutdownControlResult::Unacknowledged;
}

}  // namespace uwf::app

#include "moc_EnhancedModeSession.cpp"
