// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QScopeGuard>
#include <QTest>
#include <QTranslator>
#include <algorithm>
#include <array>
#include <atomic>
#include <cstddef>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <thread>
#include <vector>

#include "application/configuration/contracts/ExclusionImportOutcome.h"
#include "application/configuration/editing/ConfigurationEditSession.h"
#include "application/configuration/exclusions/ExclusionEditor.h"
#include "application/configuration/execution/ConfigurationExecutor.h"
#include "application/configuration/overlay/OverlayPolicyConstraints.h"
#include "application/configuration/volumes/VolumeConfigurationEditor.h"
#include "application/staging/contracts/FileStagingStore.h"
#include "application/staging/persistence/FileStagingRegistry.h"
#include "application/uwf/adapters/UwfConfigurationRepository.h"
#include "application/uwf/adapters/UwfReadState.h"
#include "cases/presentation/viewmodels/common/ViewModelStateFixture.h"
#include "presentation/viewmodels/configuration/ApplyPlanViewModel.h"
#include "presentation/viewmodels/configuration/OverlayAdvancedViewModel.h"
#include "support/app/ConfigurationFixture.h"
#include "support/uwf/FakeUwfBackend.h"

namespace {
using namespace uwf;
using namespace uwf::test::viewmodels;

class FakeConfigurationActions final : public app::ConfigurationActions {
 public:
  app::ConfigurationApplyResult result;
  std::optional<core::UwfSnapshot> observedSnapshot;
  std::optional<app::ConfigurationPlan> observedPlan;
  int calls = 0;
  ApplyOperation prepareApply(const core::PendingChanges&, const core::UwfSnapshot& snapshot) override {
    ++calls;
    observedSnapshot = snapshot;
    return [value = result] { return value; };
  }
  ApplyOperation prepareApply(const app::ConfigurationPlan& plan) override {
    observedPlan = plan;
    return prepareApply(plan.changes, plan.snapshot);
  }
};
class ConfigurationReviewViewModelTests final : public QObject {
  Q_OBJECT
 private slots:
  void activationTextReflectsDynamicThresholdsAndMixedPlans_data();
  void activationTextReflectsDynamicThresholdsAndMixedPlans();
  void storageReviewUsesTheSharedOverlayTypeTranslation() {
    class DiskTranslation final : public QTranslator {
     public:
      bool isEmpty() const override { return false; }
      QString translate(const char*, const char* source, const char*, int) const override {
        return QByteArray(source) == QByteArrayLiteral("Disk") ? QStringLiteral("Localized Disk overlay") : QString{};
      }
    } translator;
    QCoreApplication::installTranslator(&translator);
    const auto restore = qScopeGuard([&] { QCoreApplication::removeTranslator(&translator); });
    app::ConfigurationPlan plan;
    plan.snapshot = snapshot();
    plan.maintenance.storageVolume = QStringLiteral("storage-volume");
    presentation::ApplyPlanViewModel model(plan, std::make_shared<FakeConfigurationActions>());
    QVERIFY(std::ranges::any_of(model.groups(), [](const auto& group) {
      return std::ranges::any_of(group.items, [](const auto& item) { return item.after == QStringLiteral("Localized Disk overlay"); });
    }));
  }
  void safeRestartUsesTheLatestRuntimeMode_data();
  void safeRestartUsesTheLatestRuntimeMode();
  void storageReviewShowsItsFinalTypeAndFlagEffectsWithoutAddingOperations_data();
  void storageReviewShowsItsFinalTypeAndFlagEffectsWithoutAddingOperations();
  void storageReviewPreservesSubGiBRamCapacity_data();
  void storageReviewPreservesSubGiBRamCapacity();
  void storageReviewDoesNotInventUnobservedFlagEffects();
  void conflictingStorageTypeRemainsVisibleAndCannotApply();
  void applyRetryAndReconciliationDoNotReplayWrites();
  void reviewUsesApplicationValidationForResetPrerequisites_data();
  void reviewUsesApplicationValidationForResetPrerequisites();
  void persistentResetExportUsesTheSelectedScopeWithoutExecuting_data();
  void persistentResetExportUsesTheSelectedScopeWithoutExecuting();
  void stagingCleanupRemainsApplicableWhenOverlayFlagsCannotBeRead();
  void romReviewPreservesStaging_data();
  void romReviewPreservesStaging();
  void configurationReviewUsesTheNextSessionBaselineAndRetainsNonCliChanges();
  void maintenancePreviewUsesNativeCommandsAndSeparatesNextScope_data();
  void maintenancePreviewUsesNativeCommandsAndSeparatesNextScope();
  void exportScopesRequireActualCommandsWithoutLosingNextSnapshotCoverage();
  void commandPreviewPreservesOmissionScopeWithoutExecuting();
  void conflictingMaintenanceDraftRemainsReviewableAndCannotApply();
  void batchProtectionReviewExportsCommandsAndKeepsPreviewClean();
  void batchProtectionReviewShowsInheritedTargetsBeforeExplicitExceptions();
  void hiddenProtectionReviewUsesTheMatchingUuidBaseline();
  void batchProtectionResultListsIndividualReceiptsWithoutHidingPartialFailure();
  void configurationReviewIdentifiesStagingDependencies_data();
  void configurationReviewIdentifiesStagingDependencies();
  void stagingPersistenceReviewDistinguishesCleanupFromRecovery_data();
  void stagingPersistenceReviewDistinguishesCleanupFromRecovery();
  void failedProtectionCreationSkipsDependentConfiguration_data();
  void failedProtectionCreationSkipsDependentConfiguration();
};

void ConfigurationReviewViewModelTests::activationTextReflectsDynamicThresholdsAndMixedPlans_data() {
  QTest::addColumn<int>("draft");
  QTest::addColumn<bool>("running");
  QTest::addColumn<QString>("expected");
  QTest::newRow("warning-live") << 0 << true << QStringLiteral("Takes effect when applied");
  QTest::newRow("warning-saved-until-uwf-starts") << 0 << false << QStringLiteral("Used when UWF starts");
  QTest::newRow("critical-live") << 1 << true << QStringLiteral("Takes effect when applied");
  QTest::newRow("critical-saved-until-uwf-starts") << 1 << false << QStringLiteral("Used when UWF starts");
  QTest::newRow("both-thresholds-live") << 2 << true << QStringLiteral("Takes effect when applied");
  QTest::newRow("capacity-needs-restart") << 3 << false << QStringLiteral("Takes effect after restart");
  QTest::newRow("threshold-and-capacity") << 4 << false << QStringLiteral("Mixed activation times");
  QTest::newRow("staging-already-durable") << 5 << true << QStringLiteral("Takes effect when applied");
  QTest::newRow("staging-adds-next-session-exclusion") << 6 << true << QStringLiteral("Mixed activation times");
}

void ConfigurationReviewViewModelTests::activationTextReflectsDynamicThresholdsAndMixedPlans() {
  QFETCH(int, draft);
  QFETCH(bool, running);
  QFETCH(QString, expected);
  auto state = snapshot();
  state.current.filter.enabled = state.next.filter.enabled = running;
  core::PendingChanges changes;
  if (draft == 0 || draft == 2 || draft == 4) changes.setOverlay.warningThresholdMb = 5000;
  if (draft == 1 || draft == 2) changes.setOverlay.criticalThresholdMb = 7000;
  if (draft == 3 || draft == 4) changes.setOverlay.maximumSizeMb = 16384;
  std::optional<app::FileStagingChanges> staging;
  if (draft >= 5) {
    staging.emplace();
    staging->after = {{app::FileStagingKind::File, QStringLiteral("C:\\Data\\settings.json")}};
    if (draft == 5) state.current.registryExclusions = state.next.registryExclusions = {std::string(app::kFileStagingRegistryRoot)};
  }
  const auto plan = app::planConfiguration(changes, state, staging);
  const auto actions = std::make_shared<FakeConfigurationActions>();
  for (const auto& operation : plan.operations())
    actions->result.steps.push_back({"Configuration", app::ConfigurationStepOutcome::Succeeded, app::WriteEffect::Confirmed, operation});
  presentation::ApplyPlanViewModel model(plan, actions);
  QCOMPARE(model.activationText(), expected);
  const auto thresholdText = running ? QStringLiteral("Takes effect when applied") : QStringLiteral("Used when UWF starts");
  for (const auto& group : model.groups())
    for (const auto& item : group.items)
      if (item.title == QStringLiteral("Warning threshold") || item.title == QStringLiteral("Critical threshold")) QCOMPARE(item.description, thresholdText);
  QVERIFY(model.canApply());
  model.apply();
  QTRY_VERIFY(!model.applying());
  QCOMPARE(actions->calls, 1);
  QCOMPARE(model.resultDescription(), QStringLiteral("The configuration has been written to the system.\n") + expected);
}

void ConfigurationReviewViewModelTests::storageReviewShowsItsFinalTypeAndFlagEffectsWithoutAddingOperations_data() {
  QTest::addColumn<int>("initialType");
  QTest::addColumn<quint32>("initialFlags");
  QTest::addColumn<int>("requestedPersistent");  // -1 means unchanged.
  QTest::addColumn<int>("requestedPassthrough");
  QTest::addColumn<bool>("explicitDisk");
  QTest::addColumn<int>("persistentAfter");  // -1 means no review row.
  QTest::addColumn<int>("passthroughAfter");
  QTest::newRow("disk-storage-clears-persistence") << 1 << quint32{3} << -1 << -1 << false << 0 << -1;
  QTest::newRow("disk-storage-also-enables-passthrough") << 1 << quint32{2} << -1 << -1 << false << 0 << 1;
  QTest::newRow("ram-storage-becomes-disk") << 0 << quint32{0} << -1 << -1 << false << -1 << 1;
  QTest::newRow("already-ram-keeps-its-persistent-bit") << 0 << quint32{2} << -1 << -1 << false << -1 << 1;
  QTest::newRow("explicit-persistence-restores-the-reviewed-value") << 1 << quint32{3} << 1 << -1 << false << 1 << -1;
  QTest::newRow("explicit-passthrough-overrides-storage-default") << 1 << quint32{3} << -1 << 0 << false << 0 << 0;
  QTest::newRow("explicit-type-and-storage-share-one-type-row") << 1 << quint32{3} << -1 << -1 << true << 0 << -1;
}

void ConfigurationReviewViewModelTests::storageReviewShowsItsFinalTypeAndFlagEffectsWithoutAddingOperations() {
  QFETCH(int, initialType);
  QFETCH(quint32, initialFlags);
  QFETCH(int, requestedPersistent);
  QFETCH(int, requestedPassthrough);
  QFETCH(bool, explicitDisk);
  QFETCH(int, persistentAfter);
  QFETCH(int, passthroughAfter);
  app::ConfigurationPlan plan;
  plan.snapshot = snapshot();
  plan.snapshot.current.filter.enabled = plan.snapshot.next.filter.enabled = false;
  plan.snapshot.current.overlay.type = plan.snapshot.next.overlay.type = static_cast<core::OverlayType>(initialType);
  plan.snapshot.current.overlay.maximumSizeMb = plan.snapshot.next.overlay.maximumSizeMb = 2048;
  plan.maintenance.storageVolume = QString::fromLatin1(uwf::test::application::fixtureVolumeDName);
  auto& observed = plan.maintenanceSnapshot.emplace();
  observed.filterEnabled = false;
  observed.currentOverlayType = observed.nextOverlayType = initialType;
  observed.maximumOverlaySize = 2048;
  observed.currentFlags = observed.nextFlags = app::OverlayFlags{initialFlags};
  observed.volumes.push_back({*plan.maintenance.storageVolume, QStringLiteral("D:"), {}, 0, true, {}, {}});
  if (requestedPersistent >= 0) plan.overlayPolicy.persistent = requestedPersistent != 0;
  if (requestedPassthrough >= 0) plan.overlayPolicy.passthrough = requestedPassthrough != 0;
  if (explicitDisk) plan.changes.setOverlay.type = core::OverlayType::Disk;
  const auto operationCount = std::size_t{1} + static_cast<std::size_t>(requestedPersistent >= 0) + static_cast<std::size_t>(requestedPassthrough >= 0) +
                              static_cast<std::size_t>(explicitDisk);
  const auto actions = std::make_shared<FakeConfigurationActions>();
  presentation::ApplyPlanViewModel model(plan, actions);
  QVERIFY(model.canApply());
  QCOMPARE(model.changeCount(), operationCount);
  QCOMPARE(plan.operations().size(), operationCount);
  QCOMPARE(actions->calls, 0);
  const auto matching = [&](const QString& title) {
    QList<presentation::ApplyPlanViewModel::Change> result;
    for (const auto& group : model.groups())
      for (const auto& item : group.items)
        if (item.title == title) result.append(item);
    return result;
  };
  const auto type = matching(QStringLiteral("Overlay type"));
  QCOMPARE(type.size(), initialType == 0 || explicitDisk ? qsizetype{1} : qsizetype{0});
  if (!type.isEmpty()) {
    QCOMPARE(type.constFirst().before, initialType == 0 ? QStringLiteral("RAM") : QStringLiteral("Disk"));
    QCOMPARE(type.constFirst().after, QStringLiteral("Disk"));
  }
  const auto verifyFlag = [&](const presentation::OverlayAdvancedViewModel::Field field, const bool before, const int after) {
    const auto rows = matching(presentation::OverlayAdvancedViewModel::fieldName(field));
    QCOMPARE(rows.size(), after < 0 ? qsizetype{0} : qsizetype{1});
    if (rows.isEmpty()) return;
    QCOMPARE(rows.constFirst().before, before ? QStringLiteral("On") : QStringLiteral("Off"));
    QCOMPARE(rows.constFirst().after, after ? QStringLiteral("On") : QStringLiteral("Off"));
  };
  verifyFlag(presentation::OverlayAdvancedViewModel::Field::Persistent, (initialFlags & app::OverlayFlags::Persistent) != 0, persistentAfter);
  verifyFlag(presentation::OverlayAdvancedViewModel::Field::Passthrough, (initialFlags & app::OverlayFlags::Passthrough) != 0, passthroughAfter);
  QVERIFY(matching(presentation::OverlayAdvancedViewModel::fieldName(presentation::OverlayAdvancedViewModel::Field::ReadOnlyMedia)).isEmpty());
  const auto storage = matching(QStringLiteral("Configure storage location"));
  QCOMPARE(storage.size(), qsizetype{1});
  QCOMPARE(storage.constFirst().after, QStringLiteral("D:"));
  QVERIFY(storage.constFirst().description.contains(QStringLiteral("next startup")));
  QVERIFY(!storage.constFirst().description.contains(QStringLiteral("swapfile"), Qt::CaseInsensitive));
}

void ConfigurationReviewViewModelTests::storageReviewDoesNotInventUnobservedFlagEffects() {
  app::ConfigurationPlan plan;
  plan.snapshot = snapshot();
  plan.snapshot.current.filter.enabled = false;
  plan.snapshot.current.overlay.maximumSizeMb = plan.snapshot.next.overlay.maximumSizeMb = 2048;
  plan.maintenance.storageVolume = QString::fromLatin1(uwf::test::application::fixtureVolumeDName);
  plan.policySnapshot.next.flags.status = app::PolicyObservationStatus::Failed;
  auto& observed = plan.maintenanceSnapshot.emplace();
  observed.filterEnabled = false;
  observed.currentOverlayType = observed.nextOverlayType = 0;
  observed.maximumOverlaySize = 2048;
  observed.currentFlags = observed.nextFlags = app::OverlayFlags{3};
  const auto actions = std::make_shared<FakeConfigurationActions>();
  presentation::ApplyPlanViewModel model(plan, actions);
  QVERIFY(!model.canApply());
  QCOMPARE(model.changeCount(), std::size_t{1});
  QVERIFY(std::ranges::none_of(model.groups(), [](const auto& group) { return group.title == QStringLiteral("Overlay advanced settings"); }));
  QVERIFY(!model.warning().isEmpty());
  QCOMPARE(actions->calls, 0);
}

void ConfigurationReviewViewModelTests::storageReviewPreservesSubGiBRamCapacity_data() {
  QTest::addColumn<quint32>("requestedSize");
  QTest::newRow("storage-only-512") << quint32{0};
  QTest::newRow("resize-768-and-storage") << quint32{768};
}
void ConfigurationReviewViewModelTests::storageReviewPreservesSubGiBRamCapacity() {
  QFETCH(quint32, requestedSize);
  app::ConfigurationPlan plan;
  plan.snapshot = snapshot();
  plan.snapshot.current.filter.enabled = plan.snapshot.next.filter.enabled = false;
  plan.snapshot.current.overlay = plan.snapshot.next.overlay = {core::OverlayType::RAM, 512, 128, 256};
  plan.maintenance.storageVolume = QString::fromLatin1(uwf::test::application::fixtureVolumeDName);
  auto& observed = plan.maintenanceSnapshot.emplace();
  observed.filterEnabled = observed.hormEnabled = observed.overlayCommit = false;
  observed.currentOverlayType = observed.nextOverlayType = 0;
  observed.maximumOverlaySize = 512;
  observed.currentFlags = observed.nextFlags = app::OverlayFlags{0};
  observed.volumesReadable = true;
  observed.volumes.push_back({*plan.maintenance.storageVolume, QStringLiteral("D:"), {}, 0, true, {}, {}});
  if (requestedSize) plan.changes.setOverlay.maximumSizeMb = requestedSize;
  const auto target = app::configurationOverlayTarget(plan).overlay;
  QCOMPARE(target.type, core::OverlayType::Disk);
  QCOMPARE(target.maximumSizeMb, requestedSize ? requestedSize : quint32{512});
  const auto actions = std::make_shared<FakeConfigurationActions>();
  presentation::ApplyPlanViewModel model(plan, actions);
  QVERIFY(model.canApply());
  QVERIFY(model.warning().isEmpty());
  QCOMPARE(model.changeCount(), requestedSize ? std::size_t{2} : std::size_t{1});
  const auto matching = [&](const QString& title) {
    QList<presentation::ApplyPlanViewModel::Change> rows;
    for (const auto& group : model.groups())
      for (const auto& item : group.items)
        if (item.title == title) rows.append(item);
    return rows;
  };
  const auto type = matching(QStringLiteral("Overlay type"));
  QCOMPARE(type.size(), qsizetype{1});
  QCOMPARE(type.constFirst().before, QStringLiteral("RAM"));
  QCOMPARE(type.constFirst().after, QStringLiteral("Disk"));
  const auto capacity = matching(QStringLiteral("Maximum overlay capacity"));
  QCOMPARE(capacity.size(), requestedSize ? qsizetype{1} : qsizetype{0});
  if (requestedSize) {
    QCOMPARE(capacity.constFirst().before, QStringLiteral("512 MB"));
    QCOMPARE(capacity.constFirst().after, QStringLiteral("768 MB"));
    QCOMPARE(capacity.constFirst().description, QStringLiteral("Disk"));
  }
  const auto storage = matching(QStringLiteral("Configure storage location"));
  QCOMPARE(storage.size(), qsizetype{1});
  QCOMPARE(storage.constFirst().after, QStringLiteral("D:"));
  QCOMPARE(model.snapshot().next.overlay.maximumSizeMb, std::uint32_t{512});
  QCOMPARE(actions->calls, 0);
}

void ConfigurationReviewViewModelTests::conflictingStorageTypeRemainsVisibleAndCannotApply() {
  app::ConfigurationPlan plan;
  plan.snapshot = snapshot();
  plan.snapshot.next.overlay.type = core::OverlayType::Disk;
  plan.changes.setOverlay.type = core::OverlayType::RAM;
  plan.maintenance.storageVolume = QString::fromLatin1(uwf::test::application::fixtureVolumeDName);
  const auto actions = std::make_shared<FakeConfigurationActions>();
  presentation::ApplyPlanViewModel model(plan, actions);
  QVERIFY(!model.canApply());
  QVERIFY(!model.warning().isEmpty());
  QCOMPARE(model.changeCount(), std::size_t{2});
  QList<presentation::ApplyPlanViewModel::Change> types;
  for (const auto& group : model.groups())
    for (const auto& item : group.items)
      if (item.title == QStringLiteral("Overlay type")) types.append(item);
  QCOMPARE(types.size(), qsizetype{1});
  QCOMPARE(types.constFirst().before, QStringLiteral("Disk"));
  QCOMPARE(types.constFirst().after, QStringLiteral("RAM"));
  model.apply();
  QCOMPARE(actions->calls, 0);
}

void ConfigurationReviewViewModelTests::safeRestartUsesTheLatestRuntimeMode_data() {
  QTest::addColumn<int>("source");  // 0=ordinary snapshot; 1=execution result; 2=reviewed maintenance observation.
  QTest::addColumn<int>("disposition");
  QTest::addColumn<bool>("horm");
  QTest::addColumn<int>("requestedHorm");  // -1=unchanged; 0=disable; 1=enable.
  QTest::addColumn<bool>("written");
  QTest::addColumn<bool>("allowed");
  QTest::newRow("snapshot-disposition-suppresses-rom") << 0 << 1 << false << -1 << true << true;
  QTest::newRow("snapshot-loaded-rom") << 0 << 0 << false << -1 << true << true;
  QTest::newRow("result-suppression-supersedes-reviewed-rom") << 1 << 1 << false << -1 << true << true;
  QTest::newRow("result-rom-supersedes-reviewed-suppression") << 1 << 0 << false << -1 << true << true;
  QTest::newRow("result-full-commit") << 1 << 4 << false << -1 << true << true;
  QTest::newRow("result-horm-skips-automatic-commit") << 1 << 1 << true << -1 << true << true;
  QTest::newRow("horm-disabled-transition-skips-automatic-commit") << 1 << 1 << false << 0 << true << true;
  QTest::newRow("horm-enabled-skips-automatic-commit") << 1 << 1 << false << 1 << true << true;
  QTest::newRow("reviewed-maintenance-suppression-supersedes-snapshot") << 2 << 1 << false << -1 << true << true;
  QTest::newRow("reviewed-maintenance-rom-supersedes-snapshot") << 2 << 0 << false << -1 << true << true;
  QTest::newRow("no-confirmed-write") << 1 << 1 << false << -1 << false << false;
  QTest::newRow("failed-disable-HORM-does-not-publish-a-transition") << 1 << 2 << true << 0 << false << false;
}
void ConfigurationReviewViewModelTests::safeRestartUsesTheLatestRuntimeMode() {
  QFETCH(int, source);
  QFETCH(int, disposition);
  QFETCH(bool, horm);
  QFETCH(int, requestedHorm);
  QFETCH(bool, written);
  QFETCH(bool, allowed);
  app::ConfigurationPlan plan;
  plan.snapshot = snapshot();
  plan.snapshot.runtime.romDisposition = static_cast<std::uint32_t>(source == 0 ? disposition : disposition == 1 ? 0 : 1);
  plan.snapshot.runtime.fullCommit = false;
  plan.policySnapshot.current.flags = {app::PolicyObservationStatus::Available, app::OverlayFlags{4}};
  plan.changes.setFilterEnabled = false;
  if (requestedHorm >= 0) plan.maintenance.hormEnabled = requestedHorm != 0;
  app::OverlayMaintenanceSnapshot mode;
  mode.filterEnabled = true;
  mode.currentFlags = app::OverlayFlags{4};
  mode.hormEnabled = horm;
  mode.romDisposition = static_cast<std::uint32_t>(disposition);
  mode.overlayCommit = (disposition & 4) != 0;
  if (source == 2) plan.maintenanceSnapshot = mode;
  auto actions = std::make_shared<FakeConfigurationActions>();
  if (source == 1) actions->result.observedOverlayMaintenance = mode;
  actions->result.steps.push_back({"Filter", app::ConfigurationStepOutcome::Succeeded, written ? app::WriteEffect::Confirmed : app::WriteEffect::None,
                                   app::ConfigurationOperation{app::ConfigurationOperationKind::Filter, {}}});
  if (requestedHorm == 0 && !written)
    actions->result.steps.push_back({"Disable HORM", app::ConfigurationStepOutcome::Failed, app::WriteEffect::None,
                                     app::ConfigurationOperation{app::ConfigurationOperationKind::Horm, {}}});
  presentation::ApplyPlanViewModel model(std::move(plan), actions);
  QVERIFY(model.canApply());
  model.apply();
  QTRY_VERIFY(!model.applying());
  QCOMPARE(actions->calls, 1);
  QCOMPARE(model.canRestart(), written);
  QCOMPARE(model.canSafeRestart(), allowed);
  QCOMPARE(model.canDirectRestart(), written);
  // A reviewed intent is not an observed runtime transition. In particular a
  // failed DisableHORM must not claim that HORM has already been disabled.
  QVERIFY(model.safeRestartRestriction().isEmpty());
  model.setInteractionAvailability({.apply = false,
                                    .safeRestart = false,
                                    .directRestart = false,
                                    .safeRestartRestriction = QStringLiteral("Refresh failed"),
                                    .directRestartRestriction = QStringLiteral("Refresh failed")});
  QVERIFY(!model.canSafeRestart());
  QVERIFY(!model.canDirectRestart());
  QCOMPARE(model.safeRestartRestriction(), QStringLiteral("Refresh failed"));
  QCOMPARE(model.directRestartRestriction(), QStringLiteral("Refresh failed"));
  model.setInteractionAvailability({});
  QCOMPARE(model.canSafeRestart(), allowed);
  QCOMPARE(model.canDirectRestart(), written);
}

void ConfigurationReviewViewModelTests::applyRetryAndReconciliationDoNotReplayWrites() {
  const auto state = snapshot();
  auto actions = std::make_shared<FakeConfigurationActions>();
  core::PendingChanges changes;
  changes.setFilterEnabled = false;
  presentation::ApplyPlanViewModel model(changes, state, actions);
  connect(&model, &presentation::ApplyPlanViewModel::applyingChanged, &model, [&] {
    if (model.applying()) model.apply();
  });
  actions->result.steps.push_back({{}, app::ConfigurationStepOutcome::ExecutionFailed});
  model.apply();
  QTRY_VERIFY(!model.applying());
  QCOMPARE(actions->calls, 1);
  QVERIFY(model.canApply());
  QStringList notifications;
  connect(&model, &presentation::ApplyPlanViewModel::resultChanged, &model, [&] { notifications.append(QStringLiteral("result")); });
  connect(&model, &presentation::ApplyPlanViewModel::reconciliationRequired, &model, [&] { notifications.append(QStringLiteral("reconcile")); });
  actions->result.steps = {
      {{}, app::ConfigurationStepOutcome::Succeeded, app::WriteEffect::None, app::ConfigurationOperation{app::ConfigurationOperationKind::Filter, {}}, true}};
  model.apply();
  QTRY_VERIFY(!model.applying());
  // The owner must install its reconciliation gate before result observers
  // can expose actions again.
  QCOMPARE(notifications, QStringList({QStringLiteral("reconcile"), QStringLiteral("result")}));
  QVERIFY(!model.canApply());
  model.apply();
  QTRY_VERIFY(!model.applying());
  QCOMPARE(actions->calls, 2);
}

void ConfigurationReviewViewModelTests::reviewUsesApplicationValidationForResetPrerequisites_data() {
  QTest::addColumn<bool>("missingType");
  QTest::addColumn<bool>("failed");
  QTest::newRow("type-failed") << true << true;
  QTest::newRow("flags-failed") << false << true;
  QTest::newRow("type-observed") << true << false;
  QTest::newRow("flags-observed") << false << false;
}

void ConfigurationReviewViewModelTests::reviewUsesApplicationValidationForResetPrerequisites() {
  QFETCH(bool, missingType);
  QFETCH(bool, failed);
  auto state = snapshot();
  state.current.filter.enabled = true;
  state.current.overlay.type = state.next.overlay.type = core::OverlayType::Disk;
  state.current.overlay.maximumSizeMb = state.next.overlay.maximumSizeMb = 2048;
  app::OverlayPolicySnapshot baseline;
  baseline.transportAvailable = true;
  baseline.current.filterEnabled = {.status = app::PolicyObservationStatus::Available, .value = true};
  baseline.current.overlayType = baseline.next.overlayType = {.status = app::PolicyObservationStatus::Available, .value = 1};
  baseline.current.flags = baseline.next.flags = {.status = app::PolicyObservationStatus::Available, .value = app::OverlayFlags{app::OverlayFlags::Persistent}};
  baseline.resetPersistentState = {.status = app::PolicyObservationStatus::Available, .value = 1};
  if (failed) {
    if (missingType)
      baseline.current.overlayType = {.status = app::PolicyObservationStatus::Failed};
    else
      baseline.current.flags = {.status = app::PolicyObservationStatus::Failed};
  }
  app::ConfigurationChanges changes;
  changes.overlayPolicy.reset = app::PersistentReset::Preserve;
  const auto plan = app::reviewApplicationConfiguration(changes, state, baseline);
  const auto validation = app::validateOverlayPolicyPlan(plan);
  auto actions = std::make_shared<FakeConfigurationActions>();
  presentation::ApplyPlanViewModel model(plan, actions, [](const QString&, const QString&) { return app::TextFileWriteResult{}; });
  QCOMPARE(validation.valid(), !failed);
  QCOMPARE(model.canApply(), validation.valid());
  QCOMPARE(model.canExport(), validation.valid());
  if (failed) {
    QCOMPARE(validation.conflicts.size(), std::size_t{1});
    QCOMPARE(model.warning(), presentation::OverlayAdvancedViewModel::conflictText(validation.conflicts.front().issue));
    QCOMPARE(model.changeCommandsText(), model.warning());
    QCOMPARE(model.targetCommandsText(), model.warning());
    model.apply();
    QCOMPARE(actions->calls, 0);
  } else {
    QVERIFY(model.warning().isEmpty());
    QVERIFY(model.changeCommandsText().contains(QStringLiteral("reset-persistentstate off")));
  }
}

void ConfigurationReviewViewModelTests::persistentResetExportUsesTheSelectedScopeWithoutExecuting_data() {
  QTest::addColumn<bool>("clear");
  QTest::newRow("schedule-clear") << true;
  QTest::newRow("cancel-clear") << false;
}

void ConfigurationReviewViewModelTests::persistentResetExportUsesTheSelectedScopeWithoutExecuting() {
  QFETCH(bool, clear);
  using Scope = presentation::ApplyPlanViewModel::ScriptScope;
  auto state = snapshot();
  state.current.filter.enabled = state.next.filter.enabled = true;
  state.current.overlay.type = state.next.overlay.type = core::OverlayType::Disk;
  state.runtime.currentOverlayFlags = app::OverlayFlags::Persistent;
  app::OverlayPolicySnapshot policy;
  policy.transportAvailable = true;
  policy.current.filterEnabled = {app::PolicyObservationStatus::Available, true};
  policy.current.overlayType = {app::PolicyObservationStatus::Available, 1};
  policy.current.flags = {app::PolicyObservationStatus::Available, app::OverlayFlags{app::OverlayFlags::Persistent}};
  policy.next = policy.current;
  policy.resetPersistentState = {app::PolicyObservationStatus::Available, clear ? 0 : 1};
  app::ConfigurationChanges changes;
  changes.overlayPolicy.reset = clear ? app::PersistentReset::Clear : app::PersistentReset::Preserve;
  changes.settings.addRegistryExclusions = {"HKEY_LOCAL_MACHINE\\SOFTWARE\\ResetExport"};
  const auto plan = app::planApplicationConfiguration(changes, state, policy);
  const auto actions = std::make_shared<FakeConfigurationActions>();
  QString saved;
  int exports = 0;
  presentation::ApplyPlanViewModel model(plan, actions, [&](const QString&, const QString& contents) {
    ++exports;
    saved = contents;
    return app::TextFileWriteResult{};
  });
  QVERIFY2(model.canApply(), qUtf8Printable(model.warning()));
  QVERIFY(model.canExport(Scope::PendingChanges));
  QVERIFY(model.canExport(Scope::NextConfiguration));
  QVERIFY(model.exportRestrictionText(Scope::PendingChanges).isEmpty());
  QVERIFY(model.exportRestrictionText(Scope::NextConfiguration).isEmpty());
  const auto command = QStringLiteral("uwfmgr.exe overlay reset-persistentstate ") + (clear ? QStringLiteral("on") : QStringLiteral("off"));
  QVERIFY(model.changeCommandsText().contains(command));
  QVERIFY(!model.targetCommandsText().contains(QStringLiteral("uwfmgr.exe overlay reset-persistentstate")));
  QVERIFY(model.targetCommandsText().contains(QStringLiteral("Pending changes")));
  for (const auto scope : {Scope::PendingChanges, Scope::NextConfiguration}) {
    QVERIFY(model.exportFile(QStringLiteral("reset-scope.cmd"), scope).isEmpty());
    auto parsed = app::parseUwfmgrText(saved.toStdString());
    std::erase_if(parsed, [](const auto& item) { return item.parseError == app::ParseError::Comment; });
    QCOMPARE(parsed.size(), model.exportCommands(scope).size());
    QVERIFY(!parsed.empty());
    QCOMPARE(std::ranges::count(parsed, app::UwfmgrKind::OverlayResetPersistentState, &app::UwfmgrCommand::kind),
             scope == Scope::PendingChanges ? std::ptrdiff_t{1} : std::ptrdiff_t{0});
    QVERIFY(std::ranges::any_of(parsed, [](const auto& item) { return item.kind == app::UwfmgrKind::RegistryAddExclusion; }));
    if (scope == Scope::PendingChanges) {
      const auto reset = std::ranges::find(parsed, app::UwfmgrKind::OverlayResetPersistentState, &app::UwfmgrCommand::kind);
      QCOMPARE(reset->args, std::vector<std::string>{clear ? "on" : "off"});
    }
    for (std::size_t index = 0; index < parsed.size(); ++index) {
      QCOMPARE(parsed[index].parseError, app::ParseError::None);
      QCOMPARE(parsed[index].requiresPrevious, index != 0);
    }
  }
  QCOMPARE(exports, 2);
  QCOMPARE(actions->calls, 0);
  QVERIFY(!model.applying());
}

void ConfigurationReviewViewModelTests::stagingCleanupRemainsApplicableWhenOverlayFlagsCannotBeRead() {
  app::OverlayPolicySnapshot policy;
  policy.transportAvailable = true;
  policy.next.overlayType = {.status = app::PolicyObservationStatus::Available, .value = static_cast<std::int64_t>(core::OverlayType::RAM)};
  policy.next.flags = {.status = app::PolicyObservationStatus::Failed};
  const QList<app::FileStagingEntry> entries{{app::FileStagingKind::File, QStringLiteral("C:\\Data\\saved.bin")}};
  for (const bool clear : {true, false}) {
    const QList<app::FileStagingEntry> after =
        clear ? QList<app::FileStagingEntry>{} : QList<app::FileStagingEntry>{{app::FileStagingKind::File, QStringLiteral("C:\\Data\\replacement.bin")}};
    const auto plan = app::reviewApplicationConfiguration({}, snapshot(), policy, app::FileStagingChanges{entries, after, false});
    auto actions = std::make_shared<FakeConfigurationActions>();
    presentation::ApplyPlanViewModel model(plan, actions);
    QVERIFY(model.canApply());
    model.apply();
    QTRY_VERIFY(!model.applying());
    QCOMPARE(actions->calls, 1);
  }
}

void ConfigurationReviewViewModelTests::romReviewPreservesStaging_data() {
  QTest::addColumn<bool>("saved");
  QTest::newRow("retained-list") << true;
  QTest::newRow("new-list") << false;
}

void ConfigurationReviewViewModelTests::romReviewPreservesStaging() {
  QFETCH(bool, saved);
  auto state = snapshot();
  state.current.filter.enabled = false;
  state.current.overlay.type = state.next.overlay.type = core::OverlayType::RAM;
  state.next.registryExclusions = {std::string(app::kFileStagingRegistryRoot)};
  app::OverlayPolicySnapshot policy;
  policy.transportAvailable = true;
  policy.current.filterEnabled = {app::PolicyObservationStatus::Available, false};
  policy.current.overlayType = policy.next.overlayType = {app::PolicyObservationStatus::Available, 0};
  policy.current.flags = policy.next.flags = {app::PolicyObservationStatus::Available, app::OverlayFlags{0}};
  const QList<app::FileStagingEntry> entries{{app::FileStagingKind::File, QStringLiteral("C:\\Data\\saved.bin")}};
  app::ConfigurationChanges changes;
  changes.overlayPolicy.readOnlyMedia = true;
  const auto plan =
      app::reviewApplicationConfiguration(changes, state, policy, app::FileStagingChanges{saved ? entries : QList<app::FileStagingEntry>{}, entries, false});
  auto actions = std::make_shared<FakeConfigurationActions>();
  presentation::ApplyPlanViewModel model(plan, actions, [](const QString&, const QString&) { return app::TextFileWriteResult{}; });
  QVERIFY(model.canApply());
  QCOMPARE(model.canExport(), saved);
  QVERIFY(model.warning().isEmpty());
  QVERIFY(model.changeCommandsText().contains(QStringLiteral("set-rom-mode on")));
  QCOMPARE(plan.staging->after, entries);
  QCOMPARE(plan.hasStagingChanges(), !saved);
  model.apply();
  QTRY_VERIFY(!model.applying());
  QCOMPARE(actions->calls, 1);
}

void ConfigurationReviewViewModelTests::configurationReviewUsesTheNextSessionBaselineAndRetainsNonCliChanges() {
  auto state = snapshot();
  state.current.filter.enabled = false;
  state.next.filter.enabled = true;
  state.current.overlay.maximumSizeMb = 1024;
  state.next.overlay.maximumSizeMb = 2048;
  state.next.volumes = {{uwf::test::application::fixtureVolumeCName, "C:", true, uwf::core::VolumeBinding::DriveLetter, std::string("C:")}};
  core::PendingChanges changes;
  changes.setFilterEnabled = false;
  changes.setOverlay.maximumSizeMb = 4096;
  changes.volumeProtect["C:"] = false;
  changes.volumeBindings["C:"] = uwf::core::VolumeBinding::VolumeId;
  changes.addFileExclusions["C:"] = {"C:\\<literal>&data"};
  changes.removeFileExclusions["C:"] = {"C:\\remove"};
  changes.addRegistryExclusions = {"HKLM\\SOFTWARE\\Vendor"};
  changes.setPersistDomainSecretKey = true;
  changes.setPersistTSCAL = true;
  auto actions = std::make_shared<FakeConfigurationActions>();
  presentation::ApplyPlanViewModel model(changes, state, actions);
  QCOMPARE(model.groups().size(), 3);
  const auto& protection = model.groups()[0].items;
  QCOMPARE(protection[0].before, QStringLiteral("On"));
  QCOMPARE(protection[0].after, QStringLiteral("Off"));
  QCOMPARE(protection[1].before, QStringLiteral("2,048 MB"));
  QCOMPARE(protection[1].after, QStringLiteral("4,096 MB"));
  QCOMPARE(protection[3].before, QStringLiteral("By drive letter"));
  QCOMPARE(protection[3].after, QStringLiteral("By volume ID"));
  QCOMPARE(model.groups()[1].items[0].title, QStringLiteral("C:\\<literal>&data"));
  qsizetype reviewed = 0;
  for (const auto& group : model.groups()) reviewed += group.items.size();
  QCOMPARE(static_cast<std::size_t>(reviewed), changes.count());
  const auto target = model.targetCommandsText();
  state.next.overlay.maximumSizeMb = 512;
  state.next.filter.enabled = false;
  QCOMPARE(model.targetCommandsText(), target);
  QCOMPARE(protection[0].before, QStringLiteral("On"));
  QCOMPARE(actions->calls, 0);
  QVERIFY(model.changeCommandsText().contains(QStringLiteral("filter disable")));
  QVERIFY(model.changeCommandsText().startsWith(QStringLiteral("uwfmgr.exe")));
  QVERIFY(!model.changeCommandsText().contains(QStringLiteral("setlocal"), Qt::CaseInsensitive));
  QVERIFY(!model.changeCommandsText().contains(QStringLiteral("chcp"), Qt::CaseInsensitive));
  model.apply();
  QTRY_VERIFY(!model.applying());
  QCOMPARE(actions->calls, 1);
  QVERIFY(actions->observedSnapshot.has_value());
  QCOMPARE(actions->observedSnapshot->next.overlay.maximumSizeMb, uint32_t{2048});
  QVERIFY(actions->observedSnapshot->next.filter.enabled);
  // Bypassing editor normalization cannot make a reviewed zero-capacity
  // target executable or export an invalid set-size command.
  state.next.overlay.type = core::OverlayType::RAM;
  changes.setOverlay.maximumSizeMb = 0;
  presentation::ApplyPlanViewModel invalid(changes, state, actions);
  QVERIFY(!invalid.canApply());
  QVERIFY(!invalid.warning().isEmpty());
  QVERIFY(!invalid.changeCommandsText().contains(QStringLiteral("set-size 0")));
  invalid.apply();
  QCOMPARE(actions->calls, 1);
}

void ConfigurationReviewViewModelTests::maintenancePreviewUsesNativeCommandsAndSeparatesNextScope_data() {
  QTest::addColumn<int>("operation");
  QTest::newRow("disable-horm") << 0;
  QTest::newRow("enable-horm") << 1;
  QTest::newRow("storage") << 2;
  QTest::newRow("full-commit") << 3;
  QTest::newRow("unprotect-unknown-export-scope") << 4;
}

void ConfigurationReviewViewModelTests::maintenancePreviewUsesNativeCommandsAndSeparatesNextScope() {
  QFETCH(int, operation);
  app::ConfigurationPlan plan;
  plan.snapshot = snapshot();
  if (operation <= 1)
    plan.maintenance.hormEnabled = operation == 1;
  else if (operation == 2) {
    plan.snapshot.current.filter.enabled = plan.snapshot.next.filter.enabled = false;
    plan.maintenance.storageVolume = QString::fromLatin1(test::application::fixtureVolumeDName);
    auto& observed = plan.maintenanceSnapshot.emplace();
    observed.filterEnabled = observed.hormEnabled = false;
    observed.currentOverlayType = observed.nextOverlayType = static_cast<int>(core::OverlayType::RAM);
    observed.currentFlags = observed.nextFlags = app::OverlayFlags{0};
    observed.maximumOverlaySize = plan.snapshot.next.overlay.maximumSizeMb;
    observed.systemVolumeProtected = plan.snapshot.runtime.systemVolumeProtected;
    observed.swapCapability.support = app::OverlayMaintenanceSupport::Supported;
    observed.volumes.push_back({*plan.maintenance.storageVolume, QStringLiteral("D:"), {}, 0, true, {}, {}});
  } else if (operation == 3)
    plan.maintenance.fullCommit = true;
  else
    plan.maintenance.allVolumesProtected = false;
  auto actions = std::make_shared<FakeConfigurationActions>();
  int exports = 0;
  QString saved;
  presentation::ApplyPlanViewModel model(plan, actions, [&exports, &saved](const QString&, const QString& text) {
    ++exports;
    saved = text;
    return app::TextFileWriteResult{};
  });
  QVERIFY2(model.canApply(), qUtf8Printable(model.warning()));
  QCOMPARE(model.changeCount(), std::size_t{1});
  using Scope = presentation::ApplyPlanViewModel::ScriptScope;
  if (operation < 4) {
    const auto kind = operation == 0   ? app::UwfmgrKind::FilterDisableHorm
                      : operation == 1 ? app::UwfmgrKind::FilterEnableHorm
                      : operation == 2 ? app::UwfmgrKind::VolumeCreateSwapfile
                                       : app::UwfmgrKind::OverlayCommit;
    QCOMPARE(model.changeCommandCount(), std::size_t{1});
    QCOMPARE(model.exportCommands().size(), std::size_t{1});
    QCOMPARE(model.exportCommands().front().kind, kind);
    QVERIFY(!model.changeCommandsText().contains(QStringLiteral("not included in command export")));
    QVERIFY(model.canExport());
    QVERIFY(model.exportRestrictionText().isEmpty());
    QVERIFY(model.exportFile(QStringLiteral("pending.cmd")).isEmpty());
    QCOMPARE(exports, 1);
    auto parsed = app::parseUwfmgrText(saved.toStdString());
    std::erase_if(parsed, [](const auto& row) { return row.parseError == app::ParseError::Comment; });
    QCOMPARE(parsed.size(), std::size_t{1});
    QCOMPARE(parsed.front().kind, kind);
    QCOMPARE(parsed.front().parseError, app::ParseError::None);
    QVERIFY(model.canExport(Scope::NextConfiguration));
    QCOMPARE(std::ranges::count(model.exportCommands(Scope::NextConfiguration), kind, &app::UwfmgrCommand::kind),
             operation == 2 ? std::ptrdiff_t{1} : std::ptrdiff_t{0});
    QVERIFY(model.exportFile(QStringLiteral("next.cmd"), Scope::NextConfiguration).isEmpty());
    QCOMPARE(exports, 2);
  } else {
    QCOMPARE(model.changeCommandCount(), std::size_t{0});
    const auto restriction = QStringLiteral("Bulk unprotection requires a confirmed complete next-session volume scope.");
    QVERIFY(model.changeCommandsText().contains(restriction));
    QCOMPARE(model.exportRestrictionText(), restriction);
    QCOMPARE(model.exportRestrictionText(Scope::NextConfiguration), restriction);
    QVERIFY(!model.canExport());
    QVERIFY(!model.canExport(Scope::NextConfiguration));
    QVERIFY(!model.exportFile(QStringLiteral("unused.cmd")).isEmpty());
    QCOMPARE(exports, 0);
  }
  QCOMPARE(actions->calls, 0);
  model.apply();
  QTRY_VERIFY(!model.applying());
  QCOMPARE(actions->calls, 1);
  QVERIFY(actions->observedPlan.has_value());
  QCOMPARE(actions->observedPlan->maintenance.hormEnabled, plan.maintenance.hormEnabled);
  QCOMPARE(actions->observedPlan->maintenance.storageVolume, plan.maintenance.storageVolume);
  QCOMPARE(actions->observedPlan->maintenance.fullCommit, plan.maintenance.fullCommit);
  QCOMPARE(actions->observedPlan->maintenance.allVolumesProtected, plan.maintenance.allVolumesProtected);
}

void ConfigurationReviewViewModelTests::exportScopesRequireActualCommandsWithoutLosingNextSnapshotCoverage() {
  using Scope = presentation::ApplyPlanViewModel::ScriptScope;
  auto state = snapshot();
  core::PendingChanges changes;
  changes.volumeBindings["C:"] = core::VolumeBinding::VolumeId;
  QString saved;
  int writes = 0;
  auto actions = std::make_shared<FakeConfigurationActions>();
  presentation::ApplyPlanViewModel model(changes, state, actions, {}, [&](const QString&, const QString& text) {
    ++writes;
    saved = text;
    return app::TextFileWriteResult{};
  });
  QCOMPARE(model.changeCommandCount(), std::size_t{0});
  QVERIFY(model.exportCommands().empty());
  QVERIFY(!model.canExport());
  QVERIFY(!model.exportFile(QStringLiteral("empty-pending.cmd")).isEmpty());
  QCOMPARE(writes, 0);
  QVERIFY(model.canExport(Scope::NextConfiguration));
  QVERIFY(!model.exportCommands(Scope::NextConfiguration).empty());
  QVERIFY(model.exportFile(QStringLiteral("next.cmd"), Scope::NextConfiguration).isEmpty());
  QCOMPARE(writes, 1);
  QVERIFY(saved.contains(QStringLiteral("set-size")));
  QCOMPARE(actions->calls, 0);
}

void ConfigurationReviewViewModelTests::commandPreviewPreservesOmissionScopeWithoutExecuting() {
  auto state = snapshot();
  core::PendingChanges changes;
  changes.setFilterEnabled = false;
  changes.volumeBindings["C:"] = core::VolumeBinding::VolumeId;
  changes.setPersistDomainSecretKey = true;
  changes.setPersistTSCAL = true;
  const app::FileStagingChanges staging{{}, {{app::FileStagingKind::File, QStringLiteral("C:\\State\\saved.bin")}}};
  auto actions = std::make_shared<FakeConfigurationActions>();
  presentation::ApplyPlanViewModel model(changes, state, actions, staging);
  const auto preview = model.changeCommandsText();
  QVERIFY(preview.startsWith(QStringLiteral("uwfmgr.exe")));
  for (const auto* description : {"Volume binding", "Domain Secret Key persistence", "Terminal Services license persistence", "File staging"})
    QVERIFY2(preview.contains(QString::fromLatin1(description)), qUtf8Printable(preview));
  QCOMPARE(actions->calls, 0);
}

void ConfigurationReviewViewModelTests::conflictingMaintenanceDraftRemainsReviewableAndCannotApply() {
  auto actions = std::make_shared<FakeConfigurationActions>();
  app::ConfigurationChanges changes;
  changes.maintenance.fullCommit = true;
  changes.maintenance.hormEnabled = true;
  const auto plan = app::reviewApplicationConfiguration(changes, snapshot(), {});
  std::unique_ptr<presentation::ApplyPlanViewModel> model;
  try {
    model = std::make_unique<presentation::ApplyPlanViewModel>(plan, actions);
  } catch (const std::exception& error) {
    QFAIL(error.what());
  }
  QVERIFY(model);
  QCOMPARE(model->changeCount(), std::size_t{2});
  QVERIFY(!model->warning().isEmpty());
  QVERIFY(!model->canApply());
  QVERIFY(!model->canExport());
  QVERIFY(model->hasMaintenanceChanges());
  qsizetype entries = 0;
  for (const auto& group : model->groups()) entries += group.items.size();
  QCOMPARE(entries, qsizetype{2});
  model->apply();
  QCOMPARE(actions->calls, 0);
}

void ConfigurationReviewViewModelTests::batchProtectionReviewExportsCommandsAndKeepsPreviewClean() {
  using Scope = presentation::ApplyPlanViewModel::ScriptScope;
  for (const bool enabled : {true, false}) {
    auto actions = std::make_shared<FakeConfigurationActions>();
    app::ConfigurationChanges changes;
    changes.maintenance.allVolumesProtected = enabled;
    auto state = snapshot();
    state.next.volumes.push_back({"Volume{aaaaaaaa-bbbb-cccc-dddd-111111111111}", {}, true, core::VolumeBinding::VolumeId});
    auto plan = app::reviewApplicationConfiguration(changes, state, {});
    plan.maintenanceSnapshot.emplace();
    plan.maintenanceSnapshot->unprotectScopeConfirmed = true;
    for (const auto& volume : state.next.volumes) plan.maintenanceSnapshot->unprotectTargetVolumes.push_back(QString::fromStdString(volume.volumeName));
    QString saved;
    presentation::ApplyPlanViewModel model(plan, actions, [&saved](const QString&, const QString& text) {
      saved = text;
      return app::TextFileWriteResult{};
    });
    QCOMPARE(model.changeCount(), std::size_t{1});
    QVERIFY(model.canApply());
    QVERIFY(model.canExport());
    QVERIFY(model.exportRestrictionText().isEmpty());
    QVERIFY(!model.onlyDisablesHorm());
    QCOMPARE(model.groups().size(), 1);
    QCOMPARE(model.groups().front().items.size(), 1);
    QCOMPARE(model.groups().front().items.front().title, enabled ? QStringLiteral("Protect all") : QStringLiteral("Unprotect all"));
    QVERIFY(model.groups().front().items.front().description.contains(QStringLiteral("next startup")));
    const auto preview = model.targetCommandsText();
    QVERIFY(preview.contains(enabled ? QStringLiteral("volume protect all") : QStringLiteral("volume unprotect")));
    for (const auto& text : {model.changeCommandsText(), preview}) {
      QVERIFY(!text.contains(QStringLiteral("setlocal"), Qt::CaseInsensitive));
      QVERIFY(!text.contains(QStringLiteral("chcp"), Qt::CaseInsensitive));
      QVERIFY(!text.contains(QStringLiteral(" && ")));
    }
    QVERIFY(preview.contains(QStringLiteral("Not included"), Qt::CaseInsensitive));
    QVERIFY(preview.contains(QStringLiteral("Volume binding")));
    const auto lines = preview.split(u'\n', Qt::SkipEmptyParts);
    QCOMPARE(std::ranges::count_if(lines, [](const auto& line) { return line.startsWith(QStringLiteral("uwfmgr.exe")); }),
             static_cast<qsizetype>(model.exportCommands(Scope::NextConfiguration).size()));
    if (!enabled) {
      QVERIFY(preview.contains(QStringLiteral("aaaaaaaa-bbbb-cccc-dddd-111111111111"), Qt::CaseInsensitive));
      QVERIFY(!preview.contains(QStringLiteral("volume unprotect all")));
    }
    QVERIFY(model.exportFile(QStringLiteral("commands.cmd"), Scope::NextConfiguration).isEmpty());
    QVERIFY(saved.startsWith(QString::fromUtf8(app::kUwfmgrScriptMarker.data(), static_cast<qsizetype>(app::kUwfmgrScriptMarker.size()))));
    QVERIFY(saved.contains(QStringLiteral("setlocal"), Qt::CaseInsensitive));
    QCOMPARE(actions->calls, 0);
    if (!enabled) {
      auto unknownScope = plan;
      unknownScope.maintenanceSnapshot->unprotectScopeConfirmed = false;
      int writes = 0;
      presentation::ApplyPlanViewModel unavailable(unknownScope, actions, [&writes](const QString&, const QString&) {
        ++writes;
        return app::TextFileWriteResult{};
      });
      QVERIFY(unavailable.canApply());
      QVERIFY(!unavailable.canExport());
      QVERIFY(!unavailable.exportRestrictionText().isEmpty());
      QVERIFY(!unavailable.exportFile(QStringLiteral("unknown-scope.cmd")).isEmpty());
      QCOMPARE(writes, 0);
      QCOMPARE(actions->calls, 0);
    }
  }
}

void ConfigurationReviewViewModelTests::batchProtectionReviewShowsInheritedTargetsBeforeExplicitExceptions() {
  for (const bool enabled : {false, true}) {
    auto state = snapshot();
    state.next.volumes.front().isProtected = !enabled;
    app::ConfigurationEditSession session;
    const auto volume = session.editVolume(disk(), false);
    session.resetSnapshot(state, {}, false);
    const auto maintenance = session.maintenanceEditor();
    app::OverlayMaintenanceSnapshot observed;
    observed.filterEnabled = observed.hormEnabled = observed.overlayCommit = false;
    observed.protectScopeConfirmed = observed.unprotectScopeConfirmed = true;
    observed.protectTargetVolumes = observed.unprotectTargetVolumes = {QString::fromLatin1(test::application::fixtureVolumeCName)};
    const auto ticket = maintenance->beginObservation();
    QVERIFY(ticket);
    maintenance->finishObservation(*ticket, observed);
    maintenance->setInteractionContext(true, false);
    QVERIFY(maintenance->stageAllVolumesProtection(enabled));
    QVERIFY(volume->importProtect(!enabled));
    const auto plan = app::reviewApplicationConfiguration(session.pendingConfiguration(), state, {}, {}, observed);
    presentation::ApplyPlanViewModel model(plan, std::make_shared<FakeConfigurationActions>());
    QVERIFY(model.canApply());
    QCOMPARE(model.groups().size(), 1);
    const auto& rows = model.groups().front().items;
    QCOMPARE(rows.size(), 2);
    QCOMPARE(rows.front().title, enabled ? QStringLiteral("Protect all") : QStringLiteral("Unprotect all"));
    QCOMPARE(rows.back().title, QStringLiteral("Volume C: protection"));
    QCOMPARE(rows.back().before, enabled ? QStringLiteral("On") : QStringLiteral("Off"));
    QCOMPARE(rows.back().after, enabled ? QStringLiteral("Off") : QStringLiteral("On"));
    const auto commands = model.changeCommandsText().split(u'\n', Qt::SkipEmptyParts);
    QCOMPARE(commands.size(), 2);
    QVERIFY(commands.back().contains(enabled ? QStringLiteral("volume unprotect") : QStringLiteral("volume protect")));
    QVERIFY(commands.back().contains(QString::fromLatin1(test::application::fixtureVolumeCName)));
  }
  auto state = snapshot();
  state.next.volumes.clear();
  app::ConfigurationChanges changes;
  changes.maintenance.allVolumesProtected = true;
  changes.settings.volumeBindings["C:"] = core::VolumeBinding::DriveLetter;
  presentation::ApplyPlanViewModel binding(app::reviewApplicationConfiguration(changes, state, {}), std::make_shared<FakeConfigurationActions>());
  const auto& row = binding.groups().front().items.back();
  QCOMPARE(row.before, QStringLiteral("By volume ID"));
  QCOMPARE(row.after, QStringLiteral("By drive letter"));
  changes.settings.volumeBindings.clear();
  changes.settings.volumeProtect["D:"] = true;
  presentation::ApplyPlanViewModel ordinary(app::reviewApplicationConfiguration(changes, snapshot(), {}), std::make_shared<FakeConfigurationActions>());
  const auto& outside = ordinary.groups().front().items.back();
  QCOMPARE(outside.title, QStringLiteral("Volume D: protection"));
  QCOMPARE(outside.before, QStringLiteral("Off"));
  QCOMPARE(outside.after, QStringLiteral("On"));
}

void ConfigurationReviewViewModelTests::hiddenProtectionReviewUsesTheMatchingUuidBaseline() {
  auto state = snapshot();
  const std::string identity = "aaaaaaaa-1111-2222-3333-444444444444";
  state.next.volumes.push_back({"Volume{BBBBBBBB-1111-2222-3333-444444444444}", {}, false, core::VolumeBinding::VolumeId});
  state.next.volumes.push_back({"Volume{AAAAAAAA-1111-2222-3333-444444444444}", {}, true, core::VolumeBinding::VolumeId});
  core::PendingChanges changes;
  changes.volumeProtect[identity] = false;
  presentation::ApplyPlanViewModel model(changes, state, std::make_shared<FakeConfigurationActions>());
  QCOMPARE(model.groups().size(), 1);
  const auto& row = model.groups().front().items.front();
  QCOMPARE(row.title, QStringLiteral("Volume protection"));
  QCOMPARE(row.description, QString::fromStdString(identity));
  QCOMPARE(row.before, QStringLiteral("On"));
  QCOMPARE(row.after, QStringLiteral("Off"));
}

void ConfigurationReviewViewModelTests::batchProtectionResultListsIndividualReceiptsWithoutHidingPartialFailure() {
  auto actions = std::make_shared<FakeConfigurationActions>();
  app::ConfigurationChanges changes;
  changes.maintenance.allVolumesProtected = false;
  presentation::ApplyPlanViewModel model(app::reviewApplicationConfiguration(changes, snapshot(), {}), actions);
  app::OverlayMaintenanceResult batch;
  const QString known = QStringLiteral("11111111-1111-1111-1111-111111111111");
  const QString uncertain = QStringLiteral("22222222-2222-2222-2222-222222222222");
  const QString notApplied = QStringLiteral("33333333-3333-3333-3333-333333333333");
  batch.snapshot.volumes.push_back({known, QStringLiteral("C:"), QStringLiteral("System"), 0, true, {}, {}});
  batch.allVolumeItems = {{known, true, false, {}, {}, app::WriteEffect::Confirmed, true},
                          {uncertain, true, false, QStringLiteral("Provider reported a partial failure"),
                           app::OperationFailure{app::OperationFailureKind::Failed, app::WriteEffect::Uncertain, {}}, app::WriteEffect::Uncertain, true},
                          {notApplied, true, true, QStringLiteral("Access denied"),
                           app::OperationFailure{app::OperationFailureKind::AccessDenied, app::WriteEffect::None, {}}, app::WriteEffect::None, true}};
  app::ConfigurationStepResult receipt{"Unprotect all: partial failure", app::ConfigurationStepOutcome::Failed, app::WriteEffect::Uncertain,
                                       app::ConfigurationOperation{app::ConfigurationOperationKind::UnprotectAllVolumes, {}}};
  receipt.maintenance = app::ConfigurationMaintenanceDetail{batch.snapshot, batch.allVolumeItems};
  actions->result.steps.push_back(std::move(receipt));
  model.apply();
  QTRY_VERIFY(!model.applying());
  const auto text = model.resultText();
  QVERIFY(model.resultFailed());
  QVERIFY(text.contains(QStringLiteral("System (C:) — Applied\n  ") + known));
  QVERIFY(text.contains(QStringLiteral("Partition — Could not confirm\n  ") + uncertain));
  QVERIFY(text.contains(QStringLiteral("\n  Provider reported a partial failure")));
  QVERIFY(text.contains(QStringLiteral("Partition — Not applied\n  ") + notApplied));
  QVERIFY(!text.contains(uncertain + QStringLiteral(" — Applied")));
}

void ConfigurationReviewViewModelTests::configurationReviewIdentifiesStagingDependencies_data() {
  QTest::addColumn<QString>("key");
  QTest::addColumn<bool>("activeStaging");
  QTest::addColumn<bool>("added");
  QTest::addColumn<bool>("explicitChange");
  QTest::addColumn<bool>("builtIn");
  QTest::addColumn<int>("registryRows");
  const auto root = QString::fromLatin1(app::kFileStagingRegistryRoot.data(), app::kFileStagingRegistryRoot.size());
  QTest::newRow("automatic-root") << root << true << true << false << true << 1;
  QTest::newRow("explicit-root-with-staging") << root << true << true << true << true << 1;
  QTest::newRow("mixed-case-hive-alias") << QStringLiteral("hKlM\\software\\hsingyun\\uwf manager") << true << true << true << true << 1;
  QTest::newRow("manual-root-without-staging") << root << false << true << true << false << 1;
  QTest::newRow("sibling") << root + QStringLiteral(" Backup") << true << true << true << false << 2;
  QTest::newRow("descendant") << root + QStringLiteral("\\Other") << true << true << true << false << 2;
  QTest::newRow("covering-parent") << QStringLiteral("HKLM\\SOFTWARE\\HsingYun") << true << true << true << false << 1;
  QTest::newRow("clear-staging-and-remove-root") << root << false << false << true << false << 1;
}

void ConfigurationReviewViewModelTests::configurationReviewIdentifiesStagingDependencies() {
  QFETCH(QString, key);
  QFETCH(bool, activeStaging);
  QFETCH(bool, added);
  QFETCH(bool, explicitChange);
  QFETCH(bool, builtIn);
  QFETCH(int, registryRows);
  const QString dependency = QStringLiteral("File staging relies on this registry exclusion to preserve its list across restarts.");
  auto state = snapshot();
  core::PendingChanges changes;
  if (explicitChange) {
    if (added) {
      app::ExclusionEditor editor(app::ExclusionEditor::Kind::Registry);
      const auto addition = editor.addSelectedPath(key);
      QCOMPARE(addition.outcome, app::ExclusionImportOutcome::Applied);
      key = addition.path;
    }
    (added ? changes.addRegistryExclusions : changes.removeRegistryExclusions).push_back(key.toStdString());
  }
  std::optional<app::FileStagingChanges> staging;
  if (activeStaging || !added) {
    staging.emplace();
    auto& entries = activeStaging ? staging->after : staging->before;
    entries.append({app::FileStagingKind::File, QStringLiteral("C:\\Data\\settings.json")});
  }
  if (!added) state.current.registryExclusions = state.next.registryExclusions = {key.toStdString()};
  auto actions = std::make_shared<FakeConfigurationActions>();
  presentation::ApplyPlanViewModel model(changes, state, actions, staging);
  const auto group = std::ranges::find_if(model.groups(), [](const auto& value) { return value.title == QStringLiteral("Registry exclusions"); });
  QVERIFY(group != model.groups().end());
  QCOMPARE(group->items.size(), registryRows);
  QModelIndex entry;
  for (int row = 0; row < model.rowCount(); ++row) {
    const auto index = model.index(row, 0);
    if (index.data(presentation::ApplyPlanViewModel::KindRole).toInt() == presentation::ApplyPlanViewModel::Entry && index.data().toString() == key)
      entry = index;
  }
  QVERIFY(entry.isValid());
  const QString badge = builtIn ? QStringLiteral("Built-in") : added ? QStringLiteral("Added") : QStringLiteral("Removed");
  const QString description = builtIn ? dependency
                              : added ? QStringLiteral("Add this key to the next-session exclusions.")
                                      : QStringLiteral("Remove this key from the next-session exclusions.");
  QCOMPARE(entry.data(presentation::ApplyPlanViewModel::AfterRole).toString(), badge);
  QCOMPARE(entry.data(presentation::ApplyPlanViewModel::DescriptionRole).toString(), description);
  QVERIFY(entry.data(Qt::AccessibleTextRole).toString().contains(badge));
  QVERIFY(entry.data(Qt::AccessibleTextRole).toString().contains(description));
  QVERIFY(entry.data(presentation::ApplyPlanViewModel::PathRole).toBool());
  QCOMPARE(actions->calls, 0);
}

void ConfigurationReviewViewModelTests::stagingPersistenceReviewDistinguishesCleanupFromRecovery_data() {
  QTest::addColumn<bool>("cleanup");
  QTest::newRow("confirm-empty-before-removing-exclusion") << true;
  QTest::newRow("restore-durability-of-an-existing-list") << false;
}

void ConfigurationReviewViewModelTests::stagingPersistenceReviewDistinguishesCleanupFromRecovery() {
  QFETCH(bool, cleanup);
  auto state = snapshot();
  const auto root = std::string(app::kFileStagingRegistryRoot);
  state.current.registryExclusions = state.next.registryExclusions = {root};
  core::PendingChanges changes;
  app::FileStagingChanges staging;
  staging.persistencePending = true;
  if (cleanup)
    changes.removeRegistryExclusions = {root};
  else
    staging.before = staging.after = {{app::FileStagingKind::File, QStringLiteral("C:\\Data\\settings.json")}};
  const auto actions = std::make_shared<FakeConfigurationActions>();
  presentation::ApplyPlanViewModel model(changes, state, actions, staging);
  QVERIFY(model.hasStagingChanges());
  QVERIFY(!model.canExport());
  QCOMPARE(model.changeCount(), cleanup ? std::size_t{2} : std::size_t{1});
  const auto group = std::ranges::find_if(model.groups(), [](const auto& value) { return value.title == QStringLiteral("File staging"); });
  QVERIFY(group != model.groups().end());
  QCOMPARE(group->items.size(), 1);
  const auto& item = group->items.front();
  QCOMPARE(item.title, QStringLiteral("File staging"));
  QCOMPARE(item.description, cleanup ? QStringLiteral("Confirm that the saved staging list is empty before removing its registry exclusion.")
                                     : QStringLiteral("File staging is saved after Apply"));
  QCOMPARE(item.before, cleanup ? QStringLiteral("Pending confirmation") : QStringLiteral("Unavailable"));
  QCOMPARE(item.after, cleanup ? QStringLiteral("Empty") : QStringLiteral("Ready"));
  QCOMPARE(actions->calls, 0);
}

void ConfigurationReviewViewModelTests::failedProtectionCreationSkipsDependentConfiguration_data() {
  QTest::addColumn<bool>("registered");
  QTest::addColumn<test::FakeUwfBackend::FaultTiming>("timing");
  for (const bool registered : {false, true})
    for (const auto [timing, stage] : {std::pair{test::FakeUwfBackend::FaultTiming::Before, "before-protection-write"},
                                       std::pair{test::FakeUwfBackend::FaultTiming::AfterEffect, "after-protection-write"}})
      QTest::newRow(qUtf8Printable(QStringLiteral("%1/%2").arg(registered ? "existing-next" : "requires-creation", stage))) << registered << timing;
}

void ConfigurationReviewViewModelTests::failedProtectionCreationSkipsDependentConfiguration() {
  QFETCH(bool, registered);
  QFETCH(test::FakeUwfBackend::FaultTiming, timing);
  constexpr auto volumeId = "11111111-2222-3333-4444-555555555555";
  const auto system = std::make_shared<test::FakeUwfBackend>();
  auto physicalState = system->snapshot();
  physicalState.runtime.fileMaintenancePrefixes[volumeId] = "C:";
  system->seed(std::move(physicalState));
  (void)compat::call(*system, compat::AddVolume{volumeId, "C:"});
  system->simulateRestart();
  if (!registered) (void)compat::call(*system, compat::RemoveVolume{volumeId});
  const auto before = system->snapshot();
  const auto baseline = app::publishedUwfSnapshot(system->state(), app::UwfCapability::Available, true, {});
  core::PendingChanges changes;
  changes.volumeProtect["C:"] = true;
  changes.volumeBindings["C:"] = uwf::core::VolumeBinding::VolumeId;
  changes.addFileExclusions["C:"] = {"C:\\Data\\dependent.bin"};
  const auto plan = app::planConfiguration(changes, baseline);
  const compat::SetVolume faultTarget{volumeId, compat::VolumeProperty::Protected, true, registered ? "C:" : "", registered};
  system->inject({.request = faultTarget, .operation = {}, .timing = timing, .kind = compat::ErrorKind::Failed, .message = "Injected protection failure"});
  system->clearTrace();
  const auto result = app::ConfigurationExecutor(app::uwfConfigurationRepository(system)).apply(plan);
  QCOMPARE(result.failedOperations(), 1);
  QCOMPARE(result.dependencySkippedOperations(), registered ? 0 : 2);
  QVERIFY(result.reconciliationRequired());
  const auto step = [&](const app::ConfigurationOperationKind kind) {
    const auto found = std::ranges::find_if(result.steps, [kind](const auto& entry) { return entry.operation && entry.operation->kind == kind; });
    return found == result.steps.end() ? app::ConfigurationStepOutcome::Failed : found->outcome;
  };
  QCOMPARE(step(app::ConfigurationOperationKind::VolumeProtection), app::ConfigurationStepOutcome::Failed);
  QCOMPARE(step(app::ConfigurationOperationKind::VolumeBinding),
           registered ? app::ConfigurationStepOutcome::Succeeded : app::ConfigurationStepOutcome::DependencySkipped);
  QCOMPARE(step(app::ConfigurationOperationKind::AddFileExclusion),
           registered ? app::ConfigurationStepOutcome::Succeeded : app::ConfigurationStepOutcome::DependencySkipped);
  auto expected = before;
  const bool protectionWritten = timing == test::FakeUwfBackend::FaultTiming::AfterEffect;
  if (!registered)
    expected.next.volumes.emplace(volumeId, test::FakeUwfBackend::VolumeConfiguration{
                                                .volume = {volumeId, "C:", protectionWritten, false, std::nullopt}, .fileExclusions = {}, .swapfileSize = 0});
  else {
    expected.next.volumes.at(volumeId).volume.protectedVolume = protectionWritten;
    expected.next.volumes.at(volumeId).fileExclusions = {"\\Data\\dependent.bin"};
  }
  QVERIFY(system->snapshot() == expected);
  const auto trace = system->trace();
  QCOMPARE(std::ranges::count_if(trace, [](const auto& entry) { return std::holds_alternative<compat::EditFileExclusion>(entry.request); }),
           registered ? 1 : 0);
  QCOMPARE(std::ranges::count_if(trace,
                                 [](const auto& entry) {
                                   const auto* request = std::get_if<compat::SetVolume>(&entry.request);
                                   return request && request->property == compat::VolumeProperty::BindByDriveLetter;
                                 }),
           registered ? 1 : 0);
  const auto script = app::configurationChangeScript(plan);
  const auto added = std::ranges::find_if(script.commands, [](const auto& command) { return command.kind == app::UwfmgrKind::FileAddExclusion; });
  QVERIFY(added != script.commands.end());
  QCOMPARE(added->requiresPrevious, !registered);
}

}  // namespace

QTEST_GUILESS_MAIN(ConfigurationReviewViewModelTests)
#include "ConfigurationReviewViewModelTests.moc"
