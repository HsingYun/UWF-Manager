// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <functional>

#include "application/power/contracts/PowerTypes.h"

namespace uwf::app {
struct PowerInteraction {
  std::function<PowerActionDialogOutcome(PowerActionDialogRequest)> confirm;
  std::function<void(const QString&, const QString&)> warning;
  std::function<QString(const FileStagingCommitResult&)> describeStagingResult;
};
}  // namespace uwf::app
