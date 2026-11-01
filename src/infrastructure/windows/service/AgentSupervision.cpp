// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "infrastructure/windows/service/AgentSupervision.h"

#include <algorithm>
#include <stdexcept>

#include "foundation/logging/Log.h"

namespace uwf::windows {
namespace {
constexpr auto kAgentStartupGrace = std::chrono::seconds{30};
constexpr auto kAgentRetryInitial = std::chrono::seconds{5};
constexpr auto kAgentRetryMaximum = std::chrono::seconds{60};

enum class LaunchOutcome { Connected, ProcessExited, TimedOut, SessionChanged, Stopped };
struct LaunchResult {
  LaunchOutcome outcome;
  std::uint32_t processId;
  std::uint32_t exitCode = 0;
};

LaunchResult observeLaunch(const AgentSupervisionEnvironment& environment, const AgentLaunch& launched) {
  const auto deadline = environment.now() + kAgentStartupGrace;
  for (;;) {
    if (const auto connected = environment.healthyAgent()) return {LaunchOutcome::Connected, *connected};
    const auto now = environment.now();
    if (now >= deadline) return {LaunchOutcome::TimedOut, launched.processId};
    const auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(deadline - now);
    const auto result = launched.wait(remaining);
    switch (result.event) {
      case AgentSupervisionEvent::Stopped:
        return {LaunchOutcome::Stopped, launched.processId};
      case AgentSupervisionEvent::Requested:
        if (environment.activeSession() != launched.sessionId) return {LaunchOutcome::SessionChanged, launched.processId};
        break;
      case AgentSupervisionEvent::Connected:
        break;
      case AgentSupervisionEvent::ProcessExited:
        return {LaunchOutcome::ProcessExited, launched.processId, result.exitCode};
      case AgentSupervisionEvent::TimedOut:
        return {LaunchOutcome::TimedOut, launched.processId};
    }
  }
}
}  // namespace

void runAgentSupervision(const AgentSupervisionEnvironment& environment, const std::stop_token stop) {
  if (!environment.now || !environment.activeSession || !environment.healthyAgent || !environment.launch || !environment.waitForRequest ||
      !environment.waitBeforeRetry)
    throw std::invalid_argument("Agent supervision requires complete observations and waits");
  auto retryDelay = kAgentRetryInitial;
  bool delayBeforeLaunch = false;
  while (!stop.stop_requested()) {
    if (environment.healthyAgent()) {
      retryDelay = kAgentRetryInitial;
      delayBeforeLaunch = false;
      const auto event = environment.waitForRequest();
      if (event == AgentSupervisionEvent::Stopped) return;
      if (event != AgentSupervisionEvent::Requested) throw std::logic_error("Unexpected agent request wait result");
      continue;
    }
    if (delayBeforeLaunch) {
      const auto event = environment.waitBeforeRetry(std::chrono::duration_cast<std::chrono::milliseconds>(retryDelay));
      if (event == AgentSupervisionEvent::Stopped) return;
      if (event == AgentSupervisionEvent::Requested || event == AgentSupervisionEvent::Connected) {
        delayBeforeLaunch = false;
        continue;
      }
      if (event != AgentSupervisionEvent::TimedOut) throw std::logic_error("Unexpected agent retry wait result");
      retryDelay = std::min(retryDelay * 2, kAgentRetryMaximum);
    }
    try {
      const auto launched = environment.launch();
      if (!launched.wait) throw std::invalid_argument("A launched agent requires a process wait");
      UWF_LOG_I("service") << "enhanced mode UI launch started: pid=" << launched.processId << " session=" << launched.sessionId;
      const auto result = observeLaunch(environment, launched);
      switch (result.outcome) {
        case LaunchOutcome::Connected:
          UWF_LOG_I("service") << "enhanced mode UI launch confirmed: launchedPid=" << launched.processId << " connectedPid=" << result.processId;
          retryDelay = kAgentRetryInitial;
          delayBeforeLaunch = false;
          continue;
        case LaunchOutcome::Stopped:
          return;
        case LaunchOutcome::SessionChanged:
          UWF_LOG_I("service") << "enhanced mode UI launch superseded by active-session change: pid=" << launched.processId;
          retryDelay = kAgentRetryInitial;
          delayBeforeLaunch = false;
          continue;
        case LaunchOutcome::ProcessExited:
          UWF_LOG_W("service") << "enhanced mode UI exited before agent authentication: pid=" << launched.processId << " exitCode=" << result.exitCode;
          break;
        case LaunchOutcome::TimedOut:
          UWF_LOG_W("service") << "enhanced mode UI did not authenticate before timeout: pid=" << launched.processId;
          break;
      }
    } catch (const std::exception& error) {
      UWF_LOG_W("service") << "enhanced mode UI launch failed: error=" << error.what();
    } catch (...) {
      UWF_LOG_W("service") << "enhanced mode UI launch failed: error=non-standard-exception";
    }
    delayBeforeLaunch = true;
  }
}
}  // namespace uwf::windows
