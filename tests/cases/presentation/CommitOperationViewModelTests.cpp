// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QElapsedTimer>
#include <QSignalSpy>
#include <QTest>
#include <QThreadPool>

#include "presentation/commit/CommitOperationViewModel.h"
#include "support/app/CommitScenario.h"
namespace {
using namespace uwf;
using Model = presentation::CommitOperationViewModel;
class CommitOperationViewModelTests final : public QObject {
  Q_OBJECT
 private slots:
  void resultClassification_data() {
    using Cause = presentation::CommitCompletion::Cause;
    QTest::addColumn<presentation::CommitCounts>("counts");
    QTest::addColumn<presentation::CommitCompletion>("completion");
    QTest::addColumn<QString>("title");
    QTest::addColumn<presentation::CommitOutcome::Tone>("tone");
    using Tone = presentation::CommitOutcome::Tone;
    QTest::newRow("all-success") << presentation::CommitCounts{2, 0, 0} << presentation::CommitCompletion{} << QStringLiteral("All changes committed")
                                 << Tone::Success;
    QTest::newRow("all-failed") << presentation::CommitCounts{0, 0, 2} << presentation::CommitCompletion{} << QStringLiteral("No changes could be committed")
                                << Tone::Error;
    QTest::newRow("mixed") << presentation::CommitCounts{1, 0, 1} << presentation::CommitCompletion{} << QStringLiteral("Some changes committed")
                           << Tone::Warning;
    QTest::newRow("all-skipped") << presentation::CommitCounts{0, 2, 0} << presentation::CommitCompletion{} << QStringLiteral("No changes to commit")
                                 << Tone::Neutral;
    QTest::newRow("success-and-skip") << presentation::CommitCounts{1, 1, 0} << presentation::CommitCompletion{} << QStringLiteral("Commit complete")
                                      << Tone::Success;
    QTest::newRow("canceled") << presentation::CommitCounts{1, 0, 0} << presentation::CommitCompletion{Cause::Canceled, 1} << QStringLiteral("Commit canceled")
                              << Tone::Warning;
    QTest::newRow("interrupted") << presentation::CommitCounts{1, 0, 1} << presentation::CommitCompletion{Cause::Interrupted, 1}
                                 << QStringLiteral("Commit interrupted") << Tone::Warning;
    QTest::newRow("unread-branches") << presentation::CommitCounts{1, 0, 0} << presentation::CommitCompletion{Cause::Finished, 0, 1}
                                     << QStringLiteral("Some changes committed") << Tone::Warning;
  }
  void resultClassification() {
    QFETCH(presentation::CommitCounts, counts);
    QFETCH(presentation::CommitCompletion, completion);
    QFETCH(QString, title);
    QFETCH(presentation::CommitOutcome::Tone, tone);
    const auto outcome = presentation::describeCommitOutcome(counts, completion);
    QCOMPARE(outcome.title, title);
    QCOMPARE(outcome.tone, tone);
    QVERIFY(outcome.summary.contains(QString::number(counts.succeeded) + QStringLiteral(" succeeded")));
    QVERIFY(outcome.summary.contains(QString::number(counts.failed) + QStringLiteral(" failed")));
  }
  void notFoundResultsAreSkippedInsteadOfFailures() {
    auto backend = std::make_shared<test::commit::Backend>(false);
    app::CommitTargetResult missing;
    missing.hresult = static_cast<int32_t>(0x80041002u);
    backend->results = {missing, missing};
    Model model(backend->request(), backend);
    model.start();
    QTRY_COMPARE(model.phase(), Model::Phase::Ready);
    model.commit();
    QTRY_COMPARE(model.phase(), Model::Phase::Complete);
    QCOMPARE(model.text().title, QStringLiteral("No changes to commit"));
    QVERIFY(!model.text().error);
    QCOMPARE(presentation::countCommitResults(model.rows()).skipped, 2);
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
  void cancellationFinishesCurrentWriteAndSkipsRemainingTargets() {
    auto backend = std::make_shared<test::commit::Backend>(false);
    backend->blockWrite = true;
    Model model(backend->request(), backend);
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
    QCOMPARE(model.text().note, QStringLiteral("Changes already committed are retained."));
    model.retry();
    QCOMPARE(model.phase(), Model::Phase::Failed);
    QCOMPARE(backend->writes.load(), 1);
  }
};
}  // namespace
QTEST_GUILESS_MAIN(CommitOperationViewModelTests)
#include "CommitOperationViewModelTests.moc"
