// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QElapsedTimer>
#include <QScopeGuard>
#include <QSignalSpy>
#include <QStringList>
#include <QTest>
#include <QThreadPool>
#include <algorithm>
#include <array>
#include <utility>

#include "presentation/models/ItemRoles.h"
#include "presentation/viewmodels/commit/CommitOperationViewModel.h"
#include "support/app/CommitScenario.h"

Q_DECLARE_METATYPE(uwf::app::CommitNode)

namespace {
using namespace uwf;
using Model = presentation::CommitOperationViewModel;
class BoundPreflightBackend final : public app::CommitBackend {
 public:
  int attempts = 0;
  app::CommitPlan prepare(const app::CommitRequest& request, app::CommitPlan previous, std::stop_token, const app::CommitPreparationProgress&) override {
    if (++attempts == 2) throw std::runtime_error("preflight unavailable after binding");
    if (attempts == 1) {
      previous.request = request;
      auto bound = request.target;
      bound.fileTarget = test::application::memoryFileTarget(bound.path);
      previous.records = {
          {bound, app::TraversalStatus::Failed, QStringLiteral("initial inspection unavailable"), {QStringLiteral("initial inspection unavailable")}, 1}};
    } else {
      previous.request = request;
      previous.request.target = previous.records.constFirst().node;
      previous.records.front().status = app::TraversalStatus::Succeeded;
      previous.records.front().error.clear();
    }
    return previous;
  }
  app::CommitTargetResult execute(const app::CommitNode&, app::CommitOperation) override { throw std::logic_error("Unexpected execution"); }
};
class CommitOperationViewModelTests final : public QObject {
  Q_OBJECT
 private slots:
  void traverseAgainRefreshesAnEmptyScope_data() {
    QTest::addColumn<bool>("registry");
    QTest::newRow("directory") << false;
    QTest::newRow("registry-key") << true;
  }
  void traverseAgainRefreshesAnEmptyScope() {
    QFETCH(bool, registry);
    auto backend = std::make_shared<test::commit::Backend>(registry);
    backend->tree->children.clear();
    Model model(backend->request(), backend);
    model.start();
    QTRY_COMPARE(model.phase(), Model::Phase::Empty);
    QCOMPARE(backend->tree->reads.load(), 1);
    QCOMPARE(model.total(), 0);
    QVERIFY(model.text().retryVisible);
    backend->tree->children.insert(backend->tree->root.identity(), {backend->tree->leaf});
    model.retry();
    QTRY_COMPARE(model.phase(), Model::Phase::Ready);
    QCOMPARE(backend->tree->reads.load(), 2);
    QCOMPARE(model.request().preparationIntent, app::CommitPreparationIntent::TraverseScope);
    QCOMPARE(model.total(), 1);
    QCOMPARE(model.plan().progress().items, 1);
    QCOMPARE(backend->writes.load(), 0);
  }
  void preflightFailureReusesAndPreservesTheBoundRoot() {
    auto backend = std::make_shared<BoundPreflightBackend>();
    Model model({app::CommitOperation::Persist, {app::CommitTargetKind::File, QStringLiteral("C:\\Data\\state.bin"), {}, {}}}, backend);
    model.start();
    QTRY_COMPARE(model.phase(), Model::Phase::Partial);
    QCOMPARE(model.plan().records.size(), 1);
    QVERIFY(model.plan().records.constFirst().node.fileTarget);
    model.retry();
    QTRY_COMPARE(model.phase(), Model::Phase::Failed);
    QCOMPARE(model.plan().records.size(), 1);
    QCOMPARE(model.plan().records.constFirst().attempts, 2);
    QCOMPARE(model.plan().records.constFirst().errorHistory.size(), 2);
    QVERIFY(model.plan().request.target.fileTarget);
    QCOMPARE(model.plan().request.target.fileTarget, model.plan().records.constFirst().node.fileTarget);
    model.retry();
    QTRY_COMPARE(model.phase(), Model::Phase::Ready);
    QCOMPARE(model.plan().records.size(), 1);
  }
  void reportNamesPreserveTypedTargets_data() {
    using Kind = app::CommitTargetKind;
    QTest::addColumn<app::CommitNode>("target");
    QTest::addColumn<QString>("expectedName");
    QTest::addColumn<bool>("deleting");
    struct TargetCase {
      const char* name;
      app::CommitNode target;
      QString label;
    };
    const QString key = QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Vendor");
    const std::array cases{
        TargetCase{"file", {Kind::File, QStringLiteral("C:\\Data\\Name.txt"), {}, {}}, QStringLiteral("Name.txt")},
        TargetCase{"file-forward-separators", {Kind::File, QStringLiteral("C:/Data/Name.txt"), {}, {}}, QStringLiteral("Name.txt")},
        TargetCase{"file-literal-space", {Kind::File, QStringLiteral("C:\\Data\\Name "), {}, {}}, QStringLiteral("Name ")},
        TargetCase{"folder-literal-dot", {Kind::Folder, QStringLiteral("C:\\Data\\Folder."), {}, {}}, QStringLiteral("Folder.")},
        TargetCase{"volume-root", {Kind::Folder, QStringLiteral("C:\\"), {}, {}}, QStringLiteral("C:\\")},
        TargetCase{"registry-key", {Kind::RegistryKey, key + QStringLiteral("\\Name"), {}, {}}, QStringLiteral("Name")},
        TargetCase{"registry-key-literal-slash", {Kind::RegistryKey, key + QStringLiteral("\\Slash/Name"), {}, {}}, QStringLiteral("Slash/Name")},
        TargetCase{
            "registry-key-literal-display-delimiter", {Kind::RegistryKey, key + QStringLiteral("\\Vendor : Name"), {}, {}}, QStringLiteral("Vendor : Name")},
        TargetCase{"registry-key-literal-space", {Kind::RegistryKey, key + QStringLiteral("\\Name "), {}, {}}, QStringLiteral("Name ")},
        TargetCase{"registry-value", {Kind::RegistryValue, key, QStringLiteral("Name"), {}}, QStringLiteral("Name")},
        TargetCase{"registry-value-literal-slash", {Kind::RegistryValue, key, QStringLiteral("Slash/Name"), {}}, QStringLiteral("Slash/Name")},
        TargetCase{"registry-value-literal-backslash", {Kind::RegistryValue, key, QStringLiteral("Slash\\Name"), {}}, QStringLiteral("Slash\\Name")},
        TargetCase{
            "registry-value-literal-display-delimiter", {Kind::RegistryValue, key, QStringLiteral("Vendor : Name"), {}}, QStringLiteral("Vendor : Name")},
        TargetCase{"registry-value-literal-space", {Kind::RegistryValue, key, QStringLiteral("Name "), {}}, QStringLiteral("Name ")},
        TargetCase{"registry-default-value", {Kind::RegistryValue, key, {}, {}}, QStringLiteral("(Default)")}};
    for (const auto& item : cases)
      for (const bool deleting : {false, true})
        QTest::newRow(
            qUtf8Printable(QStringLiteral("%1-%2").arg(QString::fromLatin1(item.name), deleting ? QStringLiteral("delete") : QStringLiteral("persist"))))
            << item.target << item.label << deleting;
  }
  void reportNamesPreserveTypedTargets() {
    QFETCH(app::CommitNode, target);
    QFETCH(QString, expectedName);
    QFETCH(bool, deleting);
    app::CommitTargetResult result;
    result.succeeded = true;
    result.existedBefore = true;
    result.existsAfter = false;
    const auto row = presentation::makeCommitReportRow(result, target);
    QCOMPARE(row.targetName, expectedName);
    QCOMPARE(row.path, target.display());
    const presentation::CommitReportViewModel report({row}, deleting ? app::CommitOperation::DeleteAndPersist : app::CommitOperation::Persist);
    QCOMPARE(report.columnCount(), deleting ? 6 : 4);
    const auto index = report.index(0, 1);
    QCOMPARE(index.data(Qt::DisplayRole).toString(), expectedName);
    QCOMPARE(index.data(presentation::FullTextRole).toString(), target.display());
    QCOMPARE(index.data(Qt::ToolTipRole).toString(), target.display());
    QVERIFY(report.details(0).startsWith(target.display()));
    QVERIFY(report.allRowsToText().contains(QChar('\t') + target.display() + QChar('\t')));
  }
  void completedRegistryReportsKeepSiblingValueNamesDistinct() {
    auto backend = std::make_shared<test::commit::Backend>(true);
    auto& tree = *backend->tree;
    const QStringList names{QStringLiteral("Name"), QStringLiteral("Slash/Name"), QStringLiteral("Slash\\Name"), QStringLiteral("Vendor : Name"), QString{}};
    QList<app::CommitNode> values;
    for (const auto& name : names) values.append({app::CommitTargetKind::RegistryValue, tree.root.path, name, {}});
    tree.children.clear();
    tree.children.insert(tree.root.identity(), values);
    Model model(backend->request(), backend);
    model.start();
    QTRY_COMPARE(model.phase(), Model::Phase::Ready);
    model.commit();
    QTRY_COMPARE(model.phase(), Model::Phase::Complete);
    QCOMPARE(model.rows().size(), names.size());
    const presentation::CommitReportViewModel report(model.rows(), app::CommitOperation::Persist);
    for (qsizetype row = 0; row < model.rows().size(); ++row) {
      const auto& target = model.rows()[row];
      const auto expectedTarget = std::ranges::find_if(std::as_const(values), [&](const auto& value) { return value.display() == target.path; });
      QVERIFY(expectedTarget != values.cend());
      const QString expected = expectedTarget->valueName.isEmpty() ? QStringLiteral("(Default)") : expectedTarget->valueName;
      QCOMPARE(report.index(static_cast<int>(row), 1).data().toString(), expected);
      QCOMPARE(report.index(static_cast<int>(row), 1).data(presentation::FullTextRole).toString(), expectedTarget->display());
    }
  }
  void resultClassification_data() {
    using Cause = presentation::CommitCompletion::Cause;
    QTest::addColumn<presentation::CommitCounts>("counts");
    QTest::addColumn<presentation::CommitCompletion>("completion");
    QTest::addColumn<QString>("title");
    QTest::addColumn<presentation::CommitOutcome::Tone>("tone");
    QTest::addColumn<QString>("description");
    QTest::addColumn<bool>("descriptionWarning");
    using Tone = presentation::CommitOutcome::Tone;
    const auto confirmed = QStringLiteral("The selected operations completed. UWF may skip excluded targets.");
    const auto unconfirmed = QStringLiteral("Some items were not completed or could not be verified. Select an item to view its full path and details.");
    QTest::newRow("all-success") << presentation::CommitCounts{2, 0} << presentation::CommitCompletion{} << QStringLiteral("All operations succeeded")
                                 << Tone::Success << confirmed << false;
    QTest::newRow("all-failed") << presentation::CommitCounts{0, 2} << presentation::CommitCompletion{} << QStringLiteral("No operations succeeded")
                                << Tone::Error << unconfirmed << false;
    QTest::newRow("mixed") << presentation::CommitCounts{1, 1} << presentation::CommitCompletion{} << QStringLiteral("Some operations succeeded")
                           << Tone::Warning << unconfirmed << false;
    QTest::newRow("empty") << presentation::CommitCounts{} << presentation::CommitCompletion{} << QStringLiteral("No targets to process") << Tone::Neutral
                           << QStringLiteral("No targets to process") << false;
    QTest::newRow("canceled") << presentation::CommitCounts{1, 0} << presentation::CommitCompletion{Cause::Canceled, 1} << QStringLiteral("Commit canceled")
                              << Tone::Warning << QStringLiteral("1 succeeded; 0 failed.\n1 items were not processed.") << true;
    QTest::newRow("canceled-with-no-remaining-targets") << presentation::CommitCounts{1, 0} << presentation::CommitCompletion{Cause::Canceled, 0}
                                                        << QStringLiteral("Commit canceled") << Tone::Warning << unconfirmed << false;
    QTest::newRow("interrupted") << presentation::CommitCounts{1, 1} << presentation::CommitCompletion{Cause::Interrupted, 1}
                                 << QStringLiteral("Commit interrupted") << Tone::Warning
                                 << QStringLiteral("1 succeeded; 1 failed.\n1 items were not processed.") << true;
    QTest::newRow("unread-branches") << presentation::CommitCounts{1, 0} << presentation::CommitCompletion{Cause::Finished, 0, 1}
                                     << QStringLiteral("Some operations succeeded") << Tone::Warning
                                     << QStringLiteral("1 succeeded; 0 failed. · 1 branches could not be read") << true;
    QTest::newRow("unread-only-with-no-confirmed-targets")
        << presentation::CommitCounts{} << presentation::CommitCompletion{Cause::Finished, 0, 1} << QStringLiteral("Traversal completed with unread content")
        << Tone::Warning << QStringLiteral("0 succeeded; 0 failed. · 1 branches could not be read") << true;
    QTest::newRow("unprocessed-without-unread-branches")
        << presentation::CommitCounts{1, 0} << presentation::CommitCompletion{Cause::Finished, 2} << QStringLiteral("Some operations succeeded")
        << Tone::Warning << QStringLiteral("1 succeeded; 0 failed.\n2 items were not processed.") << true;
  }
  void resultClassification() {
    QFETCH(presentation::CommitCounts, counts);
    QFETCH(presentation::CommitCompletion, completion);
    QFETCH(QString, title);
    QFETCH(presentation::CommitOutcome::Tone, tone);
    QFETCH(QString, description);
    QFETCH(bool, descriptionWarning);
    const auto outcome = presentation::describeCommitOutcome(counts, completion);
    QCOMPARE(outcome.title, title);
    QCOMPARE(outcome.tone, tone);
    QVERIFY(outcome.summary.startsWith(QStringLiteral("%1 succeeded; %2 failed.").arg(counts.succeeded).arg(counts.failed)));
    QList<presentation::CommitReportRow> rows(counts.succeeded + counts.failed);
    for (int index = 0; index < counts.succeeded; ++index) rows[index].status = presentation::CommitRowStatus::Succeeded;
    const presentation::CommitReportViewModel report(rows, app::CommitOperation::Persist, completion);
    QCOMPARE(report.outcome().title, title);
    QCOMPARE(report.description(), description);
    QCOMPARE(report.descriptionWarning(), descriptionWarning);
  }
  void failedCommandsRemainFailed_data() {
    QTest::addColumn<bool>("deleting");
    QTest::addColumn<app::WriteEffect>("effect");
    for (const bool deleting : {false, true})
      for (const auto effect : {app::WriteEffect::None, app::WriteEffect::Confirmed, app::WriteEffect::Uncertain})
        QTest::newRow(qUtf8Printable(QStringLiteral("delete%1-effect%2").arg(deleting).arg(static_cast<int>(effect)))) << deleting << effect;
  }
  void failedCommandsRemainFailed() {
    QFETCH(bool, deleting);
    QFETCH(app::WriteEffect, effect);
    auto backend = std::make_shared<test::commit::Backend>(false);
    app::CommitTargetResult missing;
    missing.failureKind = app::OperationFailureKind::NotFound;
    missing.failureEffect = effect;
    Model model(backend->request(deleting ? app::CommitOperation::DeleteAndPersist : app::CommitOperation::Persist), backend);
    model.start();
    QTRY_COMPARE(model.phase(), Model::Phase::Ready);
    const int targets = deleting ? 4 : 2;  // Deletion also includes the two directories.
    QCOMPARE(model.total(), targets);
    backend->results = QList<app::CommitTargetResult>(targets, missing);
    model.commit();
    QTRY_COMPARE(model.phase(), Model::Phase::Complete);
    QCOMPARE(model.text().title, QStringLiteral("No operations succeeded"));
    QVERIFY(model.text().error);
    QCOMPARE(presentation::countCommitResults(model.rows()).succeeded, 0);
    QCOMPARE(presentation::countCommitResults(model.rows()).failed, targets);
  }
  void operationContracts_data() {
    QTest::addColumn<bool>("registry");
    QTest::addColumn<bool>("deleting");
    for (const bool registry : {false, true})
      for (const bool deleting : {false, true}) QTest::newRow(qUtf8Printable(QStringLiteral("%1-%2").arg(registry).arg(deleting))) << registry << deleting;
  }
  void operationContracts() {
    QFETCH(bool, registry);
    QFETCH(bool, deleting);
    const auto backend = std::make_shared<test::commit::Backend>(registry);
    backend->tree->fail = true;
    Model model(backend->request(deleting ? app::CommitOperation::DeleteAndPersist : app::CommitOperation::Persist), backend);
    int heartbeats = 0;
    QTimer heartbeat;
    heartbeat.setInterval(10);
    connect(&heartbeat, &QTimer::timeout, this, [&] { ++heartbeats; });
    heartbeat.start();
    QElapsedTimer elapsed;
    elapsed.start();
    model.start();
    model.commit();
    QCOMPARE(model.phase(), Model::Phase::Preparing);
    QCOMPARE(backend->writes.load(), 0);
    QTRY_COMPARE_WITH_TIMEOUT(model.phase(), Model::Phase::Partial, 3000);
    QVERIFY(elapsed.elapsed() >= 1000);
    QVERIFY(heartbeats > 10);
    QVERIFY(backend->preparationThread != std::this_thread::get_id());
    model.commit();
    QCOMPARE(backend->writes.load(), 0);
    model.continuePartial();
    QCOMPARE(model.phase(), Model::Phase::Ready);
    QCOMPARE(backend->writes.load(), 0);
    model.commit();
    QTRY_COMPARE(model.phase(), Model::Phase::Complete);
    QCOMPARE(backend->writes.load(), 1);
    QCOMPARE(model.rows().size(), 1);
    QVERIFY(backend->executionThread != std::this_thread::get_id());
    QCOMPARE(model.plan().progress().failed, 1);
    QVERIFY(model.partialAccepted());
  }
  void retryRetainsSuccessfulRecordsAndErrorHistory() {
    auto backend = std::make_shared<test::commit::Backend>(false);
    backend->tree->fail = true;
    Model model(backend->request(), backend);
    model.start();
    QTRY_COMPARE(model.phase(), Model::Phase::Partial);
    backend->tree->fail = false;
    const auto previousReads = backend->tree->reads.load();
    model.retry();
    QTRY_COMPARE(model.phase(), Model::Phase::Ready);
    QCOMPARE(model.request().preparationIntent, app::CommitPreparationIntent::RetryFailed);
    QCOMPARE(backend->tree->reads.load() - previousReads, 1);
    QCOMPARE(model.plan().records.size(), 4);
    QCOMPARE(backend->writes.load(), 0);
    QString saved;
    std::thread::id writerThread;
    QSignalSpy exported(&model, &Model::exportFinished);
    model.exportRecords(QStringLiteral("records.txt"), [&](const QString&, const QString& text) {
      saved = text;
      writerThread = std::this_thread::get_id();
      return app::TextFileWriteResult{};
    });
    QTRY_COMPARE(exported.count(), 1);
    QVERIFY(saved.contains(QStringLiteral("access denied")));
    QVERIFY(saved.contains(QStringLiteral("Succeeded")));
    QVERIFY(writerThread != std::this_thread::get_id());
  }
  void cancelAndDestructionNeverWaitForPreparation() {
    auto backend = std::make_shared<test::commit::Backend>(false);
    backend->tree->block = true;
    backend->tree->ignoreStop = true;
    auto model = std::make_unique<Model>(backend->request(), backend);
    const auto releasePreparation = qScopeGuard([backend] { backend->tree->release = true; });
    model->start();
    QTRY_VERIFY(backend->tree->reads > 0);
    QElapsedTimer elapsed;
    elapsed.start();
    QVERIFY(model->cancel());
    model.reset();
    QVERIFY(elapsed.elapsed() < 100);
    backend->tree->release = true;
    QVERIFY(QThreadPool::globalInstance()->waitForDone(2000));
    QCOMPARE(backend->writes.load(), 0);
  }
  void cancellationFinishesCurrentWriteAndLeavesRemainingTargetsUnprocessed() {
    auto backend = std::make_shared<test::commit::Backend>(false);
    backend->blockWrite = true;
    Model model(backend->request(), backend);
    const auto releaseWrite = qScopeGuard([backend] { backend->releaseWrite = true; });
    model.start();
    QTRY_COMPARE(model.phase(), Model::Phase::Ready);
    model.commit();
    QTRY_COMPARE(backend->writes.load(), 1);
    QVERIFY(!model.cancel());
    QCOMPARE(model.phase(), Model::Phase::Canceling);
    backend->releaseWrite = true;
    QTRY_COMPARE(model.phase(), Model::Phase::Canceled);
    QCOMPARE(model.rows().size(), 1);
    QCOMPARE(model.untouched(), 1);
    QCOMPARE(backend->writes.load(), 1);
  }
  void preflightFailureKeepsARecordAndCanBeRetried() {
    auto backend = std::make_shared<test::commit::Backend>(false);
    backend->failPrepare = true;
    Model model(backend->request(), backend);
    model.start();
    QTRY_COMPARE(model.phase(), Model::Phase::Failed);
    QCOMPARE(model.plan().records.size(), 1);
    QVERIFY(!model.plan().records.front().errorHistory.isEmpty());
    model.retry();
    QTRY_COMPARE(model.phase(), Model::Phase::Failed);
    QCOMPARE(model.plan().records.front().attempts, 2);
    QCOMPARE(model.plan().records.front().errorHistory.size(), 2);
    backend->failPrepare = false;
    model.retry();
    QTRY_COMPARE(model.phase(), Model::Phase::Ready);
    QCOMPARE(model.plan().records.size(), 4);
    QCOMPARE(model.plan().records.front().attempts, 3);
    QCOMPARE(backend->writes.load(), 0);
  }
  void writeFailuresRemainReportedAndAreNeverReplayed() {
    auto backend = std::make_shared<test::commit::Backend>(false);
    backend->failWrites = true;
    Model model(backend->request(), backend);
    model.start();
    QTRY_COMPARE(model.phase(), Model::Phase::Ready);
    model.commit();
    QTRY_COMPARE(model.phase(), Model::Phase::Complete);
    QCOMPARE(backend->writes.load(), 2);
    QCOMPARE(model.rows().size(), 2);
    QVERIFY(model.text().error);
    QVERIFY(model.cancel());
    QCOMPARE(model.phase(), Model::Phase::Complete);
    model.retry();
    model.commit();
    QCOMPARE(backend->writes.load(), 2);
  }
  void unexpectedWriteFailureCannotRestartPreparation() {
    auto backend = std::make_shared<test::commit::Backend>(false);
    backend->throwWrite = true;
    Model model(backend->request(), backend);
    model.start();
    QTRY_COMPARE(model.phase(), Model::Phase::Ready);
    model.commit();
    QTRY_COMPARE(model.phase(), Model::Phase::Failed);
    QCOMPARE(backend->writes.load(), 1);
    QCOMPARE(model.processed(), 1);
    QCOMPARE(model.untouched(), 1);
    QCOMPARE(model.rows().size(), 1);
    QVERIFY(model.text().showReport);
    QVERIFY(!model.text().retryVisible);
    QCOMPARE(model.text().title, QStringLiteral("Commit interrupted"));
    QCOMPARE(model.text().note, QStringLiteral("Completed operations cannot be undone."));
    model.retry();
    QCOMPARE(model.phase(), Model::Phase::Failed);
    QCOMPARE(backend->writes.load(), 1);
  }
};
}  // namespace
QTEST_GUILESS_MAIN(CommitOperationViewModelTests)
#include "CommitOperationViewModelTests.moc"
#include "support/app/FileTargetFixture.h"
