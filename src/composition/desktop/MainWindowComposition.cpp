// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "composition/desktop/MainWindowComposition.h"

#include <utility>

#include "application/uwf/adapters/UwfMaintenanceRepositoryFactory.h"
#include "composition/desktop/ViewModelComposition.h"

namespace uwf::composition {
ui::MainWindowServices mainWindowServices(std::shared_ptr<compat::Uwf> system, app::ApplicationStateSource& state,
                                          std::shared_ptr<app::FileStagingStore> staging, DesktopPlatformPorts ports,
                                          std::shared_ptr<app::EnhancedModeServiceControl> enhancedMode, app::EnhancedModeAgentConnection* agent,
                                          EnhancedModeAgentFactory agentFactory, std::shared_ptr<app::OverlayFilesQueryManager> overlayFileQueries) {
  validateDesktopPlatformPorts(ports);
  return {createViewModelFactory(system, ports, {}, std::move(overlayFileQueries)),
          createWindowWorkflowFactory(app::uwfMaintenanceRepository(system), state, staging, enhancedMode, agent, std::move(agentFactory), ports),
          *ports.fileDialogs};
}
}  // namespace uwf::composition
