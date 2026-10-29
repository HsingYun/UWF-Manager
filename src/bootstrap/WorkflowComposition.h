// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <functional>

#include "app/ports/EnhancedModeServiceControl.h"
#include "app/ports/WmiOperations.h"
#include "ui/shell/WindowWorkflowFactory.h"
namespace uwf::bootstrap {
using EnhancedModeAgentFactory = std::function<std::unique_ptr<app::EnhancedModeAgentConnection>()>;
[[nodiscard]] std::shared_ptr<ui::WindowWorkflowFactory> createWindowWorkflowFactory(app::wmi::WmiOperations& session, app::FileStagingStore& store,
                                                                                     std::shared_ptr<app::EnhancedModeServiceControl> enhancedMode,
                                                                                     app::EnhancedModeAgentConnection* agent,
                                                                                     EnhancedModeAgentFactory agentFactory);
[[nodiscard]] std::unique_ptr<app::FileStagingCoordinator> createFileStagingCoordinator(app::wmi::WmiOperations& session, app::FileStagingStore& store,
                                                                                        app::UwfCapability capability, QWidget* parentWindow,
                                                                                        QObject* parent = nullptr);
[[nodiscard]] std::unique_ptr<app::PowerCoordinator> createPowerCoordinator(app::PowerServices services, QWidget* parentWindow, QObject* parent = nullptr);
[[nodiscard]] std::unique_ptr<ui::CommitDispatcher> createCommitDispatcher(app::wmi::WmiOperations& session, const core::UwfSnapshot& snapshot,
                                                                           QTimer* usageTimer, QWidget* parent);
}  // namespace uwf::bootstrap
