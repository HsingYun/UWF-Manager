// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QSignalSpy>
#include <QTest>
#include <algorithm>
#include <stdexcept>

#include "app/configuration/ConfigurationExecutor.h"
#include "app/staging/FileStagingConflictPolicy.h"
#include "app/staging/FileStagingDraft.h"
#include "app/staging/FileStagingEditor.h"
#include "support/app/RecordingWmiOperations.h"
#include "windows/commit/WindowsCommitTraversalSource.h"

using namespace uwf;
namespace {
class Store final : public app::FileStagingStore {
 public:
  QList<app::FileStagingEntry> entries;
  bool failBeforeWrite = false;
  bool failAfterWrite = false;
  bool failReadAfterWrite = false;
  int writes = 0;
  QList<app::FileStagingEntry> load() const override {
    if (writes > 0 && failReadAfterWrite) throw std::runtime_error("readback unavailable");
    return entries;
  }
  void replace(const QList<app::FileStagingEntry>& value) override {
    if (failBeforeWrite) throw std::runtime_error("write rejected");
    ++writes;
    entries = value;
    if (failAfterWrite) throw std::runtime_error("durability incomplete");
  }
};
core::UwfSnapshot protectedSnapshot() {
  core::UwfSnapshot snapshot;
  snapshot.current.registryExclusions = snapshot.next.registryExclusions = {std::string(app::kFileStagingRegistryRoot)};
  return snapshot;
}
}  // namespace

class FileStagingDraftTests final : public QObject {
  Q_OBJECT
  const app::FileStagingEntry first{app::FileStagingKind::File, QStringLiteral("C:\\Data\\first.txt")};
  const app::FileStagingEntry second{app::FileStagingKind::Directory, QStringLiteral("D:\\Data\\folder")};
  const app::FileStagingEntry external{app::FileStagingKind::File, QStringLiteral("C:\\Data\\external.txt")};
  const std::string root{app::kFileStagingRegistryRoot};
 private slots:
  void clearingTheListOnlyCreatesAPreviewableDraft() {
    auto storeOwner = std::make_shared<Store>();
    auto& store = *storeOwner;
    store.entries = {first, second};
    app::FileStagingDraft draft(storeOwner);
    QCOMPARE(draft.load(), store.entries);
    draft.replace({});
    QCOMPARE(store.writes, 0);
    QCOMPARE(store.entries, QList<app::FileStagingEntry>({first, second}));
    const auto changes = draft.changes();
    QVERIFY(changes);
    QCOMPARE(changes->removed(), store.entries);
    QVERIFY(changes->added().isEmpty());
    QCOMPARE(changes->count(), std::size_t{2});
    const auto plan = app::planConfiguration({}, protectedSnapshot(), draft.changes()).changes;
    QCOMPARE(plan.removeRegistryExclusions, std::vector<std::string>{root});
    QCOMPARE(store.writes, 0);
  }
  void retainingAnotherVolumeKeepsTheRequiredExclusion() {
    auto storeOwner = std::make_shared<Store>();
    auto& store = *storeOwner;
    store.entries = {first, second};
    app::FileStagingDraft draft(storeOwner);
    static_cast<void>(draft.load());
    draft.replace({second});
    QVERIFY(app::planConfiguration({}, protectedSnapshot(), draft.changes()).changes.removeRegistryExclusions.empty());
    QCOMPARE(draft.changes()->removed(), QList<app::FileStagingEntry>{first});
    QCOMPARE(store.entries.size(), 2);
  }
  void cancelAndDiscardNeverWriteTheStore() {
    auto storeOwner = std::make_shared<Store>();
    auto& store = *storeOwner;
    store.entries = {first};
    app::FileStagingDraft draft(storeOwner);
    static_cast<void>(draft.load());
    draft.replace({});
    const auto cancelledPlan = draft.changes();
    QVERIFY(cancelledPlan);
    QVERIFY(draft.changes());
    draft.reload();
    QVERIFY(!draft.changes());
    QCOMPARE(draft.load(), QList<app::FileStagingEntry>{first});
    QCOMPARE(store.writes, 0);
  }
  void applyIsTheOnlyPersistentWrite() {
    auto storeOwner = std::make_shared<Store>();
    auto& store = *storeOwner;
    store.entries = {first, second};
    app::FileStagingDraft draft(storeOwner);
    static_cast<void>(draft.load());
    draft.replace({second});
    draft.apply(*draft.changes());
    QCOMPARE(store.writes, 1);
    QCOMPARE(store.entries, QList<app::FileStagingEntry>{second});
    QVERIFY(!draft.changes());
  }
  void rejectedAndPartialWritesKeepTheDraftForRetry_data() {
    QTest::addColumn<bool>("partial");
    QTest::newRow("before-write") << false;
    QTest::newRow("after-write") << true;
  }
  void rejectedAndPartialWritesKeepTheDraftForRetry() {
    QFETCH(bool, partial);
    auto storeOwner = std::make_shared<Store>();
    auto& store = *storeOwner;
    store.entries = {first};
    app::FileStagingDraft draft(storeOwner);
    static_cast<void>(draft.load());
    draft.replace({});
    const auto plan = *draft.changes();
    store.failBeforeWrite = !partial;
    store.failAfterWrite = partial;
    QVERIFY_THROWS_EXCEPTION(std::runtime_error, draft.apply(plan));
    QVERIFY(draft.changes());
    QCOMPARE(draft.baseline(), partial ? QList<app::FileStagingEntry>{} : QList<app::FileStagingEntry>{first});
    QVERIFY(draft.changes()->persistencePending);
    store.failBeforeWrite = store.failAfterWrite = false;
    draft.apply(plan);
    QVERIFY(store.entries.isEmpty());
    QVERIFY(!draft.changes());
  }
  void anExternalEditCannotBeOverwrittenByAStalePlan() {
    auto storeOwner = std::make_shared<Store>();
    auto& store = *storeOwner;
    store.entries = {first};
    app::FileStagingDraft draft(storeOwner);
    static_cast<void>(draft.load());
    draft.replace({});
    const auto plan = *draft.changes();
    store.entries.append(external);
    QVERIFY_THROWS_EXCEPTION(std::runtime_error, draft.apply(plan));
    QCOMPARE(store.entries, QList<app::FileStagingEntry>({first, external}));
    QCOMPARE(store.writes, 0);
    QVERIFY(draft.changes());
  }
  void failedApplyReconcilesObservedStorageWithoutLosingIntent_data() {
    QTest::addColumn<bool>("partial");
    QTest::addColumn<bool>("restoreOriginal");
    for (const bool partial : {false, true})
      for (const bool restore : {false, true})
        QTest::newRow(qUtf8Printable(QStringLiteral("partial-%1-restore-%2").arg(partial).arg(restore))) << partial << restore;
  }
  void failedApplyReconcilesObservedStorageWithoutLosingIntent() {
    QFETCH(bool, partial);
    QFETCH(bool, restoreOriginal);
    auto storeOwner = std::make_shared<Store>();
    auto& store = *storeOwner;
    store.entries = {first, second};
    auto draft = std::make_shared<app::FileStagingDraft>(storeOwner);
    static_cast<void>(draft->load());
    draft->replace({second});
    const auto snapshot = protectedSnapshot();
    test::application::RecordingWmiOperations wmi;
    store.failBeforeWrite = !partial;
    store.failAfterWrite = partial;
    app::ConfigurationExecutor executor(wmi, [](const std::string&) { return std::vector<std::string>{}; }, draft, draft->changes());
    const auto result = executor.apply(app::planConfiguration({}, snapshot, draft->changes()).changes, snapshot);
    QVERIFY(result.failedOperations > 0);
    QVERIFY(result.reconciliationRequired);
    QCOMPARE(draft->baseline(), store.entries);
    QCOMPARE(draft->load(), QList<app::FileStagingEntry>{second});
    if (restoreOriginal) draft->replace({first, second});
    QVERIFY(draft->changes());
    QVERIFY(draft->changes()->count() > 0);
    store.failBeforeWrite = store.failAfterWrite = false;
    draft->apply(*draft->changes());
    QCOMPARE(store.entries, draft->load());
    QVERIFY(!draft->changes());
  }
  void anotherDraftEditInvalidatesAnOpenedPlan() {
    auto storeOwner = std::make_shared<Store>();
    auto& store = *storeOwner;
    store.entries = {first};
    app::FileStagingDraft draft(storeOwner);
    static_cast<void>(draft.load());
    draft.replace({});
    const auto plan = *draft.changes();
    draft.replace({second});
    QVERIFY_THROWS_EXCEPTION(std::runtime_error, draft.apply(plan));
    QCOMPARE(store.writes, 0);
  }
  void unreadablePartialWritesRetainIntentUntilReconciliation() {
    auto storeOwner = std::make_shared<Store>();
    auto& store = *storeOwner;
    store.entries = {first, second};
    auto draft = std::make_shared<app::FileStagingDraft>(storeOwner);
    static_cast<void>(draft->load());
    draft->replace({second});
    store.failReadAfterWrite = true;
    test::application::RecordingWmiOperations wmi;
    app::ConfigurationExecutor executor(wmi, [](const std::string&) { return std::vector<std::string>{}; }, draft, draft->changes());
    const auto snapshot = protectedSnapshot();
    const auto result = executor.apply(app::planConfiguration({}, snapshot, draft->changes()).changes, snapshot);
    QVERIFY(result.reconciliationRequired);
    QVERIFY(draft->reconciliationRequired());
    QVERIFY(draft->changes());
    QVERIFY_THROWS_EXCEPTION(std::runtime_error, static_cast<void>(draft->load()));
    QCOMPARE(store.entries, QList<app::FileStagingEntry>{second});
    store.failReadAfterWrite = false;
    draft->reconcile();
    QVERIFY(!draft->reconciliationRequired());
    QCOMPARE(draft->load(), QList<app::FileStagingEntry>{second});
    QCOMPARE(draft->baseline(), store.entries);
    QVERIFY(draft->changes()->persistencePending);
    draft->replace({first, second});
    draft->apply(*draft->changes());
    QCOMPARE(store.entries, QList<app::FileStagingEntry>({first, second}));
    QVERIFY(!draft->changes());
  }
  void anInitiallyEmptyListDoesNotScheduleExclusionCleanup() {
    auto storeOwner = std::make_shared<Store>();
    auto& store = *storeOwner;
    app::FileStagingDraft draft(storeOwner);
    static_cast<void>(draft.load());
    QVERIFY(app::planConfiguration({}, protectedSnapshot(), draft.changes()).changes.empty());
    QCOMPARE(store.writes, 0);
  }
  void additionsAndAncestorExclusionsUseTheTargetConfiguration() {
    auto storeOwner = std::make_shared<Store>();
    app::FileStagingDraft draft(storeOwner);
    static_cast<void>(draft.load());
    draft.replace({first});
    auto snapshot = protectedSnapshot();
    snapshot.next.registryExclusions = {"HKEY_LOCAL_MACHINE\\SOFTWARE\\HsingYun"};
    QVERIFY(app::planConfiguration({}, snapshot, draft.changes()).changes.addRegistryExclusions.empty());
    core::PendingChanges edits;
    edits.removeRegistryExclusions = snapshot.next.registryExclusions;
    QCOMPARE(app::planConfiguration(edits, snapshot, draft.changes()).changes.addRegistryExclusions, std::vector<std::string>{root});
    snapshot.next.registryExclusions.clear();
    QCOMPARE(app::planConfiguration({}, snapshot, draft.changes()).changes.addRegistryExclusions, std::vector<std::string>{root});
  }
  void exclusionCleanupRunsOnlyAfterTheAcceptedListWasSaved() {
    auto storeOwner = std::make_shared<Store>();
    auto& store = *storeOwner;
    store.entries = {first};
    auto draft = std::make_shared<app::FileStagingDraft>(storeOwner);
    static_cast<void>(draft->load());
    draft->replace({});
    const auto plan = *draft->changes();
    const auto snapshot = protectedSnapshot();
    test::application::RecordingWmiOperations wmi;
    wmi.queryResults.push_back({{{"__PATH", app::wmi::WmiValue::fromString("registry-next")},
                                 {"CurrentSession", app::wmi::WmiValue::fromBool(false)},
                                 {"PersistDomainSecretKey", app::wmi::WmiValue::fromBool(false)},
                                 {"PersistTSCAL", app::wmi::WmiValue::fromBool(false)}}});
    app::wmi::WmiMethodOutput present, absent;
    present.values.emplace("bFound", app::wmi::WmiValue::fromBool(true));
    absent.values.emplace("bFound", app::wmi::WmiValue::fromBool(false));
    wmi.readResults = {absent};
    bool cleanupFollowedSave = false;
    wmi.invocationObserver = [&] { cleanupFollowedSave = store.entries.isEmpty() && store.writes == 1; };
    app::ConfigurationExecutor executor(wmi, [](const std::string&) { return std::vector<std::string>{}; }, draft, plan);
    const auto changes = app::planConfiguration({}, snapshot, draft->changes()).changes;
    QCOMPARE(store.writes, 0);
    QVERIFY(wmi.invocations.empty());
    const auto result = executor.apply(changes, snapshot);
    QCOMPARE(result.failedOperations, 0);
    QCOMPARE(result.succeededOperations, 2);
    QVERIFY(cleanupFollowedSave);
    QCOMPARE(wmi.invocations, std::vector<QString>{QStringLiteral("RemoveExclusion")});
    QVERIFY(!draft->changes());
  }
  void aFailedListSaveCannotRemoveItsRegistryExclusion() {
    auto storeOwner = std::make_shared<Store>();
    auto& store = *storeOwner;
    store.entries = {first};
    auto draft = std::make_shared<app::FileStagingDraft>(storeOwner);
    static_cast<void>(draft->load());
    draft->replace({});
    const auto snapshot = protectedSnapshot();
    const auto changes = app::planConfiguration({}, snapshot, draft->changes()).changes;
    test::application::RecordingWmiOperations wmi;
    app::ConfigurationExecutor executor(wmi, [](const std::string&) { return std::vector<std::string>{}; }, draft, draft->changes());
    store.failBeforeWrite = true;
    const auto result = executor.apply(changes, snapshot);
    QVERIFY(result.failedOperations > 0);
    QVERIFY(wmi.invocations.empty());
    QCOMPARE(store.entries, QList<app::FileStagingEntry>{first});
    QVERIFY(draft->changes());
  }
  void registryDependencyFailureRetainsStagingRepairIntent_data() {
    QTest::addColumn<int>("failureStage");
    QTest::newRow("read") << 0;
    QTest::newRow("write") << 1;
    QTest::newRow("verification") << 2;
    QTest::newRow("complete") << 3;
  }
  void registryDependencyFailureRetainsStagingRepairIntent() {
    QFETCH(int, failureStage);
    auto store = std::make_shared<Store>();
    store->entries = {first};
    auto draft = std::make_shared<app::FileStagingDraft>(store);
    static_cast<void>(draft->load());
    draft->replace({});
    test::application::RecordingWmiOperations wmi;
    if (failureStage != 0)
      wmi.queryResults.push_back({{{"__PATH", app::wmi::WmiValue::fromString("registry-next")},
                                   {"CurrentSession", app::wmi::WmiValue::fromBool(false)},
                                   {"PersistDomainSecretKey", app::wmi::WmiValue::fromBool(false)},
                                   {"PersistTSCAL", app::wmi::WmiValue::fromBool(false)}}});
    if (failureStage == 1) wmi.invocationFailure = std::make_exception_ptr(std::runtime_error("write rejected"));
    app::wmi::WmiMethodOutput observed;
    observed.values.emplace("bFound", app::wmi::WmiValue::fromBool(failureStage != 3));
    wmi.readResults = {observed};
    const auto plan = app::planConfiguration({}, protectedSnapshot(), draft->changes());
    app::ConfigurationExecutor executor(wmi, [](const std::string&) { return std::vector<std::string>{}; }, draft, plan.staging);
    const auto result = executor.apply(plan.changes, plan.snapshot);
    QVERIFY(store->entries.isEmpty());
    QVERIFY(result.anyWriteConfirmed);
    QVERIFY(result.reconciliationRequired);
    QCOMPARE(draft->changes().has_value(), failureStage != 3);
    if (draft->changes()) QVERIFY(draft->changes()->persistencePending);
    for (const auto& operation : plan.operations()) {
      QCOMPARE(std::ranges::count_if(result.steps, [&](const auto& step) { return step.operation == operation; }), 1);
    }
  }
  void restoringARemovalStillHonorsNewFileExclusions() {
    auto storeOwner = std::make_shared<Store>();
    auto& store = *storeOwner;
    store.entries = {first};
    app::FileStagingDraft draft(storeOwner);
    app::FileStagingConflictPolicy conflicts;
    app::FileStagingEditor editor(QStringLiteral("C:"), draft, uwf::windows::systemCommitTraversalSource(), &conflicts);
    editor.load();
    editor.removePaths({first.path});
    conflicts.setFileExclusions({first.path});
    QSignalSpy rejected(&editor, &app::FileStagingEditing::pathRejected);
    editor.restorePaths({first.path});
    QCOMPARE(rejected.count(), 1);
    QVERIFY(draft.load().isEmpty());
    QCOMPARE(store.entries, QList<app::FileStagingEntry>{first});
    QCOMPARE(store.writes, 0);
  }
};
QTEST_APPLESS_MAIN(FileStagingDraftTests)
#include "FileStagingDraftTests.moc"
