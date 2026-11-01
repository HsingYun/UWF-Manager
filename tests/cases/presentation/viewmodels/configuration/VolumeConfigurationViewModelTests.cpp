// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QSignalSpy>
#include <QTest>
#include <algorithm>
#include <array>
#include <atomic>
#include <cstddef>
#include <limits>
#include <map>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <thread>
#include <vector>

#include "application/configuration/commands/UwfmgrCommandCodec.h"
#include "application/configuration/execution/ConfigurationExecutor.h"
#include "application/uwf/adapters/UwfConfigurationRepository.h"
#include "application/uwf/adapters/UwfReadState.h"
#include "cases/presentation/viewmodels/common/ViewModelStateFixture.h"
#include "presentation/viewmodels/configuration/ConfigurationViewModel.h"
#include "presentation/viewmodels/configuration/VolumeConfigurationViewModel.h"
#include "support/app/ConfigurationFixture.h"
#include "support/app/ReadOnlyFileBrowserFixture.h"
#include "support/uwf/FakeUwfBackend.h"

namespace {
using namespace uwf;
using namespace uwf::test::viewmodels;

class VolumeConfigurationViewModelTests final : public QObject {
  Q_OBJECT
 private slots:
  void volumeEditsTrackOnlyChangesFromTheBaseline();
  void volumeBaselinesKeepCurrentOnlyAndMissingSemantics();
  void unletteredVolumesUseIdentityForProtectionAndKeepPathsReadOnly();
  void identicalSessionPatternsStayWithTheirOwnVolumeDespiteSavedDriveAliases();
  void unletteredProtectionAppliesByIdentityAndClearsOnlyAfterReadback();
  void unletteredProtectionRequiresValidIdentityAndRuntimeAdmission();
  void removedNextVolumeIsRecreatedThroughTheConfigurationBoundary_data();
  void removedNextVolumeIsRecreatedThroughTheConfigurationBoundary();
  void volumeCommitAvailability_data();
  void volumeCommitAvailability();
  void manualCommitRequiresConfirmedRuntimeCapability_data();
  void manualCommitRequiresConfirmedRuntimeCapability();
  void registrationNoticeFollowsPhysicalAdmission_data();
  void registrationNoticeFollowsPhysicalAdmission();
};

void VolumeConfigurationViewModelTests::identicalSessionPatternsStayWithTheirOwnVolumeDespiteSavedDriveAliases() {
  const std::string firstId = "Volume{aaaaaaaa-1111-2222-3333-444444444444}";
  const std::string secondId = "Volume{bbbbbbbb-1111-2222-3333-444444444444}";
  presentation::ConfigurationViewModel configuration(8192);
  const auto a = configuration.addVolume({.driveLetter = "C:", .volumeName = firstId, .fileSystem = "NTFS", .label = {}}, false);
  const auto b = configuration.addVolume({.driveLetter = "D:", .volumeName = secondId, .fileSystem = "NTFS", .label = {}}, false);
  auto state = snapshot();
  state.current.volumes = {{firstId, "C:", true, core::VolumeBinding::VolumeId, std::string("C:")}};
  // The saved C: belongs to B's registration metadata. Its actual mount is D:.
  state.next.volumes = {{secondId, "C:", true, core::VolumeBinding::DriveLetter, std::string("D:")}};
  state.current.fileExclusions = {{firstId, {"\\Shared"}}};
  state.next.fileExclusions = {{secondId, {"\\Shared"}}};
  configuration.resetSnapshot(state, {}, false);
  QCOMPARE(a->files()->draft().current(), QStringList{QStringLiteral("C:\\Shared")});
  QVERIFY(a->files()->draft().next().isEmpty());
  QVERIFY(b->files()->draft().current().isEmpty());
  QCOMPARE(b->files()->draft().next(), QStringList{QStringLiteral("D:\\Shared")});
  QCOMPARE(a->files()->rowCount(), 1);
  QCOMPARE(b->files()->rowCount(), 1);
  QCOMPARE(a->files()->index(0).data(presentation::BadgeRole).toInt(), static_cast<int>(presentation::ItemBadge::Remove));
  QCOMPARE(b->files()->index(0).data(presentation::BadgeRole).toInt(), static_cast<int>(presentation::ItemBadge::Add));
  const auto guidB = QString::fromStdString(compat::volumePath(secondId)) + QStringLiteral("Shared");
  QVERIFY(!a->files()->draft().equal(QStringLiteral("C:\\Shared"), guidB));
  QVERIFY(!b->files()->draft().equal(QStringLiteral("C:\\Shared"), guidB));
  QVERIFY(b->files()->draft().equal(QStringLiteral("D:\\Shared"), guidB));
  QVERIFY(b->editor()->files()->removeSelection({{guidB, std::nullopt}}));
  const auto pending = configuration.pendingChanges();
  QCOMPARE(pending.removeFileExclusions.size(), std::size_t{1});
  QCOMPARE(pending.removeFileExclusions.at(compat::normalizeVolumeId(secondId)), std::vector<std::string>{"D:\\Shared"});
}

void VolumeConfigurationViewModelTests::volumeEditsTrackOnlyChangesFromTheBaseline() {
  presentation::VolumeStatusViewModel model;
  const core::VolumeRecord current{"Volume{A}", "C:", true, uwf::core::VolumeBinding::DriveLetter};
  const core::VolumeRecord next{"Volume{A}", "C:", false, uwf::core::VolumeBinding::VolumeId};
  model.reset(&current, &next);

  QVERIFY(!model.pendingProtection());
  QVERIFY(!model.pendingBinding());
  QVERIFY(!model.editProtection(false));
  QVERIFY(model.editProtection(true));
  QCOMPARE(model.pendingProtection(), std::optional<bool>(true));
  QVERIFY(model.editProtection(false));
  QVERIFY(!model.pendingProtection());

  QVERIFY(!model.editBinding(uwf::core::VolumeBinding::VolumeId));
  QVERIFY(model.editBinding(uwf::core::VolumeBinding::DriveLetter));
  QCOMPARE(model.pendingBinding(), std::optional<uwf::core::VolumeBinding>(uwf::core::VolumeBinding::DriveLetter));

  model.reset(nullptr, nullptr);
  QVERIFY(!model.editProtection(true));
  QVERIFY(!model.pendingProtection());
  QVERIFY(!model.pendingBinding());
}

void VolumeConfigurationViewModelTests::volumeBaselinesKeepCurrentOnlyAndMissingSemantics() {
  presentation::VolumeStatusViewModel model;
  QSignalSpy availability(&model, &presentation::VolumeStatusViewModel::availabilityChanged);
  const core::VolumeRecord current{uwf::test::application::fixtureVolumeCName, "C:", true, uwf::core::VolumeBinding::VolumeId};
  model.reset(&current, nullptr);
  QVERIFY(model.currentProtected());
  QVERIFY(!model.draft().protectedVolume);
  QVERIFY(!model.draft().hasConfiguration);
  QCOMPARE(model.draft().binding, core::VolumeBinding::DriveLetter);
  QVERIFY(!model.bindingEnabled());
  QVERIFY(!model.editBinding(uwf::core::VolumeBinding::VolumeId));
  QVERIFY(!model.editProtection(false));
  QVERIFY(!model.pendingProtection());
  QVERIFY(!model.pendingBinding());
  // Current membership alone does not establish permission to register again.
  QVERIFY(!model.editProtection(true));
  QVERIFY(!model.pendingProtection());
  QVERIFY(!model.bindingEnabled());
  model.reset(nullptr, nullptr);
  QVERIFY(!model.pendingProtection());
  QVERIFY(!model.editProtection(true));
  QVERIFY(!model.draft().protectedVolume);
  model.setUnsupported();
  QCOMPARE(availability.count(), 3);
  QVERIFY(!model.controlsEnabled());
  model.setUnsupported();
  QCOMPARE(availability.count(), 3);
  QVERIFY(!model.editBinding(uwf::core::VolumeBinding::VolumeId));
}

void VolumeConfigurationViewModelTests::unletteredVolumesUseIdentityForProtectionAndKeepPathsReadOnly() {
  constexpr auto firstId = "Volume{AAAAAAAA-1111-2222-3333-444444444444}";
  constexpr auto secondId = "Volume{BBBBBBBB-1111-2222-3333-444444444444}";
  core::VolumeInfo first{.driveLetter = {}, .volumeName = firstId, .fileSystem = "NTFS", .label = {}};
  core::VolumeInfo second{.driveLetter = {}, .volumeName = "\\\\?\\Volume{bbbbbbbb-1111-2222-3333-444444444444}\\", .fileSystem = "NTFS", .label = {}};
  auto browser = std::make_shared<test::application::ReadOnlyFileBrowser>();
  presentation::ConfigurationViewModel configuration(8192, {}, {}, {}, browser);
  const auto a = configuration.addVolume(first, false);
  const auto b = configuration.addVolume(second, true);
  const auto addressed = configuration.addVolume(disk(), false);
  auto state = snapshot();
  state.current.volumes = {{secondId, "", false, core::VolumeBinding::VolumeId}, {firstId, "", true, core::VolumeBinding::VolumeId}};
  state.next.volumes = {{firstId, "", false, core::VolumeBinding::VolumeId}, {secondId, "", true, core::VolumeBinding::VolumeId}};
  state.current.fileExclusions[firstId] = {"\\First\\current.txt"};
  state.next.fileExclusions[firstId] = {"\\First\\next.txt"};
  state.current.fileExclusions[secondId] = {"\\Second\\current.txt"};
  state.next.fileExclusions[secondId] = {"\\Second\\next.txt"};
  configuration.resetSnapshot(state, {}, false);
  QVERIFY(a->status()->currentProtected());
  QVERIFY(!a->status()->draft().protectedVolume);
  QVERIFY(!b->status()->currentProtected());
  QVERIFY(b->status()->draft().protectedVolume);
  QCOMPARE(a->snapshotState().current->volumeName, std::string(firstId));
  QCOMPARE(b->snapshotState().current->volumeName, std::string(secondId));
  QVERIFY(a->volumeIdentity() != b->volumeIdentity());
  QCOMPARE(configuration.volume(a->volumeIdentity()), a);
  QCOMPARE(configuration.volume(b->volumeIdentity()), b);
  QVERIFY(!configuration.volume(QString{}));
  app::UwfmgrCommand emptyTarget;
  emptyTarget.kind = app::UwfmgrKind::VolumeProtect;
  emptyTarget.args = {""};
  (void)configuration.importCommands({emptyTarget});
  QVERIFY(configuration.pendingChanges().empty());
  const auto addressedPattern = [](const std::string& volume, const QString& pattern) { return app::paths::volumeFilePath({volume, pattern}); };
  QCOMPARE(a->files()->draft().current(), QStringList{addressedPattern(firstId, QStringLiteral("\\First\\current.txt"))});
  QCOMPARE(a->files()->draft().next(), QStringList{addressedPattern(firstId, QStringLiteral("\\First\\next.txt"))});
  QCOMPARE(b->files()->draft().current(), QStringList{addressedPattern(secondId, QStringLiteral("\\Second\\current.txt"))});
  QCOMPARE(b->files()->draft().next(), QStringList{addressedPattern(secondId, QStringLiteral("\\Second\\next.txt"))});
  QVERIFY(addressed->files()->canBrowseFiles());
  QVERIFY(addressed->editor()->files()->fileEntryKindOperation());
  for (const auto& model : {a, b}) {
    QVERIFY(!model->registry());
    QVERIFY(model->snapshotState().editable);
    QVERIFY(model->editor()->files()->readOnly());
    QCOMPARE(model->files()->rowCount(), 2);
    QVERIFY(!model->files()->canBrowseFiles());
    QVERIFY(!model->files()->canAdd());
    QVERIFY(!model->editor()->files()->fileEntryKindOperation());
    QVERIFY(!model->files()->removeSelection({{model->files()->draft().next().front(), {}}}));
    QVERIFY(model->status()->controlsEnabled());
    QVERIFY(!model->status()->bindingEnabled());
    QCOMPARE(model->status()->draft().binding, core::VolumeBinding::VolumeId);
    QVERIFY(!model->status()->editBinding(core::VolumeBinding::DriveLetter));
    QVERIFY(!model->status()->pendingBinding());
    const bool baseline = model->status()->draft().protectedVolume;
    const bool current = model->status()->currentProtected();
    QVERIFY(model->importProtect(!baseline));
    const auto pending = configuration.pendingChanges();
    QCOMPARE(pending.volumeProtect.size(), std::size_t{1});
    QCOMPARE(pending.volumeProtect.at(model->volumeIdentity().toStdString()), !baseline);
    QVERIFY(!pending.volumeProtect.contains(std::string{}));
    QVERIFY(pending.volumeBindings.empty());
    QVERIFY(pending.addFileExclusions.empty());
    QVERIFY(pending.removeFileExclusions.empty());
    QCOMPARE(model->status()->currentProtected(), current);
    QVERIFY(model->importProtect(baseline));
    QVERIFY(configuration.pendingChanges().empty());
    QCOMPARE(model->importAddFileExclusion(QStringLiteral("C:\\hidden.txt")), app::ExclusionImportOutcome::RejectedReadOnly);
    QVERIFY(!model->commitAvailability().files);
    QVERIFY(!model->commitAvailability().registry);
    QVERIFY(!model->commitAvailability().overlay);
  }
  QVERIFY(browser->inspections.isEmpty());
  QVERIFY(configuration.pendingChanges().empty());
  // A missing Current record must not borrow another unlettered partition.
  state.current.volumes.erase(state.current.volumes.begin());
  configuration.resetSnapshot(state, {}, false);
  QVERIFY(!b->snapshotState().current);
  QVERIFY(b->status()->draft().protectedVolume);
  QVERIFY(a->status()->currentProtected());
}

void VolumeConfigurationViewModelTests::unletteredProtectionAppliesByIdentityAndClearsOnlyAfterReadback() {
  constexpr auto id = "aaaaaaaa-1111-2222-3333-444444444444";
  const auto system = std::make_shared<test::FakeUwfBackend>();
  (void)compat::call(*system, compat::AddVolume{id, {}});
  (void)compat::call(*system, compat::SetVolume{id, compat::VolumeProperty::Protected, true});
  system->simulateRestart();
  const auto before = system->snapshot();
  const auto observed = app::publishedUwfSnapshot(system->state(), app::UwfCapability::Available, true, {});
  presentation::ConfigurationViewModel configuration(8192);
  const auto volume = configuration.addVolume({.driveLetter = {}, .volumeName = compat::volumePath(id), .fileSystem = {}, .label = {}}, false);
  configuration.resetSnapshot(observed, {}, false);
  QVERIFY(volume->status()->controlsEnabled());
  QVERIFY(volume->status()->currentProtected());
  QVERIFY(volume->status()->draft().protectedVolume);
  system->clearTrace();
  QVERIFY(volume->importProtect(false));
  const auto changes = configuration.pendingChanges();
  QCOMPARE(changes.volumeProtect, (std::map<std::string, bool>{{id, false}}));
  QVERIFY(system->trace().empty());
  QVERIFY(system->snapshot() == before);
  const auto result = app::ConfigurationExecutor(app::uwfConfigurationRepository(system)).apply(changes, observed);
  QCOMPARE(result.failedOperations(), 0);
  QCOMPARE(result.succeededOperations(), 1);
  QVERIFY(result.anyWriteConfirmed());
  // Executing the immutable plan does not pretend the editor has read it.
  QCOMPARE(volume->status()->pendingProtection(), std::optional<bool>{false});
  QVERIFY(system->snapshot().current == before.current);
  const auto next = compat::call(*system, compat::GetVolumes{compat::Session::Next});
  QCOMPARE(next.size(), std::size_t{1});
  QCOMPARE(next.front().id, std::string(id));
  QVERIFY(!next.front().protectedVolume);
  QVERIFY(!next.front().bindByDriveLetter);
  QVERIFY(next.front().driveLetter.empty());
  configuration.resetSnapshot(app::publishedUwfSnapshot(system->state(), app::UwfCapability::Available, true, {}), {}, false);
  QVERIFY(configuration.pendingChanges().empty());
  QVERIFY(volume->status()->currentProtected());
  QVERIFY(!volume->status()->draft().protectedVolume);
}

void VolumeConfigurationViewModelTests::unletteredProtectionRequiresValidIdentityAndRuntimeAdmission() {
  for (const bool available : {false, true})
    for (const bool elevated : {false, true})
      for (const bool horm : {false, true}) {
        presentation::ConfigurationViewModel configuration(8192);
        const auto volume = configuration.addVolume(
            {.driveLetter = {}, .volumeName = "Volume{aaaaaaaa-1111-2222-3333-444444444444}", .fileSystem = {}, .label = {}, .physical = {true, true}}, false);
        auto state = snapshot();
        state.uwfAvailable = available;
        state.elevated = elevated;
        configuration.resetSnapshot(state, {}, horm);
        const bool allowed = available && elevated && !horm;
        QCOMPARE(volume->status()->controlsEnabled(), allowed);
        QCOMPARE(volume->importProtect(true), allowed);
        QCOMPARE(configuration.pendingChanges().volumeProtect.size(), allowed ? std::size_t{1} : std::size_t{0});
        QVERIFY(!configuration.pendingChanges().volumeProtect.contains(std::string{}));
      }
  for (const auto& id : {std::string{}, std::string{"not-a-volume-guid"}}) {
    presentation::ConfigurationViewModel configuration(8192);
    const auto volume = configuration.addVolume({.driveLetter = {}, .volumeName = id, .fileSystem = {}, .label = {}}, false);
    configuration.resetSnapshot(snapshot(), {}, false);
    volume->status()->setControlsEnabled(true);
    QVERIFY(!volume->status()->controlsEnabled());
    QVERIFY(!volume->importProtect(true));
    QVERIFY(configuration.pendingChanges().empty());
  }
}

void VolumeConfigurationViewModelTests::removedNextVolumeIsRecreatedThroughTheConfigurationBoundary_data() {
  QTest::addColumn<bool>("currentFilter");
  QTest::addColumn<bool>("currentProtected");
  QTest::addColumn<bool>("currentBoundByDrive");
  QTest::addColumn<bool>("importCommand");
  QTest::addColumn<bool>("protect");
  for (const bool currentFilter : {false, true})
    for (const bool currentProtected : {false, true})
      for (const bool currentBoundByDrive : {false, true})
        for (const bool importCommand : {false, true})
          for (const bool protect : {false, true}) {
            const auto row = QStringLiteral("filter-%1-current-%2-binding-%3-%4-%5")
                                 .arg(currentFilter ? "on" : "off", currentProtected ? "protected" : "unprotected", currentBoundByDrive ? "drive" : "identity",
                                      importCommand ? "import" : "edit", protect ? "protect" : "unprotect");
            QTest::newRow(qUtf8Printable(row)) << currentFilter << currentProtected << currentBoundByDrive << importCommand << protect;
          }
}

void VolumeConfigurationViewModelTests::removedNextVolumeIsRecreatedThroughTheConfigurationBoundary() {
  QFETCH(bool, currentFilter);
  QFETCH(bool, currentProtected);
  QFETCH(bool, currentBoundByDrive);
  QFETCH(bool, importCommand);
  QFETCH(bool, protect);
  constexpr auto volumeId = "11111111-2222-3333-4444-555555555555";
  test::FakeUwfBackend::State initial;
  initial.runtime.fileMaintenancePrefixes[volumeId] = "C:";
  const auto system = std::make_shared<test::FakeUwfBackend>(initial);
  // Reach Current-only membership through public operations, rather than
  // seeding an arbitrary difference between the two configuration sessions.
  (void)compat::call(*system, compat::AddVolume{volumeId, "C:"});
  (void)compat::call(*system, compat::SetVolume{volumeId, compat::VolumeProperty::Protected, currentProtected});
  (void)compat::call(*system, compat::SetVolume{volumeId, compat::VolumeProperty::BindByDriveLetter, currentBoundByDrive});
  (void)compat::call(*system, compat::SetBoolean{compat::BooleanProperty::FilterEnabled, currentFilter});
  system->simulateRestart();
  const auto current = system->snapshot().current;
  (void)compat::call(*system, compat::RemoveVolume{volumeId});
  const auto before = system->snapshot();
  QVERIFY(before.current == current);
  QVERIFY(before.next.volumes.empty());
  system->clearTrace();

  const auto baseline = app::publishedUwfSnapshot(system->state(), app::UwfCapability::Available, true, {});
  QCOMPARE(baseline.current.volumes.size(), std::size_t{1});
  QVERIFY(baseline.next.volumes.empty());
  presentation::ConfigurationViewModel configuration(0);
  auto physical = disk();
  physical.volumeName = "Volume{" + std::string(volumeId) + '}';
  const auto volume = configuration.addVolume(physical, false);
  configuration.resetSnapshot(baseline);
  const auto status = volume->status();
  QCOMPARE(status->currentProtected(), currentProtected);
  QVERIFY(status->controlsEnabled());
  QVERIFY(!status->draft().hasConfiguration);
  QVERIFY(!status->draft().protectedVolume);
  QCOMPARE(status->draft().binding, core::VolumeBinding::DriveLetter);
  QVERIFY(!status->bindingEnabled());
  QVERIFY(!status->editBinding(uwf::core::VolumeBinding::VolumeId));
  QVERIFY(configuration.pendingChanges().empty());

  if (importCommand) {
    QList<app::UwfmgrCommand> commands;
    for (const auto& command : app::parseUwfmgrText(protect ? "uwfmgr volume protect C:" : "uwfmgr volume unprotect C:")) commands.append(command);
    const auto report = configuration.importCommands(commands);
    QCOMPARE(report.size(), 1);
    QCOMPARE(report.front().status, protect ? app::ImportResult::Status::Success : app::ImportResult::Status::Duplicate);
  } else {
    QCOMPARE(status->editProtection(protect), protect);
  }
  QCOMPARE(status->bindingEnabled(), protect);
  QVERIFY(!status->pendingBinding());
  QVERIFY(system->snapshot() == before);
  const auto changes = configuration.pendingChanges();
  QCOMPARE(changes.volumeProtect.size(), protect ? std::size_t{1} : std::size_t{0});
  if (protect) QCOMPARE(changes.volumeProtect.at("C:"), true);
  QVERIFY(changes.volumeBindings.empty());
  const auto result = app::ConfigurationExecutor(app::uwfConfigurationRepository(system)).apply(changes, baseline);
  QCOMPARE(result.failedOperations(), 0);
  QCOMPARE(result.anyWriteConfirmed(), protect);
  QCOMPARE(result.succeededOperations(), protect ? 1 : 0);

  auto expected = before;
  if (protect)
    expected.next.volumes.emplace(
        volumeId, test::FakeUwfBackend::VolumeConfiguration{.volume = {volumeId, "C:", true, true, std::nullopt}, .fileExclusions = {}, .swapfileSize = 0});
  QVERIFY(system->snapshot() == expected);
  if (protect) {
    const auto next = compat::call(*system, compat::GetVolumes{compat::Session::Next});
    QCOMPARE(next.size(), std::size_t{1});
    QCOMPARE(next.front().id, std::string(volumeId));
    QCOMPARE(next.front().driveLetter, std::string("C:"));
    QVERIFY(next.front().protectedVolume);
    QVERIFY(next.front().bindByDriveLetter);
    QVERIFY(!next.front().commitPending.has_value());
    QVERIFY(compat::call(*system, compat::GetFileExclusions{volumeId, compat::Session::Next}).empty());
  }
  const auto trace = system->trace();
  QCOMPARE(std::ranges::any_of(trace, [](const auto& entry) { return compat::isWrite(entry.request); }), protect);
  configuration.resetSnapshot(app::publishedUwfSnapshot(system->state(), app::UwfCapability::Available, true, {}));
  QCOMPARE(status->currentProtected(), currentProtected);
  QCOMPARE(status->draft().hasConfiguration, protect);
  QCOMPARE(status->draft().protectedVolume, protect);
  QCOMPARE(status->draft().binding, core::VolumeBinding::DriveLetter);
  QVERIFY(configuration.pendingChanges().empty());
}

void VolumeConfigurationViewModelTests::volumeCommitAvailability_data() {
  using Support = core::VolumeSupport;
  QTest::addColumn<Support>("support");
  QTest::addColumn<QString>("fileSystem");
  QTest::addColumn<bool>("fileOperations");
  QTest::addColumn<bool>("protection");
  QTest::addColumn<bool>("registryHost");
  QTest::addColumn<bool>("filter");
  QTest::addColumn<bool>("elevated");
  QTest::addColumn<bool>("protectedVolume");
  struct FileSystem {
    Support support;
    const char* name;
    bool files, protectable;
  };
  constexpr std::array systems{FileSystem{Support::Supported, "NTFS", true, true}, FileSystem{Support::Supported, "FAT32", true, true},
                               FileSystem{Support::FileSystemLimited, "exFAT", false, true}, FileSystem{Support::NotFixedLocalDisk, "NTFS", false, false},
                               FileSystem{Support::ExceedsMaxSize, "NTFS", false, false}};
  for (const auto& system : systems)
    for (const bool registryHost : {false, true})
      for (const bool filter : {false, true})
        for (const bool elevated : {false, true})
          for (const bool protectedVolume : {false, true}) {
            const auto name = QStringLiteral("%1-%2-host%3-filter%4-elevated%5-protected%6")
                                  .arg(system.name)
                                  .arg(static_cast<int>(system.support))
                                  .arg(registryHost)
                                  .arg(filter)
                                  .arg(elevated)
                                  .arg(protectedVolume);
            QTest::newRow(qUtf8Printable(name)) << system.support << QString::fromLatin1(system.name) << system.files << system.protectable << registryHost
                                                << filter << elevated << protectedVolume;
          }
}

void VolumeConfigurationViewModelTests::volumeCommitAvailability() {
  QFETCH(core::VolumeSupport, support);
  QFETCH(QString, fileSystem);
  QFETCH(bool, fileOperations);
  QFETCH(bool, protection);
  QFETCH(bool, registryHost);
  QFETCH(bool, filter);
  QFETCH(bool, elevated);
  QFETCH(bool, protectedVolume);
  auto volume = disk();
  volume.support = support;
  volume.fileSystem = fileSystem.toStdString();
  volume.freeBytes = 1ULL << 30;
  volume.totalBytes = 2ULL << 30;
  presentation::VolumeConfigurationViewModel model(volume, registryHost);
  const auto initial = model.commitAvailability();
  QVERIFY(!initial.files && !initial.registry && !initial.overlay);
  auto current = snapshot();
  current.current.filter.enabled = filter;
  current.current.volumes[0].isProtected = protectedVolume;
  current.runtime.systemVolumeProtected = protectedVolume;
  current.elevated = elevated;
  model.applySnapshot(current);
  const auto available = model.commitAvailability();
  QCOMPARE(available.files, fileOperations && filter && elevated && protectedVolume);
  QCOMPARE(available.registry, registryHost && protection && filter && elevated && protectedVolume);
  QCOMPARE(available.menu, available.files || available.registry);
  QCOMPARE(available.overlay, protection && fileSystem == QStringLiteral("NTFS") && filter && elevated && protectedVolume);
  QVERIFY(!available.overlayHint.isEmpty());
  QVERIFY(!available.commitHint.isEmpty());
  QCOMPARE(model.capacityText(), fileSystem + QStringLiteral(" · 1 GB free / 2 GB"));
  QCOMPARE(presentation::volumeSupportText(support, volume.fileSystem).isEmpty(), support == core::VolumeSupport::Supported);
}

void VolumeConfigurationViewModelTests::manualCommitRequiresConfirmedRuntimeCapability_data() {
  QTest::addColumn<int>("mode");
  QTest::addColumn<bool>("allowed");
  QTest::newRow("normal") << 0 << true;
  QTest::newRow("unknown-horm") << 1 << false;
  QTest::newRow("unknown-flags") << 2 << false;
  QTest::newRow("unknown-rom-disposition") << 3 << false;
  QTest::newRow("read-only-rom") << 4 << false;
  QTest::newRow("horm-active") << 5 << false;
  QTest::newRow("filter-transition") << 6 << true;
  QTest::newRow("horm-disabled-transition") << 7 << true;
  QTest::newRow("full-commit") << 8 << false;
}

void VolumeConfigurationViewModelTests::manualCommitRequiresConfirmedRuntimeCapability() {
  QFETCH(int, mode);
  QFETCH(bool, allowed);
  presentation::VolumeConfigurationViewModel model(disk(), true);
  auto state = snapshot();
  state.current.filter.enabled = true;
  state.current.volumes[0].isProtected = true;
  state.runtime.systemVolumeProtected = true;
  state.runtime.hormEnabled = mode == 1 ? std::nullopt : std::optional{mode == 5};
  state.runtime.currentOverlayFlags = mode == 2 ? std::nullopt : std::optional<std::uint32_t>{mode >= 3 ? 4u : 0u};
  state.runtime.romDisposition = mode == 3 ? std::nullopt : std::optional<std::uint32_t>{mode == 6 ? 1u : mode == 7 ? 3u : mode == 8 ? 4u : 0u};
  state.runtime.fullCommit = mode == 8;
  model.applySnapshot(state);
  const auto available = model.commitAvailability();
  QCOMPARE(available.files, allowed);
  QCOMPARE(available.registry, allowed);
  QVERIFY(available.overlay);
  if (mode >= 1 && mode <= 3)
    QCOMPARE(available.commitHint, QStringLiteral("The commit or HORM state is unknown. Refresh before continuing."));
  if (mode == 7) {
    // Automation pauses through the first restart; manual maintenance uses the
    // effective current capability instead of inheriting that pause.
    QVERIFY(model.status()->controlsEnabled());
  }
}

void VolumeConfigurationViewModelTests::registrationNoticeFollowsPhysicalAdmission_data() {
  QTest::addColumn<int>("physicalState");
  QTest::addColumn<bool>("registered");
  QTest::addColumn<bool>("limitedFileSystem");
  for (int state = 0; state < 4; ++state)
    for (const bool registered : {false, true})
      for (const bool limited : {false, true})
        QTest::newRow(qUtf8Printable(QStringLiteral("physical-%1/registered-%2/limited-%3").arg(state).arg(registered).arg(limited)))
            << state << registered << limited;
}
void VolumeConfigurationViewModelTests::registrationNoticeFollowsPhysicalAdmission() {
  QFETCH(int, physicalState);
  QFETCH(bool, registered);
  QFETCH(bool, limitedFileSystem);
  const core::PhysicalVolumeFacts facts[] = {{}, {false, {}}, {true, false}, {true, true}};
  auto physical = disk();
  physical.physical = facts[physicalState];
  if (limitedFileSystem) {
    physical.fileSystem = "exFAT";
    physical.support = core::VolumeSupport::FileSystemLimited;
  }
  presentation::VolumeConfigurationViewModel model(physical, false);
  auto state = snapshot();
  state.current.volumes.clear();
  state.next.volumes.clear();
  state.runtime.systemVolumeProtected = false;
  if (registered) state.next.volumes.push_back({physical.volumeName, physical.driveLetter, true, core::VolumeBinding::DriveLetter});
  model.applySnapshot(state, false);
  const bool eligible = physicalState == 3;
  QCOMPARE(model.status()->controlsEnabled(), registered || eligible);
  QCOMPARE(model.runtimeNotice().isEmpty(), registered || eligible);
  if (!registered && physicalState == 2) QVERIFY(model.runtimeNotice().contains(QStringLiteral("storage bus")));
  if (!registered && physicalState == 0) QVERIFY(model.runtimeNotice().contains(QStringLiteral("could not be confirmed")));
  const auto capability = presentation::volumeSupportText(physical.support, physical.fileSystem);
  QCOMPARE(capability.isEmpty(), !limitedFileSystem);
  QVERIFY(!capability.contains(QStringLiteral("can be protected")));
  if (limitedFileSystem) {
    QVERIFY(capability.contains(QStringLiteral("exFAT")));
    QVERIFY(capability.contains(QStringLiteral("file exclusions and per-file commit are not supported")));
  }
}

}  // namespace

QTEST_GUILESS_MAIN(VolumeConfigurationViewModelTests)
#include "VolumeConfigurationViewModelTests.moc"
