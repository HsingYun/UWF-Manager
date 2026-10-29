// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

namespace uwf::service {

// 必须在 QApplication、CrashHandler 和 WMI 初始化之前调用。
int runEnhancedModeService();

}  // namespace uwf::service
