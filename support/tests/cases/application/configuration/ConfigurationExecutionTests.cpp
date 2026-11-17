// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QStringView>
#include <QTest>
#include <algorithm>
#include <array>
#include <memory>
#include <set>
#include <stdexcept>
#include <tuple>
#include <utility>

#include "application/configuration/commands/UwfmgrCommandCodec.h"
#include "application/configuration/editing/ConfigurationEditSession.h"
#include "application/configuration/editing/GlobalConfigurationEditor.h"
#include "application/configuration/exclusions/ExclusionEditor.h"
#include "application/configuration/execution/ConfigurationExecutor.h"
#include "application/configuration/planning/ConfigurationScript.h"
#include "application/configuration/volumes/VolumeConfigurationEditor.h"
#include "application/console/ConsoleCommandExecutor.h"
#include "application/staging/editing/FileStagingDraft.h"
#include "application/staging/persistence/FileStagingRegistry.h"
#include "application/uwf/CompositeAllVolumeProtection.h"
#include "application/uwf/adapters/UwfConfigurationRepository.h"
#include "application/uwf/adapters/UwfOverlayMaintenance.h"
#include "application/uwf/adapters/UwfReadState.h"
#include "foundation/logging/Log.h"
#include "simulation/uwf/FakeUwfBackend.h"

using namespace uwf;

namespace {
using Fake = uwf::simulation::FakeUwfBackend;
constexpr auto volumeId = "11111111-2222-3333-4444-555555555555";
constexpr auto volumeName = "Volume{11111111-2222-3333-4444-555555555555}";
enum class Outcome { Success, Before, After };
enum class Coverage { RetainedRoot, RetainedAncestor, PlannedRoot, PlannedAncestor, ClearList, Alternatives };
class Store final : public app::FileStagingStore {
 public:
  QList<app::FileStagingEntry> entries;
  int writes = 0;
  QList<app::FileStagingEntry> load() const override { return entries; }
  void replace(const QList<app::FileStagingEntry>& value) override {
    entries = value;
    ++writes;
  }
};
class PhysicalVolumes final : public app::VolumeRepository {
 public:
  mutable int reads = 0;
  mutable std::vector<core::VolumeInfo> entries{{.driveLetter = "C:",
                                                 .volumeName = volumeName,
                                                 .fileSystem = "NTFS",
                                                 .label = {},
                                                 .totalBytes = 0,
                                                 .freeBytes = 0,
                                                 .support = core::VolumeSupport::Supported,
                                                 .physical = {true, true}}};
  std::vector<core::VolumeInfo> read() const override {
    ++reads;
    auto result = entries;
    entries.clear();  // A prepared batch must not resolve the same drive again.
    return result;
  }
  void publish(const std::shared_ptr<Fake>& backend) const {
    auto state = backend->snapshot();
    for (const auto& volume : entries) {
      const auto identity = compat::parseVolumeId(volume.volumeName);
      if (!identity) throw std::logic_error("Invalid fixture physical volume identity");
      state.runtime.physicalVolumes[*identity] = {volume.physical.fixedMedia.value_or(false), core::canRegisterVolume(volume)};
      state.runtime.fileMaintenancePrefixes[*identity] = volume.driveLetter;
    }
    backend->seed(std::move(state));
  }
  std::string driveForPath(const std::string&) const override { throw std::logic_error("unexpected path lookup"); }
  std::vector<std::string> mountedLetters(const std::string&) const override { return {"C:"}; }
  std::vector<std::string> mountedPaths(const std::string&) const override { return {"C:\\"}; }
  std::string systemDrive() const override { return "C:"; }
};
constexpr auto systemPartitionId = "66666666-7777-8888-9999-aaaaaaaaaaaa";
compat::AllVolumeProtectionEnvironment completeProtectionScope() {
  return {.isAdministrator = [](const compat::Context&) { return true; },
          .protectScope = [](bool, const compat::Context&) { return std::vector<std::string>{volumeId, systemPartitionId}; }};
}
class BulkConsoleEnvironment final : public app::ConsoleEnvironment {
 public:
  app::ConsoleVolumeTarget resolveVolume(const std::string& operand) const override {
    if (const auto id = compat::parseVolumeId(operand)) return {*id, {}, false};
    if (operand == "C:") return {volumeId, "C:", true};
    throw std::logic_error("Unexpected volume target");
  }
  app::ConsoleFileTarget resolveFile(const std::string& operand) const override {
    if (operand.starts_with("C:")) return {volumeId, operand.substr(2)};
    throw std::logic_error("Unexpected file target");
  }
};
struct BulkConfigurationFixture {
  app::ConfigurationEditSession session;
  Fake::State initial;
  std::shared_ptr<Fake> backend;
  std::shared_ptr<compat::AllVolumeProtection> allVolumes;
  std::shared_ptr<app::OverlayMaintenanceActions> maintenance;
  std::shared_ptr<PhysicalVolumes> physical = std::make_shared<PhysicalVolumes>();
  std::shared_ptr<app::VolumeConfigurationEditor> editor;
  core::UwfSnapshot snapshot;
  app::OverlayMaintenanceSnapshot observation;
  BulkConfigurationFixture(const bool registered, const bool protectedVolume) {
    if (registered)
      initial.next.volumes.emplace(volumeId, Fake::VolumeConfiguration{.volume = {volumeId, "C:", protectedVolume, true, {}}, .fileExclusions = {}});
    // The complete workflow includes a volume that has no UI page or drive.
    initial.next.volumes.emplace(systemPartitionId,
                                 Fake::VolumeConfiguration{.volume = {systemPartitionId, {}, protectedVolume, false, {}}, .fileExclusions = {}});
    initial.current = initial.next;
    backend = std::make_shared<Fake>(initial);
    physical->publish(backend);
    initial = backend->snapshot();
    allVolumes = app::composeAllVolumeProtection(backend, completeProtectionScope());
    (void)allVolumes->read(compat::GetAllProtected{});
    (void)allVolumes->read(compat::GetAllUnprotected{});
    maintenance = app::uwfOverlayMaintenance(backend, {}, {}, app::allVolumeProtectionActions(allVolumes));
    snapshot = app::publishedUwfSnapshot(backend->state(), app::UwfCapability::Available, true, {});
    editor = session.editVolume(physical->entries.front(), false);
    session.resetSnapshot(snapshot, {}, false);
    observation = maintenance->observe();
    const auto shared = session.maintenanceEditor();
    const auto ticket = shared->beginObservation();
    if (!ticket) throw std::logic_error("Failed to begin fixture observation");
    shared->finishObservation(*ticket, observation);
    shared->setInteractionContext(true, false);
  }
  QList<app::ImportResult> import(const std::string& input) {
    const auto parsed = app::parseUwfmgrText(input);
    return session.importCommands({parsed.begin(), parsed.end()});
  }
  app::ConfigurationPlan plan() const { return app::planApplicationConfiguration(session.pendingConfiguration(), snapshot, {}, {}, observation); }
  app::ConfigurationExecutor executor() const { return app::ConfigurationExecutor(app::uwfConfigurationRepository(backend), {}, maintenance); }
};
Fake::Fault fault(const compat::Request& request, Fake::FaultTiming timing) {
  return {.request = request, .operation = {}, .timing = timing, .kind = compat::ErrorKind::Failed, .message = "Injected configuration failure", .code = 0};
}
const app::ConfigurationStepResult& step(const app::ConfigurationApplyResult& result, app::ConfigurationOperation operation) {
  const auto entry = std::ranges::find_if(result.steps, [&](const auto& item) { return item.operation == operation; });
  if (entry == result.steps.end()) throw std::logic_error("Expected application operation was not reported");
  return *entry;
}
}  // namespace

class ConfigurationExecutionTests final : public QObject {
  Q_OBJECT
 private slots:
  void looseExclusionConflictsKeepTheConfiguredPhysicalIdentity() {
    constexpr auto otherId = "22222222-2222-3333-4444-555555555555";
    app::ConfigurationPlan plan;
    plan.snapshot.next.volumes = {{volumeName, "D:", true, core::VolumeBinding::DriveLetter, std::string("E:")}};
    plan.changes.addFileExclusions[volumeId] = {"E:\\Blocked"};
    app::FileStagingEntry entry{app::FileStagingKind::File, QStringLiteral("E:\\Blocked\\Item"),
                                app::FileCommitTarget{volumeId, QStringLiteral("\\Blocked\\Item")}};
    plan.staging = app::FileStagingChanges{{entry}, {entry}};
    plan.staging->volumeIdentities.emplace(QStringLiteral("D:"), otherId);
    plan.staging->volumeIdentities.emplace(QStringLiteral("E:"), volumeId);
    QVERIFY_THROWS_EXCEPTION(std::invalid_argument, app::validateFileStagingTarget(plan));
    // Reusing A's saved D: letter does not put B's file under A's exclusion.
    entry.path = QStringLiteral("D:\\Blocked\\Item");
    entry.fileTarget->volumeId = otherId;
    plan.staging->before = plan.staging->after = {entry};
    app::validateFileStagingTarget(plan);
  }
  void configurationEditsFollowThePhysicalOwnerAfterDriveReassignment_data() {
    QTest::addColumn<int>("route");
    QTest::newRow("E-page-keeps-guid-A") << 0;
    QTest::newRow("D-letter-without-B-registration-cannot-edit-A") << 1;
    QTest::newRow("D-letter-with-B-registration-only-edits-B") << 2;
  }
  void configurationEditsFollowThePhysicalOwnerAfterDriveReassignment() {
    QFETCH(int, route);
    constexpr auto otherId = "22222222-2222-3333-4444-555555555555";
    Fake::State initial;
    initial.current.volumes.emplace(volumeId, Fake::VolumeConfiguration{{volumeId, "D:", true, true, false}, {"\\FromA"}});
    if (route == 2) initial.current.volumes.emplace(otherId, Fake::VolumeConfiguration{{otherId, "D:", true, false, false}, {"\\FromB"}});
    initial.next = initial.current;
    initial.runtime.fileMaintenancePrefixes[volumeId] = "E:";
    initial.runtime.fileMaintenancePrefixes[otherId] = "D:";
    initial.runtime.physicalVolumes[volumeId] = initial.runtime.physicalVolumes[otherId] = {true, true};
    const auto backend = std::make_shared<Fake>(initial);
    const auto snapshot = app::publishedUwfSnapshot(backend->state(), app::UwfCapability::Available, true, {});
    core::PendingChanges changes;
    if (route == 0) {
      app::ConfigurationEditSession session;
      const auto editor =
          session.editVolume({.driveLetter = "E:", .volumeName = volumeName, .fileSystem = "NTFS", .label = {}, .physical = {true, true}}, false);
      const auto other = session.editVolume(
          {.driveLetter = "D:", .volumeName = std::string("Volume{") + otherId + '}', .fileSystem = "NTFS", .label = {}, .physical = {true, true}}, false);
      session.resetSnapshot(snapshot, {}, false);
      QVERIFY(editor->status()->draft().hasConfiguration);
      QVERIFY(editor->status()->draft().baselineProtected);
      QVERIFY(editor->files()->draft().next().contains(QStringLiteral("E:\\FromA")));
      QVERIFY(!other->status()->draft().hasConfiguration);
      QVERIFY(other->files()->draft().next().isEmpty());
      QVERIFY(editor->status()->editProtection(false));
      QCOMPARE(editor->importAddFileExclusion(QStringLiteral("E:\\Added")), app::ExclusionImportOutcome::Applied);
      QCOMPARE(editor->importRemoveFileExclusion(QStringLiteral("E:\\FromA")), app::ExclusionImportOutcome::Applied);
      changes = session.pendingChanges();
      QVERIFY(changes.volumeProtect.contains(volumeId));
      QVERIFY(changes.addFileExclusions.contains(volumeId));
      QVERIFY(changes.removeFileExclusions.contains(volumeId));
    } else {
      changes.volumeProtect["D:"] = false;
    }
    const auto result = app::ConfigurationExecutor(app::uwfConfigurationRepository(backend)).apply(changes, snapshot);
    const auto after = backend->snapshot();
    const auto& a = after.next.volumes.at(volumeId);
    if (route == 0) {
      QCOMPARE(result.failedOperations(), 0);
      QVERIFY(!a.volume.protectedVolume);
      QCOMPARE(a.fileExclusions, std::vector<std::string>{"\\Added"});
      QVERIFY(!after.next.volumes.contains(otherId));
    } else {
      QVERIFY(a == initial.next.volumes.at(volumeId));
      if (route == 1) {
        QVERIFY(result.failed());
        QVERIFY(!after.next.volumes.contains(otherId));
      } else {
        QCOMPARE(result.failedOperations(), 0);
        QVERIFY(!after.next.volumes.at(otherId).volume.protectedVolume);
        QCOMPARE(after.next.volumes.at(otherId).fileExclusions, std::vector<std::string>{"\\FromB"});
      }
    }
    const auto requests = backend->primitiveRequests();
    if (route != 1) QVERIFY(!requests.empty());
    for (const auto& request : requests) {
      std::visit(
          [&](const auto& operation) {
            if constexpr (requires { operation.volume; }) QCOMPARE(compat::normalizeVolumeId(operation.volume), std::string(route == 0 ? volumeId : otherId));
          },
          request);
    }
  }

  void importedProtectionRetainsItsCreationSelectorAndExistingBinding_data() {
    QTest::addColumn<bool>("registered");
    QTest::addColumn<bool>("storedByLetter");
    QTest::addColumn<bool>("guidInput");
    QTest::addColumn<bool>("repeatWithOtherSelector");
    for (const bool guid : {false, true}) {
      QTest::newRow(guid ? "new-GUID" : "new-letter") << false << false << guid << false;
      QTest::newRow(guid ? "planned-GUID-then-letter" : "planned-letter-then-GUID") << false << false << guid << true;
      QTest::newRow(guid ? "existing-tight-GUID" : "existing-tight-letter") << true << false << guid << true;
      QTest::newRow(guid ? "existing-loose-GUID" : "existing-loose-letter") << true << true << guid << true;
    }
  }
  void importedProtectionRetainsItsCreationSelectorAndExistingBinding() {
    QFETCH(bool, registered);
    QFETCH(bool, storedByLetter);
    QFETCH(bool, guidInput);
    QFETCH(bool, repeatWithOtherSelector);
    Fake::State initial;
    if (registered) initial.next.volumes.emplace(volumeId, Fake::VolumeConfiguration{{volumeId, "C:", false, storedByLetter, false}, {"\\Retained"}});
    const auto backend = std::make_shared<Fake>(initial);
    const auto physical = std::make_shared<PhysicalVolumes>();
    physical->publish(backend);
    const auto snapshot = app::publishedUwfSnapshot(backend->state(), app::UwfCapability::Available, true, {});
    app::ConfigurationEditSession session;
    const auto editor = session.editVolume(physical->entries.front(), false);
    session.resetSnapshot(snapshot, {}, false);
    QCOMPARE(editor->driveLetter(), QStringLiteral("C:"));
    QCOMPARE(editor->status()->draft().hasConfiguration, registered);
    const auto importProtection = [&](const bool guid) {
      app::UwfmgrCommand command;
      command.kind = app::UwfmgrKind::VolumeProtect;
      command.args = {guid ? compat::volumePath(volumeId) : std::string("C:")};
      QList<app::UwfmgrCommand> commands;
      for (auto parsed : app::parseUwfmgrText(app::renderUwfmgrCommand(command))) commands.append(std::move(parsed));
      return session.importCommands(commands);
    };
    const auto report = importProtection(guidInput);
    QCOMPARE(report.size(), 1);
    QCOMPARE(report.constFirst().status, app::ImportResult::Status::Success);
    const auto target = registered || guidInput ? std::string(volumeId) : std::string("C:");
    const bool boundByLetter = registered ? storedByLetter : !guidInput;
    const auto verifyDraft = [&] {
      QCOMPARE(editor->status()->draft().protectionTarget, target);
      QCOMPARE(editor->status()->draft().binding, boundByLetter ? core::VolumeBinding::DriveLetter : core::VolumeBinding::VolumeId);
      QVERIFY(!editor->status()->pendingBinding());
      const auto pending = session.pendingChanges();
      QCOMPARE(pending.volumeProtect.size(), std::size_t{1});
      QCOMPARE(pending.volumeProtect.at(target), true);
      QVERIFY(pending.volumeBindings.empty());
    };
    verifyDraft();
    if (repeatWithOtherSelector) {
      const auto repeated = importProtection(!guidInput);
      QCOMPARE(repeated.size(), 1);
      QCOMPARE(repeated.constFirst().status, app::ImportResult::Status::Duplicate);
      verifyDraft();
    }
    QVERIFY(backend->primitiveRequests().empty());
    const auto result = app::ConfigurationExecutor(app::uwfConfigurationRepository(backend)).apply(session.pendingChanges(), snapshot);
    QCOMPARE(result.failedOperations(), 0);
    QCOMPARE(result.succeededOperations(), 1);
    const auto observed = backend->snapshot().next.volumes.at(volumeId);
    QVERIFY(observed.volume.protectedVolume);
    QCOMPARE(observed.volume.bindByDriveLetter, boundByLetter);
    QCOMPARE(observed.volume.driveLetter, registered || !guidInput ? std::string("C:") : std::string{});
    QCOMPARE(observed.fileExclusions, registered ? std::vector<std::string>{"\\Retained"} : std::vector<std::string>{});
    std::vector<compat::Request> expected;
    if (!registered) expected.push_back(compat::AddVolume{volumeId, guidInput ? std::string{} : std::string("C:")});
    expected.push_back(compat::SetVolume{volumeId, compat::VolumeProperty::Protected, true});
    if (!registered) expected.push_back(compat::SetVolume{volumeId, compat::VolumeProperty::BindByDriveLetter, boundByLetter});
    QCOMPARE(backend->primitiveRequests(), expected);
    QCOMPARE(physical->reads, 0);
  }

  void bulkAndSingleVolumeTargetsReachExecutionAndBothScriptProducts_data() {
    QTest::addColumn<bool>("initialProtected");
    QTest::newRow("protect-all-then-unprotect-C") << false;
    QTest::newRow("unprotect-all-then-protect-C") << true;
  }
  void bulkAndSingleVolumeTargetsReachExecutionAndBothScriptProducts() {
    QFETCH(bool, initialProtected);
    BulkConfigurationFixture fixture(true, initialProtected);
    QVERIFY(fixture.session.maintenanceEditor()->stageAllVolumesProtection(!initialProtected));
    const auto report = fixture.import(initialProtected ? "uwfmgr volume protect C:" : "uwfmgr volume unprotect C:");
    QCOMPARE(report.front().status, app::ImportResult::Status::Success);
    const auto plan = fixture.plan();
    QCOMPARE(plan.changes.volumeProtect.at(volumeId), initialProtected);
    const auto result = fixture.executor().apply(plan);
    QCOMPARE(result.failedOperations(), 0);
    QCOMPARE(result.succeededOperations(), 2);
    QCOMPARE(fixture.backend->snapshot().next.volumes.at(volumeId).volume.protectedVolume, initialProtected);
    QCOMPARE(fixture.backend->snapshot().next.volumes.at(systemPartitionId).volume.protectedVolume, !initialProtected);
    const auto& bulk =
        step(result, {initialProtected ? app::ConfigurationOperationKind::UnprotectAllVolumes : app::ConfigurationOperationKind::ProtectAllVolumes, {}});
    QCOMPARE(bulk.maintenance->allVolumeItems.size(), std::size_t{2});
    QCOMPARE(bulk.outcome, app::ConfigurationStepOutcome::Succeeded);
    QCOMPARE(result.observedOverlayMaintenance->allVolumesNextProtected, std::optional<bool>{false});
    QCOMPARE(result.observedOverlayMaintenance->allVolumesNextUnprotected, std::optional<bool>{false});
    fixture.session.reconcileApplied({plan.changes, plan.overlayPolicy, plan.maintenance}, result);
    QVERIFY(fixture.session.maintenanceEditor()->draft().empty());
    for (const auto& script : {app::configurationChangeScript(plan), app::configurationTargetScript(plan)}) {
      const auto replay = std::make_shared<Fake>(fixture.initial);
      const auto bulk = app::composeAllVolumeProtection(replay, completeProtectionScope());
      (void)bulk->read(compat::GetAllProtected{});
      (void)bulk->read(compat::GetAllUnprotected{});
      const BulkConsoleEnvironment environment;
      for (const auto& command : script.commands) {
        const auto outcome = app::executeConsoleCommand(command, *replay, environment, {}, bulk.get());
        QCOMPARE(outcome.exitCode, 0);
      }
      QCOMPARE(replay->snapshot().next.volumes.at(volumeId).volume.protectedVolume, initialProtected);
      QCOMPARE(replay->snapshot().next.volumes.at(systemPartitionId).volume.protectedVolume, !initialProtected);
    }
  }
  void bulkCreatesNewRegistrationBeforeItsBindingAndExclusionWrites() {
    BulkConfigurationFixture fixture(false, false);
    const auto report = fixture.import("uwfmgr volume protect all && uwfmgr file add-exclusion C:\\Data");
    for (const auto& row : report) QCOMPARE(row.status, app::ImportResult::Status::Success);
    QVERIFY(fixture.editor->status()->editBinding(core::VolumeBinding::DriveLetter));
    const auto plan = fixture.plan();
    const auto result = fixture.executor().apply(plan);
    QCOMPARE(result.failedOperations(), 0);
    QCOMPARE(result.succeededOperations(), 3);
    const auto configured = fixture.backend->snapshot().next;
    QVERIFY(configured.volumes.at(volumeId).volume.protectedVolume);
    QVERIFY(configured.volumes.at(systemPartitionId).volume.protectedVolume);
    QVERIFY(configured.volumes.at(volumeId).volume.bindByDriveLetter);
    QCOMPARE(configured.volumes.at(volumeId).fileExclusions, std::vector<std::string>{R"(\Data)"});
    const auto script = app::configurationChangeScript(plan);
    QCOMPARE(script.commands.front().kind, app::UwfmgrKind::VolumeProtectAll);
    QCOMPARE(script.commands.back().kind, app::UwfmgrKind::FileAddExclusion);
  }
  void partialBulkFailureRetainsItsReceiptsAndSkipsEveryDependentVolumeWrite() {
    BulkConfigurationFixture fixture(false, false);
    const auto report = fixture.import("uwfmgr volume protect all && uwfmgr volume unprotect C: && uwfmgr file add-exclusion C:\\Data");
    for (const auto& row : report) QCOMPARE(row.status, app::ImportResult::Status::Success);
    QVERIFY(fixture.editor->status()->editBinding(core::VolumeBinding::DriveLetter));
    const auto plan = fixture.plan();
    fixture.backend->inject(fault(compat::SetVolume{systemPartitionId, compat::VolumeProperty::Protected, true, {}, false}, Fake::FaultTiming::Before));
    const auto result = fixture.executor().apply(plan);
    QCOMPARE(result.failedOperations(), 1);
    QVERIFY(result.anyWriteConfirmed());
    QCOMPARE(step(result, {app::ConfigurationOperationKind::ProtectAllVolumes, {}}).maintenance->allVolumeItems.size(), std::size_t{2});
    QCOMPARE(step(result, {app::ConfigurationOperationKind::VolumeProtection, "C:"}).outcome, app::ConfigurationStepOutcome::NotAttempted);
    QCOMPARE(step(result, {app::ConfigurationOperationKind::VolumeBinding, "C:"}).outcome, app::ConfigurationStepOutcome::NotAttempted);
    const auto configured = fixture.backend->snapshot().next.volumes.at(volumeId);
    QVERIFY(configured.volume.protectedVolume);
    QVERIFY(!configured.volume.bindByDriveLetter);
    QVERIFY(configured.fileExclusions.empty());
    fixture.session.reconcileApplied({plan.changes, plan.overlayPolicy, plan.maintenance}, result);
    QCOMPARE(fixture.session.maintenanceEditor()->draft().allVolumesProtected, std::optional<bool>{true});
  }
  void splittingBulkAndVolumePhasesWritesStagingExactlyOnce() {
    BulkConfigurationFixture fixture(true, false);
    const auto report = fixture.import("uwfmgr volume protect all && uwfmgr volume unprotect C:");
    for (const auto& row : report) QCOMPARE(row.status, app::ImportResult::Status::Success);
    const auto store = std::make_shared<Store>();
    const auto staging = std::make_shared<app::FileStagingDraft>(store);
    staging->replace({{app::FileStagingKind::File, QStringLiteral("C:\\Saved.bin")}});
    const auto plan =
        app::planApplicationConfiguration(fixture.session.pendingConfiguration(), fixture.snapshot, {}, staging->editSnapshot(), fixture.observation);
    app::ConfigurationExecutor executor(app::uwfConfigurationRepository(fixture.backend, staging), {}, fixture.maintenance);
    const auto result = executor.apply(plan);
    QCOMPARE(result.failedOperations(), 0);
    QCOMPARE(store->writes, 1);
    QVERIFY(result.staging());
    QCOMPARE(store->entries, staging->load());
    QVERIFY(!staging->changes());
    QCOMPARE(fixture.backend->snapshot().next.volumes.at(volumeId).volume.protectedVolume, false);
  }
  void hiddenProtectionUsesExactGuidAndKeepsUneditedBindingAndExclusions() {
    const std::string otherId = "66666666-7777-8888-9999-aaaaaaaaaaaa";
    Fake::State initial;
    initial.current.volumes.emplace(volumeId, Fake::VolumeConfiguration{.volume = {volumeId, {}, false, true, {}}, .fileExclusions = {R"(\Keep)"}});
    initial.next = initial.current;
    initial.next.volumes.emplace(otherId, Fake::VolumeConfiguration{.volume = {otherId, {}, true, false, {}}, .fileExclusions = {R"(\OfflineKeep)"}});
    const auto backend = std::make_shared<Fake>(initial);
    const auto snapshot = app::publishedUwfSnapshot(backend->state(), app::UwfCapability::Available, true, {});
    QVERIFY(app::nextVolumeRegistrationExists(snapshot.next, volumeId));
    QVERIFY(app::nextVolumeRegistrationExists(snapshot.next, "Volume{" + otherId + "}"));
    QVERIFY(!app::nextVolumeRegistrationExists(snapshot.next, ""));
    QVERIFY(!app::nextVolumeRegistrationExists(snapshot.next, "C:"));
    core::PendingChanges changes;
    changes.volumeProtect[volumeId] = true;
    changes.volumeProtect[otherId] = false;
    const auto plan = app::planConfiguration(changes, snapshot);
    const auto repository = app::uwfConfigurationRepository(backend);
    const auto prepared = app::ConfigurationExecutor(repository).prepareApply(plan);
    backend->clearTrace();
    const auto result = prepared();
    QCOMPARE(result.failedOperations(), 0);
    QCOMPARE(result.succeededOperations(), 2);
    const auto after = backend->snapshot();
    QVERIFY(after.next.volumes.at(volumeId).volume.protectedVolume);
    QVERIFY(!after.next.volumes.at(otherId).volume.protectedVolume);
    QVERIFY(after.next.volumes.at(volumeId).volume.bindByDriveLetter);
    QVERIFY(!after.next.volumes.at(otherId).volume.bindByDriveLetter);
    QCOMPARE(after.next.volumes.at(volumeId).fileExclusions, initial.next.volumes.at(volumeId).fileExclusions);
    QCOMPARE(after.next.volumes.at(otherId).fileExclusions, initial.next.volumes.at(otherId).fileExclusions);
    QVERIFY(after.current == initial.current);
    for (const auto& trace : backend->trace()) {
      const auto* write = std::get_if<compat::SetVolume>(&trace.request);
      QVERIFY(write);
      QCOMPARE(write->property, compat::VolumeProperty::Protected);
      QVERIFY(write->driveLetter.empty());
      QVERIFY(!write->bindByDriveLetterOnCreate);
      QVERIFY(write->volume == volumeId || write->volume == otherId);
    }
    QCOMPARE(step(result, {app::ConfigurationOperationKind::VolumeProtection, volumeId}).outcome, app::ConfigurationStepOutcome::Succeeded);
    QCOMPARE(step(result, {app::ConfigurationOperationKind::VolumeProtection, otherId}).outcome, app::ConfigurationStepOutcome::Succeeded);
  }
  void firstHiddenProtectionCreatesByVolumeIdentityAndFailedWriteKeepsItsReceipt() {
    auto backend = std::make_shared<Fake>();
    const auto snapshot = app::publishedUwfSnapshot(backend->state(), app::UwfCapability::Available, true, {});
    core::PendingChanges changes;
    changes.volumeProtect[volumeId] = true;
    backend->clearTrace();
    const auto result = app::ConfigurationExecutor(app::uwfConfigurationRepository(backend)).apply(changes, snapshot);
    QCOMPARE(result.failedOperations(), 0);
    const auto after = backend->snapshot();
    QVERIFY(after.next.volumes.at(volumeId).volume.protectedVolume);
    QVERIFY(!after.next.volumes.at(volumeId).volume.bindByDriveLetter);
    QVERIFY(after.current.volumes.empty());
    const auto set = compat::SetVolume{volumeId, compat::VolumeProperty::Protected, false, {}, false};
    backend->inject(fault(set, Fake::FaultTiming::AfterEffect));
    changes.volumeProtect[volumeId] = false;
    const auto failed = app::ConfigurationExecutor(app::uwfConfigurationRepository(backend)).apply(changes, snapshot);
    QCOMPARE(failed.failedOperations(), 1);
    const auto& receipt = step(failed, {app::ConfigurationOperationKind::VolumeProtection, volumeId});
    QCOMPARE(receipt.outcome, app::ConfigurationStepOutcome::Failed);
    QCOMPARE(receipt.writeEffect, app::WriteEffect::Uncertain);
    QVERIFY(failed.reconciliationRequired());
    QVERIFY(!backend->snapshot().next.volumes.at(volumeId).volume.protectedVolume);
  }
  void invalidTargetsAndUnregisteredExclusionsFailBeforeAnyWrite() {
    for (const int invalid : {0, 1, 2, 3, 4}) {
      const auto backend = std::make_shared<Fake>();
      core::PendingChanges changes;
      changes.setFilterEnabled = true;
      if (invalid == 0) changes.volumeProtect[""] = true;
      if (invalid == 1) changes.volumeProtect["Volume{incomplete}"] = true;
      if (invalid == 2) changes.volumeBindings["Volume{incomplete}"] = core::VolumeBinding::DriveLetter;
      if (invalid == 3) changes.addFileExclusions[volumeId] = {R"(\Hidden)"};
      if (invalid == 4) changes.removeFileExclusions["Volume{incomplete}"] = {R"(\Hidden)"};
      app::ConfigurationExecutor executor(app::uwfConfigurationRepository(backend));
      QVERIFY_THROWS_EXCEPTION(std::invalid_argument, (void)executor.prepareApply(changes, {}));
      QVERIFY(backend->trace().empty());
      QVERIFY(backend->snapshot().next.volumes.empty());
    }
  }
  void invalidCapacityPreflightPrecedesStagingFilterAndThresholdWrites() {
    for (const auto type : {core::OverlayType::RAM, core::OverlayType::Disk}) {
      const auto backend = std::make_shared<Fake>();
      auto store = std::make_shared<Store>();
      auto draft = std::make_shared<app::FileStagingDraft>(store);
      core::UwfSnapshot snapshot;
      snapshot.next.overlay = {.type = type, .maximumSizeMb = 2048};
      core::PendingChanges changes;
      changes.setFilterEnabled = true;
      changes.setOverlay.maximumSizeMb = 0;
      changes.setOverlay.warningThresholdMb = 100;
      const app::FileStagingChanges staging{{}, {{app::FileStagingKind::File, QStringLiteral("C:\\queued.bin")}}, false};
      const auto plan = app::planConfiguration(changes, snapshot, staging);
      const auto repository = app::uwfConfigurationRepository(backend, draft);
      app::ConfigurationExecutor executor(repository);
      QVERIFY_THROWS_EXCEPTION(std::invalid_argument, (void)executor.prepareApply(plan));
      // The executor validates the complete reviewed plan before it passes
      // validated phases to the repository's direct preparation surface.
      QCOMPARE(store->writes, 0);
      QVERIFY(store->entries.empty());
      QVERIFY(backend->trace().empty());
      QVERIFY(!backend->snapshot().next.filterEnabled);
    }
  }
  void registryLiteralNamesSurviveTheApplicationBoundary_data() {
    QTest::addColumn<QString>("key");
    QTest::addColumn<bool>("imported");
    QTest::addColumn<bool>("allowed");
    QTest::addColumn<bool>("scriptRepresentable");
    struct Name {
      const char* label;
      QString key;
      bool allowed;
      bool scriptRepresentable;
    };
    const auto root = QStringLiteral("HKLM\\SOFTWARE\\");
    const std::array names{Name{"slash", root + QStringLiteral("Slash/Name"), true, true},
                           Name{"control", root + QStringLiteral("Control") + QChar(1) + QStringLiteral("Name"), true, false},
                           Name{"tab", root + QStringLiteral("Tab\tName"), true, false},
                           Name{"repeated-separators", QStringLiteral("HKLM\\\\SOFTWARE\\\\Name\\\\"), true, true},
                           Name{"maximum-component", root + QString(255, u'X'), true, true},
                           Name{"oversized-component", root + QString(256, u'X'), false, true},
                           Name{"embedded-null", root + QStringLiteral("Before") + QChar(0) + QStringLiteral("After"), false, false}};
    for (const auto& name : names)
      for (const bool imported : {false, true})
        QTest::newRow(qPrintable(QStringLiteral("%1/%2").arg(name.label, imported ? "imported" : "selected")))
            << name.key << imported << name.allowed << name.scriptRepresentable;
  }
  void registryLiteralNamesSurviveTheApplicationBoundary() {
    QFETCH(QString, key);
    QFETCH(bool, imported);
    QFETCH(bool, allowed);
    QFETCH(bool, scriptRepresentable);
    const auto backend = std::make_shared<Fake>();
    const auto snapshot = app::publishedUwfSnapshot(backend->state(), app::UwfCapability::Available, true, {});
    app::ConfigurationEditSession session;
    session.resetSnapshot(snapshot);
    const auto editor = session.registryEditor();
    if (imported) {
      app::UwfmgrCommand command;
      command.kind = app::UwfmgrKind::RegistryAddExclusion;
      command.args = {key.toStdString()};
      // Control-character inputs exercise text import outside the generated CMD subset.
      const auto input = scriptRepresentable ? app::renderUwfmgrCommand(command) : "uwfmgr registry add-exclusion \"" + key.toStdString() + "\"";
      const auto parsed = app::parseUwfmgrText(input);
      QCOMPARE(parsed.size(), std::size_t{1});
      if (key.contains(QChar(0))) {
        QCOMPARE(parsed.front().parseError, app::ParseError::UnexpectedArgument);
        QVERIFY(parsed.front().args.empty());
      } else {
        QCOMPARE(parsed.front().parseError, app::ParseError::None);
        QCOMPARE(parsed.front().args, std::vector<std::string>{key.toStdString()});
      }
      const auto report = session.importCommands({parsed.front()});
      QCOMPARE(report.size(), 1);
      QCOMPARE(report.front().status, allowed ? app::ImportResult::Status::Success : app::ImportResult::Status::Failed);
    } else {
      const auto result = editor->addSelectedPath(key);
      QCOMPARE(result.path, key);
      QCOMPARE(result.outcome, allowed ? app::ExclusionImportOutcome::Applied : app::ExclusionImportOutcome::RejectedForbidden);
    }
    QCOMPARE(editor->pendingAdded(), allowed ? QStringList{key} : QStringList{});
    const auto result = app::ConfigurationExecutor(app::uwfConfigurationRepository(backend)).apply(session.pendingChanges(), snapshot);
    QCOMPARE(result.failedOperations(), 0);
    QCOMPARE(backend->snapshot().next.registryExclusions, allowed ? std::vector<std::string>{key.toStdString()} : std::vector<std::string>{});
    QVERIFY(backend->snapshot().current.registryExclusions.empty());
    QCOMPARE(std::ranges::count_if(backend->trace(), [](const auto& entry) { return compat::isWrite(entry.request); }), allowed ? 1 : 0);
    backend->simulateRestart();
    QCOMPARE(compat::call(*backend, compat::GetRegistryExclusions{compat::Session::Current}),
             allowed ? std::vector<std::string>{key.toStdString()} : std::vector<std::string>{});
  }
  void observedPrefixPatternsAreRemovedWithoutChangingTheirIdentity_data() {
    QTest::addColumn<QString>("pattern");
    QTest::addColumn<bool>("undo");
    for (const auto& pattern : {QStringLiteral("\\\\?\\Entry"), QStringLiteral("\\\\.\\Entry")})
      for (const bool undo : {false, true}) QTest::newRow(qPrintable(pattern + (undo ? "/undo" : "/remove"))) << pattern << undo;
  }
  void observedPrefixPatternsAreRemovedWithoutChangingTheirIdentity() {
    QFETCH(QString, pattern);
    QFETCH(bool, undo);
    Fake::State initial;
    initial.next.volumes[volumeId].volume = {volumeId, "C:", false, false, false};
    const auto backend = std::make_shared<Fake>(initial);
    // The bare entry makes an incorrectly stripped prefix address a different exclusion.
    (void)compat::call(*backend, compat::EditFileExclusion{volumeId, "Entry", compat::ExclusionAction::Add});
    (void)compat::call(*backend, compat::EditFileExclusion{volumeId, pattern.toStdString(), compat::ExclusionAction::Add});
    // A saved DriveLetter is not a mount observation. Publish the physical
    // fixture used by the editor before addressing its original patterns.
    const PhysicalVolumes physical;
    physical.publish(backend);
    const auto snapshot = app::publishedUwfSnapshot(backend->state(), app::UwfCapability::Available, true, {});
    app::ConfigurationEditSession session;
    session.globalEditor()->reset(snapshot.next);
    const auto editor = session.editVolume(physical.entries.front(), true);
    editor->applySnapshot(snapshot);
    const auto addressedPattern = QStringLiteral("C:") + pattern;
    QVERIFY(editor->files()->removeSelection({{addressedPattern, std::nullopt}}));
    QCOMPARE(editor->files()->pendingRemoved(), QStringList{addressedPattern});
    if (undo) {
      QVERIFY(editor->files()->restoreSelection({{addressedPattern, std::nullopt}}));
      QVERIFY(editor->files()->pendingRemoved().isEmpty());
      QVERIFY(editor->files()->pendingAdded().isEmpty());
    }
    backend->clearTrace();
    const auto result = app::ConfigurationExecutor(app::uwfConfigurationRepository(backend)).apply(session.pendingChanges(), snapshot);
    QStringList diagnostics;
    for (const auto& step : result.steps) diagnostics.append(QString::fromStdString(step.description));
    QVERIFY2(result.failedOperations() == 0, qPrintable(diagnostics.join(u'\n')));
    QCOMPARE(result.succeededOperations(), undo ? 0 : 1);
    const auto trace = backend->trace();
    QCOMPARE(std::ranges::count_if(trace, [](const auto& entry) { return compat::isWrite(entry.request); }), undo ? 0 : 1);
    if (!undo) {
      const auto removed = std::ranges::find_if(trace, [](const auto& entry) { return std::holds_alternative<compat::EditFileExclusion>(entry.request); });
      QVERIFY(removed != trace.end());
      QCOMPARE(std::get<compat::EditFileExclusion>(removed->request).path, pattern.toStdString());
    }
    backend->simulateRestart();
    const auto expected = undo ? std::vector<std::string>{pattern.toStdString(), "Entry"} : std::vector<std::string>{"Entry"};
    QCOMPARE(compat::call(*backend, compat::GetFileExclusions{volumeId, compat::Session::Current}), expected);
  }
  void fileAddNamesAreValidatedBeforeCreatingADraft_data() {
    QTest::addColumn<QString>("relative");
    QTest::addColumn<bool>("allowed");
    QTest::addColumn<bool>("imported");
    QTest::addColumn<bool>("precedesUnrelated");
    const auto add = [](const char* label, const QString& relative, bool allowed, bool precedesUnrelated = false) {
      for (const bool imported : {false, true})
        QTest::newRow(qPrintable(QStringLiteral("%1/%2").arg(label, imported ? "imported" : "selected")))
            << relative << allowed << imported << precedesUnrelated;
    };
    add("ordinary-260", QStringLiteral("\\") + QString(259, u'X'), true);
    add("ordinary-261", QStringLiteral("\\") + QString(260, u'X'), false);
    const auto pair = QString::fromUcs4(U"\U0001F642");
    add("utf16-260", QStringLiteral("\\") + pair.repeated(129) + u'X', true);
    add("utf16-261", QStringLiteral("\\") + pair.repeated(130), false);
    add("dot-segments-are-not-cleaned-before-validation", QStringLiteral("\\") + QString(255, u'X') + QStringLiteral("\\..\\File"), false);
    QTest::newRow("forward-slash/imported") << QStringLiteral("\\Before/After") << false << true << false;
    add("control-001e", QStringLiteral("\\Before") + QChar(0x1e) + QStringLiteral("After"), false);
    add("control-001f", QStringLiteral("\\Before") + QChar(0x1f) + QStringLiteral("After"), true, true);
    for (const QChar character : QStringView(u"<>:\"|?*")) {
      const auto label = QStringLiteral("forbidden-%1").arg(static_cast<quint32>(character.unicode()), 4, 16, u'0');
      add(qPrintable(label), QStringLiteral("\\Before") + character + QStringLiteral("After"), false);
    }
  }
  void fileAddNamesAreValidatedBeforeCreatingADraft() {
    QFETCH(QString, relative);
    QFETCH(bool, allowed);
    QFETCH(bool, imported);
    QFETCH(bool, precedesUnrelated);
    Fake::State initial;
    auto& configured = initial.next.volumes[volumeId];
    configured.volume = {volumeId, "C:", false, false, false};
    configured.fileExclusions = {"\\Unrelated"};
    initial.runtime.fileMaintenancePrefixes[volumeId] = "C:";
    const auto backend = std::make_shared<Fake>(initial);
    const auto snapshot = app::publishedUwfSnapshot(backend->state(), app::UwfCapability::Available, true, {});
    app::ConfigurationEditSession session;
    session.globalEditor()->reset(snapshot.next);
    const PhysicalVolumes physical;
    const auto editor = session.editVolume(physical.entries.front(), true);
    editor->applySnapshot(snapshot);
    const auto target = QStringLiteral("C:") + relative;
    if (imported) {
      app::UwfmgrCommand command;
      command.kind = app::UwfmgrKind::FileAddExclusion;
      command.args = {target.toStdString()};
      // Text import accepts these controls; generated CMD cannot represent them.
      const bool control = relative.contains(QChar(0x1e)) || relative.contains(QChar(0x1f));
      const auto text = control ? "uwfmgr file add-exclusion \"" + target.toStdString() + "\"" : app::renderUwfmgrCommand(command);
      const auto parsed = app::parseUwfmgrText(text);
      QCOMPARE(parsed.size(), std::size_t{1});
      QCOMPARE(parsed.front().parseError, app::ParseError::None);
      QCOMPARE(parsed.front().args, std::vector<std::string>{target.toStdString()});
      const auto report = session.importCommands({parsed.front()});
      QCOMPARE(report.size(), 1);
      QCOMPARE(report.front().status, allowed ? app::ImportResult::Status::Success : app::ImportResult::Status::Failed);
    } else {
      const auto selected = editor->files()->addSelectedPath(target);
      QCOMPARE(selected.path, target);
      QCOMPARE(selected.outcome, allowed ? app::ExclusionImportOutcome::Applied : app::ExclusionImportOutcome::RejectedForbidden);
      if (!allowed) QVERIFY(!selected.reason.isEmpty());
    }
    QCOMPARE(editor->files()->pendingAdded(), allowed ? QStringList{target} : QStringList{});
    QVERIFY(editor->files()->pendingRemoved().isEmpty());
    const auto before = backend->snapshot();
    backend->clearTrace();
    const auto result = app::ConfigurationExecutor(app::uwfConfigurationRepository(backend)).apply(session.pendingChanges(), snapshot);
    QCOMPARE(result.failedOperations(), 0);
    QCOMPARE(result.succeededOperations(), allowed ? 1 : 0);
    QCOMPARE(std::ranges::count_if(backend->trace(), [](const auto& entry) { return compat::isWrite(entry.request); }), allowed ? 1 : 0);
    if (!allowed) QVERIFY(backend->snapshot() == before);
    const auto expected = !allowed            ? std::vector<std::string>{"\\Unrelated"}
                          : precedesUnrelated ? std::vector<std::string>{relative.toStdString(), "\\Unrelated"}
                                              : std::vector<std::string>{"\\Unrelated", relative.toStdString()};
    QCOMPARE(compat::call(*backend, compat::GetFileExclusions{volumeId, compat::Session::Next}), expected);
    backend->simulateRestart();
    QCOMPARE(compat::call(*backend, compat::GetFileExclusions{volumeId, compat::Session::Current}), expected);
  }
  void fileNullNamesAreRejectedBeforeCreatingADraft_data() {
    QTest::addColumn<bool>("imported");
    QTest::addColumn<bool>("prefixAlreadyExcluded");
    for (const bool imported : {false, true})
      for (const bool existing : {false, true})
        QTest::newRow(qPrintable(QStringLiteral("import-%1/existing-prefix-%2").arg(imported).arg(existing))) << imported << existing;
  }
  void fileNullNamesAreRejectedBeforeCreatingADraft() {
    QFETCH(bool, imported);
    QFETCH(bool, prefixAlreadyExcluded);
    Fake::State initial;
    auto& volume = initial.next.volumes[volumeId];
    volume.volume = {volumeId, "C:", false, false, false};
    if (prefixAlreadyExcluded) volume.fileExclusions = {"\\Data\\Before"};
    const auto backend = std::make_shared<Fake>(initial);
    const auto snapshot = app::publishedUwfSnapshot(backend->state(), app::UwfCapability::Available, true, {});
    app::ConfigurationEditSession session;
    session.globalEditor()->reset(snapshot.next);
    const PhysicalVolumes physical;
    const auto editor = session.editVolume(physical.entries.front(), true);
    editor->applySnapshot(snapshot);
    const auto target = QStringLiteral("C:\\Data\\Before") + QChar(0) + QStringLiteral("After");
    if (imported) {
      const auto parsed = app::parseUwfmgrText("uwfmgr file add-exclusion \"" + target.toStdString() + "\"");
      QCOMPARE(parsed.size(), std::size_t{1});
      QCOMPARE(parsed.front().parseError, app::ParseError::UnexpectedArgument);
      QVERIFY(parsed.front().args.empty());
      const auto report = session.importCommands({parsed.front()});
      QCOMPARE(report.size(), 1);
      QCOMPARE(report.front().status, app::ImportResult::Status::Failed);
    } else {
      const auto result = editor->files()->addSelectedPath(target);
      QCOMPARE(result.path, target);
      QCOMPARE(result.outcome, app::ExclusionImportOutcome::RejectedForbidden);
      QCOMPARE(result.reason, QStringLiteral("File paths cannot contain null characters."));
    }
    QVERIFY(editor->files()->pendingAdded().isEmpty());
    QVERIFY(editor->files()->pendingRemoved().isEmpty());
    const auto before = backend->snapshot();
    backend->clearTrace();
    const auto result = app::ConfigurationExecutor(app::uwfConfigurationRepository(backend)).apply(session.pendingChanges(), snapshot);
    QCOMPARE(result.failedOperations(), 0);
    QCOMPARE(result.succeededOperations(), 0);
    QVERIFY(backend->snapshot() == before);
    QVERIFY(backend->trace().empty());
    backend->simulateRestart();
    QCOMPARE(compat::call(*backend, compat::GetFileExclusions{volumeId, compat::Session::Current}), volume.fileExclusions);
  }
  void removingStagingCoverageRequiresAnEmptyListConfirmation_data() {
    QTest::addColumn<QString>("removed");
    QTest::addColumn<bool>("retainedAncestor");
    QTest::addColumn<bool>("nonempty");
    const auto root = QString::fromUtf8(app::kFileStagingRegistryRoot);
    const QStringList paths{root,
                            QStringLiteral("HKLM") + root.mid(QStringLiteral("HKEY_LOCAL_MACHINE").size()),
                            root + u'\\',
                            root + QStringLiteral("\\\\"),
                            QStringLiteral("HKLM\\SOFTWARE\\HsingYun"),
                            QStringLiteral("HKLM\\SOFTWARE\\Unrelated")};
    for (qsizetype index = 0; index < paths.size(); ++index)
      for (const bool retained : {false, true})
        for (const bool nonempty : {false, true})
          QTest::newRow(qPrintable(QStringLiteral("path-%1-retained-%2-nonempty-%3").arg(index).arg(retained).arg(nonempty)))
              << paths.at(index) << retained << nonempty;
  }
  void removingStagingCoverageRequiresAnEmptyListConfirmation() {
    QFETCH(QString, removed);
    QFETCH(bool, retainedAncestor);
    QFETCH(bool, nonempty);
    const auto root = QString::fromUtf8(app::kFileStagingRegistryRoot);
    const bool unrelated = removed.endsWith(QStringLiteral("Unrelated"));
    const bool ancestor = removed == QStringLiteral("HKLM\\SOFTWARE\\HsingYun");
    const bool exactRoot = !unrelated && !ancestor && !removed.endsWith(u'\\');
    core::UwfSnapshot snapshot;
    snapshot.next.registryExclusions = {removed.toStdString()};
    if (retainedAncestor) snapshot.next.registryExclusions.push_back("HKLM\\SOFTWARE");
    core::PendingChanges changes;
    changes.removeRegistryExclusions = {removed.toStdString()};
    std::optional<app::FileStagingChanges> state;
    if (nonempty) {
      const QList<app::FileStagingEntry> entries{{app::FileStagingKind::File, QStringLiteral("C:\\saved.bin")}};
      state = app::FileStagingChanges{entries, entries, false};
    }
    const auto plan = app::planConfiguration(changes, snapshot, state);
    const bool emptyConfirmation = !nonempty && !unrelated && (!ancestor || !retainedAncestor);
    const bool replacementCoverage = nonempty && !unrelated && !exactRoot && !retainedAncestor;
    QCOMPARE(plan.staging.has_value(), nonempty || emptyConfirmation);
    QCOMPARE(plan.hasStagingChanges(), emptyConfirmation || replacementCoverage);
    if (plan.hasStagingChanges()) {
      QVERIFY(plan.staging->persistencePending);
      QCOMPARE(plan.staging->after.isEmpty(), !nonempty);
    }
    QCOMPARE(plan.changes.removeRegistryExclusions.empty(), nonempty && exactRoot);
    QCOMPARE(plan.changes.addRegistryExclusions, replacementCoverage ? std::vector{root.toStdString()} : std::vector<std::string>{});
  }
  void anUnchangedStagingSnapshotDoesNotCreateAWrite() {
    for (const bool covered : {false, true})
      for (const QList<app::FileStagingEntry>& entries :
           {QList<app::FileStagingEntry>{}, QList<app::FileStagingEntry>{{app::FileStagingKind::File, QStringLiteral("C:\\saved.bin")}}}) {
        core::UwfSnapshot snapshot;
        if (covered) snapshot.next.registryExclusions = {std::string(app::kFileStagingRegistryRoot)};
        const auto plan = app::planConfiguration({}, snapshot, app::FileStagingChanges{entries, entries, false});
        QVERIFY(plan.staging);
        QCOMPARE(plan.staging->after, entries);
        QVERIFY(!plan.hasStagingChanges());
        QVERIFY(plan.operations().empty());
        QVERIFY(app::configurationPhases(plan).empty());
        QVERIFY(plan.changes.empty());
      }
  }
  void confirmingAnEmptyListPreservesUnselectedRootSpellings() {
    const std::string root(app::kFileStagingRegistryRoot);
    const std::string alias = "HKLM" + root.substr(std::string("HKEY_LOCAL_MACHINE").size());
    Fake::State state;
    state.next.registryExclusions = {root, alias};
    const auto backend = std::make_shared<Fake>(state);
    auto store = std::make_shared<Store>();
    auto draft = std::make_shared<app::FileStagingDraft>(store);
    core::PendingChanges changes;
    changes.removeRegistryExclusions = {alias};
    static_cast<void>(draft->load());
    const auto plan =
        app::planConfiguration(changes, app::publishedUwfSnapshot(backend->state(), app::UwfCapability::Available, true, {}), draft->editSnapshot());
    QVERIFY(plan.staging);
    QCOMPARE(plan.changes.removeRegistryExclusions, std::vector{alias});
    const auto result = app::ConfigurationExecutor(app::uwfConfigurationRepository(backend, draft)).apply(plan);
    QCOMPARE(result.failedOperations(), 0);
    QCOMPARE(store->writes, 1);
    QCOMPARE(backend->snapshot().next.registryExclusions, std::vector{root});
  }
  void anImplicitEmptyConfirmationCannotEraseANonemptyDraft() {
    const auto backend = std::make_shared<Fake>();
    auto store = std::make_shared<Store>();
    store->entries = {{app::FileStagingKind::File, QStringLiteral("C:\\saved.bin")}};
    auto draft = std::make_shared<app::FileStagingDraft>(store);
    core::PendingChanges changes;
    changes.removeRegistryExclusions = {std::string(app::kFileStagingRegistryRoot)};
    const auto plan = app::planConfiguration(changes, {});
    QVERIFY(plan.staging);
    app::ConfigurationExecutor executor(app::uwfConfigurationRepository(backend, draft));
    QVERIFY_THROWS_EXCEPTION(std::runtime_error, (void)executor.prepareApply(plan));
    QCOMPARE(store->writes, 0);
    QCOMPARE(store->entries.size(), 1);
    QVERIFY(backend->trace().empty());
  }
  void stagingPlansRequireActualNextSessionBypass_data() {
    QTest::addColumn<int>("tails");
    for (const int tails : {0, 1, 2}) QTest::newRow(qPrintable(QString::number(tails))) << tails;
  }
  void stagingPlansRequireActualNextSessionBypass() {
    QFETCH(int, tails);
    const std::string root(app::kFileStagingRegistryRoot);
    core::UwfSnapshot snapshot;
    snapshot.next.registryExclusions = {root + std::string(static_cast<std::size_t>(tails), '\\')};
    const QList<app::FileStagingEntry> entries{{app::FileStagingKind::File, QStringLiteral("C:\\data.bin")}};
    const auto plan = app::planConfiguration({}, snapshot, app::FileStagingChanges{{}, entries, false});
    QCOMPARE(plan.changes.addRegistryExclusions, tails == 0 ? std::vector<std::string>{} : std::vector{root});
    QCOMPARE(plan.stagingCompletionAlternatives.size(), tails == 0 ? std::size_t{0} : std::size_t{1});
    if (tails != 0) QCOMPARE(plan.stagingCompletionAlternatives.front().target, root);
  }
  void guidFileExclusionImportsPreservePatternsThroughExecutionAndExport_data() {
    QTest::addColumn<bool>("adding");
    QTest::addColumn<bool>("storedByLetter");
    QTest::addColumn<bool>("extended");
    QTest::addColumn<QString>("pattern");
    const QStringList patterns{QStringLiteral("\\Data\\Cache"), QStringLiteral("\\Data\\.\\Cache\\"), QStringLiteral("\\Data\\\\Cache")};
    for (const bool adding : {false, true})
      for (const bool byLetter : {false, true})
        for (const bool extended : {false, true}) {
          for (qsizetype index = 0; index < patterns.size(); ++index)
            QTest::newRow(qPrintable(QStringLiteral("add-%1-letter-%2-extended-%3-pattern-%4").arg(adding).arg(byLetter).arg(extended).arg(index)))
                << adding << byLetter << extended << patterns[index];
          if (!adding)
            QTest::newRow(qPrintable(QStringLiteral("remove-raw-slash-letter-%1-extended-%2").arg(byLetter).arg(extended)))
                << false << byLetter << extended << QStringLiteral("\\Data/Cache");
        }
  }
  void guidFileExclusionImportsPreservePatternsThroughExecutionAndExport() {
    QFETCH(bool, adding);
    QFETCH(bool, storedByLetter);
    QFETCH(bool, extended);
    QFETCH(QString, pattern);
    Fake::State initial;
    initial.current.volumes.emplace(
        volumeId, Fake::VolumeConfiguration{.volume = {volumeId, "C:", true, storedByLetter, false}, .fileExclusions = {"\\Retained"}, .swapfileSize = 0});
    if (!adding) initial.current.volumes.at(volumeId).fileExclusions.push_back(pattern.toStdString());
    initial.next = initial.current;
    initial.runtime.fileMaintenancePrefixes[volumeId] = "C:";
    const auto backend = std::make_shared<Fake>(initial);
    const auto snapshot = app::publishedUwfSnapshot(backend->state(), app::UwfCapability::Available, true, {});
    const core::VolumeInfo physical{.driveLetter = "C:",
                                    .volumeName = volumeName,
                                    .fileSystem = "NTFS",
                                    .label = {},
                                    .totalBytes = 0,
                                    .freeBytes = 0,
                                    .support = core::VolumeSupport::Supported};
    int resolutions = 0;
    const app::DriveResolver resolve = [&](const std::string& path) {
      ++resolutions;
      if (!path.starts_with("C:")) throw std::logic_error("A GUID import must use the published configuration identity");
      return std::string("C:");
    };
    app::ConfigurationEditSession session(0, {}, resolve);
    const auto editor = session.editVolume(physical, false);
    session.resetSnapshot(snapshot, {}, false);
    app::UwfmgrCommand command;
    command.kind = adding ? app::UwfmgrKind::FileAddExclusion : app::UwfmgrKind::FileRemoveExclusion;
    command.args = {(extended ? std::string(R"(\\?\vOlUmE{)") : std::string("Volume{")) + volumeId + "}" + pattern.toStdString()};
    const auto import = [&](app::ConfigurationEditSession& destination, const std::string& text) {
      QList<app::UwfmgrCommand> commands;
      for (auto parsed : app::parseUwfmgrText(text))
        if (parsed.parseError != app::ParseError::Comment) commands.append(std::move(parsed));
      return destination.importCommands(commands);
    };
    const auto report = import(session, app::renderUwfmgrCommand(command));
    QCOMPARE(report.size(), 1);
    QCOMPARE(report.front().status, app::ImportResult::Status::Success);
    QCOMPARE(resolutions, 0);
    const auto addressed = QStringLiteral("C:") + pattern;
    QCOMPARE(adding ? editor->files()->pendingAdded() : editor->files()->pendingRemoved(), QStringList{addressed});
    const auto pending = session.pendingChanges();
    QVERIFY(pending.volumeProtect.empty());
    QVERIFY(pending.volumeBindings.empty());
    QCOMPARE((adding ? pending.addFileExclusions : pending.removeFileExclusions).at(volumeId), std::vector{addressed.toStdString()});
    const auto plan = app::planConfiguration(pending, snapshot);
    const auto result = app::ConfigurationExecutor(app::uwfConfigurationRepository(backend)).apply(plan);
    QVERIFY(!result.failed());
    const auto expectedRequest =
        compat::EditFileExclusion{volumeId, pattern.toStdString(), adding ? compat::ExclusionAction::Add : compat::ExclusionAction::Remove};
    QCOMPARE(backend->primitiveRequests(), std::vector<compat::Request>{expectedRequest});
    const auto expectedPatterns = adding ? std::vector<std::string>{"\\Retained", pattern.toStdString()} : std::vector<std::string>{"\\Retained"};
    QCOMPARE(backend->snapshot().next.volumes.at(volumeId).fileExclusions, expectedPatterns);
    QCOMPARE(backend->snapshot().current.volumes.at(volumeId), initial.current.volumes.at(volumeId));

    for (const auto& script : {app::configurationChangeScript(plan), app::configurationTargetScript(plan)}) {
      QVERIFY(script.exportSupported);
      const auto exported = std::ranges::find_if(
          script.commands, [&](const auto& item) { return item.kind == command.kind && item.args == std::vector{addressed.toStdString()}; });
      QVERIFY(exported != script.commands.end());
      const auto replayBackend = std::make_shared<Fake>(initial);
      app::ConfigurationEditSession replay(0, {}, resolve);
      const auto replayEditor = replay.editVolume(physical, false);
      replay.resetSnapshot(snapshot, {}, false);
      const auto replayReport = import(replay, app::renderConfigurationScript(script));
      for (const auto& row : replayReport) QVERIFY(row.status == app::ImportResult::Status::Success || row.status == app::ImportResult::Status::Duplicate);
      QCOMPARE(adding ? replayEditor->files()->pendingAdded() : replayEditor->files()->pendingRemoved(), QStringList{addressed});
      const auto replayResult = app::ConfigurationExecutor(app::uwfConfigurationRepository(replayBackend)).apply(replay.pendingChanges(), snapshot);
      QVERIFY(!replayResult.failed());
      QCOMPARE(replayBackend->primitiveRequests(), std::vector<compat::Request>{expectedRequest});
      QCOMPARE(replayBackend->snapshot().next.volumes.at(volumeId).fileExclusions, expectedPatterns);
    }
  }
  void guidFileExclusionImportKeepsThePlannedProtectionSelector_data() {
    QTest::addColumn<bool>("protectByGuid");
    QTest::newRow("protect-by-letter") << false;
    QTest::newRow("protect-by-GUID") << true;
  }
  void guidFileExclusionImportKeepsThePlannedProtectionSelector() {
    QFETCH(bool, protectByGuid);
    const auto backend = std::make_shared<Fake>();
    const auto physical = std::make_shared<PhysicalVolumes>();
    physical->publish(backend);
    const auto snapshot = app::publishedUwfSnapshot(backend->state(), app::UwfCapability::Available, true, {});
    app::ConfigurationEditSession session(0, {}, [](const std::string&) -> std::string { throw std::logic_error("Unexpected volume lookup during import"); });
    const auto editor = session.editVolume(physical->entries.front(), false);
    session.resetSnapshot(snapshot, {}, false);
    app::UwfmgrCommand protect;
    protect.kind = app::UwfmgrKind::VolumeProtect;
    protect.args = {protectByGuid ? compat::volumePath(volumeId) : std::string("C:")};
    app::UwfmgrCommand exclude;
    exclude.kind = app::UwfmgrKind::FileAddExclusion;
    exclude.args = {compat::volumePath(volumeId) + "Data\\Cache"};
    QList<app::UwfmgrCommand> commands;
    for (auto parsed : app::parseUwfmgrText(app::renderUwfmgrCommand(protect) + "\n" + app::renderUwfmgrCommand(exclude))) commands.append(std::move(parsed));
    const auto report = session.importCommands(commands);
    QCOMPARE(report.size(), 2);
    for (const auto& row : report) QCOMPARE(row.status, app::ImportResult::Status::Success);
    const auto target = protectByGuid ? std::string(volumeId) : std::string("C:");
    QCOMPARE(editor->status()->draft().protectionTarget, target);
    QCOMPARE(editor->status()->draft().binding, protectByGuid ? core::VolumeBinding::VolumeId : core::VolumeBinding::DriveLetter);
    const auto pending = session.pendingChanges();
    QCOMPARE(pending.volumeProtect, (std::map<std::string, bool>{{target, true}}));
    QVERIFY(pending.volumeBindings.empty());
    QCOMPARE(pending.addFileExclusions.at(target), std::vector<std::string>{"C:\\Data\\Cache"});
    QVERIFY(backend->primitiveRequests().empty());
    const auto result = app::ConfigurationExecutor(app::uwfConfigurationRepository(backend)).apply(pending, snapshot);
    QVERIFY(!result.failed());
    const auto stored = backend->snapshot().next.volumes.at(volumeId);
    QCOMPARE(stored.volume.bindByDriveLetter, !protectByGuid);
    QCOMPARE(stored.volume.driveLetter, protectByGuid ? std::string{} : std::string("C:"));
    QCOMPARE(stored.fileExclusions, std::vector<std::string>{"\\Data\\Cache"});
    const std::vector<compat::Request> expected{compat::AddVolume{volumeId, protectByGuid ? std::string{} : std::string("C:")},
                                                compat::SetVolume{volumeId, compat::VolumeProperty::Protected, true},
                                                compat::SetVolume{volumeId, compat::VolumeProperty::BindByDriveLetter, !protectByGuid},
                                                compat::EditFileExclusion{volumeId, "\\Data\\Cache", compat::ExclusionAction::Add}};
    QCOMPARE(backend->primitiveRequests(), expected);
    QCOMPARE(physical->reads, 0);
  }
  void configuredExclusionIdentitySurvivesEditingAndCommandRoundTrips_data() {
    QTest::addColumn<bool>("registry");
    QTest::addColumn<QString>("first");
    QTest::addColumn<QString>("second");
    const QString shortKey = QStringLiteral("HKLM\\SOFTWARE\\ConfiguredIdentity");
    const QString longKey = QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\ConfiguredIdentity");
    const QString path = QStringLiteral("\\Data\\ConfiguredIdentity");
    for (const bool reverse : {false, true}) {
      QTest::newRow(reverse ? "registry-long-selected" : "registry-short-selected") << true << (reverse ? shortKey : longKey) << (reverse ? longKey : shortKey);
      QTest::newRow(reverse ? "file-without-tail-selected" : "file-with-tail-selected")
          << false << (reverse ? path + u'\\' : path) << (reverse ? path : path + u'\\');
    }
    // IsValidRegKeyName permits trailing subkey spaces; quoting must preserve that identity.
    QTest::newRow("registry-trailing-space-selected") << true << longKey << longKey + u' ';
  }
  void configuredExclusionIdentitySurvivesEditingAndCommandRoundTrips() {
    QFETCH(bool, registry);
    QFETCH(QString, first);
    QFETCH(QString, second);
    Fake::State initial;
    initial.current.volumes.emplace(volumeId,
                                    Fake::VolumeConfiguration{.volume = {volumeId, "C:", true, false, false}, .fileExclusions = {}, .swapfileSize = 0});
    if (registry)
      initial.current.registryExclusions = {first.toStdString(), second.toStdString()};
    else
      initial.current.volumes.at(volumeId).fileExclusions = {first.toStdString(), second.toStdString()};
    initial.next = initial.current;
    initial.runtime.fileMaintenancePrefixes[volumeId] = "C:";
    const auto backend = std::make_shared<Fake>(initial);
    const auto snapshot = app::publishedUwfSnapshot(backend->state(), app::UwfCapability::Available, true, {});
    const auto initializeEditing = [](app::ConfigurationEditSession& session, const core::UwfSnapshot& state) {
      const core::VolumeInfo volume{.driveLetter = "C:",
                                    .volumeName = volumeName,
                                    .fileSystem = "NTFS",
                                    .label = {},
                                    .totalBytes = 0,
                                    .freeBytes = 0,
                                    .support = core::VolumeSupport::Supported};
      auto editor = session.editVolume(volume, true);
      session.resetSnapshot(state);
      return editor;
    };
    app::ConfigurationEditSession session;
    const auto volume = initializeEditing(session, snapshot);
    const auto editing = registry ? volume->registry() : volume->files();
    const auto displayed = [registry](const QString& path) { return registry ? path : QStringLiteral("C:") + path; };
    QCOMPARE(editing->draft().next().size(), 2);
    QVERIFY(editing->draft().next().contains(displayed(first)));
    QVERIFY(editing->draft().next().contains(displayed(second)));

    const auto exported = app::configurationTargetScript(app::planConfiguration({}, snapshot));
    auto empty = initial;
    empty.current.registryExclusions.clear();
    empty.current.volumes.at(volumeId).fileExclusions.clear();
    empty.next = empty.current;
    const auto importedBackend = std::make_shared<Fake>(empty);
    const auto emptySnapshot = app::publishedUwfSnapshot(importedBackend->state(), app::UwfCapability::Available, true, {});
    app::ConfigurationEditSession imported;
    const auto importedVolume = initializeEditing(imported, emptySnapshot);
    QList<app::UwfmgrCommand> commands;
    for (auto command : app::parseUwfmgrText(app::renderConfigurationScript(exported))) {
      if (command.parseError == app::ParseError::Comment) continue;
      QCOMPARE(command.parseError, app::ParseError::None);
      commands.append(std::move(command));
    }
    for (auto report = imported.importCommands(commands); const auto& row : std::as_const(report))
      QVERIFY(row.status == app::ImportResult::Status::Success || row.status == app::ImportResult::Status::Duplicate);
    const auto importedEditing = registry ? importedVolume->registry() : importedVolume->files();
    QCOMPARE(importedEditing->pendingAdded().size(), 2);
    const auto importedResult = app::ConfigurationExecutor(app::uwfConfigurationRepository(importedBackend)).apply(imported.pendingChanges(), emptySnapshot);
    QCOMPARE(importedResult.failedOperations(), 0);
    const auto importedState = importedBackend->snapshot();
    const auto& importedEntries = registry ? importedState.next.registryExclusions : importedState.next.volumes.at(volumeId).fileExclusions;
    QCOMPARE((std::set<std::string>(importedEntries.begin(), importedEntries.end())), (std::set<std::string>{first.toStdString(), second.toStdString()}));

    QVERIFY(editing->removeSelection({{displayed(second), std::nullopt}}));
    QCOMPARE(editing->pendingRemoved(), QStringList{displayed(second)});
    QVERIFY(editing->restoreSelection({{displayed(second), std::nullopt}}));
    QVERIFY(editing->pendingRemoved().isEmpty());
    QVERIFY(editing->removeSelection({{displayed(second), std::nullopt}}));
    const auto plan = app::planConfiguration(session.pendingChanges(), snapshot);
    const auto target = app::configurationTargetScript(plan);
    const auto addKind = registry ? app::UwfmgrKind::RegistryAddExclusion : app::UwfmgrKind::FileAddExclusion;
    const auto removeKind = registry ? app::UwfmgrKind::RegistryRemoveExclusion : app::UwfmgrKind::FileRemoveExclusion;
    QCOMPARE(std::ranges::count_if(target.commands, [&](const auto& command) { return command.kind == addKind; }), 1);
    QCOMPARE(std::ranges::count_if(target.commands, [&](const auto& command) { return command.kind == removeKind; }), 1);
    const auto removed = std::ranges::find_if(target.commands, [&](const auto& command) { return command.kind == removeKind; });
    QCOMPARE(removed->args, std::vector<std::string>{displayed(second).toStdString()});
    const auto result = app::ConfigurationExecutor(app::uwfConfigurationRepository(backend)).apply(plan);
    QCOMPARE(result.failedOperations(), 0);
    const auto observed = backend->snapshot();
    QCOMPARE(registry ? observed.next.registryExclusions : observed.next.volumes.at(volumeId).fileExclusions, std::vector<std::string>{first.toStdString()});
    QCOMPARE(registry ? observed.current.registryExclusions : observed.current.volumes.at(volumeId).fileExclusions,
             (std::vector<std::string>{first.toStdString(), second.toStdString()}));
  }
  void volumeBindingKeepsOneMeaningAcrossDraftReadAndWrite() {
    struct BindingCase {
      bool nativeBefore;
      core::VolumeBinding before;
      core::VolumeBinding requested;
      bool nativeAfter;
    };
    constexpr std::array cases{BindingCase{true, core::VolumeBinding::DriveLetter, core::VolumeBinding::VolumeId, false},
                               BindingCase{false, core::VolumeBinding::VolumeId, core::VolumeBinding::DriveLetter, true}};
    for (const auto& row : cases) {
      Fake::State initial;
      Fake::VolumeConfiguration configured;
      configured.volume = {volumeId, "C:", true, row.nativeBefore, false};
      initial.current.volumes.emplace(volumeId, configured);
      initial.next = initial.current;
      const auto backend = std::make_shared<Fake>(initial);
      const auto volumes = std::make_shared<PhysicalVolumes>();
      const auto snapshot = app::publishedUwfSnapshot(backend->state(), app::UwfCapability::Available, true, {});
      QCOMPARE(snapshot.next.volumes.front().binding, row.before);
      app::ConfigurationEditSession session;
      session.globalEditor()->reset(snapshot.next);
      const auto editor = session.editVolume(volumes->entries.front(), false);
      editor->applySnapshot(snapshot);
      QCOMPARE(editor->status()->draft().binding, row.before);
      QVERIFY(editor->status()->editBinding(row.requested));
      QCOMPARE(editor->status()->pendingBinding(), std::optional<core::VolumeBinding>{row.requested});
      const auto changes = session.pendingChanges();
      QCOMPARE(changes.volumeBindings.at(volumeId), row.requested);
      const auto result = app::ConfigurationExecutor(app::uwfConfigurationRepository(backend)).apply(changes, snapshot);
      QCOMPARE(result.failedOperations(), 0);
      const auto observed = backend->snapshot();
      QCOMPARE(observed.next.volumes.at(volumeId).volume.bindByDriveLetter, row.nativeAfter);
      QCOMPARE(observed.current.volumes.at(volumeId).volume.bindByDriveLetter, row.nativeBefore);
      const auto reread = app::publishedUwfSnapshot(backend->state(), app::UwfCapability::Available, true, {});
      QCOMPARE(reread.next.volumes.front().binding, row.requested);
    }
  }
  void firstProtectionUsesPublishedPhysicalFactsForEditorAndImport_data() {
    QTest::addColumn<bool>("imported");
    QTest::newRow("editor") << false;
    QTest::newRow("import") << true;
  }
  void firstProtectionUsesPublishedPhysicalFactsForEditorAndImport() {
    QFETCH(bool, imported);
    auto backend = std::make_shared<Fake>();
    auto volumes = std::make_shared<PhysicalVolumes>();
    volumes->publish(backend);
    auto snapshot = app::publishedUwfSnapshot(backend->state(), app::UwfCapability::Available, true, {});
    QVERIFY(snapshot.current.volumes.empty());
    QVERIFY(snapshot.next.volumes.empty());
    app::ConfigurationEditSession session;
    session.globalEditor()->reset(snapshot.next);
    const auto editor = session.editVolume(volumes->entries.front(), false);
    editor->applySnapshot(snapshot);
    QVERIFY(editor->status()->controlsEnabled());
    QVERIFY(!editor->status()->bindingEnabled());
    QVERIFY(!editor->status()->editBinding(core::VolumeBinding::VolumeId));
    if (imported) {
      QList<app::UwfmgrCommand> commands;
      for (auto command : app::parseUwfmgrText("uwfmgr.exe volume protect C:")) commands.append(std::move(command));
      const auto report = session.importCommands(commands);
      QCOMPARE(report.size(), 1);
      QCOMPARE(report.constFirst().status, app::ImportResult::Status::Success);
    } else {
      QVERIFY(editor->status()->editProtection(true));
    }
    QVERIFY(editor->status()->bindingEnabled());
    const auto changes = session.pendingChanges();
    QCOMPARE(changes.volumeProtect.at("C:"), true);
    const auto result = app::ConfigurationExecutor(app::uwfConfigurationRepository(backend)).apply(changes, snapshot);
    QCOMPARE(result.failedOperations(), 0);
    const auto observed = backend->snapshot().next.volumes.at(volumeId).volume;
    QVERIFY(observed.protectedVolume);
    QVERIFY(observed.bindByDriveLetter);
    QCOMPARE(volumes->reads, 0);
  }
  void registryConfigurationRemainsEditableWhileTheFilterIsOff() {
    auto backend = std::make_shared<Fake>();
    const auto snapshot = app::publishedUwfSnapshot(backend->state(), app::UwfCapability::Available, true, {});
    app::ConfigurationEditSession session;
    session.resetSnapshot(snapshot);
    QVERIFY(!session.registryEditor()->readOnly());
    QList<app::UwfmgrCommand> commands;
    for (auto command : app::parseUwfmgrText("uwfmgr.exe registry add-exclusion HKLM\\SOFTWARE\\Fixture")) commands.append(std::move(command));
    const auto report = session.importCommands(commands);
    QCOMPARE(report.size(), 1);
    for (const auto& entry : report) QCOMPARE(entry.status, app::ImportResult::Status::Success);
    session.registryEditor()->enablePersistence(app::ExclusionEditor::PersistenceItem::Tscal);
    const auto result = app::ConfigurationExecutor(app::uwfConfigurationRepository(backend)).apply(session.pendingChanges(), snapshot);
    QCOMPARE(result.failedOperations(), 0);
    QVERIFY(backend->snapshot().next.persistTscal);
    QCOMPARE(backend->snapshot().next.registryExclusions, std::vector<std::string>{"HKLM\\SOFTWARE\\Fixture"});
  }
  void protectionUsesPublishedPhysicalFactsAndOneDomainCommand() {
    auto backend = std::make_shared<Fake>();
    auto volumes = std::make_shared<PhysicalVolumes>();
    volumes->publish(backend);
    const auto repository = app::uwfConfigurationRepository(backend);
    core::PendingChanges changes;
    changes.volumeProtect["C:"] = true;
    const auto plan = app::planConfiguration(changes, {});
    const auto operation = repository->prepareWrite(app::ConfigurationSettingsPhase{.settings = changes}, plan);
    QCOMPARE(volumes->reads, 0);
    const auto result = operation({});
    QCOMPARE(volumes->reads, 0);
    QCOMPARE(result.failedOperations(), 0);
    QVERIFY(result.anyWriteConfirmed());
    const auto snapshot = backend->snapshot();
    const auto& observed = snapshot.next.volumes.at(volumeId).volume;
    QVERIFY(observed.protectedVolume);
    QVERIFY(observed.bindByDriveLetter);
    const auto trace = backend->trace();
    QCOMPARE(trace.size(), std::size_t{1});
    QVERIFY(trace.front().request == compat::Request(compat::SetVolume{volumeId, compat::VolumeProperty::Protected, true, "C:", true}));
  }
  void noOtherVolumeActionBlindlyRegisters_data() {
    QTest::addColumn<int>("action");
    QTest::addColumn<bool>("succeeds");
    QTest::newRow("unprotect-missing") << 0 << false;
    QTest::newRow("add-file-exclusion-missing") << 1 << false;
    QTest::newRow("remove-absent-file-exclusion") << 2 << true;
  }
  void noOtherVolumeActionBlindlyRegisters() {
    QFETCH(int, action);
    QFETCH(bool, succeeds);
    auto backend = std::make_shared<Fake>();
    auto volumes = std::make_shared<PhysicalVolumes>();
    volumes->publish(backend);
    core::PendingChanges changes;
    if (action == 0)
      changes.volumeProtect["C:"] = false;
    else if (action == 1)
      changes.addFileExclusions["C:"] = {"C:\\Fixture"};
    else
      changes.removeFileExclusions["C:"] = {"C:\\Fixture"};
    const auto before = backend->snapshot();
    app::ConfigurationExecutor executor(app::uwfConfigurationRepository(backend));
    if (action == 1) {
      // Reject a missing Next registration before inventory or UWF access.
      QVERIFY_THROWS_EXCEPTION(std::invalid_argument, static_cast<void>(executor.prepareApply(changes, {})));
      QCOMPARE(volumes->reads, 0);
      QVERIFY(backend->trace().empty());
      QVERIFY(backend->snapshot() == before);
      return;
    }
    const auto result = executor.apply(changes, {});
    QCOMPARE(result.failedOperations(), succeeds ? 0 : 1);
    QVERIFY(!result.anyWriteConfirmed());
    QVERIFY(backend->snapshot() == before);
    const auto trace = backend->trace();
    QVERIFY(backend->primitiveRequests().empty());
    for (const auto& entry : trace) {
      QVERIFY(compat::isWrite(entry.request));
      QVERIFY(!std::holds_alternative<compat::AddVolume>(entry.request));
      QCOMPARE(entry.response.effect, compat::Effect::None);
    }
  }
  void aSatisfiedCommandIsNotReportedAsAWrite() {
    auto backend = std::make_shared<Fake>();
    core::PendingChanges changes;
    changes.setFilterEnabled = false;
    const auto result = app::ConfigurationExecutor(app::uwfConfigurationRepository(backend)).apply(changes, {});
    QCOMPARE(result.succeededOperations(), 1);
    QCOMPARE(result.failedOperations(), 0);
    QVERIFY(!result.anyWriteConfirmed());
    QVERIFY(result.reconciliationRequired());
    const auto& observed = step(result, {app::ConfigurationOperationKind::Filter, {}});
    QCOMPARE(observed.outcome, app::ConfigurationStepOutcome::Succeeded);
    QCOMPARE(observed.writeEffect, app::WriteEffect::None);
  }
  void persistenceOnlyRequestsTheEditedSwitch() {
    Fake::State initial;
    initial.current.persistDomainSecretKey = initial.next.persistDomainSecretKey = true;
    auto backend = std::make_shared<Fake>(initial);
    core::PendingChanges changes;
    changes.setPersistTSCAL = true;
    const auto result = app::ConfigurationExecutor(app::uwfConfigurationRepository(backend)).apply(changes, {});
    QCOMPARE(result.failedOperations(), 0);
    const auto trace = backend->trace();
    QCOMPARE(trace.size(), std::size_t{1});
    QVERIFY(trace.front().request == compat::Request(compat::SetBoolean{compat::BooleanProperty::PersistTscal, true}));
    QVERIFY(backend->snapshot().next.persistDomainSecretKey);
    QVERIFY(backend->snapshot().next.persistTscal);
  }
  void stagingCompletionDependsOnlyOnItsExclusion_data() {
    QTest::addColumn<Coverage>("coverage");
    QTest::addColumn<bool>("exclusionFails");
    QTest::addColumn<bool>("licenseFails");
    for (const auto coverage :
         {Coverage::RetainedRoot, Coverage::RetainedAncestor, Coverage::PlannedRoot, Coverage::PlannedAncestor, Coverage::ClearList, Coverage::Alternatives})
      for (bool exclusionFails : {false, true})
        for (bool licenseFails : {false, true})
          QTest::newRow(qPrintable(QStringLiteral("coverage-%1-exclusion-%2-license-%3").arg(int(coverage)).arg(exclusionFails).arg(licenseFails)))
              << coverage << exclusionFails << licenseFails;
  }
  void stagingPreservesTheExactConfiguredExclusionSpelling_data() {
    QTest::addColumn<QStringList>("configured");
    QTest::addColumn<bool>("clear");
    const auto full = QString::fromLatin1(app::kFileStagingRegistryRoot.data(), static_cast<qsizetype>(app::kFileStagingRegistryRoot.size()));
    const auto shortened = QStringLiteral("HKLM") + full.mid(QStringLiteral("HKEY_LOCAL_MACHINE").size());
    const QList<QStringList> spellings{{full}, {shortened}, {full, shortened}};
    for (qsizetype index = 0; index < spellings.size(); ++index)
      for (const bool clear : {false, true})
        QTest::newRow(qPrintable(QStringLiteral("spelling-%1-clear-%2").arg(index).arg(clear))) << spellings.at(index) << clear;
  }
  void stagingPreservesTheExactConfiguredExclusionSpelling() {
    QFETCH(QStringList, configured);
    QFETCH(bool, clear);
    const QList<app::FileStagingEntry> entries{{app::FileStagingKind::File, QStringLiteral("C:\\Data\\state.bin")}};
    auto store = std::make_shared<Store>();
    if (clear) store->entries = entries;
    auto draft = std::make_shared<app::FileStagingDraft>(store);
    draft->replace(clear ? QList<app::FileStagingEntry>{} : entries);
    Fake::State initial;
    for (const auto& key : configured) initial.next.registryExclusions.push_back(key.toStdString());
    const auto backend = std::make_shared<Fake>(initial);
    const auto snapshot = app::publishedUwfSnapshot(backend->state(), app::UwfCapability::Available, true, {});
    const auto plan = app::planConfiguration({}, snapshot, draft->changes());
    QVERIFY(plan.changes.addRegistryExclusions.empty());
    if (clear)
      QCOMPARE((std::set<std::string>(plan.changes.removeRegistryExclusions.begin(), plan.changes.removeRegistryExclusions.end())),
               (std::set<std::string>(initial.next.registryExclusions.begin(), initial.next.registryExclusions.end())));
    else
      QVERIFY(plan.changes.removeRegistryExclusions.empty());
    const auto result = app::ConfigurationExecutor(app::uwfConfigurationRepository(backend, draft), draft->changes()).apply(plan);
    QCOMPARE(result.failedOperations(), 0);
    QVERIFY(result.staging());
    QVERIFY(!result.staging()->failure);
    QVERIFY(!draft->changes());
    QCOMPARE(backend->snapshot().next.registryExclusions, clear ? std::vector<std::string>{} : initial.next.registryExclusions);
    QCOMPARE(store->entries, clear ? QList<app::FileStagingEntry>{} : entries);
  }
  void stagingCompletionDependsOnlyOnItsExclusion() {
    QFETCH(Coverage, coverage);
    QFETCH(bool, exclusionFails);
    QFETCH(bool, licenseFails);
    const bool needsExclusion = coverage != Coverage::RetainedRoot && coverage != Coverage::RetainedAncestor;
    const bool alternatives = coverage == Coverage::Alternatives;
    auto store = std::make_shared<Store>();
    const QList<app::FileStagingEntry> entries{{app::FileStagingKind::File, QStringLiteral("C:\\data.txt")}};
    if (coverage == Coverage::ClearList) store->entries = entries;
    auto draft = std::make_shared<app::FileStagingDraft>(store);
    draft->replace(coverage == Coverage::ClearList ? QList<app::FileStagingEntry>{} : entries);
    core::UwfSnapshot snapshot;
    const std::string root(app::kFileStagingRegistryRoot);
    const std::string ancestor = "HKEY_LOCAL_MACHINE\\SOFTWARE\\HsingYun";
    if (coverage == Coverage::RetainedRoot || coverage == Coverage::ClearList) snapshot.next.registryExclusions = {root};
    if (coverage == Coverage::RetainedAncestor) snapshot.next.registryExclusions = {ancestor};
    Fake::State initial;
    initial.next.registryExclusions = snapshot.next.registryExclusions;
    auto backend = std::make_shared<Fake>(initial);
    core::PendingChanges changes;
    changes.setPersistTSCAL = true;
    if (coverage == Coverage::PlannedAncestor) changes.addRegistryExclusions = {ancestor};
    if (alternatives) changes.addRegistryExclusions = {root, ancestor};
    if (needsExclusion && exclusionFails) {
      const auto action = coverage == Coverage::ClearList ? compat::ExclusionAction::Remove : compat::ExclusionAction::Add;
      const auto key = coverage == Coverage::PlannedAncestor ? ancestor : root;
      backend->inject(fault(compat::EditRegistryExclusion{key, action}, Fake::FaultTiming::Before));
    }
    if (licenseFails) backend->inject(fault(compat::SetBoolean{compat::BooleanProperty::PersistTscal, true}, Fake::FaultTiming::Before));
    app::ConfigurationExecutor executor(app::uwfConfigurationRepository(backend, draft), draft->changes());
    const auto result = executor.apply(changes, snapshot);
    const bool pending = needsExclusion && exclusionFails && !alternatives;
    QVERIFY(result.staging());
    QCOMPARE(bool(result.staging()->failure), pending);
    QCOMPARE(bool(draft->changes()), pending);
    if (pending) QVERIFY(draft->changes()->persistencePending);
    QCOMPARE(store->writes, 1);
    QCOMPARE(store->entries, coverage == Coverage::ClearList ? QList<app::FileStagingEntry>{} : entries);
    QCOMPARE(result.failedOperations(), int(licenseFails) + int(needsExclusion && exclusionFails));
  }
  void separatePersistenceOperationsKeepTheirEffects_data() {
    QTest::addColumn<int>("selection");
    QTest::addColumn<Outcome>("outcome");
    for (int selection : {1, 2, 3})
      for (auto outcome : {Outcome::Success, Outcome::Before, Outcome::After})
        QTest::newRow(qPrintable(QStringLiteral("selection-%1-outcome-%2").arg(selection).arg(int(outcome)))) << selection << outcome;
  }
  void separatePersistenceOperationsKeepTheirEffects() {
    QFETCH(int, selection);
    QFETCH(Outcome, outcome);
    auto backend = std::make_shared<Fake>();
    core::PendingChanges changes;
    if (selection & 1) changes.setPersistDomainSecretKey = true;
    if (selection & 2) changes.setPersistTSCAL = true;
    std::vector<app::ConfigurationOperation> operations;
    for (const auto& [bit, property, kind] :
         {std::tuple{1, compat::BooleanProperty::PersistDomainSecretKey, app::ConfigurationOperationKind::DomainPersistence},
          std::tuple{2, compat::BooleanProperty::PersistTscal, app::ConfigurationOperationKind::LicensePersistence}}) {
      if (!(selection & bit)) continue;
      operations.push_back({kind, {}});
      if (outcome != Outcome::Success)
        backend->inject(fault(compat::SetBoolean{property, true}, outcome == Outcome::Before ? Fake::FaultTiming::Before : Fake::FaultTiming::AfterEffect));
    }
    clearLogLines();
    const auto result = app::ConfigurationExecutor(app::uwfConfigurationRepository(backend)).apply(changes, {});
    const auto effect = outcome == Outcome::Success  ? app::WriteEffect::Confirmed
                        : outcome == Outcome::Before ? app::WriteEffect::None
                                                     : app::WriteEffect::Uncertain;
    QCOMPARE(result.succeededOperations(), outcome == Outcome::Success ? int(operations.size()) : 0);
    QCOMPARE(result.failedOperations(), outcome == Outcome::Success ? 0 : int(operations.size()));
    QCOMPARE(result.steps.size(), operations.size());
    for (const auto& operation : operations) {
      QCOMPARE(std::ranges::count_if(result.steps, [&](const auto& item) { return item.operation == operation; }), std::ptrdiff_t{1});
      const auto& observed = step(result, operation);
      QVERIFY(!observed.description.empty());
      QCOMPARE(observed.writeEffect, effect);
      QCOMPARE(observed.outcome, outcome == Outcome::Success ? app::ConfigurationStepOutcome::Succeeded : app::ConfigurationStepOutcome::Failed);
    }
    QCOMPARE(result.anyWriteConfirmed(), outcome == Outcome::Success);
    QCOMPARE(result.reconciliationRequired(), outcome != Outcome::Before);
    const auto trace = backend->trace();
    QCOMPARE(trace.size(), operations.size());
    const auto logs = recentLogLines();
    QCOMPARE(std::ranges::count_if(logs, [](const auto& line) { return line.find(" E apply]") != std::string::npos; }),
             outcome == Outcome::Success ? std::ptrdiff_t{0} : static_cast<std::ptrdiff_t>(operations.size()));
  }
};
QTEST_APPLESS_MAIN(ConfigurationExecutionTests)
#include "ConfigurationExecutionTests.moc"
