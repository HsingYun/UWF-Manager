// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include "application/overlay/files/OverlayFilesQueryManager.h"
#include "application/staging/persistence/DurableFileStagingStore.h"
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
std::shared_ptr<app::DurableFileStagingStore> systemFileStagingStore(app::UwfCapability capability);
ui::MainWindowServices systemMainWindowServices(app::UwfCapability capability);
}  // namespace uwf::composition
