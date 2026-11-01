// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

namespace uwf::windows {

class CrashHandler final {
 public:
  // Install before creating QApplication or worker threads.
  static void install();
};

}  // namespace uwf::windows
