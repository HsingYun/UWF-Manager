// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

namespace uwf::app {

// Observed runtime mode; execution still revalidates its required live facts.
enum class RuntimeWriteRestriction { None, HormRestartRequired, HormActive, HormUnknown, ReadOnlyMediaActive, StateUnavailable };

}  // namespace uwf::app
