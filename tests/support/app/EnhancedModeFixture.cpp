// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "EnhancedModeFixture.h"

namespace uwf::test::application {

app::EnhancedModeStatus completeEnhancedModeStatus() {
  return {.state = app::EnhancedModeState::Enabled,
          .serviceExists = true,
          .ownProcess = true,
          .automaticStart = true,
          .localSystemAccount = true,
          .running = true,
          .executableMatches = true,
          .preshutdownTimeoutConfigured = true,
          .requiredPrivilegesConfigured = true,
          .preshutdownAccepted = true,
          .serviceRegistryPresent = true,
          .agentState = app::EnhancedModeAgentState::Unobserved,
          .detail = {}};
}

}  // namespace uwf::test::application
