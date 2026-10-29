// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QStringList>
#include <QTest>
#include <algorithm>
#include <memory>
#include <optional>
#include <stdexcept>
#include <thread>
#include <utility>

#include "app/power/PowerCoordinator.h"
#include "presentation/staging/FileStagingProgressViewModel.h"
#include "presentation/staging/FileStagingResultText.h"
#include "shared/Log.h"
#include "support/app/FileStagingFixture.h"
#include "support/app/RecordingWmiOperations.h"

namespace {
using namespace uwf;
using namespace uwf::app;
using namespace uwf::test::application;

class MemoryFiles final : public CommitTraversalSource {
 public:
  std::optional<CommitNode> inspect(const CommitNode& target) const override { return target; }
  bool exists(const CommitNode&) const override { return true; }
  std::string volumeForPath(const QString&) const override { return "C:"; }
  std::vector<std::string> mountedLetters(const std::string&) const override { return {"C:"}; }
  QString read(const CommitNode&, std::stop_token, const CommitNodeVisitor&) const override { throw std::logic_error("unexpected directory traversal"); }
};

class BatchFailureStore final : public FileStagingStore {
 public:
  QList<FileStagingEntry> entries{{FileStagingKind::File, QStringLiteral("C:\\Data\\批次失败.bin")}};
  std::exception_ptr failure;
  mutable int loads = 0;
  QList<FileStagingEntry> load() const override {
    ++loads;
    if (failure) std::rethrow_exception(failure);
    return entries;
  }
  void replace(const QList<FileStagingEntry>& replacement) override { entries = replacement; }
};

class CommitWorkerFailureFiles final : public CommitTraversalSource {
 public:
  mutable int inspections = 0;
  std::optional<CommitNode> inspect(const CommitNode& target) const override {
    if (++inspections == 2) throw 42;
    return target;
  }
  bool exists(const CommitNode&) const override { return true; }
  std::string volumeForPath(const QString&) const override { return "C:"; }
  std::vector<std::string> mountedLetters(const std::string&) const override { return {"C:"}; }
  QString read(const CommitNode&, std::stop_token, const CommitNodeVisitor&) const override { throw std::logic_error("unexpected directory traversal"); }
};

struct Scenario {
  std::shared_ptr<RecordingWmiState> state = std::make_shared<RecordingWmiState>();
  RecordingWmiOperations wmi{state};
  MemoryFileStagingStore store;
  std::shared_ptr<const CommitTraversalSource> files = std::make_shared<MemoryFiles>();
  const QString detail = QStringLiteral("Provider rejected the file.\n完整错误详情：拒绝访问，保留第二行。");

  explicit Scenario(const bool preparationFailure) {
    using Value = app::wmi::WmiValue;
    state->queryResults.push_back(
        {{{"__PATH", Value::fromString("filter")}, {"CurrentEnabled", Value::fromBool(true)}, {"NextEnabled", Value::fromBool(true)}}});
    state->queryResults.push_back({{{"__PATH", Value::fromString("volume")},
                                    {"CurrentSession", Value::fromBool(true)},
                                    {"DriveLetter", Value::fromString("C:")},
                                    {"VolumeName", Value::fromString("Volume{logging-fixture}")},
                                    {"BindByDriveLetter", Value::fromBool(true)},
                                    {"CommitPending", Value::fromBool(false)},
                                    {"Protected", Value::fromBool(true)}}});
    app::wmi::WmiMethodOutput exclusions;
    exclusions.arrays["ExcludedFiles"] = {};
    state->readResults.push_back(std::move(exclusions));
    const std::weak_ptr<RecordingWmiState> weak = state;
    state->invocationObserver = [weak, error = detail.toStdString()] {
      if (const auto locked = weak.lock()) locked->invocationFailure = std::make_exception_ptr(std::runtime_error(error));
    };
    for (int index = 0; index < 100; ++index) {
      const auto path = QStringLiteral("%1logging-failure-%2-资料.bin")
                            .arg(preparationFailure ? QStringLiteral("relative\\") : QStringLiteral("C:\\Data\\"))
                            .arg(index, 3, 10, QChar('0'));
      store.entries.append({FileStagingKind::File, path});
    }
  }
};

template <typename Predicate>
void awaitWorker(Predicate complete) {
  QElapsedTimer deadline;
  deadline.start();
  while (!complete()) {
    if (deadline.elapsed() > 5000) throw std::runtime_error("staging worker did not complete before the test deadline");
    QCoreApplication::processEvents(QEventLoop::AllEvents, 5);
    std::this_thread::yield();
  }
}

PowerStagingPreparation prepare(PowerActionDialogRequest& request) {
  if (!request.pollStagingPreparation) throw std::logic_error("new power action did not supply staging preparation");
  std::optional<PowerStagingPreparation> prepared;
  awaitWorker([&] {
    prepared = request.pollStagingPreparation();
    return prepared.has_value();
  });
  return std::move(*prepared);
}

std::vector<std::string> failureLogs() {
  auto lines = recentLogLines();
  std::erase_if(lines, [](const auto& line) { return line.find(" E ") == std::string::npos; });
  return lines;
}

class FileStagingFailureLoggingTests final : public QObject {
  Q_OBJECT
 private slots:
  void init() { clearLogLines(); }
  void routesRecordEveryFailureExactlyOnce_data() {
    QTest::addColumn<int>("route");
    QTest::addColumn<bool>("preparationFailure");
    const QStringList routes{QStringLiteral("command"), QStringLiteral("safe-shutdown"), QStringLiteral("safe-restart"), QStringLiteral("reused-command"),
                             QStringLiteral("service-task")};
    for (int route = 0; route < routes.size(); ++route)
      for (const bool preparation : {false, true}) {
        const auto name = routes[route] + (preparation ? QStringLiteral("-preparation") : QStringLiteral("-commit"));
        QTest::newRow(qUtf8Printable(name)) << route << preparation;
      }
  }
  void routesRecordEveryFailureExactlyOnce() {
    QFETCH(int, route);
    QFETCH(bool, preparationFailure);
    Scenario scenario(preparationFailure);
    int operations = 0;
    int completions = 0;
    int confirmations = 0;
    QString warning;
    std::optional<FileStagingCommitResult> observed;
    std::optional<FileStagingBatchResult> commandResult;
    FileStagingCoordinator coordinator([&](auto completion, auto progress) {
      ++operations;
      return std::make_unique<presentation::FileStagingProgressViewModel>(
          std::make_unique<FileStagingTask>(scenario.wmi, scenario.store, UwfCapability::Available, scenario.files), std::move(completion),
          std::move(progress));
    });
    PowerInteraction interaction;
    interaction.describeStagingResult = presentation::renderFileStagingResult;
    interaction.warning = [&](const QString&, const QString& text) { warning = text; };
    interaction.confirm = [&](PowerActionDialogRequest request) {
      ++confirmations;
      if (route == 3) {
        if (request.pollStagingPreparation || !request.completedStagingFailure || !request.completedStagingFailure->result)
          throw std::logic_error("power did not reuse the completed command batch");
        observed = request.completedStagingFailure->result;
        return PowerActionDialogOutcome::CanceledAfterStagingFailure;
      }
      auto prepared = prepare(request);
      if (const auto* failure = std::get_if<PowerStagingFailure>(&prepared)) {
        observed = failure->result;
      } else if (auto* commit = std::get_if<PowerStagingCommit>(&prepared)) {
        PowerStagingProgress status;
        awaitWorker([&] {
          status = commit->advanceStaging();
          return status.state != PowerStagingState::InProgress;
        });
        if (status.state != PowerStagingState::Failed) throw std::logic_error("expected a failed staging result");
        observed = status.failureResult;
      } else {
        throw std::logic_error("power skipped the failing staging list");
      }
      return PowerActionDialogOutcome::CanceledAfterStagingFailure;
    };
    PowerCoordinator power({scenario.wmi, scenario.store, UwfCapability::Available, coordinator, scenario.files}, std::move(interaction));
    if (route == 4) {
      // The preshutdown service drives this task directly without a presentation model.
      FileStagingTask task(scenario.wmi, scenario.store, UwfCapability::Available, scenario.files);
      awaitWorker([&] { return task.pollPreparation(); });
      while (!task.finished()) {
        static_cast<void>(task.advance());
        task.waitForWorker();
      }
      observed = task.result();
    } else if (route == 0 || route == 3) {
      coordinator.requestCommit([&](const FileStagingBatchResult& result) {
        ++completions;
        commandResult = result;
        if (route == 0) observed = result.result;
      });
      if (route == 3) power.safeRestart();
      QTRY_VERIFY_WITH_TIMEOUT(!coordinator.active(), 5000);
    } else if (route == 1) {
      power.safeShutdown();
    } else {
      power.safeRestart();
    }

    QVERIFY2(warning.isEmpty(), qUtf8Printable(warning));
    QVERIFY(observed.has_value());
    QCOMPARE(observed->failures.size(), 100);
    QCOMPARE(observed->committedFiles, std::size_t{0});
    QCOMPARE(scenario.store.loads, 1);
    QCOMPARE(operations, route == 0 || route == 3 ? 1 : 0);
    QCOMPARE(completions, route == 0 || route == 3 ? 1 : 0);
    QCOMPARE(confirmations, route == 0 || route == 4 ? 0 : 1);
    QCOMPARE(scenario.wmi.invocations.size(), preparationFailure ? std::size_t{0} : std::size_t{100});
    QVERIFY(std::ranges::all_of(scenario.wmi.invocations, [](const auto& method) { return method == QStringLiteral("CommitFile"); }));
    if (commandResult) {
      QCOMPARE(commandResult->command.outcome, ApplicationCommandOutcome::CompletedWithFailures);
      QCOMPARE(commandResult->command.failedFiles, std::size_t{100});
      QVERIFY(commandResult->result.has_value());
    }
    const auto errors = failureLogs();
    QCOMPARE(errors.size(), std::size_t{100});
    const auto kind = preparationFailure ? " kind=invalid-path" : " kind=provider-failure";
    for (const auto& failure : std::as_const(observed->failures)) {
      const auto target = "path=" + failure.path.toStdString() + " kind=";
      const auto matches = std::ranges::count_if(errors, [&](const auto& line) { return line.find(target) != std::string::npos; });
      QCOMPARE(matches, 1);
      const auto line = std::ranges::find_if(errors, [&](const auto& item) { return item.find(target) != std::string::npos; });
      QVERIFY(line != errors.end());
      QVERIFY(line->find(" E staging] file staging item failed:") != std::string::npos);
      QVERIFY(line->find(kind) != std::string::npos);
      if (!preparationFailure) QVERIFY(line->find(scenario.detail.toStdString()) != std::string::npos);
    }
    // Completion consumers and the bounded eight-item UI formatter cannot create new failure records.
    const auto copied = *observed;
    static_cast<void>(presentation::renderFileStagingResult(copied));
    QCOMPARE(failureLogs(), errors);
  }

  void infrastructureFailuresAreLoggedOnceAtTheirOwner_data() {
    QTest::addColumn<int>("route");
    QTest::addColumn<int>("source");
    QTest::addColumn<bool>("unknown");
    const QStringList routes{QStringLiteral("command"), QStringLiteral("safe-shutdown"), QStringLiteral("safe-restart"), QStringLiteral("reused-command"),
                             QStringLiteral("service-task")};
    const QStringList sources{
        QStringLiteral("store"),           QStringLiteral("filter"),          QStringLiteral("worker-accessor"),       QStringLiteral("commit-worker"),
        QStringLiteral("volume-snapshot"), QStringLiteral("exclusions-read"), QStringLiteral("commit-worker-accessor")};
    for (int route = 0; route < routes.size(); ++route)
      for (int source = 0; source < sources.size(); ++source)
        for (const bool unknown : {false, true}) {
          // Standard per-file inspection exceptions are deliberately item failures.
          if (source == 3 && !unknown) continue;
          const auto name = routes[route] + '-' + sources[source] + (unknown ? QStringLiteral("-unknown") : QStringLiteral("-multiline-utf8"));
          QTest::newRow(qUtf8Printable(name)) << route << source << unknown;
        }
  }
  void infrastructureFailuresAreLoggedOnceAtTheirOwner() {
    QFETCH(int, route);
    QFETCH(int, source);
    QFETCH(bool, unknown);
    const QString detail = QStringLiteral("批次读取/工作线程失败。\n完整原始 UTF-8 错误详情：拒绝访问。\n第三行必须保留。");
    const auto error = unknown ? std::make_exception_ptr(42) : std::make_exception_ptr(std::runtime_error(detail.toStdString()));
    BatchFailureStore store;
    RecordingWmiOperations wmi;
    if (source == 0) store.failure = error;
    if (source == 1) wmi.queryFailure = error;
    if (source == 2) wmi.workerAccessFailure = error;
    if (source == 4)
      wmi.queryObserver = [error, calls = 0]() mutable {
        if (++calls == 2) std::rethrow_exception(error);  // Filter succeeds; the volume snapshot fails.
      };
    if (source == 5) wmi.readFailure = error;
    if (source == 6)
      wmi.workerAccessObserver = [error, calls = 0]() mutable {
        if (++calls == 2) std::rethrow_exception(error);  // Preparation resolves first; commit resolves second.
      };
    Scenario seed(false);
    wmi.queryResults = seed.wmi.queryResults;
    wmi.readResults = seed.wmi.readResults;
    std::shared_ptr<const CommitTraversalSource> files =
        source == 3 ? std::shared_ptr<const CommitTraversalSource>(std::make_shared<CommitWorkerFailureFiles>()) : std::make_shared<MemoryFiles>();
    int completions = 0;
    int confirmations = 0;
    int operations = 0;
    std::optional<FileStagingBatchResult> observed;
    QString warning;
    FileStagingCoordinator coordinator([&](auto completion, auto progress) {
      ++operations;
      return std::make_unique<presentation::FileStagingProgressViewModel>(std::make_unique<FileStagingTask>(wmi, store, UwfCapability::Available, files),
                                                                          std::move(completion), std::move(progress));
    });
    PowerInteraction interaction;
    interaction.describeStagingResult = presentation::renderFileStagingResult;
    interaction.warning = [&](const QString&, const QString& text) { warning = text; };
    interaction.confirm = [&](PowerActionDialogRequest request) {
      ++confirmations;
      if (route == 3) {
        if (request.pollStagingPreparation || !request.completedStagingFailure || request.completedStagingFailure->result)
          throw std::logic_error("infrastructure failure was not reused intact");
        if (!unknown && !request.completedStagingFailure->details.contains(detail)) throw std::logic_error("reused failure lost its detail");
        return PowerActionDialogOutcome::CanceledAfterStagingFailure;
      }
      const auto prepared = prepare(request);
      QString failureDetail;
      if (const auto* failure = std::get_if<PowerStagingFailure>(&prepared)) {
        if (failure->result) throw std::logic_error("infrastructure failure invented target failures");
        failureDetail = failure->details;
      } else if (const auto* commit = std::get_if<PowerStagingCommit>(&prepared)) {
        PowerStagingProgress status;
        awaitWorker([&] {
          status = commit->advanceStaging();
          return status.state != PowerStagingState::InProgress;
        });
        if (status.state != PowerStagingState::Failed || status.failureResult) throw std::logic_error("worker failure produced a target result");
        failureDetail = status.failureDetails;
        const auto repeated = commit->advanceStaging();
        if (repeated.state != PowerStagingState::Failed || repeated.failureDetails != failureDetail)
          throw std::logic_error("repeated commit polling lost the batch failure");
      } else {
        throw std::logic_error("expected a batch infrastructure failure");
      }
      // Reusing the preparation result cannot retry a broken store or emit a new error.
      const auto repeated = request.pollStagingPreparation();
      if (!repeated || !std::holds_alternative<PowerStagingFailure>(*repeated) || std::get<PowerStagingFailure>(*repeated).details != failureDetail)
        throw std::logic_error("repeated preparation polling lost the batch failure");
      if (!unknown && !failureDetail.contains(detail)) throw std::logic_error("power failure lost its original detail");
      coordinator.requestCommit([&](const FileStagingBatchResult& result) {
        ++completions;
        observed = result;
      });
      return PowerActionDialogOutcome::CanceledAfterStagingFailure;
    };
    PowerCoordinator power({wmi, store, UwfCapability::Available, coordinator, files}, std::move(interaction));
    if (route == 4) {
      FileStagingTask task(wmi, store, UwfCapability::Available, files);
      const auto drive = [&] {
        awaitWorker([&] { return task.pollPreparation(); });
        while (!task.finished()) {
          static_cast<void>(task.advance());
          task.waitForWorker();
        }
      };
      int caught = 0;
      for (int poll = 0; poll < 3; ++poll) {
        try {
          drive();
        } catch (const std::exception& failure) {
          QVERIFY(!unknown);
          QCOMPARE(QString::fromUtf8(failure.what()), detail);
          ++caught;
        } catch (int marker) {
          QVERIFY(unknown);
          QCOMPARE(marker, 42);
          ++caught;
        }
      }
      QCOMPARE(caught, 3);
      QVERIFY(task.failed());
      QVERIFY(!task.finished());
      QCOMPARE(task.sourceEntries().has_value(), source != 0);
      QVERIFY_THROWS_EXCEPTION(std::logic_error, static_cast<void>(task.result()));
    } else if (route == 0 || route == 3) {
      coordinator.requestCommit([&](const FileStagingBatchResult& result) {
        ++completions;
        observed = result;
      });
      if (route == 3) power.safeRestart();
      QTRY_VERIFY_WITH_TIMEOUT(!coordinator.active(), 5000);
    } else if (route == 1) {
      power.safeShutdown();
    } else {
      power.safeRestart();
    }
    QVERIFY2(warning.isEmpty(), qUtf8Printable(warning));
    QCOMPARE(store.loads, 1);
    QCOMPARE(completions, route == 4 ? 0 : 1);
    QCOMPARE(operations, route == 0 || route == 3 ? 1 : 0);
    QCOMPARE(confirmations, route == 0 || route == 4 ? 0 : 1);
    QVERIFY(wmi.invocations.empty());
    if (observed) {
      QCOMPARE(observed->command.outcome, ApplicationCommandOutcome::Failed);
      QCOMPARE(observed->command.failedFiles, std::size_t{0});
      QVERIFY(!observed->result);
      QCOMPARE(observed->sourceEntries.has_value(), source != 0);
      if (!unknown) QVERIFY(observed->command.detail.contains(detail));
    }
    const auto errors = failureLogs();
    QCOMPARE(errors.size(), std::size_t{1});
    QVERIFY(errors.front().find("item failed:") == std::string::npos);
    QVERIFY(errors.front().find(unknown ? "error=non-standard-exception" : detail.toStdString()) != std::string::npos);
    if (observed) {
      const auto copied = *observed;
      QCOMPARE(copied.command.detail, observed->command.detail);
      QCOMPARE(failureLogs(), errors);
    }
    // A new request gets a fresh task and may succeed after the underlying issue is fixed.
    // One standard store failure per route covers the retry lifecycle; all other
    // rows above exercise their own failure, result reuse and cancellation.
    if (source != 0 || unknown) return;
    store.failure = {};
    store.entries.clear();
    wmi.queryFailure = {};
    wmi.workerAccessFailure = {};
    wmi.queryObserver = {};
    wmi.workerAccessObserver = {};
    wmi.readFailure = {};
    std::optional<FileStagingBatchResult> retried;
    coordinator.requestCommit([&](const FileStagingBatchResult& result) { retried = result; });
    QTRY_VERIFY_WITH_TIMEOUT(retried.has_value(), 5000);
    QCOMPARE(retried->command.outcome, ApplicationCommandOutcome::Succeeded);
    QCOMPARE(store.loads, 2);
    QCOMPARE(failureLogs(), errors);
  }

  void infrastructureFailuresCannotAuthorizePowerContinuation_data() {
    QTest::addColumn<bool>("shutdown");
    QTest::addColumn<int>("source");
    QTest::addColumn<bool>("unknown");
    for (const bool shutdown : {false, true})
      for (const int source : {0, 4, 5, 6})
        for (const bool unknown : {false, true}) {
          const auto name = QStringLiteral("%1-source-%2-%3").arg(shutdown ? QStringLiteral("shutdown") : QStringLiteral("restart")).arg(source).arg(unknown);
          QTest::newRow(qUtf8Printable(name)) << shutdown << source << unknown;
        }
  }
  void infrastructureFailuresCannotAuthorizePowerContinuation() {
    QFETCH(bool, shutdown);
    QFETCH(int, source);
    QFETCH(bool, unknown);
    BatchFailureStore store;
    const auto detail = QStringLiteral("基础设施失败\n不能视为逐目标失败授权");
    const auto failure = unknown ? std::make_exception_ptr(42) : std::make_exception_ptr(std::runtime_error(detail.toStdString()));
    if (source == 0) store.failure = failure;
    RecordingWmiOperations wmi;
    Scenario seed(false);
    wmi.queryResults = seed.wmi.queryResults;
    wmi.readResults = seed.wmi.readResults;
    if (source == 4)
      wmi.queryObserver = [failure, calls = 0]() mutable {
        if (++calls == 2) std::rethrow_exception(failure);
      };
    if (source == 5) wmi.readFailure = failure;
    if (source == 6)
      wmi.workerAccessObserver = [failure, calls = 0]() mutable {
        if (++calls == 2) std::rethrow_exception(failure);
      };
    FileStagingCoordinator coordinator([](auto, auto) -> std::unique_ptr<FileStagingCommandOperation> { throw std::logic_error("unexpected command"); });
    QString warning;
    PowerInteraction interaction;
    interaction.warning = [&](const QString&, const QString& text) { warning = text; };
    interaction.confirm = [](PowerActionDialogRequest request) {
      const auto prepared = prepare(request);
      if (const auto* commit = std::get_if<PowerStagingCommit>(&prepared)) {
        PowerStagingProgress status;
        awaitWorker([&] {
          status = commit->advanceStaging();
          return status.state != PowerStagingState::InProgress;
        });
        if (status.state != PowerStagingState::Failed || status.failureResult) throw std::logic_error("expected commit infrastructure failure");
      } else if (!std::holds_alternative<PowerStagingFailure>(prepared)) {
        throw std::logic_error("expected preparation infrastructure failure");
      }
      // Deliberately bypass the dialog model: a forged approval must still be
      // rejected by the coordinator. Widget tests cover the real dialog actions.
      return PowerActionDialogOutcome::ContinuedAfterStagingFailure;
    };
    PowerCoordinator power({wmi, store, UwfCapability::Available, coordinator, std::make_shared<MemoryFiles>()}, std::move(interaction));
    if (shutdown)
      power.safeShutdown();
    else
      power.safeRestart();
    QVERIFY(warning.contains(QStringLiteral("cannot approve a file staging failure without a recorded failed target")));
    QVERIFY(wmi.invocations.empty());
    QVERIFY(!coordinator.active());
    const auto errors = failureLogs();
    QCOMPARE(errors.size(), std::size_t{2});
    QCOMPARE(std::ranges::count_if(
                 errors, [&](const auto& line) { return line.find(unknown ? "error=non-standard-exception" : detail.toStdString()) != std::string::npos; }),
             1);
  }

  void cancellationBeforeCommitDoesNotInventFailures_data() {
    QTest::addColumn<bool>("shutdown");
    QTest::newRow("shutdown") << true;
    QTest::newRow("restart") << false;
  }
  void cancellationBeforeCommitDoesNotInventFailures() {
    QFETCH(bool, shutdown);
    Scenario scenario(false);
    bool prepared = false;
    QString warning;
    FileStagingCoordinator coordinator([](auto, auto) -> std::unique_ptr<FileStagingCommandOperation> {
      throw std::logic_error("canceling the power action must not start a command batch");
    });
    PowerInteraction interaction;
    interaction.warning = [&](const QString&, const QString& text) { warning = text; };
    interaction.confirm = [&](PowerActionDialogRequest request) {
      const auto staging = prepare(request);
      prepared = std::holds_alternative<PowerStagingCommit>(staging);
      return PowerActionDialogOutcome::Canceled;
    };
    PowerCoordinator power({scenario.wmi, scenario.store, UwfCapability::Available, coordinator, scenario.files}, std::move(interaction));
    if (shutdown)
      power.safeShutdown();
    else
      power.safeRestart();
    QVERIFY2(warning.isEmpty(), qUtf8Printable(warning));
    QVERIFY(prepared);
    QVERIFY(!coordinator.active());
    QVERIFY(scenario.wmi.invocations.empty());
    QVERIFY(failureLogs().empty());
  }
};
}  // namespace

QTEST_GUILESS_MAIN(FileStagingFailureLoggingTests)
#include "FileStagingFailureLoggingTests.moc"
