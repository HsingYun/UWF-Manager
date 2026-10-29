// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include "WorkflowComposition.h"
#include "app/staging/DurableFileStagingStore.h"
#include "ui/shell/MainWindow.h"
namespace uwf::bootstrap {
ui::MainWindowServices mainWindowServices(app::wmi::WmiOperations& session, app::ApplicationStateSource& state, std::shared_ptr<app::FileStagingStore> staging,
                                          std::shared_ptr<app::EnhancedModeServiceControl> enhancedMode = nullptr,
                                          app::EnhancedModeAgentConnection* agent = nullptr, EnhancedModeAgentFactory agentFactory = {});
std::shared_ptr<app::DurableFileStagingStore> systemFileStagingStore(app::UwfCapability capability);
ui::MainWindowServices systemMainWindowServices(app::UwfCapability capability);
}  // namespace uwf::bootstrap
