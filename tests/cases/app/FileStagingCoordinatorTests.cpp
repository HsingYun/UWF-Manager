// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QCoreApplication>
#include <QPointer>
#include <QSignalSpy>
#include <QTest>
#include <algorithm>
#include <functional>
#include <stdexcept>

#include "app/staging/FileStagingCoordinator.h"
#include "shared/Log.h"

namespace {
using namespace uwf::app;

class ControlledOperation final : public QObject, public FileStagingCommandOperation {
 public:
  FileStagingCoordinator::Completion completion;
  std::function<void()> onStart;
  void start() override {
    if (onStart) onStart();
  }
  void disposeAfterCallback() override { deleteLater(); }
};

FileStagingBatchResult success() {
  FileStagingBatchResult result;
  result.command.outcome = uwf::app::ApplicationCommandOutcome::Succeeded;
  result.command.discoveredFiles = 3;
  result.command.committedFiles = 3;
  result.sourceEntries = QList<FileStagingEntry>{{FileStagingKind::Directory, QStringLiteral("C:\\Staged")}};
  return result;
}

class FileStagingCoordinatorTests final : public QObject {
  Q_OBJECT
 private slots:
  void init() { uwf::clearLogLines(); }
  void synchronousCompletionReleasesOwnership();
  void suspendedRequestsResumeAsOneBatch_data();
  void suspendedRequestsResumeAsOneBatch();
  void availabilityObserversMayDestroyTheCoordinator_data();
  void availabilityObserversMayDestroyTheCoordinator();
  void startupReentrancyJoinsTheSameBatch();
  void startupFailureNotifiesEveryWaiter_data();
  void startupFailureNotifiesEveryWaiter();
  void externalReservationOwnsTheCompletionHandoff();
  void completionMayStartTheNextBatch();
  void completionMayDestroyTheCoordinator();
  void releasingAnUnusedExternalBatchStartsWaitingCommands();
  void failedStartupStillTransfersTheExternalReservation();
  void failingObserversDoNotSuppressOtherCompletions();
};

void FileStagingCoordinatorTests::availabilityObserversMayDestroyTheCoordinator_data() {
  QTest::addColumn<bool>("external");
  QTest::addColumn<bool>("onStart");
  for (const bool external : {false, true})
    for (const bool onStart : {false, true})
      QTest::newRow(qUtf8Printable(QStringLiteral("external-%1-on-start-%2").arg(external).arg(onStart))) << external << onStart;
}
void FileStagingCoordinatorTests::availabilityObserversMayDestroyTheCoordinator() {
  QFETCH(bool, external);
  QFETCH(bool, onStart);
  auto coordinator = std::make_unique<FileStagingCoordinator>([](auto completion, auto) {
    auto operation = std::make_unique<ControlledOperation>();
    operation->onStart = [completion] { completion(success()); };
    return operation;
  });
  connect(coordinator.get(), &FileStagingCoordinator::activeChanged, this, [&](bool active) {
    if (active == onStart) coordinator.reset();
  });
  if (external) {
    const auto batch = coordinator->reserveExternalBatch([](const auto&, auto) {});
  } else {
    coordinator->requestCommit([](const auto&) {});
  }
  QVERIFY(!coordinator);
}

void FileStagingCoordinatorTests::suspendedRequestsResumeAsOneBatch_data() {
  QTest::addColumn<bool>("synchronous");
  QTest::newRow("synchronous") << true;
  QTest::newRow("asynchronous") << false;
}
void FileStagingCoordinatorTests::suspendedRequestsResumeAsOneBatch() {
  QFETCH(bool, synchronous);
  ControlledOperation* running = nullptr;
  int factories = 0;
  FileStagingCoordinator coordinator([&](auto completion, auto) {
    ++factories;
    auto operation = std::make_unique<ControlledOperation>();
    operation->completion = completion;
    if (synchronous) operation->onStart = [completion] { completion(success()); };
    running = operation.get();
    return operation;
  });
  QSignalSpy active(&coordinator, &FileStagingCoordinator::activeChanged);
  coordinator.setSuspended(true);
  QVERIFY_THROWS_EXCEPTION(std::logic_error, static_cast<void>(coordinator.reserveExternalBatch([](const auto&, auto) {})));
  int completed = 0;
  for (int i = 0; i < 3; ++i) coordinator.requestCommit([&](const auto&) { ++completed; });
  QCOMPARE(factories, 0);
  QVERIFY(active.isEmpty());
  coordinator.setSuspended(false);
  QCOMPARE(factories, 1);
  if (!synchronous) {
    QCOMPARE(completed, 0);
    QVERIFY(coordinator.active());
    running->completion(success());
  }
  QCOMPARE(completed, 3);
  QCOMPARE(active.size(), 2);
  QCOMPARE(active[0][0].toBool(), true);
  QCOMPARE(active[1][0].toBool(), false);
  coordinator.setSuspended(false);
  QCOMPARE(factories, 1);
}

void FileStagingCoordinatorTests::synchronousCompletionReleasesOwnership() {
  int factories = 0;
  FileStagingCoordinator coordinator([&](auto completion, auto) {
    ++factories;
    auto operation = std::make_unique<ControlledOperation>();
    operation->onStart = [completion] { completion(success()); };
    return operation;
  });
  int completed = 0;
  for (int batch = 0; batch < 2; ++batch) {
    coordinator.requestCommit([&](const auto& result) {
      QCOMPARE(result.command.committedFiles, std::size_t{3});
      QVERIFY(!coordinator.active());
      ++completed;
    });
    QVERIFY(!coordinator.active());
  }
  QCOMPARE(completed, 2);
  QCOMPARE(factories, 2);
}

void FileStagingCoordinatorTests::startupReentrancyJoinsTheSameBatch() {
  FileStagingCoordinator* owner = nullptr;
  ControlledOperation* running = nullptr;
  int factories = 0;
  int completed = 0;
  FileStagingCoordinator coordinator([&](auto completion, auto progress) {
    ++factories;
    auto operation = std::make_unique<ControlledOperation>();
    operation->completion = completion;
    running = operation.get();
    operation->onStart = [&, progress] {
      QVERIFY(owner->active());
      progress(1, 3);
    };
    return operation;
  });
  owner = &coordinator;
  connect(&coordinator, &FileStagingCoordinator::progressChanged, &coordinator, [&] { coordinator.requestCommit([&](const auto&) { ++completed; }); });
  coordinator.requestCommit([&](const auto&) { ++completed; });
  QCOMPARE(factories, 1);
  QVERIFY(coordinator.active());
  QCOMPARE(completed, 0);
  running->completion(success());
  QCOMPARE(completed, 2);
  QVERIFY(!coordinator.active());
}

void FileStagingCoordinatorTests::startupFailureNotifiesEveryWaiter_data() {
  QTest::addColumn<bool>("nullOperation");
  QTest::addColumn<bool>("factoryFailure");
  QTest::addColumn<bool>("unknown");
  QTest::newRow("throwing-start") << false << false << false;
  QTest::newRow("missing-operation") << true << false << false;
  QTest::newRow("throwing-factory") << false << true << false;
  QTest::newRow("unknown-start") << false << false << true;
  QTest::newRow("unknown-factory") << false << true << true;
}

void FileStagingCoordinatorTests::startupFailureNotifiesEveryWaiter() {
  QFETCH(bool, nullOperation);
  QFETCH(bool, factoryFailure);
  QFETCH(bool, unknown);
  const auto detail = QStringLiteral("批次启动失败\n保留工厂或 start 的完整原始错误");
  FileStagingCoordinator* owner = nullptr;
  int failures = 0;
  FileStagingCoordinator coordinator([&](auto, auto) -> std::unique_ptr<FileStagingCommandOperation> {
    if (factoryFailure) {
      if (unknown) throw 42;
      throw std::runtime_error(detail.toStdString());
    }
    if (nullOperation) return {};
    auto operation = std::make_unique<ControlledOperation>();
    operation->onStart = [&] {
      owner->requestCommit([&](const auto& result) {
        QVERIFY(result.command.outcome != uwf::app::ApplicationCommandOutcome::Succeeded);
        ++failures;
      });
      if (unknown) throw 42;
      throw std::runtime_error(detail.toStdString());
    };
    return operation;
  });
  owner = &coordinator;
  coordinator.requestCommit([&](const auto& result) {
    QVERIFY(result.command.outcome != uwf::app::ApplicationCommandOutcome::Succeeded);
    QVERIFY(!result.sourceEntries);
    QCOMPARE(result.command.failedFiles, std::size_t{0});
    QVERIFY(!result.result);
    if (!nullOperation && !unknown) QVERIFY(result.command.detail.contains(detail));
    ++failures;
  });
  QCOMPARE(failures, nullOperation || factoryFailure ? 1 : 2);
  QVERIFY(!coordinator.active());
  auto errors = uwf::recentLogLines();
  std::erase_if(errors, [](const auto& line) { return line.find(" E ") == std::string::npos; });
  QCOMPARE(errors.size(), std::size_t{1});
  QVERIFY(errors.front().find("file staging batch startup failed:") != std::string::npos);
  const auto expected =
      nullOperation ? std::string("factory returned no operation") : (unknown ? std::string("error=non-standard-exception") : detail.toStdString());
  QVERIFY(errors.front().find(expected) != std::string::npos);
}

void FileStagingCoordinatorTests::externalReservationOwnsTheCompletionHandoff() {
  ControlledOperation* operation = nullptr;
  FileStagingCoordinator coordinator([&](auto completion, auto) {
    auto next = std::make_unique<ControlledOperation>();
    next->completion = std::move(completion);
    operation = next.get();
    return next;
  });
  int completed = 0;
  coordinator.requestCommit([&](const auto&) { ++completed; });
  std::optional<FileStagingCoordinator::ExternalBatch> external;
  QVERIFY(!coordinator.reserveExternalBatch([&](const auto&, FileStagingCoordinator::ExternalBatch batch) { external = std::move(batch); }));
  operation->completion(success());
  QCOMPARE(completed, 1);
  QVERIFY(external && *external);
  QVERIFY(coordinator.active());
  coordinator.requestCommit([&](const auto&) { ++completed; });
  external->complete(success());
  QCOMPARE(completed, 2);
  QVERIFY(!coordinator.active());
}

void FileStagingCoordinatorTests::completionMayStartTheNextBatch() {
  int factories = 0;
  FileStagingCoordinator coordinator([&](auto completion, auto) {
    ++factories;
    auto operation = std::make_unique<ControlledOperation>();
    operation->onStart = [completion] { completion(success()); };
    return operation;
  });
  int completed = 0;
  coordinator.requestCommit([&](const auto&) {
    ++completed;
    coordinator.requestCommit([&](const auto&) { ++completed; });
  });
  QCOMPARE(completed, 2);
  QCOMPARE(factories, 2);
  QVERIFY(!coordinator.active());
}

void FileStagingCoordinatorTests::completionMayDestroyTheCoordinator() {
  ControlledOperation* operation = nullptr;
  auto coordinator = std::make_unique<FileStagingCoordinator>([&](auto completion, auto) {
    auto next = std::make_unique<ControlledOperation>();
    next->completion = std::move(completion);
    operation = next.get();
    return next;
  });
  coordinator->requestCommit([&](const auto&) { coordinator.reset(); });
  bool handoff = false;
  QVERIFY(!coordinator->reserveExternalBatch([&](const auto&, FileStagingCoordinator::ExternalBatch batch) {
    handoff = true;
    QVERIFY(!batch);
  }));
  operation->completion(success());
  QVERIFY(!coordinator);
  QVERIFY(handoff);
}

void FileStagingCoordinatorTests::releasingAnUnusedExternalBatchStartsWaitingCommands() {
  ControlledOperation* operation = nullptr;
  int factories = 0;
  FileStagingCoordinator coordinator([&](auto completion, auto) {
    ++factories;
    auto next = std::make_unique<ControlledOperation>();
    next->completion = std::move(completion);
    operation = next.get();
    return next;
  });
  int completed = 0;
  {
    auto external = coordinator.reserveExternalBatch([](const auto&, FileStagingCoordinator::ExternalBatch) {});
    QVERIFY(external && *external);
    coordinator.requestCommit([&](const auto&) { ++completed; });
    QCOMPARE(factories, 0);
  }
  QCOMPARE(factories, 1);
  QVERIFY(coordinator.active());
  operation->completion(success());
  QCOMPARE(completed, 1);
  QVERIFY(!coordinator.active());
}

void FileStagingCoordinatorTests::failedStartupStillTransfersTheExternalReservation() {
  FileStagingCoordinator* owner = nullptr;
  std::optional<FileStagingCoordinator::ExternalBatch> external;
  bool failedHandoff = false;
  FileStagingCoordinator coordinator([&](auto, auto) {
    auto operation = std::make_unique<ControlledOperation>();
    operation->onStart = [&] {
      QVERIFY(!owner->reserveExternalBatch([&](const auto& result, FileStagingCoordinator::ExternalBatch batch) {
        failedHandoff = result.command.outcome == ApplicationCommandOutcome::Failed;
        external = std::move(batch);
      }));
      throw std::runtime_error("start rejected after reservation");
    };
    return operation;
  });
  owner = &coordinator;
  coordinator.requestCommit([](const auto&) {});
  QVERIFY(failedHandoff);
  QVERIFY(external && *external);
  QVERIFY(coordinator.active());
  external->complete(success());
  QVERIFY(!coordinator.active());
}

void FileStagingCoordinatorTests::failingObserversDoNotSuppressOtherCompletions() {
  ControlledOperation* operation = nullptr;
  FileStagingCoordinator coordinator([&](auto completion, auto) {
    auto next = std::make_unique<ControlledOperation>();
    next->completion = std::move(completion);
    operation = next.get();
    return next;
  });
  coordinator.requestCommit([](const auto&) { throw std::runtime_error("observer rejected"); });
  int completed = 0;
  coordinator.requestCommit([&](const auto&) { ++completed; });
  const QPointer<ControlledOperation> lifetime(operation);
  operation->completion(success());
  QCOMPARE(completed, 1);
  QVERIFY(!coordinator.active());
  QVERIFY(lifetime);
  QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
  QVERIFY(!lifetime);
}
}  // namespace

QTEST_GUILESS_MAIN(FileStagingCoordinatorTests)
#include "FileStagingCoordinatorTests.moc"
