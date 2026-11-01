// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QTest>
#include <chrono>
#include <functional>
#include <stdexcept>
#include <vector>

#include "infrastructure/windows/service/AgentSupervision.h"

namespace {
using namespace uwf::windows;
using namespace std::chrono_literals;
using Event = AgentSupervisionEvent;
using Duration = std::chrono::milliseconds;

struct ControlledEnvironment {
  std::chrono::steady_clock::time_point now{};
  std::uint32_t session = 1;
  std::optional<std::uint32_t> healthy;
  int launches = 0;
  int idleWaits = 0;
  std::vector<std::uint32_t> launchedSessions;
  std::vector<Duration> retryDelays;
  std::vector<Duration> startupWaits;
  std::function<void()> beforeLaunch;
  std::function<Event()> idle = [] { return Event::Stopped; };
  std::function<Event()> retry = [] { return Event::Stopped; };
  std::function<AgentLaunchWaitResult()> startup = [] { return AgentLaunchWaitResult{Event::Stopped}; };

  AgentSupervisionEnvironment ports() {
    return {.now = [this] { return now; },
            .activeSession = [this] { return session; },
            .healthyAgent = [this] { return healthy; },
            .launch =
                [this] {
                  ++launches;
                  launchedSessions.push_back(session);
                  if (beforeLaunch) beforeLaunch();
                  return AgentLaunch{static_cast<std::uint32_t>(launches), session, [this](const Duration remaining) {
                                       startupWaits.push_back(remaining);
                                       return startup();
                                     }};
                },
            .waitForRequest =
                [this] {
                  ++idleWaits;
                  return idle();
                },
            .waitBeforeRetry =
                [this](const Duration delay) {
                  retryDelays.push_back(delay);
                  const auto event = retry();
                  if (event == Event::TimedOut) now += delay;
                  return event;
                }};
  }
  void run() { runAgentSupervision(ports(), {}); }
};

class AgentSupervisionTests final : public QObject {
  Q_OBJECT
 private slots:
  void failedStartsBackOffWithoutExceedingTheCap_data() {
    QTest::addColumn<int>("failure");
    QTest::newRow("launch-failure") << 0;
    QTest::newRow("process-exit") << 1;
    QTest::newRow("authentication-timeout") << 2;
  }
  void failedStartsBackOffWithoutExceedingTheCap() {
    QFETCH(int, failure);
    ControlledEnvironment environment;
    if (failure == 0) environment.beforeLaunch = [] { throw std::runtime_error("Launch denied"); };
    environment.startup = [failure] { return AgentLaunchWaitResult{failure == 1 ? Event::ProcessExited : Event::TimedOut, 17}; };
    environment.retry = [&] { return environment.retryDelays.size() == 7 ? Event::Stopped : Event::TimedOut; };
    environment.run();
    QCOMPARE(environment.retryDelays, (std::vector<Duration>{5s, 10s, 20s, 40s, 60s, 60s, 60s}));
    QCOMPARE(environment.launches, 7);
    QCOMPARE(environment.idleWaits, 0);
  }

  void authenticationResetsBackoffBeforeTheNextExplicitLaunch() {
    ControlledEnvironment environment;
    environment.startup = [&] {
      if (environment.launches == 3) {
        environment.healthy = 3;
        return AgentLaunchWaitResult{Event::Connected};
      }
      return AgentLaunchWaitResult{Event::ProcessExited};
    };
    environment.retry = [&] { return environment.retryDelays.size() == 3 ? Event::Stopped : Event::TimedOut; };
    environment.idle = [&] {
      environment.healthy.reset();
      return Event::Requested;
    };
    environment.run();
    QCOMPARE(environment.retryDelays, (std::vector<Duration>{5s, 10s, 5s}));
    QCOMPARE(environment.launches, 4);
    QCOMPARE(environment.idleWaits, 1);
  }

  void aSessionChangeSupersedesTheStartupAndResetsBackoff() {
    ControlledEnvironment environment;
    environment.startup = [&] {
      if (environment.launches == 3) {
        environment.session = 2;
        return AgentLaunchWaitResult{Event::Requested};
      }
      return AgentLaunchWaitResult{Event::ProcessExited};
    };
    environment.retry = [&] { return environment.retryDelays.size() == 3 ? Event::Stopped : Event::TimedOut; };
    environment.run();
    QCOMPARE(environment.launchedSessions, (std::vector<std::uint32_t>{1, 1, 1, 2}));
    QCOMPARE(environment.retryDelays, (std::vector<Duration>{5s, 10s, 5s}));
    QCOMPARE(environment.idleWaits, 0);
  }

  void sameSessionRequestsDoNotExtendTheStartupDeadline() {
    ControlledEnvironment environment;
    environment.startup = [&] {
      environment.now += environment.startupWaits.size() == 1 ? 10s : 20s;
      return AgentLaunchWaitResult{Event::Requested};
    };
    environment.run();
    QCOMPARE(environment.startupWaits, (std::vector<Duration>{30s, 20s}));
    QCOMPARE(environment.retryDelays, (std::vector<Duration>{5s}));
    QCOMPARE(environment.launches, 1);
  }

  void aConnectionDuringBackoffPreventsAnotherLaunch() {
    ControlledEnvironment environment;
    environment.startup = [] { return AgentLaunchWaitResult{Event::ProcessExited}; };
    environment.retry = [&] {
      environment.healthy = 42;
      return Event::Connected;
    };
    environment.run();
    QCOMPARE(environment.launches, 1);
    QCOMPARE(environment.retryDelays, (std::vector<Duration>{5s}));
    QCOMPARE(environment.idleWaits, 1);
  }

  void closingAHealthyUiWaitsForAnExplicitRequest() {
    ControlledEnvironment environment;
    environment.healthy = 42;
    environment.idle = [&] {
      environment.healthy.reset();
      environment.now += 1h;
      return Event::Stopped;
    };
    environment.run();
    QCOMPARE(environment.idleWaits, 1);
    QCOMPARE(environment.launches, 0);
    QVERIFY(environment.retryDelays.empty());
  }

  void aStopRequestPreventsRelaunchAfterAWakeup() {
    ControlledEnvironment environment;
    std::stop_source stopped;
    environment.healthy = 42;
    environment.idle = [&] {
      environment.healthy.reset();
      stopped.request_stop();
      return Event::Requested;
    };
    runAgentSupervision(environment.ports(), stopped.get_token());
    QCOMPARE(environment.idleWaits, 1);
    QCOMPARE(environment.launches, 0);
    QVERIFY(environment.retryDelays.empty());
  }

  void stopInterruptsEachSupervisionWait_data() {
    QTest::addColumn<int>("waiting");
    QTest::newRow("healthy-idle") << 0;
    QTest::newRow("retry-backoff") << 1;
    QTest::newRow("startup-authentication") << 2;
  }
  void stopInterruptsEachSupervisionWait() {
    QFETCH(int, waiting);
    ControlledEnvironment environment;
    if (waiting == 0) environment.healthy = 42;
    if (waiting == 1) environment.startup = [] { return AgentLaunchWaitResult{Event::ProcessExited}; };
    environment.run();
    QCOMPARE(environment.launches, waiting == 0 ? 0 : 1);
    QCOMPARE(environment.idleWaits, waiting == 0 ? 1 : 0);
    QCOMPARE(environment.retryDelays.size(), waiting == 1 ? std::size_t{1} : std::size_t{0});
    QCOMPARE(environment.startupWaits.size(), waiting == 0 ? std::size_t{0} : std::size_t{1});
  }
};
}  // namespace

QTEST_APPLESS_MAIN(AgentSupervisionTests)
#include "AgentSupervisionTests.moc"
