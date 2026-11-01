// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <functional>

#include "application/enhanced/contracts/EnhancedModeAgentConnection.h"
#include "application/enhanced/contracts/EnhancedModeServiceControl.h"
#include "application/power/PowerCoordinator.h"
#include "application/uwf/contracts/UwfMaintenanceRepository.h"
#include "composition/desktop/DesktopPlatformPorts.h"
#include "presentation/views/shell/WindowWorkflowFactory.h"
namespace uwf::composition {
using EnhancedModeAgentFactory = std::function<std::unique_ptr<app::EnhancedModeAgentConnection>()>;
[[nodiscard]] std::shared_ptr<ui::WindowWorkflowFactory> createWindowWorkflowFactory(std::shared_ptr<app::UwfMaintenanceRepository> repository,
                                                                                     app::ApplicationStateSource& state,
                                                                                     std::shared_ptr<app::FileStagingStore> store,
                                                                                     std::shared_ptr<app::EnhancedModeServiceControl> enhancedMode,
                                                                                     app::EnhancedModeAgentConnection* agent,
                                                                                     EnhancedModeAgentFactory agentFactory, DesktopPlatformPorts ports);
[[nodiscard]] std::unique_ptr<app::PowerCoordinator> createPowerCoordinator(app::PowerServices services, QWidget* parentWindow, QObject* parent = nullptr);
[[nodiscard]] std::unique_ptr<ui::CommitDispatcher> createCommitDispatcher(std::shared_ptr<app::UwfMaintenanceRepository> repository,
                                                                           ui::CommitDispatcher::PauseObservation pauseObservation,
                                                                           std::shared_ptr<const app::CommitTraversalSource> traversal,
                                                                           app::TextFileWriter writeText,
                                                                           std::shared_ptr<ui::dialogs::FileDialogProvider> fileDialogs, QWidget* parent,
                                                                           ui::CommitDispatcher::ActivityChanged activityChanged = {});
}  // namespace uwf::composition
