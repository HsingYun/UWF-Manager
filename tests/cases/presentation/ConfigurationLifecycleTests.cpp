// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QSignalSpy>
#include <QTest>

#include "presentation/shell/MainWindowViewModel.h"
#include "support/app/ConfigurationFixture.h"
#include "support/app/FileStagingFixture.h"

namespace {
using namespace uwf;
using namespace uwf::test::application;

class ConfigurationLifecycleTests final : public QObject {
  Q_OBJECT
 private slots:
  void stagingChangesReconcileExclusionsBeforePublishingPending() {
    MutableApplicationStateSource source;
    source.snapshot = editableSnapshot();
    const auto root = QString::fromLatin1(app::kFileStagingRegistryRoot.data(), app::kFileStagingRegistryRoot.size());
    source.snapshot.current.registryExclusions = {root.toStdString()};
    source.snapshot.next.registryExclusions = {root.toStdString()};
    source.volumes = {{"C:", "Volume{c}", "NTFS", "System", 1000, 500, core::VolumeSupport::Supported}};
    auto store = std::make_shared<MemoryFileStagingStore>();
    auto draft = std::make_shared<app::FileStagingDraft>(store);
    presentation::MainWindowViewModel model(source, app::UwfCapability::Available, draft);
    model.initializeConfiguration(16384, QStringLiteral("C:"));
    model.refresh();
    QTRY_VERIFY(!model.refreshing());
    const auto registry = model.configuration()->volume(QStringLiteral("C:"))->registry();
    QCOMPARE(registry->importRemove(root), app::ExclusionImportOutcome::Applied);
    QList<bool> rootRemovalAtPublication;
    connect(&model, &presentation::MainWindowViewModel::pendingChanged, &model,
            [&] { rootRemovalAtPublication.append(registry->draft().removed.contains(root)); });
    QSignalSpy pending(&model, &presentation::MainWindowViewModel::pendingChanged);
    QSignalSpy availability(registry.get(), &presentation::ExclusionListViewModel::removalAvailabilityChanged);
    const QList<app::FileStagingEntry> entries{{app::FileStagingKind::File, QStringLiteral("C:\\State\\saved.bin")}};
    draft->replace(entries);
    QCOMPARE(pending.size(), 1);
    QCOMPARE(availability.size(), 1);
    QCOMPARE(rootRemovalAtPublication, QList<bool>{false});
    QVERIFY(registry->pendingRemoved().isEmpty());
    QVERIFY(!registry->canRemoveSelection({root}));
    QCOMPARE(model.pendingChangeCount(), std::size_t{1});
    QCOMPARE(pending.size(), 1);
    draft->replace({});
    QCOMPARE(pending.size(), 2);
    QCOMPARE(availability.size(), 2);
    QVERIFY(registry->canRemoveSelection({root}));
    QVERIFY(registry->pendingRemoved().isEmpty());
    QCOMPARE(model.pendingChangeCount(), std::size_t{0});
    QCOMPARE(store->writes, 0);
  }

  void stagingProtectionDoesNotDependOnConstructingAView_data() {
    QTest::addColumn<bool>("saved");
    QTest::addColumn<bool>("rebuild");
    for (const bool saved : {false, true})
      for (const bool rebuild : {false, true}) QTest::newRow(qUtf8Printable(QStringLiteral("saved-%1-refresh-%2").arg(saved).arg(rebuild))) << saved << rebuild;
  }
  void stagingProtectionDoesNotDependOnConstructingAView() {
    QFETCH(bool, saved);
    QFETCH(bool, rebuild);
    MutableApplicationStateSource source;
    source.snapshot = editableSnapshot();
    source.snapshot.next.registryExclusions = {std::string(app::kFileStagingRegistryRoot)};
    source.volumes = {{"C:", "Volume{c}", "NTFS", "System", 1000, 500, core::VolumeSupport::Supported}};
    auto store = std::make_shared<MemoryFileStagingStore>();
    const QList<app::FileStagingEntry> entries{{app::FileStagingKind::File, QStringLiteral("C:\\State\\saved.bin")}};
    if (saved) store->entries = entries;
    auto draft = std::make_shared<app::FileStagingDraft>(store);
    presentation::MainWindowViewModel model(source, app::UwfCapability::Available, draft);
    model.initializeConfiguration(16384, QStringLiteral("C:"));
    model.refresh();
    QTRY_VERIFY(!model.refreshing());
    draft->replace(entries);
    if (rebuild) {
      model.refresh();
      QTRY_VERIFY(!model.refreshing());
    }
    const auto registry = model.configuration()->volume(QStringLiteral("C:"))->registry();
    QCOMPARE(registry->importRemove(QString::fromLatin1(app::kFileStagingRegistryRoot.data())), app::ExclusionImportOutcome::RejectedRequired);
    QVERIFY(model.configuration()->pendingChanges().removeRegistryExclusions.empty());
    draft->replace({});
    QCOMPARE(registry->importRemove(QString::fromLatin1(app::kFileStagingRegistryRoot.data())), app::ExclusionImportOutcome::Applied);
    QCOMPARE(store->writes, 0);
  }
  void refreshPublishesOnlyTheCompletedState_data() {
    QTest::addColumn<int>("providerState");
    QTest::addColumn<bool>("savedStaging");
    for (int state = 0; state < 3; ++state)
      for (const bool saved : {false, true}) QTest::newRow(qUtf8Printable(QStringLiteral("provider-%1-staging-%2").arg(state).arg(saved))) << state << saved;
  }
  void refreshPublishesOnlyTheCompletedState() {
    QFETCH(int, providerState);
    QFETCH(bool, savedStaging);
    MutableApplicationStateSource source;
    source.snapshot = editableSnapshot();
    source.snapshot.uwfAvailable = providerState == 0;
    if (providerState == 2) source.failure = QStringLiteral("read failed");
    auto storeOwner = std::make_shared<MemoryFileStagingStore>();
    auto& store = *storeOwner;
    if (savedStaging) store.entries = {{app::FileStagingKind::File, QStringLiteral("C:\\State\\saved.bin")}};
    auto draft = std::make_shared<app::FileStagingDraft>(storeOwner);
    presentation::MainWindowViewModel model(source, app::UwfCapability::Available, draft);
    model.initializeConfiguration(16384, QStringLiteral("C:"));
    QCOMPARE(model.pendingChangeCount(), std::size_t{0});
    QList<std::size_t> observed;
    connect(&model, &presentation::MainWindowViewModel::pendingChanged, &model, [&] { observed.append(model.pendingChangeCount()); });
    model.refresh();
    QTRY_VERIFY(!model.refreshing());
    QCOMPARE(observed, QList<std::size_t>{0});
    QCOMPARE(model.state().hasCommittedState(), providerState != 2);
    QCOMPARE(store.writes, 0);
  }

  void discardIsAtomicAndPreservesTheReconciliationGate_data() {
    QTest::addColumn<bool>("readFails");
    QTest::addColumn<bool>("reconciliation");
    for (const bool fail : {false, true})
      for (const bool reconcile : {false, true})
        QTest::newRow(qUtf8Printable(QStringLiteral("read-fails-%1-reconciliation-%2").arg(fail).arg(reconcile))) << fail << reconcile;
  }
  void discardIsAtomicAndPreservesTheReconciliationGate() {
    QFETCH(bool, readFails);
    QFETCH(bool, reconciliation);
    MutableApplicationStateSource source;
    source.snapshot = editableSnapshot();
    source.volumes = {{"C:", "Volume{c}", "NTFS", "System", 1000, 500, core::VolumeSupport::Supported}};
    auto storeOwner = std::make_shared<MemoryFileStagingStore>();
    auto& store = *storeOwner;
    store.entries = {{app::FileStagingKind::File, QStringLiteral("C:\\State\\saved.bin")}};
    auto draft = std::make_shared<app::FileStagingDraft>(storeOwner);
    presentation::MainWindowViewModel model(source, app::UwfCapability::Available, draft);
    model.initializeConfiguration(16384, QStringLiteral("C:"));
    model.refresh();
    QTRY_VERIFY(!model.refreshing());
    const auto configuration = model.configuration();
    const auto global = configuration->global();
    const auto volume = configuration->volume(QStringLiteral("C:"));
    global->editFilterEnabled(false);
    global->editWarningThreshold(4000);
    volume->status()->editProtection(false);
    volume->registry()->enablePersistence(app::ExclusionEditor::PersistenceItem::DomainSecretKey);
    draft->replace({});
    const auto before = model.pendingChangeCount();
    QVERIFY(before > 0);
    model.initializeConfiguration(16384, QStringLiteral("C:"));
    QCOMPARE(model.configuration(), configuration);
    QCOMPARE(model.pendingChangeCount(), before);
    if (reconciliation) model.requireReconciliation();
    if (readFails) store.readFailure = QStringLiteral("storage unavailable");
    QList<std::size_t> observed;
    connect(&model, &presentation::MainWindowViewModel::pendingChanged, &model, [&] { observed.append(model.pendingChangeCount()); });
    if (readFails) {
      QVERIFY_THROWS_EXCEPTION(std::runtime_error, model.discardPendingChanges());
      QCOMPARE(model.pendingChangeCount(), before);
      QVERIFY(observed.isEmpty());
      QVERIFY(draft->load().isEmpty());
    } else {
      model.discardPendingChanges();
      QCOMPARE(observed, QList<std::size_t>{0});
      QCOMPARE(model.pendingChangeCount(), std::size_t{0});
      QVERIFY(configuration->pendingChanges().empty());
      QCOMPARE(draft->load(), store.entries);
      QVERIFY(!draft->changes());
      model.discardPendingChanges();
      QCOMPARE(model.pendingChangeCount(), std::size_t{0});
    }
    QCOMPARE(model.configuration(), configuration);
    QCOMPARE(configuration->volume(QStringLiteral("C:")), volume);
    QCOMPARE(model.state().reconciliationRequired(), reconciliation);
    QCOMPARE(source.reads, 1);
    QCOMPARE(store.writes, 0);
  }
};
}  // namespace
QTEST_GUILESS_MAIN(ConfigurationLifecycleTests)
#include "ConfigurationLifecycleTests.moc"
