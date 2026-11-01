// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QMetaType>
#include <QString>
#include <optional>
#include <stdexcept>

#include "application/registry/contracts/RegistryCommitTypes.h"

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
  Remove,
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
  bool startupDisabled = false;
  bool localSystemAccount = false;
  bool running = false;
  bool executableMatches = false;
  bool preshutdownTimeoutConfigured = false;
  bool requiredPrivilegesConfigured = false;
  bool preshutdownAccepted = false;
  bool serviceRegistryPresent = false;
  // Only the interactive client observes its agent connection. Connected means
  // mutual authentication completed; CLI service control leaves this Unobserved.
  EnhancedModeAgentState agentState = EnhancedModeAgentState::Unobserved;
  QString detail;

  [[nodiscard]] bool hasRemnants() const { return serviceExists || serviceRegistryPresent; }
  [[nodiscard]] bool serviceInstallationSatisfied() const {
    return serviceExists && ownProcess && localSystemAccount && executableMatches && preshutdownTimeoutConfigured && requiredPrivilegesConfigured;
  }
  [[nodiscard]] bool serviceConfigurationSatisfied() const { return serviceInstallationSatisfied() && automaticStart; }
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
  std::optional<RegistryCommitResult> persistence{};
  std::optional<OperationFailure> persistenceFailure{};
};

}  // namespace uwf::app

Q_DECLARE_METATYPE(uwf::app::EnhancedModeStatus)
Q_DECLARE_METATYPE(uwf::app::EnhancedModeAgentState)
