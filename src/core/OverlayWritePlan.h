// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include "UwfModel.h"

namespace uwf::core {

enum class OverlayField { Type, MaximumSize, WarningThreshold, CriticalThreshold };

struct OverlayWriteStep {
  OverlayField field;
  bool requiresPrevious = false;
};

// 基于写入前的状态安排中间步骤；依赖步骤只能在前一步写入已确认后执行。
[[nodiscard]] std::vector<OverlayWriteStep> planOverlayWrites(const OverlayConfig& baseline, const OverlayConfigDelta& changes);

}  // namespace uwf::core
