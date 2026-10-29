// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <memory>

namespace uwf::ui {

class OverlayHub;

// 应用当前使用的 Hub 组合根。新增展示端点只需实现 OverlayHubView 并在这里
// 注册；控制器和 Hub 的 fallback 算法都不需要认识具体实现类。
[[nodiscard]] std::unique_ptr<OverlayHub> createDefaultOverlayHub();

}  // namespace uwf::ui
