// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QString>
#include <vector>

#include "application/diagnostics/contracts/DiagnosticReportTypes.h"

namespace uwf::windows {

// Captures token fields and immediate Win32 errors without relying on the
// caller's thread-local last-error state.
[[nodiscard]] std::vector<app::DiagnosticField> collectProcessTokenDiagnostics();

}  // namespace uwf::windows
