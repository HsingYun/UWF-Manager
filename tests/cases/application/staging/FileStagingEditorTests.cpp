// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QPointer>
#include <QSignalSpy>
#include <QTest>
#include <array>
#include <exception>
#include <functional>
#include <memory>
#include <utility>

#include "application/configuration/contracts/ConfigurationPlan.h"
#include "application/paths/PathPolicy.h"
#include "application/staging/editing/FileStagingConflictPolicy.h"
#include "application/staging/editing/FileStagingDraft.h"
#include "application/staging/editing/FileStagingEditor.h"
#include "application/staging/editing/FileStagingEntries.h"
#include "application/staging/persistence/FileStagingRegistry.h"
#include "domain/uwf/contracts/Types.h"
#include "support/app/FileStagingFixture.h"
#include "support/app/FileTargetFixture.h"

namespace {
using namespace uwf;
using app::FileStagingEntry;
using app::FileStagingKind;

std::exception_ptr prepare(app::FileStagingPreparation& preparation) {
  try {
    if (preparation.prepare) preparation.prepare({});
  } catch (...) {
    return std::current_exception();
  }
  return {};
}

void complete(app::FileStagingPreparation preparation) {
  const auto failure = prepare(preparation);
  if (preparation.complete) preparation.complete(failure);
}

class SelectionSource final : public app::CommitTraversalSource {
 public:
  mutable int inspections = 0;
  mutable int bindings = 0, driveReads = 0;
  std::function<void()> beforeInspection;
  std::function<void()> beforeResolution;
  std::function<app::FileCommitTarget(const QString&)> targetResolver;
  std::function<std::string(const QString&)> driveResolver;
  std::string driveLetterForPath(const QString& path) const override {
    ++driveReads;
    if (beforeResolution) beforeResolution();
    if (driveResolver) return driveResolver(path);
    return path.left(2).toUpper().toStdString();
  }
  std::optional<app::CommitNode> inspect(const app::CommitNode& target) const override {
    ++inspections;
    if (beforeInspection) beforeInspection();
    if (target.path.endsWith(QLatin1String("missing"))) return {};
    auto result = target;
    result.kind =
        target.path.endsWith(QLatin1String("folder")) || target.path == QLatin1String("C:\\") ? app::CommitTargetKind::Folder : app::CommitTargetKind::File;
    return result;
  }
  bool exists(const app::CommitNode&) const override { throw std::logic_error("Editing must not request commit observations"); }
  app::FileCommitTarget resolveFileTarget(const QString& path) const override {
    ++bindings;
    if (targetResolver) return targetResolver(path);
    return test::application::memoryFileTarget(
        path, path.startsWith(u"D:", Qt::CaseInsensitive) ? test::application::otherFixtureVolumeId : test::application::fixtureVolumeId);
  }
  QString enumerateChildren(const app::CommitNode&, std::stop_token, const app::CommitNodeVisitor&) const override {
    throw std::logic_error("Selecting a directory must not enumerate its contents");
  }
};

class ReentrantStore final : public app::FileStagingStore {
 public:
  QList<FileStagingEntry> entries;
  mutable std::function<void()> beforeRead;
  bool failedRead = false;
  int writes = 0;
  QList<FileStagingEntry> load() const override {
    const auto observed = entries;
    if (const auto callback = std::exchange(beforeRead, {})) {
      callback();
      if (failedRead) throw std::runtime_error("obsolete read failed");
    }
    return observed;
  }
  void replace(const QList<FileStagingEntry>& value) override {
    entries = value;
    ++writes;
  }
};

struct BatchCase {
  const char* name;
  QList<FileStagingEntry> selection;
  QList<FileStagingEntry> expected;
  int rejected;
};
const auto& batches() {
  static const FileStagingEntry file{FileStagingKind::File, QStringLiteral("C:\\one.txt")};
  static const FileStagingEntry folder{FileStagingKind::Directory, QStringLiteral("C:\\folder")};
  static const FileStagingEntry child{FileStagingKind::File, QStringLiteral("C:\\folder\\child.txt")};
  static const FileStagingEntry supplementary{FileStagingKind::File, QStringLiteral("C:\\\U00010400.txt")};
  static const std::array cases{
      BatchCase{"empty", {}, {}, 0},
      BatchCase{"mixed", {file, folder}, {folder, file}, 0},
      BatchCase{"repeated-identities", {file, folder, file, folder}, {folder, file}, 0},
      BatchCase{"directory-before-child", {folder, child, file}, {folder, file}, 0},
      BatchCase{"child-before-directory", {child, file, folder}, {folder, file}, 0},
      BatchCase{"invalid-path-and-valid", {{FileStagingKind::File, QStringLiteral("relative.txt")}, file}, {file}, 1},
      BatchCase{"device-path-and-valid", {{FileStagingKind::File, QStringLiteral("\\\\?\\C:\\device.bin")}, file}, {file}, 1},
      BatchCase{"nul-path-and-valid", {{FileStagingKind::File, QStringLiteral("C:\\bad") + QChar(u'\0') + QStringLiteral(".bin")}, file}, {file}, 1},
      BatchCase{"wrong-drive-and-valid", {{FileStagingKind::File, QStringLiteral("D:\\other.txt")}, folder}, {folder}, 1},
      BatchCase{"missing-and-valid", {{FileStagingKind::File, QStringLiteral("C:\\missing")}, file}, {file}, 1},
      BatchCase{"wrong-type-and-valid", {{FileStagingKind::File, folder.path}, file}, {file}, 1},
      BatchCase{"unknown-type", {{static_cast<FileStagingKind>(99), file.path}}, {}, 1},
      BatchCase{"unpaired-high-surrogate", {{FileStagingKind::File, QStringLiteral("C:\\") + QChar(0xd800) + QStringLiteral(".txt")}}, {}, 1},
      BatchCase{"unpaired-low-surrogate", {{FileStagingKind::File, QStringLiteral("C:\\") + QChar(0xdc00) + QStringLiteral(".txt")}}, {}, 1},
      BatchCase{"supplementary-name", {supplementary}, {supplementary}, 0},
      BatchCase{"root-directory", {{FileStagingKind::Directory, QStringLiteral("C:\\")}}, {}, 1}};
  return cases;
}

class FileStagingEditorTests final : public QObject {
  Q_OBJECT
 private slots:
  void extendedDriveSelectionsReachInspectionAndFailOnlyWhenBindingFails() {
    auto source = std::make_shared<SelectionSource>();
    source->targetResolver = [](const QString&) -> app::FileCommitTarget { throw std::runtime_error("Volume is unavailable"); };
    auto store = std::make_shared<test::application::MemoryFileStagingStore>();
    app::FileStagingDraft draft(store, source);
    app::FileStagingEditor editor(QStringLiteral("C:"), draft, source);
    editor.load();
    QSignalSpy rejected(&editor, &app::FileStagingEditing::pathRejected);
    complete(editor.prepareAddEntries({{FileStagingKind::File, QStringLiteral(R"(\\?\Z:\Unavailable\state.bin)")}}));
    QCOMPARE(source->inspections, 1);
    QCOMPARE(source->bindings, 1);
    QCOMPARE(source->driveReads, 0);
    QCOMPARE(rejected.size(), 1);
    QCOMPARE(qvariant_cast<app::FileStagingRejection>(rejected.constFirst().constFirst()).kind, app::FileStagingRejection::Kind::ResolutionFailed);
    QVERIFY(editor.entries().isEmpty());
    QCOMPARE(store->writes, 0);
  }
  void restoringARemovedBoundSelectionRejectsADriveReboundToAnotherVolume() {
    using namespace test::application;
    const FileStagingEntry entry{FileStagingKind::File, QStringLiteral("C:\\state.bin")};
    auto source = std::make_shared<SelectionSource>();
    auto store = std::make_shared<MemoryFileStagingStore>();
    store->entries = {entry};
    app::FileStagingDraft draft(store, source);
    app::FileStagingEditor editor(QStringLiteral("C:"), draft, source);
    editor.load();
    QVERIFY(editor.entries().constFirst().fileTarget);
    QCOMPARE(editor.entries().constFirst().fileTarget->volumeId, std::string(fixtureVolumeId));
    complete(editor.prepareRemovePaths({entry.path}));
    QVERIFY(editor.entries().isEmpty());
    source->targetResolver = [](const QString& path) { return memoryFileTarget(path, otherFixtureVolumeId); };
    QSignalSpy rejected(&editor, &app::FileStagingEditing::pathRejected);
    complete(editor.prepareRestorePaths({entry.path}));
    QCOMPARE(rejected.size(), 1);
    QCOMPARE(qvariant_cast<app::FileStagingRejection>(rejected.constFirst().constFirst()).kind, app::FileStagingRejection::Kind::ResolutionFailed);
    QVERIFY(editor.entries().isEmpty());
    QVERIFY(draft.changes());
    QCOMPARE(draft.changes()->removed().size(), qsizetype{1});
    QCOMPARE(store->writes, 0);
    source->targetResolver = {};
    complete(editor.prepareRestorePaths({entry.path}));
    QCOMPARE(editor.entries().size(), qsizetype{1});
    QVERIFY(!draft.changes());
  }
  void storedGuidAliasesShareProjectionRemovalAndStorageBaseline() {
    using namespace test::application;
    const FileStagingEntry drive{FileStagingKind::File, QStringLiteral("C:\\Data\\state.bin")};
    const FileStagingEntry guid{FileStagingKind::File, QString::fromStdString(compat::volumePath(fixtureVolumeId)) + QStringLiteral("Data\\state.bin")};
    auto source = std::make_shared<SelectionSource>();
    source->targetResolver = [](const QString& path) {
      const auto address = app::paths::splitVolumeFilePath(path);
      if (!address) throw std::invalid_argument("Unexpected test path");
      return app::FileCommitTarget{compat::parseVolumeId(address->volume).value_or(fixtureVolumeId), address->relativePath};
    };
    source->driveResolver = [](const QString&) { return std::string("C:"); };
    auto store = std::make_shared<MemoryFileStagingStore>();
    store->entries = {guid, drive};
    app::FileStagingDraft draft(store, source);
    app::FileStagingEditor editor(QStringLiteral("C:"), draft, source);
    editor.load();
    QCOMPARE(editor.entries().size(), 1);
    QCOMPARE(editor.entries().front().path, drive.path);
    QVERIFY(editor.entries().front().fileTarget);
    QCOMPARE(store->entries, QList<FileStagingEntry>({guid, drive}));
    QCOMPARE(store->writes, 0);
    QVERIFY(!draft.changes());
    QVERIFY(draft.editSnapshot()->storedBefore);
    QCOMPARE(*draft.editSnapshot()->storedBefore, store->entries);
    complete(editor.prepareRemovePaths({guid.path}));
    QVERIFY(editor.entries().isEmpty());
    QVERIFY(draft.changes());
    QCOMPARE(draft.changes()->removed().size(), 1);
    complete(editor.prepareRestorePaths({guid.path}));
    QCOMPARE(editor.entries().size(), 1);
    QVERIFY(!draft.changes());
    complete(editor.prepareRemovePaths({guid.path}));
    const auto reviewed = *draft.changes();
    store->entries = {drive};
    const auto rejected = draft.prepareWrite(reviewed)();
    QVERIFY(rejected.failure);
    QVERIFY(!rejected.attempted);
    store->entries = {guid, drive};
    draft.apply(reviewed);
    QVERIFY(store->entries.isEmpty());
    QCOMPARE(store->writes, 1);
  }
  void sharedProjectionOwnsItsInputAcrossHostCallbacks_data() {
    QTest::addColumn<bool>("destroy");
    QTest::newRow("newer-draft") << false;
    QTest::newRow("destroy-draft") << true;
  }
  void sharedProjectionOwnsItsInputAcrossHostCallbacks() {
    QFETCH(bool, destroy);
    auto store = std::make_shared<test::application::MemoryFileStagingStore>();
    store->entries = {{FileStagingKind::File, QStringLiteral("C:\\Baseline.bin")}};
    auto source = std::make_shared<SelectionSource>();
    auto draft = std::make_unique<app::FileStagingDraft>(store, source);
    (void)draft->load();
    draft->replace({{FileStagingKind::File, QStringLiteral("C:\\Before1.bin")}, {FileStagingKind::File, QStringLiteral("C:\\Before2.bin")}});
    const QPointer lifetime(draft.get());
    const FileStagingEntry latest{FileStagingKind::File, QStringLiteral("C:\\Latest.bin")};
    bool reentered = false;
    source->targetResolver = [&](const QString& path) {
      const auto target = test::application::memoryFileTarget(path);
      if (path.startsWith(u"C:\\Before") && !std::exchange(reentered, true)) {
        if (destroy)
          draft.reset();
        else
          draft->replace({latest});
      }
      return target;
    };
    draft->reconcile();
    QVERIFY(reentered);
    QCOMPARE(lifetime.isNull(), destroy);
    if (!destroy) QCOMPARE(draft->load(), QList<FileStagingEntry>{latest});
  }
  void pagesConsumeOneSharedProjection_data() {
    QTest::addColumn<int>("pageCount");
    QTest::newRow("one-page") << 1;
    QTest::newRow("three-pages") << 3;
  }
  void pagesConsumeOneSharedProjection() {
    QFETCH(int, pageCount);
    auto store = std::make_shared<test::application::MemoryFileStagingStore>();
    for (int i = 0; i < 100; ++i) store->entries.append({FileStagingKind::File, QStringLiteral("C:\\entry%1.bin").arg(i)});
    auto source = std::make_shared<SelectionSource>();
    app::FileStagingDraft draft(store, source);
    (void)draft.load();
    const auto bindings = source->bindings;
    const auto driveReads = source->driveReads;
    std::vector<std::unique_ptr<app::FileStagingEditor>> pages;
    for (int i = 0; i < pageCount; ++i) {
      pages.push_back(std::make_unique<app::FileStagingEditor>(i == 0 ? QStringLiteral("C:") : QStringLiteral("D:"), draft, source));
      pages.back()->load();
    }
    QCOMPARE(source->bindings, bindings);
    QCOMPARE(source->driveReads, driveReads);
    complete(pages.front()->prepareRemovePaths({store->entries.front().path}));
    QCOMPARE(pages.front()->entries().size(), 99);
    QCOMPARE(pages.front()->baselineEntries().size(), 100);
    // 99 target bindings and one root observation, regardless of page count.
    QCOMPARE(source->bindings - bindings, 100);
    QCOMPARE(source->driveReads - driveReads, 99);
    for (const auto& page : pages) page->load();
    QCOMPARE(source->bindings - bindings, 100);
    QCOMPARE(source->driveReads - driveReads, 99);
  }
  void invalidStoredSelectionsAreNeverResolvedAsHostObjects() {
    using namespace test::application;
    SelectionSource source;
    QStringList resolved;
    source.targetResolver = [&](const QString& path) {
      resolved.append(path);
      return memoryFileTarget(path);
    };
    const QList<FileStagingEntry> entries{{FileStagingKind::File, QStringLiteral("relative.bin")},
                                          {FileStagingKind::File, QStringLiteral("\\\\.\\C:\\device.bin")},
                                          {static_cast<FileStagingKind>(99), QStringLiteral("C:\\unknown.bin")},
                                          {FileStagingKind::File, QStringLiteral("C:\\bad") + QChar(u'\0') + QStringLiteral(".bin")},
                                          {FileStagingKind::File, QStringLiteral("C:\\valid.bin")}};
    const auto projected = app::normalizeFileStagingEntries(entries, &source);
    QCOMPARE(resolved, QStringList{QStringLiteral("C:\\valid.bin")});
    QCOMPARE(projected.size(), entries.size());
    for (const auto& entry : entries) QVERIFY(projected.contains(entry));
    for (const auto& entry : projected) QCOMPARE(entry.fileTarget.has_value(), entry.path == QStringLiteral("C:\\valid.bin"));
  }

  void mountedAliasesUseTheActualPageAndShareExclusionChecks() {
    using namespace test::application;
    auto source = std::make_shared<SelectionSource>();
    source->targetResolver = [](const QString& path) {
      const auto actual = path.startsWith(u"C:\\MountedD\\") ? QStringLiteral("D:") + path.mid(11) : path;
      return memoryFileTarget(actual, actual.startsWith(u"D:") ? otherFixtureVolumeId : fixtureVolumeId);
    };
    source->driveResolver = [](const QString& path) { return path.startsWith(u"C:\\MountedD\\") ? std::string("D:") : path.left(2).toStdString(); };
    auto store = std::make_shared<MemoryFileStagingStore>();
    app::FileStagingDraft draft(store, source);
    app::FileStagingConflictPolicy conflicts;
    conflicts.setVolumeIdentity(QStringLiteral("D:"), otherFixtureVolumeId);
    conflicts.setFileExclusions({QStringLiteral("D:\\Data")});
    app::FileStagingEditor c(QStringLiteral("C:"), draft, source);
    app::FileStagingEditor d(QStringLiteral("D:"), draft, source, &conflicts);
    c.load();
    d.load();
    const FileStagingEntry alias{FileStagingKind::File, QStringLiteral("C:\\MountedD\\Data\\file.bin")};
    QSignalSpy wrongPage(&c, &app::FileStagingEditing::pathRejected);
    QSignalSpy excluded(&d, &app::FileStagingEditing::pathRejected);
    complete(c.prepareAddEntries({alias}));
    complete(d.prepareAddEntries({alias}));
    QCOMPARE(wrongPage.size(), 1);
    QCOMPARE(excluded.size(), 1);
    QVERIFY(draft.load().isEmpty());
    conflicts.setFileExclusions({});
    complete(d.prepareAddEntries({alias, {FileStagingKind::File, QStringLiteral("D:\\Data\\file.bin")}}));
    QCOMPARE(draft.load(), QList<FileStagingEntry>{alias});
    QVERIFY(c.entries().isEmpty());
    QCOMPARE(d.entries(), QList<FileStagingEntry>{alias});
    QVERIFY(d.entries().front().fileTarget);
    QCOMPARE(d.entries().front().fileTarget->volumeId, std::string(otherFixtureVolumeId));
    QCOMPARE(store->writes, 0);
    draft.apply(*draft.changes());
    QCOMPARE(store->entries, QList<FileStagingEntry>{alias});
    QCOMPARE(store->writes, 1);
  }

  void explicitAddressReplacementPersistsAndSurvivesTheOldMount_data() {
    QTest::addColumn<QString>("originalDrive");
    for (const auto& originalDrive : {QStringLiteral("C:"), QStringLiteral("E:")}) QTest::newRow(qUtf8Printable(originalDrive)) << originalDrive;
  }
  void explicitAddressReplacementPersistsAndSurvivesTheOldMount() {
    QFETCH(QString, originalDrive);
    using namespace test::application;
    const auto mountRoot = originalDrive + QStringLiteral("\\MountedD");
    const FileStagingEntry alias{FileStagingKind::File, mountRoot + QStringLiteral("\\state.bin")};
    const FileStagingEntry direct{FileStagingKind::File, QStringLiteral("D:\\state.bin")};
    bool mounted = true;
    auto source = std::make_shared<SelectionSource>();
    source->targetResolver = [&](const QString& path) {
      if (path.startsWith(mountRoot + u'\\')) {
        if (!mounted) throw std::runtime_error("Old mount address is unavailable");
        return app::FileCommitTarget{otherFixtureVolumeId, path.mid(mountRoot.size())};
      }
      return memoryFileTarget(path, path.startsWith(u"D:") ? otherFixtureVolumeId : fixtureVolumeId);
    };
    source->driveResolver = [&](const QString& path) { return path.startsWith(mountRoot + u'\\') ? std::string("D:") : path.left(2).toStdString(); };
    auto store = std::make_shared<MemoryFileStagingStore>();
    store->entries = {alias};
    app::FileStagingDraft draft(store, source);
    app::FileStagingEditor editor(QStringLiteral("D:"), draft, source);
    editor.load();
    const auto original = editor.entries().constFirst().fileTarget;
    const auto replaceAddress = [&] {
      complete(editor.prepareRemovePaths({alias.path}));
      complete(editor.prepareAddEntries({direct}));
    };
    replaceAddress();
    QCOMPARE(editor.entries(), QList<FileStagingEntry>{direct});
    QCOMPARE(editor.entries().constFirst().fileTarget, original);
    QCOMPARE(draft.changes()->removed(), QList<FileStagingEntry>{alias});
    QCOMPARE(draft.changes()->added(), QList<FileStagingEntry>{direct});
    QCOMPARE(store->writes, 0);
    complete(editor.prepareRestorePaths({alias.path}));
    QCOMPARE(editor.entries(), QList<FileStagingEntry>{alias});
    QVERIFY(!draft.changes());
    replaceAddress();
    core::UwfSnapshot snapshot;
    snapshot.next.registryExclusions = {std::string(app::kFileStagingRegistryRoot)};
    const auto plan = app::planConfiguration({}, snapshot, draft.editSnapshot());
    QVERIFY(plan.hasStagingChanges());
    QCOMPARE(plan.count(), std::size_t{2});
    QCOMPARE(plan.operations().size(), std::size_t{1});
    draft.apply(*plan.staging);
    QCOMPARE(store->entries, QList<FileStagingEntry>{direct});
    QCOMPARE(store->writes, 1);
    mounted = false;
    draft.reload();
    QCOMPARE(editor.entries(), QList<FileStagingEntry>{direct});
    QVERIFY(!draft.changes());
  }

  void restoringTheSavedSelectionReplacesItsCurrentKind() {
    const FileStagingEntry original{FileStagingKind::File, QStringLiteral("C:\\folder")};
    const FileStagingEntry replacement{FileStagingKind::Directory, original.path};
    auto store = std::make_shared<test::application::MemoryFileStagingStore>();
    store->entries = {original};
    auto source = std::make_shared<SelectionSource>();
    app::FileStagingDraft draft(store, source);
    app::FileStagingEditor editor(QStringLiteral("C:"), draft, source);
    editor.load();
    complete(editor.prepareRemovePaths({original.path}));
    complete(editor.prepareAddEntries({replacement}));
    QCOMPARE(editor.entries(), QList<FileStagingEntry>{replacement});
    QCOMPARE(draft.changes()->count(), std::size_t{2});
    complete(editor.prepareRestorePaths({original.path}));
    QCOMPARE(editor.entries(), QList<FileStagingEntry>{original});
    QVERIFY(!draft.changes());
    QCOMPARE(store->entries, QList<FileStagingEntry>{original});
    QCOMPARE(store->writes, 0);
  }

  void rejectedRestorePublishesNoneOfTheSelectedAddresses() {
    using namespace test::application;
    const auto mountRoot = QStringLiteral("E:\\MountedD");
    const FileStagingEntry first{FileStagingKind::File, mountRoot + QStringLiteral("\\first.bin")};
    const FileStagingEntry second{FileStagingKind::File, mountRoot + QStringLiteral("\\second.bin")};
    const QList<FileStagingEntry> originals{first, second};
    const QList<FileStagingEntry> replacements{{FileStagingKind::File, QStringLiteral("D:\\first.bin")},
                                               {FileStagingKind::File, QStringLiteral("D:\\second.bin")}};
    auto source = std::make_shared<SelectionSource>();
    source->targetResolver = [mountRoot](const QString& path) {
      return path.startsWith(mountRoot + u'\\') ? app::FileCommitTarget{otherFixtureVolumeId, path.mid(mountRoot.size())}
                                                : memoryFileTarget(path, otherFixtureVolumeId);
    };
    source->driveResolver = [](const QString&) { return std::string("D:"); };
    auto store = std::make_shared<MemoryFileStagingStore>();
    store->entries = originals;
    app::FileStagingDraft draft(store, source);
    app::FileStagingConflictPolicy conflicts;
    conflicts.setVolumeIdentity(QStringLiteral("D:"), otherFixtureVolumeId);
    app::FileStagingEditor editor(QStringLiteral("D:"), draft, source, &conflicts);
    editor.load();
    complete(editor.prepareRemovePaths({first.path, second.path}));
    complete(editor.prepareAddEntries(replacements));
    QCOMPARE(editor.entries(), replacements);
    QCOMPARE(draft.changes()->count(), std::size_t{4});
    const auto rawBefore = draft.editSnapshot()->storedBefore;
    conflicts.setFileExclusions({QStringLiteral("D:\\second.bin")});
    QSignalSpy changed(&draft, &app::FileStagingDraft::changed);
    QSignalSpy rejected(&editor, &app::FileStagingEditing::pathRejected);
    complete(editor.prepareRestorePaths({second.path, first.path}));
    QCOMPARE(rejected.size(), 1);
    QCOMPARE(qvariant_cast<app::FileStagingRejection>(rejected.constFirst().constFirst()).kind, app::FileStagingRejection::Kind::ExclusionConflict);
    QCOMPARE(changed.size(), 0);
    QCOMPARE(editor.entries(), replacements);
    QCOMPARE(draft.changes()->count(), std::size_t{4});
    QCOMPARE(draft.editSnapshot()->storedBefore, rawBefore);
    QCOMPARE(store->entries, originals);
    QCOMPARE(store->writes, 0);
    conflicts.setFileExclusions({});
    complete(editor.prepareRestorePaths({second.path, first.path}));
    QCOMPARE(editor.entries(), originals);
    QVERIFY(!draft.changes());
    QCOMPARE(store->entries, originals);
    QCOMPARE(store->writes, 0);
  }

  void acceptedPlanCannotSilentlyRebindASelectedAlias() {
    using namespace test::application;
    auto source = std::make_shared<SelectionSource>();
    auto store = std::make_shared<MemoryFileStagingStore>();
    app::FileStagingDraft draft(store, source);
    const FileStagingEntry entry{FileStagingKind::File, QStringLiteral("C:\\state.bin")};
    draft.replace({entry});
    const auto accepted = draft.changes();
    QVERIFY(accepted);
    QVERIFY(accepted->after.front().fileTarget);
    source->targetResolver = [](const QString& path) { return memoryFileTarget(path, otherFixtureVolumeId); };
    const auto rejected = draft.prepareWrite(*accepted)();
    QVERIFY(!rejected.attempted);
    QVERIFY(rejected.failure);
    QVERIFY_THROWS_EXCEPTION(app::OperationError, std::rethrow_exception(rejected.failure));
    QCOMPARE(store->writes, 0);
  }

  void unresolvedStoredSelectionsRemainVisibleAndRemovable() {
    using namespace test::application;
    const FileStagingEntry offline{FileStagingKind::File, QStringLiteral("C:\\offline.bin")};
    auto store = std::make_shared<MemoryFileStagingStore>();
    store->entries = {offline};
    auto source = std::make_shared<SelectionSource>();
    source->targetResolver = [](const QString&) -> app::FileCommitTarget { throw std::runtime_error("volume unavailable"); };
    source->driveResolver = [](const QString&) -> std::string { throw std::runtime_error("volume unavailable"); };
    app::FileStagingDraft draft(store, source);
    app::FileStagingEditor editor(QStringLiteral("C:"), draft, source);
    editor.load();
    QCOMPARE(editor.entries(), QList<FileStagingEntry>{offline});
    QVERIFY(!editor.entries().front().fileTarget);
    complete(editor.prepareRemovePaths({offline.path}));
    QVERIFY(editor.entries().isEmpty());
    QCOMPARE(store->entries, QList<FileStagingEntry>{offline});
    QCOMPARE(store->writes, 0);
  }

  void fallbackOwnsBothCurrentAndRemovedOfflineTargets_data() {
    QTest::addColumn<bool>("removed");
    QTest::newRow("saved") << false;
    QTest::newRow("pending-removal") << true;
  }
  void fallbackOwnsBothCurrentAndRemovedOfflineTargets() {
    QFETCH(bool, removed);
    using namespace test::application;
    const FileStagingEntry saved{FileStagingKind::File, QStringLiteral("D:\\saved.bin")};
    auto source = std::make_shared<SelectionSource>();
    auto store = std::make_shared<MemoryFileStagingStore>();
    store->entries = {saved};
    app::FileStagingDraft draft(store, source);
    app::FileStagingEditor d(QStringLiteral("D:"), draft, source);
    d.load();
    if (removed) complete(d.prepareRemovePaths({saved.path}));
    source->targetResolver = [](const QString&) -> app::FileCommitTarget { throw std::runtime_error("offline"); };
    source->driveResolver = [](const QString&) -> std::string { throw std::runtime_error("offline"); };
    draft.reconcile();
    app::FileStagingEditor fallback(QStringLiteral("C:"), draft, source, nullptr, {{QStringLiteral("C:")}, QStringLiteral("C:")});
    fallback.load();
    QCOMPARE(fallback.baselineEntries().size(), 1);
    QCOMPARE(fallback.baselineEntries().constFirst().path, saved.path);
    QCOMPARE(fallback.entries().size(), removed ? 0 : 1);
    if (!removed) complete(fallback.prepareRemovePaths({saved.path}));
    QVERIFY(fallback.entries().isEmpty());
    QVERIFY(draft.changes());
    QCOMPARE(draft.changes()->removed().size(), 1);
    QCOMPARE(store->writes, 0);
    draft.apply(*draft.changes());
    QVERIFY(store->entries.isEmpty());
    QVERIFY(fallback.baselineEntries().isEmpty());
    QCOMPARE(store->writes, 1);
  }

  void fallbackDoesNotAdmitAnotherVolumesNewOrRestoredTargets() {
    using namespace test::application;
    const FileStagingEntry saved{FileStagingKind::File, QStringLiteral("D:\\saved.bin")};
    auto source = std::make_shared<SelectionSource>();
    auto store = std::make_shared<MemoryFileStagingStore>();
    store->entries = {saved};
    app::FileStagingDraft draft(store, source);
    app::FileStagingEditor editor(QStringLiteral("C:"), draft, source, nullptr, {{QStringLiteral("C:")}, QStringLiteral("C:")});
    editor.load();
    QCOMPARE(editor.entries().size(), 1);
    complete(editor.prepareRemovePaths({saved.path}));
    QSignalSpy rejected(&editor, &app::FileStagingEditing::pathRejected);
    complete(editor.prepareAddEntries({saved}));
    complete(editor.prepareRestorePaths({saved.path}));
    QVERIFY(editor.entries().isEmpty());
    QCOMPARE(editor.baselineEntries().size(), 1);
    QCOMPARE(rejected.size(), 2);
    for (const auto& rejection : rejected)
      QCOMPARE(qvariant_cast<app::FileStagingRejection>(rejection.front()).kind, app::FileStagingRejection::Kind::WrongDrive);
    QCOMPARE(store->entries, QList<FileStagingEntry>{saved});
    QCOMPARE(store->writes, 0);
  }

  void hostCallbacksCannotCommitAnOlderCandidate_data() {
    QTest::addColumn<int>("boundary");
    QTest::addColumn<int>("outcome");
    QTest::addColumn<bool>("failedCall");
    constexpr std::array boundaries{"inspect", "resolve"};
    constexpr std::array outcomes{"newer-target", "destroy-editor-and-source", "destroy-draft"};
    for (std::size_t boundary = 0; boundary < boundaries.size(); ++boundary)
      for (std::size_t outcome = 0; outcome < outcomes.size(); ++outcome)
        for (const bool failedCall : {false, true})
          QTest::newRow(qUtf8Printable(QStringLiteral("%1/%2/%3")
                                           .arg(QString::fromLatin1(boundaries[boundary]), QString::fromLatin1(outcomes[outcome]),
                                                failedCall ? QStringLiteral("exception") : QStringLiteral("return"))))
              << static_cast<int>(boundary) << static_cast<int>(outcome) << failedCall;
  }
  void hostCallbacksCannotCommitAnOlderCandidate() {
    QFETCH(int, boundary);
    QFETCH(int, outcome);
    QFETCH(bool, failedCall);
    const FileStagingEntry latest{FileStagingKind::File, QStringLiteral("C:\\latest.bin")};
    auto store = std::make_shared<test::application::MemoryFileStagingStore>();
    auto draft = std::make_unique<app::FileStagingDraft>(store);
    auto source = std::make_shared<SelectionSource>();
    const std::weak_ptr sourceLifetime(source);
    auto editor = std::make_unique<app::FileStagingEditor>(QStringLiteral("C:"), *draft, source);
    const QPointer editorLifetime(editor.get());
    const QPointer draftLifetime(draft.get());
    editor->load();
    QSignalSpy errors(editor.get(), &app::FileStagingEditing::updateFailed);
    bool called = false;
    const auto callback = [&] {
      if (std::exchange(called, true)) return;
      if (outcome == 0) {
        draft->replace({latest});
      } else if (outcome == 1) {
        editor.reset();
        source.reset();
      } else {
        draft.reset();
      }
      if (failedCall) throw std::runtime_error("obsolete host call failed");
    };
    if (boundary == 0)
      source->beforeInspection = callback;
    else
      source->beforeResolution = callback;
    auto preparation = editor->prepareAddEntries({{FileStagingKind::File, QStringLiteral("C:\\older.bin")}});
    QVERIFY(draft->busy());
    const auto failure = prepare(preparation);
    // Match the owner lifecycle: a destroyed owner cancels instead of
    // publishing the worker result; a surviving owner rejects stale revisions.
    if (editorLifetime && draftLifetime)
      preparation.complete(failure);
    else
      preparation.cancel();
    preparation = {};
    QVERIFY(called);
    QCOMPARE(editorLifetime.isNull(), outcome == 1);
    QCOMPARE(draftLifetime.isNull(), outcome == 2);
    QCOMPARE(sourceLifetime.expired(), outcome == 1);
    QCOMPARE(errors.size(), 0);
    if (outcome == 0) {
      QCOMPARE(draft->load(), QList<FileStagingEntry>{latest});
      QCOMPARE(editor->entries(), QList<FileStagingEntry>{latest});
    } else if (outcome == 1) {
      QVERIFY(draft->load().isEmpty());
    } else {
      QVERIFY(!editor->storageAvailable());
    }
    if (draft) QVERIFY(!draft->busy());
    QCOMPARE(store->writes, 0);
  }

  void editingEntrypointsShareTheSameReadTransaction_data() {
    QTest::addColumn<int>("operation");
    QTest::addColumn<int>("outcome");
    QTest::addColumn<bool>("failedRead");
    constexpr std::array operations{"add", "remove", "restore"};
    constexpr std::array outcomes{"newer-target", "destroy-editor-and-source", "destroy-draft"};
    for (std::size_t operation = 0; operation < operations.size(); ++operation)
      for (std::size_t outcome = 0; outcome < outcomes.size(); ++outcome)
        for (const bool failedRead : {false, true})
          QTest::newRow(qUtf8Printable(QStringLiteral("%1/%2/%3")
                                           .arg(QString::fromLatin1(operations[operation]), QString::fromLatin1(outcomes[outcome]),
                                                failedRead ? QStringLiteral("exception") : QStringLiteral("return"))))
              << static_cast<int>(operation) << static_cast<int>(outcome) << failedRead;
  }
  void editingEntrypointsShareTheSameReadTransaction() {
    QFETCH(int, operation);
    QFETCH(int, outcome);
    QFETCH(bool, failedRead);
    const FileStagingEntry original{FileStagingKind::File, QStringLiteral("C:\\original.bin")};
    const FileStagingEntry latest{FileStagingKind::File, QStringLiteral("C:\\latest.bin")};
    auto store = std::make_shared<ReentrantStore>();
    store->entries = {original};
    store->failedRead = failedRead;
    auto draft = std::make_unique<app::FileStagingDraft>(store);
    auto source = std::make_shared<SelectionSource>();
    const std::weak_ptr sourceLifetime(source);
    auto editor = std::make_unique<app::FileStagingEditor>(QStringLiteral("C:"), *draft, source);
    const QPointer editorLifetime(editor.get());
    const QPointer draftLifetime(draft.get());
    QSignalSpy errors(editor.get(), &app::FileStagingEditing::updateFailed);
    bool called = false;
    store->beforeRead = [&] {
      called = true;
      if (outcome == 0) {
        draft->replace({latest});
      } else if (outcome == 1) {
        editor.reset();
        source.reset();
      } else {
        draft.reset();
      }
    };
    app::FileStagingPreparation preparation;
    switch (operation) {
      case 0:
        preparation = editor->prepareAddEntries({{FileStagingKind::File, QStringLiteral("C:\\older.bin")}});
        break;
      case 1:
        preparation = editor->prepareRemovePaths({original.path});
        break;
      case 2:
        preparation = editor->prepareRestorePaths({original.path});
        break;
    }
    QVERIFY(draft->busy());
    const auto failure = prepare(preparation);
    if (editorLifetime && draftLifetime)
      preparation.complete(failure);
    else
      preparation.cancel();
    preparation = {};
    QVERIFY(called);
    QCOMPARE(editorLifetime.isNull(), outcome == 1);
    QCOMPARE(draftLifetime.isNull(), outcome == 2);
    QCOMPARE(sourceLifetime.expired(), outcome == 1);
    QCOMPARE(errors.size(), 0);
    if (outcome == 0) {
      QCOMPARE(draft->load(), QList<FileStagingEntry>{latest});
      QCOMPARE(editor->entries(), QList<FileStagingEntry>{latest});
      QCOMPARE(editor->baselineEntries(), QList<FileStagingEntry>{original});
    } else if (outcome == 1) {
      QCOMPARE(draft->load(), QList<FileStagingEntry>{original});
    } else {
      QVERIFY(!editor->storageAvailable());
    }
    if (draft) QVERIFY(!draft->busy());
    QCOMPARE(store->writes, 0);
  }

  void reentrantLoadsPublishOnlyTheCurrentSnapshot_data() {
    QTest::addColumn<int>("boundary");
    QTest::newRow("volume-resolution") << 0;
    QTest::newRow("conflict-notification") << 1;
    QTest::newRow("storage-recovery") << 2;
    QTest::newRow("entries-notification") << 3;
  }
  void reentrantLoadsPublishOnlyTheCurrentSnapshot() {
    QFETCH(int, boundary);
    const FileStagingEntry original{FileStagingKind::File, QStringLiteral("C:\\original.bin")};
    const FileStagingEntry intermediate{FileStagingKind::File, QStringLiteral("C:\\intermediate.bin")};
    const FileStagingEntry latest{FileStagingKind::File, QStringLiteral("C:\\latest.bin")};
    auto store = std::make_shared<test::application::MemoryFileStagingStore>();
    store->entries = {original};
    if (boundary == 2) store->readFailure = QStringLiteral("temporarily unavailable");
    auto source = std::make_shared<SelectionSource>();
    app::FileStagingDraft draft(store, source);
    app::FileStagingConflictPolicy conflicts;
    app::FileStagingEditor editor(QStringLiteral("C:"), draft, source, &conflicts);
    editor.load();
    bool reentered = false;
    const auto editDuringLoad = [&] {
      if (std::exchange(reentered, true)) return;
      draft.replace({latest});
    };
    QList<QList<FileStagingEntry>> publications;
    connect(&editor, &app::FileStagingEditing::entriesChanged, &draft, [&] { publications.append(editor.entries()); });
    connect(&conflicts, &app::FileStagingConflictPolicy::stagedEntriesChanged, &draft, [&] {
      QCOMPARE(editor.entries(), draft.load());
      if (boundary == 1) editDuringLoad();
    });
    if (boundary == 0)
      source->beforeResolution = editDuringLoad;
    else if (boundary == 2)
      connect(&editor, &app::FileStagingEditing::storageAvailabilityChanged, &draft, editDuringLoad);
    else if (boundary == 3)
      connect(&editor, &app::FileStagingEditing::entriesChanged, &draft, editDuringLoad);

    if (boundary == 2) {
      store->readFailure.clear();
      editor.load();
    } else {
      draft.replace({intermediate});
    }
    QVERIFY(reentered);
    QCOMPARE(draft.load(), QList<FileStagingEntry>{latest});
    QCOMPARE(editor.entries(), QList<FileStagingEntry>{latest});
    QCOMPARE(editor.baselineEntries(), QList<FileStagingEntry>{original});
    QVERIFY(editor.storageAvailable());
    QVERIFY(editor.storageError().isEmpty());
    QVERIFY(!publications.isEmpty());
    QCOMPARE(publications.back(), QList<FileStagingEntry>{latest});
    QCOMPARE(publications.size(), boundary == 3 ? 2 : 1);
    QCOMPARE(store->writes, 0);
  }

  void failedLoadObserversMayDestroyOrRecoverTheEditor_data() {
    QTest::addColumn<bool>("destroy");
    QTest::newRow("destroy") << true;
    QTest::newRow("recover") << false;
  }
  void failedLoadObserversMayDestroyOrRecoverTheEditor() {
    QFETCH(bool, destroy);
    const FileStagingEntry saved{FileStagingKind::File, QStringLiteral("C:\\saved.bin")};
    auto store = std::make_shared<test::application::MemoryFileStagingStore>();
    store->entries = {saved};
    store->readFailure = QStringLiteral("temporarily unavailable");
    app::FileStagingDraft draft(store);
    auto editor = std::make_unique<app::FileStagingEditor>(QStringLiteral("C:"), draft, std::make_shared<SelectionSource>());
    const QPointer lifetime(editor.get());
    QSignalSpy unavailable(editor.get(), &app::FileStagingEditing::storageUnavailable);
    connect(editor.get(), &app::FileStagingEditing::storageAvailabilityChanged, &draft, [&] {
      if (destroy) {
        editor.reset();
      } else if (!editor->storageAvailable()) {
        store->readFailure.clear();
        editor->load();
      }
    });
    editor->load();
    QCOMPARE(lifetime.isNull(), destroy);
    QCOMPARE(unavailable.size(), 0);  // The obsolete failure cannot follow recovery or destruction.
    if (!destroy) {
      QVERIFY(editor->storageAvailable());
      QVERIFY(editor->storageError().isEmpty());
      QCOMPARE(editor->entries(), QList<FileStagingEntry>{saved});
    }
    QCOMPARE(store->writes, 0);
  }

  void additionsAndRestorationsShareSetRules_data() {
    QTest::addColumn<bool>("restoreParent");
    QTest::newRow("add-child-restore-parent") << true;
    QTest::newRow("add-parent-restore-child") << false;
  }
  void additionsAndRestorationsShareSetRules() {
    QFETCH(bool, restoreParent);
    const FileStagingEntry parent{FileStagingKind::Directory, QStringLiteral("C:\\folder")};
    const FileStagingEntry child{FileStagingKind::File, QStringLiteral("C:\\folder\\child.txt")};
    const FileStagingEntry offline{FileStagingKind::File, QStringLiteral("Z:\\Offline\\saved.bin")};
    const auto original = restoreParent ? parent : child;
    const auto added = restoreParent ? child : parent;
    auto store = std::make_shared<test::application::MemoryFileStagingStore>();
    store->entries = {original, offline};
    app::FileStagingDraft draft(store);
    app::FileStagingEditor editor(QStringLiteral("C:"), draft, std::make_shared<SelectionSource>());
    editor.load();
    complete(editor.prepareRemovePaths({original.path}));
    complete(editor.prepareAddEntries({added}));
    complete(editor.prepareRestorePaths({original.path}));
    const QList<FileStagingEntry> expected{parent, offline};
    QCOMPARE(draft.load(), expected);
    QCOMPARE(editor.entries(), QList<FileStagingEntry>{parent});
    QCOMPARE(store->entries, QList<FileStagingEntry>({original, offline}));
    QCOMPARE(store->writes, 0);
    if (const auto changes = draft.changes()) draft.apply(*changes);
    QCOMPARE(store->entries, expected);
    QVERIFY(!draft.changes());
  }

  void oneSelectionPublishesOneDraft_data() {
    QTest::addColumn<int>("batchIndex");
    for (std::size_t index = 0; index < batches().size(); ++index) QTest::newRow(batches()[index].name) << static_cast<int>(index);
  }
  void oneSelectionPublishesOneDraft() {
    QFETCH(int, batchIndex);
    const auto& item = batches()[static_cast<std::size_t>(batchIndex)];
    auto store = std::make_shared<test::application::MemoryFileStagingStore>();
    app::FileStagingDraft draft(store);
    auto source = std::make_shared<SelectionSource>();
    app::FileStagingEditor editor(QStringLiteral("C:"), draft, source);
    app::FileStagingEditor peer(QStringLiteral("D:"), draft, source);
    editor.load();
    peer.load();
    QSignalSpy changed(&draft, &app::FileStagingDraft::changed);
    QSignalSpy published(&editor, &app::FileStagingEditing::entriesChanged);
    QSignalSpy peerPublished(&peer, &app::FileStagingEditing::entriesChanged);
    QSignalSpy rejected(&editor, &app::FileStagingEditing::pathRejected);
    connect(&editor, &app::FileStagingEditing::entriesChanged, &editor, [&] { QCOMPARE(editor.entries(), item.expected); });
    connect(&editor, &app::FileStagingEditing::pathRejected, &editor, [&] { QCOMPARE(draft.load(), item.expected); });

    complete(editor.prepareAddEntries(item.selection));
    QCOMPARE(draft.load(), item.expected);
    QCOMPARE(editor.entries(), item.expected);
    QCOMPARE(rejected.size(), item.rejected);
    const int publications = item.expected.isEmpty() ? 0 : 1;
    QCOMPARE(changed.size(), publications);
    QCOMPARE(published.size(), publications);
    QCOMPARE(peerPublished.size(), publications);
    QVERIFY(source->inspections <= item.selection.size());
    QVERIFY(peer.entries().isEmpty());
    QCOMPARE(store->writes, 0);
    // A no-op must not reset either page or publish a second draft version.
    complete(editor.prepareAddEntries(item.expected));
    QCoreApplication::sendPostedEvents();
    QCOMPARE(changed.size(), publications);
    QCOMPARE(published.size(), publications);
    QCOMPARE(peerPublished.size(), publications);
    QCOMPARE(store->writes, 0);
  }

  void rejectedPathDefersAnotherBatchUntilCompletion_data() {
    QTest::addColumn<int>("rejectionPosition");
    QTest::addColumn<bool>("coalesceDirectory");
    constexpr std::array positions{"before", "between", "after"};
    for (std::size_t index = 0; index < positions.size(); ++index)
      for (const bool coalesce : {false, true})
        QTest::newRow(qPrintable(QString::fromLatin1(positions[index]) + (coalesce ? QStringLiteral("/coalesce") : QStringLiteral("/append"))))
            << static_cast<int>(index) << coalesce;
  }

  void rejectedPathDefersAnotherBatchUntilCompletion() {
    QFETCH(int, rejectionPosition);
    QFETCH(bool, coalesceDirectory);
    const FileStagingEntry child{FileStagingKind::File, QStringLiteral("C:\\folder\\child.txt")};
    const FileStagingEntry file{FileStagingKind::File, QStringLiteral("C:\\one.txt")};
    const FileStagingEntry next = coalesceDirectory ? FileStagingEntry{FileStagingKind::Directory, QStringLiteral("C:\\folder")}
                                                    : FileStagingEntry{FileStagingKind::File, QStringLiteral("C:\\two.txt")};
    const QList<FileStagingEntry> firstAccepted{child, file};
    const QList<FileStagingEntry> expected = coalesceDirectory ? QList<FileStagingEntry>{next, file} : QList<FileStagingEntry>{child, file, next};
    auto selection = firstAccepted;
    selection.insert(rejectionPosition, {FileStagingKind::File, QStringLiteral("relative.txt")});

    auto store = std::make_shared<test::application::MemoryFileStagingStore>();
    app::FileStagingDraft draft(store);
    auto source = std::make_shared<SelectionSource>();
    app::FileStagingEditor editor(QStringLiteral("C:"), draft, source);
    app::FileStagingEditor peer(QStringLiteral("C:"), draft, source);
    editor.load();
    peer.load();
    QSignalSpy changed(&draft, &app::FileStagingDraft::changed);
    QSignalSpy peerPublished(&peer, &app::FileStagingEditing::entriesChanged);
    QList<QList<FileStagingEntry>> publications;
    QList<FileStagingEntry> draftAtRejection;
    QList<FileStagingEntry> editorAtRejection;
    QList<FileStagingEntry> peerAtRejection;
    int rejected = 0;
    connect(&editor, &app::FileStagingEditing::entriesChanged, &draft, [&] { publications.append(editor.entries()); });
    connect(&editor, &app::FileStagingEditing::pathRejected, &draft, [&] {
      ++rejected;
      draftAtRejection = draft.load();
      editorAtRejection = editor.entries();
      peerAtRejection = peer.entries();
      QVERIFY(editor.busy());
      const auto nested = editor.prepareAddEntries({next});
      QVERIFY(!nested.prepare);
      QVERIFY(!nested.complete);
      QVERIFY(!nested.cancel);
    });

    complete(editor.prepareAddEntries(selection));
    QCoreApplication::sendPostedEvents();
    QCOMPARE(rejected, 1);
    QCOMPARE(draftAtRejection, firstAccepted);
    QCOMPARE(editorAtRejection, firstAccepted);
    QCOMPARE(peerAtRejection, firstAccepted);
    QCOMPARE(publications, QList<QList<FileStagingEntry>>{firstAccepted});
    QCOMPARE(changed.size(), 1);
    QCOMPARE(peerPublished.size(), 1);
    QVERIFY(!editor.busy());
    // A published rejection belongs to the still-running batch. A new user
    // action is accepted after completion releases that batch's busy state.
    complete(editor.prepareAddEntries({next}));
    const QList<QList<FileStagingEntry>> expectedPublications{firstAccepted, expected};
    QCOMPARE(publications, expectedPublications);
    QCOMPARE(changed.size(), 2);
    QCOMPARE(peerPublished.size(), 2);
    QCOMPARE(draft.load(), expected);
    QCOMPARE(editor.entries(), expected);
    QCOMPARE(peer.entries(), expected);
    QCOMPARE(store->writes, 0);
  }

  void rejectedPathMayDestroyItsEditor_data() {
    QTest::addColumn<bool>("attemptEditBeforeDestroy");
    QTest::newRow("destroy-after-first-publication") << false;
    QTest::newRow("reject-nested-edit-then-destroy") << true;
  }

  void rejectedPathMayDestroyItsEditor() {
    QFETCH(bool, attemptEditBeforeDestroy);
    const FileStagingEntry file{FileStagingKind::File, QStringLiteral("C:\\one.txt")};
    const FileStagingEntry next{FileStagingKind::File, QStringLiteral("C:\\two.txt")};
    const QList<FileStagingEntry> firstAccepted{file};
    auto store = std::make_shared<test::application::MemoryFileStagingStore>();
    app::FileStagingDraft draft(store);
    auto source = std::make_shared<SelectionSource>();
    auto editor = std::make_unique<app::FileStagingEditor>(QStringLiteral("C:"), draft, source);
    const QPointer guarded(editor.get());
    app::FileStagingEditor peer(QStringLiteral("C:"), draft, source);
    editor->load();
    peer.load();
    QSignalSpy changed(&draft, &app::FileStagingDraft::changed);
    QSignalSpy peerPublished(&peer, &app::FileStagingEditing::entriesChanged);
    QList<FileStagingEntry> draftAtRejection;
    QList<FileStagingEntry> peerAtRejection;
    int rejected = 0;
    connect(editor.get(), &app::FileStagingEditing::pathRejected, &draft, [&] {
      ++rejected;
      draftAtRejection = draft.load();
      peerAtRejection = peer.entries();
      QVERIFY(editor->busy());
      if (attemptEditBeforeDestroy) {
        const auto nested = editor->prepareAddEntries({next});
        QVERIFY(!nested.prepare);
        QVERIFY(!nested.complete);
        QVERIFY(!nested.cancel);
      }
      editor.reset();
    });

    // Two pending rejections require the emitter to stop after its lifetime ends.
    complete(editor->prepareAddEntries(
        {{FileStagingKind::File, QStringLiteral("first-relative.txt")}, file, {FileStagingKind::File, QStringLiteral("second-relative.txt")}}));
    QCoreApplication::sendPostedEvents();
    QVERIFY(guarded.isNull());
    QCOMPARE(rejected, 1);
    QCOMPARE(draftAtRejection, firstAccepted);
    QCOMPARE(peerAtRejection, firstAccepted);
    QCOMPARE(changed.size(), 1);
    QCOMPARE(peerPublished.size(), 1);
    QCOMPARE(draft.load(), firstAccepted);
    QCOMPARE(peer.entries(), firstAccepted);
    QVERIFY(!draft.busy());
    QCOMPARE(store->writes, 0);
  }
};
}  // namespace

QTEST_GUILESS_MAIN(FileStagingEditorTests)
#include "FileStagingEditorTests.moc"
