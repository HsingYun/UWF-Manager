// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QSize>
#include <QWindow>

#include "presentation/views/overlay/taskbar/TaskbarLayoutStrategy.h"
#include "presentation/views/overlay/taskbar/Win11TaskbarEnvironment.h"
#include "presentation/views/overlay/taskbar/Win11TaskbarLayoutStrategy.h"

namespace uwf::ui {

// Integration-test access to partial attachment setup and the production rollback path.
class Win11TaskbarLayoutStrategyTestAccess {
 public:
  [[nodiscard]] static bool plantPartialParentCommit(Win11TaskbarLayoutStrategy& strategy, QWindow* window, const win11_taskbar::Environment& environment,
                                                     const QSize& logicalSize);
  [[nodiscard]] static TaskbarLayoutStrategy::AttachResult abortIncompleteParentCommit(Win11TaskbarLayoutStrategy& strategy);
  [[nodiscard]] static bool hasLiveAttachment(const Win11TaskbarLayoutStrategy& strategy);
};

}  // namespace uwf::ui
