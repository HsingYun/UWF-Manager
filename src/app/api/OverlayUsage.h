// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <variant>

#include "core/UwfModel.h"

namespace uwf::app {

struct OverlayUsageUnavailable {};
struct OverlayUsageDisabled {};

struct OverlayUsageEnabled {
  constexpr OverlayUsageEnabled(const core::OverlayRuntime usageRuntime, const core::OverlayConfig overlayConfig) noexcept
      : runtime(usageRuntime), config(overlayConfig) {}

  core::OverlayRuntime runtime;
  core::OverlayConfig config;
};

// 展示端只接收完整、可提交的状态。用互斥类型表达“不可用 / 已禁用 / 已启用”，
// 从接口层排除 enabled=true 但缺少 runtime 等无效组合。
using OverlayUsageState = std::variant<OverlayUsageUnavailable, OverlayUsageDisabled, OverlayUsageEnabled>;

}  // namespace uwf::app
