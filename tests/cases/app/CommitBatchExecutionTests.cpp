// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QTest>

#include "app/commit/CommitBatchExecution.h"
namespace {
class CommitBatchExecutionTests final : public QObject {
  Q_OBJECT
 private slots:
  void cancellationLeavesRemainingTargetsUntouched() {
    QList<int> executed;
    uwf::app::CommitBatchRequest batch{3, uwf::app::CommitOperation::Persist, [](int index) { return QString::number(index); },
                                       [&](int index, const QString&) {
                                         executed.append(index);
                                         return uwf::app::CommitTargetResult{};
                                       }};
    const auto result = uwf::app::executeCommitBatch(batch, {[](int index) { return index < 1; }, {}, {}});
    QCOMPARE(executed, (QList<int>{0}));
    QCOMPARE(result.processed, 1);
    QCOMPARE(result.untouched, 2);
  }
  void failedTargetsDoNotPreventProcessingOtherTargets() {
    QList<bool> outcomes;
    uwf::app::CommitBatchRequest batch{3, uwf::app::CommitOperation::Persist, [](int index) { return QString::number(index); },
                                       [](int index, const QString&) {
                                         uwf::app::CommitTargetResult result;
                                         result.succeeded = index != 1;
                                         return result;
                                       }};
    uwf::app::CommitBatchProgress progress;
    progress.completed = [&](const auto& result, const QString&) { outcomes.append(result.succeeded); };
    const auto result = uwf::app::executeCommitBatch(batch, progress);
    QCOMPARE(outcomes, (QList<bool>{true, false, true}));
    QCOMPARE(result.processed, 3);
    QCOMPARE(result.untouched, 0);
  }
  void completionIsPublishedBeforeTheNextTargetStarts() {
    QStringList events;
    uwf::app::CommitBatchRequest batch{2, uwf::app::CommitOperation::Persist, [](int index) { return QString::number(index); },
                                       [&](int index, const QString&) {
                                         events.append(QStringLiteral("execute%1").arg(index));
                                         return uwf::app::CommitTargetResult{};
                                       }};
    uwf::app::CommitBatchProgress progress;
    progress.completed = [&](const auto&, const QString& display) { events.append(QStringLiteral("completed") + display); };
    (void)uwf::app::executeCommitBatch(batch, progress);
    QCOMPARE(events, (QStringList{QStringLiteral("execute0"), QStringLiteral("completed0"), QStringLiteral("execute1"), QStringLiteral("completed1")}));
  }
  void progressExecutionAndCompletionUseTheSameTargetDescription() {
    QString description = QStringLiteral("original target");
    QString shown;
    QString executed;
    QString completed;
    uwf::app::CommitBatchRequest batch{1, uwf::app::CommitOperation::Persist, [&](int) { return description; },
                                       [&](int, const QString& target) {
                                         executed = target;
                                         return uwf::app::CommitTargetResult{};
                                       }};
    uwf::app::CommitBatchProgress progress;
    progress.targetChanged = [&](int, const QString& target) {
      shown = target;
      description = QStringLiteral("changed during progress notification");
    };
    progress.completed = [&](const auto&, const QString& target) { completed = target; };
    const auto result = uwf::app::executeCommitBatch(batch, progress);
    QCOMPARE(result.processed, 1);
    QCOMPARE(shown, QStringLiteral("original target"));
    QCOMPARE(executed, shown);
    QCOMPARE(completed, shown);
  }
};
}  // namespace
QTEST_APPLESS_MAIN(CommitBatchExecutionTests)
#include "CommitBatchExecutionTests.moc"
