/*
 * Copyright (c) 2026 HsingYun (iakext@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <utility>

#include "TaskbarLayoutCoordinator.h"
#include "Win11TaskbarLayoutStrategy.h"

namespace uwf::ui {

std::unique_ptr<TaskbarLayoutCoordinator> createDefaultTaskbarLayoutCoordinator(TaskbarLayoutCoordinator::DetachObserver detachObserver) {
  std::vector<std::unique_ptr<TaskbarLayoutStrategy>> strategies;
  strategies.push_back(std::make_unique<Win11TaskbarLayoutStrategy>());
  return std::make_unique<TaskbarLayoutCoordinator>(std::move(strategies), std::move(detachObserver));
}

}  // namespace uwf::ui
