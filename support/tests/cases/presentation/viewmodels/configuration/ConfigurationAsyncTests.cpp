// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QElapsedTimer>
#include <QScopeGuard>
#include <QSignalSpy>
#include <QTest>
#include <QThread>
#include <QThreadPool>
#include <QTimer>
#include <atomic>
#include <exception>
#include <memory>
#include <thread>
#include <utility>

#include "application/configuration/execution/ConfigurationExecutor.h"
#include "application/staging/editing/FileStagingDraft.h"
#include "application/staging/persistence/FileStagingRegistry.h"
#include "application/state/contracts/ApplicationStateSource.h"
#include "application/uwf/adapters/UwfConfigurationRepository.h"
#include "foundation/logging/Log.h"
#include "presentation/viewmodels/configuration/ApplyPlanViewModel.h"
#include "presentation/viewmodels/overlay/OverlayViewModel.h"
#include "presentation/viewmodels/shell/MainWindowViewModel.h"
#include "simulation/app/ConfigurationState.h"
#include "simulation/uwf/FakeUwfBackend.h"
#include "support/app/ConfigurationFixture.h"
#include "support/app/FileTargetFixture.h"
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
  std::exception_ptr prepareFailure;
  std::exception_ptr workerFailure;
  ApplyOperation prepareApply(const core::PendingChanges&, const core::UwfSnapshot&) override {
    ++prepares;
    if (prepareFailure) std::rethrow_exception(prepareFailure);
    if (outcome == 3) throw std::runtime_error("prepare failed");
    return [gate = gate, outcome = outcome, failure = workerFailure] {
      gate->wait({}, false);
      if (failure) std::rethrow_exception(failure);
      if (outcome == 2) throw std::runtime_error("worker failed");
      app::ConfigurationApplyResult result;
      result.steps.push_back({"Filter", app::ConfigurationStepOutcome::Succeeded, outcome == 1 ? app::WriteEffect::None : app::WriteEffect::Confirmed,
                              app::ConfigurationOperation{app::ConfigurationOperationKind::Filter, {}}});
      return result;
    };
  }
  void completeApply(const app::ConfigurationApplyResult&) override { ++completions; }
};
class ResultActions final : public app::ConfigurationActions {
 public:
  app::ConfigurationApplyResult result;
  bool logFailure = false;
  ApplyOperation prepareApply(const core::PendingChanges&, const core::UwfSnapshot&) override {
    return [result = result, logFailure = logFailure] {
      if (logFailure) UWF_LOG_E("test-backend") << "owned configuration failure";
      return result;
    };
  }
};

class ApplyStagingSource final : public app::CommitTraversalSource {
 public:
  const std::shared_ptr<test::WorkerGate> preparationGate = std::make_shared<test::WorkerGate>();
  const std::shared_ptr<test::WorkerGate> readbackGate = std::make_shared<test::WorkerGate>();
  std::atomic_bool watching{false};
  std::atomic_bool replaced{false};
  mutable std::atomic_int ownerCalls{0};
  mutable std::atomic_int preparationCalls{0};
  mutable std::atomic_int readbackCalls{0};
  app::FileCommitTarget resolveFileTarget(const QString& path) const override {
    observe();
    return test::application::memoryFileTarget(path);
  }
  std::string driveLetterForPath(const QString& path) const override {
    observe();
    return path.left(2).toUpper().toStdString();
  }
  std::optional<app::CommitNode> inspect(const app::CommitNode&) const override { throw std::logic_error("Unexpected staging inspection"); }
  bool exists(const app::CommitNode&) const override { throw std::logic_error("Unexpected staging existence read"); }
  QString enumerateChildren(const app::CommitNode&, std::stop_token, const app::CommitNodeVisitor&) const override {
    throw std::logic_error("Unexpected staging traversal");
  }

 private:
  void observe() const {
    if (!watching) return;
    // A regression must fail an assertion rather than block the test's owner thread.
    if (std::this_thread::get_id() == m_owner) {
      ++ownerCalls;
      return;
    }
    if (replaced) {
      ++readbackCalls;
      if (m_blockReadback.exchange(false)) readbackGate->wait({}, false);
    } else {
      ++preparationCalls;
      if (m_blockPreparation.exchange(false)) preparationGate->wait({}, false);
    }
  }
  const std::thread::id m_owner = std::this_thread::get_id();
  mutable std::atomic_bool m_blockPreparation{true};
  mutable std::atomic_bool m_blockReadback{true};
};

class ApplyStagingStore final : public app::FileStagingStore {
 public:
  explicit ApplyStagingStore(std::shared_ptr<ApplyStagingSource> source) : m_source(std::move(source)) {}
  QList<app::FileStagingEntry> entries;
  std::atomic_int writes{0};
  QList<app::FileStagingEntry> load() const override { return entries; }
  void replace(const QList<app::FileStagingEntry>& replacement) override {
    entries = replacement;
    // Persisted data does not contain the editing session's host observations.
    for (auto& entry : entries) entry.fileTarget.reset();
    ++writes;
    m_source->replaced = true;
  }

 private:
  std::shared_ptr<ApplyStagingSource> m_source;
};

QStringList logRecords(const char level) {
  QStringList result;
  const auto marker = std::string(" ") + level + ' ';
  for (const auto& line : recentLogLines())
    if (line.find(marker) < line.find(']')) result.append(QString::fromStdString(line));
  return result;
}
}  // namespace
class ConfigurationAsyncTests final : public QObject {
  Q_OBJECT
 private slots:
  void stagingApplyPreparesAndProjectsItsReceiptOnTheWorker() {
    auto source = std::make_shared<ApplyStagingSource>();
    auto store = std::make_shared<ApplyStagingStore>(source);
    const app::FileStagingEntry original{app::FileStagingKind::File, QStringLiteral("C:\\Data\\original.bin")};
    const app::FileStagingEntry added{app::FileStagingKind::File, QStringLiteral("C:\\Data\\added.bin")};
    store->entries = {original};
    auto draft = std::make_shared<app::FileStagingDraft>(store, source);
    static_cast<void>(draft->load());
    draft->replace({original, added});
    QVERIFY(draft->changes());
    auto snapshot = uwf::simulation::application::editableSnapshot();
    snapshot.current.registryExclusions = snapshot.next.registryExclusions = {std::string(app::kFileStagingRegistryRoot)};
    uwf::simulation::FakeUwfBackend::State backendState;
    backendState.current.registryExclusions = backendState.next.registryExclusions = snapshot.current.registryExclusions;
    auto backend = std::make_shared<uwf::simulation::FakeUwfBackend>(std::move(backendState));
    auto actions = std::make_shared<app::ConfigurationExecutor>(app::uwfConfigurationRepository(backend, draft));
    presentation::ApplyPlanViewModel model({}, snapshot, actions, draft->changes());
    QVERIFY2(model.canApply(), qPrintable(model.warning()));
    QSignalSpy results(&model, &presentation::ApplyPlanViewModel::resultChanged);
    const auto unblock = qScopeGuard([&] {
      source->preparationGate->release();
      source->readbackGate->release();
      QThreadPool::globalInstance()->waitForDone(2000);
    });
    source->watching = true;
    model.apply();
    QCOMPARE(source->ownerCalls.load(), 0);
    QTRY_COMPARE(source->preparationGate->started.load(), 1);
    QVERIFY(model.applying());
    QCOMPARE(store->writes.load(), 0);
    bool preparationHeartbeat = false;
    QTimer::singleShot(0, &model, [&] { preparationHeartbeat = true; });
    QTRY_VERIFY(preparationHeartbeat);
    QCOMPARE(source->preparationGate->finished.load(), 0);

    source->preparationGate->release();
    QTRY_COMPARE(source->readbackGate->started.load(), 1);
    QCOMPARE(store->writes.load(), 1);
    QVERIFY(model.applying());
    QCOMPARE(results.count(), 0);
    bool readbackHeartbeat = false;
    QTimer::singleShot(0, &model, [&] { readbackHeartbeat = true; });
    QTRY_VERIFY(readbackHeartbeat);
    QCOMPARE(source->readbackGate->finished.load(), 0);
    source->readbackGate->release();
    QTRY_COMPARE(results.count(), 1);
    QVERIFY2(!model.resultFailed(), qPrintable(model.resultText()));
    QVERIFY(!model.applying());
    QVERIFY(model.result().staging());
    QVERIFY(model.result().staging()->attempted);
    QVERIFY(!draft->changes());
    const auto baseline = draft->baseline();
    QCOMPARE(baseline.size(), qsizetype{2});
    for (const auto& entry : baseline) QVERIFY(entry.fileTarget);
    QCOMPARE(source->ownerCalls.load(), 0);
    QVERIFY(source->preparationCalls.load() > 0);
    QVERIFY(source->readbackCalls.load() > 0);
  }
  void aFailedInitialReadDisablesWritesUntilARealSnapshotIsLoaded() {
    StateSource source;
    source.next = [](std::stop_token) -> app::ApplicationState { throw std::runtime_error("Temporarily offline"); };
    presentation::MainWindowViewModel model(source, app::UwfCapability::Available);
    model.initializeConfiguration(16384, QStringLiteral("C:"));
    QSignalSpy failures(&model, &presentation::MainWindowViewModel::refreshFailed);
    model.refresh();
    QTRY_COMPARE(failures.count(), 1);
    QVERIFY(model.workspaceDiagnostic().has_value());
    QCOMPARE(*model.workspaceDiagnostic(), QStringLiteral("Temporarily offline"));
    QVERIFY(!model.state().hasCommittedState());
    QVERIFY(!model.configurationWritesAllowed());
    QVERIFY(!model.configuration()->global()->controlsEnabled());
    auto snapshot = uwf::simulation::application::editableSnapshot();
    QVERIFY(snapshot.current.filter.enabled);
    source.next = [snapshot](std::stop_token) { return app::ApplicationState{{}, snapshot, {}}; };
    model.refresh();
    QTRY_VERIFY(!model.refreshing());
    QVERIFY(model.state().hasCommittedState());
    QVERIFY(model.state().snapshot().current.filter.enabled);
    QVERIFY(model.configurationWritesAllowed());
    QVERIFY(!model.workspaceDiagnostic());
  }
  void failedReadDiagnosticsSurviveInteractionChangesUntilAReadSucceeds_data() {
    QTest::addColumn<bool>("initialRead");
    QTest::addColumn<bool>("emptyError");
    QTest::newRow("initial-failure") << true << false;
    QTest::newRow("retained-snapshot") << false << false;
    QTest::newRow("initial-empty-error") << true << true;
    QTest::newRow("retained-snapshot-empty-error") << false << true;
  }
  void failedReadDiagnosticsSurviveInteractionChangesUntilAReadSucceeds() {
    QFETCH(bool, initialRead);
    QFETCH(bool, emptyError);
    StateSource source;
    const auto snapshot = uwf::simulation::application::editableSnapshot();
    const auto successfulRead = [snapshot](std::stop_token) { return app::ApplicationState{{}, snapshot, {}}; };
    source.next = successfulRead;
    presentation::MainWindowViewModel model(source, app::UwfCapability::Available);
    model.initializeConfiguration(16384, QStringLiteral("C:"));
    if (!initialRead) {
      model.refresh();
      QTRY_VERIFY(!model.refreshing());
      QVERIFY(model.state().hasCommittedState());
      QVERIFY(!model.workspaceDiagnostic());
    }
    bool terminalAvailabilityObserved = false;
    bool failureObserved = false;
    const auto availabilityConnection = connect(&model, &presentation::MainWindowViewModel::interactionAvailabilityChanged, &model, [&] {
      if (model.refreshing()) return;
      terminalAvailabilityObserved = true;
      QVERIFY(model.workspaceDiagnostic());
      QVERIFY(!model.workspaceDiagnostic()->isEmpty());
    });
    connect(&model, &presentation::MainWindowViewModel::refreshFailed, &model, [&] {
      failureObserved = true;
      QVERIFY(model.workspaceDiagnostic());
      QVERIFY(!model.workspaceDiagnostic()->isEmpty());
    });
    source.next = [emptyError](std::stop_token) -> app::ApplicationState { throw std::runtime_error(emptyError ? "" : "Provider temporarily unavailable"); };
    model.refresh();
    QTRY_VERIFY(failureObserved);
    QVERIFY(terminalAvailabilityObserved);
    QCOMPARE(model.state().hasCommittedState(), !initialRead);
    const auto diagnostic = model.workspaceDiagnostic();
    QVERIFY(diagnostic && !diagnostic->isEmpty());
    if (!emptyError) QCOMPARE(*diagnostic, QStringLiteral("Provider temporarily unavailable"));
    // Busy-state changes and retranslation must preserve the last read failure.
    model.setExternalOperationActive(true);
    QCOMPARE(model.workspaceDiagnostic(), diagnostic);
    model.setExternalOperationActive(false);
    QCOMPARE(model.workspaceDiagnostic(), diagnostic);
    model.retranslate();
    QCOMPARE(model.workspaceDiagnostic(), diagnostic);
    disconnect(availabilityConnection);
    auto gate = std::make_shared<test::WorkerGate>();
    const auto unblock = qScopeGuard([&] {
      gate->release();
      QThreadPool::globalInstance()->waitForDone(2000);
    });
    source.next = [successfulRead, gate](std::stop_token stop) {
      gate->wait(stop, false);
      return successfulRead(stop);
    };
    bool recoveryObserved = false;
    connect(&model, &presentation::MainWindowViewModel::stateCommitted, &model, [&] {
      recoveryObserved = true;
      QVERIFY(!model.workspaceDiagnostic());
    });
    model.refresh();
    QTRY_COMPARE(gate->started.load(), 1);
    QCOMPARE(model.workspaceDiagnostic(), diagnostic);
    gate->release();
    QTRY_VERIFY(recoveryObserved);
    QVERIFY(!model.workspaceDiagnostic());
  }
  void aCompletedUnavailableSnapshotStillPublishesADiagnostic_data() {
    QTest::addColumn<QString>("reason");
    QTest::newRow("provider-reason") << QStringLiteral("UWF is not registered");
    QTest::newRow("default-reason") << QString{};
  }
  void aCompletedUnavailableSnapshotStillPublishesADiagnostic() {
    QFETCH(QString, reason);
    StateSource source;
    source.next = [reason](std::stop_token) {
      app::ApplicationState state;
      state.snapshot.unavailableReason = reason.toStdString();
      return state;
    };
    presentation::MainWindowViewModel model(source, app::UwfCapability::Available);
    bool committed = false;
    connect(&model, &presentation::MainWindowViewModel::stateCommitted, &model, [&] {
      committed = true;
      QVERIFY(model.workspaceDiagnostic());
      QVERIFY(!model.workspaceDiagnostic()->isEmpty());
      if (!reason.isEmpty()) QCOMPARE(*model.workspaceDiagnostic(), reason);
    });
    model.refresh();
    QTRY_VERIFY(committed);
    QVERIFY(model.state().hasCommittedState());
    QVERIFY(!model.state().snapshot().uwfAvailable);
    QVERIFY(model.workspaceDiagnostic());
  }
  void exceptionalApplyBoundariesLogOnce_data() {
    QTest::addColumn<bool>("prepare");
    QTest::addColumn<int>("kind");
    QTest::addColumn<char>("level");
    QTest::newRow("prepare-standard") << true << 0 << 'E';
    QTest::newRow("worker-standard") << false << 0 << 'E';
    QTest::newRow("prepare-non-standard") << true << 1 << 'E';
    QTest::newRow("worker-non-standard") << false << 1 << 'E';
    QTest::newRow("prepare-policy-blocked") << true << 2 << 'W';
    QTest::newRow("worker-canceled") << false << 3 << 'I';
    QTest::newRow("worker-structured-diagnostic") << false << 4 << 'E';
  }
  void exceptionalApplyBoundariesLogOnce() {
    QFETCH(bool, prepare);
    QFETCH(int, kind);
    QFETCH(char, level);
    const std::string detail = "原始失败详情\n保留完整 UTF-8 第二行";
    const std::string response = "<SyncML>只应记录到日志的原始响应</SyncML>";
    const std::string extended = "COM extended error\n完整原始信息";
    std::exception_ptr failure;
    if (kind == 0)
      failure = std::make_exception_ptr(std::runtime_error(detail));
    else if (kind == 1)
      failure = std::make_exception_ptr(42);
    else if (kind == 2)
      failure = std::make_exception_ptr(
          app::OverlayPolicyError({.origin = app::OverlayPolicyFailureOrigin::Policy, .kind = app::OperationFailureKind::InvalidArgument, .message = detail}));
    else if (kind == 3)
      failure = std::make_exception_ptr(app::OperationError(std::make_error_code(std::errc::operation_canceled), detail));
    else
      failure =
          std::make_exception_ptr(app::OverlayPolicyError({.origin = app::OverlayPolicyFailureOrigin::Provider,
                                                           .kind = app::OperationFailureKind::AccessDenied,
                                                           .node = app::OverlayProperty::NextOverlayFlags,
                                                           .operation = "Replace",
                                                           .message = detail,
                                                           .diagnostics = "backend=CSP transport=0x80004005 provider=0x80070005\n" + response + "\n" + extended,
                                                           .effect = app::WriteEffect::Uncertain}));
    auto actions = std::make_shared<Actions>();
    actions->gate = std::make_shared<test::WorkerGate>();
    actions->gate->release();
    if (prepare)
      actions->prepareFailure = failure;
    else
      actions->workerFailure = failure;
    core::PendingChanges changes;
    changes.setFilterEnabled = true;
    presentation::ApplyPlanViewModel model(changes, {}, actions);
    QSignalSpy results(&model, &presentation::ApplyPlanViewModel::resultChanged);
    clearLogLines();
    model.apply();
    QTRY_COMPARE(results.count(), 1);
    QCOMPARE(logRecords(level).size(), 1);
    QCOMPARE(logRecords('E').size(), level == 'E' ? 1 : 0);
    const auto records = logRecords(level);
    const auto& record = records.front();
    QVERIFY(record.contains(prepare ? QStringLiteral("context=prepare") : QStringLiteral("context=worker")));
    QVERIFY(record.contains(kind == 1 ? QStringLiteral("error=non-standard-exception") : QString::fromStdString(detail)));
    if (level == 'W') {
      QVERIFY(record.contains(QStringLiteral("apply blocked")));
      QVERIFY(!record.contains(QStringLiteral("apply failed")));
    }
    if (kind == 4) {
      QVERIFY(record.contains(QStringLiteral("transport=0x80004005")));
      QVERIFY(record.contains(QStringLiteral("provider=0x80070005")));
      QVERIFY(record.contains(QString::fromStdString(response)));
      QVERIFY(record.contains(QString::fromStdString(extended)));
      QVERIFY(!model.resultText().contains(QString::fromStdString(response)));
      QVERIFY(!model.resultText().contains(QString::fromStdString(extended)));
    }
  }
  void returnedFailureDoesNotRepeatItsOwnersError() {
    auto actions = std::make_shared<ResultActions>();
    actions->result.steps.push_back({"Owned failure", app::ConfigurationStepOutcome::ExecutionFailed});
    actions->logFailure = true;
    core::PendingChanges changes;
    changes.setFilterEnabled = true;
    presentation::ApplyPlanViewModel model(changes, {}, actions);
    QSignalSpy results(&model, &presentation::ApplyPlanViewModel::resultChanged);
    clearLogLines();
    model.apply();
    QTRY_COMPARE(results.count(), 1);
    QVERIFY(model.resultFailed());
    QCOMPARE(logRecords('E').size(), 1);
    QVERIFY(logRecords('E').constFirst().contains(QStringLiteral("test-backend")));
  }
  void applyResultsDescribeTheOutcomeAndRestartAvailability_data() {
    QTest::addColumn<int>("failures");
    QTest::addColumn<bool>("written");
    QTest::addColumn<bool>("reconciliationRequired");
    QTest::addColumn<bool>("failed");
    QTest::addColumn<bool>("restart");
    QTest::addColumn<QString>("description");
    QTest::newRow("all-applied") << 0 << true << true << false << true
                                 << QStringLiteral("The configuration has been written to the system.\nTakes effect after restart");
    QTest::newRow("partially-applied") << 1 << true << true << true << true
                                       << QStringLiteral("Some changes may already have been written. Refresh the system state before retrying.");
    QTest::newRow("none-applied") << 1 << false << false << true << false << QStringLiteral("No configuration changes were written. You can retry.");
    QTest::newRow("write-uncertain") << 1 << false << true << true << false
                                     << QStringLiteral("Some changes may already have been written. Refresh the system state before retrying.");
    QTest::newRow("no-write-needed") << 0 << false << false << false << false << QStringLiteral("No configuration changes were needed.");
    QTest::newRow("no-write-needed-with-refresh") << 0 << false << true << false << false << QStringLiteral("No configuration changes were needed.");
  }
  void applyResultsDescribeTheOutcomeAndRestartAvailability() {
    QFETCH(int, failures);
    QFETCH(bool, written);
    QFETCH(bool, reconciliationRequired);
    QFETCH(bool, failed);
    QFETCH(bool, restart);
    QFETCH(QString, description);
    auto actions = std::make_shared<ResultActions>();
    if (written)
      actions->result.steps.push_back({"Filter", app::ConfigurationStepOutcome::Succeeded, app::WriteEffect::Confirmed,
                                       app::ConfigurationOperation{app::ConfigurationOperationKind::Filter, {}}});
    if (failures)
      actions->result.steps.push_back(
          {"Threshold failed", app::ConfigurationStepOutcome::Failed, !written && reconciliationRequired ? app::WriteEffect::Uncertain : app::WriteEffect::None,
           app::ConfigurationOperation{written ? app::ConfigurationOperationKind::WarningThreshold : app::ConfigurationOperationKind::Filter, {}}});
    if (!written && !failures)
      actions->result.steps.push_back({"Filter unchanged", app::ConfigurationStepOutcome::Succeeded, app::WriteEffect::None,
                                       app::ConfigurationOperation{app::ConfigurationOperationKind::Filter, {}}, reconciliationRequired});
    core::PendingChanges changes;
    changes.setFilterEnabled = true;
    if (written && failures) changes.setOverlay.warningThresholdMb = 1024;
    presentation::ApplyPlanViewModel model(changes, {}, actions);
    QSignalSpy results(&model, &presentation::ApplyPlanViewModel::resultChanged);
    model.apply();
    QTRY_COMPARE(results.count(), 1);
    QCOMPARE(model.resultFailed(), failed);
    QCOMPARE(model.canRestart(), restart);
    QCOMPARE(model.resultHeading(), failed ? QStringLiteral("Some changes could not be applied") : QStringLiteral("Configuration applied"));
    QCOMPARE(model.resultDescription(), description);
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
    source.snapshot = uwf::simulation::application::editableSnapshot();
    presentation::MainWindowViewModel owner(source, app::UwfCapability::Available);
    owner.initializeConfiguration(16384, QStringLiteral("C:"));
    owner.refresh();
    QTRY_VERIFY(!owner.refreshing());
    QVERIFY(owner.configurationWritesAllowed());
    QSignalSpy reconciled(&owner, &presentation::MainWindowViewModel::stateCommitted);
    QSignalSpy reconciliationFailed(&owner, &presentation::MainWindowViewModel::refreshFailed);
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
    if (outcome == 0 || outcome == 2) {
      // Reconciliation is queued; wait for its terminal signal before checking idle state.
      QTRY_COMPARE(reconciled.count() + reconciliationFailed.count(), 1);
      QCOMPARE(reconciliationFailed.count(), reconciliationReadFails ? 1 : 0);
      QCOMPARE(source.reads, 2);
    }
    QTRY_VERIFY(!owner.refreshing());
    QVERIFY(!owner.applyingConfiguration());
    QVERIFY(owner.exitAllowed());
    QCOMPARE(owner.powerActionsAllowed(), !reconciliationReadFails);
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
