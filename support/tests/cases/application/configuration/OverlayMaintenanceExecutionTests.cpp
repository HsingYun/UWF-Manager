// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QTest>
#include <algorithm>
#include <array>
#include <memory>
#include <optional>
#include <vector>

#include "application/configuration/execution/ConfigurationExecutor.h"
#include "application/configuration/planning/ConfigurationScript.h"
#include "domain/uwf/policies/OverlayWritePlan.h"

namespace {
using namespace uwf;
using namespace uwf::app;
QString storageId() { return QStringLiteral("11111111-1111-1111-1111-111111111111"); }

class Maintenance final : public OverlayMaintenanceActions {
 public:
  OverlayMaintenanceSnapshot state;
  std::shared_ptr<std::vector<std::string>> events = std::make_shared<std::vector<std::string>>();
  int observations = 0;
  bool failStorage = false;
  bool failHorm = false;
  bool failProtectAll = false;
  bool published = true;
  std::vector<AllVolumeProtectionItemResult> batchItems;
  std::optional<bool> protectedAfterBatch = true;
  std::optional<bool> unprotectedAfterBatch = false;
  Maintenance() {
    state.filterEnabled = false;
    state.hormEnabled = false;
    state.overlayCommit = false;
    state.currentOverlayType = state.nextOverlayType = 0;
    state.currentFlags = state.nextFlags = OverlayFlags{0};
    state.maximumOverlaySize = 4096;
    state.hibernationAvailable = true;
    state.allVolumesProtected = true;
    state.romDisposition = 0;
    state.volumesReadable = true;
    state.volumes.push_back({storageId(), QStringLiteral("D:"), {}, 0, true, 0, 0});
  }
  OverlayMaintenanceSnapshot observe(std::stop_token) override {
    ++observations;
    return state;
  }
  std::optional<OverlayMaintenanceSnapshot> snapshot() const override { return published ? std::optional{state} : std::nullopt; }
  OverlayMaintenanceResult fullCommit(std::stop_token) override {
    events->push_back("full-commit");
    state.overlayCommit = true;
    return {state, {}, {}, WriteEffect::Confirmed, true};
  }
  OverlayMaintenanceResult createSwap(const QString& volume, std::stop_token) override {
    events->push_back("storage");
    Q_ASSERT(volume == storageId());
    if (state.nextFlags) state.nextFlags->raw = core::overlayFlagsAfterStorage(core::overlayTypeFromValue(state.nextOverlayType), state.nextFlags->raw);
    state.nextOverlayType = 1;
    state.volumes.front().nextMarker = 1;
    if (failStorage)
      return {state, QStringLiteral("Storage failed after dispatch"), OperationFailure{OperationFailureKind::Failed, WriteEffect::Uncertain, {}},
              WriteEffect::Uncertain, false};
    return {state, {}, {}, WriteEffect::Confirmed, true};
  }
  OverlayMaintenanceResult setHorm(const bool enabled, std::stop_token) override {
    events->push_back(enabled ? "horm-on" : "horm-off");
    if (failHorm) {
      state.romDisposition = 2;
      return {state, QStringLiteral("The provider's final HORM validation failed"), OperationFailure{OperationFailureKind::Failed, WriteEffect::Uncertain, {}},
              WriteEffect::Uncertain, false};
    }
    state.hormEnabled = enabled;
    return {state, {}, {}, WriteEffect::Confirmed, true};
  }
  OverlayMaintenanceResult setAllVolumesProtected(const bool enabled, std::stop_token) override {
    events->push_back(enabled ? "protect-all" : "unprotect-all");
    state.allVolumesNextProtected = protectedAfterBatch;
    state.allVolumesNextUnprotected = unprotectedAfterBatch;
    if (failProtectAll)
      return {state,
              QStringLiteral("Protect all failed after dispatch"),
              OperationFailure{OperationFailureKind::Failed, WriteEffect::Uncertain, {}},
              WriteEffect::Uncertain,
              false,
              batchItems};
    return {state, {}, {}, WriteEffect::Confirmed, true};
  }
};
class Repository final : public ConfigurationRepository {
 public:
  explicit Repository(std::shared_ptr<std::vector<std::string>> values) : events(std::move(values)) {}
  std::shared_ptr<std::vector<std::string>> events;
  bool fail = false;
  int prepared = 0;
  std::shared_ptr<OverlayPolicySnapshot> policy = std::make_shared<OverlayPolicySnapshot>();
  bool supports(ConfigurationArea) const override { return true; }
  OverlayPolicySnapshot snapshot() const override { return *policy; }
  WriteOperation prepareWrite(const ConfigurationPhase& phase, const ConfigurationPlan& plan) override {
    ++prepared;
    auto selected = plan;
    selected.changes = {};
    selected.overlayPolicy = {};
    selected.maintenance = {};
    selected.staging.reset();
    if (const auto* settings = std::get_if<ConfigurationSettingsPhase>(&phase)) {
      selected.changes = settings->settings;
      selected.staging = settings->staging;
    } else {
      selected.overlayPolicy = std::get<ConfigurationOverlayPolicyPhase>(phase).changes;
    }
    const auto operations = selected.operations();
    return [events = events, operations, fails = fail, policy = policy, phase](const OverlayPolicySnapshot& baseline) {
      events->push_back("configuration");
      if (const auto* changes = std::get_if<ConfigurationOverlayPolicyPhase>(&phase); changes && !fails) {
        *policy = baseline;
        const auto raw = baseline.next.flags.value.value_or(OverlayFlags{}).raw;
        policy->next.flags = {PolicyObservationStatus::Available, OverlayFlags{changes->changes.applyFlags(raw)}, {}};
      }
      ConfigurationApplyResult result;
      for (const auto& operation : operations) {
        result.steps.push_back(
            {"Ordinary configuration", fails ? ConfigurationStepOutcome::Failed : ConfigurationStepOutcome::Succeeded, WriteEffect::Confirmed, operation});
      }
      return result;
    };
  }
};
ConfigurationPlan reviewed(const OverlayMaintenanceChanges& changes, const std::shared_ptr<Maintenance>& actions, bool ordinary = false) {
  core::UwfSnapshot snapshot;
  snapshot.uwfAvailable = snapshot.elevated = true;
  snapshot.current.filter.enabled = actions->state.filterEnabled.value_or(false);
  snapshot.next.overlay = {.type = core::OverlayType::RAM, .maximumSizeMb = 4096};
  ConfigurationChanges pending;
  pending.maintenance = changes;
  if (ordinary) pending.settings.setOverlay.warningThresholdMb = 512;
  return reviewApplicationConfiguration(pending, snapshot, {}, {}, actions->state);
}
const ConfigurationStepResult& step(const ConfigurationApplyResult& result, ConfigurationOperationKind kind) {
  const auto found = std::ranges::find_if(result.steps, [kind](const auto& value) { return value.operation && value.operation->kind == kind; });
  if (found == result.steps.end()) throw std::logic_error("Missing configuration operation result");
  return *found;
}

std::vector<ConfigurationStepResult> maintenanceSteps(const ConfigurationApplyResult& result) {
  std::vector<ConfigurationStepResult> receipts;
  for (const auto& step : result.steps)
    if (step.maintenance) receipts.push_back(step);
  return receipts;
}

class OverlayMaintenanceExecutionTests final : public QObject {
  Q_OBJECT
 private slots:
  void missingPublishedMaintenanceCannotUseTheReviewedSnapshot_data() {
    QTest::addColumn<int>("operation");
    QTest::newRow("storage") << 0;
    QTest::newRow("horm") << 1;
    QTest::newRow("full-commit") << 2;
    QTest::newRow("protect-all") << 3;
    QTest::newRow("unprotect-all") << 4;
  }
  void missingPublishedMaintenanceCannotUseTheReviewedSnapshot() {
    QFETCH(int, operation);
    auto actions = std::make_shared<Maintenance>();
    OverlayMaintenanceChanges changes;
    if (operation == 0)
      changes.storageVolume = storageId();
    else if (operation == 1)
      changes.hormEnabled = true;
    else if (operation == 2)
      changes.fullCommit = true;
    else
      changes.allVolumesProtected = operation == 3;
    if (operation == 1 || operation == 2) {
      actions->state.filterEnabled = true;
      actions->state.currentFlags = OverlayFlags{OverlayFlags::ReadOnlyMedia};
    }
    auto repository = std::make_shared<Repository>(actions->events);
    ConfigurationExecutor executor(repository, {}, actions);
    const auto plan = reviewed(changes, actions, true);
    QVERIFY(plan.maintenanceSnapshot.has_value());
    QCOMPARE(plan.maintenanceSnapshot->hormEnabled, std::optional{false});
    auto execute = executor.prepareApply(plan);
    actions->published = false;
    const auto result = execute();
    QCOMPARE(result.batchFailures(), 1);
    QCOMPARE(result.succeededOperations(), 0);
    QCOMPARE(result.failedOperations(), 0);
    QCOMPARE(result.writeEffect(), WriteEffect::None);
    QVERIFY(!result.reconciliationRequired());
    QVERIFY(!result.observedOverlayMaintenance);
    QVERIFY(actions->events->empty());
    QCOMPARE(actions->observations, 0);
  }
  void protectAllIsReviewedAndExecutesBeforeOtherMaintenance() {
    auto actions = std::make_shared<Maintenance>();
    actions->state.allVolumesProtected = false;
    actions->state.allVolumesNextProtected = false;
    auto repository = std::make_shared<Repository>(actions->events);
    const OverlayMaintenanceChanges changes{storageId(), false, false, true};
    const auto plan = reviewed(changes, actions, true);
    QCOMPARE(plan.count(), std::size_t{4});
    QCOMPARE(plan.operations()[1].kind, ConfigurationOperationKind::ProtectAllVolumes);
    const auto script = configurationChangeScript(plan);
    QVERIFY(script.exportSupported);
    QCOMPARE(script.commands.size(), std::size_t{4});
    QCOMPARE(script.commands[0].kind, UwfmgrKind::OverlaySetWarningThreshold);
    QCOMPARE(script.commands[1].kind, UwfmgrKind::VolumeProtectAll);
    QCOMPARE(script.commands[2].kind, UwfmgrKind::VolumeCreateSwapfile);
    QCOMPARE(script.commands[3].kind, UwfmgrKind::FilterDisableHorm);
    for (std::size_t index = 0; index < script.commands.size(); ++index) QCOMPARE(script.commands[index].requiresPrevious, index != 0);
    ConfigurationExecutor executor(repository, {}, actions);
    auto execute = executor.prepareApply(plan);
    QVERIFY(actions->events->empty());
    QCOMPARE(actions->observations, 0);
    const auto result = execute();
    QCOMPARE(*actions->events, (std::vector<std::string>{"configuration", "protect-all", "storage", "horm-off"}));
    QCOMPARE(result.succeededOperations(), 4);
    QCOMPARE(result.failedOperations(), 0);
    QCOMPARE(result.steps.size(), std::size_t{4});
    QCOMPARE(step(result, ConfigurationOperationKind::ProtectAllVolumes).outcome, ConfigurationStepOutcome::Succeeded);
    QCOMPARE(result.observedOverlayMaintenance->allVolumesNextProtected, std::optional<bool>{true});
    // Batch protection targets Next. It does not satisfy HORM's Current check.
    QCOMPARE(result.observedOverlayMaintenance->allVolumesProtected, std::optional<bool>{false});
  }
  void protectAllUsesItsReceiptEvenWhenTheAggregateProjectionIsIncomplete() {
    for (const bool enabled : {false, true})
      for (const auto next : {std::optional<bool>{}, std::optional<bool>{false}, std::optional<bool>{true}})
        for (const bool failed : {false, true}) {
          auto actions = std::make_shared<Maintenance>();
          actions->protectedAfterBatch = enabled ? next : std::optional<bool>{false};
          actions->unprotectedAfterBatch = enabled ? std::optional<bool>{false} : next;
          actions->failProtectAll = failed;
          auto repository = std::make_shared<Repository>(actions->events);
          ConfigurationExecutor executor(repository, {}, actions);
          const auto result = executor.apply(reviewed({storageId(), {}, false, enabled}, actions));
          const bool success = !failed;
          QCOMPARE(step(result, enabled ? ConfigurationOperationKind::ProtectAllVolumes : ConfigurationOperationKind::UnprotectAllVolumes).outcome,
                   success ? ConfigurationStepOutcome::Succeeded : ConfigurationStepOutcome::Failed);
          QCOMPARE(step(result, ConfigurationOperationKind::OverlayStorage).outcome,
                   success ? ConfigurationStepOutcome::Succeeded : ConfigurationStepOutcome::NotAttempted);
          QCOMPARE(result.failedOperations(), success ? 0 : 1);
          QVERIFY(result.reconciliationRequired());
          QCOMPARE(actions->events->size(), success ? std::size_t{2} : std::size_t{1});
        }
  }
  void protectAllAcceptsExplicitExceptionsAndRejectsRuntimeConflictsBeforeWriting() {
    auto actions = std::make_shared<Maintenance>();
    auto repository = std::make_shared<Repository>(actions->events);
    ConfigurationExecutor executor(repository, {}, actions);
    auto plan = reviewed({{}, {}, false, true}, actions);
    plan.changes.volumeProtect["C:"] = false;
    (void)executor.prepareApply(plan);
    QCOMPARE(repository->prepared, 1);
    actions->state.filterEnabled = true;
    actions->state.currentFlags = OverlayFlags{4};
    actions->state.allVolumesProtected = false;
    QVERIFY_THROWS_EXCEPTION(OperationError, (void)executor.prepareApply(reviewed({{}, true, false, true}, actions, true)));
    QCOMPARE(repository->prepared, 1);
    actions->state.hormEnabled = true;
    QVERIFY_THROWS_EXCEPTION(OperationError, (void)executor.prepareApply(reviewed({{}, false, false, true}, actions)));
    QCOMPARE(repository->prepared, 1);
    QVERIFY(actions->events->empty());
  }
  void partialBatchKeepsConfirmedWritesWithoutClaimingCompleteSuccess() {
    auto actions = std::make_shared<Maintenance>();
    actions->failProtectAll = true;
    actions->protectedAfterBatch = false;
    actions->batchItems.push_back({storageId(), false, true, {}, {}, WriteEffect::Confirmed, true});
    auto repository = std::make_shared<Repository>(actions->events);
    ConfigurationExecutor executor(repository, {}, actions);
    const auto result = executor.apply(reviewed({{}, {}, false, true}, actions));
    QVERIFY(result.anyWriteConfirmed());
    QVERIFY(result.reconciliationRequired());
    QCOMPARE(result.failedOperations(), 1);
    QCOMPARE(result.succeededOperations(), 0);
    QCOMPARE(maintenanceSteps(result).front().maintenance->allVolumeItems.size(), std::size_t{1});
  }
  void unprotectAllAcceptsIndividualExceptionsButConflictsWithHormEnable() {
    auto actions = std::make_shared<Maintenance>();
    auto repository = std::make_shared<Repository>(actions->events);
    ConfigurationExecutor executor(repository, {}, actions);
    auto plan = reviewed({{}, {}, false, false}, actions);
    plan.changes.volumeProtect["C:"] = true;
    (void)executor.prepareApply(plan);
    actions->state.filterEnabled = true;
    actions->state.currentFlags = OverlayFlags{4};
    QVERIFY_THROWS_EXCEPTION(OperationError, (void)executor.prepareApply(reviewed({{}, true, false, false}, actions)));
    QCOMPARE(repository->prepared, 1);
    QVERIFY(actions->events->empty());
  }
  void protectAllRechecksCapabilityBeforeOrdinaryWrites() {
    auto actions = std::make_shared<Maintenance>();
    auto repository = std::make_shared<Repository>(actions->events);
    ConfigurationExecutor executor(repository, {}, actions);
    auto execute = executor.prepareApply(reviewed({{}, {}, false, true}, actions, true));
    actions->state.protectAllCapability.support = OverlayMaintenanceSupport::Unsupported;
    const auto result = execute();
    QCOMPARE(result.failedOperations(), 0);
    QCOMPARE(result.batchFailures(), 1);
    QVERIFY(!result.anyWriteConfirmed());
    QVERIFY(actions->events->empty());
    QCOMPARE(step(result, ConfigurationOperationKind::ProtectAllVolumes).outcome, ConfigurationStepOutcome::NotAttempted);
    QCOMPARE(step(result, ConfigurationOperationKind::WarningThreshold).outcome, ConfigurationStepOutcome::NotAttempted);
  }
  void reviewAndPreparationDoNotExecuteMaintenance() {
    auto actions = std::make_shared<Maintenance>();
    auto repository = std::make_shared<Repository>(actions->events);
    const OverlayMaintenanceChanges changes{storageId(), false, false};
    const auto plan = reviewed(changes, actions, true);
    QCOMPARE(plan.maintenance, changes);
    QCOMPARE(plan.count(), std::size_t{3});
    QCOMPARE(plan.operations().size(), std::size_t{3});
    ConfigurationExecutor executor(repository, {}, actions);
    auto execute = executor.prepareApply(plan);
    QVERIFY(actions->events->empty());
    QCOMPARE(actions->observations, 0);
    const auto result = execute();
    QCOMPARE(*actions->events, (std::vector<std::string>{"configuration", "storage", "horm-off"}));
    QCOMPARE(result.succeededOperations(), 3);
    QCOMPARE(result.failedOperations(), 0);
    QCOMPARE(maintenanceSteps(result).size(), std::size_t{2});
    QCOMPARE(result.steps.size(), std::size_t{3});
    QCOMPARE(step(result, ConfigurationOperationKind::OverlayStorage).outcome, ConfigurationStepOutcome::Succeeded);
    QCOMPARE(step(result, ConfigurationOperationKind::Horm).outcome, ConfigurationStepOutcome::Succeeded);
    QVERIFY(result.observedOverlayMaintenance.has_value());
    QCOMPARE(result.observedOverlayMaintenance->volumes.front().nextMarker, std::optional<std::uint32_t>{1});
  }
  void impossibleMaintenanceCombinationsFailBeforePreparingWrites() {
    const std::array conflicts{std::pair{OverlayMaintenanceChanges{{}, true, true}, MaintenancePlanIssue::FullCommitAndHormConflict},
                               std::pair{OverlayMaintenanceChanges{storageId(), true, false}, MaintenancePlanIssue::StorageAndHormConflict},
                               std::pair{OverlayMaintenanceChanges{storageId(), {}, true}, MaintenancePlanIssue::StorageAndFullCommitConflict}};
    for (const auto& [changes, issue] : conflicts) {
      auto actions = std::make_shared<Maintenance>();
      auto repository = std::make_shared<Repository>(actions->events);
      ConfigurationExecutor executor(repository, {}, actions);
      const auto plan = reviewed(changes, actions, true);
      QCOMPARE(plan.count(), std::size_t{3});
      try {
        (void)executor.prepareApply(plan);
        QFAIL("Conflicting maintenance operations must fail validation");
      } catch (const MaintenanceValidationError& error) {
        QVERIFY(error.issue() == MaintenanceValidationIssue{issue});
        QCOMPARE(error.writeEffect(), WriteEffect::None);
      }
      QCOMPARE(repository->prepared, 0);
      QVERIFY(actions->events->empty());
    }
  }
  void freshRuntimeConflictPrecedesOrdinaryWrites() {
    auto actions = std::make_shared<Maintenance>();
    auto repository = std::make_shared<Repository>(actions->events);
    ConfigurationExecutor executor(repository, {}, actions);
    auto execute = executor.prepareApply(reviewed({storageId(), {}, false}, actions, true));
    actions->state.filterEnabled = true;
    const auto result = execute();
    QCOMPARE(result.failedOperations(), 0);
    QCOMPARE(result.batchFailures(), 1);
    QVERIFY(result.steps.front().validationIssue == std::optional<MaintenanceValidationIssue>{MaintenancePlanIssue::StorageRequiresFilterDisabled});
    QVERIFY(std::ranges::all_of(result.steps, [](const auto& step) { return step.description.empty(); }));
    QVERIFY(!result.anyWriteConfirmed());
    QVERIFY(!result.reconciliationRequired());
    QVERIFY(actions->events->empty());
    QCOMPARE(step(result, ConfigurationOperationKind::WarningThreshold).outcome, ConfigurationStepOutcome::NotAttempted);
    QCOMPARE(step(result, ConfigurationOperationKind::OverlayStorage).outcome, ConfigurationStepOutcome::NotAttempted);
    QCOMPARE(result.steps.size(), std::size_t{3});  // One admission failure and one receipt for each reviewed operation.
    for (const auto& item : result.steps)
      if (item.outcome == ConfigurationStepOutcome::NotAttempted) {
        QVERIFY(item.description.empty());
        QCOMPARE(item.writeEffect, WriteEffect::None);
      }
  }
  void ordinaryFailureStopsMaintenance() {
    auto actions = std::make_shared<Maintenance>();
    auto repository = std::make_shared<Repository>(actions->events);
    repository->fail = true;
    ConfigurationExecutor executor(repository, {}, actions);
    const auto result = executor.apply(reviewed({storageId(), false, false}, actions, true));
    QCOMPARE(*actions->events, (std::vector<std::string>{"configuration"}));
    QCOMPARE(result.failedOperations(), 1);
    QVERIFY(result.anyWriteConfirmed());
    QVERIFY(maintenanceSteps(result).empty());
    QCOMPARE(step(result, ConfigurationOperationKind::OverlayStorage).outcome, ConfigurationStepOutcome::NotAttempted);
    QCOMPARE(step(result, ConfigurationOperationKind::Horm).outcome, ConfigurationStepOutcome::NotAttempted);
  }
  void maintenanceFailurePreservesEffectsAndSkipsLaterOperations() {
    auto actions = std::make_shared<Maintenance>();
    actions->failStorage = true;
    auto repository = std::make_shared<Repository>(actions->events);
    ConfigurationExecutor executor(repository, {}, actions);
    const auto result = executor.apply(reviewed({storageId(), false, false}, actions, true));
    QCOMPARE(*actions->events, (std::vector<std::string>{"configuration", "storage"}));
    QCOMPARE(result.succeededOperations(), 1);
    QCOMPARE(result.failedOperations(), 1);
    QVERIFY(result.anyWriteConfirmed());
    QVERIFY(result.reconciliationRequired());
    QCOMPARE(maintenanceSteps(result).size(), std::size_t{1});
    QCOMPARE(maintenanceSteps(result).front().writeEffect, WriteEffect::Uncertain);
    QCOMPARE(step(result, ConfigurationOperationKind::OverlayStorage).outcome, ConfigurationStepOutcome::Failed);
    QCOMPARE(step(result, ConfigurationOperationKind::Horm).outcome, ConfigurationStepOutcome::NotAttempted);
    QCOMPARE(result.observedOverlayMaintenance->volumes.front().nextMarker, std::optional<std::uint32_t>{1});
  }
  void fullCommitAndHormAreBothStandardApplyOperations() {
    for (const bool fullCommit : {false, true}) {
      auto actions = std::make_shared<Maintenance>();
      actions->state.filterEnabled = true;
      actions->state.currentFlags = OverlayFlags{4};
      auto repository = std::make_shared<Repository>(actions->events);
      ConfigurationExecutor executor(repository, {}, actions);
      const auto changes = fullCommit ? OverlayMaintenanceChanges{{}, {}, true} : OverlayMaintenanceChanges{{}, true, false};
      const auto result = executor.apply(reviewed(changes, actions));
      QCOMPARE(result.succeededOperations(), 1);
      QCOMPARE(result.failedOperations(), 0);
      QCOMPARE(maintenanceSteps(result).size(), std::size_t{1});
      QCOMPARE(result.steps.size(), std::size_t{1});
      QCOMPARE(result.steps.front().operation->kind, fullCommit ? ConfigurationOperationKind::FullCommit : ConfigurationOperationKind::Horm);
      QCOMPARE(*actions->events, (std::vector<std::string>{fullCommit ? "full-commit" : "horm-on"}));
    }
  }
  void unsupportedExecutorCannotSilentlyDropMaintenance() {
    auto actions = std::make_shared<Maintenance>();
    auto repository = std::make_shared<Repository>(actions->events);
    ConfigurationExecutor executor(repository);
    QVERIFY_THROWS_EXCEPTION(OperationError, (void)executor.prepareApply(reviewed({storageId(), {}, false}, actions)));
    QCOMPARE(repository->prepared, 0);
  }
  void activeHormAllowsOnlyTheStandaloneDisableDraft() {
    auto actions = std::make_shared<Maintenance>();
    actions->state.hormEnabled = true;
    actions->state.filterEnabled = true;
    actions->state.currentFlags = OverlayFlags{4};
    actions->state.hibernationAvailable.reset();
    actions->state.allVolumesProtected.reset();
    actions->state.romDisposition.reset();
    auto repository = std::make_shared<Repository>(actions->events);
    ConfigurationExecutor executor(repository, {}, actions);
    // A matching HORM request and HORM disable do not require enable-only facts.
    static_cast<void>(executor.prepareApply(reviewed({{}, true, false}, actions)));
    const auto mixed = reviewed({{}, false, false}, actions, true);
    QVERIFY_THROWS_EXCEPTION(OperationError, (void)executor.prepareApply(mixed));
    QCOMPARE(repository->prepared, 0);
    const auto result = executor.apply(reviewed({{}, false, false}, actions));
    QCOMPARE(result.failedOperations(), 0);
    QCOMPARE(result.succeededOperations(), 1);
    QCOMPARE(*actions->events, (std::vector<std::string>{"horm-off"}));
    QCOMPARE(result.observedOverlayMaintenance->hormEnabled, std::optional<bool>{false});
  }
  void changedOrUnreadableHormPrerequisitesStopEveryWrite_data() {
    QTest::addColumn<int>("condition");
    QTest::addColumn<bool>("unknown");
    QTest::addColumn<quint32>("value");
    const std::array<quint32, 6> incompatible{0, 1, 5, 0, 0, 1};
    for (int condition = 0; condition != 6; ++condition) {
      QTest::newRow(qPrintable(QStringLiteral("condition-%1-unreadable").arg(condition))) << condition << true << quint32{0};
      QTest::newRow(qPrintable(QStringLiteral("condition-%1-changed").arg(condition)))
          << condition << false << incompatible[static_cast<std::size_t>(condition)];
    }
    for (const quint32 disposition : {quint32{2}, quint32{3}, quint32{4}, quint32{0x80000000}})
      QTest::newRow(qPrintable(QStringLiteral("nonidle-disposition-%1").arg(disposition))) << 5 << false << disposition;
  }
  void changedOrUnreadableHormPrerequisitesStopEveryWrite() {
    QFETCH(int, condition);
    QFETCH(bool, unknown);
    QFETCH(quint32, value);
    auto actions = std::make_shared<Maintenance>();
    actions->state.filterEnabled = true;
    actions->state.currentFlags = OverlayFlags{4};
    auto repository = std::make_shared<Repository>(actions->events);
    ConfigurationExecutor executor(repository, {}, actions);
    // Revalidate the changed maintenance state before the ordinary threshold phase writes.
    auto execute = executor.prepareApply(reviewed({{}, true, false}, actions, true));
    switch (condition) {
      case 0:
        actions->state.filterEnabled = unknown ? std::nullopt : std::optional<bool>{value != 0};
        break;
      case 1:
        actions->state.currentOverlayType = unknown ? std::nullopt : std::optional<std::int64_t>{value};
        break;
      case 2:
        actions->state.currentFlags = unknown ? std::nullopt : std::optional<OverlayFlags>{OverlayFlags{value}};
        break;
      case 3:
        actions->state.hibernationAvailable = unknown ? std::nullopt : std::optional<bool>{value != 0};
        break;
      case 4:
        actions->state.allVolumesProtected = unknown ? std::nullopt : std::optional<bool>{value != 0};
        break;
      case 5:
        actions->state.romDisposition = unknown ? std::nullopt : std::optional<std::uint32_t>{value};
        break;
    }
    const auto result = execute();
    QCOMPARE(result.failedOperations(), 0);
    QCOMPARE(result.batchFailures(), 1);
    QCOMPARE(result.succeededOperations(), 0);
    QVERIFY(result.steps.front().validationIssue.has_value());
    QVERIFY(std::holds_alternative<HormIssue>(*result.steps.front().validationIssue));
    QVERIFY(std::ranges::all_of(result.steps, [](const auto& step) { return step.description.empty(); }));
    QVERIFY(!result.anyWriteConfirmed());
    QVERIFY(!result.reconciliationRequired());
    QVERIFY(actions->events->empty());
    QCOMPARE(step(result, ConfigurationOperationKind::WarningThreshold).outcome, ConfigurationStepOutcome::NotAttempted);
    QCOMPARE(step(result, ConfigurationOperationKind::Horm).outcome, ConfigurationStepOutcome::NotAttempted);
    QCOMPARE(actions->observations, 0);
  }
  void readOnlyPrechecksDoNotReplaceTheProvidersFinalValidation() {
    auto actions = std::make_shared<Maintenance>();
    actions->state.filterEnabled = true;
    actions->state.currentFlags = OverlayFlags{4};
    actions->failHorm = true;
    auto repository = std::make_shared<Repository>(actions->events);
    ConfigurationExecutor executor(repository, {}, actions);
    const auto result = executor.apply(reviewed({{}, true, false}, actions, true));
    QCOMPARE(*actions->events, (std::vector<std::string>{"configuration", "horm-on"}));
    QCOMPARE(result.failedOperations(), 1);
    QCOMPARE(result.succeededOperations(), 1);
    QVERIFY(result.anyWriteConfirmed());
    QVERIFY(result.reconciliationRequired());
    QCOMPARE(step(result, ConfigurationOperationKind::Horm).outcome, ConfigurationStepOutcome::Failed);
    QCOMPARE(maintenanceSteps(result).front().writeEffect, WriteEffect::Uncertain);
    QCOMPARE(result.observedOverlayMaintenance->romDisposition, std::optional<std::uint32_t>{2});
  }
  void storageExecutesBeforeExplicitFlagsAndPreservesTheirFinalValue() {
    auto actions = std::make_shared<Maintenance>();
    actions->state.nextOverlayType = 1;
    auto repository = std::make_shared<Repository>(actions->events);
    auto& policy = *repository->policy;
    policy.transportAvailable = true;
    policy.current.filterEnabled = {PolicyObservationStatus::Available, false, {}};
    policy.current.overlayType = {PolicyObservationStatus::Available, 0, {}};
    policy.current.flags = {PolicyObservationStatus::Available, OverlayFlags{0}, {}};
    policy.next.filterEnabled = {PolicyObservationStatus::Available, true, {}};
    policy.next.overlayType = {PolicyObservationStatus::Available, 1, {}};
    policy.next.flags = {PolicyObservationStatus::Available, OverlayFlags{0}, {}};
    policy.resetPersistentState = {PolicyObservationStatus::Available, 0, {}};
    auto plan = reviewed({storageId(), {}, false}, actions);
    plan.snapshot.next.overlay.type = core::OverlayType::Disk;
    plan.overlayPolicy.persistent = true;
    plan.policySnapshot = policy;
    plan.hasPolicyContext = true;
    ConfigurationExecutor executor(repository, {}, actions);
    const auto result = executor.apply(plan);
    QCOMPARE(*actions->events, (std::vector<std::string>{"storage", "configuration"}));
    QCOMPARE(result.succeededOperations(), 2);
    QCOMPARE(result.failedOperations(), 0);
    QCOMPARE(step(result, ConfigurationOperationKind::OverlayPersistent).outcome, ConfigurationStepOutcome::Succeeded);
    QCOMPARE(step(result, ConfigurationOperationKind::OverlayStorage).outcome, ConfigurationStepOutcome::Succeeded);
    QVERIFY(result.reconciliationRequired());
    QVERIFY(!result.failed());
    QVERIFY(!maintenanceSteps(result).front().failure);
    QCOMPARE(policy.next.overlayType.value, std::optional<std::int64_t>{1});
    QCOMPARE(policy.next.flags.value->raw, OverlayFlags::Passthrough | OverlayFlags::Persistent);
    QCOMPARE(result.observedOverlayPolicy->next.flags.value->raw, OverlayFlags::Passthrough | OverlayFlags::Persistent);
    QCOMPARE(result.observedOverlayMaintenance->nextFlags->raw, OverlayFlags::Passthrough | OverlayFlags::Persistent);
  }
  void scriptsExportNativeMaintenanceWithExplicitRuntimeScope() {
    auto actions = std::make_shared<Maintenance>();
    const auto plan = reviewed({storageId(), false, false}, actions, true);
    const auto changes = configurationChangeScript(plan);
    const auto next = configurationTargetScript(plan);
    for (const auto& script : {changes, next}) {
      QVERIFY(script.exportSupported);
      QVERIFY(std::ranges::any_of(script.commands, [](const auto& command) { return command.kind == UwfmgrKind::VolumeCreateSwapfile; }));
      QVERIFY(std::ranges::none_of(script.commands, [](const auto& command) { return command.kind == UwfmgrKind::OverlayCommit; }));
      QVERIFY(!renderConfigurationScript(script).empty());
    }
    QVERIFY(std::ranges::any_of(changes.commands, [](const auto& command) { return command.kind == UwfmgrKind::FilterDisableHorm; }));
    QVERIFY(std::ranges::none_of(next.commands, [](const auto& command) { return command.kind == UwfmgrKind::FilterDisableHorm; }));
    QVERIFY(std::ranges::any_of(next.omissions, [](const auto& omission) { return omission.kind == ConfigurationScriptOmissionKind::CurrentSessionAction; }));
  }
};
}  // namespace
QTEST_APPLESS_MAIN(OverlayMaintenanceExecutionTests)
#include "OverlayMaintenanceExecutionTests.moc"
