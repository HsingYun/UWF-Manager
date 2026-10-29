// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)

#pragma once

namespace uwf::app {
enum class StartupMode;
}

namespace uwf::bootstrap {

int runServiceControlCommand(int argc, char* argv[], app::StartupMode mode);

}  // namespace uwf::bootstrap
