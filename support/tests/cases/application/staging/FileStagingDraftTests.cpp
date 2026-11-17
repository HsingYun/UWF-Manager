// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QPointer>
#include <QSignalSpy>
#include <QTest>
#include <algorithm>
#include <array>
#include <exception>
#include <functional>
#include <memory>
#include <stdexcept>
#include <system_error>
#include <utility>

#include "application/configuration/execution/ConfigurationExecutor.h"
#include "application/operations/contracts/OperationError.h"
#include "application/paths/PathPolicy.h"
#include "application/staging/editing/FileStagingConflictPolicy.h"
#include "application/staging/editing/FileStagingDraft.h"
#include "application/staging/editing/FileStagingEditor.h"
#include "application/staging/persistence/FileStagingRegistry.h"
#include "application/uwf/adapters/UwfConfigurationRepository.h"
#include "domain/uwf/contracts/Types.h"
#include "infrastructure/windows/commit/WindowsCommitTraversalSource.h"
#include "infrastructure/windows/staging/RegistryFileStagingCodec.h"
#include "simulation/uwf/FakeUwfBackend.h"
#include "support/app/FileStagingFixture.h"
#include "support/app/UwfMaintenanceFixture.h"

using namespace uwf;

namespace {
void complete(app::FileStagingPreparation preparation) {
  std::exception_ptr failure;
  try {
    if (preparation.prepare) preparation.prepare({});
  } catch (...) {
    failure = std::current_exception();
  }
  if (preparation.complete) preparation.complete(failure);
}

class DraftBindingSource final : public app::CommitTraversalSource {
 public:
  std::string mountedVolume = "11111111-2222-3333-4444-555555555555";
  QString unavailableAddress;
  bool committing = false;
  app::FileCommitTarget resolveFileTarget(const QString& path) const override {
    if (path == unavailableAddress) throw std::runtime_error("Old address is unavailable");
    const auto address = app::paths::splitVolumeFilePath(path);
    if (!address) throw std::invalid_argument("Unexpected draft path");
    return {compat::parseVolumeId(address->volume).value_or(mountedVolume), address->relativePath};
  }
  std::string driveLetterForPath(const QString&) const override { return "C:"; }
  std::optional<app::CommitNode> inspect(const app::CommitNode& target) const override {
    if (committing) return app::bindFileCommitTarget(*this, target);
    throw std::logic_error("Draft projection must not inspect objects");
  }
  bool exists(const app::CommitNode&) const override { throw std::logic_error("Draft projection must not probe existence"); }
  QString enumerateChildren(const app::CommitNode&, std::stop_token, const app::CommitNodeVisitor&) const override {
    throw std::logic_error("Draft projection must not traverse objects");
  }
};

class ReadbackFailure final : public app::OperationError {
 public:
  ReadbackFailure() : OperationError(std::make_error_code(std::errc::io_error), "generic error category") {}
  const char* what() const noexcept override { return "readback unavailable\nOriginal provider diagnostic"; }
  app::OperationFailureKind failureKind() const noexcept override { return app::OperationFailureKind::Unavailable; }
  app::OperationDiagnostic diagnostic() const noexcept override { return {1234, 5678}; }
};

class Store final : public app::FileStagingStore {
 public:
  QList<app::FileStagingEntry> entries;
  bool failBeforeWrite = false;
  bool failAfterWrite = false;
  bool failReadAfterWrite = false;
  bool classifiedReadFailure = false;
  mutable std::function<void()> beforeRead;
  std::function<void()> duringWrite;
  bool failAfterCallback = false;
  int writes = 0;
  QList<app::FileStagingEntry> load() const override {
    if (writes > 0 && failReadAfterWrite) {
      if (classifiedReadFailure) throw ReadbackFailure();
      throw std::runtime_error("readback unavailable");
    }
    const auto observed = entries;
    if (const auto callback = std::exchange(beforeRead, {})) {
      callback();
      if (failAfterCallback) throw std::runtime_error("obsolete read failed");
    }
    return observed;
  }
  void replace(const QList<app::FileStagingEntry>& value) override {
    if (failBeforeWrite) throw std::runtime_error("write rejected");
    ++writes;
    entries = value;
    if (const auto callback = std::exchange(duringWrite, {})) callback();
    if (failAfterWrite) throw std::runtime_error("durability incomplete");
  }
};
core::UwfSnapshot protectedSnapshot() {
  core::UwfSnapshot snapshot;
  snapshot.current.registryExclusions = snapshot.next.registryExclusions = {std::string(app::kFileStagingRegistryRoot)};
  return snapshot;
}
using Fake = uwf::simulation::FakeUwfBackend;
std::shared_ptr<Fake> protectedBackend() {
  Fake::State state;
  state.current.registryExclusions = state.next.registryExclusions = {std::string(app::kFileStagingRegistryRoot)};
  return std::make_shared<Fake>(std::move(state));
}
std::shared_ptr<app::ConfigurationRepository> settingsRepository(const std::shared_ptr<compat::Uwf>& system,
                                                                 const std::shared_ptr<app::FileStagingDraft>& draft) {
  return app::uwfConfigurationRepository(system, draft);
}
class ObservedSystem final : public compat::Uwf {
 public:
  std::shared_ptr<compat::Uwf> inner;
  std::function<void(const compat::Request&)> before;
  compat::Capability probe(const compat::Request& request) const override { return inner->probe(request); }
  compat::Response execute(const compat::Request& request, const compat::Context& context) const override {
    if (before) before(request);
    return inner->execute(request, context);
  }
};
}  // namespace

class FileStagingDraftTests final : public QObject {
  Q_OBJECT
  const app::FileStagingEntry first{app::FileStagingKind::File, QStringLiteral("C:\\Data\\first.txt")};
  const app::FileStagingEntry second{app::FileStagingKind::Directory, QStringLiteral("D:\\Data\\folder")};
  const app::FileStagingEntry external{app::FileStagingKind::File, QStringLiteral("C:\\Data\\external.txt")};
  const std::string root{app::kFileStagingRegistryRoot};
 private slots:
  void anOldWriteReceiptCannotReplaceANewerObservation() {
    auto store = std::make_shared<Store>();
    store->entries = {first};
    app::FileStagingDraft draft(store);
    static_cast<void>(draft.load());
    draft.replace({second});
    const auto receipt = draft.prepareWrite(*draft.changes())();
    QVERIFY(receipt.attempted);
    QVERIFY(!receipt.failure);
    QVERIFY(receipt.projection);
    store->entries = {external};
    draft.reload();
    draft.replace({first, external});
    const auto revision = draft.revision();
    draft.acceptWrite(receipt);
    QCOMPARE(draft.revision(), revision);
    QCOMPARE(draft.baseline(), QList<app::FileStagingEntry>{external});
    QCOMPARE(draft.load(), QList<app::FileStagingEntry>({external, first}));
    QCOMPARE(*draft.changes()->storedBefore, store->entries);
  }
  void explicitNamespaceAddressChangesAreSaved_data() {
    QTest::addColumn<QString>("alias");
    QTest::newRow("extended-drive") << QStringLiteral(R"(\\?\C:\Data\first.txt)");
    QTest::newRow("volume-guid") << QStringLiteral(R"(\\?\Volume{11111111-2222-3333-4444-555555555555}\Data\first.txt)");
  }
  void explicitNamespaceAddressChangesAreSaved() {
    QFETCH(QString, alias);
    for (const bool reverse : {false, true}) {
      const app::FileStagingEntry before{app::FileStagingKind::File, reverse ? first.path : alias};
      const app::FileStagingEntry after{app::FileStagingKind::File, reverse ? alias : first.path};
      auto store = std::make_shared<Store>();
      store->entries = {before};
      auto source = std::make_shared<DraftBindingSource>();
      app::FileStagingDraft draft(store, source);
      QCOMPARE(draft.load().size(), qsizetype{1});
      QVERIFY(draft.baseline().constFirst().fileTarget);
      draft.replace({after});
      QCOMPARE(draft.load().constFirst().path, after.path);
      QCOMPARE(draft.load().constFirst().fileTarget, draft.baseline().constFirst().fileTarget);
      QVERIFY(draft.editSnapshot());
      QCOMPARE(draft.editSnapshot()->count(), std::size_t{2});
      QVERIFY(draft.changes());
      QCOMPARE(store->writes, 0);
      QCOMPARE(store->entries, QList<app::FileStagingEntry>{before});
      draft.reconcile();
      QCOMPARE(draft.changes()->count(), std::size_t{2});
      QCOMPARE(*draft.changes()->storedBefore, QList<app::FileStagingEntry>{before});
      draft.discardChanges();
      QCOMPARE(draft.load(), QList<app::FileStagingEntry>{before});
      QVERIFY(!draft.changes());
      draft.replace({after});
      draft.apply(*draft.changes());
      QCOMPARE(store->writes, 1);
      QCOMPARE(store->entries, QList<app::FileStagingEntry>{after});
      QVERIFY(!draft.changes());
      draft.reload();
      QCOMPARE(draft.load(), QList<app::FileStagingEntry>{after});
      QVERIFY(!draft.changes());
      source->unavailableAddress = before.path;
      source->committing = true;
      test::application::UwfMaintenanceFixture fixture;
      fixture.state->fileTargetResolver = [source](const QString& path) { return source->resolveFileTarget(path); };
      fixture.stageFile(after.path);
      app::FileStagingCommitter committer(fixture.repository(), source);
      const auto persisted = windows::decodeFileStagingEntries(windows::encodeFileStagingEntries(store->load()));
      QVERIFY(!persisted.constFirst().fileTarget);
      const auto committed = test::application::completeFileStaging(committer, persisted);
      QVERIFY(committed.succeeded());
      QCOMPARE(committed.committedFiles, std::size_t{1});
      QCOMPARE(fixture.state->commits.size(), std::size_t{1});
      QCOMPARE(QString::fromStdString(fixture.state->commits.front()), QStringLiteral("\\Data\\first.txt"));
    }
  }
  void readOnlyAliasProjectionDoesNotCreateAddressIntent() {
    const app::FileStagingEntry alias{app::FileStagingKind::File, QStringLiteral(R"(\\?\Volume{11111111-2222-3333-4444-555555555555}\Data\first.txt)")};
    for (const bool reverse : {false, true}) {
      auto store = std::make_shared<Store>();
      store->entries = reverse ? QList<app::FileStagingEntry>{alias, first} : QList<app::FileStagingEntry>{first, alias};
      auto source = std::make_shared<DraftBindingSource>();
      app::FileStagingDraft draft(store, source);
      QCOMPARE(draft.load(), QList<app::FileStagingEntry>{first});
      draft.reconcile();
      QVERIFY(!draft.changes());
      QCOMPARE(*draft.editSnapshot()->storedBefore, store->entries);
      QCOMPARE(store->writes, 0);
      draft.replace({alias});
      QCOMPARE(draft.changes()->count(), std::size_t{2});
      draft.reconcile();
      QCOMPARE(draft.changes()->count(), std::size_t{2});
      QCOMPARE(*draft.changes()->storedBefore, store->entries);
      draft.apply(*draft.changes());
      QCOMPARE(store->entries, QList<app::FileStagingEntry>{alias});
      QCOMPARE(store->writes, 1);
      draft.reload();
      QVERIFY(!draft.changes());
    }
  }
  void addressIntentSurvivesFailedWritesAndReconciliation_data() {
    QTest::addColumn<int>("failure");
    QTest::newRow("write-rejected") << 0;
    QTest::newRow("durability-incomplete") << 1;
    QTest::newRow("readback-unavailable") << 2;
  }
  void addressIntentSurvivesFailedWritesAndReconciliation() {
    QFETCH(int, failure);
    const app::FileStagingEntry alias{app::FileStagingKind::File, QStringLiteral(R"(\\?\Volume{11111111-2222-3333-4444-555555555555}\Data\first.txt)")};
    auto store = std::make_shared<Store>();
    store->entries = {first};
    auto source = std::make_shared<DraftBindingSource>();
    app::FileStagingDraft draft(store, source);
    static_cast<void>(draft.load());
    draft.replace({alias});
    store->failBeforeWrite = failure == 0;
    store->failAfterWrite = failure == 1;
    store->failReadAfterWrite = failure == 2;
    QVERIFY_THROWS_EXCEPTION(std::exception, draft.apply(*draft.changes()));
    QCOMPARE(draft.reconciliationRequired(), failure == 2);
    QCOMPARE(draft.editSnapshot()->after, QList<app::FileStagingEntry>{alias});
    QVERIFY(draft.changes());
    QVERIFY(draft.changes()->persistencePending);
    store->failBeforeWrite = store->failAfterWrite = store->failReadAfterWrite = false;
    draft.reconcile();
    QVERIFY(!draft.reconciliationRequired());
    QCOMPARE(draft.load(), QList<app::FileStagingEntry>{alias});
    QCOMPARE(*draft.changes()->storedBefore, store->entries);
    draft.apply(*draft.changes());
    QCOMPARE(store->entries, QList<app::FileStagingEntry>{alias});
    QVERIFY(!draft.changes());
  }
  void equivalentNamespaceStorageChangesStillInvalidateAnAcceptedReplacement() {
    const app::FileStagingEntry extended{app::FileStagingKind::File, QStringLiteral(R"(\\?\C:\Data\first.txt)")};
    auto store = std::make_shared<Store>();
    store->entries = {extended};
    auto source = std::make_shared<DraftBindingSource>();
    app::FileStagingDraft draft(store, source);
    static_cast<void>(draft.load());
    draft.replace({first, external});
    QVERIFY(draft.changes());
    QCOMPARE(draft.changes()->count(), std::size_t{3});
    const auto reviewed = *draft.changes();
    store->entries = {first};
    const auto rejected = draft.prepareWrite(reviewed)();
    QVERIFY(rejected.failure);
    QVERIFY(!rejected.attempted);
    QCOMPARE(store->writes, 0);
  }
  void bindingObservationCompletionIsNotASelectionChange() {
    const app::FileStagingEntry unbound{app::FileStagingKind::File, first.path};
    auto bound = unbound;
    bound.fileTarget = app::FileCommitTarget{"11111111-2222-3333-4444-555555555555", QStringLiteral("\\Data\\first.txt")};
    for (const bool reverse : {false, true}) {
      const app::FileStagingChanges changes{{reverse ? bound : unbound}, {reverse ? unbound : bound}};
      QVERIFY(changes.added().isEmpty());
      QVERIFY(changes.removed().isEmpty());
      QCOMPARE(changes.count(), std::size_t{0});
    }
    auto alias = bound;
    alias.path = QStringLiteral(R"(\\?\Volume{11111111-2222-3333-4444-555555555555}\Data\first.txt)");
    for (const bool reverse : {false, true}) {
      const app::FileStagingChanges known{{reverse ? alias : bound}, {reverse ? bound : alias}};
      QCOMPARE(known.added().size(), qsizetype{1});
      QCOMPARE(known.removed().size(), qsizetype{1});
      const app::FileStagingChanges unresolved{{reverse ? alias : unbound}, {reverse ? unbound : alias}};
      QCOMPARE(unresolved.added().size(), qsizetype{1});
      QCOMPARE(unresolved.removed().size(), qsizetype{1});
    }
  }
  void knownDifferentVolumesRemainDifferentSelectionsAtTheSameDisplayPath() {
    auto volumeA = first;
    volumeA.fileTarget = app::FileCommitTarget{"11111111-2222-3333-4444-555555555555", QStringLiteral("\\Data\\first.txt")};
    auto volumeB = volumeA;
    volumeB.fileTarget->volumeId = "66666666-7777-8888-9999-aaaaaaaaaaaa";
    for (const bool reverse : {false, true}) {
      const app::FileStagingChanges changes{{reverse ? volumeB : volumeA}, {reverse ? volumeA : volumeB}};
      QCOMPARE(changes.added().size(), qsizetype{1});
      QCOMPARE(changes.removed().size(), qsizetype{1});
      QCOMPARE(changes.count(), std::size_t{2});
    }
  }
  void acceptedSynchronousWritesOutliveTheirFacade_data() {
    QTest::addColumn<int>("boundary");
    QTest::addColumn<bool>("destroy");
    QTest::addColumn<bool>("failedWrite");
    constexpr std::array boundaries{"write", "confirmation-read", "receipt-notification"};
    for (std::size_t boundary = 0; boundary < boundaries.size(); ++boundary)
      for (const bool destroy : {false, true})
        for (const bool failedWrite : {false, true})
          QTest::newRow(qUtf8Printable(QStringLiteral("%1/%2/%3")
                                           .arg(QString::fromLatin1(boundaries[boundary]), destroy ? QStringLiteral("destroy") : QStringLiteral("retain"),
                                                failedWrite ? QStringLiteral("failure") : QStringLiteral("success"))))
              << static_cast<int>(boundary) << destroy << failedWrite;
  }
  void acceptedSynchronousWritesOutliveTheirFacade() {
    QFETCH(int, boundary);
    QFETCH(bool, destroy);
    QFETCH(bool, failedWrite);
    auto store = std::make_shared<Store>();
    store->entries = {first};
    auto draft = std::make_unique<app::FileStagingDraft>(store);
    static_cast<void>(draft->load());
    draft->replace({second});
    const auto accepted = *draft->changes();
    const QPointer lifetime(draft.get());
    QSignalSpy changed(draft.get(), &app::FileStagingDraft::changed);
    store->failAfterWrite = failedWrite;
    bool called = false;
    const auto callback = [&] {
      called = true;
      if (destroy) draft.reset();
    };
    if (boundary == 0) {
      store->duringWrite = callback;
    } else if (boundary == 1) {
      store->duringWrite = [&] { store->beforeRead = callback; };
    } else {
      connect(draft.get(), &app::FileStagingDraft::changed, this, callback);
    }
    bool threw = false;
    try {
      draft->apply(accepted);
    } catch (const app::OperationError& error) {
      threw = true;
      QCOMPARE(error.writeEffect(), app::WriteEffect::Uncertain);
    }
    QVERIFY(called);
    QCOMPARE(threw, failedWrite);
    QCOMPARE(lifetime.isNull(), destroy);
    QCOMPARE(store->writes, 1);
    QCOMPARE(store->entries, QList<app::FileStagingEntry>{second});
    QCOMPARE(changed.size(), boundary == 2 || !destroy ? 1 : 0);
    if (!destroy) {
      QCOMPARE(draft->baseline(), store->entries);
      QCOMPARE(draft->load(), store->entries);
      QCOMPARE(draft->changes().has_value(), failedWrite);
      if (failedWrite) QVERIFY(draft->changes()->persistencePending);
    }
  }

  void storeCallbacksCannotReplaceNewerState_data() {
    QTest::addColumn<int>("operation");
    QTest::addColumn<bool>("destroy");
    QTest::addColumn<bool>("failedRead");
    constexpr std::array operations{"first-load", "first-edit", "reload", "reconcile"};
    for (std::size_t operation = 0; operation < operations.size(); ++operation)
      for (const bool destroy : {false, true})
        for (const bool failedRead : {false, true})
          QTest::newRow(qUtf8Printable(QStringLiteral("%1/%2/%3")
                                           .arg(QString::fromLatin1(operations[operation]), destroy ? QStringLiteral("destroy") : QStringLiteral("newer-state"),
                                                failedRead ? QStringLiteral("exception") : QStringLiteral("return"))))
              << static_cast<int>(operation) << destroy << failedRead;
  }
  void storeCallbacksCannotReplaceNewerState() {
    QFETCH(int, operation);
    QFETCH(bool, destroy);
    QFETCH(bool, failedRead);
    auto store = std::make_shared<Store>();
    store->entries = {first};
    auto draft = std::make_unique<app::FileStagingDraft>(store);
    const QPointer lifetime(draft.get());
    if (operation >= 2) static_cast<void>(draft->load());
    QSignalSpy changed(draft.get(), &app::FileStagingDraft::changed);
    store->failAfterCallback = failedRead;
    store->beforeRead = [&] {
      if (destroy) {
        draft.reset();
      } else {
        store->entries = {external};
        draft->reload();
        draft->replace({second});
      }
    };
    bool threw = false;
    try {
      switch (operation) {
        case 0:
          static_cast<void>(draft->load());
          break;
        case 1:
          draft->replace({});
          break;
        case 2:
          draft->reload();
          break;
        case 3:
          draft->reconcile();
          break;
      }
    } catch (const std::runtime_error&) {
      threw = true;
    }
    QCOMPARE(lifetime.isNull(), destroy);
    QCOMPARE(threw, destroy);
    QCOMPARE(changed.size(), destroy ? 0 : 2);
    if (!destroy) {
      QCOMPARE(draft->load(), QList<app::FileStagingEntry>{second});
      QCOMPARE(draft->baseline(), QList<app::FileStagingEntry>{external});
      QVERIFY(draft->changes());
      QCOMPARE(*draft->changes()->storedBefore, QList<app::FileStagingEntry>{external});
    }
    QCOMPARE(store->writes, 0);
  }
  void projectionRevisionsTrackIntentRatherThanOnlyObjectEquality_data() {
    QTest::addColumn<int>("intervening");
    QTest::newRow("cached-read") << 0;
    QTest::newRow("changed-and-restored-target") << 1;
    QTest::newRow("same-valued-observation") << 2;
  }
  void projectionRevisionsTrackIntentRatherThanOnlyObjectEquality() {
    QFETCH(int, intervening);
    auto store = std::make_shared<Store>();
    store->entries = {first};
    app::FileStagingDraft draft(store);
    const auto entries = draft.load();
    const auto revision = draft.revision();
    const auto project = draft.prepareProjection({});
    auto projection = project({second}, {});
    QVERIFY(projection);
    QCOMPARE(entries, QList<app::FileStagingEntry>{first});
    if (intervening == 0) {
      QCOMPARE(draft.load(), entries);
      draft.replace(entries);  // An equal target is also a genuine no-op.
    } else if (intervening == 1) {
      draft.replace({second});
      draft.replace(entries);
    } else {
      draft.reload();
    }
    QCOMPARE(draft.acceptProjection(revision, std::move(*projection)), intervening == 0);
    QCOMPARE(draft.load(), intervening == 0 ? QList<app::FileStagingEntry>{second} : entries);
    QCOMPARE(store->writes, 0);
  }

  void legacyRedundancyIsAReadOnlyProjectionWithAnExactStorageBaseline_data() {
    QTest::addColumn<bool>("descendant");
    QTest::newRow("duplicate-object") << false;
    QTest::newRow("covered-descendant") << true;
  }
  void legacyRedundancyIsAReadOnlyProjectionWithAnExactStorageBaseline() {
    QFETCH(bool, descendant);
    const app::FileStagingEntry parent{app::FileStagingKind::Directory, QStringLiteral("C:\\Data\\folder")};
    const auto redundant = descendant ? app::FileStagingEntry{app::FileStagingKind::File, parent.path + QStringLiteral("\\child.bin")} : parent;
    const QList<app::FileStagingEntry> observed{parent, redundant, second};
    const QList<app::FileStagingEntry> target{parent, second};
    auto store = std::make_shared<Store>();
    store->entries = observed;
    app::FileStagingDraft draft(store);
    QCOMPARE(draft.load(), target);
    QCOMPARE(draft.baseline(), target);
    QCOMPARE(store->entries, observed);
    QCOMPARE(store->writes, 0);
    QVERIFY(!draft.changes());
    const auto unchanged = draft.editSnapshot();
    QVERIFY(unchanged && unchanged->storedBefore);
    QCOMPARE(*unchanged->storedBefore, observed);
    // A real accepted edit writes a normalized set; mere load/reload does not.
    auto edited = target;
    edited.prepend(external);
    draft.replace(edited);
    QVERIFY(draft.changes());
    QVERIFY(draft.changes()->count() > 0);
    const auto reviewed = *draft.changes();
    // Even a duplicate-only external mutation is a changed observed baseline,
    // not permission to overwrite a different stored payload.
    if (descendant)
      store->entries.append(redundant);
    else
      store->entries = {parent, second, second};
    const auto rejected = draft.prepareWrite(reviewed)();
    QVERIFY(rejected.failure);
    QVERIFY(!rejected.attempted);
    QCOMPARE(store->writes, 0);
    store->entries = observed;
    draft.apply(reviewed);
    QCOMPARE(store->entries, edited);
    QCOMPARE(store->writes, 1);
    QVERIFY(!draft.changes());
    draft.replace(observed);
    QCOMPARE(draft.load(), target);
    QVERIFY(draft.changes());
    store->entries = observed;
    draft.reload();
    QCOMPARE(draft.baseline(), target);
    QCOMPARE(draft.load(), target);
    QVERIFY(!draft.changes());
  }

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
    const auto backend = protectedBackend();
    store.failBeforeWrite = !partial;
    store.failAfterWrite = partial;
    app::ConfigurationExecutor executor(settingsRepository(backend, draft), draft->changes());
    const auto result = executor.apply(app::planConfiguration({}, snapshot, draft->changes()).changes, snapshot);
    QVERIFY(result.failed());
    QVERIFY(result.reconciliationRequired());
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
  void unreadablePartialWritesRetainIntentUntilReconciliation_data() {
    QTest::addColumn<bool>("classified");
    QTest::newRow("unclassified-read-failure") << false;
    QTest::newRow("classified-read-failure") << true;
  }
  void unreadablePartialWritesRetainIntentUntilReconciliation() {
    QFETCH(bool, classified);
    auto storeOwner = std::make_shared<Store>();
    auto& store = *storeOwner;
    store.entries = {first, second};
    auto draft = std::make_shared<app::FileStagingDraft>(storeOwner);
    static_cast<void>(draft->load());
    draft->replace({second});
    store.failReadAfterWrite = true;
    store.classifiedReadFailure = classified;
    const auto backend = protectedBackend();
    app::ConfigurationExecutor executor(settingsRepository(backend, draft), draft->changes());
    const auto snapshot = protectedSnapshot();
    const auto result = executor.apply(app::planConfiguration({}, snapshot, draft->changes()).changes, snapshot);
    QVERIFY(result.reconciliationRequired());
    QVERIFY(result.staging() && result.staging()->failure);
    try {
      std::rethrow_exception(result.staging()->failure);
    } catch (const app::OperationError& error) {
      QCOMPARE(error.writeEffect(), app::WriteEffect::Uncertain);
      if (classified) {
        const ReadbackFailure original;
        QCOMPARE(error.code(), original.code());
        QCOMPARE(error.failureKind(), original.failureKind());
        QCOMPARE(QString::fromUtf8(error.what()), QString::fromUtf8(original.what()));
        QCOMPARE(error.diagnostic().code, original.diagnostic().code);
        QCOMPARE(error.diagnostic().result, original.diagnostic().result);
      }
    }
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
    auto store = std::make_shared<Store>();
    store->entries = {first};
    auto draft = std::make_shared<app::FileStagingDraft>(store);
    static_cast<void>(draft->load());
    draft->replace({});
    const auto backend = protectedBackend();
    auto observed = std::make_shared<ObservedSystem>();
    observed->inner = backend;
    bool cleanupFollowedSave = false;
    observed->before = [&](const compat::Request& request) {
      if (const auto* edit = std::get_if<compat::EditRegistryExclusion>(&request); edit && edit->action == compat::ExclusionAction::Remove)
        cleanupFollowedSave = store->entries.isEmpty() && store->writes == 1;
    };
    const auto snapshot = protectedSnapshot();
    app::ConfigurationExecutor executor(settingsRepository(observed, draft), draft->changes());
    const auto changes = app::planConfiguration({}, snapshot, draft->changes()).changes;
    QCOMPARE(store->writes, 0);
    QVERIFY(backend->trace().empty());
    const auto result = executor.apply(changes, snapshot);
    QCOMPARE(result.failedOperations(), 0);
    QCOMPARE(result.succeededOperations(), 2);
    QVERIFY(cleanupFollowedSave);
    const auto trace = backend->trace();
    QCOMPARE(trace.size(), std::size_t{1});
    QVERIFY(trace.front().request == compat::Request(compat::EditRegistryExclusion{root, compat::ExclusionAction::Remove}));
    QVERIFY(backend->snapshot().next.registryExclusions.empty());
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
    const auto backend = protectedBackend();
    app::ConfigurationExecutor executor(settingsRepository(backend, draft), draft->changes());
    store.failBeforeWrite = true;
    const auto result = executor.apply(changes, snapshot);
    QVERIFY(result.failed());
    QVERIFY(backend->trace().empty());
    QCOMPARE(store.entries, QList<app::FileStagingEntry>{first});
    QVERIFY(draft->changes());
  }
  void registryDependencyFailureRetainsStagingRepairIntent_data() {
    QTest::addColumn<int>("failureStage");
    QTest::newRow("unused-getter-failure") << 0;
    QTest::newRow("rejected-before-write") << 1;
    QTest::newRow("uncertain-after-removal") << 2;
    QTest::newRow("complete") << 3;
  }
  void registryDependencyFailureRetainsStagingRepairIntent() {
    QFETCH(int, failureStage);
    auto store = std::make_shared<Store>();
    store->entries = {first};
    auto draft = std::make_shared<app::FileStagingDraft>(store);
    static_cast<void>(draft->load());
    draft->replace({});
    const auto backend = protectedBackend();
    if (failureStage != 3) {
      const compat::Request request = failureStage == 0 ? compat::Request{compat::GetRegistryExclusions{compat::Session::Next}}
                                                        : compat::Request{compat::EditRegistryExclusion{root, compat::ExclusionAction::Remove}};
      backend->inject({.request = request,
                       .operation = {},
                       .timing = failureStage == 2 ? Fake::FaultTiming::AfterEffect : Fake::FaultTiming::Before,
                       .kind = compat::ErrorKind::Failed,
                       .message = "Injected cleanup failure",
                       .code = 0});
    }
    const auto plan = app::planConfiguration({}, protectedSnapshot(), draft->changes());
    app::ConfigurationExecutor executor(settingsRepository(backend, draft), plan.staging);
    const auto result = executor.apply(plan.changes, plan.snapshot);
    QVERIFY(store->entries.isEmpty());
    QVERIFY(result.anyWriteConfirmed());
    QVERIFY(result.reconciliationRequired());
    const bool completed = failureStage == 0 || failureStage == 3;
    QCOMPARE(draft->changes().has_value(), !completed);
    if (draft->changes()) QVERIFY(draft->changes()->persistencePending);
    QCOMPARE(result.failedOperations(), completed ? 0 : 1);
    QCOMPARE(result.batchFailures(), 0);
    QCOMPARE(backend->snapshot().next.registryExclusions.empty(), completed || failureStage == 2);
    if (failureStage == 0) {
      const auto primitives = backend->primitiveRequests();
      QVERIFY(std::ranges::none_of(primitives, [](const auto& request) { return !compat::isWrite(request); }));
      QCOMPARE(primitives.size(), std::size_t{1});
    }
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
    complete(editor.prepareRemovePaths({first.path}));
    conflicts.setFileExclusions({first.path});
    QSignalSpy rejected(&editor, &app::FileStagingEditing::pathRejected);
    complete(editor.prepareRestorePaths({first.path}));
    QCOMPARE(rejected.count(), 1);
    QVERIFY(draft.load().isEmpty());
    QCOMPARE(store.entries, QList<app::FileStagingEntry>{first});
    QCOMPARE(store.writes, 0);
  }
};
QTEST_APPLESS_MAIN(FileStagingDraftTests)
#include "FileStagingDraftTests.moc"
