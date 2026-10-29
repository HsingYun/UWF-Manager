// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "TaskbarLayoutCoordinatorFactory.h"

#include <utility>

#include "Win11TaskbarLayoutStrategy.h"

namespace uwf::ui {

std::unique_ptr<TaskbarLayoutCoordinator> createDefaultTaskbarLayoutCoordinator(TaskbarLayoutCoordinator::DetachObserver detachObserver) {
  std::vector<std::unique_ptr<TaskbarLayoutStrategy>> strategies;
  strategies.push_back(std::make_unique<Win11TaskbarLayoutStrategy>());
  return std::make_unique<TaskbarLayoutCoordinator>(std::move(strategies), std::move(detachObserver));
}

}  // namespace uwf::ui
