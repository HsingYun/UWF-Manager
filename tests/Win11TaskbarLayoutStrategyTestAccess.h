/*
 * Copyright (c) 2026 HsingYun (iakext@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */
#pragma once

#include <QSize>
#include <QWindow>

#include "ui/TaskbarLayoutStrategy.h"
#include "ui/Win11TaskbarEnvironment.h"
#include "ui/Win11TaskbarLayoutStrategy.h"

namespace uwf::ui {

// 测试专用 seam：种植半注入并调用与 commit mismatch 相同的原子回滚入口。
// 实现仅链接进集成测试目标，不进入 UwfOverlayRuntime / UWF.exe。
class Win11TaskbarLayoutStrategyTestAccess {
 public:
  [[nodiscard]] static bool plantPartialParentCommit(Win11TaskbarLayoutStrategy& strategy, QWindow* window, const win11_taskbar::Environment& environment,
                                                     const QSize& logicalSize);
  [[nodiscard]] static TaskbarLayoutStrategy::AttachResult abortIncompleteParentCommit(Win11TaskbarLayoutStrategy& strategy);
  [[nodiscard]] static bool hasLiveAttachment(const Win11TaskbarLayoutStrategy& strategy);
};

}  // namespace uwf::ui
