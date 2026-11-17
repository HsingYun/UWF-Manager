// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QSignalSpy>
#include <QTest>
#include <memory>
#include <optional>

#include "application/localization/OverlayTypeText.h"
#include "presentation/viewmodels/configuration/ConfigurationViewModel.h"
#include "presentation/viewmodels/configuration/OverlaySettingsEditViewModel.h"
#include "simulation/app/ConfigurationState.h"
#include "simulation/app/OverlayPolicy.h"

namespace {
using namespace uwf;
using Policy = presentation::OverlayAdvancedViewModel;
using Maintenance = presentation::OverlayMaintenanceViewModel;
using Edit = presentation::OverlaySettingsEditViewModel;
class Observations final : public app::OverlayMaintenanceActions {
 public:
  static QString storageVolume() { return QStringLiteral("66666666-7777-8888-9999-aaaaaaaaaaaa"); }
  app::OverlayMaintenanceSnapshot state;
  int writes = 0;
  Observations() {
    state.filterEnabled = state.hormEnabled = state.overlayCommit = false;
    state.currentOverlayType = state.nextOverlayType = 1;
    state.currentFlags = state.nextFlags = app::OverlayFlags{3};
    state.maximumOverlaySize = 2048;
    state.volumesReadable = true;
    state.swapCapability.support = app::OverlayMaintenanceSupport::Supported;
    state.volumes = {{storageVolume(), QStringLiteral("D:"), QStringLiteral("Storage"), std::uint64_t{8} * 1024 * 1024 * 1024, true, {}, {}}};
    state.allVolumesProtected = true;
  }
  app::OverlayMaintenanceSnapshot observe(std::stop_token) override { return state; }
  std::optional<app::OverlayMaintenanceSnapshot> snapshot() const override { return state; }
  app::OverlayMaintenanceResult fullCommit(std::stop_token) override {
    ++writes;
    return {};
  }
  app::OverlayMaintenanceResult createSwap(const QString&, std::stop_token) override {
    ++writes;
    return {};
  }
  app::OverlayMaintenanceResult setHorm(bool, std::stop_token) override {
    ++writes;
    return {};
  }
};
struct Fixture {
  std::shared_ptr<app::OverlayPolicyEditor> editor = std::make_shared<app::OverlayPolicyEditor>();
  std::shared_ptr<Policy> policy = std::make_shared<Policy>(editor);
  std::shared_ptr<Observations> actions = std::make_shared<Observations>();
  std::shared_ptr<Maintenance> maintenance = std::make_shared<Maintenance>(actions);
  core::UwfSnapshot snapshot = uwf::simulation::application::editableSnapshot();
  app::OverlayPolicySnapshot baseline = uwf::simulation::application::diskPersistentPolicy();
  Fixture() {
    snapshot.current.filter.enabled = false;
    baseline.current.filterEnabled.value = false;
    editor->reset(baseline, snapshot, false);
    maintenance->setInteractionContext(true, false);
    maintenance->refresh();
  }
};
class OverlaySettingsEditViewModelTests final : public QObject {
  Q_OBJECT
 private slots:
  void isolatedEditsPublishOnlyOnSave_data() {
    QTest::addColumn<int>("scope");
    QTest::addColumn<bool>("save");
    for (const int scope : {1, 2, 3})
      for (const bool save : {false, true}) QTest::newRow(qUtf8Printable(QStringLiteral("scope-%1-save-%2").arg(scope).arg(save))) << scope << save;
  }
  void isolatedEditsPublishOnlyOnSave() {
    QFETCH(int, scope);
    QFETCH(bool, save);
    Fixture f;
    QTRY_VERIFY(!f.maintenance->busy());
    QSignalSpy policyChanged(f.policy.get(), &Policy::pendingChanged);
    QSignalSpy maintenanceChanged(f.maintenance.get(), &Maintenance::pendingChanged);
    {
      Edit edit(f.policy, f.maintenance);
      if ((scope & 1) != 0) edit.setPolicyValue(Policy::Field::Passthrough, false);
      if ((scope & 2) != 0) QVERIFY(edit.maintenance()->stageAllVolumesProtection(true));
      QVERIFY(edit.edited());
      QVERIFY(edit.canCommit());
      QVERIFY(f.editor->pendingChanges().empty());
      QVERIFY(f.maintenance->draft().empty());
      QCOMPARE(policyChanged.count() + maintenanceChanged.count(), 0);
      // Every outward publication must see the entire saved transaction.
      const auto verify = [&] {
        QCOMPARE(f.editor->pendingChanges().passthrough.has_value(), (scope & 1) != 0);
        QCOMPARE(f.maintenance->draft().allVolumesProtected.has_value(), (scope & 2) != 0);
      };
      connect(f.policy.get(), &Policy::pendingChanged, &edit, verify);
      connect(f.editor.get(), &app::OverlayPolicyEditor::pendingChanged, &edit, verify);
      connect(f.maintenance.get(), &Maintenance::pendingChanged, &edit, verify);
      connect(f.maintenance->editor().get(), &app::OverlayMaintenanceEditor::pendingChanged, &edit, verify);
      if (save) {
        QVERIFY(edit.commit());
        QVERIFY(!edit.edited());
        QVERIFY(!edit.commit());
      }
    }
    QCOMPARE(f.editor->pendingChanges().passthrough.has_value(), save && (scope & 1) != 0);
    QCOMPARE(f.maintenance->draft().allVolumesProtected.has_value(), save && (scope & 2) != 0);
    QCOMPARE(policyChanged.count(), save && (scope & 1) != 0 ? 1 : 0);
    QCOMPARE(maintenanceChanged.count(), save && (scope & 2) != 0 ? 1 : 0);
    QCOMPARE(f.actions->writes, 0);
  }
  void nestedDraftsShareObservationAndCommitOnlyToTheirParent() {
    Fixture f;
    QTRY_VERIFY(!f.maintenance->busy());
    Edit parent(f.policy, f.maintenance);
    {
      auto child = Maintenance::editDraft(parent.maintenance());
      QVERIFY(child->stageAllVolumesProtection(true));
      QVERIFY(parent.maintenance()->draft().empty());
      QVERIFY(child->commitDraft());
    }
    QCOMPARE(parent.maintenance()->draft().allVolumesProtected, std::optional<bool>{true});
    QVERIFY(f.maintenance->draft().empty());
    {
      auto canceled = Maintenance::editDraft(parent.maintenance());
      QVERIFY(canceled->stageAllVolumesProtection(false));
    }
    QCOMPARE(parent.maintenance()->draft().allVolumesProtected, std::optional<bool>{true});
    f.actions->state.hormEnabled = true;
    f.maintenance->refresh();
    QTRY_VERIFY(!parent.maintenance()->busy());
    QVERIFY(parent.maintenance()->hormActive());
    QVERIFY(!parent.canCommit());
    QVERIFY(!parent.conflicts().isEmpty());
    QVERIFY(f.maintenance->draft().empty());
    QCOMPARE(f.actions->writes, 0);
  }
  void storageCapacityFollowsTheGlobalDraftAcrossNestedEdits_data() {
    QTest::addColumn<bool>("saveChild");
    QTest::newRow("cancel-child") << false;
    QTest::newRow("save-child-cancel-parent") << true;
  }
  void storageCapacityFollowsTheGlobalDraftAcrossNestedEdits() {
    QFETCH(bool, saveChild);
    Fixture f;
    QTRY_VERIFY(!f.maintenance->busy());
    f.actions->state.maximumOverlaySize = 4096;
    f.maintenance->refresh();
    QTRY_VERIFY(!f.maintenance->busy());
    f.snapshot.next.overlay = {core::OverlayType::Disk, 4096, 2048, 3072};
    presentation::ConfigurationViewModel configuration{16384, {}, {}, {}, {}, nullptr, f.maintenance->editor()};
    configuration.resetSnapshot(f.snapshot, f.baseline, false);
    configuration.global()->editMaximumSize(8192);
    QCOMPARE(configuration.global()->pendingOverlay().maximumSizeMb, std::optional<std::uint32_t>{8192});
    const auto capacity = [](const int value) { return QStringLiteral("%1 MB").arg(value); };
    QCOMPARE(f.maintenance->storageDialogState().capacity, capacity(4096));
    {
      Edit parent(configuration.overlayPolicy(), f.maintenance);
      QCOMPARE(parent.maintenance()->storageDialogState().capacity, capacity(8192));
      {
        auto child = Maintenance::editDraft(parent.maintenance());
        child->selectStorageVolume(Observations::storageVolume());
        QCOMPARE(child->storageDialogState().capacity, capacity(8192));
        QSignalSpy changed(child.get(), &Maintenance::stateChanged);
        configuration.global()->editMaximumSize(12288);
        QVERIFY(!changed.empty());
        QCOMPARE(child->storageDialogState().capacity, capacity(12288));
        QCOMPARE(parent.maintenance()->storageDialogState().capacity, capacity(12288));
        if (saveChild)
          QVERIFY(child->submitStorageDraft());
        else
          QVERIFY(child->stageStorageLocation(Observations::storageVolume()));
      }
      QCOMPARE(parent.maintenance()->draft().storageVolume.has_value(), saveChild);
      QVERIFY(f.maintenance->draft().empty());
    }
    QVERIFY(f.maintenance->draft().empty());
    QCOMPARE(configuration.global()->pendingOverlay().maximumSizeMb, std::optional<std::uint32_t>{12288});
    QCOMPARE(f.actions->writes, 0);
  }
  void standaloneStorageCapacityKeepsFollowingMaintenanceObservations() {
    Fixture f;
    QTRY_VERIFY(!f.maintenance->busy());
    auto child = Maintenance::editDraft(f.maintenance);
    QCOMPARE(child->storageDialogState().capacity, QStringLiteral("2048 MB"));
    QSignalSpy changed(child.get(), &Maintenance::stateChanged);
    f.actions->state.maximumOverlaySize = 4096;
    f.maintenance->refresh();
    QTRY_VERIFY(!child->busy());
    QVERIFY(!changed.empty());
    QCOMPARE(f.maintenance->storageDialogState().capacity, QStringLiteral("4096 MB"));
    QCOMPARE(child->storageDialogState().capacity, QStringLiteral("4096 MB"));
    f.actions->state.maximumOverlaySize.reset();
    f.maintenance->refresh();
    QTRY_VERIFY(!child->busy());
    QCOMPARE(child->storageDialogState().capacity, QStringLiteral("Unknown"));
    QVERIFY(f.maintenance->draft().empty());
    QCOMPARE(f.actions->writes, 0);
  }
  void storageCapacityUsesTheStorageTargetAfterTheGlobalSizeEditIsRemoved() {
    Fixture f;
    QTRY_VERIFY(!f.maintenance->busy());
    presentation::ConfigurationViewModel configuration{16384, {}, {}, {}, {}, nullptr, f.maintenance->editor()};
    configuration.resetSnapshot(f.snapshot, f.baseline, false);
    Edit parent(configuration.overlayPolicy(), f.maintenance);
    auto child = Maintenance::editDraft(parent.maintenance());
    child->selectStorageVolume(Observations::storageVolume());
    QCOMPARE(child->storageDialogState().capacity, QStringLiteral("2048 MB"));
    configuration.global()->editMaximumSize(12288);
    QCOMPARE(child->storageDialogState().capacity, QStringLiteral("12288 MB"));
    configuration.global()->editMaximumSize(f.snapshot.next.overlay.maximumSizeMb);
    QVERIFY(!configuration.global()->pendingOverlay().maximumSizeMb);
    QCOMPARE(child->storageDialogState().capacity, QStringLiteral("2048 MB"));
    QVERIFY(f.maintenance->draft().empty());
    QCOMPARE(f.actions->writes, 0);
  }
  void concurrentChangesAreMergedByFieldAndCancelNeverRestoresOldState_data() {
    QTest::addColumn<bool>("save");
    QTest::newRow("save-preserves-unedited-fields") << true;
    QTest::newRow("cancel-preserves-external-changes") << false;
  }
  void concurrentChangesAreMergedByFieldAndCancelNeverRestoresOldState() {
    QFETCH(bool, save);
    Fixture f;
    QTRY_VERIFY(!f.maintenance->busy());
    {
      Edit edit(f.policy, f.maintenance);
      edit.setPolicyValue(Policy::Field::Passthrough, false);
      QVERIFY(edit.maintenance()->stageAllVolumesProtection(true));
      auto external = f.policy->beginEdit();
      Policy::setValue(external, Policy::Field::Persistent, false);
      QVERIFY(f.policy->commit(external));
      QVERIFY(f.maintenance->stageStorageLocation(Observations::storageVolume()));
      QVERIFY(edit.canCommit());
      if (save) QVERIFY(edit.commit());
    }
    QCOMPARE(f.editor->beginEdit().intent.persistent, std::optional<bool>{false});
    QVERIFY(!f.editor->pendingChanges().persistent.has_value());
    QCOMPARE(f.editor->pendingChanges().passthrough.has_value(), save);
    QCOMPARE(f.maintenance->draft().storageVolume, std::optional{Observations::storageVolume()});
    QCOMPARE(f.maintenance->draft().allVolumesProtected.has_value(), save);
  }
  void concurrentChangesToTheSameMaintenanceFieldRequireReopening() {
    Fixture f;
    QTRY_VERIFY(!f.maintenance->busy());
    Edit edit(f.policy, f.maintenance);
    edit.setPolicyValue(Policy::Field::Passthrough, false);
    QVERIFY(edit.maintenance()->stageAllVolumesProtection(true));
    QVERIFY(f.maintenance->stageAllVolumesProtection(false));
    QVERIFY(!edit.canCommit());
    QVERIFY(!edit.conflicts().isEmpty());
    QVERIFY(!edit.commit());
    QVERIFY(f.editor->pendingChanges().empty());
    QCOMPARE(f.maintenance->draft().allVolumesProtected, std::optional<bool>{false});
    edit.revert();
    QVERIFY(edit.canCommit());
    QVERIFY(edit.commit());
    QVERIFY(f.maintenance->draft().empty());
  }
  void refreshedPrerequisitesGateNewIntentsButAllowRemovingOldIntents_data() {
    QTest::addColumn<int>("intent");
    QTest::addColumn<int>("failure");
    QTest::addColumn<bool>("removing");
    for (const int intent : {0, 1, 2, 3})
      for (const int failure : {0, 1, 2})
        for (const bool removing : {false, true})
          QTest::newRow(qUtf8Printable(QStringLiteral("intent-%1-failure-%2-remove-%3").arg(intent).arg(failure).arg(removing)))
              << intent << failure << removing;
  }
  void refreshedPrerequisitesGateNewIntentsButAllowRemovingOldIntents() {
    QFETCH(int, intent);
    QFETCH(int, failure);
    QFETCH(bool, removing);
    Fixture f;
    QTRY_VERIFY(!f.maintenance->busy());
    auto& state = f.actions->state;
    state.volumesReadable = true;
    state.volumes = {{Observations::storageVolume(), QStringLiteral("D:"), QStringLiteral("Storage"), std::uint64_t{8} * 1024 * 1024 * 1024, true, {}, {}}};
    if (intent >= 2) {
      state.filterEnabled = true;
      state.currentOverlayType = 0;
      state.currentFlags = app::OverlayFlags{app::OverlayFlags::ReadOnlyMedia};
      state.hibernationAvailable = true;
      state.romDisposition = 0;
    }
    f.maintenance->refresh();
    QTRY_VERIFY(!f.maintenance->busy());
    const auto stage = [&](Maintenance& model) {
      if (intent == 0) return model.stageAllVolumesProtection(true);
      if (intent == 1) return model.stageStorageLocation(Observations::storageVolume());
      if (intent == 2) return model.stageHorm(true);
      return model.stageFullCommit();
    };
    if (removing) QVERIFY(stage(*f.maintenance));
    auto edit = Maintenance::editDraft(f.maintenance);
    if (removing)
      edit->discardDraft();
    else
      QVERIFY(stage(*edit));
    QVERIFY(edit->canCommitDraft());
    const auto unsupported = app::OverlayMaintenanceSupport::Unsupported;
    if (intent == 0) {
      if (failure == 0) state.hormEnabled = true;
      if (failure == 1) state.protectAllCapability.support = unsupported;
      if (failure == 2) state.hormEnabled.reset();
    } else if (intent == 1) {
      if (failure == 0) state.volumes.front().eligible = false;
      if (failure == 1) state.swapCapability.support = unsupported;
      if (failure == 2) state.filterEnabled.reset();
    } else if (intent == 2) {
      if (failure == 0) state.allVolumesProtected = false;
      if (failure == 1) state.hormCapability.support = unsupported;
      if (failure == 2) state.romDisposition.reset();
    } else {
      if (failure == 0) state.currentFlags = app::OverlayFlags{0};
      if (failure == 1) state.fullCommitCapability.support = unsupported;
      if (failure == 2) state.overlayCommit.reset();
    }
    f.maintenance->refresh();
    QTRY_VERIFY(!edit->busy());
    QCOMPARE(edit->canCommitDraft(), removing);
    QCOMPARE(edit->draftCommitDisabledReason().isEmpty(), removing);
    QCOMPARE(edit->commitDraft(), removing);
    QVERIFY(f.maintenance->draft().empty());
    QCOMPARE(f.actions->writes, 0);
  }
  void revertEditsThePrivateTransaction_data() {
    QTest::addColumn<bool>("save");
    QTest::newRow("cancel-keeps-opening-draft") << false;
    QTest::newRow("save-removes-opening-draft") << true;
  }
  void revertEditsThePrivateTransaction() {
    QFETCH(bool, save);
    Fixture f;
    QTRY_VERIFY(!f.maintenance->busy());
    auto opening = f.policy->beginEdit();
    Policy::setValue(opening, Policy::Field::Passthrough, false);
    QVERIFY(f.policy->commit(opening));
    QVERIFY(f.maintenance->stageAllVolumesProtection(true));
    Edit edit(f.policy, f.maintenance);
    edit.revert();
    QVERIFY(edit.edited());
    QCOMPARE(edit.pendingCount(), 0);
    QVERIFY(f.editor->pendingChanges().passthrough.has_value());
    QVERIFY(f.maintenance->hasPendingChanges());
    if (save) QVERIFY(edit.commit());
    QCOMPARE(f.editor->pendingChanges().empty(), save);
    QCOMPARE(f.maintenance->draft().empty(), save);
  }
  void nextTypeAndFieldRequirementsUseThePrivateStorageDraft() {
    Fixture f;
    QTRY_VERIFY(!f.maintenance->busy());
    f.snapshot.next.overlay.type = core::OverlayType::RAM;
    f.baseline.next.overlayType.value = 0;
    f.baseline.next.flags.value = app::OverlayFlags{0};
    f.editor->reset(f.baseline, f.snapshot, false);
    f.actions->state.nextOverlayType = 0;
    f.actions->state.nextFlags = app::OverlayFlags{0};
    f.maintenance->refresh();
    QTRY_VERIFY(!f.maintenance->busy());
    Edit edit(f.policy, f.maintenance);
    const auto ram = app::overlayTypeText(core::OverlayType::RAM);
    const auto disk = app::overlayTypeText(core::OverlayType::Disk);
    QCOMPARE(f.policy->targetTypeText(edit.policyEdit()), ram);
    QVERIFY(!f.policy->canEditField(edit.policyEdit(), Policy::Field::Persistent));
    QVERIFY(f.policy->canEditField(edit.policyEdit(), Policy::Field::ReadOnlyMedia));

    QVERIFY(edit.maintenance()->stageStorageLocation(Observations::storageVolume()));
    QCOMPARE(f.policy->targetTypeText(edit.policyEdit()), disk);
    QCOMPARE(f.policy->targetTypeText(f.policy->beginEdit()), ram);
    QVERIFY(f.policy->canEditField(edit.policyEdit(), Policy::Field::Persistent));
    QVERIFY(!f.policy->canEditField(edit.policyEdit(), Policy::Field::ReadOnlyMedia));
    QVERIFY(f.policy->disabledReason(edit.policyEdit(), Policy::Field::Persistent).isEmpty());
    QVERIFY(!f.policy->disabledReason(edit.policyEdit(), Policy::Field::ReadOnlyMedia).isEmpty());
    QCOMPARE(f.policy->currentState(Policy::Field::Persistent), Policy::ValueState::On);
    QCOMPARE(Policy::value(edit.policyEdit(), Policy::Field::Persistent), std::optional{false});
    QVERIFY(f.maintenance->draft().empty());

    edit.setPolicyValue(Policy::Field::Persistent, true);
    QVERIFY(edit.canCommit());
    QCOMPARE(Policy::value(edit.policyEdit(), Policy::Field::Persistent), std::optional{true});
    QCOMPARE(f.actions->writes, 0);
  }
};
}  // namespace
QTEST_GUILESS_MAIN(OverlaySettingsEditViewModelTests)
#include "OverlaySettingsEditViewModelTests.moc"
