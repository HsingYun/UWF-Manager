// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

namespace uwf::composition {

// Dispatch before QApplication, crash handling, or WMI initialization.
int runEnhancedModeService();

}  // namespace uwf::composition
