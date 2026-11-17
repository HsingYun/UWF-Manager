// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "application/diagnostics/ProcessLogSource.h"

#include "foundation/logging/Log.h"

namespace uwf::app {
std::vector<std::string> ProcessLogSource::recentLines() const { return recentLogLines(); }
void ProcessLogSource::clear() { clearLogLines(); }
}  // namespace uwf::app
