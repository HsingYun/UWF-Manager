// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include "application/overlay/files/OverlayFilesQueryManager.h"
#include "application/state/contracts/ApplicationStateSource.h"
#include "composition/desktop/WorkflowComposition.h"
#include "domain/uwf/contracts/Backend.h"
#include "presentation/views/shell/MainWindow.h"

namespace uwf::composition {
ui::MainWindowServices mainWindowServices(std::shared_ptr<compat::Uwf> system, app::ApplicationStateSource& state,
                                          std::shared_ptr<app::FileStagingStore> staging, DesktopPlatformPorts ports,
                                          std::shared_ptr<app::EnhancedModeServiceControl> enhancedMode = nullptr,
                                          app::EnhancedModeAgentConnection* agent = nullptr, EnhancedModeAgentFactory agentFactory = {},
                                          std::shared_ptr<app::OverlayFilesQueryManager> overlayFileQueries = {});
}  // namespace uwf::composition
