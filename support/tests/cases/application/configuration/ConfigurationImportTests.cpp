// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QSignalSpy>
#include <QTest>
#include <algorithm>
#include <cstdint>
#include <limits>
#include <map>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>

#include "application/configuration/commands/UwfmgrCommandCodec.h"
#include "application/configuration/contracts/ConfigurationPlan.h"
#include "application/configuration/editing/ConfigurationEditSession.h"
#include "application/configuration/editing/GlobalConfigurationEditor.h"
#include "application/configuration/importing/ConfigurationImporter.h"
#include "application/configuration/planning/ConfigurationScript.h"
#include "application/configuration/volumes/VolumeConfigurationEditor.h"

namespace {
using namespace uwf;

QList<app::UwfmgrCommand> commands(const std::string& text) {
  const auto parsed = app::parseUwfmgrText(text);
  return {parsed.begin(), parsed.end()};
}

std::string protectionCommand(const bool enabled, const std::string& identity) {
  app::UwfmgrCommand command;
  command.kind = enabled ? app::UwfmgrKind::VolumeProtect : app::UwfmgrKind::VolumeUnprotect;
  command.args = {compat::volumePath(identity)};
  return app::renderUwfmgrCommand(command);
}

app::DriveResolver noDriveResolution() {
  return [](const std::string&) -> std::string { throw std::logic_error("unexpected drive resolution"); };
}

core::SessionSnapshot baseline(uint32_t warning = 1000, uint32_t critical = 3000) {
  core::SessionSnapshot state;
  state.overlay = {.type = core::OverlayType::Disk, .maximumSizeMb = 8192, .warningThresholdMb = warning, .criticalThresholdMb = critical};
  return state;
}

struct OverlayImportFixture {
  core::UwfSnapshot snapshot;
  app::OverlayPolicySnapshot policy;
  app::ConfigurationEditSession session{16384};

  OverlayImportFixture(const core::OverlayType type, const uint32_t flags, const bool currentFilter = false) {
    snapshot.uwfAvailable = snapshot.elevated = true;
    snapshot.current = snapshot.next = baseline();
    snapshot.current.overlay.type = snapshot.next.overlay.type = type;
    snapshot.current.filter.enabled = currentFilter;
    snapshot.runtime.hormEnabled = false;
    snapshot.runtime.currentOverlayFlags = flags;
    snapshot.runtime.systemVolumeProtected = false;
    snapshot.runtime.romDisposition = 0;
    snapshot.runtime.fullCommit = false;
    policy.transportAvailable = true;
    policy.current.filterEnabled = {app::PolicyObservationStatus::Available, currentFilter};
    policy.current.overlayType = policy.next.overlayType = {app::PolicyObservationStatus::Available, static_cast<std::int64_t>(type)};
    policy.current.flags = policy.next.flags = {app::PolicyObservationStatus::Available, app::OverlayFlags{flags}};
    policy.resetPersistentState = {app::PolicyObservationStatus::Available, 0};
    session.resetSnapshot(snapshot, policy, false);
  }
};

struct MaintenanceImportFixture {
  static constexpr const char* storageId = "66666666-7777-8888-9999-aaaaaaaaaaaa";
  core::UwfSnapshot snapshot;
  app::OverlayPolicySnapshot policy;
  app::OverlayMaintenanceSnapshot observed;
  app::ConfigurationEditSession session{16384, QStringLiteral("C:")};
  explicit MaintenanceImportFixture(const int operation) {
    const bool activeHorm = operation == 0;
    const bool filter = operation != 3;
    const std::uint32_t flags = filter ? app::OverlayFlags::ReadOnlyMedia : 0u;
    snapshot.uwfAvailable = snapshot.elevated = true;
    snapshot.current = snapshot.next = baseline();
    snapshot.current.filter.enabled = snapshot.next.filter.enabled = filter;
    snapshot.current.overlay.type = snapshot.next.overlay.type = core::OverlayType::RAM;
    snapshot.runtime.hormEnabled = activeHorm;
    snapshot.runtime.currentOverlayFlags = flags;
    snapshot.runtime.systemVolumeProtected = true;
    snapshot.runtime.romDisposition = activeHorm ? 2u : 0u;
    snapshot.runtime.fullCommit = false;
    policy.transportAvailable = true;
    policy.current.filterEnabled = policy.next.filterEnabled = {app::PolicyObservationStatus::Available, filter};
    policy.current.overlayType = policy.next.overlayType = {app::PolicyObservationStatus::Available, 0};
    policy.current.flags = policy.next.flags = {app::PolicyObservationStatus::Available, app::OverlayFlags{flags}};
    policy.resetPersistentState = {app::PolicyObservationStatus::Available, 0};
    observed.filterEnabled = filter;
    observed.systemVolumeProtected = true;
    observed.hormEnabled = activeHorm;
    observed.overlayCommit = false;
    observed.currentOverlayType = observed.nextOverlayType = 0;
    observed.currentFlags = observed.nextFlags = app::OverlayFlags{flags};
    observed.maximumOverlaySize = snapshot.next.overlay.maximumSizeMb;
    observed.romDisposition = activeHorm ? 2u : 0u;
    observed.hibernationAvailable = observed.allVolumesProtected = true;
    observed.volumesReadable = true;
    observed.volumes = {{QString::fromLatin1(storageId), QStringLiteral("D:"), {}, 16ULL << 30, true, 0, 0}};
    observed.hormCapability.support = observed.fullCommitCapability.support = observed.swapCapability.support = app::OverlayMaintenanceSupport::Supported;
    session.resetSnapshot(snapshot, policy, activeHorm);
    session.maintenanceEditor()->setInteractionContext(true, false);
    session.maintenanceEditor()->reconcileAppliedDraft({}, observed);
  }
  app::ConfigurationPlan plan(const int operation) const {
    app::ConfigurationPlan result;
    result.snapshot = snapshot;
    result.policySnapshot = policy;
    result.maintenanceSnapshot = observed;
    if (operation < 2)
      result.maintenance.hormEnabled = operation == 1;
    else if (operation == 2)
      result.maintenance.fullCommit = true;
    else
      result.maintenance.storageVolume = QString::fromLatin1(storageId);
    return result;
  }
};

class MemoryVolumeEditor final : public app::VolumeConfigurationEditing {
 public:
  bool protectedVolume = false;
  QStringList added;
  [[nodiscard]] QString volumeIdentity() const override { return QStringLiteral("11111111-1111-1111-1111-111111111111"); }
  [[nodiscard]] QString configurationVolumeId() const override { return volumeIdentity(); }
  [[nodiscard]] QString driveLetter() const override { return QStringLiteral("Z:"); }
  [[nodiscard]] bool editingAllowed() const override { return true; }
  [[nodiscard]] bool protectionEditingAllowed() const override { return true; }
  bool importProtect(const bool value, const QString&) override {
    const bool changed = protectedVolume != value;
    protectedVolume = value;
    return changed;
  }
  app::ExclusionImportOutcome importAddFileExclusion(const QString& path) override {
    added.append(path);
    return app::ExclusionImportOutcome::Applied;
  }
  app::ExclusionImportOutcome importRemoveFileExclusion(const QString&) override { throw std::logic_error("unexpected exclusion removal"); }
};

class ConfigurationImportTests final : public QObject {
  Q_OBJECT

 private slots:
  void largeExclusionBatchKeepsCommandOrderAndStoredAliases() {
    constexpr int count = 1000;
    const std::string identity = "Volume{11111111-2222-3333-4444-555555555555}";
    app::ConfigurationEditSession session(16384, QStringLiteral("C:"));
    const auto volume = session.editVolume({"C:", identity, "NTFS", "Data", 1000, 500, core::VolumeSupport::Supported}, false);
    core::UwfSnapshot snapshot;
    snapshot.uwfAvailable = snapshot.elevated = true;
    snapshot.current = snapshot.next = baseline();
    snapshot.current.volumes.push_back({identity, "C:", true, core::VolumeBinding::DriveLetter, std::string{"C:"}});
    snapshot.next.volumes = snapshot.current.volumes;
    snapshot.runtime.hormEnabled = false;
    snapshot.runtime.currentOverlayFlags = 0;
    for (int index = 0; index < count; ++index) snapshot.next.fileExclusions[identity].push_back("\\Data\\saved-" + std::to_string(index) + ".bin");
    session.resetSnapshot(snapshot, {}, false);
    volume->fileConflicts().setStagedEntries({{app::FileStagingKind::File, QStringLiteral("C:\\Blocked\\state.bin")}});
    std::string text;
    for (int index = 0; index < count; ++index) text += "uwfmgr file add-exclusion C:\\Data\\new-" + std::to_string(index) + ".bin\n";
    const auto alias = compat::volumePath(identity);
    text += "uwfmgr file remove-exclusion C:\\Data\\new-0.bin\nuwfmgr file add-exclusion " + alias +
            "Data\\new-0.bin\nuwfmgr file add-exclusion c:\\data\\NEW-0.bin\nuwfmgr file remove-exclusion " + alias +
            "Data\\saved-0.bin\nuwfmgr file add-exclusion C:\\Data\\saved-0.bin\nuwfmgr file add-exclusion C:\\Blocked\n";
    const auto input = commands(text);
    QSignalSpy states(volume->files().get(), &app::ExclusionEditor::stateChanged);
    QSignalSpy pending(volume->files().get(), &app::ExclusionEditor::pendingChanged);
    QList<app::ImportResult> report;
    QBENCHMARK_ONCE { report = session.importCommands(input); }
    QCOMPARE(report.size(), count + 6);
    for (int index = 0; index < count + 6; ++index) {
      const auto expected = index == count + 2   ? app::ImportResult::Status::Duplicate
                            : index == count + 5 ? app::ImportResult::Status::Failed
                                                 : app::ImportResult::Status::Success;
      QCOMPARE(report[index].status, expected);
    }
    QCOMPARE(volume->files()->pendingAdded().size(), count);
    QVERIFY(volume->files()->pendingRemoved().isEmpty());
    QCOMPARE(volume->files()->draft().next().size(), count);
    QCOMPARE(volume->files()->effectiveFileExclusions().size(), 2 * count);
    QCOMPARE(states.size(), 1);
    QCOMPARE(pending.size(), 1);
  }
  void exportedMaintenanceCommandsRoundTripIntoDraftsWithoutExecution_data() {
    QTest::addColumn<int>("operation");
    QTest::newRow("disable-HORM-while-active") << 0;
    QTest::newRow("enable-HORM") << 1;
    QTest::newRow("explicit-FullCommit") << 2;
    QTest::newRow("native-storage-configuration") << 3;
  }
  void exportedMaintenanceCommandsRoundTripIntoDraftsWithoutExecution() {
    QFETCH(int, operation);
    MaintenanceImportFixture fixture(operation);
    const auto requested = fixture.plan(operation);
    const auto script = app::configurationChangeScript(requested);
    QVERIFY(script.exportSupported);
    QCOMPARE(script.commands.size(), std::size_t{1});
    auto decoded = app::parseUwfmgrText(app::renderConfigurationScript(script));
    for (const auto& command : decoded) QVERIFY(command.parseError == app::ParseError::None || command.parseError == app::ParseError::Comment);
    std::erase_if(decoded, [](const auto& command) { return command.parseError == app::ParseError::Comment; });
    QCOMPARE(decoded.size(), std::size_t{1});
    const QList<app::UwfmgrCommand> input(decoded.begin(), decoded.end());
    const auto imported = fixture.session.importCommands(input);
    QCOMPARE(imported.size(), 1);
    QCOMPARE(imported.front().status, app::ImportResult::Status::Success);
    const auto pending = fixture.session.pendingConfiguration();
    QCOMPARE(pending.maintenance, requested.maintenance);
    QVERIFY(pending.settings.empty());
    QVERIFY(pending.overlayPolicy.empty());
    const auto duplicate = fixture.session.importCommands(input);
    QCOMPARE(duplicate.front().status, app::ImportResult::Status::Duplicate);
    QCOMPARE(fixture.session.pendingConfiguration().maintenance, requested.maintenance);
    // Import only stages intent; it cannot acknowledge a runtime transition.
    QCOMPARE(fixture.session.maintenanceEditor()->snapshot().hormEnabled, fixture.observed.hormEnabled);
    QCOMPARE(fixture.session.maintenanceEditor()->snapshot().overlayCommit, std::optional{false});
    QCOMPARE(fixture.session.maintenanceEditor()->snapshot().volumes.front().nextMarker, std::optional<std::uint32_t>{0});
  }
  void maintenanceImportHonorsTheExternalSessionWriteGate_data() { exportedMaintenanceCommandsRoundTripIntoDraftsWithoutExecution_data(); }
  void maintenanceImportHonorsTheExternalSessionWriteGate() {
    QFETCH(int, operation);
    MaintenanceImportFixture fixture(operation);
    const auto script = app::configurationChangeScript(fixture.plan(operation));
    const auto input = commands(app::renderUwfmgrCommand(script.commands.front()));
    fixture.session.setEditingAllowed(false);
    const auto denied = fixture.session.importCommands(input);
    QCOMPARE(denied.front().status, app::ImportResult::Status::Failed);
    QVERIFY(fixture.session.pendingConfiguration().empty());
    fixture.session.setEditingAllowed(true);
    QCOMPARE(fixture.session.importCommands(input).constFirst().status, app::ImportResult::Status::Success);
  }
  void activeHormOnlyAdmitsItsOwnDisableIntentAndKeepsOtherCommandsRestricted() {
    MaintenanceImportFixture fixture(0);
    const auto input = commands(
        "uwfmgr filter disable-HORM\nuwfmgr filter disable\nuwfmgr overlay set-warningthreshold 1500\n"
        "uwfmgr registry add-exclusion HKEY_LOCAL_MACHINE\\SOFTWARE\\ShouldStayReadOnly\nuwfmgr overlay commit");
    const auto report = fixture.session.importCommands(input);
    QCOMPARE(report.size(), 5);
    QCOMPARE(report.front().status, app::ImportResult::Status::Success);
    for (qsizetype index = 1; index < report.size(); ++index) QCOMPARE(report[index].status, app::ImportResult::Status::Failed);
    const auto pending = fixture.session.pendingConfiguration();
    QCOMPARE(pending.maintenance.hormEnabled, std::optional{false});
    QVERIFY(!pending.maintenance.fullCommit);
    QVERIFY(pending.settings.empty());
    QVERIFY(pending.overlayPolicy.empty());
    QCOMPARE(fixture.session.maintenanceEditor()->snapshot().hormEnabled, std::optional{true});
  }
  void nativeMaintenanceImportDoesNotBroadenUnsupportedCliScopes() {
    MaintenanceImportFixture fixture(3);
    const auto report =
        fixture.session.importCommands(commands("uwfmgr volume unprotect all\nuwfmgr overlay commit-all\n"
                                                "uwfmgr filter disable-HORM extra\nuwfmgr overlay commit C:"));
    QCOMPARE(report.size(), 4);
    QCOMPARE(report[0].status, app::ImportResult::Status::Unsupported);
    QCOMPARE(report[1].status, app::ImportResult::Status::Unsupported);
    QCOMPARE(report[2].status, app::ImportResult::Status::Failed);
    QCOMPARE(report[3].status, app::ImportResult::Status::Failed);
    QVERIFY(fixture.session.pendingConfiguration().empty());
  }
  void matchingHormIsANoOpButObservedFullCommitCannotBeTriggeredAgain() {
    MaintenanceImportFixture active(0);
    const auto unchanged = active.session.importCommands(commands("uwfmgr filter enable-HORM"));
    QCOMPARE(unchanged.front().status, app::ImportResult::Status::Duplicate);
    QVERIFY(active.session.pendingConfiguration().empty());
    MaintenanceImportFixture committed(2);
    committed.observed.overlayCommit = true;
    committed.observed.romDisposition = 4;
    committed.session.maintenanceEditor()->reconcileAppliedDraft({}, committed.observed);
    const auto repeated = committed.session.importCommands(commands("uwfmgr overlay commit"));
    QCOMPARE(repeated.front().status, app::ImportResult::Status::Failed);
    QVERIFY(committed.session.pendingConfiguration().empty());
  }
  void overlayStorageAdmissionIsSharedByEditingImportAndPlanning() {
    app::ConfigurationEditSession session(16384);
    core::UwfSnapshot snapshot;
    snapshot.uwfAvailable = snapshot.elevated = true;
    snapshot.current = baseline();
    snapshot.current.filter.enabled = true;
    snapshot.next = snapshot.current;
    snapshot.runtime.hormEnabled = false;
    snapshot.runtime.currentOverlayFlags = 0;
    snapshot.runtime.systemVolumeProtected = false;
    snapshot.runtime.romDisposition = 0;
    snapshot.runtime.fullCommit = false;
    session.resetSnapshot(snapshot, {}, false);
    const auto editor = session.globalEditor();
    QVERIFY(editor->editingAllowed());
    QVERIFY(!editor->overlayStorageEditingAllowed());
    QVERIFY(!editor->overlayStorageRestriction().isEmpty());
    editor->editOverlayType(core::OverlayType::RAM);
    editor->editMaximumSize(16384);
    QVERIFY(editor->pendingOverlay().empty());
    QCOMPARE(editor->importOverlayType(core::OverlayType::Disk).outcome, app::GlobalEditOutcome::Unchanged);
    QCOMPARE(editor->importOverlayMaxMb(8192).outcome, app::GlobalEditOutcome::Unchanged);
    QCOMPARE(editor->importOverlayType(core::OverlayType::RAM).outcome, app::GlobalEditOutcome::Rejected);
    QCOMPARE(editor->importOverlayMaxMb(16384).outcome, app::GlobalEditOutcome::Rejected);
    const auto report =
        session.importCommands(commands("uwfmgr overlay set-size 16384 && uwfmgr filter disable\n"
                                        "uwfmgr overlay set-type RAM\nuwfmgr overlay set-size 8192\n"
                                        "uwfmgr overlay set-warningthreshold 500\nuwfmgr overlay set-criticalthreshold 2500\nuwfmgr filter disable"));
    QCOMPARE(report.size(), 7);
    for (int index : {0, 1, 2}) QCOMPARE(report[index].status, app::ImportResult::Status::Failed);
    QCOMPARE(report[0].detail, editor->overlayStorageRestriction());
    QCOMPARE(report[3].status, app::ImportResult::Status::Duplicate);
    for (int index : {4, 5, 6}) QCOMPARE(report[index].status, app::ImportResult::Status::Success);
    QVERIFY(!editor->overlayStorageEditingAllowed());  // Next disable does not change Current admission.
    const auto changes = session.pendingConfiguration();
    QVERIFY(!changes.settings.setOverlay.type);
    QVERIFY(!changes.settings.setOverlay.maximumSizeMb);
    QCOMPARE(changes.settings.setOverlay.warningThresholdMb, std::optional<uint32_t>{500});
    app::OverlayPolicySnapshot policy;
    policy.transportAvailable = true;
    const auto plan = app::planApplicationConfiguration(changes, snapshot, policy);
    QVERIFY(!app::configurationChangeScript(plan).commands.empty());
    QVERIFY(!app::configurationTargetScript(plan).commands.empty());
    snapshot.current.filter.enabled = false;
    snapshot.next.filter.enabled = false;
    session.resetSnapshot(snapshot, {}, false);
    QVERIFY(editor->overlayStorageEditingAllowed());
    QVERIFY(editor->overlayStorageRestriction().isEmpty());
    QCOMPARE(editor->importOverlayType(core::OverlayType::RAM).outcome, app::GlobalEditOutcome::Applied);
    QCOMPARE(editor->importOverlayMaxMb(4096).outcome, app::GlobalEditOutcome::Applied);
    session.setEditingAllowed(false);
    QVERIFY(!editor->overlayStorageEditingAllowed());
    QCOMPARE(editor->importOverlayMaxMb(8192).outcome, app::GlobalEditOutcome::Rejected);
  }
  void zeroCapacityImportFailsWithoutNormalizingObservedStateOrUnlockingDependents() {
    app::GlobalConfigurationEditor editor;
    auto state = baseline();
    state.overlay.type = core::OverlayType::RAM;
    state.overlay.maximumSizeMb = 0;  // Preserve an abnormal raw observation.
    editor.reset(state);
    const auto report = app::applyImportCommands(commands("uwfmgr overlay set-size 0 && uwfmgr filter enable\n"
                                                          "uwfmgr overlay set-warningthreshold 0\nuwfmgr overlay set-criticalthreshold 0"),
                                                 &editor, {}, nullptr, noDriveResolution());
    QCOMPARE(report.size(), 4);
    QCOMPARE(report.at(0).status, app::ImportResult::Status::Failed);
    QCOMPARE(report.at(1).status, app::ImportResult::Status::Failed);
    QVERIFY(!editor.draft().filterEnabled);
    QCOMPARE(editor.draft().overlay.maximumSizeMb, uint32_t{0});
    QVERIFY(!editor.pendingOverlay().maximumSizeMb);
    QCOMPARE(editor.draft().overlay.warningThresholdMb, uint32_t{0});
    QCOMPARE(editor.draft().overlay.criticalThresholdMb, uint32_t{0});
    const auto repaired = app::applyImportCommands(commands("uwfmgr overlay set-size 1"), &editor, {}, nullptr, noDriveResolution());
    QCOMPARE(repaired.front().status, app::ImportResult::Status::Success);
    QCOMPARE(editor.pendingOverlay().maximumSizeMb, std::optional<uint32_t>{1});
  }
  void protectAllImportsUseTheSharedDraftAndHonorReadOnlyAndDependencies() {
    app::ConfigurationEditSession session;
    session.globalEditor()->reset(baseline());
    const auto editor = session.maintenanceEditor();
    app::OverlayMaintenanceSnapshot observed;
    observed.filterEnabled = observed.hormEnabled = observed.overlayCommit = false;
    const auto ticket = editor->beginObservation();
    QVERIFY(ticket);
    editor->finishObservation(*ticket, observed);
    editor->setInteractionContext(true, false);
    const auto result =
        session.importCommands(commands("uwfmgr volume protect all && uwfmgr filter enable\nuwfmgr volume protect all\n"
                                        "uwfmgr volume unprotect all"));
    QCOMPARE(result.size(), 4);
    QCOMPARE(result[0].status, app::ImportResult::Status::Success);
    QCOMPARE(result[1].status, app::ImportResult::Status::Success);
    QCOMPARE(result[2].status, app::ImportResult::Status::Duplicate);
    QCOMPARE(result[3].status, app::ImportResult::Status::Unsupported);
    QCOMPARE(session.pendingConfiguration().maintenance.allVolumesProtected, std::optional<bool>{true});
    session.globalEditor()->setControlsEnabled(false);
    const auto readOnly = session.importCommands(commands("uwfmgr volume protect all"));
    QCOMPARE(readOnly[0].status, app::ImportResult::Status::Failed);
    QCOMPARE(session.pendingConfiguration().maintenance.allVolumesProtected, std::optional<bool>{true});
    session.globalEditor()->setControlsEnabled(true);
    editor->setInteractionContext(true, true);
    const auto rejected = session.importCommands(commands("uwfmgr volume protect all && uwfmgr filter disable"));
    QCOMPARE(rejected[0].status, app::ImportResult::Status::Failed);
    QCOMPARE(rejected[1].status, app::ImportResult::Status::Failed);
    QVERIFY(session.globalEditor()->draft().filterEnabled);
    QCOMPARE(session.pendingConfiguration().maintenance.allVolumesProtected, std::optional<bool>{true});
  }
  void guidImportUsesConfigurationIdentityWithoutResolvingADrive() {
    auto editor = std::make_shared<MemoryVolumeEditor>();
    editor->protectedVolume = true;
    app::UwfmgrCommand command;
    command.kind = app::UwfmgrKind::VolumeUnprotect;
    command.args = {R"(\\?\Volume{11111111-1111-1111-1111-111111111111}\)"};
    const auto report = app::applyImportCommands(commands(app::renderUwfmgrCommand(command)), nullptr, {editor}, nullptr, noDriveResolution());
    QCOMPARE(report[0].status, app::ImportResult::Status::Success);
    QVERIFY(!editor->protectedVolume);
  }
  void exportedUnprotectAllRoundTripsMountedAndOfflineRegistrations() {
    constexpr auto mounted = "11111111-2222-3333-4444-555555555555";
    constexpr auto offline = "22222222-2222-3333-4444-555555555555";
    core::UwfSnapshot snapshot;
    snapshot.uwfAvailable = snapshot.elevated = true;
    snapshot.current = snapshot.next = baseline();
    snapshot.next.volumes = {{compat::volumePath(mounted), "C:", true, core::VolumeBinding::VolumeId, std::string("C:")},
                             {compat::volumePath(offline), "D:", true, core::VolumeBinding::VolumeId, std::string{}}};
    app::ConfigurationEditSession session(16384, {}, noDriveResolution());
    const auto page = session.editVolume({.driveLetter = "C:", .volumeName = compat::volumePath(mounted), .fileSystem = "NTFS", .label = {}}, false);
    session.resetSnapshot(snapshot, {}, false);
    app::ConfigurationPlan plan;
    plan.snapshot = snapshot;
    plan.maintenance.allVolumesProtected = false;
    plan.maintenanceSnapshot.emplace();
    plan.maintenanceSnapshot->unprotectScopeConfirmed = true;
    plan.maintenanceSnapshot->unprotectTargetVolumes = {QString::fromLatin1(mounted), QString::fromLatin1(offline)};
    const auto script = app::configurationChangeScript(plan);
    QVERIFY(script.exportSupported);
    std::string text;
    for (const auto& command : script.commands) text += app::renderUwfmgrCommand(command) + "\n";
    text += "uwfmgr filter enable\n";
    QSignalSpy changed(&session, &app::ConfigurationEditSession::pendingChanged);
    const auto result = session.importCommands(commands(text));
    QCOMPARE(result.size(), 3);
    for (const auto& row : result) QCOMPARE(row.status, app::ImportResult::Status::Success);
    const auto pending = session.pendingChanges();
    QCOMPARE(pending.volumeProtect.size(), std::size_t{2});
    QCOMPARE(pending.volumeProtect.at(mounted), false);
    QCOMPARE(pending.volumeProtect.at(offline), false);
    QCOMPARE(pending.setFilterEnabled, std::optional{true});
    QVERIFY(changed.size() >= 2);
    QVERIFY(!page->status()->draft().protectedVolume);
    session.setEditingAllowed(false);
    const auto rejected = session.importCommands(commands(protectionCommand(true, offline) + " && uwfmgr filter disable"));
    QCOMPARE(rejected.size(), 2);
    QCOMPARE(rejected[0].status, app::ImportResult::Status::Failed);
    QCOMPARE(rejected[1].status, app::ImportResult::Status::Failed);
    QCOMPARE(session.pendingChanges().volumeProtect.at(offline), false);
    session.discardChanges(snapshot, {}, false);
    QVERIFY(session.pendingChanges().volumeProtect.empty());
  }
  void physicalPagesKeepTheirOwnCurrentStateAndUseNextBindingTargets_data() {
    QTest::addColumn<bool>("loose");
    QTest::newRow("tight") << false;
    QTest::newRow("loose") << true;
  }
  void physicalPagesKeepTheirOwnCurrentStateAndUseNextBindingTargets() {
    QFETCH(bool, loose);
    constexpr auto a = "11111111-2222-3333-4444-555555555555";
    constexpr auto b = "22222222-2222-3333-4444-555555555555";
    app::ConfigurationEditSession session(16384, {}, noDriveResolution());
    const auto e =
        session.editVolume({.driveLetter = "E:", .volumeName = compat::volumePath(a), .fileSystem = "NTFS", .label = {}, .physical = {true, true}}, false);
    const auto d =
        session.editVolume({.driveLetter = "D:", .volumeName = compat::volumePath(b), .fileSystem = "NTFS", .label = {}, .physical = {true, true}}, false);
    core::UwfSnapshot snapshot;
    snapshot.uwfAvailable = snapshot.elevated = true;
    snapshot.current = snapshot.next = baseline();
    snapshot.current.volumes = {{compat::volumePath(a), "D:", true, core::VolumeBinding::VolumeId, std::string("E:")},
                                {compat::volumePath(b), "D:", false, core::VolumeBinding::VolumeId, std::string("D:")}};
    snapshot.current.fileExclusions[compat::volumePath(a)] = {"\\A"};
    snapshot.next = snapshot.current;
    snapshot.next.volumes.front().binding = loose ? core::VolumeBinding::DriveLetter : core::VolumeBinding::VolumeId;
    snapshot.next.fileExclusions[compat::volumePath(a)] = {"\\NextA"};
    session.resetSnapshot(snapshot, {}, false);
    QVERIFY(e->status()->currentProtection() == true);
    QVERIFY(e->status()->draft().baselineProtected);
    QVERIFY(d->status()->currentProtection() == false);
    QVERIFY(!d->status()->draft().baselineProtected);
    QCOMPARE(e->files()->draft().current(), QStringList{"E:\\A"});
    QCOMPARE(e->files()->draft().next(), QStringList{"E:\\NextA"});
    QVERIFY(d->files()->draft().current().isEmpty());
    QVERIFY(d->files()->draft().next().isEmpty());
    const auto result = session.importCommands(commands("uwfmgr volume protect D: && uwfmgr volume unprotect E:"));
    QCOMPARE(result.size(), 2);
    for (const auto& row : result) QCOMPARE(row.status, app::ImportResult::Status::Success);
    QCOMPARE(session.pendingChanges().volumeProtect, (std::map<std::string, bool>{{a, false}, {b, true}}));
    session.discardChanges(snapshot, {}, false);
    const auto unchanged = session.importCommands(commands("uwfmgr volume unprotect D:"));
    QCOMPARE(unchanged.front().status, app::ImportResult::Status::Duplicate);
    QVERIFY(session.pendingChanges().empty());
    const auto byGuid = session.importCommands(commands(protectionCommand(false, a)));
    QCOMPARE(byGuid.front().status, app::ImportResult::Status::Success);
    QCOMPARE(session.pendingChanges().volumeProtect, (std::map<std::string, bool>{{a, false}}));
  }
  void newProtectionImportRetainsItsOriginalCreationSelector_data() {
    QTest::addColumn<bool>("byGuid");
    QTest::newRow("guid-on-lettered-volume") << true;
    QTest::newRow("drive-letter") << false;
  }
  void newProtectionImportRetainsItsOriginalCreationSelector() {
    QFETCH(bool, byGuid);
    constexpr auto a = "11111111-2222-3333-4444-555555555555";
    app::ConfigurationEditSession session(16384, {}, noDriveResolution());
    const auto editor =
        session.editVolume({.driveLetter = "C:", .volumeName = compat::volumePath(a), .fileSystem = "NTFS", .label = {}, .physical = {true, true}}, false);
    core::UwfSnapshot snapshot;
    snapshot.uwfAvailable = snapshot.elevated = true;
    snapshot.current = snapshot.next = baseline();
    session.resetSnapshot(snapshot, {}, false);
    const auto text = byGuid ? protectionCommand(true, a) : std::string("uwfmgr volume protect C:");
    const auto result = session.importCommands(commands(text));
    QCOMPARE(result.front().status, app::ImportResult::Status::Success);
    const std::string target = byGuid ? a : "C:";
    const auto binding = byGuid ? core::VolumeBinding::VolumeId : core::VolumeBinding::DriveLetter;
    QCOMPARE(editor->status()->draft().protectionTarget, target);
    QCOMPARE(editor->status()->draft().binding, binding);
    QCOMPARE(session.pendingChanges().volumeProtect, (std::map<std::string, bool>{{target, true}}));
    QVERIFY(session.pendingChanges().volumeBindings.empty());
    // The native protect entry point does not recreate an already planned registration.
    const auto repeated = session.importCommands(commands(byGuid ? "uwfmgr volume protect C:" : protectionCommand(true, a)));
    QCOMPARE(repeated.front().status, app::ImportResult::Status::Duplicate);
    QCOMPARE(editor->status()->draft().protectionTarget, target);
    QCOMPARE(editor->status()->draft().binding, binding);
    QVERIFY(session.pendingChanges().volumeBindings.empty());
    const auto plan = app::reviewApplicationConfiguration(session.pendingConfiguration(), snapshot, {});
    const auto script = app::configurationChangeScript(plan);
    QVERIFY(script.exportSupported);
    QCOMPARE(script.commands.size(), std::size_t{1});
    QCOMPARE(script.commands.front().args.front(), byGuid ? compat::volumePath(a) : std::string("C:"));
    session.discardChanges(snapshot, {}, false);
    QVERIFY(session.pendingChanges().empty());
    QCOMPARE(editor->status()->draft().protectionTarget, std::string("C:"));
  }
  void bulkProtectionKeepsTheBindingOfAnAlreadyPlannedCreation_data() {
    QTest::addColumn<bool>("byGuid");
    QTest::newRow("guid-creation-then-explicit-loose") << true;
    QTest::newRow("letter-creation-before-protect-all") << false;
  }
  void bulkProtectionKeepsTheBindingOfAnAlreadyPlannedCreation() {
    QFETCH(bool, byGuid);
    constexpr auto a = "11111111-2222-3333-4444-555555555555";
    app::ConfigurationEditSession session;
    const auto editor =
        session.editVolume({.driveLetter = "C:", .volumeName = compat::volumePath(a), .fileSystem = "NTFS", .label = {}, .physical = {true, true}}, false);
    core::UwfSnapshot snapshot;
    snapshot.uwfAvailable = snapshot.elevated = true;
    snapshot.current = snapshot.next = baseline();
    session.resetSnapshot(snapshot, {}, false);
    const auto result = session.importCommands(commands(byGuid ? protectionCommand(true, a) : "uwfmgr volume protect C:"));
    QCOMPARE(result.front().status, app::ImportResult::Status::Success);
    if (byGuid) QVERIFY(editor->status()->editBinding(core::VolumeBinding::DriveLetter));
    const auto maintenance = session.maintenanceEditor();
    app::OverlayMaintenanceSnapshot observed;
    observed.filterEnabled = observed.hormEnabled = observed.overlayCommit = false;
    observed.protectScopeConfirmed = true;
    observed.protectTargetVolumes = {QString::fromLatin1(a)};
    const auto ticket = maintenance->beginObservation();
    QVERIFY(ticket);
    maintenance->finishObservation(*ticket, observed);
    maintenance->setInteractionContext(true, false);
    QVERIFY(maintenance->stageAllVolumesProtection(true));
    QCOMPARE(editor->status()->draft().binding, core::VolumeBinding::DriveLetter);
    const auto pending = session.pendingChanges();
    QVERIFY(pending.volumeProtect.empty());
    QCOMPARE(pending.volumeBindings, (std::map<std::string, core::VolumeBinding>{{byGuid ? a : "C:", core::VolumeBinding::DriveLetter}}));
  }
  void bulkImportReportsResettingAHiddenGuidOverrideAsAChange() {
    constexpr auto hidden = "22222222-2222-3333-4444-555555555555";
    core::UwfSnapshot snapshot;
    snapshot.uwfAvailable = snapshot.elevated = true;
    snapshot.current = snapshot.next = baseline();
    snapshot.next.volumes = {{compat::volumePath(hidden), "D:", true, core::VolumeBinding::VolumeId, std::string{}}};
    app::ConfigurationEditSession session;
    session.resetSnapshot(snapshot, {}, false);
    const auto maintenance = session.maintenanceEditor();
    app::OverlayMaintenanceSnapshot observed;
    observed.filterEnabled = observed.hormEnabled = observed.overlayCommit = false;
    observed.protectScopeConfirmed = true;
    observed.protectTargetVolumes = {QString::fromLatin1(hidden)};
    const auto ticket = maintenance->beginObservation();
    QVERIFY(ticket);
    maintenance->finishObservation(*ticket, observed);
    maintenance->setInteractionContext(true, false);
    auto result = session.importCommands(commands("uwfmgr volume protect all"));
    QCOMPARE(result.front().status, app::ImportResult::Status::Success);
    result = session.importCommands(commands(protectionCommand(false, hidden)));
    QCOMPARE(result.front().status, app::ImportResult::Status::Success);
    QCOMPARE(session.pendingChanges().volumeProtect.at(hidden), false);
    result = session.importCommands(commands("uwfmgr volume protect all"));
    QCOMPARE(result.front().status, app::ImportResult::Status::Success);
    QVERIFY(session.pendingChanges().volumeProtect.empty());
  }
  void unletteredConfigurationDoesNotMoveToAReusedLetter() {
    constexpr auto a = "11111111-2222-3333-4444-555555555555";
    constexpr auto b = "22222222-2222-3333-4444-555555555555";
    app::ConfigurationEditSession session;
    const auto hidden = session.editVolume({.driveLetter = {}, .volumeName = compat::volumePath(a), .fileSystem = "NTFS", .label = {}}, false);
    const auto d = session.editVolume({.driveLetter = "D:", .volumeName = compat::volumePath(b), .fileSystem = "NTFS", .label = {}}, false);
    core::UwfSnapshot snapshot;
    snapshot.uwfAvailable = snapshot.elevated = true;
    snapshot.current = snapshot.next = baseline();
    snapshot.current.volumes = {{compat::volumePath(a), "D:", true, core::VolumeBinding::VolumeId, std::string{}}};
    snapshot.next.volumes = {{compat::volumePath(a), "D:", true, core::VolumeBinding::DriveLetter, std::string{}}};
    session.resetSnapshot(snapshot, {}, false);
    QVERIFY(hidden->status()->currentProtection() == true);
    QVERIFY(hidden->status()->draft().hasConfiguration);
    QCOMPARE(hidden->status()->draft().baselineBinding, core::VolumeBinding::DriveLetter);
    QVERIFY(!d->status()->draft().hasConfiguration);
    const auto result = session.importCommands(commands(protectionCommand(false, a)));
    QCOMPARE(result.front().status, app::ImportResult::Status::Success);
    QVERIFY(!hidden->status()->draft().protectedVolume);
    QVERIFY(!d->status()->draft().protectedVolume);
    QCOMPARE(session.pendingChanges().volumeProtect, (std::map<std::string, bool>{{a, false}}));
  }
  void reportsNormalizedThresholds_data();
  void reportsNormalizedThresholds();
  void importsFullUint32SizesAndReportsRepeatedNoOps_data();
  void importsFullUint32SizesAndReportsRepeatedNoOps();
  void matchingUint32MaximumBaselineDoesNotQueueChange();
  void repeatedFieldsReportTheirFinalValueWithoutReordering();
  void reverseCommandsReportNoNetChange_data();
  void reverseCommandsReportNoNetChange();
  void oneUnchangedSettingDoesNotHideOtherPendingChanges_data();
  void oneUnchangedSettingDoesNotHideOtherPendingChanges();
  void supersededGlobalCommandsKeepTheirAcceptedDependencies();
  void normalizationBackToTheOriginalValueIsANoOp();
  void unchangedCommandRemainsANoOpWhenAnotherFieldNormalizesIt();
  void policyReportsUseTheFinalStructuralTarget_data();
  void policyReportsUseTheFinalStructuralTarget();
  void repeatedPolicyCommandsReportTheirFinalValue_data();
  void repeatedPolicyCommandsReportTheirFinalValue();
  void globalTypeReportUsesTheSharedStorageTarget();
  void unreadablePolicyTypesDoNotReplaceAGlobalTypeReport_data();
  void unreadablePolicyTypesDoNotReplaceAGlobalTypeReport();
  void acceptedPolicyCommandsRetainTheirOwnFinalStateWhenGlobalAdmissionCloses();
  void unavailableGlobalRejectsCommandsAndTheirDependents_data();
  void unavailableGlobalRejectsCommandsAndTheirDependents();
  void parseFailuresAndNoOpsKeepDependencySemantics();
  void fileRoutingUsesTheInjectedResolver();
  void importingTheSystemValueRestoresAManualDraft_data();
  void importingTheSystemValueRestoresAManualDraft();
  void importingTheOriginalVolumeProtectionRestoresAManualDraft();
  void importingAnOriginalExclusionRestoresAManualDraft();
  void fileImportRejectsInvalidAddWithoutRewritingRawRemoval_data();
  void fileImportRejectsInvalidAddWithoutRewritingRawRemoval();
};

void ConfigurationImportTests::fileImportRejectsInvalidAddWithoutRewritingRawRemoval_data() {
  QTest::addColumn<QString>("pattern");
  QTest::addColumn<bool>("existing");
  QTest::addColumn<bool>("adding");
  const QStringList patterns{QStringLiteral("Z:\\Data/Cache"), QStringLiteral("Z:/Data/Cache"), QStringLiteral("Z:\\Data\\Cache/")};
  for (qsizetype index = 0; index < patterns.size(); ++index)
    for (const bool existing : {false, true})
      for (const bool adding : {false, true})
        QTest::newRow(qPrintable(QStringLiteral("pattern-%1-existing-%2-add-%3").arg(index).arg(existing).arg(adding)))
            << patterns.at(index) << existing << adding;
}
void ConfigurationImportTests::fileImportRejectsInvalidAddWithoutRewritingRawRemoval() {
  QFETCH(QString, pattern);
  QFETCH(bool, existing);
  QFETCH(bool, adding);
  const auto native = QString(pattern).replace(u'/', u'\\');
  core::VolumeInfo info;
  info.driveLetter = "Z:";
  info.volumeName = "Volume{11111111-2222-3333-4444-555555555555}";
  info.support = core::VolumeSupport::Supported;
  const auto volume = std::make_shared<app::VolumeConfigurationEditor>(info, false);
  core::UwfSnapshot snapshot;
  snapshot.uwfAvailable = true;
  snapshot.elevated = true;
  snapshot.current.volumes = {{info.volumeName, info.driveLetter, true, core::VolumeBinding::VolumeId, info.driveLetter}};
  auto& observed = snapshot.current.fileExclusions[info.volumeName];
  observed = {native.mid(2).toStdString()};
  if (existing) observed.push_back(pattern.mid(2).toStdString());
  snapshot.next = snapshot.current;
  volume->applySnapshot(snapshot);
  QStringList resolved;
  const app::DriveResolver resolve = [&](const std::string& path) {
    resolved.append(QString::fromStdString(path));
    return std::string("Z:");
  };
  const auto script = QStringLiteral("uwfmgr file %1 \"%2\" && uwfmgr volume unprotect Z:\nuwfmgr file add-exclusion \"Z:\\Data\\Allowed\"")
                          .arg(adding ? QStringLiteral("add-exclusion") : QStringLiteral("remove-exclusion"), pattern);
  const auto report = app::applyImportCommands(commands(script.toStdString()), nullptr, {volume}, nullptr, resolve);
  QCOMPARE(report.size(), 3);
  if (adding) {
    QCOMPARE(report.front().status, app::ImportResult::Status::Failed);
    QCOMPARE(report.front().detail, QStringLiteral("UWF exclusion paths must use backslashes."));
    QCOMPARE(report.at(1).status, app::ImportResult::Status::Failed);
    QVERIFY(report.at(1).detail.contains(QStringLiteral("prerequisite")));
    QVERIFY(!volume->status()->pendingProtection());
    QCOMPARE(resolved, QStringList{QStringLiteral("Z:\\Data\\Allowed")});
  } else {
    QCOMPARE(report.front().status, existing ? app::ImportResult::Status::Success : app::ImportResult::Status::Duplicate);
    QCOMPARE(report.at(1).status, app::ImportResult::Status::Success);
    QCOMPARE(volume->status()->pendingProtection(), std::optional<bool>{false});
    QCOMPARE(resolved, QStringList({native, QStringLiteral("Z:\\Data\\Allowed")}));
  }
  QCOMPARE(report.back().status, app::ImportResult::Status::Success);
  QCOMPARE(volume->files()->pendingAdded(), QStringList{QStringLiteral("Z:\\Data\\Allowed")});
  QCOMPARE(volume->files()->pendingRemoved(), !adding && existing ? QStringList{pattern} : QStringList{});
  QVERIFY(volume->files()->draft().next().contains(native));
  QCOMPARE(volume->files()->draft().next().contains(pattern), existing);
}

void ConfigurationImportTests::reportsNormalizedThresholds_data() {
  QTest::addColumn<bool>("criticalFirst");
  QTest::newRow("warning-first") << false;
  QTest::newRow("critical-first") << true;
}

void ConfigurationImportTests::reportsNormalizedThresholds() {
  QFETCH(bool, criticalFirst);
  app::GlobalConfigurationEditor editor;
  editor.reset(baseline());
  const auto warning = std::string("uwfmgr overlay set-warningthreshold 500");
  const auto critical = std::string("uwfmgr overlay set-criticalthreshold 400");
  const auto input = commands(criticalFirst ? critical + "\n" + warning : warning + "\n" + critical);
  const auto report = app::applyImportCommands(input, &editor, {}, nullptr, noDriveResolution());

  QCOMPARE(report.size(), 2);
  QCOMPARE(editor.draft().overlay.warningThresholdMb, uint32_t{399});
  QCOMPARE(editor.draft().overlay.criticalThresholdMb, uint32_t{400});
  const auto& warningReport = report.at(criticalFirst ? 1 : 0);
  const auto& criticalReport = report.at(criticalFirst ? 0 : 1);
  QCOMPARE(warningReport.status, app::ImportResult::Status::Success);
  QVERIFY(warningReport.detail.contains(QStringLiteral("→ 399 MB")));
  QVERIFY(warningReport.detail.contains(QStringLiteral("requested 500 MB")));
  QCOMPARE(criticalReport.status, app::ImportResult::Status::Success);
  QVERIFY(criticalReport.detail.contains(QStringLiteral("→ 400 MB")));
  QCOMPARE(report.at(0).lineNo, 1);
  QCOMPARE(report.at(1).lineNo, 2);
  QCOMPARE(report.at(0).lineText, QString::fromStdString(input.at(0).rawLine));
  QCOMPARE(report.at(1).lineText, QString::fromStdString(input.at(1).rawLine));
}

void ConfigurationImportTests::importsFullUint32SizesAndReportsRepeatedNoOps_data() {
  QTest::addColumn<uint32_t>("maximumSize");
  constexpr auto signedMaximum = static_cast<uint32_t>(std::numeric_limits<int32_t>::max());
  QTest::newRow("signed-maximum") << signedMaximum;
  QTest::newRow("first-unsigned-only-value") << signedMaximum + uint32_t{1};
  QTest::newRow("uint32-maximum") << std::numeric_limits<uint32_t>::max();
}
void ConfigurationImportTests::importsFullUint32SizesAndReportsRepeatedNoOps() {
  QFETCH(uint32_t, maximumSize);
  const auto command = "uwfmgr overlay set-size " + std::to_string(maximumSize);
  const auto finalValue = QString::number(maximumSize) + QStringLiteral(" MB");
  app::GlobalConfigurationEditor editor;
  editor.reset(baseline());
  const auto initial = app::applyImportCommands(commands(command), &editor, {}, nullptr, noDriveResolution());
  QCOMPARE(initial.size(), 1);
  QCOMPARE(initial.at(0).status, app::ImportResult::Status::Success);
  QVERIFY(initial.at(0).detail.contains(QStringLiteral("→ ") + finalValue));
  QVERIFY(!initial.at(0).detail.contains(QStringLiteral("requested"), Qt::CaseInsensitive));
  QCOMPARE(editor.draft().overlay.maximumSizeMb, maximumSize);

  editor.reset(baseline());
  const auto report = app::applyImportCommands(commands(command + '\n' + command), &editor, {}, nullptr, noDriveResolution());

  QCOMPARE(report.size(), 2);
  QCOMPARE(editor.draft().overlay.maximumSizeMb, maximumSize);
  QCOMPARE(report.at(0).status, app::ImportResult::Status::Success);
  QVERIFY(report.at(0).detail.contains(QStringLiteral("replaced by a later command")));
  QVERIFY(report.at(0).detail.contains(QStringLiteral("Final draft value → ") + finalValue));
  QCOMPARE(report.at(1).status, app::ImportResult::Status::Duplicate);
  QVERIFY(report.at(1).detail.contains(QStringLiteral("→ ") + finalValue));
  QVERIFY(!report.at(1).detail.contains(QStringLiteral("requested"), Qt::CaseInsensitive));
  QCOMPARE((editor.importOverlayMaxMb(maximumSize)).outcome, uwf::app::GlobalEditOutcome::Unchanged);
}

void ConfigurationImportTests::repeatedFieldsReportTheirFinalValueWithoutReordering() {
  app::GlobalConfigurationEditor editor;
  editor.reset(baseline());
  const auto input = commands(
      "uwfmgr overlay set-warningthreshold 500\nuwfmgr overlay set-criticalthreshold 800\n"
      "uwfmgr overlay set-warningthreshold 600\nuwfmgr overlay set-warningthreshold 600");
  const auto report = app::applyImportCommands(input, &editor, {}, nullptr, noDriveResolution());

  QCOMPARE(report.size(), 4);
  QCOMPARE(editor.draft().overlay.warningThresholdMb, uint32_t{600});
  QCOMPARE(editor.draft().overlay.criticalThresholdMb, uint32_t{800});
  QCOMPARE(report.at(0).status, app::ImportResult::Status::Success);
  QCOMPARE(report.at(1).status, app::ImportResult::Status::Success);
  QCOMPARE(report.at(2).status, app::ImportResult::Status::Success);
  QCOMPARE(report.at(3).status, app::ImportResult::Status::Duplicate);
  QVERIFY(report.at(0).detail.contains(QStringLiteral("Final draft value → 600 MB")));
  QVERIFY(report.at(2).detail.contains(QStringLiteral("Final draft value → 600 MB")));
  QVERIFY(report.at(3).detail.contains(QStringLiteral("Final draft warning threshold → 600 MB")));
  for (qsizetype index = 0; index < report.size(); ++index) {
    QCOMPARE(report.at(index).lineNo, input.at(index).sourceLineNo);
    QCOMPARE(report.at(index).lineText, QString::fromStdString(input.at(index).rawLine));
  }
}

void ConfigurationImportTests::normalizationBackToTheOriginalValueIsANoOp() {
  app::GlobalConfigurationEditor editor;
  editor.reset(baseline(399, 400));
  const auto report = app::applyImportCommands(commands("uwfmgr overlay set-warningthreshold 500"), &editor, {}, nullptr, noDriveResolution());

  QCOMPARE(report.size(), 1);
  QCOMPARE(editor.draft().overlay.warningThresholdMb, uint32_t{399});
  QVERIFY(editor.pendingOverlay().empty());
  QCOMPARE(report.at(0).status, app::ImportResult::Status::Duplicate);
  QVERIFY(report.at(0).detail.contains(QStringLiteral("→ 399 MB")));
  QVERIFY(report.at(0).detail.contains(QStringLiteral("requested 500 MB")));
}

void ConfigurationImportTests::reverseCommandsReportNoNetChange_data() {
  QTest::addColumn<QString>("text");
  QTest::newRow("warning") << QStringLiteral("uwfmgr overlay set-warningthreshold 500\nuwfmgr overlay set-warningthreshold 1000");
  QTest::newRow("type") << QStringLiteral("uwfmgr overlay set-type RAM\nuwfmgr overlay set-type Disk");
  QTest::newRow("filter") << QStringLiteral("uwfmgr filter enable\nuwfmgr filter disable");
  QTest::newRow("size") << QStringLiteral("uwfmgr overlay set-size 16000\nuwfmgr overlay set-size 8192");
  QTest::newRow("critical") << QStringLiteral("uwfmgr overlay set-criticalthreshold 2000\nuwfmgr overlay set-criticalthreshold 3000");
}

void ConfigurationImportTests::reverseCommandsReportNoNetChange() {
  QFETCH(QString, text);
  app::GlobalConfigurationEditor editor;
  editor.reset(baseline());
  const auto input = commands(text.toStdString());
  const auto report = app::applyImportCommands(input, &editor, {}, nullptr, noDriveResolution());

  QCOMPARE(report.size(), 2);
  QCOMPARE(report.at(0).status, app::ImportResult::Status::Success);
  QCOMPARE(report.at(1).status, app::ImportResult::Status::Duplicate);
  QVERIFY(report.at(1).detail.contains(QStringLiteral("No net change for this setting")));
  QVERIFY(!report.at(1).detail.contains(QStringLiteral("Pending")));
  QVERIFY(!report.at(0).detail.contains(QStringLiteral("Pending")));
  QVERIFY(report.at(0).detail.contains(QStringLiteral("replaced by a later command")));
  QCOMPARE(editor.draft().overlay.warningThresholdMb, uint32_t{1000});
  QCOMPARE(editor.draft().overlay.type, core::OverlayType::Disk);
  QVERIFY(!editor.draft().filterEnabled);
  QVERIFY(editor.pendingOverlay().empty());
  QVERIFY(!editor.pendingFilterEnabled());
  QCOMPARE(report.at(0).lineText, QString::fromStdString(input.at(0).rawLine));
  QCOMPARE(report.at(1).lineText, QString::fromStdString(input.at(1).rawLine));
}

void ConfigurationImportTests::oneUnchangedSettingDoesNotHideOtherPendingChanges_data() {
  QTest::addColumn<QString>("script");
  QTest::addColumn<bool>("changesCritical");
  QTest::newRow("warning") << QStringLiteral(
                                  "uwfmgr overlay set-warningthreshold 500\nuwfmgr overlay set-warningthreshold 1000\n"
                                  "uwfmgr overlay set-criticalthreshold 2000")
                           << true;
  QTest::newRow("critical") << QStringLiteral(
                                   "uwfmgr overlay set-criticalthreshold 2000\nuwfmgr overlay set-criticalthreshold 3000\n"
                                   "uwfmgr overlay set-warningthreshold 500")
                            << false;
  QTest::newRow("maximum") << QStringLiteral(
                                  "uwfmgr overlay set-size 16000\nuwfmgr overlay set-size 8192\n"
                                  "uwfmgr overlay set-warningthreshold 500")
                           << false;
  QTest::newRow("type") << QStringLiteral(
                               "uwfmgr overlay set-type RAM\nuwfmgr overlay set-type Disk\n"
                               "uwfmgr overlay set-warningthreshold 500")
                        << false;
  QTest::newRow("filter") << QStringLiteral("uwfmgr filter enable\nuwfmgr filter disable\nuwfmgr overlay set-warningthreshold 500") << false;
}

void ConfigurationImportTests::oneUnchangedSettingDoesNotHideOtherPendingChanges() {
  QFETCH(QString, script);
  QFETCH(bool, changesCritical);
  app::GlobalConfigurationEditor editor;
  editor.reset(baseline());
  const auto input = commands(script.toStdString());
  const auto report = app::applyImportCommands(input, &editor, {}, nullptr, noDriveResolution());
  QCOMPARE(report.size(), 3);
  QVERIFY(!editor.pendingOverlay().empty());
  QCOMPARE(editor.draft().overlay.warningThresholdMb, changesCritical ? uint32_t{1000} : uint32_t{500});
  QCOMPARE(editor.draft().overlay.criticalThresholdMb, changesCritical ? uint32_t{2000} : uint32_t{3000});
  QCOMPARE(editor.draft().overlay.maximumSizeMb, uint32_t{8192});
  QCOMPARE(editor.draft().overlay.type, core::OverlayType::Disk);
  QVERIFY(!editor.draft().filterEnabled);
  QCOMPARE(report[0].status, app::ImportResult::Status::Success);
  QCOMPARE(report[1].status, app::ImportResult::Status::Duplicate);
  QCOMPARE(report[2].status, app::ImportResult::Status::Success);
  QVERIFY(report[0].detail.contains(QStringLiteral("replaced by a later command")));
  QVERIFY(!report[0].detail.contains(QStringLiteral("Pending")));
  QVERIFY(report[1].detail.contains(QStringLiteral("No net change for this setting")));
  QVERIFY(report[2].detail.startsWith(QStringLiteral("Final draft ")));
  QVERIFY(!report[2].detail.contains(QStringLiteral("Pending")));
  for (qsizetype row = 0; row < report.size(); ++row) {
    QVERIFY(!report[row].detail.contains(QStringLiteral("No net change from this import")));
    QCOMPARE(report[row].lineNo, input[row].sourceLineNo);
    QCOMPARE(report[row].lineText, QString::fromStdString(input[row].rawLine));
  }
}

void ConfigurationImportTests::supersededGlobalCommandsKeepTheirAcceptedDependencies() {
  app::GlobalConfigurationEditor editor;
  editor.reset(baseline());
  auto volume = std::make_shared<MemoryVolumeEditor>();
  const auto input = commands(
      "uwfmgr overlay set-warningthreshold 500 && uwfmgr volume protect Z:\n"
      "uwfmgr overlay set-warningthreshold 1000\nuwfmgr overlay set-criticalthreshold 2000");
  const auto report = app::applyImportCommands(input, &editor, {volume}, nullptr, noDriveResolution());
  QCOMPARE(report.size(), 4);
  QVERIFY(volume->protectedVolume);
  QCOMPARE(report[0].status, app::ImportResult::Status::Success);
  QCOMPARE(report[1].status, app::ImportResult::Status::Success);
  QCOMPARE(report[2].status, app::ImportResult::Status::Duplicate);
  QCOMPARE(report[3].status, app::ImportResult::Status::Success);
  QVERIFY(report[0].detail.contains(QStringLiteral("replaced by a later command")));
  QVERIFY(!report[0].detail.contains(QStringLiteral("Pending")));
  QCOMPARE(editor.draft().overlay.warningThresholdMb, uint32_t{1000});
  QCOMPARE(editor.draft().overlay.criticalThresholdMb, uint32_t{2000});
}

void ConfigurationImportTests::unchangedCommandRemainsANoOpWhenAnotherFieldNormalizesIt() {
  app::GlobalConfigurationEditor editor;
  editor.reset(baseline(500, 1000));
  const auto report = app::applyImportCommands(commands("uwfmgr overlay set-warningthreshold 500\nuwfmgr overlay set-criticalthreshold 400"), &editor, {},
                                               nullptr, noDriveResolution());

  QCOMPARE(report.size(), 2);
  QCOMPARE(editor.draft().overlay.warningThresholdMb, uint32_t{399});
  QCOMPARE(report.at(0).status, app::ImportResult::Status::Duplicate);
  QVERIFY(report.at(0).detail.contains(QStringLiteral("→ 399 MB")));
  QCOMPARE(report.at(1).status, app::ImportResult::Status::Success);
}

void ConfigurationImportTests::policyReportsUseTheFinalStructuralTarget_data() {
  QTest::addColumn<uint32_t>("flags");
  QTest::addColumn<QString>("script");
  QTest::addColumn<int>("field");
  QTest::addColumn<int>("reportIndex");
  QTest::addColumn<bool>("target");
  QTest::addColumn<bool>("targetRam");
  QTest::addColumn<bool>("changedAtCommand");
  QTest::newRow("passthrough-between-type-commands") << app::OverlayFlags::Passthrough
                                                     << QStringLiteral(
                                                            "uwfmgr overlay set-type RAM\nuwfmgr overlay set-passthrough off\nuwfmgr overlay set-type Disk")
                                                     << int(app::OverlayPolicyField::Passthrough) << 1 << false << false << false;
  QTest::newRow("persistent-between-type-commands") << app::OverlayFlags::Persistent
                                                    << QStringLiteral(
                                                           "uwfmgr overlay set-type RAM\nuwfmgr overlay set-persistent off\nuwfmgr overlay set-type Disk")
                                                    << int(app::OverlayPolicyField::Persistent) << 1 << false << false << false;
  QTest::newRow("same-value-intent-before-type-change")
      << app::OverlayFlags::Passthrough << QStringLiteral("uwfmgr overlay set-passthrough on\nuwfmgr overlay set-type RAM")
      << int(app::OverlayPolicyField::Passthrough) << 0 << true << true << false;
  QTest::newRow("restore-value-after-type-change") << app::OverlayFlags::Passthrough
                                                   << QStringLiteral("uwfmgr overlay set-type RAM\nuwfmgr overlay set-passthrough on")
                                                   << int(app::OverlayPolicyField::Passthrough) << 1 << true << true << true;
}

void ConfigurationImportTests::policyReportsUseTheFinalStructuralTarget() {
  QFETCH(uint32_t, flags);
  QFETCH(QString, script);
  QFETCH(int, field);
  QFETCH(int, reportIndex);
  QFETCH(bool, target);
  QFETCH(bool, targetRam);
  QFETCH(bool, changedAtCommand);
  OverlayImportFixture fixture(core::OverlayType::Disk, flags);
  const auto input = commands(script.toStdString());
  const auto report = fixture.session.importCommands(input);
  QCOMPARE(report.size(), input.size());
  const auto& row = report.at(reportIndex);
  QCOMPARE(row.status, changedAtCommand ? app::ImportResult::Status::Success : app::ImportResult::Status::Duplicate);
  QVERIFY(row.detail.startsWith(QStringLiteral("Final draft ")));
  QVERIFY(row.detail.contains(target ? QStringLiteral("→ Enabled") : QStringLiteral("→ Disabled")));
  QVERIFY(!row.detail.contains(QStringLiteral("no-op")));
  QVERIFY(!row.detail.contains(QStringLiteral("No net change")));
  QCOMPARE(row.lineNo, input.at(reportIndex).sourceLineNo);
  QCOMPARE(row.lineText, QString::fromStdString(input.at(reportIndex).rawLine));

  const auto changes = fixture.session.pendingConfiguration();
  const auto option = static_cast<app::OverlayPolicyField>(field);
  QCOMPARE(app::OverlayPolicyEditor::value(changes.overlayPolicy, option), std::optional{target});
  const auto plan = app::planApplicationConfiguration(changes, fixture.snapshot, fixture.policy);
  QCOMPARE(app::configurationOverlayTarget(plan).overlay.type, targetRam ? core::OverlayType::RAM : core::OverlayType::Disk);
  const auto changeScript = app::configurationChangeScript(plan);
  const auto kind = input.at(reportIndex).kind;
  const auto emitted = std::ranges::find(changeScript.commands, kind, &app::UwfmgrCommand::kind);
  QVERIFY(emitted != changeScript.commands.end());
  QCOMPARE(emitted->args, std::vector<std::string>{target ? "on" : "off"});
  QCOMPARE(std::ranges::count(changeScript.commands, kind, &app::UwfmgrCommand::kind), 1);
}

void ConfigurationImportTests::repeatedPolicyCommandsReportTheirFinalValue_data() {
  QTest::addColumn<QString>("verb");
  QTest::addColumn<int>("type");
  QTest::addColumn<uint32_t>("flags");
  QTest::addColumn<bool>("currentFilter");
  QTest::addColumn<bool>("withGlobal");
  for (const bool withGlobal : {false, true}) {
    const auto add = [&](const char* name, const char* verb, const core::OverlayType type, const uint32_t flags = 0, const bool currentFilter = false) {
      QTest::newRow(qPrintable(QStringLiteral("%1-global-%2").arg(QString::fromLatin1(name)).arg(withGlobal)))
          << QString::fromLatin1(verb) << int(type) << flags << currentFilter << withGlobal;
    };
    add("passthrough", "set-passthrough", core::OverlayType::Disk);
    add("persistent", "set-persistent", core::OverlayType::Disk);
    add("read-only-media", "set-rom-mode", core::OverlayType::RAM);
    add("reset", "reset-persistentstate", core::OverlayType::Disk, app::OverlayFlags::Persistent, true);
  }
}

void ConfigurationImportTests::repeatedPolicyCommandsReportTheirFinalValue() {
  QFETCH(QString, verb);
  QFETCH(int, type);
  QFETCH(uint32_t, flags);
  QFETCH(bool, currentFilter);
  QFETCH(bool, withGlobal);
  OverlayImportFixture fixture(static_cast<core::OverlayType>(type), flags, currentFilter);
  const auto input = commands(QStringLiteral("uwfmgr overlay %1 on\nuwfmgr overlay %1 off\nuwfmgr overlay %1 off").arg(verb).toStdString());
  const auto report = app::applyImportCommands(input, withGlobal ? fixture.session.globalEditor().get() : nullptr, {}, nullptr, noDriveResolution(),
                                               fixture.session.overlayPolicyEditor().get());
  QCOMPARE(report.size(), 3);
  for (int index : {0, 1}) {
    QCOMPARE(report.at(index).status, app::ImportResult::Status::Success);
    QVERIFY(report.at(index).detail.contains(QStringLiteral("replaced by a later command")));
    QVERIFY(report.at(index).detail.contains(QStringLiteral("Final draft value → Disabled")));
  }
  QCOMPARE(report.back().status, app::ImportResult::Status::Duplicate);
  QVERIFY(report.back().detail.startsWith(QStringLiteral("Final draft ")));
  QVERIFY(report.back().detail.contains(QStringLiteral("→ Disabled")));
  QVERIFY(report.back().detail.contains(QStringLiteral("No net change for this setting")));
  QVERIFY(fixture.session.pendingConfiguration().overlayPolicy.empty());
  for (qsizetype index = 0; index < report.size(); ++index) {
    QCOMPARE(report.at(index).lineNo, input.at(index).sourceLineNo);
    QCOMPARE(report.at(index).lineText, QString::fromStdString(input.at(index).rawLine));
  }
}

void ConfigurationImportTests::globalTypeReportUsesTheSharedStorageTarget() {
  OverlayImportFixture fixture(core::OverlayType::RAM, 0);
  fixture.session.maintenanceEditor()->replaceDraft({.storageVolume = QStringLiteral("11111111-2222-3333-4444-555555555555")});
  const auto report = fixture.session.importCommands(commands("uwfmgr overlay set-type RAM"));
  QCOMPARE(report.size(), 1);
  QCOMPARE(report.front().status, app::ImportResult::Status::Duplicate);
  QVERIFY(report.front().detail.contains(QStringLiteral("Final draft Overlay type → Disk (requested RAM)")));
  QVERIFY(!fixture.session.pendingChanges().setOverlay.type);
  const auto plan = app::reviewApplicationConfiguration(fixture.session.pendingConfiguration(), fixture.snapshot, fixture.policy);
  QCOMPARE(app::configurationOverlayTarget(plan).overlay.type, core::OverlayType::Disk);
}

void ConfigurationImportTests::unreadablePolicyTypesDoNotReplaceAGlobalTypeReport_data() {
  QTest::addColumn<int>("status");
  QTest::addColumn<std::int64_t>("type");
  QTest::newRow("unobserved") << int(app::PolicyObservationStatus::Unobserved) << std::int64_t{0};
  QTest::newRow("failed-stale-ram") << int(app::PolicyObservationStatus::Failed) << std::int64_t{0};
  QTest::newRow("unknown-observed-type") << int(app::PolicyObservationStatus::Available) << std::int64_t{99};
}

void ConfigurationImportTests::unreadablePolicyTypesDoNotReplaceAGlobalTypeReport() {
  QFETCH(int, status);
  QFETCH(std::int64_t, type);
  app::GlobalConfigurationEditor global;
  global.reset(baseline());
  app::OverlayPolicySnapshot observations;
  observations.next.overlayType = {static_cast<app::PolicyObservationStatus>(status), type};
  app::OverlayPolicyEditor policy;
  policy.reset(observations, {});
  const auto report = app::applyImportCommands(commands("uwfmgr overlay set-type Disk"), &global, {}, nullptr, noDriveResolution(), &policy);
  QCOMPARE(report.size(), 1);
  QCOMPARE(report.front().status, app::ImportResult::Status::Duplicate);
  QVERIFY(report.front().detail.contains(QStringLiteral("Final draft Overlay type → Disk")));
  QVERIFY(!report.front().detail.contains(QStringLiteral("requested")));
  QVERIFY(global.pendingOverlay().empty());
}

void ConfigurationImportTests::acceptedPolicyCommandsRetainTheirOwnFinalStateWhenGlobalAdmissionCloses() {
  OverlayImportFixture fixture(core::OverlayType::Disk, 0);
  const auto global = fixture.session.globalEditor();
  QObject receiver;
  connect(fixture.session.overlayPolicyEditor().get(), &app::OverlayPolicyEditor::pendingChanged, &receiver, [global] { global->setAvailable(false); });
  const auto report = fixture.session.importCommands(commands("uwfmgr overlay set-persistent on && uwfmgr filter enable"));
  QCOMPARE(report.size(), 2);
  QVERIFY(!global->available());
  QCOMPARE(report.front().status, app::ImportResult::Status::Success);
  QVERIFY(report.front().detail.contains(QStringLiteral("Final draft Persistent Overlay → Enabled")));
  QCOMPARE(report.back().status, app::ImportResult::Status::Failed);
  QVERIFY(report.back().detail.contains(QStringLiteral("Global configuration is unavailable")));
  QCOMPARE(fixture.session.pendingConfiguration().overlayPolicy.persistent, std::optional{true});
}

void ConfigurationImportTests::unavailableGlobalRejectsCommandsAndTheirDependents_data() {
  QTest::addColumn<bool>("missing");
  QTest::newRow("missing-global") << true;
  QTest::newRow("unavailable-global") << false;
}

void ConfigurationImportTests::unavailableGlobalRejectsCommandsAndTheirDependents() {
  QFETCH(bool, missing);
  app::GlobalConfigurationEditor editor;
  editor.reset(baseline());
  editor.setAvailable(false);
  auto volume = std::make_shared<MemoryVolumeEditor>();
  const auto input = commands(
      "uwfmgr filter enable && uwfmgr volume protect Z:\nuwfmgr overlay set-size 16000\n"
      "uwfmgr overlay set-type RAM\nuwfmgr volume protect Z:");
  const auto report = app::applyImportCommands(input, missing ? nullptr : &editor, {volume}, nullptr, noDriveResolution());

  QCOMPARE(report.size(), 5);
  for (const qsizetype index : {qsizetype{0}, qsizetype{2}, qsizetype{3}}) {
    QCOMPARE(report.at(index).status, app::ImportResult::Status::Failed);
    QVERIFY(report.at(index).detail.contains(QStringLiteral("unavailable")));
  }
  QCOMPARE(report.at(1).status, app::ImportResult::Status::Failed);
  QVERIFY(report.at(1).detail.contains(QStringLiteral("prerequisite")));
  QCOMPARE(report.at(4).status, missing ? app::ImportResult::Status::Success : app::ImportResult::Status::Failed);
  QCOMPARE(volume->protectedVolume, missing);
  QVERIFY(!editor.draft().filterEnabled);
  QCOMPARE(editor.draft().overlay.maximumSizeMb, uint32_t{8192});
  QCOMPARE(editor.draft().overlay.type, core::OverlayType::Disk);
}

void ConfigurationImportTests::parseFailuresAndNoOpsKeepDependencySemantics() {
  app::GlobalConfigurationEditor editor;
  editor.reset(baseline());
  const auto input = commands(
      "uwfmgr overlay set-size nope && uwfmgr filter enable\n"
      "uwfmgr overlay set-size 8192 && uwfmgr filter enable");
  const auto report = app::applyImportCommands(input, &editor, {}, nullptr, noDriveResolution());

  QCOMPARE(report.size(), 4);
  QCOMPARE(report.at(0).status, app::ImportResult::Status::Failed);
  QCOMPARE(report.at(1).status, app::ImportResult::Status::Failed);
  QVERIFY(report.at(1).detail.contains(QStringLiteral("prerequisite")));
  QCOMPARE(report.at(2).status, app::ImportResult::Status::Duplicate);
  QCOMPARE(report.at(3).status, app::ImportResult::Status::Success);
  QVERIFY(editor.draft().filterEnabled);
}

void ConfigurationImportTests::matchingUint32MaximumBaselineDoesNotQueueChange() {
  app::GlobalConfigurationEditor editor;
  auto state = baseline();
  state.overlay.maximumSizeMb = std::numeric_limits<uint32_t>::max();
  editor.reset(state);
  const auto report = app::applyImportCommands(commands("uwfmgr overlay set-size 4294967295"), &editor, {}, nullptr, noDriveResolution());
  QCOMPARE(report.size(), 1);
  QCOMPARE(report.front().status, app::ImportResult::Status::Duplicate);
  QVERIFY(report.front().detail.contains(QStringLiteral("4294967295 MB")));
  QCOMPARE(editor.draft().overlay.maximumSizeMb, std::numeric_limits<uint32_t>::max());
  QVERIFY(editor.pendingOverlay().empty());
}

void ConfigurationImportTests::importingTheSystemValueRestoresAManualDraft_data() {
  QTest::addColumn<int>("field");
  QTest::addColumn<QString>("script");
  QTest::addColumn<QString>("finalValue");
  QTest::newRow("filter") << 0 << QStringLiteral("uwfmgr filter disable") << QStringLiteral("Disabled");
  QTest::newRow("type") << 1 << QStringLiteral("uwfmgr overlay set-type Disk") << QStringLiteral("Disk");
  QTest::newRow("maximum") << 2 << QStringLiteral("uwfmgr overlay set-size 8192") << QStringLiteral("8192 MB");
  QTest::newRow("warning") << 3 << QStringLiteral("uwfmgr overlay set-warningthreshold 1000") << QStringLiteral("1000 MB");
  QTest::newRow("critical") << 4 << QStringLiteral("uwfmgr overlay set-criticalthreshold 3000") << QStringLiteral("3000 MB");
}

void ConfigurationImportTests::importingTheSystemValueRestoresAManualDraft() {
  QFETCH(int, field);
  QFETCH(QString, script);
  QFETCH(QString, finalValue);
  app::GlobalConfigurationEditor editor;
  editor.reset(baseline());
  switch (field) {
    case 0:
      editor.editFilterEnabled(true);
      break;
    case 1:
      editor.editOverlayType(core::OverlayType::RAM);
      break;
    case 2:
      editor.editMaximumSize(16000);
      break;
    case 3:
      editor.editWarningThreshold(500);
      break;
    case 4:
      editor.editCriticalThreshold(2000);
      break;
    default:
      QFAIL("unknown test field");
  }
  QVERIFY(editor.pendingFilterEnabled().has_value() || !editor.pendingOverlay().empty());
  const auto report = app::applyImportCommands(commands(script.toStdString()), &editor, {}, nullptr, noDriveResolution());
  QVERIFY(!editor.pendingFilterEnabled());
  QVERIFY(editor.pendingOverlay().empty());
  QCOMPARE(report.size(), 1);
  QCOMPARE(report.front().status, app::ImportResult::Status::Success);
  QVERIFY(report.front().detail.startsWith(QStringLiteral("Final draft ")));
  QVERIFY(report.front().detail.contains(QStringLiteral("→ ") + finalValue));
  QVERIFY(!report.front().detail.contains(QStringLiteral("Pending")));
}

void ConfigurationImportTests::importingTheOriginalVolumeProtectionRestoresAManualDraft() {
  core::VolumeInfo info;
  info.driveLetter = "Z:";
  info.volumeName = "Volume{11111111-2222-3333-4444-555555555555}";
  info.support = core::VolumeSupport::Supported;
  auto volume = std::make_shared<app::VolumeConfigurationEditor>(info, false);
  core::UwfSnapshot snapshot;
  snapshot.uwfAvailable = true;
  snapshot.elevated = true;
  snapshot.current.volumes = {{info.volumeName, "Z:", true, core::VolumeBinding::DriveLetter, std::string("Z:")}};
  snapshot.next = snapshot.current;
  volume->applySnapshot(snapshot);
  QVERIFY(volume->status()->editProtection(false));
  QVERIFY(volume->status()->pendingProtection().has_value());
  const auto report = app::applyImportCommands(commands("uwfmgr volume protect Z:"), nullptr, {volume}, nullptr, noDriveResolution());
  QVERIFY(!volume->status()->pendingProtection());
  QCOMPARE(report.size(), 1);
  QCOMPARE(report.front().status, app::ImportResult::Status::Success);
  QVERIFY(report.front().detail.contains(QStringLiteral("draft")));
  QVERIFY(!report.front().detail.contains(QStringLiteral("Pending")));
}

void ConfigurationImportTests::importingAnOriginalExclusionRestoresAManualDraft() {
  app::ExclusionEditor registry(app::ExclusionEditor::Kind::Registry);
  const QString key = QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\DraftFixture");
  registry.setBaseline({key}, {key});
  QVERIFY(registry.removeSelection({{key, std::nullopt}}));
  QCOMPARE(registry.pendingRemoved(), QStringList{key});
  const auto report = app::applyImportCommands(commands("uwfmgr registry add-exclusion HKEY_LOCAL_MACHINE\\SOFTWARE\\DraftFixture"), nullptr, {}, &registry,
                                               noDriveResolution());
  QVERIFY(registry.pendingAdded().isEmpty());
  QVERIFY(registry.pendingRemoved().isEmpty());
  QCOMPARE(report.size(), 1);
  QCOMPARE(report.front().status, app::ImportResult::Status::Success);
  QVERIFY(report.front().detail.contains(QStringLiteral("draft")));
  QVERIFY(!report.front().detail.contains(QStringLiteral("pending"), Qt::CaseInsensitive));
}

void ConfigurationImportTests::fileRoutingUsesTheInjectedResolver() {
  auto volume = std::make_shared<MemoryVolumeEditor>();
  QStringList resolved;
  const app::DriveResolver resolve = [&](const std::string& path) {
    resolved.append(QString::fromStdString(path));
    return std::string("Z:");
  };
  const auto report = app::applyImportCommands(commands(R"(uwfmgr file add-exclusion "X:\Data")"), nullptr, {volume}, nullptr, resolve);

  QCOMPARE(report.size(), 1);
  QCOMPARE(report.at(0).status, app::ImportResult::Status::Success);
  QCOMPARE(resolved, QStringList{QStringLiteral("X:\\Data")});
  QCOMPARE(volume->added, QStringList{QStringLiteral("X:\\Data")});
}

}  // namespace

QTEST_APPLESS_MAIN(ConfigurationImportTests)
#include "ConfigurationImportTests.moc"
