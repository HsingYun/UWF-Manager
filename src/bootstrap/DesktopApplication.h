// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)

#pragma once

namespace uwf::app {
struct StartupOptions;
}

namespace uwf::bootstrap {

int runDesktopApplication(int argc, char* argv[], const app::StartupOptions& options);

}  // namespace uwf::bootstrap
