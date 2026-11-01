// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QStringList>
#include <QTest>
#include <algorithm>
#include <array>
#include <exception>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "application/enhanced/EnhancedModeOperations.h"
#include "application/operations/contracts/OperationError.h"
#include "foundation/logging/Log.h"
#include "support/app/EnhancedModeFixture.h"
#include "support/app/OperationFailureFixture.h"
#include "support/app/UwfMaintenanceFixture.h"

namespace {
using namespace uwf;
using namespace uwf::test::application;

class EnhancedModeOperationsTests final : public QObject {
  Q_OBJECT
  enum class RemovalCoverage { None, DriverBypass, ManagementOnly };
  Q_ENUM(RemovalCoverage)
  enum class RemovalFailure { None, Cleanup, PrepareTarget, BeforeCommit, AfterVisibleDeletion, AfterCommit };
  Q_ENUM(RemovalFailure)

 private slots:
  void persistenceResultsRetainItemAndBatchTypedEvidence() {
    using namespace app;
    for (const bool batchFailure : {false, true})
      for (const auto kind : {OperationFailureKind::NotFound, OperationFailureKind::Unsupported, OperationFailureKind::Protocol})
        for (const auto effect : {WriteEffect::None, WriteEffect::Confirmed, WriteEffect::Uncertain}) {
          UwfMaintenanceFixture fixture;
          auto service = std::make_shared<MemoryEnhancedModeServiceControl>();
          const RegistryCommitTarget target{"HKLM\\SOFTWARE\\EnhancedTypedEvidence", "Value"};
          service->installationPlan = {target};
          fixture.stageRegistryValues(service->installationPlan);
          const auto failure = std::make_exception_ptr(DetailedOperationFailure(kind, effect));
          if (batchFailure)
            fixture.state->registryTargetFailure = failure;
          else
            fixture.state->registryCommitFailure = failure;
          EnhancedModeOperations operations(service, fixture.repository(), UwfCapability::Available);
          const auto result = operations.enable(QStringLiteral("structured persistence fixture"));
          QCOMPARE(result.status.state, EnhancedModeState::Enabled);
          QVERIFY(result.persistenceWarning.contains(QStringLiteral("Original provider failure")));
          const OperationFailure* evidence = nullptr;
          if (batchFailure) {
            QVERIFY(result.persistenceFailure);
            QVERIFY(!result.persistence);
            evidence = &*result.persistenceFailure;
          } else {
            QVERIFY(!result.persistenceFailure);
            QVERIFY(result.persistence);
            QCOMPARE(result.persistence->attempted, std::size_t{1});
            QCOMPARE(result.persistence->committed, std::size_t{0});
            QCOMPARE(result.persistence->failures.size(), std::size_t{1});
            const auto& item = result.persistence->failures.front();
            QCOMPARE(item.target.key, target.key);
            QCOMPARE(item.target.valueName, target.valueName);
            QVERIFY(item.operation);
            evidence = &*item.operation;
            QCOMPARE(result.persistence->writeEffect(), effect);
          }
          QCOMPARE(evidence->kind, kind);
          QCOMPARE(evidence->effect, effect);
          QCOMPARE(evidence->diagnostic.code, int32_t{1234});
          QCOMPARE(evidence->diagnostic.result, uint32_t{0x80070005u});
        }
  }
  void init() { clearLogLines(); }
  void disablingPreservesTheServiceAndPersistsItsStartupSetting();
  void disablingReportsPersistenceAndServiceFailures_data();
  void disablingReportsPersistenceAndServiceFailures();
  void enhancedModeServiceContractRejectsEveryBrokenInvariant();
  void enhancedModeRemovalPreflightsProtectedRegistryPersistence();
  void persistenceAdmissionPrecedesServiceChanges_data();
  void persistenceAdmissionPrecedesServiceChanges();
  void removalRechecksNewWritesAfterReadingItsDeletionPlan();
  void persistenceFailuresKeepOneOriginalBatchLog_data();
  void persistenceFailuresKeepOneOriginalBatchLog();
  void itemPersistenceWarningsDoNotRepeatRegistryFailureLogs();
  void notFoundPersistenceIsNotSilentlyAccepted_data();
  void notFoundPersistenceIsNotSilentlyAccepted();
  void removalPersistsOnlyAfterTheOwnerDeletesRegistryArtifacts_data();
  void removalPersistsOnlyAfterTheOwnerDeletesRegistryArtifacts();
};

void EnhancedModeOperationsTests::disablingPreservesTheServiceAndPersistsItsStartupSetting() {
  UwfMaintenanceFixture fixture;
  auto service = std::make_shared<MemoryEnhancedModeServiceControl>();
  service->current = completeEnhancedModeStatus();
  const std::string key = "HKEY_LOCAL_MACHINE\\SYSTEM\\ControlSet001\\Services\\UwfEnhancedFixture";
  const std::string value = "Start";
  service->installationPlan = {{key, value}};
  fixture.backend->seedRegistryValue(key, value, "automatic");
  service->beforeDisable = [&] { fixture.backend->writeRegistryValue(key, value, "disabled"); };
  service->beforeInstall = [&] { fixture.backend->writeRegistryValue(key, value, "automatic"); };
  service->beforeDeletionPlan = [] { throw std::runtime_error("disable must not prepare a deletion"); };
  service->beforeDeleteRemnants = [] { throw std::runtime_error("disable must not delete service registry data"); };
  app::EnhancedModeOperations operations(service, fixture.repository(), app::UwfCapability::Available);

  const auto disabled = operations.disable();
  QVERIFY(disabled.persistenceWarning.isEmpty());
  QCOMPARE(disabled.status.state, app::EnhancedModeState::Disabled);
  QCOMPARE(disabled.status.availableAction(), app::EnhancedModeAction::Enable);
  QVERIFY(disabled.status.serviceExists);
  QVERIFY(disabled.status.serviceRegistryPresent);
  QVERIFY(disabled.status.serviceInstallationSatisfied());
  QVERIFY(disabled.status.startupDisabled);
  QVERIFY(!disabled.status.automaticStart);
  QVERIFY(!disabled.status.running);
  QVERIFY(!disabled.status.serviceContractSatisfied());
  QCOMPARE(service->disables, 1);
  QCOMPARE(service->removals, 0);
  QVERIFY(fixture.state->registryDeletions.empty());
  QCOMPARE(fixture.state->registryCommits.size(), std::size_t{1});
  fixture.backend->simulateRestart();
  QCOMPARE(fixture.backend->visibleRegistryValue(key, value), std::optional<std::string>{"disabled"});

  const auto enabled = operations.enable(QStringLiteral("restored service"));
  QVERIFY(enabled.persistenceWarning.isEmpty());
  QVERIFY(enabled.status.serviceContractSatisfied());
  QVERIFY(!enabled.status.startupDisabled);
  QCOMPARE(service->removals, 0);
  fixture.backend->simulateRestart();
  QCOMPARE(fixture.backend->visibleRegistryValue(key, value), std::optional<std::string>{"automatic"});

  service->beforeDeletionPlan = {};
  service->beforeDeleteRemnants = {};
  service->deletionPlan = {app::RegistryCommitTarget::wholeKey(key)};
  service->afterDeletionBarrier = [&] { fixture.backend->deleteRegistryKey(key); };
  const auto removed = operations.remove();
  QVERIFY(removed.persistenceWarning.isEmpty());
  QVERIFY(!removed.status.hasRemnants());
  QCOMPARE(service->removals, 1);
  fixture.backend->simulateRestart();
  QVERIFY(!fixture.backend->visibleRegistryKey(key));
}

void EnhancedModeOperationsTests::disablingReportsPersistenceAndServiceFailures_data() {
  QTest::addColumn<bool>("serviceFailure");
  QTest::newRow("service-change-fails") << true;
  QTest::newRow("persistence-fails") << false;
}

void EnhancedModeOperationsTests::disablingReportsPersistenceAndServiceFailures() {
  QFETCH(bool, serviceFailure);
  UwfMaintenanceFixture fixture;
  auto service = std::make_shared<MemoryEnhancedModeServiceControl>();
  service->current = completeEnhancedModeStatus();
  service->installationPlan = {{"HKEY_LOCAL_MACHINE\\SOFTWARE\\UwfEnhancedDisableFixture", "Start"}};
  fixture.stageRegistryValues(service->installationPlan);
  if (serviceFailure)
    service->beforeDisable = [] { throw std::runtime_error("service stop failed"); };
  else
    fixture.state->registryCommitFailure = std::make_exception_ptr(std::runtime_error("startup setting could not be saved"));
  app::EnhancedModeOperations operations(service, fixture.repository(), app::UwfCapability::Available);
  if (serviceFailure) {
    QVERIFY_THROWS_EXCEPTION(std::runtime_error, static_cast<void>(operations.disable()));
    QVERIFY(operations.status().serviceContractSatisfied());
    QVERIFY(fixture.state->registryCommits.empty());
  } else {
    const auto result = operations.disable();
    QCOMPARE(result.status.state, app::EnhancedModeState::Disabled);
    QVERIFY(result.status.serviceExists);
    QVERIFY(result.persistenceWarning.contains(QStringLiteral("startup setting could not be saved")));
    QVERIFY(result.persistence);
    QCOMPARE(result.persistence->failures.size(), std::size_t{1});
  }
  QCOMPARE(service->removals, 0);
  QVERIFY(fixture.state->registryDeletions.empty());
}

void EnhancedModeOperationsTests::enhancedModeServiceContractRejectsEveryBrokenInvariant() {
  const auto complete = completeEnhancedModeStatus();
  QVERIFY(complete.serviceContractSatisfied());

  constexpr std::array requirements{
      &app::EnhancedModeStatus::serviceExists,
      &app::EnhancedModeStatus::ownProcess,
      &app::EnhancedModeStatus::automaticStart,
      &app::EnhancedModeStatus::localSystemAccount,
      &app::EnhancedModeStatus::running,
      &app::EnhancedModeStatus::executableMatches,
      &app::EnhancedModeStatus::preshutdownTimeoutConfigured,
      &app::EnhancedModeStatus::requiredPrivilegesConfigured,
      &app::EnhancedModeStatus::preshutdownAccepted,
  };
  for (const auto requirement : requirements) {
    auto broken = complete;
    broken.*requirement = false;
    QVERIFY(!broken.serviceContractSatisfied());
  }
}

void EnhancedModeOperationsTests::enhancedModeRemovalPreflightsProtectedRegistryPersistence() {
  UwfMaintenanceFixture unavailableUwf;
  const auto unavailableRepository = unavailableUwf.repository();
  unavailableUwf.backend->inject({.request = compat::GetBoolean{compat::BooleanProperty::RegistryChangesAlwaysPersisted},
                                  .operation = {},
                                  .kind = compat::ErrorKind::Unavailable,
                                  .message = "Registry persistence state is unavailable"});
  // Admission consumes the published facts. A missing system identity blocks
  // removal without issuing the obsolete derived getter above.
  auto unavailableState = unavailableUwf.backend->snapshot();
  unavailableState.runtime.systemVolumeId.reset();
  unavailableUwf.backend->seed(std::move(unavailableState));
  auto protectedServiceOwner = std::make_shared<MemoryEnhancedModeServiceControl>();
  auto& protectedService = *protectedServiceOwner;
  protectedService.current = completeEnhancedModeStatus();
  app::EnhancedModeOperations unavailableOperations(protectedServiceOwner, unavailableRepository, app::UwfCapability::Available);

  QVERIFY_THROWS_EXCEPTION(std::runtime_error, static_cast<void>(unavailableOperations.remove()));
  QCOMPARE(protectedService.removals, 0);
  QCOMPARE(protectedService.current.state, app::EnhancedModeState::Enabled);
  QVERIFY(unavailableUwf.backend->trace().empty());

  UwfMaintenanceFixture availableUwf;
  auto removableServiceOwner = std::make_shared<MemoryEnhancedModeServiceControl>();
  auto& removableService = *removableServiceOwner;
  removableService.current = protectedService.current;
  // The prepared deletion plan must execute while the service deletion barrier is held.
  removableService.deletionPlan = {app::RegistryCommitTarget::wholeKey("HKEY_LOCAL_MACHINE\\SOFTWARE\\UwfEnhancedFixture")};
  availableUwf.backend->seedRegistryKey(removableService.deletionPlan.front().key);
  bool deletionCommittedInsideBarrier = false;
  availableUwf.state->beforeRegistryCommit = [&] { deletionCommittedInsideBarrier = removableService.activeDeletionBarriers == 1; };
  app::EnhancedModeOperations availableOperations(removableServiceOwner, availableUwf.repository(), app::UwfCapability::Available);
  const auto result = availableOperations.remove();
  QCOMPARE(removableService.removals, 1);
  QCOMPARE(removableService.activeDeletionBarriers, 0);
  QVERIFY(deletionCommittedInsideBarrier);
  QCOMPARE(availableUwf.state->registryDeletions.size(), std::size_t{1});
  QCOMPARE(availableUwf.state->registryTargets, 1);
  const auto trace = availableUwf.backend->trace();
  QCOMPARE(std::ranges::count_if(trace, [](const auto& entry) { return std::holds_alternative<compat::CommitRegistryDeletion>(entry.request); }), 1);
  QVERIFY(std::ranges::all_of(trace, [](const auto& entry) { return compat::isWrite(entry.request); }));
  QCOMPARE(result.status.state, app::EnhancedModeState::Disabled);
  QVERIFY(result.persistenceWarning.isEmpty());
}

void EnhancedModeOperationsTests::removalPersistsOnlyAfterTheOwnerDeletesRegistryArtifacts_data() {
  using RemovalState = app::EnhancedModeServiceControl::RemovalState;
  QTest::addColumn<bool>("filterEnabled");
  QTest::addColumn<RemovalState>("removalState");
  QTest::addColumn<RemovalCoverage>("coverage");
  QTest::addColumn<RemovalFailure>("failure");
  for (const bool filterEnabled : {false, true})
    for (const auto& [removalName, removal] : {std::pair{"absent", RemovalState::Absent}, std::pair{"marked", RemovalState::MarkedForDeletion},
                                               std::pair{"already-marked", RemovalState::AlreadyMarkedForDeletion}})
      for (const auto& [coverageName, coverage] : {std::pair{"not-excluded", RemovalCoverage::None}, std::pair{"driver-bypass", RemovalCoverage::DriverBypass},
                                                   std::pair{"management-only", RemovalCoverage::ManagementOnly}})
        for (const auto& [failureName, failure] :
             {std::pair{"no-fault", RemovalFailure::None}, std::pair{"cleanup-failed", RemovalFailure::Cleanup},
              std::pair{"target-unavailable", RemovalFailure::PrepareTarget}, std::pair{"before-commit", RemovalFailure::BeforeCommit},
              std::pair{"after-visible-deletion", RemovalFailure::AfterVisibleDeletion}, std::pair{"after-commit", RemovalFailure::AfterCommit}}) {
          if (failure == RemovalFailure::Cleanup && removal != RemovalState::Absent) continue;
          if (failure == RemovalFailure::PrepareTarget && !filterEnabled) continue;
          if ((failure == RemovalFailure::BeforeCommit || failure == RemovalFailure::AfterVisibleDeletion || failure == RemovalFailure::AfterCommit) &&
              (!filterEnabled || coverage != RemovalCoverage::None))
            continue;
          if ((failure == RemovalFailure::AfterVisibleDeletion || failure == RemovalFailure::AfterCommit) && removal == RemovalState::Absent) continue;
          const auto name = std::string(filterEnabled ? "uwf-on/" : "uwf-off/") + removalName + '/' + coverageName + '/' + failureName;
          QTest::newRow(name.c_str()) << filterEnabled << removal << coverage << failure;
        }
}

void EnhancedModeOperationsTests::removalPersistsOnlyAfterTheOwnerDeletesRegistryArtifacts() {
  using RemovalState = app::EnhancedModeServiceControl::RemovalState;
  using Fake = test::FakeUwfBackend;
  QFETCH(bool, filterEnabled);
  QFETCH(RemovalState, removalState);
  QFETCH(RemovalCoverage, coverage);
  QFETCH(RemovalFailure, failure);
  const std::string key = "HKEY_LOCAL_MACHINE\\SOFTWARE\\UwfEnhancedRemovalFixture\\Service";
  const std::string sibling = "HKEY_LOCAL_MACHINE\\SOFTWARE\\UwfEnhancedRemovalFixture\\ServiceSibling";
  const std::string value = "Artifact";
  const std::string detail = "Enhanced removal fixture failure";
  UwfMaintenanceFixture fixture;
  fixture.state->filterEnabled = filterEnabled;
  const auto repository = fixture.repository();
  auto configuration = fixture.backend->snapshot();
  if (coverage != RemovalCoverage::None) configuration.current.registryExclusions = {coverage == RemovalCoverage::DriverBypass ? key : key + '\\'};
  // The future list must not be used to infer the running driver's behavior.
  configuration.next.registryExclusions = coverage == RemovalCoverage::DriverBypass ? std::vector<std::string>{} : std::vector<std::string>{key};
  fixture.backend->seed(std::move(configuration));
  fixture.backend->seedRegistryValue(key, value, "service artifact");
  fixture.backend->seedRegistryValue(sibling, value, "unrelated artifact");
  const auto nextExclusions = fixture.backend->snapshot().next.registryExclusions;
  auto control = std::make_shared<MemoryEnhancedModeServiceControl>();
  control->current = completeEnhancedModeStatus();
  control->removalState = removalState;
  control->deletionPlan = {app::RegistryCommitTarget::wholeKey(key)};
  int cleanupCalls = 0;
  int barrierReleases = 0;
  bool cleanupPrecededPersistence = true;
  bool commitUsedTheExpectedBarrier = true;
  control->beforeDeleteRemnants = [&] {
    ++cleanupCalls;
    if (failure == RemovalFailure::Cleanup) throw std::runtime_error(detail);
    fixture.backend->deleteRegistryKey(key);
  };
  control->afterDeletionBarrier = [&] {
    ++barrierReleases;
    // SCM performs its ordinary deletion only when the last owned service
    // handle closes. An absent service has no such event or implicit cleanup.
    fixture.backend->deleteRegistryKey(key);
  };
  control->beforeQuery = [&] {
    control->current.serviceRegistryPresent = fixture.backend->visibleRegistryKey(key);
    control->current.state = control->current.serviceRegistryPresent ? app::EnhancedModeState::RepairRequired : app::EnhancedModeState::Disabled;
  };
  fixture.state->beforeRegistryTarget = [&] {
    if (removalState == RemovalState::Absent) cleanupPrecededPersistence = cleanupCalls == 1;
  };
  fixture.state->beforeRegistryCommit = [&] {
    commitUsedTheExpectedBarrier = control->activeDeletionBarriers == (removalState == RemovalState::MarkedForDeletion ? 1 : 0);
  };
  if (failure == RemovalFailure::PrepareTarget) fixture.state->registryTargetFailure = std::make_exception_ptr(std::runtime_error(detail));
  if (failure == RemovalFailure::BeforeCommit || failure == RemovalFailure::AfterVisibleDeletion || failure == RemovalFailure::AfterCommit) {
    const auto timing = failure == RemovalFailure::BeforeCommit           ? Fake::FaultTiming::Before
                        : failure == RemovalFailure::AfterVisibleDeletion ? Fake::FaultTiming::AfterVisibleDeletion
                                                                          : Fake::FaultTiming::AfterEffect;
    fixture.backend->inject({.request = compat::CommitRegistryDeletion{key, std::nullopt}, .operation = {}, .timing = timing, .message = detail});
  }
  app::EnhancedModeOperations operations(control, repository, app::UwfCapability::Available);
  const auto result = operations.remove();
  QCOMPARE(control->removals, 1);
  QCOMPARE(cleanupCalls, removalState == RemovalState::Absent ? 1 : 0);
  QCOMPARE(barrierReleases, removalState == RemovalState::MarkedForDeletion ? 1 : 0);
  QCOMPARE(control->activeDeletionBarriers, 0);
  QVERIFY(!control->queriedDuringDeletion);
  QVERIFY(cleanupPrecededPersistence);
  QVERIFY(commitUsedTheExpectedBarrier);
  const bool targetPrepared = filterEnabled && failure != RemovalFailure::Cleanup;
  const bool commitDispatched = targetPrepared && failure != RemovalFailure::PrepareTarget && coverage == RemovalCoverage::None;
  QCOMPARE(fixture.state->registryTargets, targetPrepared ? 1 : 0);
  QCOMPARE(fixture.state->registryDeletions.size(), commitDispatched ? std::size_t{1} : std::size_t{0});
  const bool warning =
      failure != RemovalFailure::None ||
      (filterEnabled && (coverage == RemovalCoverage::ManagementOnly || (coverage == RemovalCoverage::None && removalState == RemovalState::Absent)));
  QCOMPARE(result.persistenceWarning.isEmpty(), !warning);
  if (failure != RemovalFailure::None) QVERIFY(result.persistenceWarning.contains(QString::fromStdString(detail)));
  auto errors = recentLogLines();
  std::erase_if(errors, [](const auto& line) { return line.find(" E ") == std::string::npos; });
  QCOMPARE(errors.size(), warning ? std::size_t{1} : std::size_t{0});
  if (failure != RemovalFailure::None) QVERIFY(errors.front().find(detail) != std::string::npos);
  const bool commitDeletedArtifact =
      commitDispatched && removalState != RemovalState::Absent && (failure == RemovalFailure::None || failure == RemovalFailure::AfterCommit);
  const bool ownerDeletedArtifact = failure != RemovalFailure::Cleanup && removalState != RemovalState::AlreadyMarkedForDeletion;
  const bool durableDeletion = commitDeletedArtifact || (ownerDeletedArtifact && (!filterEnabled || coverage == RemovalCoverage::DriverBypass));
  const bool visibleDeletion = ownerDeletedArtifact || (commitDispatched && removalState != RemovalState::Absent && failure != RemovalFailure::BeforeCommit);
  QCOMPARE(fixture.backend->visibleRegistryKey(key), !visibleDeletion);
  QCOMPARE(result.status.serviceRegistryPresent, !visibleDeletion);
  QCOMPARE(result.status.state, visibleDeletion ? app::EnhancedModeState::Disabled : app::EnhancedModeState::RepairRequired);
  QCOMPARE(fixture.backend->readRegistryValue(key, value).has_value(), !durableDeletion);
  QCOMPARE(fixture.backend->snapshot().next.registryExclusions, nextExclusions);
  const auto trace = fixture.backend->trace();
  QCOMPARE(std::ranges::count_if(trace, [](const auto& entry) { return std::holds_alternative<compat::CommitRegistryDeletion>(entry.request); }),
           commitDispatched ? 1 : 0);
  for (const auto& entry : trace) {
    if (compat::isWrite(entry.request)) QVERIFY(std::holds_alternative<compat::CommitRegistryDeletion>(entry.request));
    if (!std::holds_alternative<compat::CommitRegistryDeletion>(entry.request)) continue;
    const bool completed = commitDeletedArtifact && failure != RemovalFailure::AfterCommit;
    QCOMPARE(entry.response.succeeded(), completed);
    // Only a pre-invocation fault proves no effect; an opaque setter failure remains uncertain.
    const auto expectedEffect = completed                                 ? compat::Effect::Confirmed
                                : failure == RemovalFailure::BeforeCommit ? compat::Effect::None
                                                                          : compat::Effect::Uncertain;
    QCOMPARE(entry.response.effect, expectedEffect);
  }
  fixture.backend->simulateRestart();
  QCOMPARE(fixture.backend->visibleRegistryKey(key), !durableDeletion);
  QCOMPARE(fixture.backend->visibleRegistryValue(key, value).has_value(), !durableDeletion);
  QCOMPARE(fixture.backend->visibleRegistryValue(sibling, value), std::optional<std::string>{"unrelated artifact"});
}

void EnhancedModeOperationsTests::persistenceAdmissionPrecedesServiceChanges_data() {
  QTest::addColumn<bool>("rom");
  QTest::addColumn<bool>("horm");
  QTest::addColumn<bool>("fullCommit");
  QTest::addColumn<app::EnhancedModeAction>("action");
  QTest::addColumn<bool>("systemProtected");
  struct Mode {
    bool rom = false;
    bool horm = false;
    bool fullCommit = false;
  };
  for (const auto& [name, state] : {std::pair{"ordinary", Mode{}}, std::pair{"rom", Mode{.rom = true}}, std::pair{"horm", Mode{.rom = true, .horm = true}},
                                    std::pair{"full-commit", Mode{.rom = true, .fullCommit = true}}})
    for (const auto& [actionName, action] : {std::pair{"enable", app::EnhancedModeAction::Enable}, std::pair{"disable", app::EnhancedModeAction::Disable},
                                             std::pair{"remove", app::EnhancedModeAction::Remove}})
      for (const bool systemProtected : {false, true})
        QTest::newRow((std::string(name) + '/' + actionName + (systemProtected ? "/system-protected" : "/system-unprotected")).c_str())
            << state.rom << state.horm << state.fullCommit << action << systemProtected;
}

void EnhancedModeOperationsTests::persistenceAdmissionPrecedesServiceChanges() {
  QFETCH(bool, rom);
  QFETCH(bool, horm);
  QFETCH(bool, fullCommit);
  QFETCH(app::EnhancedModeAction, action);
  QFETCH(bool, systemProtected);
  UwfMaintenanceFixture fixture;
  fixture.state->protectedVolume = systemProtected;
  const auto repository = fixture.repository();
  auto state = fixture.backend->snapshot();
  constexpr auto dataVolume = "99999999-2222-3333-4444-555555555555";
  test::FakeUwfBackend::VolumeConfiguration data;
  data.volume = {dataVolume, "D:", true, false, false};
  state.current.volumes.emplace(dataVolume, std::move(data));
  state.next = state.current;
  if (rom) state.current.overlayFlags = compat::flags::readOnlyMedia;
  state.runtime.hormEnabled = horm;
  state.runtime.romDisposition = fullCommit ? compat::disposition::fullCommit : 0;
  fixture.backend->seed(std::move(state));
  auto service = std::make_shared<MemoryEnhancedModeServiceControl>();
  service->current = completeEnhancedModeStatus();
  const std::string key = "HKEY_LOCAL_MACHINE\\SOFTWARE\\UwfEnhancedAdmissionFixture";
  const std::string value = "Configuration";
  service->installationPlan = {{key, value}};
  service->deletionPlan = {app::RegistryCommitTarget::wholeKey(key)};
  service->beforeInstall = [&] { fixture.backend->writeRegistryValue(key, value, "installed"); };
  service->beforeDisable = [&] { fixture.backend->writeRegistryValue(key, value, "disabled"); };
  service->afterDeletionBarrier = [&] { fixture.backend->deleteRegistryKey(key); };
  const bool enable = action == app::EnhancedModeAction::Enable;
  const bool remove = action == app::EnhancedModeAction::Remove;
  if (!enable) fixture.backend->seedRegistryValue(key, value, "installed");
  const auto before = fixture.backend->snapshot();
  app::EnhancedModeOperations operations(service, repository, app::UwfCapability::Available);
  const auto change = [&] {
    if (enable) return operations.enable(QStringLiteral("admission fixture"));
    return remove ? operations.remove() : operations.disable();
  };
  const bool blocked = systemProtected && (horm || (rom && !fullCommit));
  if (blocked) {
    QVERIFY_THROWS_EXCEPTION(app::OperationError, static_cast<void>(change()));
    QCOMPARE(service->installs, 0);
    QCOMPARE(service->disables, 0);
    QCOMPARE(service->removals, 0);
    QVERIFY(service->current.serviceContractSatisfied());
    QCOMPARE(fixture.backend->snapshot(), before);
    for (const auto& entry : fixture.backend->trace()) QVERIFY(!compat::isWrite(entry.request));
  } else {
    const auto result = change();
    QVERIFY(result.persistenceWarning.isEmpty());
    QCOMPARE(service->installs, enable ? 1 : 0);
    QCOMPARE(service->disables, !enable && !remove ? 1 : 0);
    QCOMPARE(service->removals, remove ? 1 : 0);
    QCOMPARE(result.status.state, enable ? app::EnhancedModeState::Enabled : app::EnhancedModeState::Disabled);
    QCOMPARE(fixture.backend->readRegistryValue(key, value).has_value(), !remove);
    const auto trace = fixture.backend->trace();
    if (!systemProtected || fullCommit) {
      QVERIFY(fixture.state->registryCommits.empty());
      QVERIFY(fixture.state->registryDeletions.empty());
      QVERIFY(std::ranges::none_of(trace, [](const auto& entry) {
        return compat::isWrite(entry.request) || std::holds_alternative<compat::GetRegistryExclusions>(entry.request) ||
               (std::get_if<compat::GetBoolean>(&entry.request) &&
                std::get<compat::GetBoolean>(entry.request).property == compat::BooleanProperty::HormEnabled) ||
               std::holds_alternative<compat::GetNumber>(entry.request);
      }));
    }
    fixture.backend->simulateRestart();
    QCOMPARE(fixture.backend->visibleRegistryValue(key, value).has_value(), !remove);
  }
  auto errors = recentLogLines();
  std::erase_if(errors, [](const auto& line) { return line.find(" E ") == std::string::npos; });
  QCOMPARE(errors.size(), blocked ? std::size_t{1} : std::size_t{0});
  const auto trace = fixture.backend->trace();
  QVERIFY(std::ranges::all_of(trace, [](const auto& entry) { return compat::isWrite(entry.request); }));
}

void EnhancedModeOperationsTests::removalRechecksNewWritesAfterReadingItsDeletionPlan() {
  UwfMaintenanceFixture fixture;
  const auto repository = fixture.repository();
  const std::string key = "HKEY_LOCAL_MACHINE\\SOFTWARE\\UwfEnhancedRemovalTransition";
  const std::string value = "Configuration";
  fixture.backend->seedRegistryValue(key, value, "installed");
  auto service = std::make_shared<MemoryEnhancedModeServiceControl>();
  service->current = completeEnhancedModeStatus();
  service->deletionPlan = {app::RegistryCommitTarget::wholeKey(key)};
  service->beforeDeletionPlan = [&] {
    auto state = fixture.backend->snapshot();
    state.current.overlayFlags = compat::flags::readOnlyMedia;
    state.runtime.romDisposition = compat::disposition::fullCommit;
    fixture.backend->seed(std::move(state));
  };
  service->afterDeletionBarrier = [&] { fixture.backend->deleteRegistryKey(key); };
  app::EnhancedModeOperations operations(service, repository, app::UwfCapability::Available);
  const auto result = operations.remove();
  QCOMPARE(service->removals, 1);
  QVERIFY(result.persistenceWarning.isEmpty());
  QVERIFY(!result.persistence);
  QVERIFY(fixture.state->registryCommits.empty());
  QVERIFY(fixture.state->registryDeletions.empty());
  QVERIFY(fixture.backend->trace().empty());
  fixture.backend->simulateRestart();
  QVERIFY(!fixture.backend->visibleRegistryKey(key));
}

void EnhancedModeOperationsTests::persistenceFailuresKeepOneOriginalBatchLog_data() {
  QTest::addColumn<int>("source");
  QTest::addColumn<bool>("unknown");
  const QStringList sources{QStringLiteral("installation-admission"), QStringLiteral("installation-plan"), QStringLiteral("installation-registry-target"),
                            QStringLiteral("removal-admission"),      QStringLiteral("removal-plan"),      QStringLiteral("removal-registry-target"),
                            QStringLiteral("remnant-cleanup")};
  for (int source = 0; source < sources.size(); ++source)
    for (const bool unknown : {false, true}) {
      const auto name = sources[source] + (unknown ? QStringLiteral("-unknown") : QStringLiteral("-multiline-utf8"));
      QTest::newRow(qUtf8Printable(name)) << source << unknown;
    }
}

void EnhancedModeOperationsTests::persistenceFailuresKeepOneOriginalBatchLog() {
  QFETCH(int, source);
  QFETCH(bool, unknown);
  const auto detail = QStringLiteral("增强模式持久化失败\n保留原始 provider/计划/清理错误的第二行");
  const auto failure = unknown ? std::make_exception_ptr(42) : std::make_exception_ptr(std::runtime_error(detail.toStdString()));
  UwfMaintenanceFixture uwf;
  const auto repository = uwf.repository();
  // Admission is an in-memory repository port. Its failures retain their
  // original evidence without triggering a provider observation.
  const bool admissionFailure = source == 0 || source == 3;
  auto service = std::make_shared<MemoryEnhancedModeServiceControl>();
  service->current = completeEnhancedModeStatus();
  service->installationPlan = {{"HKEY_LOCAL_MACHINE\\SOFTWARE\\UwfEnhancedFixture", "value"}};
  service->deletionPlan = {app::RegistryCommitTarget::wholeKey("HKEY_LOCAL_MACHINE\\SOFTWARE\\UwfEnhancedFixture")};
  if (admissionFailure) uwf.state->filterFailure = failure;
  if (source == 1) service->beforeCommitPlan = [failure] { std::rethrow_exception(failure); };
  if (source == 2) service->beforeCommitPlan = [&] { uwf.state->registryTargetFailure = failure; };
  if (source == 4) service->beforeDeletionPlan = [failure] { std::rethrow_exception(failure); };
  if (source == 5) service->beforeDeletionPlan = [&] { uwf.state->registryTargetFailure = failure; };
  if (source == 6) {
    service->removalState = app::EnhancedModeServiceControl::RemovalState::Absent;
    service->beforeDeleteRemnants = [failure] { std::rethrow_exception(failure); };
  }
  app::EnhancedModeOperations operations(service, repository, source == 6 ? app::UwfCapability::Unavailable : app::UwfCapability::Available);
  const bool preflight = source == 0 || source == 3 || source == 4;
  std::optional<app::EnhancedModeChangeResult> result;
  int caught = 0;
  try {
    result = source <= 2 ? operations.enable(QStringLiteral("logging fixture")) : operations.remove();
  } catch (const std::exception& error) {
    QVERIFY(preflight);
    QVERIFY(!unknown);
    QCOMPARE(QString::fromUtf8(error.what()), detail);
    ++caught;
  } catch (int marker) {
    QVERIFY(preflight);
    QVERIFY(unknown);
    QCOMPARE(marker, 42);
    ++caught;
  }
  QCOMPARE(caught, preflight ? 1 : 0);
  QCOMPARE(service->installs, source > 0 && source <= 2 ? 1 : 0);
  QCOMPARE(service->removals, source <= 2 || preflight ? 0 : 1);
  QCOMPARE(service->activeDeletionBarriers, 0);
  QVERIFY(uwf.state->registryCommits.empty());
  QVERIFY(uwf.state->registryDeletions.empty());
  if (result) {
    QVERIFY(!result->persistenceWarning.isEmpty());
    QVERIFY(result->persistenceFailure);
    QVERIFY(!result->persistence);
    QCOMPARE(result->persistenceFailure->kind, app::OperationFailureKind::Failed);
    QCOMPARE(result->persistenceFailure->effect, app::WriteEffect::Uncertain);
    if (!unknown) QCOMPARE(result->persistenceWarning, detail);
    const auto copied = *result;
    QCOMPARE(copied.persistenceWarning, result->persistenceWarning);
  }
  auto errors = recentLogLines();
  std::erase_if(errors, [](const auto& line) { return line.find(" E ") == std::string::npos; });
  QCOMPARE(errors.size(), std::size_t{1});
  const auto expected = unknown ? "error=non-standard-exception" : detail.toStdString();
  QVERIFY(errors.front().find(expected) != std::string::npos);
  QVERIFY(errors.front().find(source == 2 || source == 5 ? "registry commit batch read failed:" : " E enhanced]") != std::string::npos);
  QVERIFY(errors.front().find("item failed:") == std::string::npos);
  const auto trace = uwf.backend->trace();
  QVERIFY(std::ranges::all_of(trace, [](const auto& entry) { return compat::isWrite(entry.request); }));
}

void EnhancedModeOperationsTests::notFoundPersistenceIsNotSilentlyAccepted_data() {
  QTest::addColumn<int>("effect");
  for (const auto& [name, effect] :
       {std::pair{"none", app::WriteEffect::None}, std::pair{"confirmed", app::WriteEffect::Confirmed}, std::pair{"uncertain", app::WriteEffect::Uncertain}})
    QTest::newRow(name) << static_cast<int>(effect);
}
void EnhancedModeOperationsTests::notFoundPersistenceIsNotSilentlyAccepted() {
  QFETCH(int, effect);
  UwfMaintenanceFixture fixture;
  fixture.state->registryCommitFailure = std::make_exception_ptr(app::OperationError(
      std::make_error_code(std::errc::no_such_file_or_directory), "Registry persistence target was not found", static_cast<app::WriteEffect>(effect)));
  auto service = std::make_shared<MemoryEnhancedModeServiceControl>();
  service->current = completeEnhancedModeStatus();
  service->deletionPlan = {app::RegistryCommitTarget::wholeKey("HKEY_LOCAL_MACHINE\\SOFTWARE\\UwfEnhancedFixture")};
  app::EnhancedModeOperations operations(service, fixture.repository(), app::UwfCapability::Available);
  const auto result = operations.remove();
  QVERIFY(!result.persistenceWarning.isEmpty());
  QVERIFY(result.persistenceWarning.contains(QStringLiteral("Registry persistence target was not found")));
  QVERIFY(result.persistence);
  QVERIFY(!result.persistenceFailure);
  QCOMPARE(result.persistence->failures.size(), std::size_t{1});
  QVERIFY(result.persistence->failures.front().operation);
  QCOMPARE(result.persistence->failures.front().operation->kind, app::OperationFailureKind::NotFound);
  QCOMPARE(result.persistence->failures.front().operation->effect, static_cast<app::WriteEffect>(effect));
  QCOMPARE(result.persistence->writeEffect(), static_cast<app::WriteEffect>(effect));
  QCOMPARE(fixture.state->registryDeletions.size(), std::size_t{1});
  auto errors = recentLogLines();
  std::erase_if(errors, [](const auto& line) { return line.find(" E ") == std::string::npos; });
  QCOMPARE(errors.size(), std::size_t{1});
  QVERIFY(errors.front().find("registry commit item failed:") != std::string::npos);
}

void EnhancedModeOperationsTests::itemPersistenceWarningsDoNotRepeatRegistryFailureLogs() {
  UwfMaintenanceFixture uwf;
  auto service = std::make_shared<MemoryEnhancedModeServiceControl>();
  for (int index = 0; index < 100; ++index)
    service->installationPlan.push_back({"HKEY_LOCAL_MACHINE\\SOFTWARE\\UwfEnhancedFixture", "value-" + std::to_string(index)});
  const auto detail = QStringLiteral("逐目标持久化失败\n完整原始详情");
  uwf.state->registryCommitFailure = std::make_exception_ptr(std::runtime_error(detail.toStdString()));
  app::EnhancedModeOperations operations(service, uwf.repository(), app::UwfCapability::Available);
  const auto result = operations.enable(QStringLiteral("logging fixture"));
  QVERIFY(result.persistenceWarning.contains(QStringLiteral("100")));
  QVERIFY(result.persistence);
  QVERIFY(!result.persistenceFailure);
  QCOMPARE(result.persistence->failures.size(), std::size_t{100});
  QCOMPARE(result.persistence->writeEffect(), app::WriteEffect::Uncertain);
  for (const auto& failure : result.persistence->failures) {
    QVERIFY(failure.operation);
    QCOMPARE(failure.operation->kind, app::OperationFailureKind::Failed);
    QCOMPARE(failure.operation->effect, app::WriteEffect::Uncertain);
  }
  QCOMPARE(uwf.state->registryCommits.size(), std::size_t{100});
  auto errors = recentLogLines();
  std::erase_if(errors, [](const auto& line) { return line.find(" E ") == std::string::npos; });
  QCOMPARE(errors.size(), std::size_t{100});
  for (const auto& line : errors) {
    QVERIFY(line.find("registry commit item failed:") != std::string::npos);
    QVERIFY(line.find(detail.toStdString()) != std::string::npos);
  }
}
}  // namespace

QTEST_GUILESS_MAIN(EnhancedModeOperationsTests)
#include "EnhancedModeOperationsTests.moc"
