// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <windows.h>
#include <wtsapi32.h>

#include <QTest>
#include <functional>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "infrastructure/windows/resources/UniqueHandle.h"
#include "infrastructure/windows/service/EnhancedModeServiceRuntime.h"
#include "infrastructure/windows/service/PreshutdownCommitGate.h"

namespace {
using namespace uwf;
using windows::EnhancedModeServiceRuntime;

struct ServiceFixture {
  windows::UniqueHandle stop{CreateEventW(nullptr, TRUE, FALSE, nullptr)};
  windows::UniqueHandle failure{CreateEventW(nullptr, TRUE, FALSE, nullptr)};
  windows::UniqueHandle preshutdown{CreateEventW(nullptr, TRUE, FALSE, nullptr)};
  windows::PreshutdownCommitGate gate;
  std::unique_ptr<EnhancedModeServiceRuntime> runtime;
  std::function<void()> beforeWait;
  std::function<void()> onStop;
  std::function<void()> onCommit;
  std::function<void(const SERVICE_STATUS&)> onStatus;
  bool failStart = false;
  bool failWait = false;
  bool failCommit = false;
  int failedShutdownSignals = 0;
  DWORD agentFailure = NO_ERROR;
  int startCalls = 0;
  int stopCalls = 0;
  int requestCalls = 0;
  int stopSignals = 0;
  int shutdownSignals = 0;
  int commits = 0;
  std::mutex mutex;
  std::vector<std::string> trace;
  std::vector<SERVICE_STATUS> statuses;

  ServiceFixture() {
    if (!stop.valid() || !failure.valid() || !preshutdown.valid()) throw std::runtime_error("Cannot create test service events");
    runtime = std::make_unique<EnhancedModeServiceRuntime>(
        [this] {
          ++commits;
          record("commit");
          if (onCommit) onCommit();
          if (failCommit) throw std::runtime_error("Commit failed");
          return app::FileStagingCommitResult{};
        },
        gate,
        windows::EnhancedModeServiceEnvironment{.startAgent =
                                                    [this] {
                                                      ++startCalls;
                                                      record("start-agent");
                                                      if (failStart) throw std::runtime_error("Agent startup failed");
                                                    },
                                                .requestAgent = [this] { ++requestCalls; },
                                                .stopAgent =
                                                    [this] {
                                                      ++stopCalls;
                                                      if (onStop) onStop();
                                                      record("agent-joined");
                                                    },
                                                .agentFailureCode = [this] { return agentFailure; },
                                                .waitForShutdown =
                                                    [this] {
                                                      if (beforeWait) beforeWait();
                                                      if (failWait) throw std::runtime_error("Service wait failed");
                                                      return windows::waitForServiceShutdown(stop.get(), failure.get(), preshutdown.get());
                                                    },
                                                .signalStop =
                                                    [this] {
                                                      ++stopSignals;
                                                      SetEvent(stop.get());
                                                    },
                                                .signalPreshutdown =
                                                    [this] {
                                                      ++shutdownSignals;
                                                      if (failedShutdownSignals > 0) {
                                                        --failedShutdownSignals;
                                                        throw std::runtime_error("Cannot signal preshutdown");
                                                      }
                                                      SetEvent(preshutdown.get());
                                                    },
                                                .publishStatus =
                                                    [this](const SERVICE_STATUS& status) {
                                                      {
                                                        const std::scoped_lock lock(mutex);
                                                        statuses.push_back(status);
                                                      }
                                                      if (onStatus) onStatus(status);
                                                    }});
  }
  void record(std::string event) {
    const std::scoped_lock lock(mutex);
    trace.push_back(std::move(event));
  }
  void stopService() { (void)runtime->handleControl(SERVICE_CONTROL_STOP); }
  void requestShutdown() { (void)runtime->handleControl(SERVICE_CONTROL_PRESHUTDOWN); }
};

class EnhancedModeServiceRuntimeTests final : public QObject {
  Q_OBJECT
 private slots:
  void controlOrderingDeterminesWhetherShutdownCommits_data() {
    QTest::addColumn<int>("sequence");
    QTest::addColumn<bool>("commit");
    QTest::addColumn<DWORD>("exitCode");
    QTest::newRow("stop-before-preshutdown") << 0 << false << DWORD{NO_ERROR};
    QTest::newRow("preshutdown-before-stop") << 1 << true << DWORD{NO_ERROR};
    QTest::newRow("duplicate-preshutdown") << 2 << true << DWORD{NO_ERROR};
    QTest::newRow("failure-before-preshutdown") << 3 << false << DWORD{ERROR_ACCESS_DENIED};
    QTest::newRow("preshutdown-before-failure") << 4 << false << DWORD{ERROR_ACCESS_DENIED};
    QTest::newRow("all-events-pending") << 5 << false << DWORD{ERROR_ACCESS_DENIED};
  }
  void controlOrderingDeterminesWhetherShutdownCommits() {
    QFETCH(int, sequence);
    QFETCH(bool, commit);
    QFETCH(DWORD, exitCode);
    ServiceFixture service;
    service.agentFailure = exitCode;
    service.beforeWait = [&] {
      switch (sequence) {
        case 0:
          service.stopService();
          service.requestShutdown();
          break;
        case 1:
          service.requestShutdown();
          service.stopService();
          break;
        case 2:
          service.requestShutdown();
          service.requestShutdown();
          break;
        case 3:
          SetEvent(service.failure.get());
          service.requestShutdown();
          break;
        case 4:
          service.requestShutdown();
          SetEvent(service.failure.get());
          break;
        case 5:
          service.stopService();
          SetEvent(service.failure.get());
          service.requestShutdown();
          break;
      }
    };
    service.runtime->run();
    QCOMPARE(service.startCalls, 1);
    QCOMPARE(service.stopCalls, 1);
    QCOMPARE(service.commits, commit ? 1 : 0);
    QCOMPARE(service.shutdownSignals, 1);
    QCOMPARE(service.stopSignals, sequence == 0 || sequence == 5 ? 1 : 0);
    QCOMPARE(service.statuses.front().dwCurrentState, DWORD{SERVICE_START_PENDING});
    QCOMPARE(service.statuses[1].dwCurrentState, DWORD{SERVICE_RUNNING});
    QCOMPARE(service.statuses[1].dwControlsAccepted, DWORD{SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PRESHUTDOWN | SERVICE_ACCEPT_SESSIONCHANGE});
    QCOMPARE(service.statuses.back().dwCurrentState, DWORD{SERVICE_STOPPED});
    QCOMPARE(service.statuses.back().dwWin32ExitCode, exitCode);
    QCOMPARE(service.statuses.back().dwControlsAccepted, DWORD{0});
    const std::vector<std::string> expected =
        commit ? std::vector<std::string>{"start-agent", "agent-joined", "commit"} : std::vector<std::string>{"start-agent", "agent-joined"};
    QCOMPARE(service.trace, expected);
  }
  void onlySessionArrivalEventsRequestAnAgent() {
    ServiceFixture service;
    std::vector<DWORD> responses;
    service.beforeWait = [&] {
      for (const auto event : {WTS_SESSION_LOGON, WTS_CONSOLE_CONNECT, WTS_REMOTE_CONNECT, WTS_SESSION_LOCK, WTS_SESSION_UNLOCK, WTS_SESSION_LOGOFF,
                               WTS_CONSOLE_DISCONNECT, WTS_REMOTE_DISCONNECT})
        responses.push_back(service.runtime->handleControl(SERVICE_CONTROL_SESSIONCHANGE, static_cast<DWORD>(event)));
      responses.push_back(service.runtime->handleControl(SERVICE_CONTROL_INTERROGATE));
      responses.push_back(service.runtime->handleControl(SERVICE_CONTROL_PAUSE));
      service.stopService();
    };
    service.runtime->run();
    QCOMPARE(service.requestCalls, 3);
    QCOMPARE(service.commits, 0);
    QCOMPARE(responses, (std::vector<DWORD>{0, 0, 0, 0, 0, 0, 0, 0, 0, ERROR_CALL_NOT_IMPLEMENTED}));
    // INTERROGATE republishes the current status without changing lifecycle state.
    QCOMPARE(service.statuses[1].dwCurrentState, DWORD{SERVICE_RUNNING});
    QCOMPARE(service.statuses[2].dwCurrentState, DWORD{SERVICE_RUNNING});
    QCOMPARE(service.statuses[1].dwControlsAccepted, service.statuses[2].dwControlsAccepted);
  }
  void agentJoinPrecedesReceiptConsumption_data() {
    QTest::addColumn<bool>("revokeDuringStop");
    QTest::newRow("recent-precommit-skips") << false;
    QTest::newRow("final-agent-revocation-requires-commit") << true;
  }
  void agentJoinPrecedesReceiptConsumption() {
    QFETCH(bool, revokeDuringStop);
    ServiceFixture service;
    service.gate.markHandled();
    service.beforeWait = [&] { service.requestShutdown(); };
    service.onStop = [&] {
      if (revokeDuringStop) service.gate.requireCommit();
    };
    service.runtime->run();
    QCOMPARE(service.commits, revokeDuringStop ? 1 : 0);
    QVERIFY(!service.gate.consumeHandled());
    if (revokeDuringStop) QCOMPARE(service.trace, (std::vector<std::string>{"start-agent", "agent-joined", "commit"}));
  }
  void controlsDuringCommitDoNotStartAnotherShutdown() {
    ServiceFixture service;
    DWORD shutdownReply = ERROR_GEN_FAILURE;
    DWORD stopReply = ERROR_GEN_FAILURE;
    service.beforeWait = [&] { service.requestShutdown(); };
    service.onCommit = [&] {
      shutdownReply = service.runtime->handleControl(SERVICE_CONTROL_PRESHUTDOWN);
      stopReply = service.runtime->handleControl(SERVICE_CONTROL_STOP);
    };
    service.runtime->run();
    QCOMPARE(shutdownReply, DWORD{NO_ERROR});
    QCOMPARE(stopReply, DWORD{NO_ERROR});
    QCOMPARE(service.commits, 1);
    QCOMPARE(service.shutdownSignals, 1);
    QCOMPARE(service.stopSignals, 0);
    QCOMPARE(service.stopCalls, 1);
    QCOMPARE(service.statuses.size(), std::size_t{4});
    QCOMPARE(service.statuses[2].dwCheckPoint, DWORD{2});
  }
  void failedPreshutdownAdmissionCanBeRetried_data() {
    QTest::addColumn<bool>("statusFailure");
    QTest::newRow("status-publication-failure") << true;
    QTest::newRow("event-signal-failure") << false;
  }
  void failedPreshutdownAdmissionCanBeRetried() {
    QFETCH(bool, statusFailure);
    ServiceFixture service;
    bool failed = false;
    bool rejectStatus = statusFailure;
    service.failedShutdownSignals = statusFailure ? 0 : 1;
    service.onStatus = [&](const SERVICE_STATUS& status) {
      if (status.dwCurrentState == SERVICE_STOP_PENDING && std::exchange(rejectStatus, false)) throw std::runtime_error("SCM status rejected");
    };
    service.beforeWait = [&] {
      try {
        service.requestShutdown();
      } catch (const std::runtime_error&) {
        failed = true;
      }
      service.requestShutdown();
      if (WaitForSingleObject(service.preshutdown.get(), 0) != WAIT_OBJECT_0) SetEvent(service.stop.get());
    };
    service.runtime->run();
    QVERIFY(failed);
    QCOMPARE(service.commits, 1);
    QCOMPARE(service.shutdownSignals, statusFailure ? 1 : 2);
    QCOMPARE(service.stopCalls, 1);
    QCOMPARE(service.statuses.back().dwWin32ExitCode, DWORD{NO_ERROR});
  }
  void infrastructureFailureStopsTheAgentAndReportsFailure_data() {
    QTest::addColumn<bool>("startup");
    QTest::newRow("partial-startup") << true;
    QTest::newRow("shutdown-wait") << false;
  }
  void infrastructureFailureStopsTheAgentAndReportsFailure() {
    QFETCH(bool, startup);
    ServiceFixture service;
    service.failStart = startup;
    service.failWait = !startup;
    QVERIFY_THROWS_EXCEPTION(std::runtime_error, service.runtime->run());
    QCOMPARE(service.commits, 0);
    QCOMPARE(service.stopCalls, 1);
    QCOMPARE(service.stopSignals, 1);
    QCOMPARE(service.statuses.back().dwCurrentState, DWORD{SERVICE_STOPPED});
    QCOMPARE(service.statuses.back().dwWin32ExitCode, DWORD{ERROR_GEN_FAILURE});
  }
  void applicationCommitFailureStillCompletesServiceShutdown() {
    ServiceFixture service;
    service.failCommit = true;
    service.beforeWait = [&] { service.requestShutdown(); };
    service.runtime->run();
    QCOMPARE(service.commits, 1);
    QCOMPARE(service.stopCalls, 1);
    QCOMPARE(service.statuses.back().dwCurrentState, DWORD{SERVICE_STOPPED});
  }
};
}  // namespace

QTEST_APPLESS_MAIN(EnhancedModeServiceRuntimeTests)
#include "EnhancedModeServiceRuntimeTests.moc"
