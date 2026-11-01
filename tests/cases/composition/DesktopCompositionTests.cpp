// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QScopeGuard>
#include <QSignalSpy>
#include <QTest>
#include <QThreadPool>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <future>
#include <stdexcept>

#include "application/settings/UserConfig.h"
#include "application/uwf/UwfRequestPrecheck.h"
#include "application/uwf/UwfStateUpdates.h"
#include "application/uwf/adapters/UwfApplicationStateSource.h"
#include "application/uwf/adapters/UwfOverlayOperations.h"
#include "application/uwf/adapters/UwfReadState.h"
#include "composition/desktop/MainWindowComposition.h"
#include "composition/desktop/ViewModelComposition.h"
#include "composition/desktop/WorkflowComposition.h"
#include "presentation/models/ItemRoles.h"
#include "presentation/viewmodels/shell/MainWindowViewModel.h"
#include "presentation/views/shell/WindowWorkflowFactory.h"
#include "support/app/FileStagingFixture.h"
#include "support/app/FileTargetFixture.h"
#include "support/app/ReadOnlyFileBrowserFixture.h"
#include "support/app/UwfMaintenanceFixture.h"
#include "support/app/WorkerGate.h"
#include "support/ui/DesktopPlatformFixture.h"
#include "support/ui/FileDialogStub.h"
#include "support/uwf/FakeUwfBackend.h"

namespace {
using namespace uwf;

class RecordingVolumes final : public app::VolumeRepository {
 public:
  std::vector<core::VolumeInfo> read() const override {
    ++reads;
    return entries;
  }
  std::string driveForPath(const std::string&) const override {
    ++resolutions;
    return "Z:";
  }
  std::vector<std::string> mountedLetters(const std::string&) const override { return {"Z:"}; }
  std::vector<std::string> mountedPaths(const std::string&) const override { return {"Z:\\"}; }
  std::string systemDrive() const override {
    ++systemDrives;
    return "Z:";
  }
  mutable int reads = 0;
  mutable int resolutions = 0;
  mutable int systemDrives = 0;
  std::vector<core::VolumeInfo> entries;
};
class MemoryRegistry final : public app::RegistryBrowser {
 public:
  std::vector<std::string> rootHiveLongNames(std::stop_token = {}) const override { return {"HKEY_LOCAL_MACHINE"}; }
  std::vector<std::string> subkeyNames(std::string_view, std::stop_token = {}) const override { return {"Injected"}; }
  bool hasSubkeys(std::string_view, std::stop_token = {}) const override { return true; }
  std::vector<app::RegistryValueInfo> values(std::string_view, std::stop_token = {}) const override { return {}; }
};
class MemorySystemInformation final : public app::SystemInformationSource {
 public:
  uint32_t totalRamMb() const override { return 777; }
  QString systemDriveLetter() const override { return QStringLiteral("Z:"); }
  const app::WindowsVersionInfo& windowsVersion() const override { return version; }
  const app::SystemHardwareInfo& hardware() const override { return facts; }
  app::SystemDiagnosticFacts diagnostics() const override { return {}; }

 private:
  app::WindowsVersionInfo version;
  app::SystemHardwareInfo facts;
};

class DesktopCompositionTests final : public QObject {
  Q_OBJECT
 private slots:
  void configuredPortsReachImportAndExportWithoutFactoryLookup() {
    auto ports = test::localDesktopPorts();
    const auto volumes = std::make_shared<RecordingVolumes>();
    ports.volumes = volumes;
    ports.applicationDirectory = QStringLiteral("Z:/Injected/Application");
    QString readPath;
    ports.readText = [&](const QString& path, qint64 maximum) {
      readPath = path;
      if (maximum <= 0) throw std::runtime_error("a bounded read is required");
      return app::TextFileReadResult{app::TextFileReadResult::Failure::None, {}, "uwfmgr filter enable\n"};
    };
    QString createdPath;
    QString createdText;
    ports.createText = [&](const QString& path, const QString& text) {
      createdPath = path;
      createdText = text;
      return app::TextFileCreationResult{};
    };
    const auto factory = composition::createViewModelFactory(std::make_shared<test::FakeUwfBackend>(), ports);
    const auto importer = factory->importCommands();
    QCOMPARE(volumes->systemDrives, 1);
    QSignalSpy text(importer.get(), &presentation::ImportViewModel::textBlockReady);
    importer->loadFile(QStringLiteral("Z:/Injected/commands.txt"));
    QCOMPARE(readPath, QStringLiteral("Z:/Injected/commands.txt"));
    QCOMPARE(text.count(), 1);
    QVERIFY(importer->defaultRulesText(presentation::ImportViewModel::Defender).contains(QStringLiteral("Z:")));
    const auto exported = factory->userConfigExport()->exportConfig({});
    QCOMPARE(exported.outcome, app::TextFileCreationResult::Outcome::Created);
    QCOMPARE(createdPath, app::userConfigPath(ports.applicationDirectory));
    QCOMPARE(createdText, app::serializeUserConfig({}));
    QCOMPARE(volumes->reads, 0);
  }
  void childModelsOwnPortsAfterFactoryRelease() {
    auto ports = test::localDesktopPorts();
    auto registry = std::make_shared<MemoryRegistry>();
    auto information = std::make_shared<MemorySystemInformation>();
    const std::weak_ptr<const app::RegistryBrowser> registryLifetime = registry;
    const std::weak_ptr<const app::SystemInformationSource> informationLifetime = information;
    ports.registry = std::move(registry);
    ports.systemInformation = std::move(information);
    auto factory = composition::createViewModelFactory(std::make_shared<test::FakeUwfBackend>(), ports);
    auto picker = factory->registryPicker(presentation::RegistryPickerViewModel::Mode::CommitValue);
    auto system = factory->systemInformation();
    factory.reset();
    ports = {};
    QVERIFY(!registryLifetime.expired());
    QVERIFY(!informationLifetime.expired());
    QTRY_COMPARE(picker->rootHives(), std::vector<std::string>{"HKEY_LOCAL_MACHINE"});
    QCOMPARE(system->totalRamMb(), std::uint32_t{777});
    picker.reset();
    system.reset();
    QTRY_VERIFY(registryLifetime.expired());
    QVERIFY(informationLifetime.expired());
  }
  void incompletePortsFailBeforeCreatingAnyModel() {
    QVERIFY_THROWS_EXCEPTION(std::invalid_argument, static_cast<void>(composition::createViewModelFactory(std::make_shared<test::FakeUwfBackend>(), {})));
  }
  void recommendedImportPreservesAnExistingSyntheticInventory() {
    class StateSource final : public app::ApplicationStateSource {
     public:
      app::ApplicationState state;
      ReadOperation readOperation(app::UwfCapability) override {
        return [copy = state](std::stop_token) { return copy; };
      }
    } source;
    const std::string volumeId = "11111111-2222-3333-4444-555555555555";
    constexpr int existingCount = 48;
    test::FakeUwfBackend::State initial;
    initial.next.filterEnabled = true;
    initial.next.volumes[volumeId].volume = {volumeId, "Z:", true, false, {}};
    QStringList existing;
    for (int index = 0; index < existingCount; ++index) {
      const auto relative = QStringLiteral("\\Synthetic\\Application %1\\Profile Data\\Cache").arg(index);
      initial.next.volumes[volumeId].fileExclusions.push_back(relative.toStdString());
      existing.append(QStringLiteral("Z:") + relative);
    }
    initial.current = initial.next;
    initial.runtime.systemVolumeId = volumeId;
    const auto backend = std::make_shared<test::FakeUwfBackend>(std::move(initial));
    auto ports = test::localDesktopPorts();
    const auto volumes = std::make_shared<RecordingVolumes>();
    volumes->entries = {{"Z:", compat::volumePath(volumeId), "NTFS", "Synthetic", 1000, 500, core::VolumeSupport::Supported}};
    ports.volumes = volumes;
    ports.files = std::make_shared<test::application::ReadOnlyFileBrowser>();
    ports.maintenanceObservations = {};
    const auto facts = backend->state();
    source.state = {volumes->entries, app::publishedUwfSnapshot(facts, app::UwfCapability::Available, true, {}), app::UwfOverlayOperations::snapshot(facts),
                    false};
    const auto store = std::make_shared<test::application::MemoryFileStagingStore>();
    const auto services = composition::mainWindowServices(backend, source, store, ports, nullptr);
    const auto window = services.workflows->mainWindow(services.viewModelFactory, app::UwfCapability::Available, nullptr);
    window->initializeConfiguration(16384, QStringLiteral("Z:"));
    window->refresh();
    QTRY_VERIFY(!window->refreshing());
    const auto configuration = window->configuration();
    const auto volume = configuration->volume(QStringLiteral("Z:"));
    QVERIFY(volume);
    QCOMPARE(volume->files()->rowCount(), existingCount);
    const auto importer = window->importModel();
    QVERIFY(importer);
    QTRY_VERIFY(!importer->preparing());
    importer->setText(importer->defaultRulesText(presentation::ImportViewModel::AllRecommendedSections));
    QSignalSpy fileResets(volume->files().get(), &QAbstractItemModel::modelReset);
    QSignalSpy registryResets(volume->registry().get(), &QAbstractItemModel::modelReset);
    QSignalSpy pending(window.get(), &presentation::MainWindowViewModel::pendingChanged);
    QBENCHMARK_ONCE { importer->importText(); }
    QCOMPARE(importer->rowCount(), 31);
    QCOMPARE(importer->totals().success, 29);
    QCOMPARE(importer->totals().duplicate, 2);
    QCOMPARE(importer->totals().failed, 0);
    QCOMPARE(importer->totals().unsupported, 0);
    QCOMPARE(fileResets.size(), 1);
    QCOMPARE(registryResets.size(), 1);
    QCOMPARE(pending.size(), 3);
    QCOMPARE(volume->files()->draft().current().size(), existingCount);
    QCOMPARE(volume->files()->draft().next().size(), existingCount);
    QCOMPARE(volume->files()->rowCount(), existingCount + 9);
    for (const auto& path : existing) {
      bool retained = false;
      for (int row = 0; row < volume->files()->rowCount(); ++row) {
        const auto index = volume->files()->index(row);
        if (index.data(presentation::PathRole).toString() != path) continue;
        QCOMPARE(index.data(presentation::StatusRole).toInt(), static_cast<int>(presentation::ItemStatus::Normal));
        retained = true;
        break;
      }
      QVERIFY(retained);
    }
    const auto changes = configuration->pendingChanges();
    QCOMPARE(changes.addFileExclusions.size(), std::size_t{1});
    QCOMPARE(changes.addFileExclusions.begin()->second.size(), std::size_t{9});
    QCOMPARE(changes.addRegistryExclusions.size(), std::size_t{20});
    QVERIFY(changes.removeFileExclusions.empty());
    QVERIFY(changes.removeRegistryExclusions.empty());
    importer->importText();
    QCOMPARE(importer->totals().success, 29);
    QCOMPARE(importer->totals().duplicate, 33);
    QCOMPARE(fileResets.size(), 1);
    QCOMPARE(registryResets.size(), 1);
    QVERIFY(std::ranges::none_of(backend->trace(), [](const auto& entry) { return compat::isWrite(entry.request); }));
    QCOMPARE(store->writes, 0);
  }
  void observedHostFactsReachMaintenanceApply_data() {
    QTest::addColumn<bool>("enableHorm");
    QTest::newRow("storage-location") << false;
    QTest::newRow("enable-horm") << true;
  }
  void observedHostFactsReachMaintenanceApply() {
    QFETCH(bool, enableHorm);
    const std::string volumeId = "11111111-2222-3333-4444-555555555555";
    test::FakeUwfBackend::State initial;
    initial.next.filterEnabled = enableHorm;
    initial.next.overlayFlags = enableHorm ? app::OverlayFlags::ReadOnlyMedia : 0;
    initial.next.volumes[volumeId].volume = {volumeId, "Z:", enableHorm, false, {}};
    initial.current = initial.next;
    initial.runtime.systemVolumeId = volumeId;
    const auto backend = std::make_shared<test::FakeUwfBackend>(std::move(initial));
    auto ports = test::localDesktopPorts();
    const auto volumes = std::make_shared<RecordingVolumes>();
    volumes->entries = {
        {"Z:", compat::volumePath(volumeId), "NTFS", "Data", 100ULL * 1024 * 1024 * 1024, 80ULL * 1024 * 1024 * 1024, core::VolumeSupport::Supported}};
    ports.volumes = volumes;
    std::atomic_int hibernationReads{0};
    ports.maintenanceObservations.hibernationCapabilities = [&](std::stop_token) {
      ++hibernationReads;
      return app::OverlayHibernationCapabilities{true, true, app::HibernationFileType::Full};
    };
    ports.maintenanceObservations.hormVolumeProtection = [](std::stop_token) { return app::OverlayHormVolumeProtection{true, {}, {}}; };
    auto factory = composition::createViewModelFactory(backend, ports);
    auto maintenance = factory->overlayMaintenance();
    maintenance->setInteractionContext(true, false);
    maintenance->refresh();
    QTRY_VERIFY(maintenance->hasSnapshot() && !maintenance->busy());
    QVERIFY(maintenance->snapshot().volumesReadable);
    QCOMPARE(maintenance->snapshot().hibernationAvailable, std::optional{true});
    QVERIFY(enableHorm ? maintenance->stageHorm(true) : maintenance->stageStorageLocation(QString::fromStdString(volumeId)));
    app::ConfigurationChanges changes;
    changes.maintenance = maintenance->draft();
    const auto facts = backend->state();
    const auto review = factory->applyPlan(changes, app::publishedUwfSnapshot(facts, app::UwfCapability::Available, true, {}),
                                           app::UwfOverlayOperations::snapshot(facts), {}, maintenance->snapshot());
    QVERIFY(review->canApply());
    const auto volumeReads = volumes->reads;
    factory.reset();
    maintenance.reset();
    review->apply();
    QTRY_VERIFY(!review->applying());
    QVERIFY2(!review->resultFailed(), qPrintable(review->resultText()));
    QVERIFY(review->result().anyWriteConfirmed());
    QCOMPARE(volumes->reads, volumeReads);
    QCOMPARE(hibernationReads.load(), 1);
    const auto applied = backend->snapshot();
    if (enableHorm) {
      QVERIFY(applied.runtime.hormEnabled);
    } else {
      QCOMPARE(applied.next.overlayType, static_cast<std::uint32_t>(compat::OverlayType::Disk));
      QCOMPARE(applied.next.volumes.at(volumeId).swapfileSize, std::uint32_t{1});
    }
    const auto trace = backend->trace();
    QCOMPARE(std::ranges::count_if(trace,
                                   [enableHorm](const auto& entry) {
                                     if (!enableHorm) return std::holds_alternative<compat::CreateSwapfile>(entry.request);
                                     const auto* request = std::get_if<compat::SetBoolean>(&entry.request);
                                     return request && request->property == compat::BooleanProperty::HormEnabled && request->value;
                                   }),
             1);
  }
  void bulkProtectionUsesTheInjectedSystemForObservationAndApply_data() {
    QTest::addColumn<bool>("enabled");
    QTest::newRow("protect") << true;
    QTest::newRow("unprotect") << false;
  }
  void bulkProtectionUsesTheInjectedSystemForObservationAndApply() {
    QFETCH(bool, enabled);
    const auto volumeId = "11111111-2222-3333-4444-555555555555";
    const auto makeBackend = [enabled, volumeId] {
      test::FakeUwfBackend::State initial;
      initial.next.volumes[volumeId].volume = {volumeId, "C:", !enabled, false, {}};
      initial.current = initial.next;
      auto backend = std::make_shared<test::FakeUwfBackend>(std::move(initial));
      auto facts = backend->state();
      facts.runtime.protectScope = compat::Fact<std::vector<std::string>>::known({volumeId});
      backend->updateState(std::move(facts));
      return backend;
    };
    const auto firstBackend = makeBackend();
    const auto secondBackend = makeBackend();
    auto ports = test::localDesktopPorts();
    ports.volumes = std::make_shared<RecordingVolumes>();
    ports.maintenanceObservations = {};
    auto firstFactory = composition::createViewModelFactory(firstBackend, ports);
    auto secondFactory = composition::createViewModelFactory(secondBackend, ports);
    const auto firstMaintenance = firstFactory->overlayMaintenance();
    const auto secondMaintenance = secondFactory->overlayMaintenance();
    for (const auto& maintenance : {firstMaintenance, secondMaintenance}) {
      maintenance->setInteractionContext(true, false);
      maintenance->refresh();
      QTRY_VERIFY(maintenance->hasSnapshot() && !maintenance->busy());
      QCOMPARE(maintenance->snapshot().protectTargetVolumes, (std::vector<QString>{QString::fromLatin1(volumeId)}));
      QVERIFY(maintenance->stageAllVolumesProtection(enabled));
    }
    const auto review = [](const auto& factory, const auto& backend, const auto& maintenance) {
      const auto facts = backend->state();
      app::ConfigurationChanges changes;
      changes.maintenance = maintenance->draft();
      return factory->applyPlan(changes, app::publishedUwfSnapshot(facts, app::UwfCapability::Available, true, {}),
                                app::UwfOverlayOperations::snapshot(facts), {}, maintenance->snapshot());
    };
    const auto firstReview = review(firstFactory, firstBackend, firstMaintenance);
    const auto secondReview = review(secondFactory, secondBackend, secondMaintenance);
    QVERIFY(firstReview->canApply());
    QVERIFY(secondReview->canApply());
    firstFactory.reset();
    secondFactory.reset();
    firstReview->apply();
    QTRY_VERIFY(!firstReview->applying());
    QVERIFY(!firstReview->resultFailed());
    QVERIFY(firstReview->result().anyWriteConfirmed());
    QCOMPARE(firstBackend->snapshot().next.volumes.at(volumeId).volume.protectedVolume, enabled);
    QCOMPARE(secondBackend->snapshot().next.volumes.at(volumeId).volume.protectedVolume, !enabled);
    QVERIFY(std::ranges::none_of(secondBackend->trace(), [](const auto& entry) { return compat::isWrite(entry.request); }));
    secondReview->apply();
    QTRY_VERIFY(!secondReview->applying());
    QVERIFY(!secondReview->resultFailed());
    QVERIFY(secondReview->result().anyWriteConfirmed());
    QCOMPARE(secondBackend->snapshot().next.volumes.at(volumeId).volume.protectedVolume, enabled);
  }
  void overlayWindowsShareQueriesByGuidAndKeepTheirOwnDisplayRoots() {
    const auto gate = std::make_shared<test::WorkerGate>();
    std::atomic_int reads{0};
    QString observedGuid;
    const auto release = qScopeGuard([&] {
      gate->release();
      QThreadPool::globalInstance()->waitForDone(2000);
    });
    const auto queries = std::make_shared<app::OverlayFilesQueryManager>([&](const QString& guid, const std::stop_token stop) {
      observedGuid = guid;
      ++reads;
      gate->wait(stop);
      return QVector<app::OverlayFileEntry>{{QStringLiteral("\\Data\\shared.bin"), {}, false, false, 8}};
    });
    const auto backend = std::make_shared<test::FakeUwfBackend>();
    const auto firstFactory = composition::createViewModelFactory(backend, test::localDesktopPorts(), {}, queries);
    const auto secondFactory = composition::createViewModelFactory(backend, test::localDesktopPorts(), {}, queries);
    const auto volume = [](const std::string& drive) {
      return std::make_shared<presentation::VolumeConfigurationViewModel>(
          core::VolumeInfo{drive, "Volume{11111111-2222-3333-4444-555555555555}", "NTFS", "Data", 1000, 500, core::VolumeSupport::Supported}, false);
    };
    auto first = firstFactory->overlayFiles(volume("C:"));
    const auto second = secondFactory->overlayFiles(volume("D:"));
    app::paths::VolumeIdentities mounts{{QStringLiteral("C:"), "11111111-2222-3333-4444-555555555555"},
                                        {QStringLiteral("D:"), "11111111-2222-3333-4444-555555555555"}};
    first->startLoading();
    second->startLoading();
    QTRY_COMPARE(gate->started.load(), 1);
    QCOMPARE(observedGuid, QStringLiteral("11111111-2222-3333-4444-555555555555"));
    first->cancelLoading();
    first.reset();
    const auto reopened = firstFactory->overlayFiles(volume("C:"));
    reopened->startLoading();
    QCOMPARE(reads.load(), 1);
    QCOMPARE(gate->canceled.load(), 0);
    // C: is now another volume while both windows retain the query's original GUID.
    mounts.insert_or_assign(QStringLiteral("C:"), "66666666-7777-8888-9999-aaaaaaaaaaaa");
    gate->release();
    QTRY_VERIFY(second->loaded() && reopened->loaded());
    QCOMPARE(reads.load(), 1);
    QCOMPARE(second->entries().constFirst().absolutePath, QStringLiteral("D:\\Data\\shared.bin"));
    QCOMPARE(reopened->entries().constFirst().absolutePath, QStringLiteral("C:\\Data\\shared.bin"));
    QSignalSpy firstCommit(reopened.get(), &presentation::OverlayFilesViewModel::commitFileRequested);
    QSignalSpy secondCommit(second.get(), &presentation::OverlayFilesViewModel::commitFileRequested);
    reopened->setCommitAvailability(true);
    second->setCommitAvailability(true);
    reopened->requestCommit(QStringLiteral("C:\\Data\\shared.bin"));
    second->requestCommit(QStringLiteral("D:\\Data\\shared.bin"));
    QCOMPARE(firstCommit.size(), 1);
    QCOMPARE(secondCommit.size(), 1);
    const auto expected = QStringLiteral("\\\\?\\Volume{11111111-2222-3333-4444-555555555555}\\Data\\shared.bin");
    QCOMPARE(firstCommit.constFirst().constFirst().toString(), expected);
    QCOMPARE(secondCommit.constFirst().constFirst().toString(), expected);
    const auto dispatched = app::paths::resolveVolumeFilePath(firstCommit.constFirst().constFirst().toString(), mounts);
    QVERIFY(dispatched);
    QCOMPARE(dispatched->volume, std::string("11111111-2222-3333-4444-555555555555"));
  }
  void overlayQueriesNeverReplaceAnUnknownGuidWithTheDisplayDrive() {
    std::atomic_int reads{0};
    const auto factory =
        composition::createViewModelFactory(std::make_shared<test::FakeUwfBackend>(), test::localDesktopPorts(), [&](const QString&, std::stop_token) {
          ++reads;
          return QVector<app::OverlayFileEntry>{};
        });
    const auto volume = std::make_shared<presentation::VolumeConfigurationViewModel>(
        core::VolumeInfo{"C:", {}, "NTFS", "Data", 1000, 500, core::VolumeSupport::Supported}, false);
    const auto model = factory->overlayFiles(volume);
    QSignalSpy failed(model.get(), &presentation::OverlayFilesViewModel::loadFailed);
    model->startLoading();
    QTRY_COMPARE(failed.size(), 1);
    QCOMPARE(reads.load(), 0);
    QCOMPARE(model->loadState(), presentation::ContentLoadState::Failed);
  }
  void workflowFactoriesRetainTheirExplicitDependencies() {
    class EmptyStagingStore final : public app::FileStagingStore {
     public:
      QList<app::FileStagingEntry> load() const override { return {}; }
      void replace(const QList<app::FileStagingEntry>&) override {}
    } store;
    test::application::UwfMaintenanceFixture fixture;
    auto traversal = std::make_shared<test::application::StoredPathSource>();
    const std::weak_ptr<const app::CommitTraversalSource> traversalLifetime = traversal;
    auto power = composition::createPowerCoordinator({fixture.repository(), store, app::UwfCapability::Available, traversal}, nullptr);
    traversal.reset();
    QVERIFY(!traversalLifetime.expired());
    power.reset();
    QVERIFY(traversalLifetime.expired());

    traversal = std::make_shared<test::application::StoredPathSource>();
    const std::weak_ptr<const app::CommitTraversalSource> commitTraversalLifetime = traversal;
    auto dialogs = std::make_shared<test::FileDialogStub>();
    const std::weak_ptr<ui::dialogs::FileDialogProvider> dialogLifetime = dialogs;
    auto writerLease = std::make_shared<int>(0);
    const std::weak_ptr<int> writerLifetime = writerLease;
    auto dispatcher = composition::createCommitDispatcher(
        fixture.repository(), {}, traversal,
        [lease = std::move(writerLease)](const QString&, const QString&) {
          ++*lease;
          return app::TextFileWriteResult{};
        },
        dialogs, nullptr);
    traversal.reset();
    dialogs.reset();
    QVERIFY(!commitTraversalLifetime.expired());
    QVERIFY(!dialogLifetime.expired());
    QVERIFY(!writerLifetime.expired());
    dispatcher.reset();
    QVERIFY(commitTraversalLifetime.expired());
    QVERIFY(dialogLifetime.expired());
    QVERIFY(writerLifetime.expired());
  }
};
}  // namespace
QTEST_MAIN(DesktopCompositionTests)
#include "DesktopCompositionTests.moc"
