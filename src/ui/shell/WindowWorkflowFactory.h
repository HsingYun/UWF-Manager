// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <memory>

#include "app/enhanced/EnhancedModeSession.h"
#include "app/power/PowerCoordinator.h"
#include "app/staging/FileStagingCoordinator.h"
#include "ui/commit/CommitDispatcher.h"
class QWidget;
class QTimer;
namespace uwf::ui {
class WindowWorkflowFactory {
 public:
  virtual ~WindowWorkflowFactory() = default;
  virtual std::unique_ptr<app::EnhancedModeSession> enhancedMode(app::UwfCapability capability) = 0;
  virtual std::unique_ptr<app::FileStagingCoordinator> fileStaging(app::UwfCapability capability, QWidget* parent) = 0;
  virtual std::unique_ptr<app::PowerCoordinator> power(app::UwfCapability capability, app::FileStagingCoordinator& staging,
                                                       app::PreshutdownCommitControl preshutdown, QWidget* parent) = 0;
  virtual std::unique_ptr<CommitDispatcher> commit(const core::UwfSnapshot& snapshot, QTimer* usageTimer, QWidget* parent) = 0;
};
}  // namespace uwf::ui
