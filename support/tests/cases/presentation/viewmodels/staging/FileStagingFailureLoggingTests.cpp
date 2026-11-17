// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QStringList>
#include <QTest>
#include <algorithm>
#include <array>
#include <memory>
#include <optional>
#include <stdexcept>
#include <thread>
#include <utility>

#include "application/commit/contracts/CommitTraversalSource.h"
#include "application/operations/contracts/OperationError.h"
#include "application/power/PowerCoordinator.h"
#include "application/power/contracts/PowerInteraction.h"
#include "application/staging/execution/FileStagingTask.h"
#include "foundation/logging/Log.h"
#include "presentation/viewmodels/staging/FileStagingResultText.h"
#include "simulation/app/MemoryFileStagingStore.h"
#include "support/app/FileTargetFixture.h"
#include "support/app/UwfMaintenanceFixture.h"

enum class ExecutionRoute { SafeShutdown, SafeRestart, Service };
Q_DECLARE_METATYPE(ExecutionRoute)

namespace {
using namespace uwf;
using namespace uwf::app;
using namespace uwf::test::application;
enum class BatchFailureSource { Store, Filter, VolumeSnapshot, Exclusions, CommitWorker };

class MemoryFiles final : public CommitTraversalSource {
 public:
  std::optional<CommitNode> inspect(const CommitNode& target) const override { return target; }
  bool exists(const CommitNode&) const override { return true; }
  std::string driveLetterForPath(const QString&) const override { return "C:"; }
  FileCommitTarget resolveFileTarget(const QString& path) const override { return memoryFileTarget(path); }
  QString enumerateChildren(const CommitNode&, std::stop_token, const CommitNodeVisitor&) const override {
    throw std::logic_error("unexpected directory traversal");
  }
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
  std::string driveLetterForPath(const QString&) const override { return "C:"; }
  FileCommitTarget resolveFileTarget(const QString& path) const override { return memoryFileTarget(path); }
  QString enumerateChildren(const CommitNode&, std::stop_token, const CommitNodeVisitor&) const override {
    throw std::logic_error("unexpected directory traversal");
  }
};

struct Scenario {
  UwfMaintenanceFixture maintenance;
  uwf::simulation::application::MemoryFileStagingStore store;
  std::shared_ptr<const CommitTraversalSource> files = std::make_shared<MemoryFiles>();
  const QString detail = QStringLiteral("Provider rejected the file.\n完整错误详情：拒绝访问，保留第二行。");

  explicit Scenario(const bool preparationFailure) {
    maintenance.state->commitFailure = std::make_exception_ptr(std::runtime_error(detail.toStdString()));
    for (int index = 0; index < 100; ++index) {
      const auto path = QStringLiteral("%1logging-failure-%2-资料.bin")
                            .arg(preparationFailure ? QStringLiteral("relative\\") : QStringLiteral("C:\\Data\\"))
                            .arg(index, 3, 10, QChar('0'));
      store.entries.append({FileStagingKind::File, path});
      if (!preparationFailure) maintenance.stageFile(path);
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
    QTest::addColumn<ExecutionRoute>("route");
    QTest::addColumn<bool>("preparationFailure");
    const std::array routes{std::pair{"safe-shutdown", ExecutionRoute::SafeShutdown}, std::pair{"safe-restart", ExecutionRoute::SafeRestart},
                            std::pair{"service-task", ExecutionRoute::Service}};
    for (const auto& [routeName, route] : routes)
      for (const bool preparation : {false, true}) {
        const auto name = QString::fromLatin1(routeName) + (preparation ? QStringLiteral("-preparation") : QStringLiteral("-commit"));
        QTest::newRow(qUtf8Printable(name)) << route << preparation;
      }
  }
  void routesRecordEveryFailureExactlyOnce() {
    QFETCH(ExecutionRoute, route);
    QFETCH(bool, preparationFailure);
    Scenario scenario(preparationFailure);
    int confirmations = 0;
    QString warning;
    std::optional<FileStagingCommitResult> observed;
    PowerInteraction interaction;
    interaction.describeStagingResult = presentation::renderFileStagingResult;
    interaction.warning = [&](const QString&, const QString& text) { warning = text; };
    interaction.confirm = [&](PowerActionDialogRequest request) {
      ++confirmations;
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
    PowerCoordinator power({scenario.maintenance.repository(), scenario.store, UwfCapability::Available, scenario.files}, std::move(interaction));
    if (route == ExecutionRoute::Service) {
      // The preshutdown service drives this task directly without a presentation model.
      FileStagingTask task(scenario.maintenance.repository(), scenario.store.load(), UwfCapability::Available, scenario.files);
      awaitWorker([&] { return task.pollPreparation(); });
      while (!task.finished()) {
        static_cast<void>(task.advance());
        task.waitForWorker();
      }
      observed = task.result();
    } else if (route == ExecutionRoute::SafeShutdown) {
      power.safeShutdown();
    } else {
      power.safeRestart();
    }

    QVERIFY2(warning.isEmpty(), qUtf8Printable(warning));
    QVERIFY(observed.has_value());
    QCOMPARE(observed->failures.size(), 100);
    QCOMPARE(observed->committedFiles, std::size_t{0});
    QCOMPARE(scenario.store.loads, 1);
    QCOMPARE(confirmations, route == ExecutionRoute::Service ? 0 : 1);
    QCOMPARE(scenario.maintenance.state->commits.size(), preparationFailure ? std::size_t{0} : std::size_t{100});
    if (!preparationFailure)
      for (const auto& entry : std::as_const(scenario.store.entries))
        QCOMPARE(std::ranges::count(scenario.maintenance.state->commits, memoryFileTarget(entry.path).relativePath.toStdString()), std::ptrdiff_t{1});
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
    // Rendering a completed result must not log its failures again.
    const auto copied = *observed;
    static_cast<void>(presentation::renderFileStagingResult(copied));
    QCOMPARE(failureLogs(), errors);
  }

  void notFoundFailureDoesNotAuthorizeSafePower_data() {
    QTest::addColumn<int>("effect");
    for (const auto& [name, effect] :
         {std::pair{"none", WriteEffect::None}, std::pair{"confirmed", WriteEffect::Confirmed}, std::pair{"uncertain", WriteEffect::Uncertain}})
      QTest::newRow(name) << static_cast<int>(effect);
  }
  void notFoundFailureDoesNotAuthorizeSafePower() {
    QFETCH(int, effect);
    Scenario scenario(false);
    scenario.store.entries = {{FileStagingKind::File, QStringLiteral("C:\\Data\\pending.bin")}};
    scenario.maintenance.state->commitFailure = std::make_exception_ptr(
        OperationError(std::make_error_code(std::errc::no_such_file_or_directory), "Staging target was not found", static_cast<WriteEffect>(effect)));
    std::optional<FileStagingCommitResult> observed;
    QString warning;
    PowerInteraction interaction;
    interaction.describeStagingResult = presentation::renderFileStagingResult;
    interaction.warning = [&](const QString&, const QString& detail) { warning = detail; };
    interaction.confirm = [&](PowerActionDialogRequest request) {
      auto preparation = prepare(request);
      auto* commit = std::get_if<PowerStagingCommit>(&preparation);
      if (!commit) throw std::logic_error("expected the selected staging target");
      PowerStagingProgress status;
      awaitWorker([&] {
        status = commit->advanceStaging();
        return status.state != PowerStagingState::InProgress;
      });
      if (status.state != PowerStagingState::Failed) throw std::logic_error("NotFound failure was silently accepted");
      observed = status.failureResult;
      return PowerActionDialogOutcome::CanceledAfterStagingFailure;
    };
    PowerCoordinator power({scenario.maintenance.repository(), scenario.store, UwfCapability::Available, scenario.files}, std::move(interaction));
    power.safeRestart();
    QVERIFY2(warning.isEmpty(), qUtf8Printable(warning));
    QVERIFY(observed);
    QVERIFY(!observed->succeeded());
    QCOMPARE(observed->failures.size(), std::size_t{1});
    QCOMPARE(observed->skippedFiles, std::size_t{0});
    QCOMPARE(failureLogs().size(), std::size_t{1});
    QVERIFY(scenario.maintenance.backend->snapshot().powerRequests.empty());
  }

  void infrastructureFailuresAreLoggedOnceAtTheirOwner_data() {
    QTest::addColumn<ExecutionRoute>("route");
    QTest::addColumn<int>("source");
    QTest::addColumn<bool>("unknown");
    const std::array routes{std::pair{"safe-shutdown", ExecutionRoute::SafeShutdown}, std::pair{"safe-restart", ExecutionRoute::SafeRestart},
                            std::pair{"service-task", ExecutionRoute::Service}};
    const std::array sources{std::pair{"store", BatchFailureSource::Store}, std::pair{"filter", BatchFailureSource::Filter},
                             std::pair{"volume-snapshot", BatchFailureSource::VolumeSnapshot}, std::pair{"exclusions-read", BatchFailureSource::Exclusions},
                             std::pair{"commit-worker", BatchFailureSource::CommitWorker}};
    for (const auto& [routeName, route] : routes)
      for (const auto& [name, source] : sources)
        for (const bool unknown : {false, true}) {
          // The service host reads storage before constructing this task from an entries snapshot.
          if (route == ExecutionRoute::Service && source == BatchFailureSource::Store) continue;
          // Standard per-file inspection exceptions are deliberately item failures.
          if (source == BatchFailureSource::CommitWorker && !unknown) continue;
          const auto row =
              QString::fromLatin1(routeName) + '-' + QString::fromLatin1(name) + (unknown ? QStringLiteral("-unknown") : QStringLiteral("-multiline-utf8"));
          QTest::newRow(qUtf8Printable(row)) << route << static_cast<int>(source) << unknown;
        }
  }
  void infrastructureFailuresAreLoggedOnceAtTheirOwner() {
    QFETCH(ExecutionRoute, route);
    QFETCH(int, source);
    QFETCH(bool, unknown);
    const auto failureSource = static_cast<BatchFailureSource>(source);
    const QString detail = QStringLiteral("批次读取/工作线程失败。\n完整原始 UTF-8 错误详情：拒绝访问。\n第三行必须保留。");
    const auto error = unknown ? std::make_exception_ptr(42) : std::make_exception_ptr(std::runtime_error(detail.toStdString()));
    BatchFailureStore store;
    UwfMaintenanceFixture maintenance;
    if (failureSource == BatchFailureSource::Store) store.failure = error;
    if (failureSource == BatchFailureSource::Filter) maintenance.state->filterFailure = error;
    if (failureSource == BatchFailureSource::VolumeSnapshot) maintenance.state->volumeFailure = error;
    if (failureSource == BatchFailureSource::Exclusions) maintenance.state->exclusionFailure = error;
    std::shared_ptr<const CommitTraversalSource> files = failureSource == BatchFailureSource::CommitWorker
                                                             ? std::shared_ptr<const CommitTraversalSource>(std::make_shared<CommitWorkerFailureFiles>())
                                                             : std::make_shared<MemoryFiles>();
    int confirmations = 0;
    std::optional<PowerStagingFailure> observed;
    bool retrying = false;
    QString warning;
    PowerInteraction interaction;
    interaction.describeStagingResult = presentation::renderFileStagingResult;
    interaction.warning = [&](const QString&, const QString& text) { warning = text; };
    interaction.confirm = [&](PowerActionDialogRequest request) {
      ++confirmations;
      const auto prepared = prepare(request);
      if (retrying) {
        if (!std::holds_alternative<PowerStagingNotRequired>(prepared)) throw std::logic_error("empty retry did not finish preparation");
        return PowerActionDialogOutcome::Canceled;
      }
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
      observed = PowerStagingFailure{failureDetail};
      return PowerActionDialogOutcome::CanceledAfterStagingFailure;
    };
    PowerCoordinator power({maintenance.repository(), store, UwfCapability::Available, files}, std::move(interaction));
    if (route == ExecutionRoute::Service) {
      FileStagingTask task(maintenance.repository(), store.load(), UwfCapability::Available, files);
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
      QVERIFY_THROWS_EXCEPTION(std::logic_error, static_cast<void>(task.result()));
    } else if (route == ExecutionRoute::SafeShutdown) {
      power.safeShutdown();
    } else {
      power.safeRestart();
    }
    QVERIFY2(warning.isEmpty(), qUtf8Printable(warning));
    QCOMPARE(store.loads, 1);
    QCOMPARE(confirmations, route == ExecutionRoute::Service ? 0 : 1);
    QVERIFY(maintenance.state->commits.empty());
    if (observed) {
      QVERIFY(!observed->result);
      if (!unknown) QVERIFY(observed->details.contains(detail));
    }
    const auto errors = failureLogs();
    QCOMPARE(errors.size(), std::size_t{1});
    QVERIFY(errors.front().find("item failed:") == std::string::npos);
    QVERIFY(errors.front().find(unknown ? "error=non-standard-exception" : detail.toStdString()) != std::string::npos);
    if (observed) {
      const auto copied = *observed;
      QCOMPARE(copied.details, observed->details);
      QCOMPARE(failureLogs(), errors);
    }
    // After recovery, a new power request must reread storage and reacquire the released operation gate.
    if (failureSource != BatchFailureSource::Store || unknown) return;
    store.failure = {};
    store.entries.clear();
    maintenance.state->filterFailure = {};
    maintenance.state->volumeFailure = {};
    maintenance.state->exclusionFailure = {};
    retrying = true;
    power.safeRestart();
    QVERIFY(warning.isEmpty());
    QVERIFY(!power.active());
    QCOMPARE(store.loads, 2);
    QCOMPARE(failureLogs(), errors);
  }

  void infrastructureFailuresCannotAuthorizePowerContinuation_data() {
    QTest::addColumn<bool>("shutdown");
    QTest::addColumn<int>("source");
    QTest::addColumn<bool>("unknown");
    for (const bool shutdown : {false, true})
      for (const auto source :
           {BatchFailureSource::Store, BatchFailureSource::VolumeSnapshot, BatchFailureSource::Exclusions, BatchFailureSource::CommitWorker})
        for (const bool unknown : {false, true}) {
          if (source == BatchFailureSource::CommitWorker && !unknown) continue;
          const auto name = QStringLiteral("%1-source-%2-%3")
                                .arg(shutdown ? QStringLiteral("shutdown") : QStringLiteral("restart"))
                                .arg(static_cast<int>(source))
                                .arg(unknown);
          QTest::newRow(qUtf8Printable(name)) << shutdown << static_cast<int>(source) << unknown;
        }
  }
  void infrastructureFailuresCannotAuthorizePowerContinuation() {
    QFETCH(bool, shutdown);
    QFETCH(int, source);
    QFETCH(bool, unknown);
    const auto failureSource = static_cast<BatchFailureSource>(source);
    BatchFailureStore store;
    const auto detail = QStringLiteral("基础设施失败\n不能视为逐目标失败授权");
    const auto failure = unknown ? std::make_exception_ptr(42) : std::make_exception_ptr(std::runtime_error(detail.toStdString()));
    if (failureSource == BatchFailureSource::Store) store.failure = failure;
    UwfMaintenanceFixture maintenance;
    if (failureSource == BatchFailureSource::VolumeSnapshot) maintenance.state->volumeFailure = failure;
    if (failureSource == BatchFailureSource::Exclusions) maintenance.state->exclusionFailure = failure;
    const std::shared_ptr<const CommitTraversalSource> files = failureSource == BatchFailureSource::CommitWorker
                                                                   ? std::shared_ptr<const CommitTraversalSource>(std::make_shared<CommitWorkerFailureFiles>())
                                                                   : std::make_shared<MemoryFiles>();
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
      // Bypass the dialog to verify the coordinator rejects approval without a recorded target failure.
      return PowerActionDialogOutcome::ContinuedAfterStagingFailure;
    };
    PowerCoordinator power({maintenance.repository(), store, UwfCapability::Available, files}, std::move(interaction));
    if (shutdown)
      power.safeShutdown();
    else
      power.safeRestart();
    QVERIFY(warning.contains(QStringLiteral("cannot approve a file staging failure without a recorded failed target")));
    QVERIFY(maintenance.state->commits.empty());
    QVERIFY(!power.active());
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
    PowerInteraction interaction;
    interaction.warning = [&](const QString&, const QString& text) { warning = text; };
    interaction.confirm = [&](PowerActionDialogRequest request) {
      const auto staging = prepare(request);
      prepared = std::holds_alternative<PowerStagingCommit>(staging);
      return PowerActionDialogOutcome::Canceled;
    };
    PowerCoordinator power({scenario.maintenance.repository(), scenario.store, UwfCapability::Available, scenario.files}, std::move(interaction));
    if (shutdown)
      power.safeShutdown();
    else
      power.safeRestart();
    QVERIFY2(warning.isEmpty(), qUtf8Printable(warning));
    QVERIFY(prepared);
    QVERIFY(!power.active());
    QVERIFY(scenario.maintenance.state->commits.empty());
    QVERIFY(failureLogs().empty());
  }
};
}  // namespace

QTEST_GUILESS_MAIN(FileStagingFailureLoggingTests)
#include "FileStagingFailureLoggingTests.moc"
