// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QString>
#include <vector>

#include "app/api/DiagnosticReportTypes.h"

namespace uwf::windows {

// 收集当前进程 Token 的稳定文本字段。Win32 错误在 API 返回后立即捕获，
// 调用者不依赖线程 last-error 的隐式状态。
[[nodiscard]] std::vector<app::DiagnosticField> collectProcessTokenDiagnostics();

}  // namespace uwf::windows
