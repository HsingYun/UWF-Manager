// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QTest>

#include "support/app/CommitScenario.h"
namespace {
using namespace uwf;
class CommitPreparationTests final : public QObject {
  Q_OBJECT
 private slots:
  void traversalContracts_data() {
    QTest::addColumn<bool>("registry");
    QTest::addColumn<bool>("deleting");
    for (const bool registry : {false, true})
      for (const bool deleting : {false, true}) QTest::newRow(qUtf8Printable(QStringLiteral("%1-%2").arg(registry).arg(deleting))) << registry << deleting;
  }
  void traversalContracts() {
    QFETCH(bool, registry);
    QFETCH(bool, deleting);
    test::commit::Tree source(registry);
    const app::CommitRequest request{deleting ? app::CommitOperation::DeleteAndPersist : app::CommitOperation::Persist, source.root};
    source.fail = true;
    auto plan = app::prepareCommit(request, source, {}, {}, {});
    QCOMPARE(plan.records.size(), 3);
    QCOMPARE(plan.progress().failed, 1);
    QCOMPARE(plan.progress().items, 1);
    QCOMPARE(plan.targets().size(), 1);
    QCOMPARE(plan.targets().constFirst().display(), source.leaf.display());
    QVERIFY(std::ranges::any_of(plan.records, [](const auto& row) { return row.status == app::TraversalStatus::Failed && !row.errorHistory.isEmpty(); }));
    source.fail = false;
    const auto before = source.reads.load();
    plan = app::prepareCommit(request, source, plan, {}, {});
    QCOMPARE(source.reads.load() - before, 1);
    QCOMPARE(plan.records.size(), 4);
    QCOMPARE(plan.progress().failed, 0);
    QCOMPARE(plan.progress().items, 2);
    QCOMPARE(plan.progress().containers, 2);
    QCOMPARE(plan.targets().size(), deleting ? registry ? 2 : 4 : 2);
    const auto retried = std::ranges::find_if(plan.records, [&](const auto& row) { return row.node.identity() == source.branch.identity(); });
    QCOMPARE(retried->attempts, 2);
    QCOMPARE(retried->errorHistory.size(), 1);
    if (deleting) QCOMPARE(plan.targets().constLast().identity(), source.root.identity());
  }
  void skipsBlockAncestorDeletion_data() { traversalContracts_data(); }
  void incompleteEnumerationKeepsPublishedItems_data() { traversalContracts_data(); }
  void incompleteEnumerationKeepsPublishedItems() {
    QFETCH(bool, registry);
    QFETCH(bool, deleting);
    test::commit::Tree source(registry);
    source.failAfterChildren = true;
    app::CommitTraversalProgress progress;
    bool publishedBeforeEnumerationReturned = false;
    source.afterChild = [&] { publishedBeforeEnumerationReturned |= progress.items > 0; };
    const app::CommitRequest request{deleting ? app::CommitOperation::DeleteAndPersist : app::CommitOperation::Persist, source.root};
    auto plan = app::prepareCommit(request, source, {}, {}, [&](const auto& update) { progress = update; });
    QVERIFY(publishedBeforeEnumerationReturned);
    QCOMPARE(plan.progress().items, 2);
    QCOMPARE(plan.progress().failed, 1);
    QCOMPARE(plan.targets().size(), deleting && !registry ? 3 : 2);
    QVERIFY(std::ranges::none_of(plan.targets(), [&](const auto& node) { return node.identity() == source.root.identity(); }));
    source.failAfterChildren = false;
    const auto reads = source.reads.load();
    plan = app::prepareCommit(request, source, std::move(plan), {}, {});
    QCOMPARE(source.reads.load() - reads, 1);
    QCOMPARE(plan.records.size(), 4);
    QCOMPARE(plan.progress().failed, 0);
    QCOMPARE(plan.progress().items, 2);
  }
  void skipsBlockAncestorDeletion() {
    QFETCH(bool, registry);
    QFETCH(bool, deleting);
    test::commit::Tree source(registry);
    source.children[source.root.identity()][0].skippedReason = QStringLiteral("reparse point");
    const auto plan = app::prepareCommit({deleting ? app::CommitOperation::DeleteAndPersist : app::CommitOperation::Persist, source.root}, source, {}, {}, {});
    QCOMPARE(plan.records.size(), 3);
    QCOMPARE(plan.targets().size(), 1);
    QCOMPARE(plan.targets().constFirst().display(), source.leaf.display());
  }
  void cancellationNeverReturnsAnExecutablePartialPlan() {
    test::commit::Tree source(false);
    std::stop_source cancel;
    const auto plan =
        app::prepareCommit({app::CommitOperation::Persist, source.root}, source, {}, cancel.get_token(), [&](const auto&) { cancel.request_stop(); });
    QVERIFY(plan.canceled);
    QVERIFY(plan.targets().isEmpty());
    QCOMPARE(plan.records.size(), 1);
  }
  void emptyContainerContracts_data() { traversalContracts_data(); }
  void emptyContainerContracts() {
    QFETCH(bool, registry);
    QFETCH(bool, deleting);
    test::commit::Tree source(registry);
    source.children.clear();
    const auto plan = app::prepareCommit({deleting ? app::CommitOperation::DeleteAndPersist : app::CommitOperation::Persist, source.root}, source, {}, {}, {});
    QCOMPARE(plan.progress().items, 0);
    QCOMPARE(plan.progress().containers, 1);
    QCOMPARE(plan.targets().size(), deleting ? 1 : 0);
  }
  void rootClassificationSurvivesAFailedPreparationRetry() {
    test::commit::Tree source(false);
    auto initial = source.root;
    initial.kind = app::CommitTargetKind::File;
    const app::CommitRequest request{app::CommitOperation::Persist, initial};
    app::CommitPlan previous;
    previous.request = request;
    previous.records.append({initial, app::TraversalStatus::Failed, QStringLiteral("unavailable"), {}, 1});
    const auto plan = app::prepareCommit(request, source, previous, {}, {});
    QCOMPARE(plan.request.target.kind, app::CommitTargetKind::Folder);
    QCOMPARE(plan.records.size(), 4);
    QCOMPARE(plan.records.front().attempts, 2);
  }
  void defaultValuesNeverBecomePartialKeyDeletionRequests() {
    test::commit::Tree source(true);
    source.fail = true;
    auto value = source.leaf;
    value.valueName.clear();
    source.children[source.root.identity()].append(value);
    const auto plan = app::prepareCommit({app::CommitOperation::DeleteAndPersist, source.root}, source, {}, {}, {});
    QCOMPARE(plan.targets().size(), 1);
    QCOMPARE(plan.targets().constFirst().valueName, source.leaf.valueName);
  }
  void registryControlCharactersDoNotMergeDistinctTargets() {
    test::commit::Tree source(true);
    auto first = source.leaf;
    first.valueName = QStringLiteral("Part\nValue");
    auto second = source.leaf;
    second.path += QStringLiteral("\nPart");
    second.valueName = QStringLiteral("Value");
    source.children.clear();
    source.children.insert(source.root.identity(), {first, second});
    const auto plan = app::prepareCommit({app::CommitOperation::Persist, source.root}, source, {}, {}, {});
    QCOMPARE(plan.targets().size(), 2);
  }
};
}  // namespace
QTEST_APPLESS_MAIN(CommitPreparationTests)
#include "CommitPreparationTests.moc"
