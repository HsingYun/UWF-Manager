// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QMetaType>
#include <QString>
#include <stdexcept>

namespace uwf::app {

enum class EnhancedModeState {
  Disabled,
  Enabled,
  Stopped,
  RepairRequired,
};

enum class EnhancedModeAction {
  Enable,
  Disable,
  Start,
  Repair,
};

enum class EnhancedModeAgentState {
  Unobserved,
  Connecting,
  Disconnected,
  Connected,
};

struct EnhancedModeStatus {
  EnhancedModeState state = EnhancedModeState::Disabled;
  bool serviceExists = false;
  bool ownProcess = false;
  bool automaticStart = false;
  bool localSystemAccount = false;
  bool running = false;
  bool executableMatches = false;
  bool preshutdownTimeoutConfigured = false;
  bool requiredPrivilegesConfigured = false;
  bool preshutdownAccepted = false;
  bool serviceRegistryPresent = false;
  // 只有交互式 UI 进程能观测自己的代理管道。命令行服务控制入口保持
  // Unobserved；Connected 表示服务与 UI 已完成双向身份认证。
  EnhancedModeAgentState agentState = EnhancedModeAgentState::Unobserved;
  QString detail;

  [[nodiscard]] bool hasRemnants() const { return serviceExists || serviceRegistryPresent; }
  [[nodiscard]] bool serviceConfigurationSatisfied() const {
    return serviceExists && ownProcess && automaticStart && localSystemAccount && executableMatches && preshutdownTimeoutConfigured &&
           requiredPrivilegesConfigured;
  }
  [[nodiscard]] bool serviceContractSatisfied() const { return serviceConfigurationSatisfied() && running && preshutdownAccepted; }
  [[nodiscard]] EnhancedModeAction availableAction() const {
    switch (state) {
      case EnhancedModeState::Disabled:
        return EnhancedModeAction::Enable;
      case EnhancedModeState::Enabled:
        return EnhancedModeAction::Disable;
      case EnhancedModeState::Stopped:
        return EnhancedModeAction::Start;
      case EnhancedModeState::RepairRequired:
        return EnhancedModeAction::Repair;
    }
    throw std::logic_error("unknown enhanced mode state");
  }
};

struct EnhancedModeChangeResult {
  EnhancedModeStatus status;
  QString persistenceWarning;
};

}  // namespace uwf::app

Q_DECLARE_METATYPE(uwf::app::EnhancedModeStatus)
Q_DECLARE_METATYPE(uwf::app::EnhancedModeAgentState)
