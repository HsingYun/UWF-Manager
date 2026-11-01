// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QPointer>
#include <QSignalSpy>
#include <QTest>
#include <QThreadPool>
#include <utility>

#include "presentation/models/ItemRoles.h"
#include "presentation/viewmodels/configuration/OverlayAdvancedViewModel.h"
#include "presentation/viewmodels/shell/MainWindowViewModel.h"
#include "support/app/ConfigurationFixture.h"
#include "support/app/FileStagingFixture.h"
#include "support/app/OverlayPolicyFixture.h"
#include "support/app/ReadOnlyFileBrowserFixture.h"

namespace {
using namespace uwf;
using namespace uwf::test::application;

class ReviewActions final : public app::ConfigurationActions {
 public:
  int preparations = 0;
  ApplyOperation prepareApply(const core::PendingChanges&, const core::UwfSnapshot&) override {
    ++preparations;
    return [] { return app::ConfigurationApplyResult{}; };
  }
};

class ConfigurationLifecycleTests final : public QObject {
  Q_OBJECT
 private slots:
  void globalSnapshotPublicationUsesTheConfigurationTransaction_data() {
    QTest::addColumn<bool>("releaseOwner");
    QTest::addColumn<bool>("discard");
    for (const bool releaseOwner : {false, true})
      for (const bool discard : {false, true})
        QTest::newRow(qPrintable(QStringLiteral("release-%1/discard-%2").arg(releaseOwner).arg(discard))) << releaseOwner << discard;
  }
  void globalSnapshotPublicationUsesTheConfigurationTransaction() {
    QFETCH(bool, releaseOwner);
    QFETCH(bool, discard);
    auto configuration = std::make_unique<presentation::ConfigurationViewModel>(16384);
    const auto model = configuration->global();
    auto initial = editableSnapshot();
    initial.current.filter.enabled = false;
    configuration->resetSnapshot(initial);
    const auto editor = model->editor();
    auto observed = initial;
    observed.current.filter.enabled = true;
    observed.next.overlay.maximumSizeMb = 12000;
    observed.elevated = false;
    auto newer = initial;
    newer.next.overlay.maximumSizeMb = 16000;
    newer.elevated = false;
    bool notified = false;
    connect(configuration.get(), &presentation::ConfigurationViewModel::pendingChanged, this, [&] {
      if (std::exchange(notified, true)) return;
      QVERIFY(model->currentFilterEnabled());
      QVERIFY(!model->overlayStorageEditingAllowed());
      QCOMPARE(model->draft().overlay.maximumSizeMb, uint32_t{12000});
      QVERIFY(!model->controlsEnabled());
      QVERIFY(configuration->pendingChanges().empty());
      if (releaseOwner)
        configuration.reset();
      else
        configuration->resetSnapshot(newer);
    });
    int sharedEditorNotifications = 0;
    connect(editor.get(), &app::GlobalConfigurationEditor::stateChanged, this, [&] { ++sharedEditorNotifications; });
    if (discard)
      configuration->discardChanges(observed);
    else
      configuration->resetSnapshot(observed);
    QVERIFY(notified);
    QVERIFY(sharedEditorNotifications > 0);
    QCOMPARE(editor->draft().overlay.maximumSizeMb, releaseOwner ? uint32_t{12000} : uint32_t{16000});
    if (releaseOwner) {
      QVERIFY(!configuration);
    } else {
      QVERIFY(!model->controlsEnabled());
      QVERIFY(!model->currentFilterEnabled());
      QVERIFY(!model->overlayStorageEditingAllowed());
    }
  }
  void volumeChildPublicationCannotResumeAnOlderContext_data() {
    QTest::addColumn<int>("notification");
    QTest::addColumn<bool>("releaseOwner");
    for (int notification = 0; notification < 4; ++notification)
      for (const bool releaseOwner : {false, true})
        QTest::newRow(qPrintable(QStringLiteral("notification-%1/release-%2").arg(notification).arg(releaseOwner))) << notification << releaseOwner;
  }
  void volumeChildPublicationCannotResumeAnOlderContext() {
    QFETCH(int, notification);
    QFETCH(bool, releaseOwner);
    const core::VolumeInfo physical{"C:", fixtureVolumeCName, "NTFS", "System", 1000, 500, core::VolumeSupport::Supported};
    auto model = std::make_unique<presentation::VolumeConfigurationViewModel>(physical, true);
    const auto observed = editableSnapshot();
    model->applySnapshot(observed, false, std::uint32_t{0});
    const auto files = model->files();
    const auto registry = model->registry();
    bool notified = false;
    const auto observe = [&] {
      if (std::exchange(notified, true)) return;
      if (releaseOwner)
        model.reset();
      else
        model->setCommitContext(false, QStringLiteral("newer context"));
    };
    if (notification < 2)
      connect(notification == 0 ? files.get() : registry.get(), &QAbstractItemModel::dataChanged, this, observe);
    else
      connect(notification == 2 ? files.get() : registry.get(), &presentation::ExclusionListViewModel::commitAvailabilityChanged, this, observe);
    if (notification < 2)
      model->applySnapshot(observed, false, std::uint32_t{4});
    else
      model->setCommitContext(false, QStringLiteral("older context"));
    QVERIFY(notified);
    if (releaseOwner) {
      QVERIFY(!model);
    } else {
      QVERIFY(!files->commitEnabled());
      QVERIFY(!registry->commitEnabled());
      QCOMPARE(files->commitRestriction(), QStringLiteral("newer context"));
      QCOMPARE(registry->commitRestriction(), QStringLiteral("newer context"));
    }
  }
  void configurationResetMayDestroyItsPresentation_data() {
    QTest::addColumn<bool>("volumeNotification");
    QTest::newRow("global-admission") << false;
    QTest::newRow("volume-observation") << true;
  }
  void configurationResetMayDestroyItsPresentation() {
    QFETCH(bool, volumeNotification);
    auto configuration = std::make_unique<presentation::ConfigurationViewModel>(16384);
    const core::VolumeInfo physical{"C:", fixtureVolumeCName, "NTFS", "System", 1000, 500, core::VolumeSupport::Supported};
    const auto volume = configuration->addVolume(physical, true);
    const auto global = configuration->global();
    configuration->resetSnapshot(editableSnapshot());
    const auto destroy = [&] { configuration.reset(); };
    if (volumeNotification)
      connect(volume->status().get(), &presentation::VolumeStatusViewModel::stateChanged, this, destroy);
    else
      connect(global.get(), &presentation::GlobalConfigurationViewModel::availabilityChanged, this, destroy);
    configuration->resetSnapshot(editableSnapshot());
    QVERIFY(!configuration);
    QVERIFY(!global->controlsEnabled());
    QVERIFY(!volume->status()->controlsEnabled());
    QVERIFY(volume->files()->readOnly());
  }
  void commitContextCannotRevisitReplacedVolumes() {
    presentation::ConfigurationViewModel configuration(16384);
    const core::VolumeInfo physical{"C:", fixtureVolumeCName, "NTFS", "System", 1000, 500, core::VolumeSupport::Supported};
    const auto first = configuration.addVolume(physical, true);
    auto other = physical;
    other.driveLetter = "D:";
    other.volumeName = fixtureVolumeDName;
    const auto second = configuration.addVolume(other, false);
    configuration.resetSnapshot(editableSnapshot());
    std::shared_ptr<presentation::VolumeConfigurationViewModel> replacement;
    bool replacing = false;
    connect(first.get(), &presentation::VolumeConfigurationViewModel::stateChanged, this, [&] {
      if (std::exchange(replacing, true)) return;
      configuration.clearVolumes();
      replacement = configuration.addVolume(other, false);
    });
    configuration.setCommitContext(false, QStringLiteral("execution active"));
    QVERIFY(replacement);
    QVERIFY(!first->status()->controlsEnabled());
    QVERIFY(!second->status()->controlsEnabled());
    QVERIFY(!replacement->snapshotState().available);
    QVERIFY(!replacement->commitAvailability().files);
    QCOMPARE(configuration.volume(QStringLiteral("D:")), replacement);
  }

  void mainConfigurationUsesTheInjectedLiteralFileMetadata_data() {
    QTest::addColumn<QString>("suffix");
    QTest::addColumn<bool>("directory");
    const QStringList suffixes{QString{}, QStringLiteral(" "), QStringLiteral(".")};
    for (qsizetype index = 0; index < suffixes.size(); ++index)
      for (const bool directory : {false, true})
        QTest::newRow(qPrintable(QStringLiteral("suffix-%1-directory-%2").arg(index).arg(directory))) << suffixes.at(index) << directory;
  }
  void mainConfigurationUsesTheInjectedLiteralFileMetadata() {
    QFETCH(QString, suffix);
    QFETCH(bool, directory);
    MutableApplicationStateSource source;
    source.snapshot = editableSnapshot();
    source.volumes = {{"C:", uwf::test::application::fixtureVolumeCName, "NTFS", "System", 1000, 500, core::VolumeSupport::Supported}};
    const auto base = QStringLiteral("C:\\Data\\Entry.bin");
    const auto target = base + suffix;
    source.snapshot.current.fileExclusions.clear();
    source.snapshot.next.fileExclusions = {{uwf::test::application::fixtureVolumeCName, {target.mid(2).toStdString()}}};
    const auto files = std::make_shared<ReadOnlyFileBrowser>();
    files->entries[base] = {.path = base, .name = {}, .directory = !directory};
    files->entries[target] = {.path = target, .name = {}, .directory = directory};
    presentation::MainWindowViewModel model(source, app::UwfCapability::Available, {}, {}, files);
    model.initializeConfiguration(16384, QStringLiteral("C:"));
    for (int round = 0; round < 2; ++round) {
      if (round != 0) files->entries[target].directory = !directory;
      model.refresh();
      QTRY_VERIFY(!model.refreshing());
      // The committed state queues decorative metadata separately. Finish that
      // observation before checking it or mutating this read-only fixture.
      QCoreApplication::processEvents();
      QVERIFY(QThreadPool::globalInstance()->waitForDone(2000));
      QCoreApplication::processEvents();
      const auto entries = model.configuration()->volume(QStringLiteral("C:"))->files();
      QCOMPARE(entries->rowCount(), 1);
      const auto index = entries->index(0);
      QCOMPARE(index.data(presentation::PathRole).toString(), target);
      const bool observedDirectory = round == 0 ? directory : !directory;
      QCOMPARE(index.data(presentation::IconResourceRole).toString(),
               observedDirectory ? QStringLiteral(":/icons/folder.svg") : QStringLiteral(":/icons/file.svg"));
      entries->setCommitAvailability(true);
      QTRY_VERIFY(entries->fileCommitState(target).enabled);
      QCOMPARE(entries->fileCommitState(target).kind,
               observedDirectory ? app::ExclusionEditor::FileEntryKind::Directory : app::ExclusionEditor::FileEntryKind::File);
    }
    QVERIFY(!files->inspections.isEmpty());
    for (const auto& path : std::as_const(files->inspections)) QCOMPARE(path, target);
  }
  void overlayFlagControlsAndReviewUseCurrentSessionEligibility_data() {
    QTest::addColumn<int>("currentState");
    QTest::addColumn<bool>("persistent");
    for (const int state : {-1, 0, 1})
      for (const bool persistent : {false, true})
        QTest::newRow(qUtf8Printable(QStringLiteral("current-%1-%2").arg(state).arg(persistent ? "persistent" : "passthrough"))) << state << persistent;
  }
  void overlayFlagControlsAndReviewUseCurrentSessionEligibility() {
    QFETCH(int, currentState);
    QFETCH(bool, persistent);
    auto snapshot = editableSnapshot();
    snapshot.current.overlay.type = snapshot.next.overlay.type = core::OverlayType::Disk;
    snapshot.current.filter.enabled = currentState > 0;
    snapshot.next.filter.enabled = false;
    auto policy = diskPersistentPolicy();
    policy.current.filterEnabled = currentState < 0 ? app::PolicyObservation<bool>{app::PolicyObservationStatus::Failed}
                                                    : app::PolicyObservation<bool>{app::PolicyObservationStatus::Available, currentState > 0};
    policy.next.filterEnabled.value = false;
    auto editor = std::make_shared<app::OverlayPolicyEditor>();
    editor->reset(policy, snapshot);
    presentation::OverlayAdvancedViewModel controls(editor);
    const auto field = persistent ? app::OverlayPolicyField::Persistent : app::OverlayPolicyField::Passthrough;
    auto edit = controls.beginEdit();
    const QString reason = currentState < 0   ? QStringLiteral("The overlay configuration could not be read. Refresh before editing.")
                           : currentState > 0 ? QStringLiteral("UWF is enabled in the current session. Disable UWF and restart before changing this setting.")
                                              : QString{};
    QCOMPARE(controls.disabledReason(edit, field), reason);
    app::OverlayPolicyEditor::setValue(edit, field, false);
    app::ConfigurationChanges changes;
    changes.settings.setFilterEnabled = false;
    app::OverlayPolicyEditor::setValue(changes.overlayPolicy, field, false);
    auto actions = std::make_shared<ReviewActions>();
    presentation::ApplyPlanViewModel review(app::reviewApplicationConfiguration(changes, snapshot, policy), actions);
    const bool allowed = currentState == 0;
    QCOMPARE(controls.conflicts(edit).isEmpty(), allowed);
    QCOMPARE(review.canApply(), allowed);
    if (!allowed) {
      const auto conflict =
          currentState > 0
              ? QStringLiteral(
                    "UWF is enabled in the current session, so these overlay settings cannot be changed. Revert the changes, or disable UWF and restart.")
              : reason;
      QCOMPARE(controls.conflicts(edit), QStringList{conflict});
      QCOMPARE(review.warning(), conflict);
      QVERIFY(review.exportCommands().empty());
      review.apply();
      QCOMPARE(actions->preparations, 0);
    }
  }
  void reviewDisplaysTheApplicationResetRules_data() {
    QTest::addColumn<bool>("cancelClear");
    QTest::addColumn<bool>("currentFilter");
    for (const bool cancel : {false, true})
      for (const bool current : {false, true})
        QTest::newRow(qUtf8Printable(QStringLiteral("cancel-%1-current-%2").arg(cancel).arg(current))) << cancel << current;
  }
  void reviewDisplaysTheApplicationResetRules() {
    QFETCH(bool, cancelClear);
    QFETCH(bool, currentFilter);
    auto snapshot = editableSnapshot();
    snapshot.current.filter.enabled = currentFilter;
    snapshot.current.overlay.type = snapshot.next.overlay.type = core::OverlayType::Disk;
    auto policy = diskPersistentPolicy();
    policy.current.filterEnabled.value = currentFilter;
    policy.resetPersistentState.value = 1;
    app::ConfigurationChanges changes;
    changes.overlayPolicy = {.persistent = false, .reset = cancelClear ? app::PersistentReset::Preserve : app::PersistentReset::Clear};
    const auto plan = app::reviewApplicationConfiguration(changes, snapshot, policy);
    auto actions = std::make_shared<ReviewActions>();
    presentation::ApplyPlanViewModel review(plan, actions);
    QCOMPARE(review.canApply(), app::validateOverlayPolicyPlan(plan).valid());
    // Cancellation needs Current ON; changing Persistent needs Current OFF.
    // Retaining Clear instead conflicts with removing the next-session Persistent bit.
    QVERIFY(!review.canApply());
    QVERIFY(!review.warning().isEmpty());
    QVERIFY(review.exportCommands().empty());
    review.apply();
    QCOMPARE(actions->preparations, 0);
  }
  void stagingChangesReconcileExclusionsBeforePublishingPending() {
    MutableApplicationStateSource source;
    source.snapshot = editableSnapshot();
    const auto root = QString::fromLatin1(app::kFileStagingRegistryRoot.data(), app::kFileStagingRegistryRoot.size());
    source.snapshot.current.registryExclusions = {root.toStdString()};
    source.snapshot.next.registryExclusions = {root.toStdString()};
    source.volumes = {{"C:", uwf::test::application::fixtureVolumeCName, "NTFS", "System", 1000, 500, core::VolumeSupport::Supported}};
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
            [&] { rootRemovalAtPublication.append(registry->draft().removed().contains(root)); });
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
    source.volumes = {{"C:", uwf::test::application::fixtureVolumeCName, "NTFS", "System", 1000, 500, core::VolumeSupport::Supported}};
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
    QTest::addColumn<int>("stagingObservation");
    QTest::addColumn<bool>("reconciliation");
    for (const int observation : {0, 1, 2})
      for (const bool reconcile : {false, true})
        QTest::newRow(qUtf8Printable(QStringLiteral("staging-observation-%1-reconciliation-%2").arg(observation).arg(reconcile))) << observation << reconcile;
  }
  void discardIsAtomicAndPreservesTheReconciliationGate() {
    QFETCH(int, stagingObservation);
    QFETCH(bool, reconciliation);
    MutableApplicationStateSource source;
    source.snapshot = editableSnapshot();
    source.volumes = {{"C:", uwf::test::application::fixtureVolumeCName, "NTFS", "System", 1000, 500, core::VolumeSupport::Supported}};
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
    if (stagingObservation != 0) store.readFailure = QStringLiteral("storage unavailable");
    if (stagingObservation == 2) QVERIFY_THROWS_EXCEPTION(std::runtime_error, draft->reconcile());
    const auto reads = store.loads;
    QList<std::size_t> observed;
    connect(&model, &presentation::MainWindowViewModel::pendingChanged, &model, [&] { observed.append(model.pendingChangeCount()); });
    if (stagingObservation == 2) {
      QVERIFY_THROWS_EXCEPTION(std::runtime_error, model.discardPendingChanges());
      QCOMPARE(model.pendingChangeCount(), before);
      QVERIFY(observed.isEmpty());
      QVERIFY(draft->changes()->after.isEmpty());
      QVERIFY(!draft->storageAvailable());
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
    QCOMPARE(store.loads, reads);
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
