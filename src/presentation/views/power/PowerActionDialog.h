// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QString>
#include <chrono>
#include <cstddef>

#include "application/power/contracts/PowerTypes.h"

class QWidget;

namespace uwf::ui {

// A single view follows preparation, confirmation, commit progress and failure decisions.
// The ViewModel owns timing and outcomes; cancellation stays the default action.
app::PowerActionDialogOutcome runPowerActionDialog(QWidget* parent, app::PowerActionDialogRequest request);
// The minimum calculating-state duration is a presentation policy; polling still uses the Qt event loop.
app::PowerActionDialogOutcome runPowerActionDialog(QWidget* parent, app::PowerActionDialogRequest request,
                                                 std::chrono::milliseconds minimumPreparationDuration);

}  // namespace uwf::ui
