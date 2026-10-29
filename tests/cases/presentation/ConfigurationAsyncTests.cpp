// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QElapsedTimer>
#include <QScopeGuard>
#include <QSignalSpy>
#include <QTest>
#include <QThread>
#include <QThreadPool>
#include <QTimer>

#include "presentation/configuration/ApplyPlanViewModel.h"
#include "presentation/overlay/OverlayViewModel.h"
#include "presentation/shell/MainWindowViewModel.h"
#include "support/app/ConfigurationFixture.h"
#include "support/app/WorkerGate.h"

using namespace uwf;
namespace {
class StateSource final : public app::ApplicationStateSource {
 public:
  ReadOperation next;
  ReadOperation readOperation(app::UwfCapability) override { return next; }
};
class UsageSource final : public app::OverlayUsageReader {
 public:
  std::shared_ptr<test::WorkerGate> gate;
  app::OverlayUsageState read() const override {
    gate->wait({}, false);
    return app::OverlayUsageUnavailable{};
  }
};
class Actions final : public app::ConfigurationActions {
 public:
  std::shared_ptr<test::WorkerGate> gate;
  int prepares = 0;
  int completions = 0;
  int outcome = 0;
  ApplyOperation prepareApply(const core::PendingChanges&, const core::UwfSnapshot&) override {
    ++prepares;
    if (outcome == 3) throw std::runtime_error("prepare failed");
    return [gate = gate, outcome = outcome] {
      gate->wait({}, false);
      if (outcome == 2) throw std::runtime_error("worker failed");
      if (outcome == 1) return app::ConfigurationApplyResult{};
      return app::ConfigurationApplyResult{{}, true, true};
    };
  }
  void completeApply(const app::ConfigurationApplyResult&) override { ++completions; }
};
class ResultActions final : public app::ConfigurationActions {
 public:
  app::ConfigurationApplyResult result;
  ApplyOperation prepareApply(const core::PendingChanges&, const core::UwfSnapshot&) override {
    return [result = result] { return result; };
  }
};
}  // namespace
class ConfigurationAsyncTests final : public QObject {
  Q_OBJECT
 private slots:
  void applyResultsDescribeTheOutcomeAndRestartAvailability_data() {
    QTest::addColumn<bool>("connected");
    QTest::addColumn<int>("failures");
    QTest::addColumn<bool>("written");
    QTest::addColumn<bool>("failed");
    QTest::addColumn<bool>("restart");
    QTest::newRow("connection-failed") << false << 0 << false << true << false;
    QTest::newRow("all-applied") << true << 0 << true << false << true;
    QTest::newRow("partially-applied") << true << 1 << true << true << true;
    QTest::newRow("none-applied") << true << 1 << false << true << false;
    QTest::newRow("no-write-needed") << true << 0 << false << false << false;
  }
  void applyResultsDescribeTheOutcomeAndRestartAvailability() {
    QFETCH(bool, connected);
    QFETCH(int, failures);
    QFETCH(bool, written);
    QFETCH(bool, failed);
    QFETCH(bool, restart);
    auto actions = std::make_shared<ResultActions>();
    actions->result.connectionEstablished = connected;
    actions->result.failedOperations = failures;
    actions->result.anyWriteConfirmed = written;
    core::PendingChanges changes;
    changes.setFilterEnabled = true;
    presentation::ApplyPlanViewModel model(changes, {}, actions);
    QSignalSpy results(&model, &presentation::ApplyPlanViewModel::resultChanged);
    model.apply();
    QTRY_COMPARE(results.count(), 1);
    QCOMPARE(model.resultFailed(), failed);
    QCOMPARE(model.canRestart(), restart);
    QCOMPARE(model.resultHeading(), failed ? QStringLiteral("Some changes could not be applied") : QStringLiteral("Configuration applied"));
    QCOMPARE(model.resultDescription(),
             failed ? QStringLiteral("Review the operation details. Read the current system state before retrying changes that may have been written.")
                    : QStringLiteral("The configuration has been written to the system. Most changes take effect after restart."));
  }
  void applySessionOwnsAllInteractionGates_data() {
    QTest::addColumn<int>("outcome");
    QTest::addColumn<bool>("reconciliationReadFails");
    for (int outcome = 0; outcome < 4; ++outcome) {
      QTest::newRow(qUtf8Printable(QStringLiteral("outcome-%1").arg(outcome))) << outcome << false;
      if (outcome == 0 || outcome == 2) QTest::newRow(qUtf8Printable(QStringLiteral("outcome-%1-read-fails").arg(outcome))) << outcome << true;
    }
  }
  void applySessionOwnsAllInteractionGates() {
    QFETCH(int, outcome);
    QFETCH(bool, reconciliationReadFails);
    auto gate = std::make_shared<test::WorkerGate>();
    const auto unblock = qScopeGuard([&] {
      gate->release();
      QThreadPool::globalInstance()->waitForDone(2000);
    });
    test::application::MutableApplicationStateSource source;
    source.snapshot = test::application::editableSnapshot();
    presentation::MainWindowViewModel owner(source, app::UwfCapability::Available);
    owner.initializeConfiguration(16384, QStringLiteral("C:"));
    owner.refresh();
    QTRY_VERIFY(!owner.refreshing());
    QVERIFY(owner.configurationWritesAllowed());
    core::PendingChanges changes;
    changes.setFilterEnabled = false;
    auto actions = std::make_shared<Actions>();
    actions->gate = gate;
    actions->outcome = outcome;
    auto plan = std::make_shared<presentation::ApplyPlanViewModel>(changes, source.snapshot, actions);
    owner.trackApplyPlan(plan);
    owner.setExternalOperationActive(true);
    QVERIFY(!plan->canApply());
    plan->apply();
    QCOMPARE(actions->prepares, 0);
    owner.setExternalOperationActive(false);
    QVERIFY(plan->canApply());
    bool busyObserved = false;
    bool resultObserved = false;
    connect(&owner, &presentation::MainWindowViewModel::configurationApplyingChanged, &owner, [&](bool applying) {
      if (!applying) return;
      busyObserved = true;
      QVERIFY(!owner.exitAllowed());
      QVERIFY(!owner.powerActionsAllowed());
      QVERIFY(!owner.configurationWritesAllowed());
      QVERIFY(!owner.discardPendingChanges());
      owner.refresh();
      QVERIFY(!owner.refreshing());
      plan->apply();  // Reentrant input cannot prepare a second write.
    });
    connect(plan.get(), &presentation::ApplyPlanViewModel::resultChanged, &owner, [&] {
      resultObserved = true;
      QCOMPARE(owner.state().reconciliationRequired(), outcome == 0 || outcome == 2);
    });
    if (reconciliationReadFails) source.failure = QStringLiteral("reconciliation read failed");
    plan->apply();
    QVERIFY(busyObserved);
    QCOMPARE(actions->prepares, 1);
    if (outcome != 3) {
      QTRY_COMPARE(gate->started.load(), 1);
      QVERIFY(owner.applyingConfiguration());
      QVERIFY(!resultObserved);
      gate->release();
    }
    QTRY_VERIFY(resultObserved);
    QTRY_VERIFY(!owner.refreshing());
    QVERIFY(!owner.applyingConfiguration());
    QVERIFY(owner.exitAllowed());
    QVERIFY(owner.powerActionsAllowed());
    QCOMPARE(owner.configurationWritesAllowed(), !reconciliationReadFails);
    if (reconciliationReadFails) {
      QVERIFY(!plan->canApply());
      plan->apply();
      QCOMPARE(actions->prepares, 1);
    }
    QCOMPARE(actions->completions, 1);
    QCOMPARE(source.reads, outcome == 0 || outcome == 2 ? 2 : 1);
  }
  void readsAndWritesKeepTheEventLoopResponsive_data() {
    QTest::addColumn<int>("operation");
    QTest::addColumn<bool>("destroy");
    for (int operation = 0; operation < 3; ++operation)
      for (const bool destroy : {false, true})
        QTest::newRow(qPrintable(QStringLiteral("operation-%1-destroy-%2").arg(operation).arg(destroy))) << operation << destroy;
  }
  void readsAndWritesKeepTheEventLoopResponsive() {
    QFETCH(int, operation);
    QFETCH(bool, destroy);
    auto gate = std::make_shared<test::WorkerGate>();
    const auto unblock = qScopeGuard([&] {
      gate->release();
      QThreadPool::globalInstance()->waitForDone(2000);
    });
    StateSource source;
    source.next = [gate](std::stop_token) {
      gate->wait({}, false);
      return app::ApplicationState{};
    };
    std::unique_ptr<QObject> model;
    std::function<void()> start;
    int delivered = 0;
    bool wrongThread = false;
    const auto completed = [&] {
      ++delivered;
      wrongThread |= QThread::currentThread() != thread();
    };
    if (operation == 0) {
      auto value = std::make_unique<presentation::MainWindowViewModel>(source, app::UwfCapability::Available);
      connect(value.get(), &presentation::MainWindowViewModel::stateCommitted, this, completed, Qt::DirectConnection);
      start = [target = value.get()] { target->refresh(); };
      model = std::move(value);
    } else if (operation == 1) {
      auto reader = std::make_shared<UsageSource>();
      reader->gate = gate;
      auto value = std::make_unique<presentation::OverlayViewModel>(reader);
      core::UwfSnapshot snapshot;
      snapshot.uwfAvailable = true;
      value->applySnapshot(snapshot);
      connect(value.get(), &presentation::OverlayViewModel::usageChanged, this, completed, Qt::DirectConnection);
      start = [target = value.get()] { target->refreshUsage(); };
      model = std::move(value);
    } else {
      auto actions = std::make_shared<Actions>();
      actions->gate = gate;
      core::PendingChanges changes;
      changes.setFilterEnabled = true;
      auto value = std::make_unique<presentation::ApplyPlanViewModel>(changes, core::UwfSnapshot{}, actions);
      connect(value.get(), &presentation::ApplyPlanViewModel::resultChanged, this, completed, Qt::DirectConnection);
      start = [target = value.get()] {
        target->apply();
        target->apply();
      };
      model = std::move(value);
    }
    start();
    QTRY_COMPARE(gate->started.load(), 1);
    bool heartbeat = false;
    QTimer::singleShot(0, this, [&] { heartbeat = true; });
    QTRY_VERIFY(heartbeat);
    QCOMPARE(gate->finished.load(), 0);
    if (destroy) {
      QElapsedTimer elapsed;
      elapsed.start();
      model.reset();
      QVERIFY(elapsed.elapsed() < 100);
    }
    gate->release();
    QTRY_COMPARE(gate->finished.load(), 1);
    if (destroy) {
      QCoreApplication::sendPostedEvents();
      QCOMPARE(delivered, 0);
    } else if (operation != 1)
      QTRY_COMPARE(delivered, 1);
    QVERIFY(!wrongThread);
  }
  void supersededStateCannotReplaceTheLatestSnapshot() {
    auto gate = std::make_shared<test::WorkerGate>();
    const auto unblock = qScopeGuard([&] {
      gate->release();
      QThreadPool::globalInstance()->waitForDone(2000);
    });
    StateSource source;
    source.next = [gate](std::stop_token) {
      gate->wait({}, false);
      return app::ApplicationState{};
    };
    presentation::MainWindowViewModel model(source, app::UwfCapability::Available);
    QSignalSpy committed(&model, &presentation::MainWindowViewModel::stateCommitted);
    model.refresh();
    QTRY_COMPARE(gate->started.load(), 1);
    source.next = [](std::stop_token) {
      app::ApplicationState state;
      state.snapshot.uwfAvailable = true;
      return state;
    };
    model.refresh();
    QTRY_COMPARE(committed.size(), 1);
    QVERIFY(model.state().snapshot().uwfAvailable);
    gate->release();
    QTRY_COMPARE(gate->finished.load(), 1);
    QCoreApplication::sendPostedEvents();
    QCOMPARE(committed.size(), 1);
    QVERIFY(model.state().snapshot().uwfAvailable);
  }
};
QTEST_GUILESS_MAIN(ConfigurationAsyncTests)
#include "ConfigurationAsyncTests.moc"
