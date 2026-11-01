// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "application/startup/StartupDispatch.h"

#include <stdexcept>

namespace uwf::app {
int dispatchStartup(const StartupOptions& options, const StartupActions& actions) {
  switch (options.mode) {
    case StartupMode::Console:
      return actions.console();
    case StartupMode::Service:
      return actions.service();
    case StartupMode::Interactive:
    case StartupMode::Quiet:
      return actions.isElevated() ? actions.desktop() : actions.elevateUi();
  }
  throw std::logic_error("unknown startup mode");
}
}  // namespace uwf::app
