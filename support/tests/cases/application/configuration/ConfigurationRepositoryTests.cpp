// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QStringList>
#include <QTest>
#include <algorithm>
#include <exception>
#include <memory>

#include "application/configuration/execution/CompositeConfigurationRepository.h"
#include "application/configuration/execution/ConfigurationExecutor.h"
#include "application/configuration/execution/ConfigurationResult.h"
#include "domain/uwf/policies/OverlayWritePlan.h"
#include "foundation/logging/Log.h"

namespace {
using namespace uwf;
using namespace uwf::app;

struct State {
  core::UwfSnapshot settings;
  bool failSnapshot = false;
  int snapshotReads = 0;
  OverlayPolicySnapshot policy;
  std::vector<ConfigurationPhase> writes;
  std::vector<OverlayPolicySnapshot> writeBaselines;
  std::function<void(const ConfigurationPhase&)> afterWrite;
  std::exception_ptr writeFailure;
  std::size_t failingWrite = 0;
  std::function<void(const ConfigurationPhase&, ConfigurationApplyResult&)> decorateResult;
  State() {
    settings.uwfAvailable = settings.elevated = true;
    settings.next.overlay = {.type = core::OverlayType::RAM, .maximumSizeMb = 2048, .warningThresholdMb = 512, .criticalThresholdMb = 1024};
    policy.transportAvailable = true;
    policy.current.filterEnabled = {PolicyObservationStatus::Available, false};
    policy.current.overlayType = {PolicyObservationStatus::Available, 0};
    policy.current.flags = {PolicyObservationStatus::Available, OverlayFlags{0}};
    policy.next = policy.current;
    policy.resetPersistentState = {PolicyObservationStatus::Available, 0};
  }
};

// Repository fixture for application-level plans and receipts.
class MemoryRepository final : public ConfigurationRepository {
 public:
  explicit MemoryRepository(std::shared_ptr<State> state, ConfigurationArea area) : state(std::move(state)), area(area) {}
  std::shared_ptr<State> state;
  ConfigurationArea area;
  bool supports(ConfigurationArea requested) const override { return requested == area; }
  OverlayPolicySnapshot snapshot() const override {
    ++state->snapshotReads;
    if (state->failSnapshot) throw std::runtime_error("Published policy is unavailable");
    return state->policy;
  }
  WriteOperation prepareWrite(const ConfigurationPhase& phase, const ConfigurationPlan& plan) override {
    return [state = state, phase, plan](const OverlayPolicySnapshot& baseline) {
      state->writeBaselines.push_back(baseline);
      state->writes.push_back(phase);
      if (state->writeFailure && (state->failingWrite == 0 || state->failingWrite == state->writes.size())) std::rethrow_exception(state->writeFailure);
      auto applied = plan;
      applied.changes = {};
      applied.overlayPolicy = {};
      applied.maintenance = {};
      applied.staging.reset();
      if (const auto* settings = std::get_if<ConfigurationSettingsPhase>(&phase)) {
        applied.changes = settings->settings;
        applied.staging = settings->staging;
        if (settings->settings.setOverlay.type) {
          const auto before = state->settings.next.overlay.type;
          state->settings.next.overlay.type = *settings->settings.setOverlay.type;
          state->policy.next.overlayType.value = static_cast<std::int64_t>(*settings->settings.setOverlay.type);
          if (state->policy.next.flags.value)
            state->policy.next.flags.value->raw =
                core::overlayFlagsAfterTypeChange(before, *settings->settings.setOverlay.type, state->policy.next.flags.value->raw);
        }
        if (settings->settings.setOverlay.warningThresholdMb)
          state->settings.current.overlay.warningThresholdMb = state->settings.next.overlay.warningThresholdMb =
              *settings->settings.setOverlay.warningThresholdMb;
        if (settings->settings.setOverlay.criticalThresholdMb)
          state->settings.current.overlay.criticalThresholdMb = state->settings.next.overlay.criticalThresholdMb =
              *settings->settings.setOverlay.criticalThresholdMb;
      } else if (const auto* policy = std::get_if<ConfigurationOverlayPolicyPhase>(&phase)) {
        applied.overlayPolicy = policy->changes;
        if (!baseline.next.flags.value || baseline.next.flags.value != state->policy.next.flags.value)
          throw std::runtime_error("repository received a stale write baseline");
        state->policy.next.flags.value = OverlayFlags{policy->changes.applyFlags(state->policy.next.flags.value->raw)};
        if (policy->changes.reset) state->policy.resetPersistentState.value = static_cast<std::int64_t>(*policy->changes.reset);
      } else {
        throw std::logic_error("Unexpected bulk phase");
      }
      if (state->afterWrite) state->afterWrite(phase);
      ConfigurationApplyResult result;
      for (const auto& operation : applied.operations()) {
        result.steps.push_back({"confirmed", ConfigurationStepOutcome::Succeeded, WriteEffect::Confirmed, operation});
      }
      result.observedOverlayPolicy = state->policy;
      if (state->decorateResult) state->decorateResult(phase, result);
      return result;
    };
  }
};

struct Fixture {
  std::shared_ptr<State> state = std::make_shared<State>();
  std::shared_ptr<ConfigurationRepository> settings = std::make_shared<MemoryRepository>(state, ConfigurationArea::Settings);
  std::shared_ptr<ConfigurationRepository> policy = std::make_shared<MemoryRepository>(state, ConfigurationArea::OverlayPolicy);
  std::shared_ptr<CompositeConfigurationRepository> repository = std::make_shared<CompositeConfigurationRepository>(std::vector{settings, policy});
  ConfigurationPlan plan(ConfigurationChanges changes) const { return planApplicationConfiguration(std::move(changes), state->settings, state->policy); }
};

QStringList records(const char level) {
  QStringList result;
  const auto marker = std::string(" ") + level + ' ';
  for (const auto& line : recentLogLines())
    if (line.find(marker) < line.find(']')) result.append(QString::fromStdString(line));
  return result;
}

class ConfigurationRepositoryTests final : public QObject {
  Q_OBJECT
 private slots:
  void operationAccountingAcrossPhaseBoundaries_data() {
    QTest::addColumn<int>("failureBoundary");
    QTest::addColumn<WriteEffect>("failureEffect");
    QTest::newRow("completed") << 0 << WriteEffect::None;
    QTest::newRow("preflight") << 1 << WriteEffect::None;
    for (const auto effect : {WriteEffect::None, WriteEffect::Uncertain, WriteEffect::Confirmed}) {
      QTest::newRow(qPrintable(QStringLiteral("settings-effect-%1").arg(int(effect)))) << 2 << effect;
      QTest::newRow(qPrintable(QStringLiteral("policy-effect-%1").arg(int(effect)))) << 3 << effect;
    }
  }
  void operationAccountingAcrossPhaseBoundaries() {
    QFETCH(int, failureBoundary);
    QFETCH(WriteEffect, failureEffect);
    Fixture fixture;
    ConfigurationChanges changes;
    changes.settings.setOverlay.warningThresholdMb = 640;
    changes.overlayPolicy.passthrough = true;
    const auto plan = fixture.plan(changes);
    if (failureBoundary == 1)
      fixture.state->policy.next.flags.value = OverlayFlags{OverlayFlags::Persistent};
    else if (failureBoundary > 1) {
      fixture.state->failingWrite = static_cast<std::size_t>(failureBoundary - 1);
      fixture.state->writeFailure = std::make_exception_ptr(OperationError(std::make_error_code(std::errc::io_error), "phase rejected", failureEffect));
    }
    const auto result = ConfigurationExecutor(fixture.repository).apply(plan);
    const int successful = failureBoundary == 0 ? 2 : failureBoundary == 3 ? 1 : 0;
    QCOMPARE(result.succeededOperations(), successful);
    QCOMPARE(result.failedOperations(), 0);
    QCOMPARE(result.batchFailures(), failureBoundary == 0 ? 0 : 1);
    QCOMPARE(std::ranges::count_if(result.steps, [](const auto& step) { return !step.description.empty(); }),
             static_cast<std::ptrdiff_t>(successful + (failureBoundary == 0 ? 0 : 1)));
    QCOMPARE(result.anyWriteConfirmed(), successful > 0 || failureEffect == WriteEffect::Confirmed);
    QCOMPARE(result.reconciliationRequired(), successful > 0 || failureEffect != WriteEffect::None);
    const auto operations = plan.operations();
    for (std::size_t index = 0; index < operations.size(); ++index) {
      const auto& operation = operations[index];
      QCOMPARE(std::ranges::count_if(result.steps, [&](const auto& item) { return item.operation == operation; }), std::ptrdiff_t{1});
      const auto found = std::ranges::find_if(result.steps, [&](const auto& item) { return item.operation == operation; });
      const bool completed = index < static_cast<std::size_t>(successful);
      QCOMPARE(found->outcome, completed ? ConfigurationStepOutcome::Succeeded : ConfigurationStepOutcome::NotAttempted);
      QCOMPARE(found->writeEffect, completed ? WriteEffect::Confirmed : WriteEffect::None);
      if (!completed) QVERIFY(found->description.empty());
    }
    QCOMPARE(result.steps.size(), operations.size() + (failureBoundary == 0 ? 0u : 1u));
  }
  void phaseAggregationRetainsAllReceiptsAndLatestObservations() {
    Fixture fixture;
    fixture.state->decorateResult = [](const ConfigurationPhase& phase, ConfigurationApplyResult& result) {
      const bool settings = std::holds_alternative<ConfigurationSettingsPhase>(phase);
      result.steps.back().description = settings ? "Settings applied" : "Policy applied";
      ConfigurationMaintenanceDetail maintenance;
      maintenance.snapshot.maximumOverlaySize = settings ? 1024 : 2048;
      result.observedOverlayMaintenance = maintenance.snapshot;
      result.steps.back().maintenance = maintenance;
    };
    ConfigurationChanges changes;
    changes.settings.setOverlay.warningThresholdMb = 640;
    changes.overlayPolicy.passthrough = true;
    const auto result = ConfigurationExecutor(fixture.repository).apply(fixture.plan(changes));
    QCOMPARE(result.succeededOperations(), 2);
    QCOMPARE(result.failedOperations(), 0);
    QCOMPARE(result.steps.size(), std::size_t{2});
    QCOMPARE(result.steps.front().description, std::string{"Settings applied"});
    QCOMPARE(result.steps.back().description, std::string{"Policy applied"});
    QCOMPARE(result.steps.front().maintenance->snapshot.maximumOverlaySize, std::optional<std::uint32_t>{1024});
    QCOMPARE(result.steps.back().maintenance->snapshot.maximumOverlaySize, std::optional<std::uint32_t>{2048});
    QVERIFY(result.observedOverlayMaintenance);
    QCOMPARE(result.observedOverlayMaintenance->maximumOverlaySize, std::optional<std::uint32_t>{2048});
    QCOMPARE(result.observedOverlayPolicy->next.flags.value->raw, OverlayFlags::Passthrough);
  }
  void receiptCategoriesRetainEffectsWithoutCountingSkippedOperationsAsFailures() {
    ConfigurationApplyResult result;
    result.steps = {
        {"Already disabled", ConfigurationStepOutcome::Succeeded, WriteEffect::None, ConfigurationOperation{ConfigurationOperationKind::Filter, {}}, true},
        {"Protection failed", ConfigurationStepOutcome::Failed, WriteEffect::None, ConfigurationOperation{ConfigurationOperationKind::VolumeProtection, "C:"}},
        {"Binding skipped", ConfigurationStepOutcome::DependencySkipped, WriteEffect::None,
         ConfigurationOperation{ConfigurationOperationKind::VolumeBinding, "C:"}}};
    ConfigurationPlan plan;
    plan.changes.setFilterEnabled = false;
    plan.changes.volumeProtect["C:"] = true;
    plan.changes.volumeBindings["C:"] = core::VolumeBinding::VolumeId;
    plan.changes.setOverlay.warningThresholdMb = 640;
    completeConfigurationResult(result, plan);
    completeConfigurationResult(result, plan);
    QCOMPARE(result.steps.size(), std::size_t{4});
    QCOMPARE(result.succeededOperations(), 1);
    QCOMPARE(result.failedOperations(), 1);
    QCOMPARE(result.dependencySkippedOperations(), 1);
    QCOMPARE(result.notAttemptedOperations(), 1);
    QCOMPARE(result.batchFailures(), 0);
    QVERIFY(result.failed());
    QVERIFY(!result.anyWriteConfirmed());
    QVERIFY(result.reconciliationRequired());
    result.steps.front().writeEffect = WriteEffect::Confirmed;
    recordConfigurationStep(result, {"Final observation unavailable", ConfigurationStepOutcome::VerificationFailed, WriteEffect::Uncertain});
    QCOMPARE(result.batchFailures(), 1);
    QCOMPARE(result.failedOperations(), 1);
    QCOMPARE(result.writeEffect(), WriteEffect::Uncertain);
    QVERIFY(result.anyWriteConfirmed());
  }
  void stagingReceiptBelongsToItsOperationAndSurvivesPhaseMerge() {
    ConfigurationApplyResult phase;
    phase.steps.push_back(
        {"Staging saved", ConfigurationStepOutcome::Succeeded, WriteEffect::Confirmed, ConfigurationOperation{ConfigurationOperationKind::Staging, {}}});
    phase.steps.back().staging = FileStagingWriteResult{QList<FileStagingEntry>{{FileStagingKind::File, QStringLiteral("C:\\saved.txt")}}, {}, true};
    ConfigurationApplyResult result;
    mergeConfigurationResult(result, std::move(phase));
    QVERIFY(result.staging() && result.staging()->attempted && result.staging()->observed);
    QCOMPARE(result.staging(), &*result.steps.front().staging);
    QCOMPARE(result.staging()->observed->front().path, QStringLiteral("C:\\saved.txt"));
    result.steps.clear();
    QVERIFY(!result.staging());
    QVERIFY(!result.reconciliationRequired());
  }
  void plannerAssignsStagingAndItsDependenciesToOneSettingsPhase() {
    Fixture fixture;
    fixture.state->policy.next.flags.value = OverlayFlags{OverlayFlags::ReadOnlyMedia};
    ConfigurationChanges changes;
    changes.settings.setOverlay.type = core::OverlayType::Disk;
    changes.settings.volumeProtect["C:"] = false;
    changes.overlayPolicy = {.persistent = true, .readOnlyMedia = false};
    changes.maintenance.allVolumesProtected = true;
    const FileStagingChanges staging{{}, {{FileStagingKind::File, QStringLiteral("C:\\saved.txt")}}, false};
    const auto plan = planApplicationConfiguration(changes, fixture.state->settings, fixture.state->policy, staging);
    const auto phases = configurationPhases(plan);
    QCOMPARE(phases.size(), std::size_t{5});
    QCOMPARE(std::get<ConfigurationOverlayPolicyPhase>(phases[0]).changes.readOnlyMedia, std::optional<bool>{false});
    const auto& settings = std::get<ConfigurationSettingsPhase>(phases[1]);
    QVERIFY(settings.staging);
    QVERIFY(!settings.stagingCompletionAlternatives.empty());
    QCOMPARE(settings.stagingCompletionAlternatives, plan.stagingCompletionAlternatives);
    QCOMPARE(settings.settings.setOverlay.type, std::optional{core::OverlayType::Disk});
    QVERIFY(std::holds_alternative<ConfigurationOverlayPolicyPhase>(phases[2]));
    QVERIFY(std::get<ConfigurationBulkProtectionPhase>(phases[3]).enabled);
    const auto& exceptions = std::get<ConfigurationSettingsPhase>(phases[4]);
    QVERIFY(!exceptions.staging);
    QVERIFY(exceptions.stagingCompletionAlternatives.empty());
    QVERIFY(exceptions.bulkProtectionPrecedes);
    QCOMPARE(exceptions.settings.volumeProtect.at("C:"), false);
    QCOMPARE(std::ranges::count_if(phases,
                                   [](const auto& phase) {
                                     const auto* settings = std::get_if<ConfigurationSettingsPhase>(&phase);
                                     return settings && settings->staging.has_value();
                                   }),
             std::ptrdiff_t{1});
  }
  void nestedRepositoriesExposeOnlyTheirPublishedPolicy() {
    Fixture f;
    CompositeConfigurationRepository nested({f.repository});
    QCOMPARE(nested.snapshot().next.flags.value->raw, 0u);
    f.state->policy.next.flags.value = OverlayFlags{OverlayFlags::Persistent};
    QCOMPARE(nested.snapshot().next.flags.value->raw, OverlayFlags::Persistent);
  }
  void declaredPolicyOwnerDoesNotFallBackToAnotherRepository() {
    Fixture first, second;
    first.state->failSnapshot = true;
    CompositeConfigurationRepository repository({first.policy, second.policy});
    QVERIFY_THROWS_EXCEPTION(std::runtime_error, (void)repository.snapshot());
    QCOMPARE(first.state->snapshotReads, 1);
    QCOMPARE(second.state->snapshotReads, 0);
  }
  void standalonePolicyRepositoryExecutesWithoutStandardStorage() {
    Fixture f;
    ConfigurationChanges changes;
    changes.overlayPolicy.passthrough = true;
    ConfigurationExecutor executor(f.policy);
    const auto result = executor.apply(f.plan(changes));
    QCOMPARE(result.failedOperations(), 0);

    QCOMPARE(result.observedOverlayPolicy->next.flags.value->raw, 1u);
  }
  void unknownTypeSideEffectsAreObserved_data() {
    QTest::addColumn<std::uint32_t>("returnedFlags");
    for (const auto flags : {0x80u, 0x81u, 0x100u, 0x101u}) QTest::newRow(qPrintable(QString::number(flags))) << flags;
  }
  void unknownTypeSideEffectsAreObserved() {
    QFETCH(std::uint32_t, returnedFlags);
    Fixture f;
    ConfigurationChanges changes;
    changes.settings.setOverlay.type = core::OverlayType::Disk;
    const auto plan = f.plan(changes);
    f.state->afterWrite = [&](const auto&) { f.state->policy.next.flags.value = OverlayFlags{returnedFlags}; };
    ConfigurationExecutor executor(f.repository);
    const auto result = executor.apply(plan);
    QCOMPARE(result.failedOperations(), 0);
    const bool normalized = (returnedFlags & OverlayFlags::Passthrough) != 0;
    QCOMPARE(result.succeededOperations(), 1);
    QCOMPARE(result.batchFailures(), normalized ? 0 : 1);
    QCOMPARE(result.failed(), !normalized);
    QCOMPARE(std::ranges::count(result.steps, ConfigurationStepOutcome::VerificationFailed, &ConfigurationStepResult::outcome),
             normalized ? std::ptrdiff_t{0} : std::ptrdiff_t{1});
    QCOMPARE(f.state->writes.size(), std::size_t{1});
    QVERIFY(std::holds_alternative<ConfigurationSettingsPhase>(f.state->writes[0]));
    QCOMPARE(result.observedOverlayPolicy->next.flags.value->raw, returnedFlags);
    QCOMPARE(f.state->policy.next.flags.value->raw, returnedFlags);
  }
  void laterExplicitWriteUsesTheActualPrecedingObservation() {
    Fixture f;
    ConfigurationChanges changes;
    changes.settings.setOverlay.type = core::OverlayType::Disk;
    changes.overlayPolicy.persistent = true;
    const auto plan = f.plan(changes);
    f.state->afterWrite = [&](const auto& phase) {
      if (const auto* settings = std::get_if<ConfigurationSettingsPhase>(&phase); settings && settings->settings.setOverlay.type)
        f.state->policy.next.flags.value = OverlayFlags{0x181};
    };
    const auto result = ConfigurationExecutor(f.repository).apply(plan);
    QCOMPARE(result.failedOperations(), 0);
    QCOMPARE(f.state->writes.size(), std::size_t{2});
    QCOMPARE(f.state->writeBaselines[1].next.flags.value->raw, 0x181u);
    QCOMPARE(result.observedOverlayPolicy->next.flags.value->raw, 0x183u);
    QVERIFY(!std::get<ConfigurationOverlayPolicyPhase>(f.state->writes[1]).changes.passthrough.has_value());
  }
  void receiptDrivenPhasesDoNotRequireRepositoryReads() {
    clearLogLines();
    Fixture f;
    ConfigurationChanges changes;
    changes.settings.setOverlay.type = core::OverlayType::Disk;
    changes.overlayPolicy.persistent = true;
    const auto plan = f.plan(changes);
    const auto result = ConfigurationExecutor(f.repository).apply(plan);
    QVERIFY(!result.failed());
    QVERIFY(result.anyWriteConfirmed() && result.reconciliationRequired());
    QCOMPARE(f.state->writes.size(), std::size_t{2});

    QVERIFY(records('E').isEmpty());
  }
  void staleInitialSnapshotStillPreventsEveryWrite() {
    clearLogLines();
    Fixture f;
    ConfigurationChanges changes;
    changes.overlayPolicy.passthrough = true;
    const auto plan = f.plan(changes);
    f.state->policy.next.flags.value = OverlayFlags{0x80};
    const auto result = ConfigurationExecutor(f.repository).apply(plan);
    QVERIFY(result.failed());
    QVERIFY(!result.reconciliationRequired());
    QVERIFY(f.state->writes.empty());
    QVERIFY(records('E').isEmpty());
    QCOMPARE(records('W').size(), 1);
  }
  void freshCurrentSessionBlocksFlagChangesBeforeAnySettingsWrite_data() {
    QTest::addColumn<bool>("persistent");
    QTest::addColumn<bool>("enable");
    QTest::newRow("enable-passthrough") << false << true;
    QTest::newRow("disable-passthrough") << false << false;
    QTest::newRow("enable-persistent") << true << true;
    QTest::newRow("disable-persistent") << true << false;
  }
  void freshCurrentSessionBlocksFlagChangesBeforeAnySettingsWrite() {
    QFETCH(bool, persistent);
    QFETCH(bool, enable);
    Fixture f;
    const auto bit = persistent ? OverlayFlags::Persistent : OverlayFlags::Passthrough;
    f.state->policy.next.flags.value = OverlayFlags{enable ? 0u : bit};
    f.state->policy.current.overlayType.value = f.state->policy.next.overlayType.value = 1;
    f.state->settings.next.overlay.type = core::OverlayType::Disk;
    ConfigurationChanges changes;
    changes.settings.setOverlay.warningThresholdMb = 640;
    if (persistent)
      changes.overlayPolicy.persistent = enable;
    else
      changes.overlayPolicy.passthrough = enable;
    const auto plan = f.plan(changes);
    f.state->policy.current.filterEnabled.value = true;  // Fresh observation, not the reviewed snapshot.
    f.state->settings.current.filter.enabled = true;
    clearLogLines();
    const auto result = ConfigurationExecutor(f.repository).apply(plan);
    QVERIFY(result.failed());
    QVERIFY(!result.anyWriteConfirmed() && !result.reconciliationRequired());
    QVERIFY(f.state->writes.empty());
    QVERIFY(records('E').isEmpty());
    QCOMPARE(records('W').size(), 1);
    QVERIFY(records('W').constFirst().contains(QStringLiteral("CurrentSession/FilterEnabled")));
  }
  void freshCurrentSessionDoesNotBlockUnrelatedChanges_data() {
    QTest::addColumn<int>("kind");
    QTest::newRow("threshold-only") << 0;
    QTest::newRow("cancel-reset") << 1;
  }
  void freshCurrentSessionDoesNotBlockUnrelatedChanges() {
    QFETCH(int, kind);
    Fixture f;
    ConfigurationChanges changes;
    changes.settings.setOverlay.warningThresholdMb = 640;
    if (kind == 1) {
      f.state->policy.current.overlayType.value = f.state->policy.next.overlayType.value = 1;
      f.state->settings.next.overlay.type = core::OverlayType::Disk;
      f.state->settings.current.overlay.type = core::OverlayType::Disk;
      f.state->settings.current.filter.enabled = true;
      f.state->policy.current.filterEnabled.value = true;
      f.state->policy.current.flags.value = f.state->policy.next.flags.value = OverlayFlags{OverlayFlags::Persistent};
      f.state->policy.resetPersistentState.value = 1;
      changes.overlayPolicy.reset = PersistentReset::Preserve;
    }
    const auto plan = f.plan(changes);
    f.state->policy.current.filterEnabled.value = true;
    f.state->settings.current.filter.enabled = true;
    const auto result = ConfigurationExecutor(f.repository).apply(plan);
    QCOMPARE(result.failedOperations(), 0);
    QVERIFY(!f.state->writes.empty());
    if (kind == 1) QCOMPARE(f.state->policy.resetPersistentState.value, std::optional<std::int64_t>{0});
  }
  void explicitSameValueFlagStillBlocksUnrelatedSettingsOnAnActiveFilter_data() {
    QTest::addColumn<bool>("persistent");
    QTest::newRow("passthrough") << false;
    QTest::newRow("persistent") << true;
  }
  void explicitSameValueFlagStillBlocksUnrelatedSettingsOnAnActiveFilter() {
    QFETCH(bool, persistent);
    Fixture fixture;
    ConfigurationChanges changes;
    changes.settings.setOverlay.warningThresholdMb = 640;
    if (persistent)
      changes.overlayPolicy.persistent = false;
    else
      changes.overlayPolicy.passthrough = false;
    const auto plan = fixture.plan(changes);
    fixture.state->policy.current.filterEnabled.value = true;
    fixture.state->settings.current.filter.enabled = true;
    const auto result = ConfigurationExecutor(fixture.repository).apply(plan);
    QVERIFY(result.failed());
    QVERIFY(fixture.state->writes.empty());
    QVERIFY(!result.anyWriteConfirmed() && !result.reconciliationRequired());
  }
  void executionExceptionsAreLoggedAtTheirBoundary_data() {
    QTest::addColumn<int>("kind");
    QTest::addColumn<char>("level");
    QTest::newRow("canceled") << 0 << 'I';
    QTest::newRow("rejected-without-effects") << 1 << 'E';
    QTest::newRow("uncertain") << 2 << 'E';
    QTest::newRow("non-standard") << 3 << 'E';
  }
  void executionExceptionsAreLoggedAtTheirBoundary() {
    QFETCH(int, kind);
    QFETCH(char, level);
    Fixture f;
    const std::string detail = "执行失败详情\n第二行 UTF-8 必须保留";
    if (kind == 3)
      f.state->writeFailure = std::make_exception_ptr(42);
    else
      f.state->writeFailure =
          std::make_exception_ptr(OperationError(std::make_error_code(kind == 0 ? std::errc::operation_canceled : std::errc::invalid_argument), detail,
                                                 kind == 2 ? WriteEffect::Uncertain : WriteEffect::None));
    ConfigurationChanges changes;
    changes.settings.setFilterEnabled = true;
    clearLogLines();
    const auto result = ConfigurationExecutor(f.repository).apply(f.plan(changes));
    QCOMPARE(result.failedOperations(), 0);
    QCOMPARE(result.batchFailures(), 1);
    QCOMPARE(records(level).size(), 1);
    QCOMPARE(records('E').size(), level == 'E' ? 1 : 0);
    QVERIFY(records(level).constFirst().contains(QStringLiteral("context=execute")));
    QVERIFY(records(level).constFirst().contains(kind == 3 ? QStringLiteral("error=non-standard-exception") : QString::fromStdString(detail)));
    QCOMPARE(result.reconciliationRequired(), kind == 2 || kind == 3);
    QCOMPARE(f.state->writes.size(), std::size_t{1});
  }
  void laterConflictingObservationRetainsAcknowledgedRequestsAndFailsFinalVerification() {
    Fixture f;
    f.state->policy.next.flags.value = OverlayFlags{4};
    ConfigurationChanges changes;
    changes.settings.setOverlay.type = core::OverlayType::Disk;
    changes.overlayPolicy.readOnlyMedia = false;
    const auto plan = f.plan(changes);
    f.state->afterWrite = [&](const auto& phase) {
      if (const auto* settings = std::get_if<ConfigurationSettingsPhase>(&phase); settings && settings->settings.setOverlay.type)
        f.state->policy.next.flags.value = OverlayFlags{5};
    };
    const auto result = ConfigurationExecutor(f.repository).apply(plan);
    QVERIFY(result.failed());
    QCOMPARE(result.succeededOperations(), 2);
    QCOMPARE(result.failedOperations(), 0);
    QCOMPARE(result.batchFailures(), 1);
    QCOMPARE(std::ranges::count(result.steps, ConfigurationStepOutcome::VerificationFailed, &ConfigurationStepResult::outcome), std::ptrdiff_t{1});
    for (const auto kind : {ConfigurationOperationKind::OverlayReadOnlyMedia, ConfigurationOperationKind::OverlayType})
      QCOMPARE(std::ranges::count_if(
                   result.steps,
                   [kind](const auto& step) { return step.operation && step.operation->kind == kind && step.outcome == ConfigurationStepOutcome::Succeeded; }),
               std::ptrdiff_t{1});
    QVERIFY(result.reconciliationRequired());
    QCOMPARE(f.state->writes.size(), std::size_t{2});
    QCOMPARE(result.observedOverlayPolicy->next.flags.value->raw, 5u);
    QCOMPARE(f.state->policy.next.flags.value->raw, 5u);
  }
};
}  // namespace
QTEST_APPLESS_MAIN(ConfigurationRepositoryTests)
#include "ConfigurationRepositoryTests.moc"
