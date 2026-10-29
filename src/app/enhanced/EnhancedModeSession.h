// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <memory>

#include "app/api/EnhancedModeActions.h"
#include "app/api/PreshutdownCommitControl.h"
#include "app/ports/EnhancedModeAgentConnection.h"

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
  // Callbacks borrow this session and must be released before its destruction.
  [[nodiscard]] PreshutdownCommitControl preshutdownControl();

 signals:
  void statusChanged(const uwf::app::EnhancedModeStatus& status);
  void agentConnectionStateChanged(uwf::app::EnhancedModeAgentState state);
  void refreshRequested();

 private:
  void bind();
  void applyStatus(EnhancedModeStatus status);
  [[nodiscard]] PreshutdownControlResult markPreshutdown(bool handled);

  std::unique_ptr<EnhancedModeActions> m_actions;
  std::unique_ptr<EnhancedModeAgentConnection> m_ownedAgent;
  EnhancedModeAgentConnection* m_agent;
  EnhancedModeStatus m_status;
};

}  // namespace uwf::app
