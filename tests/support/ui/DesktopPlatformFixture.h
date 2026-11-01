// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include "composition/desktop/DesktopPlatformPorts.h"

namespace uwf::test {
// Local Windows ports retain host filesystem/dialog I/O; UWF is injected separately.
// Copies share resolved dependencies and may replace individual ports.
[[nodiscard]] inline composition::DesktopPlatformPorts localDesktopPorts() { return composition::systemDesktopPlatformPorts(); }
}  // namespace uwf::test
