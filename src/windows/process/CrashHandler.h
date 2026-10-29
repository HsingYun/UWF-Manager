// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

namespace uwf::windows {

class CrashHandler final {
 public:
  // 必须在 QApplication 和工作线程创建前调用。
  static void install();
};

}  // namespace uwf::windows
