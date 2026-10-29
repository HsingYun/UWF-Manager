// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QSize>
#include <QWindow>

#include "ui/overlay/taskbar/TaskbarLayoutStrategy.h"
#include "ui/overlay/taskbar/Win11TaskbarEnvironment.h"
#include "ui/overlay/taskbar/Win11TaskbarLayoutStrategy.h"

namespace uwf::ui {

// 测试专用访问边界：构造部分附着状态并调用生产回滚入口，仅链接进集成测试。
class Win11TaskbarLayoutStrategyTestAccess {
 public:
  [[nodiscard]] static bool plantPartialParentCommit(Win11TaskbarLayoutStrategy& strategy, QWindow* window, const win11_taskbar::Environment& environment,
                                                     const QSize& logicalSize);
  [[nodiscard]] static TaskbarLayoutStrategy::AttachResult abortIncompleteParentCommit(Win11TaskbarLayoutStrategy& strategy);
  [[nodiscard]] static bool hasLiveAttachment(const Win11TaskbarLayoutStrategy& strategy);
};

}  // namespace uwf::ui
