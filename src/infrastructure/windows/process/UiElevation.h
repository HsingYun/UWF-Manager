// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

namespace uwf::windows {
// Startup must distinguish a negative token observation from a failed query.
[[nodiscard]] bool currentProcessElevatedForUi();
// This launches only the fixed interactive or tray entry, never CLI commands.
// A canceled UAC request returns failure without starting a desktop application.
[[nodiscard]] int relaunchUiElevated(bool quiet);
}  // namespace uwf::windows
