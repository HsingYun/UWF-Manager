// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QSignalSpy>
#include <QTest>

#include "application/configuration/contracts/ConfigurationPlan.h"
#include "application/configuration/editing/OverlaySettingsEditSession.h"
#include "support/app/OverlayPolicyFixture.h"

namespace {
using namespace uwf;
using Field = app::OverlayPolicyField;
using Session = app::OverlaySettingsEditSession;
struct Fixture {
  static QString storageVolume() { return QStringLiteral("66666666-7777-8888-9999-aaaaaaaaaaaa"); }
  std::shared_ptr<app::OverlayPolicyEditor> policy = std::make_shared<app::OverlayPolicyEditor>();
  std::shared_ptr<app::OverlayMaintenanceEditor> maintenance = std::make_shared<app::OverlayMaintenanceEditor>();
  core::UwfSnapshot snapshot;
  app::OverlayPolicySnapshot baseline = test::application::diskPersistentPolicy();
  app::OverlayMaintenanceSnapshot observed;
  Fixture() {
    snapshot.uwfAvailable = snapshot.elevated = true;
    snapshot.current.overlay.type = snapshot.next.overlay.type = core::OverlayType::Disk;
    snapshot.current.overlay.maximumSizeMb = snapshot.next.overlay.maximumSizeMb = 2048;
    baseline.current.filterEnabled.value = false;
    policy->reset(baseline, snapshot, false);
    observed.filterEnabled = observed.hormEnabled = observed.overlayCommit = false;
    observed.currentOverlayType = observed.nextOverlayType = 1;
    observed.currentFlags = observed.nextFlags = app::OverlayFlags{3};
    observed.maximumOverlaySize = 2048;
    observed.volumesReadable = true;
    observed.swapCapability.support = app::OverlayMaintenanceSupport::Supported;
    observed.volumes = {{storageVolume(), QStringLiteral("D:"), QStringLiteral("Storage"), std::uint64_t{8} * 1024 * 1024 * 1024, true, {}, {}}};
    observed.allVolumesProtected = true;
    maintenance->setInteractionContext(true, false);
    refresh();
  }
  void refresh() { maintenance->finishObservation(*maintenance->beginObservation(), observed); }
};
class OverlaySettingsEditSessionTests final : public QObject {
  Q_OBJECT
 private slots:
  void combinedDraftPublishesOnlyItsCompleteCommit_data() {
    QTest::addColumn<bool>("save");
    QTest::newRow("save") << true;
    QTest::newRow("cancel") << false;
  }
  void combinedDraftPublishesOnlyItsCompleteCommit() {
    QFETCH(bool, save);
    Fixture f;
    QSignalSpy policyChanged(f.policy.get(), &app::OverlayPolicyEditor::pendingChanged);
    QSignalSpy maintenanceChanged(f.maintenance.get(), &app::OverlayMaintenanceEditor::pendingChanged);
    {
      Session session(f.policy, f.maintenance);
      session.setPolicyValue(Field::Passthrough, false);
      QVERIFY(session.maintenance()->stageAllVolumesProtection(true));
      QVERIFY(session.canCommit());
      QVERIFY(f.policy->pendingChanges().empty());
      QVERIFY(f.maintenance->draft().empty());
      const auto verify = [&] {
        QCOMPARE(f.policy->pendingChanges().passthrough, std::optional{false});
        QCOMPARE(f.maintenance->draft().allVolumesProtected, std::optional{true});
      };
      connect(f.policy.get(), &app::OverlayPolicyEditor::pendingChanged, &session, verify);
      connect(f.maintenance.get(), &app::OverlayMaintenanceEditor::pendingChanged, &session, verify);
      if (save) {
        QVERIFY(session.commit());
        QVERIFY(!session.edited());
        QVERIFY(!session.commit());
      }
    }
    QCOMPARE(f.policy->pendingChanges().passthrough.has_value(), save);
    QCOMPARE(f.maintenance->draft().allVolumesProtected.has_value(), save);
    QCOMPARE(policyChanged.size(), save ? 1 : 0);
    QCOMPARE(maintenanceChanged.size(), save ? 1 : 0);
  }
  void unrelatedSharedEditsRebaseWithoutLosingPrivateIntent() {
    Fixture f;
    Session session(f.policy, f.maintenance);
    session.setPolicyValue(Field::Passthrough, false);
    QVERIFY(session.maintenance()->stageAllVolumesProtection(true));
    auto policy = f.policy->beginEdit();
    app::OverlayPolicyEditor::setValue(policy, Field::Persistent, false);
    QVERIFY(f.policy->commit(policy));
    QVERIFY(f.maintenance->stageStorageLocation(Fixture::storageVolume()));
    const auto maintenance = f.maintenance->draft();
    QCOMPARE(app::OverlayPolicyEditor::value(session.policyEdit().values, Field::Persistent), std::optional{false});
    QCOMPARE(session.maintenance()->draft().storageVolume, maintenance.storageVolume);
    QVERIFY(session.canCommit());
    QVERIFY(session.commit());
    QCOMPARE(f.policy->pendingChanges().passthrough, std::optional{false});
    QCOMPARE(f.policy->beginEdit().intent.persistent, std::optional{false});
    QVERIFY(!f.policy->pendingChanges().persistent.has_value());
    QCOMPARE(f.maintenance->draft().storageVolume, maintenance.storageVolume);
    QCOMPARE(f.maintenance->draft().allVolumesProtected, std::optional{true});
  }
  void togglingAfterAConvergentSharedEditUsesTheUpdatedSource() {
    Fixture f;
    Session session(f.policy, f.maintenance);
    session.setPolicyValue(Field::Passthrough, false);
    auto external = f.policy->beginEdit();
    app::OverlayPolicyEditor::setValue(external, Field::Passthrough, false);
    QVERIFY(f.policy->commit(external));
    QCOMPARE(session.policyEdit().values.passthrough, std::optional{false});
    QVERIFY(session.conflicts().empty());

    session.setPolicyValue(Field::Passthrough, true);
    QCOMPARE(session.policyEdit().values.passthrough, std::optional{true});
    QVERIFY(session.canCommit());
    QCOMPARE(f.policy->beginEdit().values.passthrough, std::optional{false});
    QVERIFY(session.commit());
    QCOMPARE(f.policy->beginEdit().values.passthrough, std::optional{true});
    QVERIFY(!f.policy->pendingChanges().passthrough.has_value());
  }
  void conflictsAndFreshPrerequisitesBlockBothDestinations_data() {
    QTest::addColumn<int>("restriction");
    QTest::newRow("same-maintenance-field") << 0;
    QTest::newRow("policy-generation") << 1;
    QTest::newRow("horm-became-active") << 2;
  }
  void conflictsAndFreshPrerequisitesBlockBothDestinations() {
    QFETCH(int, restriction);
    Fixture f;
    Session session(f.policy, f.maintenance);
    session.setPolicyValue(Field::Passthrough, false);
    QVERIFY(session.maintenance()->stageAllVolumesProtection(true));
    if (restriction == 0) {
      QVERIFY(f.maintenance->stageAllVolumesProtection(false));
    } else if (restriction == 1) {
      f.policy->reset(f.baseline, f.snapshot, false);
    } else {
      f.observed.hormEnabled = true;
      f.refresh();
    }
    const auto policy = f.policy->pendingChanges();
    const auto maintenance = f.maintenance->draft();
    QVERIFY(!session.conflicts().empty());
    QVERIFY(!session.canCommit());
    QVERIFY(!session.commit());
    QCOMPARE(f.policy->pendingChanges(), policy);
    QCOMPARE(f.maintenance->draft(), maintenance);
  }
  void revertRemovesOnlyThePrivateOpeningIntentsUntilCommitted() {
    Fixture f;
    QVERIFY(f.policy->importChanges({.passthrough = false}));
    QVERIFY(f.maintenance->stageAllVolumesProtection(true));
    Session session(f.policy, f.maintenance);
    session.revert();
    QVERIFY(session.edited());
    QCOMPARE(session.pendingCount(), 0);
    QVERIFY(!f.policy->pendingChanges().empty());
    QVERIFY(!f.maintenance->draft().empty());
    QVERIFY(session.commit());
    QVERIFY(f.policy->pendingChanges().empty());
    QVERIFY(f.maintenance->draft().empty());
  }
  void togglingBackAfterRevertMatchesSavingTheRevertedTarget_data() {
    QTest::addColumn<int>("openingPersistentIntent");
    QTest::addColumn<bool>("toggleBack");
    QTest::newRow("storage-direct-save") << -1 << false;
    QTest::newRow("storage-toggle-back") << -1 << true;
    QTest::newRow("storage-and-explicit-on-direct-save") << 1 << false;
    QTest::newRow("storage-and-explicit-on-toggle-back") << 1 << true;
    QTest::newRow("storage-and-explicit-off-direct-save") << 0 << false;
    QTest::newRow("storage-and-explicit-off-toggle-back") << 0 << true;
  }
  void togglingBackAfterRevertMatchesSavingTheRevertedTarget() {
    QFETCH(int, openingPersistentIntent);
    QFETCH(bool, toggleBack);
    Fixture f;
    f.policy->setMaintenanceEditor(f.maintenance);
    QVERIFY(f.maintenance->stageStorageLocation(Fixture::storageVolume()));
    if (openingPersistentIntent >= 0) QVERIFY(f.policy->importChanges({.persistent = openingPersistentIntent != 0}));
    Session session(f.policy, f.maintenance);
    QCOMPARE(session.policyEdit().values.persistent, std::optional{openingPersistentIntent == 1});
    const auto sharedIntent = f.policy->beginEdit().intent;
    const auto sharedMaintenance = f.maintenance->draft();

    session.revert();
    QCOMPARE(session.policyEdit().values.persistent, std::optional{true});
    QVERIFY(session.policyEdit().intent.empty());
    QVERIFY(session.maintenance()->draft().empty());
    QCOMPARE(session.pendingCount(), 0);
    if (toggleBack) {
      session.setPolicyValue(Field::Persistent, false);
      QCOMPARE(session.policyEdit().values.persistent, std::optional{false});
      session.setPolicyValue(Field::Persistent, true);
      QCOMPARE(session.policyEdit().values.persistent, std::optional{true});
    }
    QVERIFY(session.policyEdit().intent.empty());
    QCOMPARE(session.pendingCount(), 0);
    QVERIFY(session.canCommit());
    QCOMPARE(f.policy->beginEdit().intent, sharedIntent);
    QCOMPARE(f.maintenance->draft(), sharedMaintenance);
    QVERIFY(session.commit());
    QVERIFY(f.policy->beginEdit().intent.empty());
    QVERIFY(f.policy->pendingChanges().empty());
    QVERIFY(f.maintenance->draft().empty());

    Session reopened(f.policy, f.maintenance);
    QVERIFY(reopened.maintenance()->stageStorageLocation(Fixture::storageVolume()));
    QCOMPARE(reopened.policyEdit().values.persistent, std::optional{false});
    QVERIFY(reopened.policyEdit().intent.empty());
    QCOMPARE(reopened.pendingCount(), 1);
    QVERIFY(reopened.canCommit());
    QVERIFY(f.policy->pendingChanges().empty());
    QVERIFY(f.maintenance->draft().empty());
  }
  void localStorageProjectionAndCancellationDoNotPublishPolicyIntents_data() {
    QTest::addColumn<bool>("restorePersistent");
    QTest::newRow("storage-side-effect-only") << false;
    QTest::newRow("storage-and-explicit-restore") << true;
  }
  void localStorageProjectionAndCancellationDoNotPublishPolicyIntents() {
    QFETCH(bool, restorePersistent);
    Fixture f;
    {
      Session session(f.policy, f.maintenance);
      QVERIFY(session.maintenance()->stageStorageLocation(Fixture::storageVolume()));
      const auto edit = session.policyEdit();
      QCOMPARE(f.policy->targetType(edit), core::OverlayType::Disk);
      QCOMPARE(edit.values.persistent, std::optional{false});
      QVERIFY(edit.intent.empty());
      QCOMPARE(f.policy->current(Field::Persistent), std::optional{true});
      QCOMPARE(session.pendingCount(), 1);
      QVERIFY(session.canCommit());
      QVERIFY(f.policy->pendingChanges().empty());
      QVERIFY(f.maintenance->draft().empty());
      if (restorePersistent) {
        session.setPolicyValue(Field::Persistent, true);
        QCOMPARE(session.policyEdit().intent.persistent, std::optional{true});
        QVERIFY(session.canCommit());
      }
    }
    QVERIFY(f.policy->pendingChanges().empty());
    QVERIFY(f.maintenance->draft().empty());
  }
  void restoringPersistentAfterLocalStorageKeepsTheExplicitIntentAndPublishesAtomically() {
    Fixture f;
    Session session(f.policy, f.maintenance);
    QVERIFY(session.maintenance()->stageStorageLocation(Fixture::storageVolume()));
    QCOMPARE(f.policy->disabledReason(session.policyEdit(), Field::Persistent), app::OverlayPolicyIssue::None);
    session.setPolicyValue(Field::Persistent, true);
    QCOMPARE(session.policyEdit().values.persistent, std::optional{true});
    QCOMPARE(session.policyEdit().intent.persistent, std::optional{true});
    QCOMPARE(session.pendingCount(), 2);
    QVERIFY(session.canCommit());
    QVERIFY(f.policy->pendingChanges().empty());
    QVERIFY(f.maintenance->draft().empty());

    int publications = 0;
    const auto inspectCompleteTarget = [&] {
      ++publications;
      QCOMPARE(f.policy->pendingChanges().persistent, std::optional{true});
      QCOMPARE(f.maintenance->draft().storageVolume, std::optional{Fixture::storageVolume()});
      const auto plan = app::planApplicationConfiguration({{}, f.policy->pendingChanges(), f.maintenance->draft()}, f.snapshot, f.baseline, {}, f.observed);
      const auto target = app::configurationOverlayTarget(plan);
      QCOMPARE(target.overlay.type, core::OverlayType::Disk);
      QVERIFY(target.flags.has_value());
      QVERIFY(target.flags->persistent());
    };
    connect(f.policy.get(), &app::OverlayPolicyEditor::pendingChanged, &session, inspectCompleteTarget);
    connect(f.maintenance.get(), &app::OverlayMaintenanceEditor::pendingChanged, &session, inspectCompleteTarget);
    QVERIFY(session.commit());
    QVERIFY(publications >= 2);
    QVERIFY(!session.edited());
    QCOMPARE(f.policy->pendingChanges().persistent, std::optional{true});
    QCOMPARE(f.maintenance->draft().storageVolume, std::optional{Fixture::storageVolume()});
  }
  void localStorageChangesRamToDiskBeforeFieldAdmission() {
    Fixture f;
    f.snapshot.next.overlay.type = core::OverlayType::RAM;
    f.baseline.next.overlayType.value = 0;
    f.baseline.next.flags.value = app::OverlayFlags{0};
    f.policy->reset(f.baseline, f.snapshot, false);
    f.observed.nextOverlayType = 0;
    f.observed.nextFlags = app::OverlayFlags{0};
    f.refresh();
    Session session(f.policy, f.maintenance);
    QCOMPARE(f.policy->targetType(session.policyEdit()), core::OverlayType::RAM);
    QCOMPARE(f.policy->disabledReason(session.policyEdit(), Field::Persistent), app::OverlayPolicyIssue::PersistentNeedsDisk);
    QCOMPARE(f.policy->disabledReason(session.policyEdit(), Field::ReadOnlyMedia), app::OverlayPolicyIssue::None);

    QVERIFY(session.maintenance()->stageStorageLocation(Fixture::storageVolume()));
    QCOMPARE(f.policy->targetType(session.policyEdit()), core::OverlayType::Disk);
    QCOMPARE(f.policy->disabledReason(session.policyEdit(), Field::Persistent), app::OverlayPolicyIssue::None);
    QCOMPARE(f.policy->disabledReason(session.policyEdit(), Field::ReadOnlyMedia), app::OverlayPolicyIssue::MediaNeedsRam);
    session.setPolicyValue(Field::ReadOnlyMedia, true);
    QVERIFY(!session.policyEdit().intent.readOnlyMedia.has_value());
    session.setPolicyValue(Field::Persistent, true);
    QCOMPARE(session.policyEdit().intent.persistent, std::optional{true});
    QVERIFY(session.canCommit());
  }
  void removingAndRestagingStoragePreservesRestoreIntentUntilRevert() {
    Fixture f;
    Session session(f.policy, f.maintenance);
    QVERIFY(session.maintenance()->stageStorageLocation(Fixture::storageVolume()));
    session.setPolicyValue(Field::Persistent, true);
    session.maintenance()->discardDraft();
    QCOMPARE(session.policyEdit().intent.persistent, std::optional{true});
    QVERIFY(session.maintenance()->stageStorageLocation(Fixture::storageVolume()));
    QCOMPARE(session.policyEdit().values.persistent, std::optional{true});
    QCOMPARE(session.policyEdit().intent.persistent, std::optional{true});

    auto external = f.policy->beginEdit();
    app::OverlayPolicyEditor::setValue(external, Field::Passthrough, false);
    QVERIFY(f.policy->commit(external));
    QCOMPARE(session.policyEdit().intent.persistent, std::optional{true});
    QCOMPARE(session.policyEdit().values.passthrough, std::optional{false});

    session.revert();
    QVERIFY(session.policyEdit().intent.empty());
    QVERIFY(session.maintenance()->draft().empty());
    QVERIFY(session.maintenance()->stageStorageLocation(Fixture::storageVolume()));
    QCOMPARE(session.policyEdit().values.persistent, std::optional{false});
    QVERIFY(!session.policyEdit().intent.persistent.has_value());
    QCOMPARE(f.policy->pendingChanges().passthrough, std::optional{false});
    QVERIFY(f.maintenance->draft().empty());
  }
  void sameValueImportedPersistentIntentAppliesAfterAStorageChange() {
    Fixture f;
    QVERIFY(f.policy->importChanges({.persistent = true}));
    QVERIFY(f.policy->pendingChanges().empty());
    Session session(f.policy, f.maintenance);
    QVERIFY(session.maintenance()->stageStorageLocation(Fixture::storageVolume()));
    QCOMPARE(session.policyEdit().values.persistent, std::optional{true});
    QCOMPARE(session.policyEdit().intent.persistent, std::optional{true});
    QVERIFY(session.canCommit());
    QVERIFY(session.commit());
    QCOMPARE(f.policy->pendingChanges().persistent, std::optional{true});
    QCOMPARE(f.maintenance->draft().storageVolume, std::optional{Fixture::storageVolume()});
    const auto plan = app::planApplicationConfiguration({{}, f.policy->pendingChanges(), f.maintenance->draft()}, f.snapshot, f.baseline, {}, f.observed);
    const auto target = app::configurationOverlayTarget(plan);
    QVERIFY(target.flags.has_value());
    QVERIFY(target.flags->persistent());
  }
  void removingStorageKeepsAnExplicitPersistentOffChoice() {
    Fixture f;
    Session session(f.policy, f.maintenance);
    QVERIFY(session.maintenance()->stageStorageLocation(Fixture::storageVolume()));
    QCOMPARE(session.policyEdit().values.persistent, std::optional{false});
    QVERIFY(!session.policyEdit().intent.persistent.has_value());
    session.setPolicyValue(Field::Persistent, false);
    QCOMPARE(session.policyEdit().intent.persistent, std::optional{false});
    session.maintenance()->discardDraft();
    QVERIFY(session.maintenance()->draft().empty());
    QCOMPARE(session.policyEdit().values.persistent, std::optional{false});
    QCOMPARE(session.policyEdit().intent.persistent, std::optional{false});
    QCOMPARE(session.pendingCount(), 1);
    QVERIFY(session.canCommit());
    QVERIFY(session.commit());
    QCOMPARE(f.policy->pendingChanges().persistent, std::optional{false});
    QVERIFY(f.maintenance->draft().empty());
  }
  void togglingPersistentBackAfterLocalStorageDoesNotLeaveAnExplicitOffChoice() {
    Fixture f;
    Session session(f.policy, f.maintenance);
    QVERIFY(session.maintenance()->stageStorageLocation(Fixture::storageVolume()));
    QCOMPARE(session.policyEdit().values.persistent, std::optional{false});
    QVERIFY(session.policyEdit().intent.empty());
    session.setPolicyValue(Field::Persistent, true);
    QCOMPARE(session.policyEdit().intent.persistent, std::optional{true});
    session.setPolicyValue(Field::Persistent, false);
    QCOMPARE(session.policyEdit().values.persistent, std::optional{false});
    QVERIFY(session.policyEdit().intent.empty());
    QCOMPARE(session.pendingCount(), 1);
    QVERIFY(session.canCommit());

    session.maintenance()->discardDraft();
    QCOMPARE(session.policyEdit().values.persistent, std::optional{true});
    QVERIFY(session.policyEdit().intent.empty());
    QCOMPARE(session.pendingCount(), 0);
    QVERIFY(!session.edited());
    QVERIFY(!session.canCommit());
    QVERIFY(f.policy->pendingChanges().empty());
    QVERIFY(f.maintenance->draft().empty());
  }
  void togglingBackKeepsTheOpeningExplicitPersistentIntent_data() {
    QTest::addColumn<bool>("persistent");
    QTest::newRow("same-as-observed-on") << true;
    QTest::newRow("pending-off") << false;
  }
  void togglingBackKeepsTheOpeningExplicitPersistentIntent() {
    QFETCH(bool, persistent);
    Fixture f;
    QVERIFY(f.policy->importChanges({.persistent = persistent}));
    const auto sharedIntent = f.policy->beginEdit().intent;
    Session session(f.policy, f.maintenance);
    QCOMPARE(session.policyEdit().values.persistent, std::optional{persistent});
    QCOMPARE(session.policyEdit().intent.persistent, std::optional{persistent});
    session.setPolicyValue(Field::Persistent, !persistent);
    QVERIFY(session.edited());
    session.setPolicyValue(Field::Persistent, persistent);
    QCOMPARE(session.policyEdit().values.persistent, std::optional{persistent});
    QCOMPARE(session.policyEdit().intent, sharedIntent);
    QVERIFY(!session.edited());
    QVERIFY(!session.canCommit());

    QVERIFY(session.maintenance()->stageStorageLocation(Fixture::storageVolume()));
    QCOMPARE(session.policyEdit().values.persistent, std::optional{persistent});
    QCOMPARE(session.policyEdit().intent, sharedIntent);
    QCOMPARE(f.policy->beginEdit().intent, sharedIntent);
    QVERIFY(f.maintenance->draft().empty());
  }
};
}  // namespace
QTEST_GUILESS_MAIN(OverlaySettingsEditSessionTests)
#include "OverlaySettingsEditSessionTests.moc"
