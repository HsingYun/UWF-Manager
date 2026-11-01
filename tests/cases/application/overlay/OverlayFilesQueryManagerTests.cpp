// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QCoreApplication>
#include <QScopeGuard>
#include <QTest>
#include <QThread>
#include <QThreadPool>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <exception>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <stop_token>
#include <utility>
#include <variant>
#include <vector>

#include "application/overlay/files/OverlayFilesFailure.h"
#include "application/overlay/files/OverlayFilesQueryManager.h"

namespace {
using namespace uwf;
using Manager = app::OverlayFilesQueryManager;
using Result = Manager::Result;
using Entries = Manager::Entries;

QString volumeA() { return QStringLiteral("aaaaaaaa-1111-2222-3333-444444444444"); }
QString volumeB() { return QStringLiteral("bbbbbbbb-1111-2222-3333-444444444444"); }
QString aliasA() { return QStringLiteral("Volume{AAAAAAAA-1111-2222-3333-444444444444}"); }

QVector<app::OverlayFileEntry> catalog(const QString& name = QStringLiteral("\\one.txt:stream")) {
  return {{name, {}, false, false, 7}};
}

// Workers stop at an explicit barrier. Failure cleanup releases it even when a
// Qt assertion returns early; a deadline bounds an otherwise broken test.
class Gate final {
 public:
  void wait(const std::stop_token stop) {
    std::unique_lock lock(m_mutex);
    ++started;
    m_changed.wait_until(lock, stop, std::chrono::steady_clock::now() + std::chrono::seconds{10}, [this] { return m_released; });
    if (stop.stop_requested()) ++canceled;
    ++finished;
  }
  void release() {
    {
      const std::lock_guard lock(m_mutex);
      m_released = true;
    }
    m_changed.notify_all();
  }
  std::atomic_int started{0};
  std::atomic_int finished{0};
  std::atomic_int canceled{0};

 private:
  std::mutex m_mutex;
  std::condition_variable_any m_changed;
  bool m_released = false;
};

class Receiver final : public QObject {
 public:
  std::vector<Result> results;
  std::vector<QThread*> deliveryThreads;
  Manager::Completion completion() {
    return [this](Result result) {
      deliveryThreads.push_back(QThread::currentThread());
      results.push_back(std::move(result));
    };
  }
};

struct LoaderCalls {
  std::mutex mutex;
  std::vector<QString> volumes;
  std::vector<QThread*> threads;
};

class OverlayFilesQueryManagerTests final : public QObject {
  Q_OBJECT
 private slots:
  void initTestCase() {
    m_poolThreads = QThreadPool::globalInstance()->maxThreadCount();
    QThreadPool::globalInstance()->setMaxThreadCount(std::max(2, m_poolThreads));
  }
  void cleanupTestCase() {
    const bool completed = QThreadPool::globalInstance()->waitForDone(5000);
    QThreadPool::globalInstance()->setMaxThreadCount(m_poolThreads);
    QVERIFY(completed);
  }

  void canonicalAliasesShareOneQueryAndOneRawCatalog_data() {
    QTest::addColumn<QString>("alias");
    QTest::newRow("uppercase-bare") << volumeA().toUpper();
    QTest::newRow("volume-shell") << aliasA();
    QTest::newRow("namespace-shell") << QStringLiteral("\\\\?\\Volume{AAAAAAAA-1111-2222-3333-444444444444}\\");
  }
  void canonicalAliasesShareOneQueryAndOneRawCatalog() {
    QFETCH(QString, alias);
    auto gate = std::make_shared<Gate>();
    auto requests = std::make_shared<LoaderCalls>();
    const auto unblock = qScopeGuard([gate] { gate->release(); });
    Manager manager([gate, requests](const QString& volume, const std::stop_token stop) {
      {
        const std::lock_guard lock(requests->mutex);
        requests->volumes.push_back(volume);
        requests->threads.push_back(QThread::currentThread());
      }
      gate->wait(stop);
      return catalog();
    });
    Receiver first, second;
    auto firstSubscription = manager.subscribe(volumeA(), &first, first.completion());
    QTRY_COMPARE(gate->started.load(), 1);
    auto secondSubscription = manager.subscribe(alias, &second, second.completion());
    QVERIFY(first.results.empty());
    QVERIFY(second.results.empty());
    gate->release();
    QTRY_COMPARE(first.results.size(), std::size_t{1});
    QTRY_COMPARE(second.results.size(), std::size_t{1});
    QCOMPARE(gate->started.load(), 1);
    QCOMPARE(gate->canceled.load(), 0);
    {
      const std::lock_guard lock(requests->mutex);
      QCOMPARE(requests->volumes.size(), std::size_t{1});
      QCOMPARE(requests->volumes.front(), volumeA());
      QVERIFY(requests->threads.front() != first.thread());
    }
    QCOMPARE(first.deliveryThreads.front(), first.thread());
    QCOMPARE(second.deliveryThreads.front(), second.thread());
    QVERIFY(std::holds_alternative<Entries>(first.results.front()));
    QVERIFY(std::holds_alternative<Entries>(second.results.front()));
    const auto& firstEntries = std::get<Entries>(first.results.front());
    const auto& secondEntries = std::get<Entries>(second.results.front());
    QCOMPARE(firstEntries.get(), secondEntries.get());
    QCOMPARE(firstEntries->size(), 1);
    QCOMPARE(firstEntries->constFirst().rawName, QStringLiteral("\\one.txt:stream"));
    QVERIFY(firstEntries->constFirst().absolutePath.isEmpty());
    QCoreApplication::processEvents();
    QCOMPARE(first.results.size(), std::size_t{1});
    QCOMPARE(second.results.size(), std::size_t{1});
  }

  void detachingOneReaderDoesNotStopItsPeersOrRetry_data() {
    QTest::addColumn<bool>("destroySubscription");
    QTest::newRow("cancel") << false;
    QTest::newRow("destruct") << true;
  }
  void detachingOneReaderDoesNotStopItsPeersOrRetry() {
    QFETCH(bool, destroySubscription);
    auto gate = std::make_shared<Gate>();
    const auto unblock = qScopeGuard([gate] { gate->release(); });
    Manager manager([gate](const QString&, const std::stop_token stop) {
      gate->wait(stop);
      return catalog();
    });
    Receiver detached, peer, retry;
    std::optional<Manager::Subscription> original(manager.subscribe(volumeA(), &detached, detached.completion()));
    QTRY_COMPARE(gate->started.load(), 1);
    auto peerSubscription = manager.subscribe(aliasA(), &peer, peer.completion());
    if (destroySubscription)
      original.reset();
    else
      original->cancel();
    auto retrySubscription = manager.subscribe(volumeA(), &retry, retry.completion());
    gate->release();
    QTRY_COMPARE(peer.results.size(), std::size_t{1});
    QTRY_COMPARE(retry.results.size(), std::size_t{1});
    QVERIFY(detached.results.empty());
    QCOMPARE(gate->started.load(), 1);
    QCOMPARE(gate->canceled.load(), 0);
    QCOMPARE(std::get<Entries>(peer.results.front()).get(), std::get<Entries>(retry.results.front()).get());
  }

  void aRunningQuerySurvivesLosingEverySubscriber() {
    auto gate = std::make_shared<Gate>();
    const auto unblock = qScopeGuard([gate] { gate->release(); });
    Manager manager([gate](const QString&, const std::stop_token stop) {
      gate->wait(stop);
      return catalog();
    });
    Receiver original, reopened;
    {
      auto subscription = manager.subscribe(volumeA(), &original, original.completion());
      QTRY_COMPARE(gate->started.load(), 1);
    }
    auto reopenedSubscription = manager.subscribe(aliasA(), &reopened, reopened.completion());
    gate->release();
    QTRY_COMPARE(reopened.results.size(), std::size_t{1});
    QVERIFY(original.results.empty());
    QCOMPARE(gate->started.load(), 1);
    QCOMPARE(gate->canceled.load(), 0);
  }

  void destroyingAReceiverDetachesItsDeliveryAndKeepsTheQuery() {
    auto gate = std::make_shared<Gate>();
    const auto unblock = qScopeGuard([gate] { gate->release(); });
    Manager manager([gate](const QString&, const std::stop_token stop) {
      gate->wait(stop);
      return catalog();
    });
    auto removed = std::make_unique<QObject>();
    auto removedDeliveries = std::make_shared<std::atomic_int>(0);
    auto removedSubscription = manager.subscribe(volumeA(), removed.get(), [removedDeliveries](Result) { ++*removedDeliveries; });
    QTRY_COMPARE(gate->started.load(), 1);
    removed.reset();
    Receiver remaining;
    auto remainingSubscription = manager.subscribe(aliasA(), &remaining, remaining.completion());
    gate->release();
    QTRY_COMPARE(remaining.results.size(), std::size_t{1});
    QCOMPARE(removedDeliveries->load(), 0);
    QCOMPARE(gate->started.load(), 1);
    QCOMPARE(gate->canceled.load(), 0);
    removedSubscription.cancel();
    removedSubscription.cancel();
  }

  void differentVolumesHaveIndependentRunningQueries() {
    auto firstGate = std::make_shared<Gate>();
    auto secondGate = std::make_shared<Gate>();
    const auto unblock = qScopeGuard([firstGate, secondGate] {
      firstGate->release();
      secondGate->release();
    });
    Manager manager([firstGate, secondGate](const QString& volume, const std::stop_token stop) {
      (volume == volumeA() ? firstGate : secondGate)->wait(stop);
      return catalog(volume);
    });
    Receiver first, second;
    auto firstSubscription = manager.subscribe(volumeA(), &first, first.completion());
    QTRY_COMPARE(firstGate->started.load(), 1);
    auto secondSubscription = manager.subscribe(volumeB(), &second, second.completion());
    QTRY_COMPARE(secondGate->started.load(), 1);
    firstGate->release();
    QTRY_COMPARE(first.results.size(), std::size_t{1});
    QVERIFY(second.results.empty());
    QCOMPARE(secondGate->finished.load(), 0);
    firstSubscription.cancel();
    secondGate->release();
    QTRY_COMPARE(second.results.size(), std::size_t{1});
    QCOMPARE(firstGate->started.load(), 1);
    QCOMPARE(secondGate->started.load(), 1);
    QCOMPARE(secondGate->canceled.load(), 0);
    QCOMPARE(std::get<Entries>(first.results.front())->constFirst().rawName, volumeA());
    QCOMPARE(std::get<Entries>(second.results.front())->constFirst().rawName, volumeB());
  }

  void completedQueriesAreRetiredSoRefreshReadsAgain() {
    auto gate = std::make_shared<Gate>();
    auto calls = std::make_shared<std::atomic_int>(0);
    const auto unblock = qScopeGuard([gate] { gate->release(); });
    Manager manager([gate, calls](const QString&, const std::stop_token stop) {
      const int generation = ++*calls;
      if (generation == 1) gate->wait(stop);
      return catalog(QString::number(generation));
    });
    Receiver first, refreshed;
    auto firstSubscription = manager.subscribe(volumeA(), &first, first.completion());
    QTRY_COMPARE(gate->started.load(), 1);
    gate->release();
    QTRY_COMPARE(first.results.size(), std::size_t{1});
    auto refreshSubscription = manager.subscribe(aliasA(), &refreshed, refreshed.completion());
    QVERIFY(refreshed.results.empty());
    QTRY_COMPARE(refreshed.results.size(), std::size_t{1});
    QCOMPARE(calls->load(), 2);
    const auto& firstEntries = std::get<Entries>(first.results.front());
    const auto& refreshedEntries = std::get<Entries>(refreshed.results.front());
    QVERIFY(firstEntries.get() != refreshedEntries.get());
    QCOMPARE(firstEntries->constFirst().rawName, QStringLiteral("1"));
    QCOMPARE(refreshedEntries->constFirst().rawName, QStringLiteral("2"));
  }

  void ordinaryFailuresAreRetiredAndCanBeRetried_data() {
    QTest::addColumn<int>("failureKind");
    QTest::newRow("standard-exception") << 0;
    QTest::newRow("operation-error") << 1;
    QTest::newRow("unexpected-exception") << 2;
  }
  void ordinaryFailuresAreRetiredAndCanBeRetried() {
    QFETCH(int, failureKind);
    auto gate = std::make_shared<Gate>();
    auto calls = std::make_shared<std::atomic_int>(0);
    const auto unblock = qScopeGuard([gate] { gate->release(); });
    Manager manager([gate, calls, failureKind](const QString&, const std::stop_token stop) {
      if (++*calls == 1) {
        gate->wait(stop);
        if (failureKind == 0) throw std::runtime_error("ordinary failure");
        if (failureKind == 1) throw app::OperationError(std::make_error_code(std::errc::io_error), "provider failure");
        throw 17;
      }
      return catalog();
    });
    Receiver rejected, retry;
    auto rejectedSubscription = manager.subscribe(volumeA(), &rejected, rejected.completion());
    QTRY_COMPARE(gate->started.load(), 1);
    gate->release();
    QTRY_COMPARE(rejected.results.size(), std::size_t{1});
    QVERIFY(std::holds_alternative<std::exception_ptr>(rejected.results.front()));
    auto retrySubscription = manager.subscribe(aliasA(), &retry, retry.completion());
    QTRY_COMPARE(retry.results.size(), std::size_t{1});
    QVERIFY(std::holds_alternative<Entries>(retry.results.front()));
    QCOMPARE(calls->load(), 2);
    QCOMPARE(rejected.results.size(), std::size_t{1});
  }

  void unknownCompletionRemainsTheSameAdmissionBarrierAfterSubscribersLeave() {
    auto gate = std::make_shared<Gate>();
    auto calls = std::make_shared<std::atomic_int>(0);
    const auto unblock = qScopeGuard([gate] { gate->release(); });
    Manager manager([gate, calls](const QString&, const std::stop_token stop) -> QVector<app::OverlayFileEntry> {
      ++*calls;
      gate->wait(stop);
      throw app::OverlayFilesQueryUncertain("provider completion is unknown");
    });
    Receiver first, peer, reopened;
    std::optional<Manager::Subscription> firstSubscription(manager.subscribe(volumeA(), &first, first.completion()));
    QTRY_COMPARE(gate->started.load(), 1);
    std::optional<Manager::Subscription> peerSubscription(manager.subscribe(aliasA(), &peer, peer.completion()));
    gate->release();
    QTRY_COMPARE(first.results.size(), std::size_t{1});
    QTRY_COMPARE(peer.results.size(), std::size_t{1});
    QVERIFY(std::holds_alternative<std::exception_ptr>(first.results.front()));
    const auto failure = std::get<std::exception_ptr>(first.results.front());
    QVERIFY(failure == std::get<std::exception_ptr>(peer.results.front()));
    try {
      std::rethrow_exception(failure);
    } catch (const app::OverlayFilesQueryUncertain& error) {
      QCOMPARE(error.writeEffect(), app::WriteEffect::None);
    } catch (...) {
      QFAIL("The retained failure lost its completion-unknown type");
    }
    firstSubscription.reset();
    peerSubscription.reset();
    auto reopenedSubscription = manager.subscribe(volumeA().toUpper(), &reopened, reopened.completion());
    QVERIFY(reopened.results.empty());
    QTRY_COMPARE(reopened.results.size(), std::size_t{1});
    QVERIFY(failure == std::get<std::exception_ptr>(reopened.results.front()));
    QCOMPARE(calls->load(), 1);
  }

  void shutdownStopsWorkersSuppressesDeliveryAndClosesAdmission() {
    auto gate = std::make_shared<Gate>();
    const auto unblock = qScopeGuard([gate] { gate->release(); });
    Manager manager([gate](const QString&, const std::stop_token stop) {
      gate->wait(stop);
      return catalog();
    });
    Receiver receiver;
    auto subscription = manager.subscribe(volumeA(), &receiver, receiver.completion());
    QTRY_COMPARE(gate->started.load(), 1);
    manager.shutdown();
    QTRY_COMPARE(gate->finished.load(), 1);
    QCOMPARE(gate->canceled.load(), 1);
    QCoreApplication::processEvents();
    QVERIFY(receiver.results.empty());
    QVERIFY_THROWS_EXCEPTION(std::logic_error, (void)manager.subscribe(volumeA(), &receiver, receiver.completion()));
    manager.shutdown();
  }

  void cancellationAfterWorkerCompletionStillSuppressesQueuedDelivery() {
    auto calls = std::make_shared<std::atomic_int>(0);
    Manager manager([calls](const QString&, std::stop_token) {
      ++*calls;
      return catalog();
    });
    Receiver canceled, refresh;
    auto canceledSubscription = manager.subscribe(volumeA(), &canceled, canceled.completion());
    // No event pumping before cancellation: the completed worker's watcher and
    // receiver deliveries are still queued on this test's thread.
    QVERIFY(QThreadPool::globalInstance()->waitForDone(5000));
    QCOMPARE(calls->load(), 1);
    canceledSubscription.cancel();
    QCoreApplication::processEvents();
    QVERIFY(canceled.results.empty());
    auto refreshSubscription = manager.subscribe(aliasA(), &refresh, refresh.completion());
    QTRY_COMPARE(refresh.results.size(), std::size_t{1});
    QCOMPARE(calls->load(), 2);
    QVERIFY(canceled.results.empty());
  }

  void aCompletionCanDetachItselfAndStartAFreshSameVolumeQuery() {
    auto calls = std::make_shared<std::atomic_int>(0);
    Manager manager([calls](const QString&, std::stop_token) { return catalog(QString::number(++*calls)); });
    Receiver first, reentered;
    std::optional<Manager::Subscription> original;
    std::optional<Manager::Subscription> next;
    original.emplace(manager.subscribe(volumeA(), &first, [&](Result result) {
      first.results.push_back(std::move(result));
      original->cancel();
      next.emplace(manager.subscribe(aliasA(), &reentered, reentered.completion()));
    }));
    QVERIFY(first.results.empty());
    QTRY_COMPARE(reentered.results.size(), std::size_t{1});
    QCOMPARE(first.results.size(), std::size_t{1});
    QCOMPARE(calls->load(), 2);
    QCOMPARE(std::get<Entries>(first.results.front())->constFirst().rawName, QStringLiteral("1"));
    QCOMPARE(std::get<Entries>(reentered.results.front())->constFirst().rawName, QStringLiteral("2"));
    QCoreApplication::processEvents();
    QCOMPARE(first.results.size(), std::size_t{1});
    QCOMPARE(reentered.results.size(), std::size_t{1});
  }

  void shutdownFromOneCompletionCancelsOtherQueuedDeliveries() {
    auto gate = std::make_shared<Gate>();
    const auto unblock = qScopeGuard([gate] { gate->release(); });
    Manager manager([gate](const QString&, const std::stop_token stop) {
      gate->wait(stop);
      return catalog();
    });
    QObject first, second;
    int deliveries = 0;
    const auto complete = [&](Result) {
      ++deliveries;
      manager.shutdown();
    };
    auto firstSubscription = manager.subscribe(volumeA(), &first, complete);
    QTRY_COMPARE(gate->started.load(), 1);
    auto secondSubscription = manager.subscribe(aliasA(), &second, complete);
    gate->release();
    QTRY_VERIFY(deliveries >= 1);
    QCoreApplication::processEvents();
    QCOMPARE(deliveries, 1);
  }

  void invalidSubscriptionsNeverLaunchTheLoader() {
    auto calls = std::make_shared<std::atomic_int>(0);
    Manager manager([calls](const QString&, std::stop_token) {
      ++*calls;
      return catalog();
    });
    Receiver receiver;
    for (const auto& invalid : {QStringLiteral("C:"), QStringLiteral("Volume{not-a-guid}"), QString{}})
      QVERIFY_THROWS_EXCEPTION(std::invalid_argument, (void)manager.subscribe(invalid, &receiver, receiver.completion()));
    QVERIFY_THROWS_EXCEPTION(std::invalid_argument, (void)manager.subscribe(volumeA(), nullptr, receiver.completion()));
    QVERIFY_THROWS_EXCEPTION(std::invalid_argument, (void)manager.subscribe(volumeA(), &receiver, {}));
    QCOMPARE(calls->load(), 0);
  }

 private:
  int m_poolThreads = 0;
};
}  // namespace

QTEST_GUILESS_MAIN(OverlayFilesQueryManagerTests)
#include "OverlayFilesQueryManagerTests.moc"
