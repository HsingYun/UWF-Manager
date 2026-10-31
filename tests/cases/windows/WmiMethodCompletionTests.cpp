// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QScopeGuard>
#include <QTest>
#include <barrier>
#include <chrono>
#include <future>
#include <stop_token>
#include <thread>

#include "windows/wmi/WmiMethodCompletion.h"

namespace {
using namespace std::chrono_literals;
using uwf::windows::WmiMethodCompletion;

class WmiMethodCompletionTests final : public QObject {
  Q_OBJECT

 private slots:
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
