// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QPointer>
#include <cstdint>
#include <memory>

#include "application/enhanced/contracts/EnhancedModeActions.h"
#include "application/enhanced/contracts/EnhancedModeAgentConnection.h"
#include "application/enhanced/contracts/PreshutdownCommitControl.h"

namespace uwf::app {

// Coordinates service actions and authentication for one interactive session.
// Borrowed connections must outlive the session; owned connections stop before
// their worker and service actions are destroyed.
class EnhancedModeSession final : public QObject {
  Q_OBJECT
 public:
  EnhancedModeSession(std::unique_ptr<EnhancedModeActions> actions, EnhancedModeAgentConnection& agent);
  EnhancedModeSession(std::unique_ptr<EnhancedModeActions> actions, std::unique_ptr<EnhancedModeAgentConnection> agent);
  ~EnhancedModeSession() override;

  [[nodiscard]] EnhancedModeActions& actions() const { return *m_actions; }
  void refresh();
  // Retained callbacks become NotApplicable after this session is destroyed.
  [[nodiscard]] PreshutdownCommitControl preshutdownControl();

 signals:
  void statusChanged(const uwf::app::EnhancedModeStatus& status);
  void agentConnectionStateChanged(uwf::app::EnhancedModeAgentState state);
  void refreshRequested();

 private:
  void bind();
  void applyStatus(EnhancedModeStatus status);
  [[nodiscard]] PreshutdownControlResult markPreshutdown(bool handled);

  std::shared_ptr<EnhancedModeActions> m_actions;
  std::shared_ptr<EnhancedModeAgentConnection> m_ownedAgent;
  QPointer<EnhancedModeAgentConnection> m_agent;
  EnhancedModeStatus m_status;
  std::uint64_t m_observationGeneration = 0;
};

}  // namespace uwf::app
