// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include "TaskbarLayoutCoordinator.h"

namespace uwf::ui {
[[nodiscard]] std::unique_ptr<TaskbarLayoutCoordinator> createDefaultTaskbarLayoutCoordinator(TaskbarLayoutCoordinator::DetachObserver detachObserver);
}  // namespace uwf::ui
