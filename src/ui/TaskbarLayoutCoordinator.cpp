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
#include "TaskbarLayoutCoordinator.h"

#include <windows.h>

#include <QCoreApplication>
#include <QTimer>
#include <algorithm>
#include <utility>

#include "Win11TaskbarLayoutStrategy.h"

namespace uwf::ui {

TaskbarLayoutCoordinator::TaskbarLayoutCoordinator(RefreshRequest refreshRequest)
    : m_refreshRequest(std::move(refreshRequest)), m_taskbarCreatedMessage(RegisterWindowMessageW(L"TaskbarCreated")) {
  if (QCoreApplication::instance()) QCoreApplication::instance()->installNativeEventFilter(this);
}

TaskbarLayoutCoordinator::~TaskbarLayoutCoordinator() {
  if (m_activeStrategy && m_attachedWindow) m_activeStrategy->detach(m_attachedWindow);
  if (QCoreApplication::instance()) QCoreApplication::instance()->removeNativeEventFilter(this);
}

void TaskbarLayoutCoordinator::registerStrategy(std::unique_ptr<TaskbarLayoutStrategy> strategy) {
  if (!strategy) return;
  m_strategies.push_back(std::move(strategy));
  std::stable_sort(
      m_strategies.begin(), m_strategies.end(),
      [](const std::unique_ptr<TaskbarLayoutStrategy>& lhs, const std::unique_ptr<TaskbarLayoutStrategy>& rhs) { return lhs->priority() > rhs->priority(); });
}

bool TaskbarLayoutCoordinator::attach(const WId window, const QSize& logicalSize) {
  for (const auto& strategy : m_strategies) {
    if (!strategy->available()) continue;

    if (m_activeStrategy && (m_activeStrategy != strategy.get() || m_attachedWindow != window)) {
      m_activeStrategy->detach(m_attachedWindow);
      m_activeStrategy = nullptr;
      m_attachedWindow = 0;
    }
    if (strategy->attach(window, logicalSize)) {
      m_activeStrategy = strategy.get();
      m_attachedWindow = window;
      return true;
    }
    strategy->detach(window);
    if (m_activeStrategy == strategy.get()) {
      m_activeStrategy = nullptr;
      m_attachedWindow = 0;
    }
  }

  if (m_activeStrategy) {
    m_activeStrategy->detach(m_attachedWindow);
    m_activeStrategy = nullptr;
    m_attachedWindow = 0;
  }
  return false;
}

bool TaskbarLayoutCoordinator::verify(const WId window) const { return m_activeStrategy && m_attachedWindow == window && m_activeStrategy->verify(window); }

void TaskbarLayoutCoordinator::detach(const WId window) {
  if (!m_activeStrategy) return;
  m_activeStrategy->detach(m_attachedWindow ? m_attachedWindow : window);
  m_activeStrategy = nullptr;
  m_attachedWindow = 0;
}

bool TaskbarLayoutCoordinator::nativeEventFilter(const QByteArray&, void* message, qintptr*) {
  if (!message || m_taskbarCreatedMessage == 0) return false;
  const auto* nativeMessage = static_cast<const MSG*>(message);
  if (nativeMessage->message != m_taskbarCreatedMessage) return false;

  // Explorer 已经重建，旧策略内的所有 Shell 句柄都只可视为失效快照。
  // 不在原生消息栈中重建 QWidget；下一事件循环再请求 HubView 刷新。
  for (const auto& strategy : m_strategies) strategy->invalidate();
  m_activeStrategy = nullptr;
  m_attachedWindow = 0;
  scheduleRefresh();
  return false;
}

void TaskbarLayoutCoordinator::scheduleRefresh() {
  if (m_refreshScheduled) return;
  m_refreshScheduled = true;
  QTimer::singleShot(0, this, [this]() {
    m_refreshScheduled = false;
    if (m_refreshRequest) m_refreshRequest();
  });
}

std::unique_ptr<TaskbarLayoutCoordinator> createDefaultTaskbarLayoutCoordinator(TaskbarLayoutCoordinator::RefreshRequest refreshRequest) {
  auto coordinator = std::make_unique<TaskbarLayoutCoordinator>(std::move(refreshRequest));
  coordinator->registerStrategy(std::make_unique<Win11TaskbarLayoutStrategy>());
  return coordinator;
}

}  // namespace uwf::ui
