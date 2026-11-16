// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QPointer>
#include <QSignalSpy>
#include <QTest>
#include <algorithm>
#include <array>
#include <set>
#include <stdexcept>
#include <utility>

#include "application/configuration/commands/UwfmgrCommandCodec.h"
#include "application/configuration/contracts/ConfigurationActions.h"
#include "application/configuration/editing/ConfigurationEditSession.h"
#include "application/configuration/editing/GlobalConfigurationEditor.h"
#include "application/configuration/volumes/VolumeConfigurationEditor.h"
#include "application/staging/contracts/FileStagingStore.h"
#include "application/staging/persistence/FileStagingRegistry.h"
#include "application/uwf/adapters/UwfReadState.h"
#include "application/uwf/contracts/UwfMaintenanceRepository.h"
#include "support/app/ConfigurationFixture.h"
#include "support/app/OverlayPolicyFixture.h"

namespace {
using namespace uwf;

QList<app::UwfmgrCommand> commands(const std::string& text) {
  QList<app::UwfmgrCommand> result;
  for (const auto& command : app::parseUwfmgrText(text)) result.append(command);
  return result;
}

std::shared_ptr<app::VolumeConfigurationEditor> addVolume(app::ConfigurationEditSession& session) {
  const core::VolumeInfo volume{"C:", uwf::test::application::fixtureVolumeCName, "NTFS", "System", 512ULL << 30, 256ULL << 30, core::VolumeSupport::Supported};
  auto editor = session.editVolume(volume, true);
  core::UwfSnapshot state;
  state.uwfAvailable = true;
  state.elevated = true;
  state.current.overlay = {core::OverlayType::RAM, 4096, 1024, 3072};
  state.current.volumes = {{uwf::test::application::fixtureVolumeCName, "C:", true, core::VolumeBinding::DriveLetter}};
  state.next = state.current;
  session.resetSnapshot(state);
  return editor;
}

class ConfigurationEditSessionTests final : public QObject {
  Q_OBJECT
 private slots:
  void systemVolumePersistenceKeepsMissingMountInformationUnknown_data() {
    QTest::addColumn<int>("mapping");
    QTest::addColumn<bool>("ordinaryAllowed");
    QTest::newRow("system-mount-not-observed") << 0 << false;
    QTest::newRow("matching-system-mount-protected") << 1 << false;
    QTest::newRow("matching-system-mount-unprotected") << 2 << true;
    QTest::newRow("authoritative-unprotected-without-mount") << 3 << true;
  }
  void systemVolumePersistenceKeepsMissingMountInformationUnknown() {
    QFETCH(int, mapping);
    QFETCH(bool, ordinaryAllowed);
    app::ConfigurationEditSession session(16384, QStringLiteral("C:"));
    const core::VolumeInfo data{"D:", test::application::fixtureVolumeDName, "NTFS", "Data", 1000, 500, core::VolumeSupport::Supported, {true, true}};
    const auto volume = session.editVolume(data, false);
    auto state = test::application::editableSnapshot();
    state.current.filter.enabled = true;
    state.runtime.hormEnabled = false;
    state.runtime.currentOverlayFlags = 4;
    state.runtime.romDisposition = 0;
    state.runtime.fullCommit = false;
    state.runtime.systemVolumeProtected = mapping == 3 ? std::optional{false} : std::nullopt;
    state.current.volumes = {{data.volumeName, "D:", true, core::VolumeBinding::VolumeId, std::string("D:")}};
    if (mapping == 1 || mapping == 2)
      state.current.volumes.push_back({test::application::fixtureVolumeCName, "C:", mapping == 1, core::VolumeBinding::VolumeId, std::string("C:")});
    state.next = state.current;
    session.resetSnapshot(state);
    QCOMPARE(volume->protectionEditingAllowed(), ordinaryAllowed);
    QCOMPARE(volume->files()->readOnly(), !ordinaryAllowed);
    QCOMPARE(session.registryEditor()->readOnly(), !ordinaryAllowed);
    QCOMPARE(session.globalEditor()->thresholdEditingAllowed(), ordinaryAllowed);
    QVERIFY(session.globalEditor()->controlsEnabled());
    QVERIFY(session.pendingConfiguration().empty());
  }
  void fileAliasesDispatchOneExactProviderPattern() {
    app::ConfigurationEditSession session;
    const core::VolumeInfo physical{"C:", test::application::fixtureVolumeCName, "NTFS", "System", 1000, 500, core::VolumeSupport::Supported};
    const auto editor = session.editVolume(physical, false);
    session.resetSnapshot(test::application::editableSnapshot(), {}, false);
    const auto files = editor->files();
    const auto id = compat::normalizeVolumeId(physical.volumeName);
    const auto guid = app::paths::volumeFilePath({id, QStringLiteral("\\Data\\state.bin")});
    files->setBaseline({}, {QStringLiteral("C:\\Data\\state.bin"), guid});
    QVERIFY(files->removeSelection({{guid, {}}}));
    const auto pending = session.pendingConfiguration().settings;
    QCOMPARE(pending.removeFileExclusions.at(id), std::vector<std::string>{"C:\\Data\\state.bin"});

    // The provider can store an address-looking string as a literal pattern.
    // Its application shell is stripped once; the raw marker survives dispatch.
    files->setProviderBaseline({}, {guid});
    const auto wrapped = QStringLiteral("C:") + guid;
    QVERIFY(files->removeSelection({{wrapped, {}}}));
    QCOMPARE(session.pendingConfiguration().settings.removeFileExclusions.at(id), std::vector<std::string>{wrapped.toStdString()});
    QCOMPARE(app::toUwfFilePath(wrapped.toStdString(), "C:"), guid.toStdString());

    const auto foreign = app::paths::volumeFilePath({compat::normalizeVolumeId(test::application::fixtureVolumeDName), QStringLiteral("\\Data\\state.bin")});
    for (const auto& unresolved : {foreign, QStringLiteral("Z:\\Data\\state.bin")}) {
      files->setBaseline({}, {unresolved});
      QVERIFY(files->removeSelection({{unresolved, {}}}));
      QVERIFY_THROWS_EXCEPTION(std::invalid_argument, (void)session.pendingConfiguration());
    }
  }
  void bulkProtectionAndLaterPerVolumeRequestsShareTheFinalTarget_data() {
    QTest::addColumn<bool>("initialProtected");
    QTest::addColumn<bool>("repeatBulk");
    for (const bool initial : {false, true})
      for (const bool repeat : {false, true})
        QTest::newRow(qUtf8Printable(QStringLiteral("initial-%1/repeat-%2").arg(initial).arg(repeat))) << initial << repeat;
  }
  void bulkProtectionAndLaterPerVolumeRequestsShareTheFinalTarget() {
    QFETCH(bool, initialProtected);
    QFETCH(bool, repeatBulk);
    app::ConfigurationEditSession session;
    auto snapshot = test::application::editableSnapshot();
    snapshot.next.volumes.front().isProtected = initialProtected;
    auto volume = core::VolumeInfo{"C:", test::application::fixtureVolumeCName, "NTFS", "System", 1000, 500, core::VolumeSupport::Supported, {true, true}};
    const auto editor = session.editVolume(volume, false);
    session.resetSnapshot(snapshot, {}, false);
    const auto maintenance = session.maintenanceEditor();
    app::OverlayMaintenanceSnapshot observed;
    observed.hormEnabled = observed.filterEnabled = observed.overlayCommit = false;
    observed.protectScopeConfirmed = observed.unprotectScopeConfirmed = true;
    observed.protectTargetVolumes = observed.unprotectTargetVolumes = {QString::fromStdString(volume.volumeName)};
    const auto ticket = maintenance->beginObservation();
    QVERIFY(ticket);
    maintenance->finishObservation(*ticket, observed);
    maintenance->setInteractionContext(true, false);
    QVERIFY(maintenance->stageAllVolumesProtection(!initialProtected));
    QCOMPARE(editor->status()->draft().protectedVolume, !initialProtected);
    const auto command = initialProtected ? "uwfmgr volume protect C:" : "uwfmgr volume unprotect C:";
    const auto report = session.importCommands(commands(command));
    QCOMPARE(report.front().status, app::ImportResult::Status::Success);
    QCOMPARE(editor->status()->draft().protectedVolume, initialProtected);
    QCOMPARE(session.pendingConfiguration().settings.volumeProtect.at(compat::normalizeVolumeId(test::application::fixtureVolumeCName)), initialProtected);
    if (repeatBulk) {
      if (initialProtected)
        QVERIFY(maintenance->stageAllVolumesProtection(false));
      else {
        const auto repeated = session.importCommands(commands("uwfmgr volume protect all"));
        QCOMPARE(repeated.front().status, app::ImportResult::Status::Success);
      }
      QCOMPARE(editor->status()->draft().protectedVolume, !initialProtected);
      QVERIFY(session.pendingConfiguration().settings.volumeProtect.empty());
    }
  }
  void bulkProtectionEstablishesNewVolumeExclusionAndBindingTargets() {
    app::ConfigurationEditSession session;
    const core::VolumeInfo volume{"C:", test::application::fixtureVolumeCName, "NTFS", "System", 1000, 500, core::VolumeSupport::Supported, {true, true}};
    const auto editor = session.editVolume(volume, false);
    auto snapshot = test::application::editableSnapshot();
    snapshot.current.volumes.clear();
    snapshot.next.volumes.clear();
    session.resetSnapshot(snapshot, {}, false);
    const auto maintenance = session.maintenanceEditor();
    app::OverlayMaintenanceSnapshot observed;
    observed.hormEnabled = observed.filterEnabled = observed.overlayCommit = false;
    observed.protectScopeConfirmed = true;
    observed.protectTargetVolumes = {QString::fromStdString(volume.volumeName)};
    const auto ticket = maintenance->beginObservation();
    QVERIFY(ticket);
    maintenance->finishObservation(*ticket, observed);
    maintenance->setInteractionContext(true, false);
    const auto report = session.importCommands(commands("uwfmgr volume protect all && uwfmgr file add-exclusion C:\\Data"));
    QCOMPARE(report.size(), 2);
    for (const auto& row : report) QCOMPARE(row.status, app::ImportResult::Status::Success);
    QVERIFY(editor->files()->canAdd());
    QVERIFY(editor->status()->bindingEnabled());
    QCOMPARE(editor->status()->draft().binding, core::VolumeBinding::VolumeId);
    QVERIFY(!editor->status()->pendingBinding());
    QVERIFY(editor->status()->editBinding(core::VolumeBinding::DriveLetter));
    const auto changes = session.pendingConfiguration();
    QCOMPARE(changes.settings.volumeBindings.at("C:"), core::VolumeBinding::DriveLetter);
    QCOMPARE(changes.settings.addFileExclusions.at("C:"), std::vector<std::string>{"C:\\Data"});
    QVERIFY(changes.settings.volumeProtect.empty());
  }
  void maintenanceAndOrdinaryDraftsSharePendingImportAndReset() {
    app::ConfigurationEditSession session;
    const auto snapshot = test::application::editableSnapshot();
    session.resetSnapshot(snapshot, {}, false);
    const auto maintenance = session.maintenanceEditor();
    app::OverlayMaintenanceSnapshot observed;
    observed.filterEnabled = observed.hormEnabled = observed.overlayCommit = false;
    const auto ticket = maintenance->beginObservation();
    QVERIFY(ticket);
    maintenance->finishObservation(*ticket, observed);
    maintenance->setInteractionContext(true, false);
    const auto report = session.importCommands(commands("uwfmgr filter disable\nuwfmgr volume protect all"));
    QCOMPARE(report.size(), 2);
    for (const auto& item : report) QCOMPARE(item.status, app::ImportResult::Status::Success);
    auto changes = session.pendingConfiguration();
    QCOMPARE(changes.settings.setFilterEnabled, std::optional<bool>{false});
    QCOMPARE(changes.maintenance.allVolumesProtected, std::optional<bool>{true});
    QCOMPARE(changes.count(), std::size_t{2});
    session.resetSnapshot(snapshot, {}, false);
    QVERIFY(session.pendingConfiguration().settings.empty());
    QCOMPARE(session.pendingConfiguration().maintenance.allVolumesProtected, std::optional<bool>{true});
    session.globalEditor()->editFilterEnabled(false);
    QCOMPARE(session.pendingConfiguration().count(), std::size_t{2});
    bool globalNotified = false, maintenanceNotified = false;
    connect(session.globalEditor().get(), &app::GlobalConfigurationEditor::stateChanged, this, [&] {
      globalNotified = true;
      QVERIFY(session.pendingConfiguration().empty());
    });
    connect(maintenance.get(), &app::OverlayMaintenanceEditor::pendingChanged, this, [&] {
      maintenanceNotified = true;
      QVERIFY(session.pendingConfiguration().empty());
    });
    session.discardChanges(snapshot, {}, false);
    QVERIFY(globalNotified && maintenanceNotified);
    QVERIFY(session.pendingConfiguration().empty());
    QCOMPARE(maintenance->snapshot().hormEnabled, std::optional<bool>{false});
  }
  void maintenanceNestedEditsMergeOnlyIntoTheirParentAndDetectConflicts() {
    app::ConfigurationEditSession session;
    const auto snapshot = test::application::editableSnapshot();
    session.resetSnapshot(snapshot, {}, false);
    const auto root = session.maintenanceEditor();
    app::OverlayMaintenanceSnapshot observed;
    observed.filterEnabled = observed.hormEnabled = observed.overlayCommit = false;
    const auto ticket = root->beginObservation();
    QVERIFY(ticket);
    root->finishObservation(*ticket, observed);
    root->setInteractionContext(true, false);
    const auto parent = app::OverlayMaintenanceEditor::editDraft(root);
    const auto child = app::OverlayMaintenanceEditor::editDraft(parent);
    QVERIFY(child->stageAllVolumesProtection(true));
    QVERIFY(child->commitDraft());
    QCOMPARE(parent->draft().allVolumesProtected, std::optional<bool>{true});
    QVERIFY(session.pendingConfiguration().empty());
    QVERIFY(root->stageAllVolumesProtection(false));
    session.resetSnapshot(snapshot, {}, false);
    QCOMPARE(session.pendingConfiguration().maintenance.allVolumesProtected, std::optional<bool>{false});
    QVERIFY(parent->draftConflicted());
    QVERIFY(!parent->commitDraft());
    QCOMPARE(session.pendingConfiguration().maintenance.allVolumesProtected, std::optional<bool>{false});
    parent->discardDraft();
    QVERIFY(!parent->draftConflicted());
    QVERIFY(parent->commitDraft());
    QVERIFY(session.pendingConfiguration().empty());
    QVERIFY(root->stageAllVolumesProtection(false));
    session.discardChanges(snapshot, {}, false);
    QVERIFY(session.pendingConfiguration().empty());
  }
  void maintenanceReceiptsRequireSuccessfulReadbackAndSupersedeOlderReads_data() {
    QTest::addColumn<bool>("succeeded");
    QTest::addColumn<bool>("confirmed");
    for (const bool succeeded : {false, true})
      for (const bool confirmed : {false, true})
        QTest::newRow(qUtf8Printable(QStringLiteral("success-%1/readback-%2").arg(succeeded).arg(confirmed))) << succeeded << confirmed;
  }
  void maintenanceReceiptsRequireSuccessfulReadbackAndSupersedeOlderReads() {
    QFETCH(bool, succeeded);
    QFETCH(bool, confirmed);
    app::ConfigurationEditSession session;
    const auto editor = session.maintenanceEditor();
    app::OverlayMaintenanceSnapshot observed;
    observed.filterEnabled = observed.hormEnabled = observed.overlayCommit = false;
    observed.allVolumesNextProtected = false;
    const auto ticket = editor->beginObservation();
    QVERIFY(ticket);
    editor->finishObservation(*ticket, observed);
    editor->setInteractionContext(true, false);
    QVERIFY(editor->stageAllVolumesProtection(true));
    const auto requested = session.pendingConfiguration();
    const auto oldRead = editor->beginObservation();
    QVERIFY(oldRead);
    app::ConfigurationApplyResult result;
    result.observedOverlayMaintenance = observed;
    result.observedOverlayMaintenance->allVolumesNextProtected = confirmed;
    result.steps.push_back({"Protect all", succeeded ? app::ConfigurationStepOutcome::Succeeded : app::ConfigurationStepOutcome::Failed,
                            succeeded ? app::WriteEffect::Confirmed : app::WriteEffect::None,
                            app::ConfigurationOperation{app::ConfigurationOperationKind::ProtectAllVolumes, {}}});
    session.reconcileApplied(requested, result);
    QCOMPARE(editor->busy(), false);
    QCOMPARE(session.pendingConfiguration().maintenance.empty(), succeeded && confirmed);
    session.resetSnapshot(test::application::editableSnapshot(), {}, false);
    QCOMPARE(session.pendingConfiguration().maintenance.empty(), succeeded && confirmed);
    editor->finishObservation(*oldRead, observed);
    QCOMPARE(editor->snapshot().allVolumesNextProtected, std::optional<bool>{confirmed});
    QCOMPARE(session.pendingConfiguration().maintenance.empty(), succeeded && confirmed);
  }
  void partialBulkApplyRebasesTheUnfinishedVolumeException_data() {
    QTest::addColumn<bool>("bulkSucceeded");
    QTest::addColumn<bool>("discard");
    for (const bool bulk : {false, true})
      for (const bool discard : {false, true}) QTest::newRow(qUtf8Printable(QStringLiteral("bulk-%1/discard-%2").arg(bulk).arg(discard))) << bulk << discard;
  }
  void partialBulkApplyRebasesTheUnfinishedVolumeException() {
    QFETCH(bool, bulkSucceeded);
    QFETCH(bool, discard);
    app::ConfigurationEditSession session;
    auto snapshot = test::application::editableSnapshot();
    snapshot.current.filter.enabled = snapshot.next.filter.enabled = false;
    snapshot.current.volumes.front().isProtected = snapshot.next.volumes.front().isProtected = false;
    const core::VolumeInfo physical{"C:", test::application::fixtureVolumeCName, "NTFS", "System", 1000, 500, core::VolumeSupport::Supported, {true, true}};
    const auto id = compat::normalizeVolumeId(physical.volumeName);
    session.editVolume(physical, true);
    session.resetSnapshot(snapshot, {}, false);
    const auto maintenance = session.maintenanceEditor();
    app::OverlayMaintenanceSnapshot observed;
    observed.filterEnabled = observed.hormEnabled = observed.overlayCommit = false;
    observed.allVolumesNextProtected = false;
    observed.protectScopeConfirmed = true;
    observed.protectTargetVolumes = {QString::fromStdString(test::application::fixtureVolumeCName),
                                     QString::fromStdString(test::application::fixtureVolumeDName)};
    const auto ticket = maintenance->beginObservation();
    QVERIFY(ticket);
    maintenance->finishObservation(*ticket, observed);
    maintenance->setInteractionContext(true, false);
    const auto report = session.importCommands(commands("uwfmgr volume protect all && uwfmgr volume unprotect C:"));
    QCOMPARE(report.size(), 2);
    for (const auto& row : report) QCOMPARE(row.status, app::ImportResult::Status::Success);
    const auto requested = session.pendingConfiguration();
    QCOMPARE(requested.count(), std::size_t{2});
    QCOMPARE(requested.settings.volumeProtect.at(id), false);
    app::ConfigurationApplyResult result;
    result.observedOverlayMaintenance = observed;
    result.observedOverlayMaintenance->allVolumesNextProtected = bulkSucceeded;
    result.steps.push_back({"Protect all", bulkSucceeded ? app::ConfigurationStepOutcome::Succeeded : app::ConfigurationStepOutcome::Failed,
                            app::WriteEffect::Confirmed, app::ConfigurationOperation{app::ConfigurationOperationKind::ProtectAllVolumes, {}}});
    result.steps.push_back({"Unprotect C", bulkSucceeded ? app::ConfigurationStepOutcome::Failed : app::ConfigurationStepOutcome::NotAttempted,
                            app::WriteEffect::None, app::ConfigurationOperation{app::ConfigurationOperationKind::VolumeProtection, id}});
    session.reconcileApplied(requested, result);
    const auto verifyRemaining = [&] {
      QCOMPARE(session.pendingChanges().volumeProtect.at(id), false);
      QCOMPARE(session.pendingConfiguration().maintenance.allVolumesProtected, bulkSucceeded ? std::nullopt : std::optional{true});
      QCOMPARE(session.pendingConfiguration().count(), bulkSucceeded ? std::size_t{1} : std::size_t{2});
    };
    verifyRemaining();
    // A failed read never installs a new snapshot. Replacing presentation
    // editors alone must not consume the outstanding operation targets.
    session.clearVolumes();
    verifyRemaining();
    session.editVolume(physical, true);
    snapshot.next.volumes.front().isProtected = true;
    snapshot.next.volumes.back().isProtected = bulkSucceeded;
    if (discard) {
      session.discardChanges(snapshot, {}, false);
      QVERIFY(session.pendingConfiguration().empty());
      session.resetSnapshot(snapshot, {}, false);
      QVERIFY(session.pendingConfiguration().empty());
      return;
    }
    session.resetSnapshot(snapshot, {}, false);
    verifyRemaining();
    const auto retry = session.pendingConfiguration();
    app::ConfigurationApplyResult completed;
    completed.observedOverlayMaintenance = *result.observedOverlayMaintenance;
    completed.observedOverlayMaintenance->allVolumesNextProtected = false;
    if (!bulkSucceeded) {
      app::ConfigurationStepResult bulk{"Protect all", app::ConfigurationStepOutcome::Succeeded, app::WriteEffect::Confirmed,
                                        app::ConfigurationOperation{app::ConfigurationOperationKind::ProtectAllVolumes, {}}};
      bulk.maintenance = app::ConfigurationMaintenanceDetail{*completed.observedOverlayMaintenance, {}};
      bulk.maintenance->snapshot.allVolumesNextProtected = true;
      completed.steps.push_back(std::move(bulk));
    }
    completed.steps.push_back({"Unprotect C", app::ConfigurationStepOutcome::Succeeded, app::WriteEffect::Confirmed,
                               app::ConfigurationOperation{app::ConfigurationOperationKind::VolumeProtection, id}});
    session.reconcileApplied(retry, completed);
    QVERIFY(session.pendingConfiguration().empty());
    snapshot.next.volumes.front().isProtected = false;
    snapshot.next.volumes.back().isProtected = true;
    session.resetSnapshot(snapshot, {}, false);
    QVERIFY(session.pendingConfiguration().empty());
  }
  void postWriteObservationRebasesEveryOrdinaryPartitionByVolumeIdentity_data() {
    QTest::addColumn<QString>("mountedDrive");
    QTest::newRow("same-volume-remounted-at-E") << QStringLiteral("E:");
    QTest::newRow("configured-volume-without-a-page") << QString{};
  }
  void postWriteObservationRebasesEveryOrdinaryPartitionByVolumeIdentity() {
    QFETCH(QString, mountedDrive);
    app::ConfigurationEditSession session(16384);
    auto snapshot = test::application::editableSnapshot();
    snapshot.current.filter.enabled = false;
    const core::VolumeInfo physical{"C:", test::application::fixtureVolumeCName, "NTFS", "System", 1000, 500, core::VolumeSupport::Supported, {true, true}};
    const auto id = compat::normalizeVolumeId(physical.volumeName);
    const auto editor = session.editVolume(physical, true);
    session.resetSnapshot(snapshot, {}, false);
    const auto report =
        session.importCommands(commands("uwfmgr filter disable\nuwfmgr overlay set-type RAM\nuwfmgr overlay set-size 4096\n"
                                        "uwfmgr overlay set-warningthreshold 1000\nuwfmgr overlay set-criticalthreshold 3000\n"
                                        "uwfmgr volume unprotect C:\nuwfmgr file add-exclusion C:\\New\nuwfmgr file remove-exclusion C:\\Existing\n"
                                        "uwfmgr registry add-exclusion HKEY_LOCAL_MACHINE\\SOFTWARE\\New\n"
                                        "uwfmgr registry remove-exclusion HKEY_LOCAL_MACHINE\\SOFTWARE\\Existing"));
    QCOMPARE(report.size(), 10);
    for (const auto& row : report) QCOMPARE(row.status, app::ImportResult::Status::Success);
    QVERIFY(editor->status()->editBinding(core::VolumeBinding::VolumeId));
    session.registryEditor()->enablePersistence(app::ExclusionEditor::PersistenceItem::DomainSecretKey);
    session.registryEditor()->enablePersistence(app::ExclusionEditor::PersistenceItem::Tscal);
    const auto requested = session.pendingConfiguration();
    QCOMPARE(requested.count(), std::size_t{13});
    app::ConfigurationApplyResult result;
    result.steps.push_back({"Filter", app::ConfigurationStepOutcome::Succeeded, app::WriteEffect::Confirmed,
                            app::ConfigurationOperation{app::ConfigurationOperationKind::Filter, {}}});
    result.steps.push_back({"Overlay type", app::ConfigurationStepOutcome::Failed, app::WriteEffect::None,
                            app::ConfigurationOperation{app::ConfigurationOperationKind::OverlayType, {}}});
    session.reconcileApplied(requested, result);
    QCOMPARE(session.pendingConfiguration().count(), std::size_t{12});
    QVERIFY(!session.pendingChanges().setFilterEnabled);
    session.clearVolumes();
    QCOMPARE(session.pendingConfiguration().count(), std::size_t{12});
    if (!mountedDrive.isEmpty()) {
      auto moved = physical;
      moved.driveLetter = mountedDrive.toStdString();
      session.editVolume(moved, true);
    }
    snapshot.next.filter.enabled = false;
    snapshot.current.volumes.front().physicalDriveLetter = snapshot.next.volumes.front().physicalDriveLetter = mountedDrive.toStdString();
    session.setEditingAllowed(false);
    session.resetSnapshot(snapshot, {}, false);
    const auto remaining = session.pendingChanges();
    QCOMPARE(session.pendingConfiguration().count(), std::size_t{12});
    QVERIFY(!remaining.setFilterEnabled);
    QCOMPARE(remaining.setOverlay.type, std::optional{core::OverlayType::RAM});
    QCOMPARE(remaining.setOverlay.maximumSizeMb, std::optional<std::uint32_t>{4096});
    QCOMPARE(remaining.setOverlay.warningThresholdMb, std::optional<std::uint32_t>{1000});
    QCOMPARE(remaining.setOverlay.criticalThresholdMb, std::optional<std::uint32_t>{3000});
    QCOMPARE(remaining.volumeProtect.at(id), false);
    QCOMPARE(remaining.volumeBindings.at(id), core::VolumeBinding::VolumeId);
    const auto verifyPattern = [&](const std::vector<std::string>& paths, const QString& expected) {
      QCOMPARE(paths.size(), std::size_t{1});
      app::paths::VolumeIdentities mounts;
      if (!mountedDrive.isEmpty()) mounts.emplace(mountedDrive, id);
      const auto address = app::paths::resolveVolumeFilePath(QString::fromStdString(paths.front()), mounts, id);
      QVERIFY(address);
      QCOMPARE(address->volume, id);
      QCOMPARE(address->relativePath, expected);
    };
    verifyPattern(remaining.addFileExclusions.at(id), QStringLiteral("\\New"));
    verifyPattern(remaining.removeFileExclusions.at(id), QStringLiteral("\\Existing"));
    QCOMPARE(remaining.addRegistryExclusions, std::vector<std::string>{"HKEY_LOCAL_MACHINE\\SOFTWARE\\New"});
    QCOMPARE(remaining.removeRegistryExclusions, std::vector<std::string>{"HKEY_LOCAL_MACHINE\\SOFTWARE\\Existing"});
    QCOMPARE(remaining.setPersistDomainSecretKey, std::optional{true});
    QCOMPARE(remaining.setPersistTSCAL, std::optional{true});
    session.discardChanges(snapshot, {}, false);
    QVERIFY(session.pendingConfiguration().empty());
  }
  void postWriteObservationKeepsOverlayIntentWithoutReenteringEditAdmission_data() {
    QTest::addColumn<int>("field");
    for (const auto [name, field] :
         {std::pair{"passthrough", app::OverlayPolicyField::Passthrough}, std::pair{"persistent", app::OverlayPolicyField::Persistent},
          std::pair{"ROM", app::OverlayPolicyField::ReadOnlyMedia}, std::pair{"reset", app::OverlayPolicyField::Reset}})
      QTest::newRow(name) << static_cast<int>(field);
  }
  void postWriteObservationKeepsOverlayIntentWithoutReenteringEditAdmission() {
    QFETCH(int, field);
    const auto option = static_cast<app::OverlayPolicyField>(field);
    app::ConfigurationEditSession session(16384);
    auto snapshot = test::application::editableSnapshot();
    auto policy = test::application::diskPersistentPolicy();
    const bool value = option == app::OverlayPolicyField::ReadOnlyMedia || option == app::OverlayPolicyField::Reset;
    if (option == app::OverlayPolicyField::ReadOnlyMedia) {
      snapshot.current.filter.enabled = false;
      snapshot.next.overlay.type = core::OverlayType::RAM;
      policy.current.filterEnabled.value = false;
      policy.current.overlayType.value = policy.next.overlayType.value = 0;
      policy.current.flags.value = policy.next.flags.value = app::OverlayFlags{0};
    }
    session.resetSnapshot(snapshot, policy, false);
    app::OverlayPolicyDelta delta;
    app::OverlayPolicyEditor::setValue(delta, option, value);
    QVERIFY(session.overlayPolicyEditor()->importChanges(delta));
    session.registryEditor()->enablePersistence(app::ExclusionEditor::PersistenceItem::DomainSecretKey);
    const auto requested = session.pendingConfiguration();
    QCOMPARE(requested.count(), std::size_t{2});
    app::ConfigurationApplyResult result;
    result.steps.push_back({"Domain persistence", app::ConfigurationStepOutcome::Succeeded, app::WriteEffect::Confirmed,
                            app::ConfigurationOperation{app::ConfigurationOperationKind::DomainPersistence, {}}});
    session.reconcileApplied(requested, result);
    QCOMPARE(app::OverlayPolicyEditor::value(session.pendingConfiguration().overlayPolicy, option), std::optional{value});
    snapshot.next.persistDomainSecretKey = true;
    session.setEditingAllowed(false);
    session.resetSnapshot(snapshot, policy, false);
    const auto remaining = session.pendingConfiguration();
    QVERIFY(remaining.settings.empty());
    QCOMPARE(remaining.count(), std::size_t{1});
    QCOMPARE(app::OverlayPolicyEditor::value(remaining.overlayPolicy, option), std::optional{value});
    QVERIFY(!session.overlayPolicyEditor()->editingAllowed());
    session.discardChanges(snapshot, policy, false);
    QVERIFY(session.pendingConfiguration().empty());
  }
  void recoveryUsesTheNewBaselineAndNoEffectFailuresLeaveEditingLive() {
    app::ConfigurationEditSession session(16384);
    auto snapshot = test::application::editableSnapshot();
    snapshot.current.filter.enabled = false;
    session.resetSnapshot(snapshot, {}, false);
    session.globalEditor()->editFilterEnabled(false);
    session.globalEditor()->editWarningThreshold(1000);
    const auto requested = session.pendingConfiguration();
    QCOMPARE(requested.count(), std::size_t{2});
    app::ConfigurationApplyResult result;
    result.steps.push_back({"Filter", app::ConfigurationStepOutcome::Failed, app::WriteEffect::Uncertain,
                            app::ConfigurationOperation{app::ConfigurationOperationKind::Filter, {}}});
    result.steps.push_back({"Warning threshold", app::ConfigurationStepOutcome::Succeeded, app::WriteEffect::Confirmed,
                            app::ConfigurationOperation{app::ConfigurationOperationKind::WarningThreshold, {}}});
    session.reconcileApplied(requested, result);
    QCOMPARE(session.pendingChanges().setFilterEnabled, std::optional{false});
    QVERIFY(session.pendingChanges().setOverlay.empty());
    snapshot.next.filter.enabled = false;
    snapshot.next.overlay.warningThresholdMb = 1000;
    session.resetSnapshot(snapshot, {}, false);
    QVERIFY(session.pendingConfiguration().empty());
    session.globalEditor()->editWarningThreshold(1500);
    const auto next = session.pendingConfiguration();
    app::ConfigurationApplyResult noEffect;
    noEffect.steps.push_back({"Warning threshold", app::ConfigurationStepOutcome::Failed, app::WriteEffect::None,
                              app::ConfigurationOperation{app::ConfigurationOperationKind::WarningThreshold, {}}});
    session.reconcileApplied(next, noEffect);
    session.globalEditor()->editWarningThreshold(2000);
    QCOMPARE(session.pendingChanges().setOverlay.warningThresholdMb, std::optional<std::uint32_t>{2000});
    session.resetSnapshot(snapshot, {}, false);
    QVERIFY(session.pendingConfiguration().empty());
  }
  void pendingNewRegistrationKeepsItsReviewedIdentityAfterTheDiskDisappears() {
    app::ConfigurationEditSession session;
    auto snapshot = test::application::editableSnapshot();
    snapshot.current.filter.enabled = snapshot.next.filter.enabled = false;
    const auto id = compat::normalizeVolumeId(test::application::fixtureVolumeDName);
    std::erase_if(snapshot.next.volumes, [&](const auto& volume) { return compat::normalizeVolumeId(volume.volumeName) == id; });
    const core::VolumeInfo physical{"D:", test::application::fixtureVolumeDName, "NTFS", "Data", 1000, 500, core::VolumeSupport::Supported, {true, true}};
    const auto editor = session.editVolume(physical, false);
    session.resetSnapshot(snapshot, {}, false);
    QVERIFY(!editor->status()->draft().hasConfiguration);
    const auto report = session.importCommands(commands("uwfmgr volume protect D:"));
    QCOMPARE(report.front().status, app::ImportResult::Status::Success);
    const auto requested = session.pendingConfiguration();
    QCOMPARE(requested.settings.volumeProtect.at("D:"), true);
    QVERIFY(requested.settings.volumeBindings.empty());
    app::ConfigurationApplyResult result;
    result.steps.push_back({"Protect D", app::ConfigurationStepOutcome::Failed, app::WriteEffect::Uncertain,
                            app::ConfigurationOperation{app::ConfigurationOperationKind::VolumeProtection, "D:"}});
    session.reconcileApplied(requested, result);
    session.clearVolumes();
    std::erase_if(snapshot.current.volumes, [&](const auto& volume) { return compat::normalizeVolumeId(volume.volumeName) == id; });
    session.resetSnapshot(snapshot, {}, false);
    const auto remaining = session.pendingChanges();
    QCOMPARE(remaining.volumeProtect.at(id), true);
    // The original drive-addressed registration selected drive-letter binding.
    // Stabilizing its target as a GUID must not silently change that selection.
    QCOMPARE(remaining.volumeBindings.at(id), core::VolumeBinding::DriveLetter);
    QCOMPARE(session.pendingConfiguration().count(), std::size_t{2});
    session.discardChanges(snapshot, {}, false);
    QVERIFY(session.pendingConfiguration().empty());
  }
  void mixedExclusionBatchConsumesOnlySuccessfulReceiptsAcrossRebase() {
    constexpr int entriesPerKind = 250;
    app::ConfigurationEditSession session;
    auto snapshot = test::application::editableSnapshot();
    snapshot.current.filter.enabled = false;
    const core::VolumeInfo physical{"C:", test::application::fixtureVolumeCName, "NTFS", "System", 1000, 500, core::VolumeSupport::Supported, {true, true}};
    const auto id = compat::normalizeVolumeId(physical.volumeName);
    auto& savedFiles = snapshot.next.fileExclusions[physical.volumeName];
    auto& savedRegistry = snapshot.next.registryExclusions;
    savedFiles.clear();
    savedRegistry.clear();
    QStringList input;
    for (int index = 0; index < entriesPerKind; ++index) {
      const auto suffix = QStringLiteral("%1").arg(index, 4, 10, QLatin1Char('0'));
      const auto added = QStringLiteral("\\Added%1").arg(suffix);
      const auto saved = QStringLiteral("\\Saved%1").arg(suffix);
      const auto addedKey = QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Added%1").arg(suffix);
      const auto savedKey = QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Saved%1").arg(suffix);
      savedFiles.push_back(saved.toStdString());
      savedRegistry.push_back(savedKey.toStdString());
      input.append(QStringLiteral("uwfmgr file add-exclusion C:%1").arg(added));
      input.append(QStringLiteral("uwfmgr file remove-exclusion C:%1").arg(saved));
      input.append(QStringLiteral("uwfmgr registry add-exclusion %1").arg(addedKey));
      input.append(QStringLiteral("uwfmgr registry remove-exclusion %1").arg(savedKey));
    }
    session.editVolume(physical, true);
    session.resetSnapshot(snapshot, {}, false);
    const auto report = session.importCommands(commands(input.join(u'\n').toStdString()));
    QCOMPARE(report.size(), entriesPerKind * 4);
    for (const auto& row : report) QCOMPARE(row.status, app::ImportResult::Status::Success);
    const auto requested = session.pendingConfiguration();
    QCOMPARE(requested.count(), std::size_t{1000});
    auto observed = snapshot;
    app::ConfigurationApplyResult result;
    std::array<std::set<QString>, 4> expected;
    const std::array outcomes{app::ConfigurationStepOutcome::Succeeded, app::ConfigurationStepOutcome::Failed, app::ConfigurationStepOutcome::NotAttempted,
                              app::ConfigurationStepOutcome::DependencySkipped};
    for (int index = 0; index < entriesPerKind; ++index) {
      const auto suffix = QStringLiteral("%1").arg(index, 4, 10, QLatin1Char('0'));
      const std::array paths{QStringLiteral("\\Added%1").arg(suffix), QStringLiteral("\\Saved%1").arg(suffix),
                             QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Added%1").arg(suffix),
                             QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Saved%1").arg(suffix)};
      const std::array kinds{app::ConfigurationOperationKind::AddFileExclusion, app::ConfigurationOperationKind::RemoveFileExclusion,
                             app::ConfigurationOperationKind::AddRegistryExclusion, app::ConfigurationOperationKind::RemoveRegistryExclusion};
      for (std::size_t kind = 0; kind < paths.size(); ++kind) {
        const auto outcome = outcomes[(static_cast<std::size_t>(index) + kind) % outcomes.size()];
        const auto target = kind < 2 ? id + "C:" + paths[kind].toStdString() : paths[kind].toStdString();
        result.steps.push_back({"Exclusion", outcome,
                                outcome == app::ConfigurationStepOutcome::Succeeded ? app::WriteEffect::Confirmed : app::WriteEffect::None,
                                app::ConfigurationOperation{kinds[kind], target}});
        if (outcome != app::ConfigurationStepOutcome::Succeeded) {
          expected[kind].insert(paths[kind]);
          continue;
        }
        if (kind == 0)
          observed.next.fileExclusions[physical.volumeName].push_back(paths[kind].toStdString());
        else if (kind == 1)
          std::erase(observed.next.fileExclusions[physical.volumeName], paths[kind].toStdString());
        else if (kind == 2)
          observed.next.registryExclusions.push_back(paths[kind].toStdString());
        else
          std::erase(observed.next.registryExclusions, paths[kind].toStdString());
      }
    }
    const auto filePatterns = [&](const std::vector<std::string>& paths) {
      std::set<QString> patterns;
      const app::paths::VolumeIdentities mounts{{QStringLiteral("C:"), id}};
      for (const auto& path : paths) {
        const auto address = app::paths::resolveVolumeFilePath(QString::fromStdString(path), mounts, id);
        patterns.insert(address && address->volume == id ? address->relativePath : QStringLiteral("unresolved:") + QString::fromStdString(path));
      }
      return patterns;
    };
    const auto registryPaths = [](const std::vector<std::string>& paths) {
      std::set<QString> result;
      for (const auto& path : paths) result.insert(QString::fromStdString(path));
      return result;
    };
    const auto verifyRemaining = [&] {
      const auto remaining = session.pendingChanges();
      QCOMPARE(session.pendingConfiguration().count(), std::size_t{750});
      QCOMPARE(remaining.count(), std::size_t{750});
      QCOMPARE(filePatterns(remaining.addFileExclusions.at(id)), expected[0]);
      QCOMPARE(filePatterns(remaining.removeFileExclusions.at(id)), expected[1]);
      QCOMPARE(registryPaths(remaining.addRegistryExclusions), expected[2]);
      QCOMPARE(registryPaths(remaining.removeRegistryExclusions), expected[3]);
    };
    session.reconcileApplied(requested, result);
    verifyRemaining();
    session.clearVolumes();
    verifyRemaining();
    session.editVolume(physical, true);
    session.resetSnapshot(observed, {}, false);
    verifyRemaining();
    session.discardChanges(observed, {}, false);
    QVERIFY(session.pendingConfiguration().empty());
  }
  void registrationAdmissionIsIndependentOfExistingMembership_data() {
    QTest::addColumn<int>("physicalState");
    QTest::addColumn<int>("membership");
    for (int physicalState = 0; physicalState < 4; ++physicalState)
      for (int membership = 0; membership < 4; ++membership)
        QTest::newRow(qUtf8Printable(QStringLiteral("physical-%1/membership-%2").arg(physicalState).arg(membership))) << physicalState << membership;
  }
  void registrationAdmissionIsIndependentOfExistingMembership() {
    QFETCH(int, physicalState);
    QFETCH(int, membership);
    const core::PhysicalVolumeFacts facts[] = {{}, {false, {}}, {true, false}, {true, true}};
    const core::VolumeInfo physical{
        "C:", test::application::fixtureVolumeCName, "NTFS", "System", 1000, 500, core::VolumeSupport::Supported, facts[physicalState]};
    app::VolumeConfigurationEditor editor(physical, false);
    core::UwfSnapshot snapshot;
    snapshot.uwfAvailable = snapshot.elevated = true;
    const core::VolumeRecord record{physical.volumeName, "C:", true, core::VolumeBinding::DriveLetter};
    const bool current = (membership & 1) != 0;
    const bool next = (membership & 2) != 0;
    if (current) snapshot.current.volumes.push_back(record);
    if (next) snapshot.next.volumes.push_back(record);
    editor.applySnapshot(snapshot, false);
    const bool eligible = physicalState == 3;
    QCOMPARE(editor.status()->controlsEnabled(), next || eligible);
    QCOMPARE(editor.status()->currentProtection().has_value(), current);
    QCOMPARE(editor.status()->draft().hasConfiguration, next);
    QCOMPARE(editor.editingAllowed(), current || next || eligible);
    QVERIFY(!editor.files()->readOnly());
    // Missing Next membership requires a new registration even if Current
    // remains protected. An existing Next registration can always be disabled.
    QCOMPARE(editor.status()->editProtection(!next), next || eligible);
    QCOMPARE(editor.status()->pendingProtection().has_value(), next || eligible);
  }
  void volumeUpdatePublishesOnlyAfterExplicitResume() {
    const core::VolumeInfo physical{"C:", test::application::fixtureVolumeCName, "NTFS", "System", 1000, 500, core::VolumeSupport::Supported};
    app::VolumeConfigurationEditor editor(physical, true);
    const auto status = editor.status();
    const auto files = editor.files();
    const auto registry = editor.registry();
    auto snapshot = test::application::editableSnapshot();
    snapshot.next.fileExclusions[physical.volumeName] = {"\\Saved"};
    snapshot.next.registryExclusions = {"HKEY_LOCAL_MACHINE\\SOFTWARE\\Saved"};
    int publications = 0;
    const auto verify = [&] {
      ++publications;
      QVERIFY(!editor.signalsBlocked());
      QVERIFY(!status->signalsBlocked());
      QVERIFY(!files->signalsBlocked());
      QVERIFY(!registry->signalsBlocked());
      QVERIFY(editor.snapshotState().available);
      QCOMPARE(files->draft().next(), QStringList{QStringLiteral("C:\\Saved")});
      QCOMPARE(registry->draft().next(), QStringList{QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Saved")});
    };
    connect(status.get(), &app::VolumeStatusEditor::stateChanged, this, verify);
    connect(files.get(), &app::ExclusionEditor::baselineChanged, this, verify);
    connect(registry.get(), &app::ExclusionEditor::baselineChanged, this, verify);
    connect(&editor, &app::VolumeConfigurationEditor::stateChanged, this, verify);
    auto update = editor.beginUpdate();
    editor.applySnapshot(snapshot, false);
    QCOMPARE(publications, 0);
    update.resume();
    QCOMPARE(publications, 0);
    QVERIFY(update.publishSnapshot());
    QCOMPARE(publications, 4);
    {
      auto silent = editor.beginUpdate();
      editor.setEditingAllowed(false);
    }
    QCOMPARE(publications, 4);
    QVERIFY(!editor.editingAllowed());
    QVERIFY(!editor.signalsBlocked());
    QVERIFY(!status->signalsBlocked());
    QVERIFY(!files->signalsBlocked());
    QVERIFY(!registry->signalsBlocked());
  }
  void volumePublicationCanStopAtEveryChildBoundary_data() {
    QTest::addColumn<int>("interruptAfter");
    for (int boundary = -1; boundary != 10; ++boundary) QTest::newRow(qPrintable(QString::number(boundary))) << boundary;
  }
  void volumePublicationCanStopAtEveryChildBoundary() {
    QFETCH(int, interruptAfter);
    const core::VolumeInfo physical{"C:", test::application::fixtureVolumeCName, "NTFS", "System", 1000, 500, core::VolumeSupport::Supported};
    app::VolumeConfigurationEditor editor(physical, true);
    auto update = editor.beginUpdate();
    editor.applySnapshot(test::application::editableSnapshot(), false);
    update.resume();
    bool current = interruptAfter != -1;
    QStringList publications;
    const auto record = [&](const QString& name) {
      publications.append(name);
      if (publications.size() == interruptAfter + 1) current = false;
    };
    connect(editor.status().get(), &app::VolumeStatusEditor::stateChanged, this, [&] { record(QStringLiteral("status-state")); });
    connect(editor.status().get(), &app::VolumeStatusEditor::availabilityChanged, this, [&] { record(QStringLiteral("status-availability")); });
    connect(editor.files().get(), &app::ExclusionEditor::baselineChanged, this, [&] { record(QStringLiteral("files-baseline")); });
    connect(editor.files().get(), &app::ExclusionEditor::stateChanged, this, [&] { record(QStringLiteral("files-state")); });
    connect(editor.files().get(), &app::ExclusionEditor::pendingChanged, this, [&] { record(QStringLiteral("files-pending")); });
    connect(editor.registry().get(), &app::ExclusionEditor::baselineChanged, this, [&] { record(QStringLiteral("registry-baseline")); });
    connect(editor.registry().get(), &app::ExclusionEditor::stateChanged, this, [&] { record(QStringLiteral("registry-state")); });
    connect(editor.registry().get(), &app::ExclusionEditor::pendingChanged, this, [&] { record(QStringLiteral("registry-pending")); });
    connect(editor.files().get(), &app::ExclusionEditor::editingAvailabilityChanged, this, [&] { record(QStringLiteral("files-availability")); });
    connect(&editor, &app::VolumeConfigurationEditor::stateChanged, this, [&] { record(QStringLiteral("volume-state")); });
    QVERIFY(!update.publishSnapshot([&] { return current; }));
    const QStringList expected{QStringLiteral("status-state"),   QStringLiteral("status-availability"), QStringLiteral("files-baseline"),
                               QStringLiteral("files-state"),    QStringLiteral("files-pending"),       QStringLiteral("registry-baseline"),
                               QStringLiteral("registry-state"), QStringLiteral("registry-pending"),    QStringLiteral("files-availability"),
                               QStringLiteral("volume-state")};
    QCOMPARE(publications, expected.mid(0, interruptAfter + 1));
  }
  void volumeUpdateSurvivesOwnerDestructionWithoutPublishing() {
    const core::VolumeInfo physical{"C:", test::application::fixtureVolumeCName, "NTFS", "System", 1000, 500, core::VolumeSupport::Supported};
    auto editor = std::make_unique<app::VolumeConfigurationEditor>(physical, true);
    const auto status = editor->status();
    const auto files = editor->files();
    const auto registry = editor->registry();
    QSignalSpy statusSignals(status.get(), &app::VolumeStatusEditor::stateChanged);
    QSignalSpy fileSignals(files.get(), &app::ExclusionEditor::stateChanged);
    QSignalSpy registrySignals(registry.get(), &app::ExclusionEditor::stateChanged);
    auto update = editor->beginUpdate();
    editor->applySnapshot(test::application::editableSnapshot(), false);
    editor.reset();
    update.resume();
    QVERIFY(!update.publishSnapshot());
    QVERIFY(!status->signalsBlocked());
    QVERIFY(!files->signalsBlocked());
    QVERIFY(!registry->signalsBlocked());
    QCOMPARE(statusSignals.size(), 0);
    QCOMPARE(fileSignals.size(), 0);
    QCOMPARE(registrySignals.size(), 0);
  }
  void resetPublicationsResumeAllParticipantsBeforeTheFirstSignal() {
    app::ConfigurationEditSession session;
    const auto volume = addVolume(session);
    int publications = 0;
    const auto verify = [&] {
      ++publications;
      QVERIFY(!session.globalEditor()->signalsBlocked());
      QVERIFY(!session.overlayPolicyEditor()->signalsBlocked());
      QVERIFY(!session.registryEditor()->signalsBlocked());
      QVERIFY(!session.maintenanceEditor()->signalsBlocked());
      QVERIFY(!volume->signalsBlocked());
      QVERIFY(!volume->status()->signalsBlocked());
      QVERIFY(!volume->files()->signalsBlocked());
    };
    connect(volume->status().get(), &app::VolumeStatusEditor::stateChanged, this, verify);
    connect(session.registryEditor().get(), &app::ExclusionEditor::baselineChanged, this, verify);
    connect(session.overlayPolicyEditor().get(), &app::OverlayPolicyEditor::stateChanged, this, verify);
    connect(session.globalEditor().get(), &app::GlobalConfigurationEditor::stateChanged, this, verify);
    session.resetSnapshot(test::application::editableSnapshot(), test::application::diskPersistentPolicy(), false);
    QVERIFY(publications >= 4);
  }
  void volumeAvailabilityUsesTheLatestAdmissionBetweenSignals() {
    const core::VolumeInfo physical{"C:", test::application::fixtureVolumeCName, "NTFS", "System", 1000, 500, core::VolumeSupport::Supported};
    app::VolumeConfigurationEditor editor(physical, true);
    editor.applySnapshot(test::application::editableSnapshot(), false);
    auto update = editor.beginUpdate();
    editor.setEditingAllowed(false);
    update.resume();
    bool restored = false;
    connect(editor.status().get(), &app::VolumeStatusEditor::availabilityChanged, this, [&] {
      if (std::exchange(restored, true)) return;
      editor.setEditingAllowed(true);
    });
    QSignalSpy files(editor.files().get(), &app::ExclusionEditor::editingAvailabilityChanged);
    QVERIFY(update.publishAvailability());
    QVERIFY(restored);
    QVERIFY(editor.editingAllowed());
    QVERIFY(!editor.files()->readOnly());
    QCOMPARE(files.size(), 1);
  }
  void volumePublicationStopsWhenTheFacadeIsDestroyedByAChildSignal() {
    const core::VolumeInfo physical{"C:", test::application::fixtureVolumeCName, "NTFS", "System", 1000, 500, core::VolumeSupport::Supported};
    auto editor = std::make_unique<app::VolumeConfigurationEditor>(physical, true);
    const auto status = editor->status();
    const auto files = editor->files();
    const auto registry = editor->registry();
    auto update = editor->beginUpdate();
    editor->applySnapshot(test::application::editableSnapshot(), false);
    update.resume();
    connect(status.get(), &app::VolumeStatusEditor::stateChanged, this, [&] { editor.reset(); });
    QSignalSpy statusAvailability(status.get(), &app::VolumeStatusEditor::availabilityChanged);
    QSignalSpy fileSignals(files.get(), &app::ExclusionEditor::stateChanged);
    QSignalSpy registrySignals(registry.get(), &app::ExclusionEditor::stateChanged);
    QVERIFY(!update.publishSnapshot());
    QVERIFY(!editor);
    QCOMPARE(statusAvailability.size(), 0);
    QCOMPARE(fileSignals.size(), 0);
    QCOMPARE(registrySignals.size(), 0);
  }
  void resetTransactionsSurviveOwnerDestruction_data() {
    QTest::addColumn<bool>("discard");
    QTest::addColumn<int>("notification");
    for (const bool discard : {false, true}) {
      const auto mode = discard ? QStringLiteral("discard") : QStringLiteral("refresh");
      QTest::newRow(qUtf8Printable(mode + QStringLiteral("/admission"))) << discard << 0;
      QTest::newRow(qUtf8Printable(mode + QStringLiteral("/registry-baseline"))) << discard << 1;
      QTest::newRow(qUtf8Printable(mode + QStringLiteral("/volume-baseline"))) << discard << 2;
    }
  }
  void resetTransactionsSurviveOwnerDestruction() {
    QFETCH(bool, discard);
    QFETCH(int, notification);
    auto session = std::make_unique<app::ConfigurationEditSession>();
    const auto first = addVolume(*session);
    const auto global = session->globalEditor();
    const auto registry = session->registryEditor();
    const auto destroy = [&] { session.reset(); };
    if (notification == 0)
      connect(global.get(), &app::GlobalConfigurationEditor::availabilityChanged, this, destroy);
    else if (notification == 1)
      connect(registry.get(), &app::ExclusionEditor::baselineChanged, this, destroy);
    else
      connect(first->status().get(), &app::VolumeStatusEditor::stateChanged, this, destroy);
    const auto snapshot = test::application::editableSnapshot();
    if (discard)
      session->discardChanges(snapshot);
    else
      session->resetSnapshot(snapshot);
    QVERIFY(!session);
    QVERIFY(!global->controlsEnabled());
    QVERIFY(registry->readOnly());
    QVERIFY(!first->editingAllowed());
    QVERIFY(first->files()->readOnly());
  }
  void resetTransactionsDoNotReviveReplacedVolumes_data() {
    QTest::addColumn<bool>("registryNotification");
    QTest::newRow("snapshot-volume-callback") << false;
    QTest::newRow("registry-baseline-callback") << true;
  }
  void resetTransactionsDoNotReviveReplacedVolumes() {
    QFETCH(bool, registryNotification);
    app::ConfigurationEditSession session;
    const auto first = addVolume(session);
    const core::VolumeInfo physical{"D:", test::application::fixtureVolumeDName, "NTFS", "Data", 1000, 500, core::VolumeSupport::Supported};
    const auto second = session.editVolume(physical, false);
    session.resetSnapshot(test::application::editableSnapshot());
    auto observed = test::application::editableSnapshot();
    observed.next.filter.enabled = false;
    observed.next.registryExclusions = {"HKEY_LOCAL_MACHINE\\SOFTWARE\\AfterReplacement"};
    bool globalPublished = false;
    bool registryPublished = false;
    connect(session.globalEditor().get(), &app::GlobalConfigurationEditor::stateChanged, this,
            [&] { globalPublished = session.globalEditor()->draft().filterEnabled == observed.next.filter.enabled; });
    connect(session.registryEditor().get(), &app::ExclusionEditor::stateChanged, this, [&] {
      registryPublished = session.registryEditor()->draft().next() == QStringList{QString::fromStdString(observed.next.registryExclusions.front())};
    });
    std::shared_ptr<app::VolumeConfigurationEditor> replacement;
    const auto replace = [&] {
      if (replacement) return;
      session.clearVolumes();
      replacement = session.editVolume(physical, false);
    };
    if (registryNotification)
      connect(session.registryEditor().get(), &app::ExclusionEditor::baselineChanged, this, replace);
    else
      connect(first->status().get(), &app::VolumeStatusEditor::stateChanged, this, replace);
    session.resetSnapshot(observed);
    QVERIFY(replacement);
    QVERIFY(registryPublished);
    QVERIFY(globalPublished);
    QVERIFY(!first->editingAllowed());
    QVERIFY(!second->editingAllowed());
    QVERIFY(first->files()->readOnly());
    QVERIFY(second->files()->readOnly());
    QVERIFY(!replacement->snapshotState().available);
    QVERIFY(!replacement->editingAllowed());
    session.resetSnapshot(test::application::editableSnapshot());
    QVERIFY(replacement->editingAllowed());
    QVERIFY(!first->editingAllowed());
    QVERIFY(!second->editingAllowed());
  }
  void nestedResetPublishesTheNewestObservation_data() {
    QTest::addColumn<bool>("hormEnabled");
    QTest::newRow("latest-editable") << false;
    QTest::newRow("latest-horm") << true;
  }
  void nestedResetPublishesTheNewestObservation() {
    QFETCH(bool, hormEnabled);
    app::ConfigurationEditSession session;
    const auto volume = addVolume(session);
    auto first = test::application::editableSnapshot();
    auto latest = first;
    first.next.filter.enabled = false;
    latest.next.registryExclusions = {"HKEY_LOCAL_MACHINE\\SOFTWARE\\LatestObservation"};
    bool requested = false;
    QList<bool> admission;
    connect(session.globalEditor().get(), &app::GlobalConfigurationEditor::availabilityChanged, this,
            [&] { admission.append(session.globalEditor()->controlsEnabled()); });
    connect(session.globalEditor().get(), &app::GlobalConfigurationEditor::stateChanged, this, [&] {
      if (std::exchange(requested, true)) return;
      session.resetSnapshot(latest, {}, hormEnabled);
    });
    session.resetSnapshot(first);
    QVERIFY(requested);
    QCOMPARE(session.globalEditor()->draft().filterEnabled, latest.next.filter.enabled);
    QCOMPARE(session.registryEditor()->draft().next(), QStringList{QString::fromStdString(latest.next.registryExclusions.front())});
    QCOMPARE(volume->snapshotState().hormEnabled, std::optional{hormEnabled});
    QCOMPARE(volume->editingAllowed(), !hormEnabled);
    if (hormEnabled) QVERIFY(!admission.contains(true));
  }

  void globalGateNotificationsSurviveVolumeReplacement() {
    app::ConfigurationEditSession session;
    const auto first = addVolume(session);
    session.resetSnapshot(test::application::editableSnapshot(), test::application::diskPersistentPolicy());
    QSignalSpy policy(session.overlayPolicyEditor().get(), &app::OverlayPolicyEditor::stateChanged);
    QSignalSpy registry(session.registryEditor().get(), &app::ExclusionEditor::editingAvailabilityChanged);
    bool replaced = false;
    connect(session.globalEditor().get(), &app::GlobalConfigurationEditor::availabilityChanged, this, [&] {
      if (session.globalEditor()->controlsEnabled() || std::exchange(replaced, true)) return;
      session.clearVolumes();
      session.editVolume({"D:", test::application::fixtureVolumeDName, "NTFS", "Data", 1000, 500, core::VolumeSupport::Supported}, false);
    });
    session.setEditingAllowed(false);
    QVERIFY(replaced);
    QVERIFY(!first->editingAllowed());
    QVERIFY(!session.overlayPolicyEditor()->editingAllowed());
    QVERIFY(session.registryEditor()->readOnly());
    QVERIFY(policy.count() > 0);
    QVERIFY(registry.count() > 0);
  }

  void editingContextNotificationsExposeTheCompleteBoundary() {
    app::ConfigurationEditSession session;
    const auto volume = addVolume(session);
    auto snapshot = test::application::editableSnapshot();
    session.resetSnapshot(snapshot, test::application::diskPersistentPolicy());
    bool expected = true;
    int notifications = 0;
    const auto verify = [&] {
      ++notifications;
      QCOMPARE(session.globalEditor()->controlsEnabled(), expected);
      QCOMPARE(session.overlayPolicyEditor()->editingAllowed(), expected);
      QCOMPARE(session.registryEditor()->readOnly(), !expected);
      QCOMPARE(volume->editingAllowed(), expected);
      QCOMPARE(volume->files()->readOnly(), !expected);
      if (!expected) {
        QVERIFY(!volume->status()->editProtection(false));
        QCOMPARE(volume->files()->importAdd(QStringLiteral("C:\\Data\\New")), app::ExclusionImportOutcome::RejectedReadOnly);
        QCOMPARE(session.registryEditor()->importAdd(QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\New")), app::ExclusionImportOutcome::RejectedReadOnly);
        QCOMPARE(session.globalEditor()->importFilterEnabled(false).outcome, app::GlobalEditOutcome::Rejected);
        QVERIFY(!session.overlayPolicyEditor()->importChanges({.readOnlyMedia = true}));
      }
    };
    connect(session.globalEditor().get(), &app::GlobalConfigurationEditor::availabilityChanged, this, verify);
    connect(session.overlayPolicyEditor().get(), &app::OverlayPolicyEditor::stateChanged, this, verify);
    connect(session.registryEditor().get(), &app::ExclusionEditor::editingAvailabilityChanged, this, verify);
    connect(volume->status().get(), &app::VolumeStatusEditor::availabilityChanged, this, verify);
    connect(volume->files().get(), &app::ExclusionEditor::editingAvailabilityChanged, this, verify);
    for (const bool allowed : {false, true, false, true}) {
      expected = allowed;
      session.setEditingAllowed(allowed);
      QVERIFY(session.pendingConfiguration().empty());
    }
    QVERIFY(notifications > 0);
  }
  void snapshotResetKeepsReentrantContextChanges_data() {
    QTest::addColumn<bool>("requestedAllowed");
    QTest::newRow("revoked-during-reset") << false;
    QTest::newRow("granted-during-reset") << true;
  }
  void snapshotResetKeepsReentrantContextChanges() {
    QFETCH(bool, requestedAllowed);
    app::ConfigurationEditSession session;
    const auto volume = addVolume(session);
    const auto snapshot = test::application::editableSnapshot();
    const auto policy = test::application::diskPersistentPolicy();
    session.setEditingAllowed(!requestedAllowed);
    bool observed = false;
    connect(session.globalEditor().get(), &app::GlobalConfigurationEditor::stateChanged, this, [&] {
      observed = true;
      session.setEditingAllowed(requestedAllowed);
      QVERIFY(!session.globalEditor()->controlsEnabled());
      QVERIFY(!session.overlayPolicyEditor()->editingAllowed());
      QVERIFY(session.registryEditor()->readOnly());
      QVERIFY(!volume->editingAllowed());
      QVERIFY(volume->files()->readOnly());
    });
    session.resetSnapshot(snapshot, policy);
    QVERIFY(observed);
    QCOMPARE(session.globalEditor()->controlsEnabled(), requestedAllowed);
    QCOMPARE(session.overlayPolicyEditor()->editingAllowed(), requestedAllowed);
    QCOMPARE(session.registryEditor()->readOnly(), !requestedAllowed);
    QCOMPARE(volume->editingAllowed(), requestedAllowed);
    QCOMPARE(volume->files()->readOnly(), !requestedAllowed);
  }

  void sharedEditingContextSurvivesSnapshotAndVolumeReplacement_data() {
    QTest::addColumn<bool>("contextAllowed");
    QTest::addColumn<int>("observation");
    QTest::addColumn<int>("volumeKind");
    for (const bool allowed : {false, true})
      for (int observation = 0; observation < 4; ++observation)
        for (int volumeKind = 0; volumeKind < 3; ++volumeKind)
          QTest::newRow(qUtf8Printable(QStringLiteral("allowed-%1-observation-%2-volume-%3").arg(allowed).arg(observation).arg(volumeKind)))
              << allowed << observation << volumeKind;
  }
  void sharedEditingContextSurvivesSnapshotAndVolumeReplacement() {
    QFETCH(bool, contextAllowed);
    QFETCH(int, observation);
    QFETCH(int, volumeKind);
    app::ConfigurationEditSession session;
    auto snapshot = test::application::editableSnapshot();
    snapshot.uwfAvailable = observation != 1;
    snapshot.elevated = observation != 2;
    const std::optional<bool> horm = observation == 3;
    auto policy = test::application::diskPersistentPolicy();
    const core::VolumeInfo physical{volumeKind == 2 ? "" : "C:",
                                    test::application::fixtureVolumeCName,
                                    volumeKind == 1 ? "exFAT" : "NTFS",
                                    "System",
                                    1000,
                                    500,
                                    volumeKind == 1 ? core::VolumeSupport::FileSystemLimited : core::VolumeSupport::Supported};
    if (volumeKind == 2)
      for (auto* state : {&snapshot.current, &snapshot.next}) state->volumes.front().driveLetter.clear();
    const bool allowed = contextAllowed && observation == 0;
    const bool filesAllowed = allowed && volumeKind == 0;
    session.setEditingAllowed(contextAllowed);
    auto volume = session.editVolume(physical, true);
    for (int replacement = 0; replacement < 2; ++replacement) {
      if (replacement) {
        const auto retired = volume;
        session.clearVolumes();
        QVERIFY(!retired->editingAllowed());
        QVERIFY(retired->files()->readOnly());
        volume = session.editVolume(physical, true);
      }
      session.resetSnapshot(snapshot, policy, horm);
      QCOMPARE(session.globalEditor()->controlsEnabled(), allowed);
      QCOMPARE(session.overlayPolicyEditor()->editingAllowed(), allowed);
      QCOMPARE(session.registryEditor()->readOnly(), !allowed);
      QCOMPARE(volume->editingAllowed(), allowed);
      QCOMPARE(volume->files()->readOnly(), !filesAllowed);
      QCOMPARE(volume->status()->editProtection(false), allowed);
      QCOMPARE(volume->files()->importAdd(QStringLiteral("C:\\Data\\New")),
               filesAllowed ? app::ExclusionImportOutcome::Applied : app::ExclusionImportOutcome::RejectedReadOnly);
      QCOMPARE(session.registryEditor()->importAdd(QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\New")),
               allowed ? app::ExclusionImportOutcome::Applied : app::ExclusionImportOutcome::RejectedReadOnly);
      if (!allowed) QVERIFY(session.pendingConfiguration().empty());
    }
    session.setEditingAllowed(false);
    QVERIFY(!session.globalEditor()->controlsEnabled());
    QVERIFY(!session.overlayPolicyEditor()->editingAllowed());
    QVERIFY(session.registryEditor()->readOnly());
    QVERIFY(!volume->editingAllowed());
    QVERIFY(volume->files()->readOnly());
    session.setEditingAllowed(true);
    QCOMPARE(volume->editingAllowed(), observation == 0);
    QCOMPARE(session.registryEditor()->readOnly(), observation != 0);
  }

  void sessionAdmissionAppliesToEveryImportFamily_data() {
    QTest::addColumn<bool>("unavailable");
    QTest::addColumn<QString>("script");
    const QStringList scripts{QStringLiteral("uwfmgr filter enable"),
                              QStringLiteral("uwfmgr overlay set-type Disk"),
                              QStringLiteral("uwfmgr overlay set-size 8192"),
                              QStringLiteral("uwfmgr overlay set-warningthreshold 512"),
                              QStringLiteral("uwfmgr overlay set-criticalthreshold 2048"),
                              QStringLiteral("uwfmgr volume unprotect C:"),
                              QStringLiteral("uwfmgr file add-exclusion C:\\New"),
                              QStringLiteral("uwfmgr file remove-exclusion C:\\Old"),
                              QStringLiteral("uwfmgr registry add-exclusion HKLM\\SOFTWARE\\New"),
                              QStringLiteral("uwfmgr registry remove-exclusion HKLM\\SOFTWARE\\Old"),
                              QStringLiteral("uwfmgr overlay set-passthrough off"),
                              QStringLiteral("uwfmgr overlay set-persistent off"),
                              QStringLiteral("uwfmgr overlay set-rom-mode on"),
                              QStringLiteral("uwfmgr overlay reset-persistentstate on")};
    for (const bool unavailable : {false, true})
      for (qsizetype script = 0; script < scripts.size(); ++script)
        QTest::newRow(qUtf8Printable(QStringLiteral("%1/command-%2").arg(unavailable ? "unavailable" : "controls-disabled").arg(script)))
            << unavailable << scripts.at(script);
  }
  void sessionAdmissionAppliesToEveryImportFamily() {
    QFETCH(bool, unavailable);
    QFETCH(QString, script);
    app::ConfigurationEditSession session;
    const auto volume = addVolume(session);
    auto snapshot = test::application::editableSnapshot();
    snapshot.next.filter.enabled = false;
    snapshot.next.overlay.type = core::OverlayType::RAM;
    session.resetSnapshot(snapshot, test::application::diskPersistentPolicy());
    if (unavailable)
      session.globalEditor()->setAvailable(false);
    else
      session.globalEditor()->setControlsEnabled(false);
    // Leave subeditors writable to isolate the session-wide command gate.
    QVERIFY(volume->editingAllowed());
    QVERIFY(!session.registryEditor()->readOnly());
    QSignalSpy globalChanged(session.globalEditor().get(), &app::GlobalConfigurationEditor::pendingChanged);
    QSignalSpy registryChanged(session.registryEditor().get(), &app::ExclusionEditor::pendingChanged);
    QSignalSpy volumeChanged(volume.get(), &app::VolumeConfigurationEditor::pendingChanged);
    QSignalSpy policyChanged(session.overlayPolicyEditor().get(), &app::OverlayPolicyEditor::pendingChanged);
    const auto report = session.importCommands(commands(script.toStdString() + " && uwfmgr filter enable"));
    QCOMPARE(report.size(), 2);
    QCOMPARE(report.front().status, app::ImportResult::Status::Failed);
    QCOMPARE(report.back().status, app::ImportResult::Status::Failed);
    QVERIFY(report.back().detail.contains(QStringLiteral("prerequisite")));
    QVERIFY(session.pendingConfiguration().empty());
    QCOMPARE(globalChanged.size(), 0);
    QCOMPARE(registryChanged.size(), 0);
    QCOMPARE(volumeChanged.size(), 0);
    QCOMPARE(policyChanged.size(), 0);
    session.globalEditor()->setAvailable(true);
    session.globalEditor()->setControlsEnabled(true);
    QCOMPARE(session.importCommands(commands("uwfmgr filter enable")).constFirst().status, app::ImportResult::Status::Success);
  }
  void disabledGlobalEntryPointsCannotChangeTheDraft() {
    app::GlobalConfigurationEditor editor;
    core::SessionSnapshot baseline;
    baseline.overlay = {core::OverlayType::RAM, 4096, 1024, 3072};
    editor.reset(baseline);
    editor.setControlsEnabled(false);
    QSignalSpy changed(&editor, &app::GlobalConfigurationEditor::pendingChanged);
    editor.editFilterEnabled(true);
    editor.editOverlayType(core::OverlayType::Disk);
    editor.editMaximumSize(8192);
    editor.editWarningThreshold(512);
    editor.editCriticalThreshold(2048);
    QCOMPARE((editor.importFilterEnabled(true)).outcome, uwf::app::GlobalEditOutcome::Rejected);
    QCOMPARE((editor.importOverlayType(core::OverlayType::Disk)).outcome, uwf::app::GlobalEditOutcome::Rejected);
    QCOMPARE((editor.importOverlayMaxMb(8192)).outcome, uwf::app::GlobalEditOutcome::Rejected);
    QCOMPARE((editor.importOverlayWarnMb(512)).outcome, uwf::app::GlobalEditOutcome::Rejected);
    QCOMPARE((editor.importOverlayCritMb(2048)).outcome, uwf::app::GlobalEditOutcome::Rejected);
    editor.finishImport();
    QCOMPARE(editor.draft().filterEnabled, baseline.filter.enabled);
    QVERIFY(editor.pendingOverlay().empty());
    QCOMPARE(changed.size(), 0);
  }
  void importRechecksSessionAdmissionBeforeEachCommand() {
    app::ConfigurationEditSession session;
    const auto volume = addVolume(session);
    auto snapshot = test::application::editableSnapshot();
    snapshot.next.filter.enabled = false;
    session.resetSnapshot(snapshot, test::application::diskPersistentPolicy());
    connect(session.globalEditor().get(), &app::GlobalConfigurationEditor::pendingChanged, this, [&] { session.globalEditor()->setControlsEnabled(false); });
    const auto report =
        session.importCommands(commands("uwfmgr filter enable\nuwfmgr volume unprotect C:\n"
                                        "uwfmgr registry add-exclusion HKLM\\SOFTWARE\\New\nuwfmgr overlay set-passthrough off"));
    QCOMPARE(report.size(), 4);
    QCOMPARE(report.front().status, app::ImportResult::Status::Success);
    for (qsizetype index = 1; index < report.size(); ++index) QCOMPARE(report.at(index).status, app::ImportResult::Status::Failed);
    QCOMPARE(session.pendingChanges().setFilterEnabled, std::optional{true});
    QCOMPARE(session.pendingConfiguration().count(), std::size_t{1});
    QVERIFY(volume->status()->draft().protectedVolume);
  }
  void localImportRejectionsRemainFailures_data() {
    QTest::addColumn<int>("target");
    QTest::addColumn<QString>("script");
    QTest::newRow("registry") << 0 << QStringLiteral("uwfmgr registry add-exclusion HKLM\\SOFTWARE\\New");
    QTest::newRow("file") << 1 << QStringLiteral("uwfmgr file add-exclusion C:\\New");
    QTest::newRow("overlay") << 2 << QStringLiteral("uwfmgr overlay set-passthrough off");
  }
  void localImportRejectionsRemainFailures() {
    QFETCH(int, target);
    QFETCH(QString, script);
    app::ConfigurationEditSession session;
    const auto volume = addVolume(session);
    const auto state = test::application::editableSnapshot();
    session.resetSnapshot(state, test::application::diskPersistentPolicy());
    if (target == 0) session.registryEditor()->setReadOnly(true);
    if (target == 1) volume->files()->setReadOnly(true);
    if (target == 2) session.overlayPolicyEditor()->setControlsEnabled(false);
    QVERIFY(session.globalEditor()->editingAllowed());
    const auto report = session.importCommands(commands(script.toStdString()));
    QCOMPARE(report.front().status, app::ImportResult::Status::Failed);
    QVERIFY(session.pendingConfiguration().empty());
  }
  void pathResolutionCannotDispatchAfterSessionAdmissionIsRevoked() {
    std::shared_ptr<app::GlobalConfigurationEditor> admission;
    app::ConfigurationEditSession session(0, {}, [&](const std::string&) {
      admission->setControlsEnabled(false);
      return std::string("C:");
    });
    admission = session.globalEditor();
    const auto volume = addVolume(session);
    const auto report = session.importCommands(commands("uwfmgr file add-exclusion C:\\New"));
    QCOMPARE(report.front().status, app::ImportResult::Status::Failed);
    QVERIFY(volume->files()->pendingAdded().isEmpty());
    QVERIFY(session.pendingChanges().empty());
  }
  void localVolumeAdmissionRejectsDirectAndImportedChanges_data() {
    QTest::addColumn<int>("restriction");
    QTest::newRow("unavailable") << 0;
    QTest::newRow("not-elevated") << 1;
    QTest::newRow("horm-active") << 2;
    QTest::newRow("controls-disabled") << 3;
  }
  void localVolumeAdmissionRejectsDirectAndImportedChanges() {
    QFETCH(int, restriction);
    app::ConfigurationEditSession session;
    const auto volume = addVolume(session);
    core::UwfSnapshot snapshot;
    snapshot.uwfAvailable = restriction != 0;
    snapshot.elevated = restriction != 1;
    snapshot.current.volumes = {{test::application::fixtureVolumeCName, "C:", true, core::VolumeBinding::DriveLetter}};
    snapshot.next = snapshot.current;
    volume->applySnapshot(snapshot, restriction == 2 ? std::optional{true} : std::nullopt);
    if (restriction == 3) volume->status()->setControlsEnabled(false);
    QSignalSpy changed(volume.get(), &app::VolumeConfigurationEditor::pendingChanged);
    QVERIFY(!volume->status()->editProtection(false));
    QVERIFY(!volume->status()->editBinding(core::VolumeBinding::VolumeId));
    QVERIFY(!volume->importProtect(false));
    QCOMPARE(volume->importAddFileExclusion(QStringLiteral("C:\\New")), app::ExclusionImportOutcome::RejectedReadOnly);
    QCOMPARE(volume->importRemoveFileExclusion(QStringLiteral("C:\\Old")), app::ExclusionImportOutcome::RejectedReadOnly);
    const auto report = app::applyImportCommands(commands("uwfmgr volume unprotect C:"), nullptr, {volume}, nullptr, {});
    QCOMPARE(report.front().status, app::ImportResult::Status::Failed);
    QVERIFY(session.pendingChanges().empty());
    QCOMPARE(changed.size(), 0);
  }
  void policySnapshotResetPreservesTheSessionEditingGate() {
    app::ConfigurationEditSession session;
    session.globalEditor()->setAvailable(false);
    core::UwfSnapshot snapshot;
    snapshot.uwfAvailable = snapshot.elevated = true;
    snapshot.next.overlay.type = core::OverlayType::Disk;
    const auto policy = session.overlayPolicyEditor();
    auto baseline = test::application::diskPersistentPolicy();
    baseline.current.filterEnabled.value = false;
    policy->reset(baseline, snapshot);
    QCOMPARE(policy->disabledReason(policy->beginEdit(), app::OverlayPolicyField::Passthrough), app::OverlayPolicyIssue::WritesBlocked);
    QVERIFY(!policy->canCommit(policy->beginEdit()));

    session.globalEditor()->reset(snapshot.next);
    QVERIFY(policy->canCommit(policy->beginEdit()));
    session.globalEditor()->setControlsEnabled(false);
    QVERIFY(!policy->canCommit(policy->beginEdit()));
    policy->reset(baseline, snapshot);
    auto edit = policy->beginEdit();
    app::OverlayPolicyEditor::setValue(edit, app::OverlayPolicyField::Passthrough, false);
    QCOMPARE(policy->disabledReason(edit, app::OverlayPolicyField::Passthrough), app::OverlayPolicyIssue::WritesBlocked);
    QVERIFY(!policy->commit(edit));
    QVERIFY(policy->pendingChanges().empty());

    session.globalEditor()->setControlsEnabled(true);
    QVERIFY(policy->canCommit(edit));
    QVERIFY(policy->commit(edit));
    QCOMPARE(policy->pendingChanges().passthrough, std::optional{false});
  }
  void overlayPolicyFollowsTheApplicationEditorsWithoutAViewModel() {
    app::ConfigurationEditSession session;
    core::UwfSnapshot snapshot;
    snapshot.uwfAvailable = snapshot.elevated = true;
    snapshot.next.overlay.type = core::OverlayType::Disk;
    session.globalEditor()->reset(snapshot.next);
    const auto policy = session.overlayPolicyEditor();
    auto observedPolicy = test::application::diskPersistentPolicy();
    observedPolicy.current.filterEnabled.value = false;
    policy->reset(observedPolicy, snapshot);
    QVERIFY(policy->canCommit(policy->beginEdit()));

    session.globalEditor()->editOverlayType(core::OverlayType::RAM);
    QCOMPARE(policy->targetType(), core::OverlayType::RAM);
    QVERIFY(policy->conflicts(policy->beginEdit()).empty());
    QVERIFY(policy->pendingChanges().empty());  // A type edit does not request restoring the old Persistent bit.
    session.globalEditor()->editOverlayType(core::OverlayType::Disk);
    QVERIFY(policy->canCommit(policy->beginEdit()));

    session.globalEditor()->setControlsEnabled(false);
    QCOMPARE(policy->disabledReason(policy->beginEdit(), app::OverlayPolicyField::Passthrough), app::OverlayPolicyIssue::WritesBlocked);
    QVERIFY(!policy->canCommit(policy->beginEdit()));
    session.globalEditor()->setControlsEnabled(true);
    QVERIFY(policy->canCommit(policy->beginEdit()));
    session.globalEditor()->setAvailable(false);
    QVERIFY(!policy->canCommit(policy->beginEdit()));
    session.globalEditor()->setAvailable(true);
    QVERIFY(policy->canCommit(policy->beginEdit()));
  }
  void importedOverlayTypeUpdatesPolicyConstraintsWithoutAViewModel() {
    app::ConfigurationEditSession session;
    core::UwfSnapshot snapshot;
    snapshot.uwfAvailable = snapshot.elevated = true;
    snapshot.next.overlay.type = core::OverlayType::Disk;
    session.globalEditor()->reset(snapshot.next);
    auto observedPolicy = test::application::diskPersistentPolicy();
    observedPolicy.current.filterEnabled.value = false;
    session.overlayPolicyEditor()->reset(observedPolicy, snapshot);
    const auto report = session.importCommands(commands("uwfmgr overlay set-type RAM"));
    QCOMPARE(report.front().status, app::ImportResult::Status::Success);
    QCOMPARE(session.overlayPolicyEditor()->targetType(), core::OverlayType::RAM);
    QVERIFY(session.overlayPolicyEditor()->canCommit(session.overlayPolicyEditor()->beginEdit()));
    QVERIFY(session.overlayPolicyEditor()->pendingChanges().empty());
  }
  void exclusionPolicyUsesTheInjectedSystemDrive() {
    app::ConfigurationEditSession session(16384, QStringLiteral("R:"));
    core::UwfSnapshot baseline;
    baseline.uwfAvailable = baseline.elevated = true;
    baseline.next.volumes = {{test::application::fixtureVolumeCName, "R:", false, core::VolumeBinding::DriveLetter, std::string("R:")},
                             {test::application::fixtureVolumeDName, "Z:", false, core::VolumeBinding::DriveLetter, std::string("Z:")}};
    core::VolumeInfo volume;
    volume.driveLetter = "R:";
    volume.volumeName = test::application::fixtureVolumeCName;
    volume.support = core::VolumeSupport::Supported;
    auto system = session.editVolume(volume, true);
    system->applySnapshot(baseline);
    QCOMPARE(system->files()->importAdd(QStringLiteral("R:\\Windows")), app::ExclusionImportOutcome::RejectedForbidden);
    volume.driveLetter = "Z:";
    volume.volumeName = test::application::fixtureVolumeDName;
    auto data = session.editVolume(volume, false);
    data->applySnapshot(baseline);
    QCOMPARE(data->files()->importAdd(QStringLiteral("Z:\\Windows")), app::ExclusionImportOutcome::Applied);
  }
  void activeImportRetainsItsDependenciesAfterSessionDestruction() {
    int resolutions = 0;
    auto session = std::make_unique<app::ConfigurationEditSession>(16384, QStringLiteral("C:"), [&](const std::string&) {
      ++resolutions;
      return std::string("C:");
    });
    core::SessionSnapshot baseline;
    session->globalEditor()->reset(baseline);
    const std::weak_ptr<app::GlobalConfigurationEditor> global = session->globalEditor();
    const std::weak_ptr<app::ExclusionEditor> registry = session->registryEditor();
    connect(session->globalEditor().get(), &app::GlobalConfigurationEditor::pendingChanged, this, [&] { session.reset(); });
    const auto report = session->importCommands(
        commands("uwfmgr filter enable\nuwfmgr file add-exclusion C:\\Data\nuwfmgr registry add-exclusion HKLM\\SOFTWARE\\ImportLifetimeFixture"));
    QVERIFY(!session);
    QCOMPARE(resolutions, 1);
    QCOMPARE(report.size(), 3);
    QCOMPARE(report[0].status, app::ImportResult::Status::Success);
    QCOMPARE(report[1].status, app::ImportResult::Status::Failed);
    QCOMPARE(report[2].status, app::ImportResult::Status::Success);
    QVERIFY(global.expired());
    QVERIFY(registry.expired());
  }
  void activeImportRetainsCspDraftAfterSessionDestruction() {
    auto session = std::make_unique<app::ConfigurationEditSession>();
    core::UwfSnapshot snapshot;
    snapshot.uwfAvailable = snapshot.elevated = true;
    session->globalEditor()->reset(snapshot.next);
    session->overlayPolicyEditor()->reset(test::application::diskPersistentPolicy(), snapshot);
    const std::weak_ptr<app::OverlayPolicyEditor> policy = session->overlayPolicyEditor();
    connect(session->globalEditor().get(), &app::GlobalConfigurationEditor::pendingChanged, this, [&] { session.reset(); });
    const auto report = session->importCommands(commands("uwfmgr filter enable\nuwfmgr overlay set-passthrough off"));
    QVERIFY(!session);
    QCOMPARE(report.size(), 2);
    QCOMPARE(report[0].status, app::ImportResult::Status::Success);
    QCOMPARE(report[1].status, app::ImportResult::Status::Success);
    QVERIFY(policy.expired());
  }
  void activeImportRetainsItsTargetUntilTheCommandReturns_data() {
    QTest::addColumn<QString>("script");
    QTest::addColumn<bool>("batchPublished");
    QTest::newRow("volume") << QStringLiteral("uwfmgr.exe volume unprotect C:\nuwfmgr.exe volume protect C:") << false;
    QTest::newRow("file-exclusions") << QStringLiteral("uwfmgr.exe file add-exclusion C:\\UwfData\nuwfmgr.exe file add-exclusion C:\\UwfOther") << true;
  }
  void activeImportRetainsItsTargetUntilTheCommandReturns() {
    QFETCH(QString, script);
    QFETCH(bool, batchPublished);
    app::ConfigurationEditSession session;
    auto editor = addVolume(session);
    QPointer<app::VolumeConfigurationEditor> lifetime(editor.get());
    bool reset = false;
    bool aliveDuringReset = false;
    connect(editor.get(), &app::VolumeConfigurationEditor::pendingChanged, this, [&] {
      session.clearVolumes();
      reset = true;
      aliveDuringReset = !lifetime.isNull();
    });
    editor.reset();
    const auto report = session.importCommands(commands(script.toStdString()));
    QVERIFY(reset);
    // Exclusions publish after the batch; other commands may notify while dispatch still owns their target.
    QCOMPARE(aliveDuringReset, !batchPublished);
    QVERIFY(!lifetime);
    QCOMPARE(report.size(), 2);
    QCOMPARE(report[0].status, app::ImportResult::Status::Success);
    QCOMPARE(report[1].status, batchPublished ? app::ImportResult::Status::Success : app::ImportResult::Status::Failed);
    QVERIFY(session.pendingChanges().empty());
  }
  void targetsRemovedByAnEarlierCommandAreNotDispatched_data() {
    QTest::addColumn<QString>("targetCommand");
    QTest::newRow("volume") << QStringLiteral("uwfmgr.exe volume unprotect C:");
    QTest::newRow("file-exclusions") << QStringLiteral("uwfmgr.exe file add-exclusion C:\\UwfData");
  }
  void targetsRemovedByAnEarlierCommandAreNotDispatched() {
    QFETCH(QString, targetCommand);
    app::ConfigurationEditSession session;
    auto editor = addVolume(session);
    QPointer<app::VolumeConfigurationEditor> lifetime(editor.get());
    editor.reset();
    connect(session.globalEditor().get(), &app::GlobalConfigurationEditor::pendingChanged, this, [&] { session.clearVolumes(); });
    const auto report = session.importCommands(commands((QStringLiteral("uwfmgr.exe filter enable\n") + targetCommand).toStdString()));
    QCOMPARE(report.size(), 2);
    QCOMPARE(report[0].status, app::ImportResult::Status::Success);
    QCOMPARE(report[1].status, app::ImportResult::Status::Failed);
    QVERIFY(!lifetime);
    QCOMPARE(session.pendingChanges().setFilterEnabled, std::optional<bool>(true));
  }
  void refreshingStagingConstraintsDoesNotRequireVolumeEditorsToStayAlive() {
    app::ConfigurationEditSession session;
    auto editor = addVolume(session);
    const auto root = QString::fromLatin1(app::kFileStagingRegistryRoot.data(), app::kFileStagingRegistryRoot.size());
    bool stagingHasData = false;
    editor->registry()->setFileStagingDataProbe([&] { return stagingHasData; });
    editor->registry()->setBaseline({root}, {root});
    QCOMPARE(editor->registry()->importRemove(root), app::ExclusionImportOutcome::Applied);
    QPointer<app::VolumeConfigurationEditor> lifetime(editor.get());
    bool notified = false;
    bool aliveDuringReset = false;
    connect(editor->registry().get(), &app::ExclusionEditor::stateChanged, this, [&] {
      session.clearVolumes();
      notified = true;
      aliveDuringReset = !lifetime.isNull();
    });
    editor.reset();
    stagingHasData = true;
    session.registryEditor()->refreshFileStagingConstraints();
    const auto pending = session.pendingChanges();
    QVERIFY(notified);
    QVERIFY(!aliveDuringReset);
    QVERIFY(!lifetime);
    QVERIFY(pending.empty());
  }
  void stagingConstraintUpdatesAreExplicitAndQueriesAreReadOnly_data() {
    QTest::addColumn<bool>("required");
    QTest::addColumn<bool>("readFails");
    QTest::newRow("empty") << false << false;
    QTest::newRow("contains-entries") << true << false;
    QTest::newRow("unreadable") << false << true;
  }
  void stagingConstraintUpdatesAreExplicitAndQueriesAreReadOnly() {
    QFETCH(bool, required);
    QFETCH(bool, readFails);
    app::ConfigurationEditSession session;
    addVolume(session);
    const auto registry = session.registryEditor();
    const auto root = QString::fromLatin1(app::kFileStagingRegistryRoot.data(), app::kFileStagingRegistryRoot.size());
    bool stagingHasData = false;
    bool stagingReadFails = false;
    int probes = 0;
    registry->setFileStagingDataProbe([&] {
      ++probes;
      if (stagingReadFails) throw std::runtime_error("staging read failed");
      return stagingHasData;
    });
    registry->setBaseline({root}, {root});
    QCOMPARE(registry->importRemove(root), app::ExclusionImportOutcome::Applied);
    QSignalSpy states(registry.get(), &app::ExclusionEditor::stateChanged);
    QSignalSpy pending(registry.get(), &app::ExclusionEditor::pendingChanged);
    QSignalSpy availability(registry.get(), &app::ExclusionEditor::removalAvailabilityChanged);
    stagingHasData = required;
    stagingReadFails = readFails;
    const int probesBeforeQuery = probes;
    const auto& readOnlySession = std::as_const(session);
    for (int iteration = 0; iteration < 2; ++iteration) {
      QCOMPARE(readOnlySession.pendingChanges().removeRegistryExclusions, std::vector<std::string>{root.toStdString()});
      QCOMPARE(registry->pendingRemoved(), QStringList{root});
    }
    QCOMPARE(probes, probesBeforeQuery);
    QCOMPARE(states.size(), 0);
    QCOMPARE(pending.size(), 0);
    QCOMPARE(availability.size(), 0);

    registry->refreshFileStagingConstraints();
    const bool protectedRoot = required || readFails;
    QCOMPARE(registry->pendingRemoved().isEmpty(), protectedRoot);
    QCOMPARE(readOnlySession.pendingChanges().removeRegistryExclusions.empty(), protectedRoot);
    QCOMPARE(states.size(), protectedRoot ? 1 : 0);
    QCOMPARE(pending.size(), protectedRoot ? 1 : 0);
    QCOMPARE(availability.size(), 1);
    stagingHasData = false;
    stagingReadFails = false;
    registry->refreshFileStagingConstraints();
    QCOMPARE(registry->pendingRemoved().isEmpty(), protectedRoot);
    QCOMPARE(availability.size(), 2);
  }
  void globalRegistryIntentDoesNotDependOnVolumeSupportOrLifetime_data() {
    QTest::addColumn<core::VolumeSupport>("support");
    QTest::addColumn<bool>("removeVolumes");
    for (const auto support :
         {core::VolumeSupport::Supported, core::VolumeSupport::FileSystemLimited, core::VolumeSupport::ExceedsMaxSize, core::VolumeSupport::NotFixedLocalDisk})
      for (const bool remove : {false, true})
        QTest::newRow(qUtf8Printable(QStringLiteral("support-%1-remove-%2").arg(static_cast<int>(support)).arg(remove))) << support << remove;
  }
  void globalRegistryIntentDoesNotDependOnVolumeSupportOrLifetime() {
    QFETCH(core::VolumeSupport, support);
    QFETCH(bool, removeVolumes);
    app::ConfigurationEditSession session;
    core::SessionSnapshot baseline;
    baseline.overlay.type = core::OverlayType::RAM;
    session.globalEditor()->reset(baseline);
    core::VolumeInfo volume;
    volume.driveLetter = "C:";
    volume.support = support;
    auto editor = session.editVolume(volume, true);
    const QString path = QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\UwfData");
    QCOMPARE(editor->registry()->importAdd(path), app::ExclusionImportOutcome::Applied);
    if (removeVolumes) session.clearVolumes();
    QCOMPARE(session.pendingChanges().addRegistryExclusions, std::vector<std::string>{path.toStdString()});
    const auto report = session.importCommands(commands("uwfmgr.exe registry remove-exclusion HKEY_LOCAL_MACHINE\\SOFTWARE\\UwfData"));
    QCOMPARE(report.front().status, app::ImportResult::Status::Success);
    QVERIFY(session.pendingChanges().empty());
  }
};
}  // namespace

QTEST_APPLESS_MAIN(ConfigurationEditSessionTests)
#include "ConfigurationEditSessionTests.moc"
