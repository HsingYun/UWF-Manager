// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QString>
#include <cstddef>
#include <functional>
#include <optional>
#include <utility>
#include <variant>

#include "app/api/PowerInteraction.h"

class QWidget;

namespace uwf::ui {

// A single view follows preparation, confirmation, commit progress and failure decisions.
// The ViewModel owns timing and outcomes; cancellation stays the default action.
app::PowerActionDialogOutcome runPowerActionDialog(QWidget* parent, app::PowerActionDialogRequest request);

}  // namespace uwf::ui
