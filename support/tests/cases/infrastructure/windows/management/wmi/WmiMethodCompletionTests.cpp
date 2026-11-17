// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QScopeGuard>
#include <QTest>
#include <barrier>
#include <future>
#include <stop_token>
#include <thread>

#include "infrastructure/windows/management/wmi/WmiMethodCompletion.h"

namespace {
using namespace std::chrono_literals;
using uwf::windows::WmiMethodCompletion;

class WmiMethodCompletionTests final : public QObject {
  Q_OBJECT

 private slots:
  void rawTerminalStatusesPreserveCompletionEvidence_data() {
    QTest::addColumn<qint32>("status");
    QTest::addColumn<bool>("unknown");
    const auto add = [](const char* name, const HRESULT status, const bool unknown = false) { QTest::newRow(name) << static_cast<qint32>(status) << unknown; };
    add("success", S_OK);
    add("win32-out-of-memory", E_OUTOFMEMORY);
    add("wmi-out-of-memory", static_cast<HRESULT>(WBEM_E_OUT_OF_MEMORY));
    add("quota-exhausted", static_cast<HRESULT>(WBEM_E_QUOTA_VIOLATION));
    add("access-denied", E_ACCESSDENIED);
    add("provider-access-denied", static_cast<HRESULT>(WBEM_E_ACCESS_DENIED));
    add("provider-failure", static_cast<HRESULT>(WBEM_E_PROVIDER_FAILURE));
    add("generic-failure", E_FAIL);
    add("not-found", HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND));
    add("invalid-method", static_cast<HRESULT>(WBEM_E_INVALID_METHOD));
    add("custom-provider-error", MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x4321));
    add("client-cancellation", static_cast<HRESULT>(WBEM_E_CALL_CANCELLED), true);
    add("aborted", E_ABORT, true);
    add("rpc-call-canceled", RPC_E_CALL_CANCELED, true);
    add("win32-canceled", HRESULT_FROM_WIN32(ERROR_CANCELLED), true);
    add("win32-aborted", HRESULT_FROM_WIN32(ERROR_OPERATION_ABORTED), true);
    add("rpc-disconnected", RPC_E_DISCONNECTED, true);
    add("rpc-server-died", RPC_E_SERVER_DIED, true);
    add("wmi-transport", static_cast<HRESULT>(WBEM_E_TRANSPORT_FAILURE), true);
    add("wmi-fatal-transport", static_cast<HRESULT>(WBEM_E_FATAL_TRANSPORT_ERROR), true);
    add("wmi-connection-failed", static_cast<HRESULT>(WBEM_E_CONNECTION_FAILED), true);
    add("wmi-shutting-down", static_cast<HRESULT>(WBEM_E_SHUTTING_DOWN), true);
    add("wmi-invalid-stream", static_cast<HRESULT>(WBEM_E_INVALID_STREAM), true);
    add("rpc-communication-failure", HRESULT_FROM_WIN32(RPC_S_COMM_FAILURE), true);
    add("timeout", static_cast<HRESULT>(WBEM_E_TIMED_OUT), true);
    add("provider-timeout", static_cast<HRESULT>(WBEM_E_PROVIDER_TIMED_OUT), true);
    add("rpc-timeout", RPC_E_TIMEOUT, true);
    add("win32-timeout", HRESULT_FROM_WIN32(ERROR_TIMEOUT), true);
    add("positive-timeout", static_cast<HRESULT>(WBEM_S_TIMEDOUT), true);
    add("positive-cancellation", static_cast<HRESULT>(WBEM_S_OPERATION_CANCELLED), true);
    add("positive-pending", static_cast<HRESULT>(WBEM_S_PENDING), true);
  }
  void rawTerminalStatusesPreserveCompletionEvidence() {
    QFETCH(qint32, status);
    QFETCH(bool, unknown);
    using namespace uwf::windows::wmi;
    WmiMethodCompletion completion;
    completion.complete(static_cast<HRESULT>(status));
    auto evidence = ReadCompletion::NotDispatched;
    bool threw = false;
    try {
      completion.checkReadStatus(evidence, "controlled terminal status");
    } catch (const WmiException& error) {
      threw = true;
      QCOMPARE(error.readCompletion(), unknown ? ReadCompletion::Unknown : ReadCompletion::ProviderCompleted);
      QCOMPARE(error.diagnostic().code, status);
      QCOMPARE(error.writeEffect(), WriteEffect::None);
    }
    QCOMPARE(threw, FAILED(static_cast<HRESULT>(status)) || unknown);
    QCOMPARE(evidence, unknown ? ReadCompletion::Unknown : ReadCompletion::ProviderCompleted);
  }
  void absenceOfATerminalCallbackCannotEstablishCompletion() {
    using namespace uwf::windows::wmi;
    WmiMethodCompletion completion;
    auto evidence = ReadCompletion::NotDispatched;
    QVERIFY_THROWS_EXCEPTION(WmiProtocolError, completion.checkReadStatus(evidence, "not completed"));
    QCOMPARE(evidence, ReadCompletion::Unknown);
  }
  void waitsUntilCompletion() {
    WmiMethodCompletion completion;
    std::promise<void> started;
    auto startedResult = started.get_future();
    std::promise<DWORD> outcome;
    auto result = outcome.get_future();
    std::jthread worker([&](std::stop_token token) {
      started.set_value();
      outcome.set_value(completion.wait(token));
    });
    const auto finish = qScopeGuard([&] {
      if (worker.joinable()) {
        completion.complete(E_ABORT);
        worker.request_stop();
      }
    });

    QVERIFY(startedResult.wait_for(2s) == std::future_status::ready);
    const bool remainedPending = result.wait_for(50ms) == std::future_status::timeout;
    completion.complete(E_ACCESSDENIED);
    const bool wokeOnCompletion = result.wait_for(2s) == std::future_status::ready;
    if (!wokeOnCompletion) worker.request_stop();
    QVERIFY(remainedPending);
    QVERIFY(wokeOnCompletion);
    QCOMPARE(result.get(), DWORD{ERROR_SUCCESS});
    worker.join();
    QVERIFY(completion.completed);
    QCOMPARE(completion.status, E_ACCESSDENIED);
  }

  void cancellationWakesPendingWait() {
    WmiMethodCompletion completion;
    std::promise<void> started;
    auto startedResult = started.get_future();
    std::promise<DWORD> outcome;
    auto result = outcome.get_future();
    std::jthread worker([&](std::stop_token token) {
      started.set_value();
      outcome.set_value(completion.wait(token));
    });
    const auto finish = qScopeGuard([&] {
      if (worker.joinable()) {
        completion.complete(E_ABORT);
        worker.request_stop();
      }
    });

    QVERIFY(startedResult.wait_for(2s) == std::future_status::ready);
    const bool remainedPending = result.wait_for(50ms) == std::future_status::timeout;
    worker.request_stop();
    const bool wokeOnCancellation = result.wait_for(2s) == std::future_status::ready;
    if (!wokeOnCancellation) completion.complete(E_ABORT);
    QVERIFY(remainedPending);
    QVERIFY(wokeOnCancellation);
    QCOMPARE(result.get(), DWORD{ERROR_SUCCESS});
    worker.join();
    QVERIFY(!completion.completed);
  }

  void alreadyCanceledWaitReturnsImmediately() {
    WmiMethodCompletion completion;
    std::stop_source stop;
    stop.request_stop();
    std::promise<DWORD> outcome;
    auto result = outcome.get_future();
    std::jthread worker([&] { outcome.set_value(completion.wait(stop.get_token())); });
    const auto finish = qScopeGuard([&] {
      if (worker.joinable()) completion.complete(E_ABORT);
    });

    const bool returned = result.wait_for(2s) == std::future_status::ready;
    if (!returned) completion.complete(E_ABORT);
    QVERIFY(returned);
    QCOMPARE(result.get(), DWORD{ERROR_SUCCESS});
    worker.join();
    QVERIFY(!completion.completed);
  }

  void completionBeforeWaitingIsRetained() {
    WmiMethodCompletion completion;
    completion.complete(S_OK);
    std::promise<DWORD> outcome;
    auto result = outcome.get_future();
    std::jthread worker([&](std::stop_token token) { outcome.set_value(completion.wait(token)); });
    const auto finish = qScopeGuard([&] {
      if (worker.joinable()) {
        completion.complete(E_ABORT);
        worker.request_stop();
      }
    });

    const bool returned = result.wait_for(2s) == std::future_status::ready;
    if (!returned) worker.request_stop();
    QVERIFY(returned);
    QCOMPARE(result.get(), DWORD{ERROR_SUCCESS});
    worker.join();
    QVERIFY(completion.completed);
    QCOMPARE(completion.status, S_OK);
  }

  void cancellationAtWaitEntryNeverLosesWakeup() {
    for (int iteration = 0; iteration < 128; ++iteration) {
      WmiMethodCompletion completion;
      std::barrier start(2);
      std::promise<DWORD> outcome;
      auto result = outcome.get_future();
      std::jthread worker([&](std::stop_token token) {
        start.arrive_and_wait();
        outcome.set_value(completion.wait(token));
      });
      const auto finish = qScopeGuard([&] {
        if (worker.joinable()) {
          completion.complete(E_ABORT);
          worker.request_stop();
        }
      });

      start.arrive_and_wait();
      worker.request_stop();
      const bool returned = result.wait_for(2s) == std::future_status::ready;
      if (!returned) completion.complete(E_ABORT);
      QVERIFY2(returned, qPrintable(QStringLiteral("Cancellation wakeup was lost at iteration %1").arg(iteration)));
      QCOMPARE(result.get(), DWORD{ERROR_SUCCESS});
      worker.join();
      QVERIFY(!completion.completed);
    }
  }
};
}  // namespace

QTEST_APPLESS_MAIN(WmiMethodCompletionTests)
#include "WmiMethodCompletionTests.moc"
