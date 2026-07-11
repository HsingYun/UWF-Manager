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
#pragma once

#include <QAbstractNativeEventFilter>
#include <QObject>
#include <functional>
#include <memory>
#include <vector>

#include "TaskbarLayoutStrategy.h"

namespace uwf::ui {

// Taskbar HubView 与具体 Shell 布局之间的唯一边界。策略按优先级注册；每次
// attach 都重新探测，以便未来更高优先级策略可在运行期接管。
class TaskbarLayoutCoordinator final : public QObject, private QAbstractNativeEventFilter {
 public:
  using RefreshRequest = std::function<void()>;

  explicit TaskbarLayoutCoordinator(RefreshRequest refreshRequest);
  ~TaskbarLayoutCoordinator() override;

  void registerStrategy(std::unique_ptr<TaskbarLayoutStrategy> strategy);
  bool attach(WId window, const QSize& logicalSize);
  [[nodiscard]] bool verify(WId window) const;
  void detach(WId window);

 private:
  bool nativeEventFilter(const QByteArray& eventType, void* message, qintptr* result) override;
  void scheduleRefresh();

  std::vector<std::unique_ptr<TaskbarLayoutStrategy>> m_strategies;
  TaskbarLayoutStrategy* m_activeStrategy = nullptr;
  WId m_attachedWindow = 0;
  RefreshRequest m_refreshRequest;
  quint32 m_taskbarCreatedMessage = 0;
  bool m_refreshScheduled = false;
};

[[nodiscard]] std::unique_ptr<TaskbarLayoutCoordinator> createDefaultTaskbarLayoutCoordinator(TaskbarLayoutCoordinator::RefreshRequest refreshRequest);

}  // namespace uwf::ui
