// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <chrono>
#include <cstdint>
#include <functional>
#include <optional>
#include <stop_token>

namespace uwf::windows {
enum class AgentSupervisionEvent { Stopped, Requested, Connected, TimedOut, ProcessExited };
struct AgentLaunchWaitResult {
  AgentSupervisionEvent event;
  std::uint32_t exitCode = 0;
};
struct AgentLaunch {
  std::uint32_t processId;
  std::uint32_t sessionId;
  // Retains the launched process until this receipt is released.
  std::function<AgentLaunchWaitResult(std::chrono::milliseconds)> wait;
};
struct AgentSupervisionEnvironment {
  std::function<std::chrono::steady_clock::time_point()> now;
  std::function<std::uint32_t()> activeSession;
  std::function<std::optional<std::uint32_t>()> healthyAgent;
  std::function<AgentLaunch()> launch;
  // A healthy agent waits for an explicit request, not for disconnection, so
  // closing the UI does not trigger an automatic relaunch.
  std::function<AgentSupervisionEvent()> waitForRequest;
  std::function<AgentSupervisionEvent(std::chrono::milliseconds)> waitBeforeRetry;
};

// The production supervision loop; native observations and waits stay in its host.
void runAgentSupervision(const AgentSupervisionEnvironment& environment, std::stop_token stop);
}  // namespace uwf::windows
