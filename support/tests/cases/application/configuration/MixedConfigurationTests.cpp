// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QTest>
#include <algorithm>
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <utility>
#include <variant>
#include <vector>

#include "application/configuration/commands/UwfmgrCommandCodec.h"
#include "application/configuration/editing/ConfigurationEditSession.h"
#include "application/configuration/editing/GlobalConfigurationEditor.h"
#include "application/configuration/execution/CompositeConfigurationRepository.h"
#include "application/configuration/execution/ConfigurationExecutor.h"
#include "application/configuration/execution/ConfigurationPlanExecution.h"
#include "application/configuration/execution/ConfigurationResult.h"
#include "application/configuration/overlay/OverlayPolicyDraft.h"
#include "application/configuration/planning/ConfigurationScript.h"
#include "application/overlay/policy/OverlayPolicyRepository.h"
#include "application/overlay/policy/OverlayPolicyService.h"
#include "application/staging/persistence/FileStagingRegistry.h"
#include "application/uwf/adapters/UwfConfigurationRepository.h"
#include "application/uwf/adapters/UwfOverlayOperations.h"
#include "application/uwf/adapters/UwfReadState.h"
#include "simulation/uwf/FakeUwfBackend.h"

namespace {
using namespace uwf;
using namespace uwf::app;

std::vector<OverlayPolicyIssue> policyIssues(const OverlayPolicyValidation& validation) {
  std::vector<OverlayPolicyIssue> result;
  for (const auto& conflict : validation.conflicts) result.push_back(conflict.issue);
  return result;
}

class ObservedSystem final : public compat::Uwf {
 public:
  struct Write {
    compat::Request request;
    uwf::simulation::FakeUwfBackend::Configuration next;
  };
  const std::shared_ptr<uwf::simulation::FakeUwfBackend> backend = std::make_shared<uwf::simulation::FakeUwfBackend>();
  mutable std::vector<Write> writes;
  mutable int reads = 0;
  std::function<void(const compat::Request&)> beforeCall;
  std::function<void(const compat::Request&, const compat::Response&)> afterCall;
  compat::Capability probe(const compat::Request& request) const override { return backend->probe(request); }
  compat::UwfStateSnapshot stateSnapshot() const override { return backend->stateSnapshot(); }
  compat::Response execute(const compat::Request& request, const compat::Context& context) const override {
    if (!compat::isWrite(request)) ++reads;
    if (beforeCall) beforeCall(request);
    auto response = backend->execute(request, context);
    if (compat::isWrite(request)) writes.push_back({request, backend->snapshot().next});
    if (afterCall) afterCall(request, response);
    return response;
  }
};
class ObservedRepository final : public ConfigurationRepository {
 public:
  explicit ObservedRepository(std::shared_ptr<ConfigurationRepository> inner) : inner(std::move(inner)) {}
  bool published = true;
  bool supports(const ConfigurationArea area) const override { return inner->supports(area); }
  OverlayPolicySnapshot snapshot() const override { return published ? inner->snapshot() : OverlayPolicySnapshot{}; }
  WriteOperation prepareWrite(const ConfigurationPhase& phase, const ConfigurationPlan& plan) override { return inner->prepareWrite(phase, plan); }
  void completeWrite(const ConfigurationApplyResult& result) override { inner->completeWrite(result); }

 private:
  std::shared_ptr<ConfigurationRepository> inner;
};

class ActivationActions final : public OverlayMaintenanceActions {
 public:
  OverlayMaintenanceSnapshot state;
  std::function<void()> afterWrite;
  int writes = 0;
  OverlayMaintenanceSnapshot observe(std::stop_token) override { return state; }
  std::optional<OverlayMaintenanceSnapshot> snapshot() const override { return state; }
  OverlayMaintenanceResult fullCommit(std::stop_token) override { throw std::logic_error("Unexpected full commit"); }
  OverlayMaintenanceResult createSwap(const QString&, std::stop_token) override { throw std::logic_error("Unexpected storage change"); }
  OverlayMaintenanceResult setHorm(const bool enabled, std::stop_token) override {
    state.hormEnabled = enabled;
    return applied();
  }
  OverlayMaintenanceResult setAllVolumesProtected(const bool enabled, std::stop_token) override {
    state.allVolumesNextProtected = enabled;
    state.allVolumesNextUnprotected = !enabled;
    return applied();
  }

 private:
  OverlayMaintenanceResult applied() {
    ++writes;
    if (afterWrite) afterWrite();
    return {state, {}, {}, WriteEffect::Confirmed, true};
  }
};
struct Fixture {
  const std::shared_ptr<ObservedSystem> system = std::make_shared<ObservedSystem>();
  const std::shared_ptr<uwf::simulation::FakeUwfBackend> backend = system->backend;
  std::shared_ptr<OverlayPolicyService> policy = std::make_shared<OverlayPolicyService>(std::make_shared<UwfOverlayOperations>(system));
  core::UwfSnapshot snapshot;
  struct PolicyWrite {
    OverlayProperty node;
    std::int64_t value;
    core::OverlayType type;
    std::size_t precedingSettings;
  };
  Fixture() {
    configure([](auto& state) {
      state.current.overlayType = state.next.overlayType = static_cast<std::uint32_t>(compat::OverlayType::Disk);
      state.current.maximumOverlaySize = state.next.maximumOverlaySize = 2048;
      state.current.warningThreshold = state.next.warningThreshold = 512;
      state.current.criticalThreshold = state.next.criticalThreshold = 1024;
      state.current.overlayFlags = 2;
      state.next.overlayFlags = 0x82;
      state.current.resetPersistentStateSavedMode = state.next.resetPersistentStateSavedMode = 1;
      state.next.filterEnabled = true;
      state.runtime.controlDevicePresent = true;
    });
  }
  template <class Change>
  void configure(Change change) {
    auto state = backend->snapshot();
    change(state);
    backend->seed(std::move(state));
    snapshot = publishedUwfSnapshot(backend->state(), UwfCapability::Available, true, {});
  }
  std::uint32_t flags() const { return backend->snapshot().next.overlayFlags; }
  core::OverlayType type() const { return static_cast<core::OverlayType>(backend->snapshot().next.overlayType); }
  std::int64_t reset() const { return backend->snapshot().runtime.resetPersistentState; }
  std::vector<compat::SetNumber> settingsWrites() const {
    std::vector<compat::SetNumber> result;
    for (const auto& write : system->writes)
      if (const auto* number = std::get_if<compat::SetNumber>(&write.request);
          number && (number->property == compat::NumberProperty::OverlayType || number->property == compat::NumberProperty::MaximumOverlaySize ||
                     number->property == compat::NumberProperty::WarningThreshold || number->property == compat::NumberProperty::CriticalThreshold))
        result.push_back(*number);
    return result;
  }
  std::vector<PolicyWrite> policyWrites() const {
    std::vector<PolicyWrite> result;
    std::size_t settings = 0;
    for (const auto& write : system->writes) {
      if (std::holds_alternative<compat::SetOverlayOption>(write.request))
        result.push_back({OverlayProperty::NextOverlayFlags, write.next.overlayFlags, static_cast<core::OverlayType>(write.next.overlayType), settings});
      else if (const auto* number = std::get_if<compat::SetNumber>(&write.request)) {
        if (number->property == compat::NumberProperty::ResetPersistentState)
          result.push_back({OverlayProperty::ResetPersistentState, number->value, static_cast<core::OverlayType>(write.next.overlayType), settings});
        else
          ++settings;
      }
    }
    return result;
  }
  ConfigurationExecutor executor(std::shared_ptr<OverlayMaintenanceActions> maintenance = {}) {
    auto repository = std::make_shared<CompositeConfigurationRepository>(
        std::vector<std::shared_ptr<ConfigurationRepository>>{uwfConfigurationRepository(system, {}), overlayPolicyRepository(policy)});
    return ConfigurationExecutor(std::make_shared<ObservedRepository>(std::move(repository)), {}, std::move(maintenance));
  }
  ConfigurationPlan toRam() {
    ConfigurationChanges changes;
    changes.settings.setOverlay.type = core::OverlayType::RAM;
    changes.overlayPolicy.persistent = false;
    changes.overlayPolicy.readOnlyMedia = true;
    return planApplicationConfiguration(changes, snapshot, policy->snapshot());
  }
};

class MixedConfigurationTests final : public QObject {
  Q_OBJECT
 private slots:
  void romBatchesRejectOrdinaryWritesBeforeTheirFilterTransition_data() {
    QTest::addColumn<int>("operation");
    for (int index = 0; index < 5; ++index) QTest::newRow(qPrintable(QStringLiteral("operation-%1").arg(index))) << index;
  }
  void romBatchesRejectOrdinaryWritesBeforeTheirFilterTransition() {
    QFETCH(int, operation);
    Fixture fixture;
    fixture.configure([](auto& state) {
      state.current.filterEnabled = state.next.filterEnabled = true;
      state.current.overlayType = state.next.overlayType = static_cast<std::uint32_t>(compat::OverlayType::Ram);
      state.current.overlayFlags = state.next.overlayFlags = OverlayFlags::ReadOnlyMedia;
      state.runtime.hormEnabled = false;
      state.runtime.romDisposition = 0;
    });
    constexpr auto id = "11111111-2222-3333-4444-555555555555";
    fixture.snapshot.runtime.systemVolumeProtected = true;
    fixture.snapshot.current.volumes = {{id, "C:", true, core::VolumeBinding::VolumeId}};
    fixture.snapshot.next.volumes = fixture.snapshot.current.volumes;
    ConfigurationChanges changes;
    changes.settings.setFilterEnabled = false;
    switch (operation) {
      case 0:
        changes.settings.setPersistDomainSecretKey = true;
        break;
      case 1:
        changes.settings.volumeProtect[id] = false;
        break;
      case 2:
        changes.settings.addFileExclusions[id] = {"C:\\Data"};
        break;
      case 3:
        changes.settings.addRegistryExclusions = {"HKEY_LOCAL_MACHINE\\SOFTWARE\\Example"};
        break;
      case 4:
        changes.settings.setOverlay.warningThresholdMb = 256;
        break;
    }
    const auto reviewed = reviewApplicationConfiguration(changes, fixture.snapshot, fixture.policy->snapshot());
    QVERIFY(reviewed.count() >= 2);
    QVERIFY_THROWS_EXCEPTION(OperationError, static_cast<void>(fixture.executor().prepareApply(reviewed)));
    QVERIFY(fixture.system->writes.empty());
    // Removing the ordinary write leaves the explicitly requested Filter exit
    // available; planning must not silently use it to make the batch durable.
    ConfigurationChanges filterOnly;
    filterOnly.settings.setFilterEnabled = false;
    const auto allowed = planApplicationConfiguration(filterOnly, fixture.snapshot, fixture.policy->snapshot());
    QCOMPARE(configurationPhases(allowed).size(), std::size_t{1});
    QVERIFY(fixture.system->writes.empty());
  }
  void clearingStagingDoesNotRequireOverlayObservations_data() {
    QTest::addColumn<bool>("missingType");
    QTest::addColumn<bool>("unsupported");
    QTest::newRow("flags-failed") << false << false;
    QTest::newRow("flags-unsupported") << false << true;
    QTest::newRow("type-failed") << true << false;
    QTest::newRow("type-unsupported") << true << true;
  }
  void clearingStagingDoesNotRequireOverlayObservations() {
    QFETCH(bool, missingType);
    QFETCH(bool, unsupported);
    Fixture fixture;
    auto baseline = fixture.policy->snapshot();
    const auto invalidate = [&](auto& field) {
      field.status = unsupported ? PolicyObservationStatus::Unsupported : PolicyObservationStatus::Failed;
      field.value.reset();
    };
    if (missingType)
      invalidate(baseline.next.overlayType);
    else
      invalidate(baseline.next.flags);
    const QList<FileStagingEntry> entries{{FileStagingKind::File, QStringLiteral("C:\\saved.bin")}};
    const auto plan = planApplicationConfiguration({}, fixture.snapshot, baseline, FileStagingChanges{entries, {}, false});
    const auto phases = configurationPhases(plan);
    QCOMPARE(phases.size(), std::size_t{1});
    QVERIFY(!plan.requiresPolicyContext());
    QVERIFY(configurationChangeScript(plan).commands.empty());
    static_cast<void>(configurationTargetScript(plan));

    bool cleared = false;
    const std::vector<PreparedConfigurationPhase> prepared{
        {phases.front(), [&](const OverlayPolicySnapshot&) {
           cleared = true;
           ConfigurationApplyResult result;
           recordConfigurationStep(
               result, {{}, ConfigurationStepOutcome::Succeeded, WriteEffect::Confirmed, ConfigurationOperation{ConfigurationOperationKind::Staging, {}}});
           return result;
         }}};
    const auto result = executeConfigurationPlan(plan, prepared, plan.policySnapshot);
    QVERIFY(cleared);
    QCOMPARE(result.failedOperations(), 0);
    QCOMPARE(result.succeededOperations(), 1);

    // Editing the saved list needs no Next overlay observation. An explicit
    // overlay edit still requires its own baseline.
    const QList<FileStagingEntry> replacement{{FileStagingKind::File, QStringLiteral("C:\\replacement.bin")}};
    const auto retained = reviewApplicationConfiguration({}, fixture.snapshot, baseline, FileStagingChanges{entries, replacement, false});
    static_cast<void>(configurationPhases(retained));
    QVERIFY(!retained.requiresPolicyContext());
    ConfigurationChanges changes;
    changes.overlayPolicy.persistent = false;
    const auto overlayEdit = reviewApplicationConfiguration(changes, fixture.snapshot, baseline, FileStagingChanges{entries, {}, false});
    QVERIFY_THROWS_EXCEPTION(OverlayPolicyError, static_cast<void>(configurationPhases(overlayEdit)));
  }
  void automaticCommitModeChangesDoNotInvalidateProtectionActivation_data() {
    QTest::addColumn<int>("activation");
    QTest::addColumn<int>("drift");
    constexpr std::array operations{"filter-on", "protect-all", "horm-on"};
    constexpr std::array moments{"stable", "after-review", "during-write"};
    for (std::size_t operation = 0; operation < operations.size(); ++operation)
      for (std::size_t moment = 0; moment < moments.size(); ++moment)
        QTest::newRow(qUtf8Printable(QStringLiteral("%1/%2").arg(QLatin1StringView(operations[operation]), QLatin1StringView(moments[moment]))))
            << static_cast<int>(operation) << static_cast<int>(moment);
  }
  void automaticCommitModeChangesDoNotInvalidateProtectionActivation() {
    QFETCH(int, activation);
    QFETCH(int, drift);
    Fixture fixture;
    fixture.configure([&](auto& state) {
      state.current.filterEnabled = activation == 2;
      state.next.filterEnabled = false;
      state.current.overlayFlags = activation == 2 ? OverlayFlags::ReadOnlyMedia : 0;
      state.next.overlayFlags = 0;
      state.current.overlayType = state.next.overlayType = static_cast<std::uint32_t>(compat::OverlayType::Ram);
      state.next.registryExclusions = {std::string(kFileStagingRegistryRoot)};
    });
    auto maintenance = std::make_shared<ActivationActions>();
    auto& state = maintenance->state;
    state.filterEnabled = activation == 2;
    state.hormEnabled = false;
    state.overlayCommit = false;
    state.currentOverlayType = state.nextOverlayType = static_cast<std::int64_t>(core::OverlayType::RAM);
    state.currentFlags = OverlayFlags{activation == 2 ? OverlayFlags::ReadOnlyMedia : 0};
    state.nextFlags = OverlayFlags{0};
    state.hibernationAvailable = state.allVolumesProtected = true;
    state.romDisposition = 0;
    ConfigurationChanges changes;
    const auto kind = activation == 0   ? ConfigurationOperationKind::Filter
                      : activation == 1 ? ConfigurationOperationKind::ProtectAllVolumes
                                        : ConfigurationOperationKind::Horm;
    if (activation == 0) changes.settings.setFilterEnabled = true;
    if (activation == 1) changes.maintenance.allVolumesProtected = true;
    if (activation == 2) changes.maintenance.hormEnabled = true;
    const QList<FileStagingEntry> entries{{FileStagingKind::File, QStringLiteral("C:\\saved.bin")}};
    const auto plan = planApplicationConfiguration(changes, fixture.snapshot, fixture.policy->snapshot(), FileStagingChanges{entries, entries, false}, state);
    QVERIFY(!plan.requiresOverlayFlagObservation());
    QVERIFY(!plan.hasStagingChanges());
    const auto changeFlags = [&] {
      auto actual = fixture.backend->snapshot();
      actual.next.overlayFlags = OverlayFlags::ReadOnlyMedia;
      fixture.backend->seed(std::move(actual));
      state.nextFlags = OverlayFlags{OverlayFlags::ReadOnlyMedia};
    };
    if (drift == 1) changeFlags();
    if (drift == 2) {
      maintenance->afterWrite = changeFlags;
      fixture.system->afterCall = [&](const compat::Request& request, const compat::Response&) {
        if (const auto* value = std::get_if<compat::SetBoolean>(&request); value && value->property == compat::BooleanProperty::FilterEnabled) changeFlags();
      };
    }
    const auto reads = fixture.system->reads;
    const auto result = fixture.executor(maintenance).apply(plan);

    QCOMPARE(fixture.system->reads, reads);
    const int expectedWrites = 1;
    QCOMPARE(static_cast<int>(fixture.system->writes.size()) + maintenance->writes, expectedWrites);
    QCOMPARE(result.succeededOperations(), expectedWrites);
    QCOMPARE(result.failedOperations(), 0);
    QCOMPARE(result.batchFailures(), 0);
    QCOMPARE(result.anyWriteConfirmed(), expectedWrites != 0);
    QCOMPARE(result.reconciliationRequired(), expectedWrites != 0);
    QCOMPARE(std::ranges::count_if(result.steps, [&](const auto& step) { return step.operation && step.operation->kind == kind; }), 1);
    const auto operation = std::ranges::find_if(result.steps, [&](const auto& step) { return step.operation && step.operation->kind == kind; });
    QVERIFY(operation != result.steps.end());
    QCOMPARE(operation->outcome, ConfigurationStepOutcome::Succeeded);
  }

  void stagingTargetAdmissionDoesNotDependOnWhetherTheListWasEdited_data() {
    QTest::addColumn<bool>("rom");
    QTest::addColumn<bool>("saved");
    QTest::addColumn<bool>("target");
    QTest::addColumn<bool>("persistence");
    for (const bool rom : {false, true})
      for (const bool saved : {false, true})
        for (const bool target : {false, true})
          for (const bool persistence : {false, true})
            QTest::newRow(qPrintable(QStringLiteral("rom-%1-saved-%2-target-%3-persistence-%4").arg(rom).arg(saved).arg(target).arg(persistence)))
                << rom << saved << target << persistence;
  }
  void stagingTargetAdmissionDoesNotDependOnWhetherTheListWasEdited() {
    QFETCH(bool, rom);
    QFETCH(bool, saved);
    QFETCH(bool, target);
    QFETCH(bool, persistence);
    Fixture fixture;
    fixture.configure([](auto& state) {
      state.current.filterEnabled = false;
      state.current.overlayFlags = state.next.overlayFlags = 0;
      state.current.overlayType = state.next.overlayType = static_cast<std::uint32_t>(compat::OverlayType::Ram);
      state.next.registryExclusions = {std::string(kFileStagingRegistryRoot)};
    });
    const QList<FileStagingEntry> entries{{FileStagingKind::File, QStringLiteral("C:\\saved.bin")}};
    const FileStagingChanges staging{saved ? entries : QList<FileStagingEntry>{}, target ? entries : QList<FileStagingEntry>{}, persistence};
    ConfigurationChanges changes;
    changes.overlayPolicy.readOnlyMedia = rom;
    const auto plan = reviewApplicationConfiguration(changes, fixture.snapshot, fixture.policy->snapshot(), staging);
    QVERIFY(plan.staging);
    QCOMPARE(plan.staging->before, staging.before);
    QCOMPARE(plan.staging->after, staging.after);
    const bool writeStaging = saved != target || persistence;
    QCOMPARE(plan.hasStagingChanges(), writeStaging);
    const auto operations = plan.operations();
    QCOMPARE(std::ranges::count_if(operations, [](const auto& operation) { return operation.kind == ConfigurationOperationKind::Staging; }),
             writeStaging ? 1 : 0);
    // ROM pauses automatic commits at runtime; the saved list remains valid.
    QVERIFY(!configurationPhases(plan).empty());
    const auto script = configurationChangeScript(plan);
    QCOMPARE(std::ranges::any_of(script.omissions, [](const auto& omission) { return omission.kind == ConfigurationScriptOmissionKind::FileStaging; }),
             writeStaging);
    static_cast<void>(configurationTargetScript(plan));
    QVERIFY(fixture.system->writes.empty());
  }
  void enablingRomPreservesSavedStagingWithoutCommittingFiles() {
    Fixture fixture;
    fixture.configure([](auto& state) {
      state.current.filterEnabled = false;
      state.current.overlayFlags = state.next.overlayFlags = 0;
      state.current.overlayType = state.next.overlayType = static_cast<std::uint32_t>(compat::OverlayType::Ram);
      state.next.registryExclusions = {std::string(kFileStagingRegistryRoot)};
    });
    const QList<FileStagingEntry> entries{{FileStagingKind::File, QStringLiteral("C:\\saved.bin")}};
    ConfigurationChanges changes;
    changes.overlayPolicy.readOnlyMedia = true;
    const auto plan = planApplicationConfiguration(changes, fixture.snapshot, fixture.policy->snapshot(), FileStagingChanges{entries, entries, false});
    QVERIFY(!plan.hasStagingChanges());
    const auto result = fixture.executor().apply(plan);
    QVERIFY(!result.failed());
    QCOMPARE(result.succeededOperations(), 1);
    QCOMPARE(fixture.flags(), OverlayFlags::ReadOnlyMedia);
    QCOMPARE(fixture.system->writes.size(), std::size_t{1});
    QVERIFY(std::holds_alternative<compat::SetOverlayOption>(fixture.system->writes.front().request));
    QCOMPARE(plan.staging->after, entries);
  }
  void savedStagingDoesNotRestrictProtectionActivation_data() {
    QTest::addColumn<int>("operation");
    QTest::addColumn<bool>("saved");
    constexpr std::array names{"filter-on", "filter-off", "protect", "unprotect", "protect-all", "threshold", "horm-on", "horm-off"};
    for (std::size_t operation = 0; operation < names.size(); ++operation)
      for (const bool saved : {false, true})
        QTest::newRow(qPrintable(QStringLiteral("%1-staging-%2").arg(QLatin1StringView(names[operation])).arg(saved))) << static_cast<int>(operation) << saved;
  }
  void savedStagingDoesNotRestrictProtectionActivation() {
    QFETCH(int, operation);
    QFETCH(bool, saved);
    Fixture fixture;
    fixture.configure([](auto& state) {
      state.current.filterEnabled = false;
      state.current.overlayFlags = 0;
      state.next.overlayFlags = OverlayFlags::ReadOnlyMedia;
      state.current.overlayType = state.next.overlayType = static_cast<std::uint32_t>(compat::OverlayType::Ram);
      state.next.registryExclusions = {std::string(kFileStagingRegistryRoot)};
    });
    ConfigurationChanges changes;
    switch (operation) {
      case 0:
      case 1:
        changes.settings.setFilterEnabled = operation == 0;
        break;
      case 2:
      case 3:
        changes.settings.volumeProtect["C:"] = operation == 2;
        break;
      case 4:
        changes.maintenance.allVolumesProtected = true;
        break;
      case 5:
        changes.settings.setOverlay.warningThresholdMb = 256;
        break;
      case 6:
      case 7:
        changes.maintenance.hormEnabled = operation == 6;
        break;
    }
    QList<FileStagingEntry> entries;
    if (saved) entries.append({FileStagingKind::File, QStringLiteral("C:\\saved.bin")});
    const auto plan = reviewApplicationConfiguration(changes, fixture.snapshot, fixture.policy->snapshot(), FileStagingChanges{entries, entries, false});
    QVERIFY(!plan.hasStagingChanges());
    QVERIFY(!plan.requiresOverlayFlagObservation());
    QVERIFY(!plan.requiresPolicyContext());
    static_cast<void>(configurationPhases(plan));
    QVERIFY(fixture.system->writes.empty());
  }
  void laterFlagProjectionDoesNotInvalidateAnAcceptedActivation() {
    Fixture fixture;
    fixture.configure([](auto& state) {
      state.current.filterEnabled = false;
      state.next.filterEnabled = false;
      state.current.overlayFlags = state.next.overlayFlags = 0;
      state.current.overlayType = state.next.overlayType = static_cast<std::uint32_t>(compat::OverlayType::Ram);
      state.next.registryExclusions = {std::string(kFileStagingRegistryRoot)};
    });
    const QList<FileStagingEntry> entries{{FileStagingKind::File, QStringLiteral("C:\\saved.bin")}};
    ConfigurationChanges changes;
    changes.settings.setFilterEnabled = true;
    const auto plan = planApplicationConfiguration(changes, fixture.snapshot, fixture.policy->snapshot(), FileStagingChanges{entries, entries, false});
    fixture.system->afterCall = [&](const compat::Request& request, const compat::Response&) {
      if (const auto* value = std::get_if<compat::SetBoolean>(&request); value && value->property == compat::BooleanProperty::FilterEnabled) {
        auto state = fixture.backend->snapshot();
        state.next.overlayFlags = OverlayFlags::ReadOnlyMedia;
        fixture.backend->seed(std::move(state));
      }
    };
    const auto result = fixture.executor().apply(plan);
    QCOMPARE(result.failedOperations(), 0);
    QCOMPARE(result.batchFailures(), 0);
    QVERIFY(result.anyWriteConfirmed());
    QVERIFY(result.reconciliationRequired());
    QCOMPARE(fixture.flags(), OverlayFlags::ReadOnlyMedia);
  }
  void finalFileExclusionsAndStagingAreValidatedTogether_data() {
    QTest::addColumn<bool>("current");
    QTest::addColumn<bool>("next");
    QTest::addColumn<bool>("remove");
    QTest::addColumn<bool>("add");
    QTest::addColumn<bool>("saved");
    for (const bool current : {false, true})
      for (const bool next : {false, true})
        for (const bool remove : {false, true})
          for (const bool add : {false, true})
            for (const bool saved : {false, true})
              QTest::newRow(qPrintable(QStringLiteral("current-%1-next-%2-remove-%3-add-%4-saved-%5").arg(current).arg(next).arg(remove).arg(add).arg(saved)))
                  << current << next << remove << add << saved;
  }
  void finalFileExclusionsAndStagingAreValidatedTogether() {
    QFETCH(bool, current);
    QFETCH(bool, next);
    QFETCH(bool, remove);
    QFETCH(bool, add);
    QFETCH(bool, saved);
    constexpr auto volume = "Volume{11111111-2222-3333-4444-555555555555}";
    core::UwfSnapshot snapshot;
    snapshot.current.volumes = snapshot.next.volumes = {{volume, "C:", true, core::VolumeBinding::DriveLetter, std::string("C:")}};
    snapshot.next.registryExclusions = {std::string(kFileStagingRegistryRoot)};
    if (current) snapshot.current.fileExclusions[volume] = {R"(\Data)"};
    if (next) snapshot.next.fileExclusions[volume] = {R"(\Data)"};
    core::PendingChanges changes;
    if (remove) changes.removeFileExclusions["C:"] = {R"(\Data)"};
    if (add) changes.addFileExclusions["C:"] = {R"(\Data)"};
    const QList<FileStagingEntry> entries{{FileStagingKind::File, QStringLiteral("C:\\Data\\saved.bin")}};
    const auto plan = planConfiguration(changes, snapshot, FileStagingChanges{saved ? entries : QList<FileStagingEntry>{}, entries, false});
    const bool relationEdited = !saved || remove || add;
    const bool conflict = current || (next && !remove) || add;
    if (relationEdited && conflict) {
      QVERIFY_THROWS_EXCEPTION(std::invalid_argument, (void)configurationPhases(plan));
      QVERIFY_THROWS_EXCEPTION(std::invalid_argument, (void)configurationChangeScript(plan));
      QVERIFY_THROWS_EXCEPTION(std::invalid_argument, (void)configurationTargetScript(plan));
    } else {
      static_cast<void>(configurationPhases(plan));
      static_cast<void>(configurationChangeScript(plan));
      static_cast<void>(configurationTargetScript(plan));
    }
  }
  void flagEditEligibilityIsSharedByModalReviewAndPlanner_data() {
    QTest::addColumn<bool>("currentFilter");
    QTest::addColumn<bool>("nextFilter");
    QTest::addColumn<bool>("ram");
    QTest::addColumn<bool>("persistent");
    for (const bool current : {false, true})
      for (const bool next : {false, true})
        for (const bool ram : {false, true})
          for (const bool persistent : {false, true}) {
            if (current && ram) continue;
            QTest::newRow(qUtf8Printable(
                QStringLiteral("current-%1-next-%2-%3-%4").arg(current).arg(next).arg(ram ? "ram" : "disk", persistent ? "persistent" : "passthrough")))
                << current << next << ram << persistent;
          }
  }
  void flagEditEligibilityIsSharedByModalReviewAndPlanner() {
    QFETCH(bool, currentFilter);
    QFETCH(bool, nextFilter);
    QFETCH(bool, ram);
    QFETCH(bool, persistent);
    Fixture f;
    f.configure([&](auto& state) {
      state.current.filterEnabled = currentFilter;
      state.next.filterEnabled = nextFilter;
      state.current.overlayFlags = 1;
      state.next.overlayFlags = 0x81;
      state.next.overlayType = static_cast<std::uint32_t>(ram ? compat::OverlayType::Ram : compat::OverlayType::Disk);
    });
    const auto baseline = f.policy->snapshot();
    ConfigurationEditSession session;
    session.globalEditor()->reset(f.snapshot.next);
    session.overlayPolicyEditor()->reset(baseline, f.snapshot);
    const auto parsed = parseUwfmgrText(persistent ? "uwfmgr overlay set-persistent on" : "uwfmgr overlay set-passthrough off");
    QList<UwfmgrCommand> commands;
    for (const auto& command : parsed) commands.append(command);
    const auto imported = session.importCommands(commands);
    QCOMPARE(imported.size(), 1);
    QCOMPARE(imported.front().status, ImportResult::Status::Success);
    const auto changes = session.pendingConfiguration();
    const auto review = reviewApplicationConfiguration(changes, f.snapshot, baseline);
    OverlayPolicyEditor editor;
    editor.reset(baseline, f.snapshot);
    auto edit = editor.beginEdit();
    const auto field = persistent ? OverlayPolicyField::Persistent : OverlayPolicyField::Passthrough;
    const auto expected = currentFilter       ? OverlayPolicyIssue::CurrentFilterMustBeOff
                          : ram && persistent ? OverlayPolicyIssue::PersistentNeedsDisk
                                              : OverlayPolicyIssue::None;
    QCOMPARE(editor.disabledReason(edit, field), expected);
    edit.intent = changes.overlayPolicy;
    edit = editor.projectEdit(edit, edit.maintenance);
    QCOMPARE(editor.conflicts(edit), policyIssues(validateOverlayPolicyPlan(review)));
    const bool allowed = expected == OverlayPolicyIssue::None;
    QCOMPARE(editor.canCommit(edit), allowed);
    if (allowed) {
      const auto plan = planApplicationConfiguration(changes, f.snapshot, baseline);
      QVERIFY(validateOverlayPolicyPlan(plan).valid());
      QVERIFY(!configurationPhases(plan).empty());
      const auto script = QString::fromStdString(renderConfigurationScript(configurationChangeScript(plan)));
      QVERIFY(script.contains(persistent ? QStringLiteral("set-persistent on") : QStringLiteral("set-passthrough off")));
    } else {
      QCOMPARE(policyIssues(validateOverlayPolicyPlan(review)), std::vector{expected});
      QVERIFY_THROWS_EXCEPTION(OverlayPolicyError, (void)planApplicationConfiguration(changes, f.snapshot, baseline));
      QVERIFY_THROWS_EXCEPTION(OverlayPolicyError, (void)f.executor().prepareApply(review));
      QVERIFY_THROWS_EXCEPTION(OverlayPolicyError, (void)configurationChangeScript(review));
    }
    QVERIFY(f.settingsWrites().empty());
    QVERIFY(f.policyWrites().empty());
  }
  void resetPrerequisiteFailuresRejectEveryExecutableEntryPoint_data() {
    QTest::addColumn<bool>("missingType");
    QTest::addColumn<bool>("providerFailure");
    QTest::newRow("type-unavailable") << true << false;
    QTest::newRow("flags-unavailable") << false << false;
    QTest::newRow("type-provider-failure") << true << true;
    QTest::newRow("flags-provider-failure") << false << true;
  }
  void resetPrerequisiteFailuresRejectEveryExecutableEntryPoint() {
    QFETCH(bool, missingType);
    QFETCH(bool, providerFailure);
    Fixture f;
    f.configure([](auto& state) {
      state.current.filterEnabled = true;
      state.current.overlayFlags = OverlayFlags::Persistent;
      state.runtime.resetPersistentState = 1;
    });
    auto baseline = f.policy->snapshot();
    const auto node = missingType ? OverlayProperty::CurrentOverlayType : OverlayProperty::CurrentOverlayFlags;
    const auto failObservation = [&](auto& field) {
      field.status = PolicyObservationStatus::Failed;
      field.value.reset();
      if (providerFailure)
        field.failure = OverlayPolicyFailure{.origin = OverlayPolicyFailureOrigin::Transport,
                                             .kind = OperationFailureKind::Unavailable,
                                             .node = node,
                                             .operation = "ReadOverlay",
                                             .message = "Injected prerequisite failure",
                                             .diagnostics = "provider-details"};
    };
    if (missingType)
      failObservation(baseline.current.overlayType);
    else
      failObservation(baseline.current.flags);
    ConfigurationEditSession session;
    session.globalEditor()->reset(f.snapshot.next);
    session.overlayPolicyEditor()->reset(baseline, f.snapshot);
    QList<UwfmgrCommand> commands;
    for (const auto& command : parseUwfmgrText("uwfmgr overlay reset-persistentstate off")) commands.append(command);
    const auto report = session.importCommands(commands);
    QCOMPARE(report.size(), 1);
    QCOMPARE(report.front().status, ImportResult::Status::Success);
    const auto changes = session.pendingConfiguration();
    const auto review = reviewApplicationConfiguration(changes, f.snapshot, baseline);
    const auto validation = validateOverlayPolicyPlan(review);
    QVERIFY(!validation.valid());
    QCOMPARE(validation.conflicts.size(), std::size_t{1});
    QCOMPARE(validation.conflicts.front().issue, OverlayPolicyIssue::FlagsUnavailable);
    QCOMPARE(validation.conflicts.front().failure.node, node);
    if (providerFailure) {
      QCOMPARE(validation.conflicts.front().failure.operation, std::string("ReadOverlay"));
      QCOMPARE(validation.conflicts.front().failure.diagnostics, std::string("provider-details"));
    }
    try {
      (void)configurationPhases(review);
      QFAIL("An unobserved reset prerequisite must reject planning");
    } catch (const OverlayPolicyError& error) {
      QCOMPARE(error.failure().node, node);
      QCOMPARE(error.failure().message, validation.conflicts.front().failure.message);
      QCOMPARE(error.failure().diagnostics, validation.conflicts.front().failure.diagnostics);
    }
    QVERIFY_THROWS_EXCEPTION(OverlayPolicyError, (void)planApplicationConfiguration(changes, f.snapshot, baseline));
    QVERIFY_THROWS_EXCEPTION(OverlayPolicyError, (void)configurationChangeScript(review));
    QVERIFY_THROWS_EXCEPTION(OverlayPolicyError, (void)configurationTargetScript(review));
    QVERIFY_THROWS_EXCEPTION(OverlayPolicyError, (void)f.executor().prepareApply(review));
    QVERIFY(f.system->writes.empty());
  }
  void modalReviewAndExecutionShareResetRules_data() {
    QTest::addColumn<bool>("currentFilter");
    QTest::addColumn<bool>("currentPersistent");
    QTest::addColumn<bool>("nextPersistent");
    QTest::addColumn<bool>("targetRam");
    QTest::addColumn<bool>("scheduledClear");
    QTest::addColumn<int>("resetRequest");
    QTest::addColumn<bool>("valid");
    QTest::newRow("clear-active-persistent-disk") << true << true << true << false << false << 1 << true;
    QTest::newRow("clear-off-with-residual-persistent-disk") << false << true << true << false << false << 1 << false;
    QTest::newRow("clear-without-current-persistence") << true << false << true << false << false << 1 << false;
    QTest::newRow("clear-without-next-persistence") << true << true << false << false << false << 1 << false;
    QTest::newRow("clear-while-targeting-ram") << true << true << false << true << false << 1 << false;
    QTest::newRow("retained-clear-prevents-removing-persistence") << false << true << false << false << true << -1 << false;
    QTest::newRow("cancel-active-persistent-disk") << true << true << true << false << true << 0 << true;
    QTest::newRow("cancel-off-with-residual-persistent-disk") << false << true << true << false << true << 0 << false;
    QTest::newRow("cancel-clear-and-remove-persistence") << true << true << false << false << true << 0 << false;
    QTest::newRow("cancel-clear-and-target-ram") << true << true << false << true << true << 0 << false;
    QTest::newRow("preserve-without-current-persistence") << true << false << false << false << true << 0 << false;
  }
  void modalReviewAndExecutionShareResetRules() {
    QFETCH(bool, currentFilter);
    QFETCH(bool, currentPersistent);
    QFETCH(bool, nextPersistent);
    QFETCH(bool, targetRam);
    QFETCH(bool, scheduledClear);
    QFETCH(int, resetRequest);
    QFETCH(bool, valid);
    Fixture f;
    f.configure([&](auto& state) {
      state.current.filterEnabled = currentFilter;
      state.current.overlayFlags = currentPersistent ? compat::flags::persistent : 0;
      state.runtime.resetPersistentState = scheduledClear ? 1 : 0;
    });
    const auto baseline = f.policy->snapshot();
    ConfigurationChanges changes;
    // RAM already clears Persistent; the editor emits only remaining explicit writes.
    if (!nextPersistent && !targetRam) changes.overlayPolicy.persistent = false;
    if (targetRam) changes.settings.setOverlay.type = core::OverlayType::RAM;
    if (resetRequest >= 0) changes.overlayPolicy.reset = static_cast<PersistentReset>(resetRequest);
    const auto review = reviewApplicationConfiguration(changes, f.snapshot, baseline);
    OverlayPolicyEditor modal;
    modal.reset(baseline, f.snapshot);
    modal.setOverlayChanges(changes.settings.setOverlay);
    auto edit = modal.beginEdit();
    edit.intent = changes.overlayPolicy;
    edit = modal.projectEdit(edit, edit.maintenance);
    QCOMPARE(modal.conflicts(edit), policyIssues(validateOverlayPolicyPlan(review)));
    QCOMPARE(modal.canCommit(edit), valid);
    if (!valid) {
      QVERIFY_THROWS_EXCEPTION(OverlayPolicyError, (void)planApplicationConfiguration(changes, f.snapshot, baseline));
      QVERIFY_THROWS_EXCEPTION(OverlayPolicyError, (void)f.executor().prepareApply(review));
      QVERIFY_THROWS_EXCEPTION(OverlayPolicyError, (void)configurationChangeScript(review));
      QVERIFY(f.settingsWrites().empty());
      QVERIFY(f.policyWrites().empty());
      return;
    }
    const auto plan = planApplicationConfiguration(changes, f.snapshot, baseline);
    QVERIFY(validateOverlayPolicyPlan(plan).valid());
    const auto result = f.executor().apply(plan);
    QCOMPARE(result.failedOperations(), 0);
    if (scheduledClear && resetRequest == 0) {
      QVERIFY(!f.policyWrites().empty());
      QCOMPARE(f.policyWrites().front().node, OverlayProperty::ResetPersistentState);
      QCOMPARE(f.policyWrites().front().value, std::int64_t{0});
    }
  }
  void importedClearAndPersistenceChangesCannotBypassPlanning() {
    Fixture f;
    ConfigurationEditSession session;
    session.globalEditor()->reset(f.snapshot.next);
    session.overlayPolicyEditor()->reset(f.policy->snapshot(), f.snapshot);
    QList<UwfmgrCommand> commands;
    for (const auto& command : parseUwfmgrText("uwfmgr overlay reset-persistentstate on\nuwfmgr overlay set-persistent off")) commands.append(command);
    const auto report = session.importCommands(commands);
    QCOMPARE(report.size(), 2);
    for (const auto& row : report) QCOMPARE(row.status, ImportResult::Status::Success);
    const auto review = reviewApplicationConfiguration(session.pendingConfiguration(), f.snapshot, f.policy->snapshot());
    QCOMPARE(policyIssues(validateOverlayPolicyPlan(review)),
             (std::vector{OverlayPolicyIssue::CurrentPersistentRequired, OverlayPolicyIssue::NextPersistentRequired}));
    QVERIFY_THROWS_EXCEPTION(OverlayPolicyError, (void)f.executor().prepareApply(review));
    QVERIFY(f.policyWrites().empty());
    QVERIFY(f.settingsWrites().empty());
  }
  void standardPlansWithoutPolicyBaselineKeepTheirExistingContract() {
    core::UwfSnapshot snapshot;
    snapshot.current.filter.enabled = true;
    snapshot.next.overlay = {.type = core::OverlayType::RAM, .maximumSizeMb = 2048};
    ConfigurationChanges changes;
    changes.settings.setOverlay.type = core::OverlayType::Disk;
    const auto plan = planApplicationConfiguration(changes, snapshot, {});
    QVERIFY(!plan.hasPolicyContext);
    QVERIFY(validateOverlayPolicyPlan(plan).valid());
    const auto phases = configurationPhases(plan);
    QCOMPARE(phases.size(), std::size_t{1});
    QCOMPARE(std::get<ConfigurationSettingsPhase>(phases.front()).settings.setOverlay.type, std::optional{core::OverlayType::Disk});
    QVERIFY(std::holds_alternative<ConfigurationSettingsPhase>(phases.front()));
    snapshot.next.overlay.maximumSizeMb = 0;
    QVERIFY_THROWS_EXCEPTION(std::invalid_argument, (void)planApplicationConfiguration(changes, snapshot, {}));
  }
  void typeOnlyAcceptsDriverFlagsWithoutWritingThemBack_data() {
    QTest::addColumn<bool>("disk");
    QTest::newRow("to-disk") << true;
    QTest::newRow("to-ram") << false;
  }
  void typeOnlyAcceptsDriverFlagsWithoutWritingThemBack() {
    QFETCH(bool, disk);
    Fixture f;
    f.configure([&](auto& state) {
      state.next.overlayType = static_cast<std::uint32_t>(disk ? compat::OverlayType::Ram : compat::OverlayType::Disk);
      state.next.overlayFlags = disk ? 0x80u : 0x83u;
    });
    ConfigurationChanges changes;
    changes.settings.setOverlay.type = disk ? core::OverlayType::Disk : core::OverlayType::RAM;
    const auto plan = planApplicationConfiguration(changes, f.snapshot, f.policy->snapshot());
    const auto commands = configurationTargetScript(plan).commands;
    QVERIFY(std::ranges::none_of(commands, [](const auto& command) { return command.kind == UwfmgrKind::OverlaySetPassthrough; }));
    const auto result = f.executor().apply(plan);
    QCOMPARE(result.failedOperations(), 0);
    QVERIFY(f.policyWrites().empty());
    QCOMPARE(result.observedOverlayPolicy->next.flags.value->raw, disk ? 0x81u : 0x80u);
  }
  void unknownSideEffectOfAStandardWriteBecomesThePolicyBaseline() {
    Fixture f;
    f.system->afterCall = [backend = f.backend](const auto& request, const auto& response) {
      if (response.succeeded() && std::holds_alternative<compat::SetNumber>(request)) {
        auto state = backend->snapshot();
        state.next.overlayFlags = 0x181;
        backend->seed(std::move(state));
      }
    };
    ConfigurationChanges changes;
    changes.settings.setOverlay.maximumSizeMb = 2500;
    changes.overlayPolicy.persistent = true;
    const auto plan = planApplicationConfiguration(changes, f.snapshot, f.policy->snapshot());
    const auto result = f.executor().apply(plan);
    QCOMPARE(result.failedOperations(), 0);
    QCOMPARE(f.policyWrites().size(), std::size_t{1});
    QCOMPARE(f.policyWrites().front().value, 0x183);
    QCOMPARE(result.observedOverlayPolicy->next.flags.value->raw, 0x183u);
  }
  void unusedGetterFailureAfterTypeChangeDoesNotBlockTheFollowingWrite() {
    Fixture f;
    f.configure([](auto& state) {
      state.next.overlayType = static_cast<std::uint32_t>(compat::OverlayType::Ram);
      state.next.overlayFlags = 0x80;
    });
    ConfigurationChanges changes;
    changes.settings.setOverlay.type = core::OverlayType::Disk;
    changes.overlayPolicy.persistent = true;
    const auto plan = planApplicationConfiguration(changes, f.snapshot, f.policy->snapshot());
    f.system->afterCall = [observer = std::weak_ptr<ObservedSystem>(f.system)](const auto& request, const auto& response) {
      if (const auto system = observer.lock(); system && response.succeeded() && std::holds_alternative<compat::SetNumber>(request))
        system->beforeCall = [](const auto& observation) {
          if (observation == compat::Request{compat::GetNumber{compat::NumberProperty::OverlayFlags, compat::Session::Next}})
            throw std::runtime_error("Injected flags observation failure");
        };
    };
    f.backend->clearTrace();
    const auto reads = f.system->reads;
    const auto result = f.executor().apply(plan);
    QVERIFY(!result.failed());
    QVERIFY(result.anyWriteConfirmed() && result.reconciliationRequired());
    QCOMPARE(f.policyWrites().size(), std::size_t{1});
    QCOMPARE(result.observedOverlayPolicy->next.flags.status, PolicyObservationStatus::Available);
    QCOMPARE(result.observedOverlayPolicy->next.flags.value->raw, 0x83u);
    for (const auto& entry : f.backend->trace()) QVERIFY(compat::isWrite(entry.request));
    const auto published = f.policy->snapshot();
    QCOMPARE(published.next.flags.status, PolicyObservationStatus::Available);
    QCOMPARE(published.next.flags.value->raw, 0x83u);
    QCOMPARE(f.system->reads, reads);
  }
  void diskPersistentToRamObservesTypeSideEffectsBeforeExplicitFlags() {
    Fixture f;
    const auto plan = f.toRam();
    QCOMPARE(plan.count(), std::size_t{3});
    const auto phases = configurationPhases(plan);
    QCOMPARE(phases.size(), std::size_t{2});
    QVERIFY(std::holds_alternative<ConfigurationSettingsPhase>(phases[0]));
    QVERIFY(std::holds_alternative<ConfigurationOverlayPolicyPhase>(phases[1]));
    const auto result = f.executor().apply(plan);
    QCOMPARE(result.failedOperations(), 0);
    QCOMPARE(result.succeededOperations(), 3);
    QVERIFY(result.anyWriteConfirmed());
    QVERIFY(result.reconciliationRequired());
    QCOMPARE(f.policyWrites().size(), std::size_t{2});
    QCOMPARE(f.policyWrites()[0].value, 0x80);  // Explicit Persistent off is dispatched even after SetType cleared it.
    QCOMPARE(f.policyWrites()[1].value, 0x84);
    QCOMPARE(f.policyWrites()[0].type, core::OverlayType::RAM);
    QVERIFY(f.policyWrites()[0].precedingSettings > 0);
    QCOMPARE(result.observedOverlayPolicy->next.flags.value->raw, std::uint32_t{0x84});
  }

  void ramRomToDiskPersistentAlsoClearsBeforeTypeChange() {
    Fixture f;
    f.configure([](auto& state) {
      state.next.overlayType = static_cast<std::uint32_t>(compat::OverlayType::Ram);
      state.next.maximumOverlaySize = 512;
      state.next.overlayFlags = 0x84;
    });
    ConfigurationChanges changes;
    changes.settings.setOverlay.type = core::OverlayType::Disk;
    changes.settings.setOverlay.maximumSizeMb = 2048;
    changes.overlayPolicy.readOnlyMedia = false;
    changes.overlayPolicy.persistent = true;
    const auto plan = planApplicationConfiguration(changes, f.snapshot, f.policy->snapshot());
    const auto result = f.executor().apply(plan);
    QCOMPARE(result.failedOperations(), 0);
    QCOMPARE(f.settingsWrites(),
             (std::vector<compat::SetNumber>{{compat::NumberProperty::MaximumOverlaySize, 2048},
                                             {compat::NumberProperty::OverlayType, static_cast<std::uint32_t>(compat::OverlayType::Disk)}}));
    QCOMPARE(f.policyWrites()[0].value, 0x80);
    QCOMPARE(f.policyWrites()[0].type, core::OverlayType::RAM);
    QCOMPARE(f.policyWrites()[1].value, 0x83);
    QCOMPARE(f.policyWrites()[1].type, core::OverlayType::Disk);
  }

  void explicitPersistentOnStillConflictsWithRam() {
    Fixture f;
    ConfigurationChanges changes;
    changes.settings.setOverlay.type = core::OverlayType::RAM;
    changes.overlayPolicy.persistent = true;
    QVERIFY_THROWS_EXCEPTION(OverlayPolicyError, (void)planApplicationConfiguration(changes, f.snapshot, f.policy->snapshot()));
    QVERIFY(f.settingsWrites().empty());
    QVERIFY(f.policyWrites().empty());
  }

  void flagsFailureAfterTypeChangePreservesConfirmedTypeEffect() {
    Fixture f;
    const auto plan = f.toRam();
    f.backend->inject({.request = compat::SetOverlayOption{compat::OverlayOption::Persistent, false},
                       .operation = {},
                       .kind = compat::ErrorKind::AccessDenied,
                       .message = "Injected option rejection"});
    const auto result = f.executor().apply(plan);
    QVERIFY(result.failed());
    QVERIFY(result.anyWriteConfirmed());
    QVERIFY(result.reconciliationRequired());
    QCOMPARE(f.type(), core::OverlayType::RAM);
    QCOMPARE(f.policyWrites().size(), std::size_t{1});
    QCOMPARE(f.flags(), std::uint32_t{0x80});
  }

  void typeFailureDoesNotIssueTheLaterFlagsWrite_data() {
    QTest::addColumn<bool>("afterEffect");
    QTest::newRow("rejected-before-write") << false;
    QTest::newRow("lost-acknowledgement-after-effect") << true;
  }
  void typeFailureDoesNotIssueTheLaterFlagsWrite() {
    QFETCH(bool, afterEffect);
    Fixture f;
    const auto plan = f.toRam();
    f.backend->inject({.request = compat::SetNumber{compat::NumberProperty::OverlayType, static_cast<std::uint32_t>(compat::OverlayType::Ram)},
                       .operation = {},
                       .timing = afterEffect ? uwf::simulation::FakeUwfBackend::FaultTiming::AfterEffect : uwf::simulation::FakeUwfBackend::FaultTiming::Before,
                       .kind = compat::ErrorKind::Unavailable,
                       .message = "Injected type command failure"});
    const auto result = f.executor().apply(plan);
    QVERIFY(result.failed());
    QVERIFY(!result.anyWriteConfirmed());
    QCOMPARE(result.reconciliationRequired(), afterEffect);
    QCOMPARE(f.flags(), afterEffect ? std::uint32_t{0x80} : std::uint32_t{0x82});
    QVERIFY(f.policyWrites().empty());
    QCOMPARE(f.type(), afterEffect ? core::OverlayType::RAM : core::OverlayType::Disk);
  }

  void changedPublishedBaselineCausesNoWrites() {
    Fixture f;
    const auto plan = f.toRam();
    f.configure([](auto& state) { state.next.overlayFlags |= 0x40; });
    const auto reads = f.system->reads;
    f.backend->clearTrace();
    const auto result = f.executor().apply(plan);
    QVERIFY(result.failed());
    QVERIFY(!result.reconciliationRequired());
    QCOMPARE(result.steps.front().policyFailure->origin, OverlayPolicyFailureOrigin::Conflict);
    QVERIFY(f.settingsWrites().empty());
    QVERIFY(f.policyWrites().empty());

    QCOMPARE(f.system->reads, reads);
    QVERIFY(f.backend->trace().empty());
  }
  void missingPublishedPolicyContextBlocksBeforeAnyWrite() {
    Fixture fixture;
    const auto plan = fixture.toRam();
    auto composite = std::make_shared<CompositeConfigurationRepository>(
        std::vector<std::shared_ptr<ConfigurationRepository>>{uwfConfigurationRepository(fixture.system), overlayPolicyRepository(fixture.policy)});
    auto repository = std::make_shared<ObservedRepository>(std::move(composite));
    repository->published = false;
    const auto reads = fixture.system->reads;
    const auto result = ConfigurationExecutor(repository).apply(plan);
    QVERIFY(result.failed());
    QCOMPARE(result.batchFailures(), 1);
    QVERIFY(!result.anyWriteConfirmed());
    QVERIFY(fixture.system->writes.empty());
    QCOMPARE(fixture.system->reads, reads);
  }

  void preparedOperationOwnsPolicyAndWorksAfterExecutorDestruction() {
    Fixture f;
    ConfigurationActions::ApplyOperation operation;
    {
      auto executor = f.executor();
      operation = executor.prepareApply(f.toRam());
    }
    f.policy.reset();
    const auto result = operation();
    QCOMPARE(result.failedOperations(), 0);
    QCOMPARE(f.flags(), std::uint32_t{0x84});
  }

  void policyOnlyDoesNotPrepareTheStandardSettingsRepository() {
    Fixture f;
    class UnusedSettings final : public ConfigurationRepository {
     public:
      int writes = 0;
      bool supports(ConfigurationArea area) const override { return area == ConfigurationArea::Settings; }
      WriteOperation prepareWrite(const ConfigurationPhase&, const ConfigurationPlan&) override {
        ++writes;
        throw std::logic_error("Unexpected standard settings write");
      }
    };
    const auto unused = std::make_shared<UnusedSettings>();
    ConfigurationChanges changes;
    changes.overlayPolicy.passthrough = true;
    const auto plan = planApplicationConfiguration(changes, f.snapshot, f.policy->snapshot());
    ConfigurationExecutor executor(
        std::make_shared<CompositeConfigurationRepository>(std::vector<std::shared_ptr<ConfigurationRepository>>{unused, overlayPolicyRepository(f.policy)}));
    const auto result = executor.apply(plan);
    QCOMPARE(result.failedOperations(), 0);
    QCOMPARE(unused->writes, 0);
    QCOMPARE(f.flags(), std::uint32_t{0x83});
  }

  void optionFailureIsAttributedToItsOwnConfigurationOperation() {
    Fixture f;
    f.system->afterCall = [backend = f.backend](const auto& request, const auto& response) {
      if (response.succeeded() && request == compat::Request{compat::SetOverlayOption{compat::OverlayOption::Passthrough, true}})
        backend->inject({.request = compat::SetOverlayOption{compat::OverlayOption::Persistent, false},
                         .operation = {},
                         .kind = compat::ErrorKind::AccessDenied,
                         .message = "Injected subsequent option rejection"});
    };
    ConfigurationChanges changes;
    changes.overlayPolicy = {.passthrough = true, .persistent = false};
    const auto result = f.executor().apply(planApplicationConfiguration(changes, f.snapshot, f.policy->snapshot()));
    QCOMPARE(result.succeededOperations(), 1);
    QCOMPARE(result.failedOperations(), 1);
    const auto find = [&](const ConfigurationOperationKind kind) {
      return std::ranges::find_if(result.steps, [&](const auto& step) { return step.operation == ConfigurationOperation{kind, {}}; });
    };
    const auto passthrough = find(ConfigurationOperationKind::OverlayPassthrough);
    const auto persistent = find(ConfigurationOperationKind::OverlayPersistent);
    QVERIFY(passthrough != result.steps.end());
    QVERIFY(persistent != result.steps.end());
    QCOMPARE(passthrough->outcome, ConfigurationStepOutcome::Succeeded);
    QCOMPARE(persistent->outcome, ConfigurationStepOutcome::Failed);
    QCOMPARE(f.flags(), std::uint32_t{0x83});
  }

  void normalPolicyReadsDoNotFetchOrInterpretSavedMode() {
    Fixture f;
    f.configure([](auto& state) {
      state.runtime.resetPersistentState = 255;
      state.current.resetPersistentStateSavedMode = 0;
      state.next.resetPersistentStateSavedMode = 1;
    });
    bool readSavedMode = false;
    f.system->beforeCall = [&](const compat::Request& request) {
      if (const auto* number = std::get_if<compat::GetNumber>(&request); number && number->property == compat::NumberProperty::ResetPersistentStateSavedMode)
        readSavedMode = true;
    };
    const auto baseline = f.policy->snapshot();
    QCOMPARE(baseline.resetMode(), PersistentReset::UseSavedMode);
    QVERIFY(!readSavedMode);
    // The low-level capability remains available with its real session scope;
    // the application has no product consumer for it.
    QCOMPARE(compat::call(*f.backend, compat::GetNumber{compat::NumberProperty::ResetPersistentStateSavedMode, compat::Session::Current}), 0u);
    QCOMPARE(compat::call(*f.backend, compat::GetNumber{compat::NumberProperty::ResetPersistentStateSavedMode, compat::Session::Next}), 1u);
    ConfigurationChanges changes;
    changes.overlayPolicy.reset = PersistentReset::UseSavedMode;
    QVERIFY_THROWS_EXCEPTION(OverlayPolicyError, (void)planApplicationConfiguration(changes, f.snapshot, baseline));
    OverlayPolicyDraft draft;
    draft.reset(baseline);
    QVERIFY_THROWS_EXCEPTION(std::invalid_argument, draft.edit(changes.overlayPolicy));
    changes.overlayPolicy.reset = PersistentReset::Preserve;
    draft.edit(changes.overlayPolicy);
    QCOMPARE(draft.intent().reset, PersistentReset::Preserve);
  }

  void resetCancellationAndStorageChangesHaveIncompatibleCurrentRequirements_data() {
    QTest::addColumn<bool>("current");
    QTest::newRow("current-off") << false;
    QTest::newRow("current-on") << true;
  }
  void resetCancellationAndStorageChangesHaveIncompatibleCurrentRequirements() {
    QFETCH(bool, current);
    Fixture f;
    f.configure([&](auto& state) {
      state.current.filterEnabled = current;
      state.runtime.resetPersistentState = 1;
    });
    ConfigurationChanges changes;
    changes.settings.setOverlay.type = core::OverlayType::RAM;
    changes.overlayPolicy = {.persistent = false, .readOnlyMedia = true, .reset = PersistentReset::Preserve};
    const auto review = reviewApplicationConfiguration(changes, f.snapshot, f.policy->snapshot());
    QVERIFY_THROWS_EXCEPTION(OverlayPolicyError, (void)planApplicationConfiguration(changes, f.snapshot, f.policy->snapshot()));
    QVERIFY_THROWS_EXCEPTION(OverlayPolicyError, (void)configurationTargetScript(review));
    QVERIFY_THROWS_EXCEPTION(OverlayPolicyError, (void)f.executor().prepareApply(review));
    QVERIFY(f.policyWrites().empty());
    QVERIFY(f.settingsWrites().empty());
  }

  void failedClearCancellationSkipsPersistenceAndTypeChanges() {
    Fixture f;
    f.configure([](auto& state) {
      state.current.filterEnabled = true;
      state.runtime.resetPersistentState = 1;
    });
    ConfigurationChanges changes;
    changes.overlayPolicy.reset = PersistentReset::Preserve;
    const auto plan = planApplicationConfiguration(changes, f.snapshot, f.policy->snapshot());
    f.backend->inject({.request = compat::SetNumber{compat::NumberProperty::ResetPersistentState, 0},
                       .operation = {},
                       .kind = compat::ErrorKind::AccessDenied,
                       .message = "Injected reset cancellation rejection"});
    const auto result = f.executor().apply(plan);
    QVERIFY(result.failed());
    QVERIFY(!result.anyWriteConfirmed());
    QCOMPARE(f.policyWrites().size(), std::size_t{1});
    QCOMPARE(f.policyWrites().front().node, OverlayProperty::ResetPersistentState);
    QCOMPARE(f.reset(), std::int64_t{1});
    QCOMPARE(f.flags(), std::uint32_t{0x82});
    QVERIFY(f.settingsWrites().empty());
  }

  void fullExportDoesNotReplayAnUneditedClear() {
    Fixture f;
    f.configure([](auto& state) { state.runtime.resetPersistentState = 1; });
    ConfigurationChanges changes;
    changes.overlayPolicy.passthrough = true;
    const auto plan = planApplicationConfiguration(changes, f.snapshot, f.policy->snapshot());
    const auto commands = configurationTargetScript(plan).commands;
    QVERIFY(std::ranges::none_of(commands, [](const auto& command) { return command.kind == UwfmgrKind::OverlayResetPersistentState; }));
    for (std::size_t index = 1; index < commands.size(); ++index) QVERIFY(commands[index].requiresPrevious);
  }

  void cliExportPreservesPhaseDependenciesAndReimportsFourFields() {
    Fixture f;
    const auto commands = configurationChangeScript(f.toRam()).commands;
    QCOMPARE(commands.size(), std::size_t{3});
    QCOMPARE(commands[0].kind, UwfmgrKind::OverlaySetType);
    QCOMPARE(commands[1].kind, UwfmgrKind::OverlaySetPersistent);
    QCOMPARE(commands[1].args.front(), std::string("off"));
    QCOMPARE(commands[2].kind, UwfmgrKind::OverlaySetReadOnlyMedia);
    QVERIFY(commands[1].requiresPrevious && commands[2].requiresPrevious);
    const auto parsed = parseUwfmgrText(renderUwfmgrCommandScript(commands));
    for (const auto& command : parsed) QVERIFY(command.parseError == ParseError::None || command.parseError == ParseError::Comment);
    ConfigurationEditSession replay;
    replay.globalEditor()->reset(f.snapshot.next);
    replay.overlayPolicyEditor()->reset(f.policy->snapshot(), f.snapshot);
    QList<UwfmgrCommand> replayCommands;
    for (const auto& command : parsed)
      if (command.parseError != ParseError::Comment) replayCommands.append(command);
    const auto replayReport = replay.importCommands(replayCommands);
    QCOMPARE(replayReport.size(), 3);
    QCOMPARE(replayReport[0].status, ImportResult::Status::Success);
    QCOMPARE(replayReport[1].status, ImportResult::Status::Duplicate);  // RAM already cleared Persistent.
    QCOMPARE(replayReport[2].status, ImportResult::Status::Success);
    const auto replayPlan = planApplicationConfiguration(replay.pendingConfiguration(), f.snapshot, f.policy->snapshot());
    QCOMPARE(replayPlan.count(), std::size_t{2});
    const auto replayResult = f.executor().apply(replayPlan);
    QCOMPARE(replayResult.failedOperations(), 0);
    QCOMPARE(f.flags(), std::uint32_t{0x84});
    f.configure([](auto& state) {
      state.next.overlayType = static_cast<std::uint32_t>(compat::OverlayType::Disk);
      state.next.overlayFlags = 0x82;
    });
    ConfigurationEditSession session;
    session.globalEditor()->reset(f.snapshot.next);
    session.overlayPolicyEditor()->reset(f.policy->snapshot(), f.snapshot);
    QList<UwfmgrCommand> policyCommands;
    for (const auto& command : parseUwfmgrText(
             "uwfmgr overlay set-persistent off\nuwfmgr overlay set-rom-mode on\nuwfmgr overlay set-passthrough on\nuwfmgr overlay reset-persistentstate on"))
      policyCommands.append(command);
    const auto report = session.importCommands(policyCommands);
    QCOMPARE(report.size(), 4);
    for (const auto& row : report) QCOMPARE(row.status, ImportResult::Status::Success);
    QCOMPARE(session.pendingConfiguration().count(), std::size_t{4});
    QVERIFY(session.pendingChanges().empty());
    QCOMPARE(session.pendingConfiguration().overlayPolicy.reset, PersistentReset::Clear);
    const auto invalid = parseUwfmgrText("uwfmgr overlay reset-persistentstate 255\nuwfmgr overlay set-savedmode on");
    QVERIFY(invalid[0].parseError != ParseError::None);
    QCOMPARE(invalid[1].parseError, ParseError::Unsupported);
  }
};
}  // namespace

QTEST_APPLESS_MAIN(MixedConfigurationTests)
#include "MixedConfigurationTests.moc"
