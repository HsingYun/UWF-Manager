// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <memory>

#include "application/uwf/contracts/UwfCapability.h"
#include "presentation/viewmodels/shell/MainWindowViewModel.h"
#include "presentation/views/commit/CommitDispatcher.h"
class QWidget;

namespace uwf::ui {
class WindowWorkflowFactory {
 public:
  virtual ~WindowWorkflowFactory() = default;
  virtual std::unique_ptr<presentation::MainWindowViewModel> mainWindow(std::shared_ptr<presentation::ViewModelFactory> models, app::UwfCapability capability,
                                                                        QWidget* parent) = 0;
  virtual std::unique_ptr<CommitDispatcher> commit(CommitDispatcher::PauseObservation pauseObservation, CommitDispatcher::ActivityChanged activityChanged,
                                                    QWidget* parent) = 0;
};
}  // namespace uwf::ui
