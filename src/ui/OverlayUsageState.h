/*
 * Copyright (c) 2026 HsingYun (iakext@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

#include <variant>

#include "../core/UwfModel.h"

namespace uwf::ui {

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

}  // namespace uwf::ui
