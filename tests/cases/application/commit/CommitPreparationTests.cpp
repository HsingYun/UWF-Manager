// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QTest>
#include <array>
#include <functional>
#include <optional>

#include "application/operations/contracts/OperationError.h"
#include "application/paths/PathPolicy.h"
#include "domain/uwf/contracts/Types.h"
#include "support/app/CommitScenario.h"
namespace {
using namespace uwf;
class TextBoundarySource final : public app::CommitTraversalSource {
 public:
  mutable int inspections = 0;
  QList<app::CommitNode> children;
  std::function<app::CommitNode(const app::CommitNode&)> observation;
  std::optional<app::CommitNode> inspect(const app::CommitNode& node) const override {
    ++inspections;
    return observation ? observation(node) : node;
  }
  bool exists(const app::CommitNode&) const override { return true; }
  std::string driveLetterForPath(const QString&) const override { return "C:"; }
  app::FileCommitTarget resolveFileTarget(const QString& path) const override { return test::application::memoryFileTarget(path); }
  QString enumerateChildren(const app::CommitNode&, std::stop_token, const app::CommitNodeVisitor& discover) const override {
    for (const auto& child : children) discover(child);
    return {};
  }
};
class AliasTraversalSource final : public app::CommitTraversalSource {
 public:
  bool failBranch = true;
  mutable int rootEnumerations = 0;
  mutable int branchEnumerations = 0;
  QString guidPath(const QString& relative) const {
    return QString::fromStdString(compat::volumePath(test::application::fixtureVolumeId)) + relative.mid(1);
  }
  app::FileCommitTarget resolveFileTarget(const QString& path) const override {
    const auto address = app::paths::splitVolumeFilePath(path);
    if (!address) throw std::invalid_argument("Unexpected test path");
    return {compat::parseVolumeId(address->volume).value_or(test::application::fixtureVolumeId), address->relativePath};
  }
  std::optional<app::CommitNode> inspect(const app::CommitNode& selected) const override {
    return app::bindFileCommitTarget(*this, selected);
  }
  bool exists(const app::CommitNode&) const override { return true; }
  std::string driveLetterForPath(const QString&) const override { return "C:"; }
  QString enumerateChildren(const app::CommitNode& selected, std::stop_token, const app::CommitNodeVisitor& discover) const override {
    const auto& relative = selected.fileTarget->relativePath;
    if (relative == QStringLiteral("\\Tree")) {
      ++rootEnumerations;
      for (const auto& path : {QStringLiteral("C:\\Tree\\Branch"), guidPath(QStringLiteral("\\Tree\\Branch"))})
        discover({app::CommitTargetKind::Folder, path, {}, {}});
      for (const auto& path : {QStringLiteral("C:\\Tree\\one.txt"), guidPath(QStringLiteral("\\Tree\\one.txt"))})
        discover({app::CommitTargetKind::File, path, {}, {}});
    } else if (relative == QStringLiteral("\\Tree\\Branch")) {
      ++branchEnumerations;
      if (failBranch) return QStringLiteral("branch unavailable");
      for (const auto& path : {guidPath(QStringLiteral("\\Tree\\Branch\\two.txt")), QStringLiteral("C:\\Tree\\Branch\\two.txt")})
        discover({app::CommitTargetKind::File, path, {}, {}});
    }
    return {};
  }
};

void setContainerTailSpellings(test::commit::Tree& source, const bool rootTail, const bool branchTail) {
  const auto rootIdentity = source.root.identity();
  const auto branchIdentity = source.branch.identity();
  auto children = source.children.take(rootIdentity);
  const auto nested = source.children.take(branchIdentity);
  if (rootTail) source.root.path += u'\\';
  if (branchTail) source.branch.path += u'\\';
  for (auto& child : children)
    if (child.identity() == branchIdentity) child = source.branch;
  source.children.insert(source.root.identity(), children);
  source.children.insert(source.branch.identity(), nested);
}
class CommitPreparationTests final : public QObject {
  Q_OBJECT
 private slots:
  void explicitTraversalDiscoversContentAddedToAnEmptyScope_data() {
    QTest::addColumn<bool>("registry");
    QTest::newRow("directory") << false;
    QTest::newRow("registry-key") << true;
  }
  void explicitTraversalDiscoversContentAddedToAnEmptyScope() {
    QFETCH(bool, registry);
    test::commit::Tree source(registry);
    source.children.clear();
    app::CommitRequest request{app::CommitOperation::Persist, source.root};
    const auto empty = app::prepareCommit(request, source, {}, {}, {});
    QCOMPARE(empty.records.size(), 1);
    QVERIFY(empty.targets().isEmpty());
    source.children.insert(source.root.identity(), {source.leaf});
    const auto failedOnly = app::prepareCommit(request, source, empty, {}, {});
    QVERIFY(failedOnly.targets().isEmpty());
    QCOMPARE(source.reads.load(), 1);
    request.preparationIntent = app::CommitPreparationIntent::TraverseScope;
    const auto traversed = app::prepareCommit(request, source, empty, {}, {});
    QCOMPARE(source.reads.load(), 2);
    QCOMPARE(traversed.targets().size(), 1);
    QCOMPARE(traversed.progress().items, 1);
    const auto root = std::ranges::find_if(traversed.records, [&](const auto& row) { return row.node.identity() == source.root.identity(); });
    QVERIFY(root != traversed.records.end());
    QCOMPARE(root->attempts, 2);
  }
  void explicitTraversalReplacesTheOldActiveScopeAndKeepsObservedHistory() {
    test::commit::Tree source(false);
    source.failAfterChildren = true;
    app::CommitRequest request{app::CommitOperation::Persist, source.root};
    const auto previous = app::prepareCommit(request, source, {}, {}, {});
    QCOMPARE(previous.records.size(), 4);
    QCOMPARE(previous.progress().failed, 1);
    source.failAfterChildren = false;
    source.children.clear();
    source.children.insert(source.root.identity(), {source.leaf});
    request.preparationIntent = app::CommitPreparationIntent::TraverseScope;
    const auto current = app::prepareCommit(request, source, previous, {}, {});
    QCOMPARE(current.records.size(), 2);
    QCOMPARE(current.progress().failed, 0);
    QCOMPARE(current.targets().size(), 1);
    const auto root = std::ranges::find_if(current.records, [&](const auto& row) { return row.node.identity() == source.root.identity(); });
    QVERIFY(root != current.records.end());
    QCOMPARE(root->attempts, 2);
    QCOMPARE(root->errorHistory, QStringList{QStringLiteral("enumeration interrupted")});
    QVERIFY(std::ranges::none_of(current.records, [&](const auto& row) { return row.node.identity() == source.branch.identity(); }));
  }
  void explicitTraversalRefreshesTheSelectedRootsSkipObservation() {
    TextBoundarySource source;
    const app::CommitNode root{app::CommitTargetKind::Folder, QStringLiteral("C:\\Tree"), {}, {},
                               test::application::memoryFileTarget(QStringLiteral("C:\\Tree"))};
    source.observation = [](const auto& node) {
      auto skipped = node;
      skipped.skippedReason = QStringLiteral("temporarily skipped");
      return skipped;
    };
    const auto previous = app::prepareCommit({app::CommitOperation::Persist, root}, source, {}, {}, {});
    QVERIFY(previous.targets().isEmpty());
    QCOMPARE(previous.request.target.skippedReason, QStringLiteral("temporarily skipped"));
    source.observation = {};
    source.children = {{app::CommitTargetKind::File, root.path + QStringLiteral("\\one.txt"), {}, {}}};
    auto request = previous.request;
    request.preparationIntent = app::CommitPreparationIntent::TraverseScope;
    const auto current = app::prepareCommit(request, source, previous, {}, {});
    QCOMPARE(current.targets().size(), 1);
    QVERIFY(current.request.target.skippedReason.isEmpty());
    QCOMPARE(current.request.target.fileTarget, previous.request.target.fileTarget);
  }
  void explicitTraversalPreservesAliasDeduplicationAndPartialDeletionSafety() {
    AliasTraversalSource source;
    app::CommitRequest request{app::CommitOperation::DeleteAndPersist,
                               {app::CommitTargetKind::Folder, QStringLiteral("C:\\Tree"), {}, {}}};
    const auto previous = app::prepareCommit(request, source, {}, {}, {});
    request.preparationIntent = app::CommitPreparationIntent::TraverseScope;
    const auto current = app::prepareCommit(request, source, previous, {}, {});
    QCOMPARE(current.records.size(), 3);
    QCOMPARE(current.progress().failed, 1);
    QCOMPARE(source.rootEnumerations, 2);
    QCOMPARE(source.branchEnumerations, 2);
    QCOMPARE(current.targets().size(), 1);
    QCOMPARE(current.targets().constFirst().fileTarget->relativePath, QStringLiteral("\\Tree\\one.txt"));
    QCOMPARE(current.request.target.fileTarget, previous.request.target.fileTarget);
    const auto branch = std::ranges::find_if(current.records, [](const auto& row) {
      return row.node.fileTarget->relativePath == QStringLiteral("\\Tree\\Branch");
    });
    QVERIFY(branch != current.records.end());
    QCOMPARE(branch->attempts, 2);
    QCOMPARE(branch->errorHistory.size(), 2);
  }
  void cancellationOfANewTraversalCannotExecuteHistoricalTargets() {
    test::commit::Tree source(false);
    app::CommitRequest request{app::CommitOperation::Persist, source.root};
    const auto previous = app::prepareCommit(request, source, {}, {}, {});
    QCOMPARE(previous.targets().size(), 2);
    request.preparationIntent = app::CommitPreparationIntent::TraverseScope;
    std::stop_source cancel;
    const auto current = app::prepareCommit(request, source, previous, cancel.get_token(),
                                            [&](const auto& progress) { if (progress.items > 0) cancel.request_stop(); });
    QVERIFY(current.canceled);
    QVERIFY(current.targets().isEmpty());
  }
  void fileAliasesShareTraversalIdentityAndRetry() {
    AliasTraversalSource source;
    const app::CommitRequest request{app::CommitOperation::Persist, {app::CommitTargetKind::Folder, QStringLiteral("C:\\Tree"), {}, {}}};
    auto plan = app::prepareCommit(request, source, {}, {}, {});
    QCOMPARE(plan.records.size(), 3);
    QCOMPARE(plan.progress().failed, 1);
    QCOMPARE(plan.progress().items, 1);
    QCOMPARE(plan.targets().size(), 1);
    QCOMPARE(source.branchEnumerations, 1);
    source.failBranch = false;
    auto aliasRequest = request;
    aliasRequest.target.path = source.guidPath(QStringLiteral("\\Tree"));
    plan = app::prepareCommit(aliasRequest, source, plan, {}, {});
    QCOMPARE(plan.records.size(), 4);
    QCOMPARE(plan.progress().failed, 0);
    QCOMPARE(plan.progress().items, 2);
    QCOMPARE(plan.targets().size(), 2);
    QCOMPARE(source.branchEnumerations, 2);
    const auto branch = std::ranges::find_if(plan.records, [](const auto& row) {
      return row.node.fileTarget->relativePath == QStringLiteral("\\Tree\\Branch");
    });
    QVERIFY(branch != plan.records.end());
    QCOMPARE(branch->attempts, 2);
    QCOMPARE(branch->errorHistory, QStringList{QStringLiteral("branch unavailable")});
  }
  void fileAliasFailuresBlockTheSameDeletionScope() {
    AliasTraversalSource source;
    const auto plan = app::prepareCommit(
        {app::CommitOperation::DeleteAndPersist, {app::CommitTargetKind::Folder, QStringLiteral("C:\\Tree"), {}, {}}}, source, {}, {}, {});
    QCOMPARE(plan.targets().size(), 1);
    QCOMPARE(plan.targets().constFirst().fileTarget->relativePath, QStringLiteral("\\Tree\\one.txt"));
  }
  void unresolvedLegacyAliasesStillBlockTheSelectedDeletionRoot() {
    AliasTraversalSource source;
    const auto root = app::bindFileCommitTarget(
        source, {app::CommitTargetKind::Folder, source.guidPath(QStringLiteral("\\Tree")), {}, {}});
    app::CommitPlan plan;
    plan.request = {app::CommitOperation::DeleteAndPersist, root};
    plan.records = {{root, app::TraversalStatus::Succeeded, {}, {}, 1},
                    {{app::CommitTargetKind::File, QStringLiteral("C:\\Tree\\unread.txt"), {}, {}}, app::TraversalStatus::Failed,
                     QStringLiteral("identity unavailable"), {}, 1}};
    QVERIFY(plan.targets().isEmpty());
  }
  void existingBoundAliasesProduceOneExecutableTarget() {
    AliasTraversalSource source;
    const auto direct = app::bindFileCommitTarget(source, {app::CommitTargetKind::File, QStringLiteral("C:\\Tree\\one.txt"), {}, {}});
    auto alias = direct;
    alias.path = source.guidPath(QStringLiteral("\\Tree\\one.txt"));
    app::CommitPlan plan;
    plan.request = {app::CommitOperation::Persist, direct};
    plan.records = {{direct, app::TraversalStatus::Succeeded, {}, {}, 0}, {alias, app::TraversalStatus::Succeeded, {}, {}, 0}};
    QCOMPARE(plan.targets().size(), 1);
    const auto prepared = app::prepareCommit(plan.request, source, plan, {}, {});
    QCOMPARE(prepared.records.size(), 1);
    QCOMPARE(prepared.progress().items, 1);
    plan.records.back().status = app::TraversalStatus::Failed;
    QVERIFY(plan.targets().isEmpty());
    plan.records.back().status = app::TraversalStatus::Succeeded;
    plan.records.back().node.fileTarget->volumeId = test::application::otherFixtureVolumeId;
    QCOMPARE(plan.targets().size(), 2);
  }
  void anUnboundFailedAliasReusesAnExistingBoundRecord() {
    AliasTraversalSource source;
    const auto bound = app::bindFileCommitTarget(source, {app::CommitTargetKind::File, QStringLiteral("C:\\Tree\\one.txt"), {}, {}});
    auto alias = bound;
    alias.path = source.guidPath(QStringLiteral("\\Tree\\one.txt"));
    alias.fileTarget.reset();
    app::CommitPlan previous;
    previous.request = {app::CommitOperation::Persist, bound};
    previous.records = {{bound, app::TraversalStatus::Succeeded, {}, {}, 1},
                        {alias, app::TraversalStatus::Failed, QStringLiteral("old inspection failure"), {}, 1}};
    const auto prepared = app::prepareCommit(previous.request, source, previous, {}, {});
    QCOMPARE(prepared.records.size(), 1);
    QCOMPARE(prepared.progress().failed, 0);
    QCOMPARE(prepared.progress().items, 1);
    QCOMPARE(prepared.targets().size(), 1);
    QVERIFY(prepared.records.front().errorHistory.contains(QStringLiteral("old inspection failure")));
  }
  void bindingAcceptsEquivalentVolumeAndPathSpellings() {
    AliasTraversalSource source;
    app::CommitNode node{app::CommitTargetKind::File, QStringLiteral("C:\\Tree\\one.txt"), {}, {},
                         app::FileCommitTarget{"Volume{11111111-2222-3333-4444-555555555555}", QStringLiteral("\\TREE\\ONE.TXT")}};
    const auto bound = app::bindFileCommitTarget(source, node);
    QCOMPARE(bound.fileTarget->volumeId, std::string(test::application::fixtureVolumeId));
    QCOMPARE(node.fileTarget, bound.fileTarget);
    node.fileTarget->volumeId = test::application::otherFixtureVolumeId;
    QVERIFY_THROWS_EXCEPTION(app::OperationError, static_cast<void>(app::bindFileCommitTarget(source, node)));
  }
  void traversalPreservesTextBeforeEveryObservation_data() {
    QTest::addColumn<int>("field");
    QTest::addColumn<int>("boundary");
    QTest::addColumn<QString>("text");
    QTest::addColumn<bool>("valid");
    const std::array fields{"path", "value-name", "bound-relative-path"};
    const std::array boundaries{"selected-root", "observed-root", "selected-child", "observed-child"};
    struct Text {
      const char* name;
      QString value;
      bool valid;
    };
    const std::array texts{Text{"bmp", QStringLiteral("资料"), true},
                           Text{"supplementary", QString::fromUcs4(U"\U0001f642"), true},
                           Text{"high-surrogate", QString(QChar(char16_t{0xd800})), false},
                           Text{"low-surrogate", QString(QChar(char16_t{0xdc00})), false},
                           Text{"reversed-pair", QString(QChar(char16_t{0xdc00})) + QChar(char16_t{0xd800}), false},
                           Text{"broken-pair", QString(QChar(char16_t{0xd800})) + u'X', false},
                           Text{"embedded-null", QString(u'X') + QChar::Null + u'Y', false}};
    for (std::size_t field = 0; field < fields.size(); ++field)
      for (std::size_t boundary = 0; boundary < boundaries.size(); ++boundary)
        for (const auto& text : texts)
          QTest::newRow(qUtf8Printable(QStringLiteral("%1/%2/%3").arg(fields[field], boundaries[boundary], text.name)))
              << static_cast<int>(field) << static_cast<int>(boundary) << text.value << text.valid;
  }
  void traversalPreservesTextBeforeEveryObservation() {
    QFETCH(int, field);
    QFETCH(int, boundary);
    QFETCH(QString, text);
    QFETCH(bool, valid);
    TextBoundarySource source;
    app::CommitNode selected{field == 1 ? app::CommitTargetKind::RegistryValue : app::CommitTargetKind::File,
                             field == 1 ? QStringLiteral("HKEY_LOCAL_MACHINE\\Software\\Exact") : QStringLiteral("C:\\Exact\\Leaf"),
                             field == 1 ? QStringLiteral("Value") : QString{},
                             {}};
    const auto withText = [field, text](app::CommitNode node) {
      if (field == 0)
        node.path += text;
      else if (field == 1)
        node.valueName += text;
      else
        node.fileTarget = app::FileCommitTarget{test::application::fixtureVolumeId, QStringLiteral("\\Exact\\Leaf") + text};
      return node;
    };
    const bool child = boundary >= 2;
    const bool observed = boundary % 2 != 0;
    app::CommitNode root = selected;
    if (child) {
      root.kind = field == 1 ? app::CommitTargetKind::RegistryKey : app::CommitTargetKind::Folder;
      root.path = field == 1 ? QStringLiteral("HKEY_LOCAL_MACHINE\\Software\\Exact") : QStringLiteral("C:\\Exact");
      root.valueName.clear();
      source.children = {observed ? selected : withText(selected)};
    } else if (!observed) {
      root = withText(root);
    }
    if (observed) source.observation = [selected, withText](const auto& node) { return node.identity() == selected.identity() ? withText(node) : node; };
    const auto plan = app::prepareCommit({app::CommitOperation::DeleteAndPersist, root}, source, {}, {}, {});
    QCOMPARE(plan.progress().failed, valid ? 0 : 1);
    QCOMPARE(plan.targets().isEmpty(), !valid);
    // Reject invalid selections before source access and invalid observed names before execution or ancestor deletion.
    QCOMPARE(source.inspections, valid || observed ? (child ? 2 : 1) : (child ? 1 : 0));
    if (valid) {
      const auto found = std::ranges::find_if(plan.records, [&](const auto& record) { return record.node.identity() == withText(selected).identity(); });
      QVERIFY(found != plan.records.end());
      if (field == 2) QCOMPARE(found->node.fileTarget, withText(selected).fileTarget);
    }
  }
  void traversalContracts_data() {
    QTest::addColumn<bool>("registry");
    QTest::addColumn<bool>("deleting");
    QTest::addColumn<bool>("rootTail");
    QTest::addColumn<bool>("branchTail");
    for (const bool registry : {false, true})
      for (const bool deleting : {false, true})
        for (const bool rootTail : {false, true})
          for (const bool branchTail : {false, true}) {
            if (registry && (rootTail || branchTail)) continue;  // Registry spelling is its own exact-target contract.
            QTest::newRow(
                qUtf8Printable(QStringLiteral("registry-%1/delete-%2/root-tail-%3/branch-tail-%4").arg(registry).arg(deleting).arg(rootTail).arg(branchTail)))
                << registry << deleting << rootTail << branchTail;
          }
  }
  void traversalContracts() {
    QFETCH(bool, registry);
    QFETCH(bool, deleting);
    QFETCH(bool, rootTail);
    QFETCH(bool, branchTail);
    test::commit::Tree source(registry);
    setContainerTailSpellings(source, rootTail, branchTail);
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
    if (deleting && !registry) {
      const auto targets = plan.targets();
      const auto branch = std::ranges::find_if(targets, [&](const auto& node) { return node.identity() == source.branch.identity(); });
      const auto nested = std::ranges::find_if(targets, [](const auto& node) { return node.path.endsWith(QStringLiteral("\\two.txt")); });
      QVERIFY(nested != targets.end() && branch != targets.end());
      QVERIFY(nested < branch);
      QVERIFY(branch < targets.end() - 1);
    }
  }
  void skipsBlockAncestorDeletion_data() { traversalContracts_data(); }
  void incompleteEnumerationKeepsPublishedItems_data() { traversalContracts_data(); }
  void incompleteEnumerationKeepsPublishedItems() {
    QFETCH(bool, registry);
    QFETCH(bool, deleting);
    QFETCH(bool, rootTail);
    QFETCH(bool, branchTail);
    test::commit::Tree source(registry);
    setContainerTailSpellings(source, rootTail, branchTail);
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
    QFETCH(bool, rootTail);
    QFETCH(bool, branchTail);
    test::commit::Tree source(registry);
    setContainerTailSpellings(source, rootTail, branchTail);
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
    QFETCH(bool, rootTail);
    QFETCH(bool, branchTail);
    test::commit::Tree source(registry);
    setContainerTailSpellings(source, rootTail, branchTail);
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
  void distinctUnicodeBranchesKeepIndependentTraversalAndDeletionState_data() {
    QTest::addColumn<bool>("registry");
    QTest::addColumn<bool>("firstSkipped");
    for (const bool registry : {false, true})
      for (const bool firstSkipped : {false, true})
        QTest::newRow(qPrintable(QStringLiteral("%1/%2").arg(registry ? "registry" : "files").arg(firstSkipped ? "partial" : "complete")))
            << registry << firstSkipped;
  }
  void distinctUnicodeBranchesKeepIndependentTraversalAndDeletionState() {
    QFETCH(bool, registry);
    QFETCH(bool, firstSkipped);
    test::commit::Tree source(registry);
    auto micro = source.root;
    micro.path += QStringLiteral("\\") + QChar(0x00b5);
    if (firstSkipped) micro.skippedReason = QStringLiteral("branch is not traversable");
    auto mu = source.root;
    mu.path += QStringLiteral("\\") + QChar(0x03bc);
    source.children.clear();
    source.children.insert(source.root.identity(), {micro, mu});
    const auto plan = app::prepareCommit({app::CommitOperation::DeleteAndPersist, source.root}, source, {}, {}, {});
    QCOMPARE(plan.records.size(), 3);
    QCOMPARE(plan.progress().containers, firstSkipped ? 2 : 3);
    const auto targets = plan.targets();
    QCOMPARE(targets.size(), firstSkipped ? 1 : 3);
    QVERIFY(std::ranges::any_of(targets, [&](const auto& node) { return node.path == mu.path; }));
    QCOMPARE(std::ranges::any_of(targets, [&](const auto& node) { return node.path == source.root.path; }), !firstSkipped);
  }
};
}  // namespace
QTEST_APPLESS_MAIN(CommitPreparationTests)
#include "CommitPreparationTests.moc"
