// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <functional>

#include "application/startup/StartupOptions.h"

namespace uwf::app {
struct StartupActions {
  std::function<bool()> isElevated;
  std::function<int()> elevateUi;
  std::function<int()> desktop;
  std::function<int()> service;
  std::function<int()> console;
};

// The process host supplies mechanisms. Console and service paths never read
// UI elevation state; unelevated UI never constructs the desktop application.
[[nodiscard]] int dispatchStartup(const StartupOptions& options, const StartupActions& actions);
}  // namespace uwf::app
