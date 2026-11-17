// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QTest>
#include <cstddef>
#include <optional>

#include "presentation/models/PageProxyModel.h"
#include "presentation/viewmodels/commit/CommitReportViewModel.h"
#include "support/app/CommitBatchFixture.h"

namespace {
using namespace uwf;

class CommitPresentationTests final : public QObject {
  Q_OBJECT
 private slots:
  void reportModelPreservesAllRowsAcrossPages();
  void commitBatchesKeepCallbacksLazyAndObserveTheOriginalTargets();
};

void CommitPresentationTests::reportModelPreservesAllRowsAcrossPages() {
  using namespace presentation;
  QList<CommitReportRow> rows;
  for (int index = 0; index < 205; ++index)
    rows.append({uwf::presentation::CommitRowStatus::Succeeded, QString::number(index), QStringLiteral("-"), QStringLiteral("-"), {}, {}});
  CommitReportViewModel model(rows, app::CommitOperation::DeleteAndPersist, {CommitCompletion::Cause::Canceled, 2});
  presentation::PageProxyModel pages;
  pages.setPageSize(200);
  pages.setSourceModel(&model);
  QCOMPARE(pages.pager().pageCount(205), 2);
  pages.goNext();
  QCOMPARE(pages.pager().pageStart(), 200);
  QVERIFY(model.showExistence());
  QCOMPARE(model.counts().succeeded, 205);
  QCOMPARE(model.counts().failed, 0);
  QVERIFY(model.summary().contains(QStringLiteral("205 succeeded")));
  QVERIFY(model.allRowsToText().contains(QStringLiteral("Succeeded\t204\tUnknown\tUnknown")));
  app::CommitTargetResult result;
  result.existedBefore = false;
  result.failureKind = app::OperationFailureKind::NotFound;
  const auto row = makeCommitReportRow(result, {app::CommitTargetKind::File, QStringLiteral("C:\\missing"), {}, {}});
  QCOMPARE(row.status, presentation::CommitRowStatus::Failed);
  QCOMPARE(row.existedBefore, std::optional<bool>(false));
}

void CommitPresentationTests::commitBatchesKeepCallbacksLazyAndObserveTheOriginalTargets() {
  QList<QString> targets{QStringLiteral("first"), QStringLiteral("second")};
  QStringList calls;
  auto display = [&](const QString& value) {
    calls.append(QStringLiteral("display:") + value);
    return value;
  };
  auto commit = [&](const QString& value) { calls.append(QStringLiteral("commit:") + value); };
  auto exists = nullptr;
  const auto request = test::makeCommitBatchRequest(app::CommitOperation::Persist, targets, display, commit, exists);
  QCOMPARE(request.total, 2);
  QVERIFY(calls.isEmpty());
  targets[1] = QStringLiteral("updated");
  const auto text = request.displayOf(1);
  const auto result = request.execute(1, text);
  QVERIFY(result.succeeded);
  QCOMPARE(calls, QStringList({QStringLiteral("display:updated"), QStringLiteral("commit:updated")}));
}

}  // namespace

QTEST_GUILESS_MAIN(CommitPresentationTests)
#include "CommitPresentationTests.moc"
