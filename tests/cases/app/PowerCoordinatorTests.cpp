// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QCoreApplication>
#include <QTest>
#include <memory>
#include <stdexcept>

#include "app/power/PowerCoordinator.h"
#include "windows/commit/WindowsCommitTraversalSource.h"

namespace {
using namespace uwf;
using namespace uwf::app;

class UnusedWmiOperations final : public uwf::app::wmi::WmiOperations {
 public:
  mutable int calls = 0;
  [[noreturn]] void unexpected() const {
    ++calls;
    throw std::logic_error("unexpected WMI operation");
  }
  void ensureConnected() const override { unexpected(); }
  std::vector<uwf::app::wmi::WmiRow> query(const std::string&) const override { unexpected(); }
  std::vector<uwf::app::wmi::WmiRow> queryInstances(const std::string&) const override { unexpected(); }
  uwf::app::wmi::WmiClassStatus classStatus(const std::string&) const override { unexpected(); }
  uwf::app::wmi::WmiRow getObject(const std::string&) const override { unexpected(); }
  void invokeMethod(const std::string&, const std::string&, const uwf::app::wmi::WmiRow&) const override { unexpected(); }
  uwf::app::wmi::WmiMethodOutput callMethodRead(const std::string&, const std::string&, const uwf::app::wmi::WmiRow&) const override { unexpected(); }
  uwf::app::wmi::WmiMethodOutput callMethodReadCancelable(const std::string&, const std::string&, const uwf::app::wmi::WmiRow&,
                                                          std::stop_token) const override {
    unexpected();
  }
  void putInstance(const std::string&, const uwf::app::wmi::WmiRow&, uwf::app::wmi::WmiPutMode) const override { unexpected(); }
};
class UnusedStore final : public FileStagingStore {
 public:
  mutable int calls = 0;
  QList<FileStagingEntry> load() const override {
    ++calls;
    throw std::logic_error("unexpected staging read");
  }
  void replace(const QList<FileStagingEntry>&) override {
    ++calls;
    throw std::logic_error("unexpected staging write");
  }
};
class ControlledOperation final : public QObject, public FileStagingCommandOperation {
 public:
  FileStagingCoordinator::Completion completed;
  void start() override {}
  void disposeAfterCallback() override { deleteLater(); }
};

class PowerCoordinatorTests final : public QObject {
  Q_OBJECT
 private slots:
  void infrastructureFailureCannotContinueWithAnInventedTargetCount_data() {
    QTest::addColumn<bool>("shutdown");
    QTest::newRow("safe-shutdown") << true;
    QTest::newRow("safe-restart") << false;
  }
  void infrastructureFailureCannotContinueWithAnInventedTargetCount() {
    QFETCH(bool, shutdown);
    UnusedWmiOperations wmi;
    UnusedStore store;
    ControlledOperation* operation = nullptr;
    FileStagingCoordinator staging([&](auto completion, auto) {
      auto next = std::make_unique<ControlledOperation>();
      next->completed = std::move(completion);
      operation = next.get();
      return next;
    });
    staging.requestCommit([](const auto&) {});
    QString warning;
    int confirmations = 0;
    PowerInteraction interaction;
    interaction.confirm = [&](PowerActionDialogRequest request) {
      ++confirmations;
      if (!request.completedStagingFailure || request.pollStagingPreparation) throw std::logic_error("expected reused staging infrastructure failure");
      return PowerActionDialogOutcome::ContinuedAfterStagingFailure;
    };
    interaction.warning = [&](const QString&, const QString& text) { warning = text; };
    PowerCoordinator power({wmi, store, UwfCapability::Available, staging, windows::systemCommitTraversalSource()}, std::move(interaction));
    if (shutdown)
      power.safeShutdown();
    else
      power.safeRestart();
    FileStagingBatchResult failed;
    failed.command = ApplicationCommandResult::infrastructureFailure(QStringLiteral("staging infrastructure failed"));
    // Counts from an older or malformed producer cannot turn Failed into a
    // completed item batch that is eligible for the user's continuation choice.
    failed.command.failedFiles = 1;
    operation->completed(failed);
    QTRY_VERIFY(!warning.isEmpty());
    QCOMPARE(confirmations, 1);
    QVERIFY(warning.contains(QStringLiteral("cannot approve a file staging failure without a recorded failed target")));
    QCOMPARE(wmi.calls, 0);
    QCOMPARE(store.calls, 0);
    QVERIFY(!staging.active());
  }
  void waitingActionsRespectTheirOwnerLifetime_data() {
    QTest::addColumn<bool>("destroyBeforeCompletion");
    QTest::addColumn<bool>("destroyBeforeDialog");
    QTest::addColumn<bool>("stagingFailure");
    QTest::newRow("live-owner-success") << false << false << false;
    QTest::newRow("live-owner-failure") << false << false << true;
    QTest::newRow("destroy-before-completion") << true << false << false;
    QTest::newRow("destroy-before-dialog") << false << true << false;
  }
  void waitingActionsRespectTheirOwnerLifetime() {
    QTest::failOnWarning();
    QFETCH(bool, destroyBeforeCompletion);
    QFETCH(bool, destroyBeforeDialog);
    QFETCH(bool, stagingFailure);
    UnusedWmiOperations wmi;
    UnusedStore store;
    ControlledOperation* operation = nullptr;
    FileStagingCoordinator staging([&](auto completion, auto) {
      auto next = std::make_unique<ControlledOperation>();
      next->completed = std::move(completion);
      operation = next.get();
      return next;
    });
    int commandCompletions = 0;
    staging.requestCommit([&](const auto&) { ++commandCompletions; });
    int confirmations = 0;
    bool reusedFailure = false;
    PowerInteraction interaction;
    interaction.confirm = [&](PowerActionDialogRequest request) {
      ++confirmations;
      reusedFailure = request.completedStagingFailure.has_value();
      return PowerActionDialogOutcome::Canceled;
    };
    auto power = std::make_unique<PowerCoordinator>(
        PowerServices{wmi, store, uwf::app::UwfCapability::Available, staging, uwf::windows::systemCommitTraversalSource()}, std::move(interaction));
    power->safeRestart();
    power->safeShutdown();
    if (destroyBeforeCompletion) power.reset();
    FileStagingBatchResult result;
    if (stagingFailure)
      result.command = ApplicationCommandResult::infrastructureFailure(QStringLiteral("staging rejected"));
    else
      result.sourceEntries = QList<FileStagingEntry>{};
    operation->completed(result);
    QCOMPARE(commandCompletions, 1);
    if (destroyBeforeDialog) power.reset();
    if (!destroyBeforeCompletion && !destroyBeforeDialog) {
      QTRY_COMPARE(confirmations, 1);
      QCOMPARE(reusedFailure, stagingFailure);
    } else {
      QCoreApplication::processEvents();
      QCOMPARE(confirmations, 0);
    }
    QTRY_VERIFY(!staging.active());
    QCOMPARE(wmi.calls, 0);
    QCOMPARE(store.calls, 0);
  }
};
}  // namespace

QTEST_GUILESS_MAIN(PowerCoordinatorTests)
#include "PowerCoordinatorTests.moc"
