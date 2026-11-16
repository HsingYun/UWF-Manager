// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QTest>
#include <algorithm>
#include <array>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "application/registry/RegistryBatchCommitter.h"
#include "application/uwf/UwfRequestPrecheck.h"
#include "application/uwf/adapters/UwfMaintenanceRepositoryFactory.h"
#include "foundation/logging/Log.h"
#include "support/app/OperationFailureFixture.h"
#include "support/app/UwfMaintenanceFixture.h"
#include "support/uwf/FakeUwfBackend.h"

namespace {
using namespace uwf;

using Fixture = test::application::UwfMaintenanceFixture;

test::FakeUwfBackend::State protectedRegistryState() {
  test::FakeUwfBackend::State state;
  state.current.filterEnabled = true;
  state.runtime.systemVolumeId = Fixture::volumeId;
  test::FakeUwfBackend::VolumeConfiguration volume;
  volume.volume = {Fixture::volumeId, "C:", true, false, false};
  state.current.volumes.emplace(Fixture::volumeId, std::move(volume));
  return state;
}

std::vector<std::string> failureLogs() {
  auto lines = recentLogLines();
  std::erase_if(lines, [](const auto& line) { return line.find(" E registry] registry commit item failed:") == std::string::npos; });
  return lines;
}

class RegistryBatchCommitterTests final : public QObject {
  Q_OBJECT
  enum class ControlSetObservation { Missing, Failing, Available };
  Q_ENUM(ControlSetObservation)
  enum class MaintenanceOperation { CommitValue, PersistValueDeletion, PersistKeyDeletion };
  Q_ENUM(MaintenanceOperation)
 private slots:
  void registryPlanTextIsValidatedBeforeEveryPersistencePath_data() {
    QTest::addColumn<QByteArray>("malformed");
    QTest::addColumn<bool>("keyField");
    QTest::addColumn<bool>("deletion");
    QTest::addColumn<int>("admission");
    constexpr std::array paths{"needs-commit", "driver-exclusion", "already-persistent"};
    for (const auto* hex : {"00", "80", "c0af", "e4b8", "eda080", "f4908080", "ff"})
      for (const bool key : {false, true})
        for (const bool deletion : {false, true})
          for (std::size_t admission = 0; admission < paths.size(); ++admission) {
            const auto name = QStringLiteral("%1/%2/%3/%4")
                                  .arg(QString::fromLatin1(hex), key ? QStringLiteral("key") : QStringLiteral("value"),
                                       deletion ? QStringLiteral("deletion") : QStringLiteral("commit"), QString::fromLatin1(paths[admission]));
            QTest::newRow(qUtf8Printable(name)) << QByteArray::fromHex(hex) << key << deletion << static_cast<int>(admission);
          }
  }
  void registryPlanTextIsValidatedBeforeEveryPersistencePath() {
    QFETCH(QByteArray, malformed);
    QFETCH(bool, keyField);
    QFETCH(bool, deletion);
    QFETCH(int, admission);
    auto initial = protectedRegistryState();
    const std::string key = "HKLM\\SOFTWARE\\StrictTransport";
    if (admission == 1) initial.current.registryExclusions = {key};
    if (admission == 2) initial.current.volumes.at(Fixture::volumeId).volume.protectedVolume = false;
    initial.next = initial.current;
    const auto backend = std::make_shared<test::FakeUwfBackend>(initial);

    const app::RegistryBatchCommitter committer(app::uwfMaintenanceRepository(backend));
    auto invalid = app::RegistryCommitTarget{key, "named"};
    auto& field = keyField ? invalid.key : invalid.valueName;
    field.append(malformed.constData(), static_cast<std::size_t>(malformed.size()));
    field += "suffix";
    const std::vector<app::RegistryCommitTarget> plan{{key, "valid-first"}, invalid};
    const auto before = backend->snapshot();
    backend->clearTrace();
    const auto result = deletion ? committer.persistDeletions(plan) : committer.commit(plan);
    QVERIFY(!result.succeeded());
    QCOMPARE(result.attempted, std::size_t{2});
    QCOMPARE(result.committed, std::size_t{0});
    QCOMPARE(result.failures.size(), std::size_t{1});
    QCOMPARE(result.failures.front().operation->kind, app::OperationFailureKind::InvalidArgument);
    QCOMPARE(result.writeEffect(), app::WriteEffect::None);
    QCOMPARE(result.failures.front().target.key, invalid.key);
    QCOMPARE(result.failures.front().target.valueName, invalid.valueName);
    QCOMPARE(backend->snapshot(), before);
    QVERIFY(backend->trace().empty());
  }

  void malformedRegistryPlansFailBeforeEveryItem_data() {
    QTest::addColumn<int>("scene");
    QTest::newRow("delete-default-value") << 0;
    QTest::newRow("delete-unknown-tag") << 1;
    QTest::newRow("delete-key-with-value") << 2;
    QTest::newRow("commit-whole-key") << 3;
    QTest::newRow("commit-unknown-tag") << 4;
  }
  void malformedRegistryPlansFailBeforeEveryItem() {
    QFETCH(int, scene);
    Fixture fixture;
    const std::string key = "HKLM\\SOFTWARE\\InvalidLateRegistryIntent";
    fixture.backend->seedRegistryValue(key, "valid-first", "before");
    fixture.backend->writeRegistryValue(key, "valid-first", "after");
    auto invalid = app::RegistryCommitTarget{key, "named", static_cast<app::RegistryTargetKind>(99)};
    if (scene == 0) invalid = {key, ""};
    if (scene == 2) invalid = {key, "named", app::RegistryTargetKind::Key};
    if (scene == 3) invalid = app::RegistryCommitTarget::wholeKey(key);
    const app::RegistryBatchCommitter committer(fixture.repository());
    const auto before = fixture.backend->snapshot();
    const std::vector<app::RegistryCommitTarget> plan{{key, "valid-first"}, invalid};
    const auto result = scene < 3 ? committer.persistDeletions(plan) : committer.commit(plan);
    QVERIFY(!result.succeeded());
    QCOMPARE(result.attempted, std::size_t{2});
    QCOMPARE(result.committed, std::size_t{0});
    QCOMPARE(result.failures.size(), std::size_t{1});
    QCOMPARE(result.failures.front().operation->kind, app::OperationFailureKind::InvalidArgument);
    QCOMPARE(result.writeEffect(), app::WriteEffect::None);
    QCOMPARE(fixture.backend->snapshot(), before);
    QCOMPARE(fixture.state->registryTargets, 0);
    QVERIFY(fixture.state->registryCommits.empty());
    QVERIFY(fixture.state->registryDeletions.empty());
  }
  void registryTargetIntentDoesNotInferWholeKeyFromEmptyValue() {
    Fixture fixture;
    const std::string key = "HKLM\\SOFTWARE\\ExplicitRegistryIntent";
    fixture.backend->seedRegistryValue(key, "", "default-before");
    fixture.backend->seedRegistryValue(key, "sibling", "untouched");
    fixture.backend->writeRegistryValue(key, "", "default-after");
    const app::RegistryCommitTarget defaultValue{key, ""};
    const auto wholeKey = app::RegistryCommitTarget::wholeKey(key);
    QVERIFY(!defaultValue.isKey());
    QVERIFY(wholeKey.isKey());
    const app::RegistryBatchCommitter committer(fixture.repository());
    QVERIFY(committer.commit({defaultValue}).succeeded());
    QCOMPARE(fixture.backend->readRegistryValue(key, ""), std::optional<std::string>{"default-after"});
    const auto before = fixture.backend->snapshot();
    const auto deletion = committer.persistDeletions({defaultValue});
    QVERIFY(!deletion.succeeded());
    QCOMPARE(deletion.writeEffect(), app::WriteEffect::None);
    QCOMPARE(fixture.backend->snapshot(), before);
    QVERIFY(fixture.state->registryDeletions.empty());
    const auto invalidCommit = committer.commit({wholeKey});
    QVERIFY(!invalidCommit.succeeded());
    QCOMPARE(invalidCommit.writeEffect(), app::WriteEffect::None);
    QCOMPARE(fixture.backend->snapshot(), before);
    fixture.backend->deleteRegistryKey(key);
    const auto missingCurrent = committer.persistDeletions({wholeKey});
    // The explicit selector reaches UWF as a whole-key request, but a prior
    // host deletion hides Current and cannot prove persistent deletion.
    QVERIFY(!missingCurrent.succeeded());
    QCOMPARE(missingCurrent.failures.size(), std::size_t{1});
    QVERIFY(missingCurrent.failures.front().operation);
    QCOMPARE(missingCurrent.failures.front().operation->kind, app::OperationFailureKind::NotFound);
    QCOMPARE(missingCurrent.writeEffect(), app::WriteEffect::Uncertain);
    QVERIFY(!fixture.backend->visibleRegistryKey(key));
    QCOMPARE(fixture.backend->readRegistryValue(key, ""), std::optional<std::string>{"default-after"});
    QCOMPARE(fixture.backend->readRegistryValue(key, "sibling"), std::optional<std::string>{"untouched"});
    QCOMPARE(fixture.state->registryDeletions.size(), std::size_t{1});
    QVERIFY(!fixture.state->registryDeletions.front().valueName);
  }
  void explicitWholeKeyTargetDeletesAVisibleCurrentKey() {
    Fixture fixture;
    const std::string key = "HKLM\\SOFTWARE\\VisibleWholeKeyIntent";
    fixture.backend->seedRegistryValue(key, "", "default");
    fixture.backend->seedRegistryValue(key, "sibling", "named");
    const auto repository = fixture.repository();
    const auto current = repository->prepareRegistryMaintenance();
    const auto wholeKey = app::RegistryCommitTarget::wholeKey(key);
    QVERIFY(fixture.backend->visibleRegistryKey(key));
    QCOMPARE(current->commitDeletion(wholeKey.key, wholeKey.deletionValueName()), app::WriteEffect::Confirmed);
    QVERIFY(!fixture.backend->visibleRegistryKey(key));
    QVERIFY(!fixture.backend->readRegistryValue(key, ""));
    QVERIFY(!fixture.backend->readRegistryValue(key, "sibling"));
    QCOMPARE(fixture.state->registryDeletions.size(), std::size_t{1});
    QCOMPARE(fixture.state->registryDeletions.front().key, key);
    QVERIFY(!fixture.state->registryDeletions.front().valueName);
  }
  void batchResultsPreserveTypedFailureEffectAndDiagnostic() {
    using namespace app;
    for (const bool deletion : {false, true})
      for (const auto kind :
           {OperationFailureKind::NotFound, OperationFailureKind::Unsupported, OperationFailureKind::AccessDenied, OperationFailureKind::Protocol})
        for (const auto effect : {WriteEffect::None, WriteEffect::Confirmed, WriteEffect::Uncertain}) {
          Fixture fixture;
          const std::vector<RegistryCommitTarget> targets{{"HKLM\\SOFTWARE\\First", "Value"}, {"HKLM\\SOFTWARE\\Second", "Value"}};
          fixture.stageRegistryValues(targets);
          int writes = 0;
          fixture.state->beforeRegistryCommit = [&] {
            if (++writes == 2) throw test::application::DetailedOperationFailure(kind, effect);
          };
          const RegistryBatchCommitter committer(fixture.repository());
          const auto result = deletion ? committer.persistDeletions(targets) : committer.commit(targets);
          QCOMPARE(result.committed, std::size_t{1});
          QCOMPARE(result.failures.size(), std::size_t{1});
          const auto& failure = result.failures.front();
          QCOMPARE(failure.target.key, targets.back().key);
          QVERIFY(failure.operation);
          QCOMPARE(failure.operation->kind, kind);
          QCOMPARE(failure.operation->effect, effect);
          QCOMPARE(failure.operation->diagnostic.code, int32_t{1234});
          QCOMPARE(failure.operation->diagnostic.result, uint32_t{0x80070005u});
          QVERIFY(failure.detail.find("Original provider failure") != std::string::npos);
          QCOMPARE(result.writeEffect(), effect == WriteEffect::Uncertain ? WriteEffect::Uncertain : WriteEffect::Confirmed);
          QCOMPARE(fixture.backend->readRegistryValue(targets.front().key, targets.front().valueName),
                   deletion ? std::nullopt : std::optional<std::string>{"fixture"});
        }
  }
  void init() { clearLogLines(); }
  void fullCommitDoesNotConfirmHistoricalRegistryPlans_data();
  void fullCommitDoesNotConfirmHistoricalRegistryPlans();
  void emptyPlansSkipUwfAndExternalDeletionsRetainTheirProviderFailure();
  void commitLogsEveryFailureBeforeTheVisibleSummaryLimit_data();
  void commitLogsEveryFailureBeforeTheVisibleSummaryLimit();
  void deletionLogsEveryFailureBeforeTheVisibleSummaryLimit_data();
  void deletionLogsEveryFailureBeforeTheVisibleSummaryLimit();
  void persistedExternalDeletionsAreNotLoggedButUnconfirmedBaseDeletionsAre();
  void requestPreparationFailuresRemainBatchFailures_data();
  void requestPreparationFailuresRemainBatchFailures();
  void missingCommitValuesAreNotCreated_data();
  void missingCommitValuesAreNotCreated();
  void currentExclusionCoveragePreservesRegistryDurability_data();
  void currentExclusionCoveragePreservesRegistryDurability();
  void controlSetReadFailuresCannotBecomeDurabilityProof_data();
  void controlSetReadFailuresCannotBecomeDurabilityProof();
  void controlSetNumericFormatGovernsEnvironmentReads_data();
  void controlSetNumericFormatGovernsEnvironmentReads();
  void independentExclusionProofDoesNotDependOnListOrderOrEnvironment_data();
  void independentExclusionProofDoesNotDependOnListOrderOrEnvironment();
  void unprotectedSystemVolumeNeedsNoRegistryPersistence_data();
  void unprotectedSystemVolumeNeedsNoRegistryPersistence();
};

void RegistryBatchCommitterTests::fullCommitDoesNotConfirmHistoricalRegistryPlans_data() {
  QTest::addColumn<MaintenanceOperation>("operation");
  QTest::newRow("commit-value") << MaintenanceOperation::CommitValue;
  QTest::newRow("persist-value-deletion") << MaintenanceOperation::PersistValueDeletion;
  QTest::newRow("persist-key-deletion") << MaintenanceOperation::PersistKeyDeletion;
}

void RegistryBatchCommitterTests::fullCommitDoesNotConfirmHistoricalRegistryPlans() {
  QFETCH(MaintenanceOperation, operation);
  auto initial = protectedRegistryState();
  initial.current.overlayFlags = compat::flags::readOnlyMedia;
  initial.next = initial.current;
  initial.runtime.fullCommitCopyFails = true;
  const auto backend = std::make_shared<test::FakeUwfBackend>(initial);
  const std::string key = "HKEY_LOCAL_MACHINE\\SOFTWARE\\HistoricalRegistryPlan";
  const std::string name = "Setting";
  backend->seedRegistryValue(key, name, "base");
  if (operation == MaintenanceOperation::CommitValue)
    backend->writeRegistryValue(key, name, "overlay");
  else if (operation == MaintenanceOperation::PersistValueDeletion)
    backend->deleteRegistryValue(key, name);
  else
    backend->deleteRegistryKey(key);
  QVERIFY(backend->execute(compat::SetBoolean{compat::BooleanProperty::OverlayCommit, true}).succeeded());
  QCOMPARE(backend->readRegistryValue(key, name), std::optional<std::string>{"base"});
  const auto repository = app::uwfMaintenanceRepository(backend);
  QCOMPARE(repository->registryWriteRequirement(), app::CommitRequirement::AlreadyPersistent);
  backend->writeRegistryValue(key, "Future", "new-direct-write");
  QCOMPARE(backend->readRegistryValue(key, "Future"), std::optional<std::string>{"new-direct-write"});
  const app::RegistryBatchCommitter committer(repository);
  const auto before = backend->snapshot();
  backend->clearTrace();
  const std::vector<app::RegistryCommitTarget> targets{operation == MaintenanceOperation::PersistKeyDeletion ? app::RegistryCommitTarget::wholeKey(key)
                                                                                                         : app::RegistryCommitTarget{key, name}};
  if (operation == MaintenanceOperation::CommitValue)
    QVERIFY_THROWS_EXCEPTION(app::OperationError, (void)committer.commit(targets));
  else
    QVERIFY_THROWS_EXCEPTION(app::OperationError, (void)committer.persistDeletions(targets));
  QCOMPARE(backend->snapshot(), before);
  QVERIFY(backend->trace().empty());
}

void RegistryBatchCommitterTests::unprotectedSystemVolumeNeedsNoRegistryPersistence_data() {
  QTest::addColumn<bool>("rom");
  QTest::addColumn<bool>("horm");
  QTest::addColumn<MaintenanceOperation>("operation");
  for (const auto& [mode, flags] :
       {std::pair{"ordinary", std::pair{false, false}}, std::pair{"rom", std::pair{true, false}}, std::pair{"horm", std::pair{true, true}}})
    for (const auto& [name, operation] :
         {std::pair{"commit", MaintenanceOperation::CommitValue}, std::pair{"value-deleted", MaintenanceOperation::PersistValueDeletion},
          std::pair{"key-deleted", MaintenanceOperation::PersistKeyDeletion}})
      QTest::newRow((std::string(mode) + '/' + name).c_str()) << flags.first << flags.second << operation;
}

void RegistryBatchCommitterTests::unprotectedSystemVolumeNeedsNoRegistryPersistence() {
  QFETCH(bool, rom);
  QFETCH(bool, horm);
  QFETCH(MaintenanceOperation, operation);
  using Fake = test::FakeUwfBackend;
  auto initial = protectedRegistryState();
  initial.current.volumes.at(Fixture::volumeId).volume.protectedVolume = false;
  constexpr auto dataVolume = "66666666-7777-8888-9999-aaaaaaaaaaaa";
  Fake::VolumeConfiguration data;
  data.volume = {dataVolume, "D:", true, false, false};
  initial.current.volumes.emplace(dataVolume, std::move(data));
  if (rom) initial.current.overlayFlags = compat::flags::readOnlyMedia;
  initial.runtime.hormEnabled = horm;
  initial.next = initial.current;
  const auto backend = std::make_shared<Fake>(initial);
  const std::string key = "HKLM\\SOFTWARE\\AlreadyDurable\\Entry";
  const std::string sibling = "HKLM\\SOFTWARE\\AlreadyDurable\\Sibling";
  const std::string name = "Named";
  backend->seedRegistryValue(key, name, "before");
  backend->seedRegistryValue(sibling, name, "untouched");
  switch (operation) {
    case MaintenanceOperation::CommitValue:
      backend->writeRegistryValue(key, name, "after");
      break;
    case MaintenanceOperation::PersistValueDeletion:
      backend->deleteRegistryValue(key, name);
      break;
    case MaintenanceOperation::PersistKeyDeletion:
      backend->deleteRegistryKey(key);
      break;
  }
  const auto before = backend->snapshot();
  backend->clearTrace();

  app::RegistryBatchCommitter committer(app::uwfMaintenanceRepository(backend));
  const std::vector<app::RegistryCommitTarget> targets{operation == MaintenanceOperation::PersistKeyDeletion ? app::RegistryCommitTarget::wholeKey(key)
                                                                                                             : app::RegistryCommitTarget{key, name}};
  const auto result = operation == MaintenanceOperation::CommitValue ? committer.commit(targets) : committer.persistDeletions(targets);
  QVERIFY(result.succeeded());
  QCOMPARE(result.attempted, std::size_t{1});
  QCOMPARE(result.committed, std::size_t{1});

  QCOMPARE(result.writeEffect(), app::WriteEffect::None);
  QCOMPARE(backend->snapshot(), before);
  const auto trace = backend->trace();
  QVERIFY(std::ranges::none_of(trace, [](const auto& entry) {
    return compat::isWrite(entry.request) || std::holds_alternative<compat::GetRegistryExclusions>(entry.request) ||
           (std::get_if<compat::GetBoolean>(&entry.request) && std::get<compat::GetBoolean>(entry.request).property == compat::BooleanProperty::HormEnabled);
  }));
  QVERIFY(failureLogs().empty());
  backend->simulateRestart();
  QCOMPARE(backend->visibleRegistryValue(key, name), operation == MaintenanceOperation::CommitValue ? std::optional<std::string>{"after"} : std::nullopt);
  if (operation == MaintenanceOperation::PersistKeyDeletion) QVERIFY(!backend->visibleRegistryKey(key));
  QCOMPARE(backend->visibleRegistryValue(sibling, name), std::optional<std::string>{"untouched"});
}

void RegistryBatchCommitterTests::independentExclusionProofDoesNotDependOnListOrderOrEnvironment_data() {
  QTest::addColumn<bool>("targetAlias");
  QTest::addColumn<bool>("knownProof");
  QTest::addColumn<bool>("reverse");
  QTest::addColumn<ControlSetObservation>("observation");
  QTest::addColumn<MaintenanceOperation>("operation");
  for (const bool targetAlias : {false, true})
    for (const bool knownProof : {false, true})
      for (const bool reverse : {false, true}) {
        if (!knownProof && reverse) continue;
        for (const auto& [observationName, fact] :
             {std::pair{"missing-fact", ControlSetObservation::Missing}, std::pair{"failed-fact", ControlSetObservation::Failing},
              std::pair{"available-fact", ControlSetObservation::Available}})
          for (const auto& [operationName, operation] :
               {std::pair{"commit-value", MaintenanceOperation::CommitValue}, std::pair{"persist-value-deletion", MaintenanceOperation::PersistValueDeletion},
                std::pair{"persist-key-deletion", MaintenanceOperation::PersistKeyDeletion}}) {
            const auto name = QStringLiteral("%1/%2/%3/%4/%5")
                                  .arg(targetAlias ? QStringLiteral("target-CSS") : QStringLiteral("target-concrete"),
                                       knownProof ? QStringLiteral("known-proof") : QStringLiteral("unresolved-proof"),
                                       reverse ? QStringLiteral("known-first") : QStringLiteral("pending-first"), QString::fromLatin1(observationName),
                                       QString::fromLatin1(operationName));
            QTest::newRow(qPrintable(name)) << targetAlias << knownProof << reverse << fact << operation;
          }
      }
}

void RegistryBatchCommitterTests::independentExclusionProofDoesNotDependOnListOrderOrEnvironment() {
  QFETCH(bool, targetAlias);
  QFETCH(bool, knownProof);
  QFETCH(bool, reverse);
  QFETCH(ControlSetObservation, observation);
  QFETCH(MaintenanceOperation, operation);
  using Fake = test::FakeUwfBackend;
  const std::string targetRoot = targetAlias ? "HKLM\\SYSTEM\\CurrentControlSet\\CoverageFixture" : "HKLM\\SYSTEM\\ControlSet001\\CoverageFixture";
  const std::string pending = targetAlias ? "HKLM\\SYSTEM\\ControlSet001\\CoverageFixture" : "HKLM\\SYSTEM\\CurrentControlSet\\CoverageFixture";
  const auto key = targetRoot + "\\Child";
  const std::string value = "Named";
  Fake::State initial;
  initial.current.filterEnabled = false;
  initial.runtime.currentControlSet = 1;
  Fake::VolumeConfiguration system;
  system.volume = {Fixture::volumeId, "C:", true, false, false};
  initial.current.volumes.emplace(Fixture::volumeId, std::move(system));
  initial.runtime.systemVolumeId = Fixture::volumeId;
  std::vector<std::string> patterns{pending};
  if (knownProof) patterns.push_back(targetRoot);
  if (reverse) std::reverse(patterns.begin(), patterns.end());
  initial.next = initial.current;
  const auto backend = std::make_shared<Fake>(std::move(initial));
  // Establish simultaneous CSS/concrete exclusions through public Add and restart operations.
  for (const auto& pattern : patterns) (void)compat::call(*backend, compat::EditRegistryExclusion{pattern, compat::ExclusionAction::Add});
  (void)compat::call(*backend, compat::SetBoolean{compat::BooleanProperty::FilterEnabled, true});
  backend->simulateRestart();
  (void)compat::call(*backend, compat::EditRegistryExclusion{{}, compat::ExclusionAction::Clear});
  QCOMPARE(backend->snapshot().current.registryExclusions, patterns);
  QVERIFY(backend->snapshot().next.registryExclusions.empty());
  backend->seedRegistryValue(key, value, "before");
  switch (operation) {
    case MaintenanceOperation::CommitValue:
      backend->writeRegistryValue(key, value, "after");
      break;
    case MaintenanceOperation::PersistValueDeletion:
      backend->deleteRegistryValue(key, value);
      break;
    case MaintenanceOperation::PersistKeyDeletion:
      backend->deleteRegistryKey(key);
      break;
  }
  // The simulated driver bypasses these writes; application admission must independently establish their durability.
  QCOMPARE(backend->readRegistryValue(key, value).has_value(), operation == MaintenanceOperation::CommitValue);

  const auto checkedSystem = std::make_shared<app::PrecheckedUwf>(backend);
  auto published = backend->state();
  if (observation != ControlSetObservation::Available) published.runtime.currentControlSet = {};
  checkedSystem->updateState(std::move(published));
  app::RegistryBatchCommitter committer(app::uwfMaintenanceRepository(checkedSystem));
  const auto before = backend->snapshot();
  backend->clearTrace();
  const std::vector<app::RegistryCommitTarget> targets{operation == MaintenanceOperation::PersistKeyDeletion ? app::RegistryCommitTarget::wholeKey(key)
                                                                                                             : app::RegistryCommitTarget{key, value}};
  const auto result = operation == MaintenanceOperation::CommitValue ? committer.commit(targets) : committer.persistDeletions(targets);
  const bool succeeded = knownProof || observation == ControlSetObservation::Available;
  QCOMPARE(result.succeeded(), succeeded);
  QCOMPARE(result.committed, succeeded ? std::size_t{1} : std::size_t{0});
  QCOMPARE(result.failures.size(), succeeded ? std::size_t{0} : std::size_t{1});

  QCOMPARE(failureLogs().size(), result.failures.size());
  if (!succeeded) QVERIFY(result.failures.front().detail.find("control set is unavailable") != std::string::npos);
  QCOMPARE(backend->snapshot(), before);
  const auto trace = backend->trace();
  QVERIFY(std::ranges::none_of(trace, [](const auto& entry) { return compat::isWrite(entry.request); }));
  backend->simulateRestart();
  if (operation == MaintenanceOperation::CommitValue)
    QCOMPARE(backend->visibleRegistryValue(key, value), std::optional<std::string>{"after"});
  else
    QVERIFY(!backend->visibleRegistryValue(key, value));
  if (operation == MaintenanceOperation::PersistKeyDeletion) QVERIFY(!backend->visibleRegistryKey(key));
}

void RegistryBatchCommitterTests::emptyPlansSkipUwfAndExternalDeletionsRetainTheirProviderFailure() {
  Fixture fixture;
  app::RegistryBatchCommitter committer(fixture.repository());
  QVERIFY(committer.commit({}).succeeded());
  QVERIFY(committer.persistDeletions({}).succeeded());
  QVERIFY(fixture.state->registryCommits.empty());
  QVERIFY(fixture.state->registryDeletions.empty());
  const std::string missingKey = "HKEY_LOCAL_MACHINE\\SOFTWARE\\ControlledRegistry\\Missing";
  fixture.backend->seedRegistryKey(missingKey);
  fixture.backend->deleteRegistryKey(missingKey);
  QVERIFY(!fixture.backend->visibleRegistryKey(missingKey));
  const auto result = committer.persistDeletions({app::RegistryCommitTarget::wholeKey(missingKey)});
  QCOMPARE(result.attempted, std::size_t{1});
  QCOMPARE(result.committed, std::size_t{0});
  QCOMPARE(result.failures.size(), std::size_t{1});
  QCOMPARE(result.failures.front().target.key, missingKey);
  QVERIFY(result.failures.front().detail.find("The registry deletion target is not visible") != std::string::npos);
  QCOMPARE(fixture.state->registryDeletions.size(), std::size_t{1});
  QCOMPARE(fixture.state->registryDeletions.front().key, missingKey);
  QCOMPARE(failureLogs().size(), std::size_t{1});
  QVERIFY(!fixture.backend->visibleRegistryKey(missingKey));
  QVERIFY(fixture.backend->snapshot().durable.registryKeys.contains(missingKey));
}

void RegistryBatchCommitterTests::commitLogsEveryFailureBeforeTheVisibleSummaryLimit_data() {
  QTest::addColumn<int>("count");
  QTest::newRow("beyond-four-visible-failures") << 5;
  QTest::newRow("one-hundred-failures") << 100;
}

void RegistryBatchCommitterTests::commitLogsEveryFailureBeforeTheVisibleSummaryLimit() {
  QFETCH(int, count);
  Fixture fixture;
  std::vector<app::RegistryCommitTarget> targets;
  std::vector<std::string> reasons;
  for (int index = 0; index < count; ++index) {
    targets.push_back(
        {QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\日志目标-%1").arg(index).toStdString(), QStringLiteral("配置值-%1").arg(index).toStdString()});
    reasons.push_back(index % 2 == 0 ? QStringLiteral("拒绝写入-%1\n原始 provider 多行详情").arg(index).toStdString() : "non-standard exception");
  }
  fixture.stageRegistryValues(targets);
  std::size_t invoked = 0;
  fixture.state->beforeRegistryCommit = [&] {
    const auto index = invoked++;
    if (index >= targets.size()) throw std::logic_error("unexpected registry invocation");
    fixture.state->registryCommitFailure = index % 2 == 0 ? std::make_exception_ptr(std::runtime_error(reasons[index])) : std::make_exception_ptr(42);
  };
  app::RegistryBatchCommitter committer(fixture.repository());
  const auto result = committer.commit(targets);
  QCOMPARE(result.attempted, targets.size());
  QCOMPARE(result.committed, std::size_t{0});
  QCOMPARE(result.failures.size(), targets.size());
  QCOMPARE(fixture.state->registryCommits.size(), targets.size());
  const auto lines = failureLogs();
  QCOMPARE(lines.size(), targets.size());
  for (std::size_t index = 0; index < targets.size(); ++index) {
    QCOMPARE(result.failures[index].target.key, targets[index].key);
    QCOMPARE(result.failures[index].target.valueName, targets[index].valueName);
    QCOMPARE(result.failures[index].detail, reasons[index]);
    const auto expected = "operation=commit key=" + targets[index].key + " value=" + targets[index].valueName + " error=" + reasons[index];
    QCOMPARE(std::count_if(lines.begin(), lines.end(), [&](const auto& line) { return line.find(expected) != std::string::npos; }), 1);
  }
  const auto copied = result;
  QVERIFY(!copied.succeeded());
  QCOMPARE(failureLogs(), lines);
}

void RegistryBatchCommitterTests::deletionLogsEveryFailureBeforeTheVisibleSummaryLimit_data() { commitLogsEveryFailureBeforeTheVisibleSummaryLimit_data(); }

void RegistryBatchCommitterTests::deletionLogsEveryFailureBeforeTheVisibleSummaryLimit() {
  QFETCH(int, count);
  const std::string root = "HKEY_LOCAL_MACHINE\\SOFTWARE\\ControlledRegistry";
  std::vector<app::RegistryCommitTarget> targets;
  std::vector<std::string> reasons;
  for (int index = 0; index < count; ++index) {
    const auto key = index % 3 == 0 ? root + QStringLiteral("\\分支-%1").arg(index).toStdString() : root;
    const auto name = index % 3 == 0 ? std::string{} : QStringLiteral("删除值-%1").arg(index).toStdString();
    targets.push_back(name.empty() ? app::RegistryCommitTarget::wholeKey(key) : app::RegistryCommitTarget{key, name});
    reasons.push_back(index % 2 == 0 ? QStringLiteral("删除失败-%1\n原始 Unicode 详情").arg(index).toStdString() : "non-standard exception");
  }
  Fixture fixture;
  app::RegistryBatchCommitter committer(fixture.repository());
  for (const auto& target : targets) {
    if (target.isKey()) {
      fixture.backend->seedRegistryKey(target.key);
      fixture.backend->deleteRegistryKey(target.key);
    } else {
      fixture.backend->seedRegistryValue(target.key, target.valueName, "before");
      fixture.backend->deleteRegistryValue(target.key, target.valueName);
    }
  }
  std::size_t invoked = 0;
  fixture.state->beforeRegistryCommit = [&] {
    const auto index = invoked++;
    if (index >= targets.size()) throw std::logic_error("unexpected registry deletion invocation");
    fixture.state->registryCommitFailure = index % 2 == 0 ? std::make_exception_ptr(std::runtime_error(reasons[index])) : std::make_exception_ptr(42);
  };
  const auto result = committer.persistDeletions(targets);
  QCOMPARE(result.attempted, targets.size());
  QCOMPARE(result.committed, std::size_t{0});
  QCOMPARE(result.failures.size(), targets.size());
  QCOMPARE(fixture.state->registryDeletions.size(), targets.size());
  const auto lines = failureLogs();
  QCOMPARE(lines.size(), targets.size());
  for (std::size_t index = 0; index < targets.size(); ++index) {
    QCOMPARE(result.failures[index].detail, reasons[index]);
    const auto expected = "operation=commit-deletion key=" + targets[index].key + " value=" + targets[index].valueName + " error=" + reasons[index];
    QCOMPARE(std::count_if(lines.begin(), lines.end(), [&](const auto& line) { return line.find(expected) != std::string::npos; }), 1);
  }
}

void RegistryBatchCommitterTests::persistedExternalDeletionsAreNotLoggedButUnconfirmedBaseDeletionsAre() {
  const std::string root = "HKEY_LOCAL_MACHINE\\SOFTWARE\\ControlledRegistry";
  const auto direct = root + "\\Direct";
  const auto pending = root + "\\Pending";
  const auto pendingKey = root + "\\PendingKey";
  Fixture fixture;
  auto initial = fixture.backend->snapshot();
  initial.current.registryExclusions = {direct};
  fixture.backend->seed(std::move(initial));
  app::RegistryBatchCommitter committer(fixture.repository());
  fixture.backend->seedRegistryValue(direct, "existing-value", "before");
  fixture.backend->seedRegistryValue(pending, "pending-value", "before");
  fixture.backend->seedRegistryKey(pendingKey);
  fixture.backend->deleteRegistryValue(direct, "existing-value");
  fixture.backend->deleteRegistryValue(pending, "pending-value");
  fixture.backend->deleteRegistryKey(pendingKey);
  QVERIFY(!fixture.backend->readRegistryValue(direct, "existing-value"));
  QVERIFY(fixture.backend->readRegistryValue(pending, "pending-value"));
  const auto result = committer.persistDeletions({{direct, "existing-value"}, {pending, "pending-value"}, app::RegistryCommitTarget::wholeKey(pendingKey)});
  QVERIFY(!result.succeeded());
  QCOMPARE(result.committed, std::size_t{1});
  QCOMPARE(result.failures.size(), std::size_t{2});
  QCOMPARE(result.failures[0].target.valueName, std::string("pending-value"));
  QCOMPARE(result.failures[1].target.key, pendingKey);
  const auto lines = failureLogs();
  QCOMPARE(lines.size(), std::size_t{2});
  for (const auto& line : lines) QVERIFY(line.find("existing-value") == std::string::npos);
  QVERIFY(fixture.state->registryCommits.empty());
  QCOMPARE(fixture.state->registryDeletions.size(), std::size_t{2});
  QVERIFY(fixture.backend->readRegistryValue(pending, "pending-value"));
  QVERIFY(fixture.backend->snapshot().durable.registryKeys.contains(pendingKey));
}

void RegistryBatchCommitterTests::requestPreparationFailuresRemainBatchFailures_data() {
  QTest::addColumn<bool>("unknown");
  QTest::newRow("multiline-utf8") << false;
  QTest::newRow("unknown") << true;
}

void RegistryBatchCommitterTests::requestPreparationFailuresRemainBatchFailures() {
  QFETCH(bool, unknown);
  Fixture fixture;
  const auto detail = QStringLiteral("无法读取注册表批次\n保留完整的原始 Unicode 错误");
  fixture.state->registryTargetFailure = unknown ? std::make_exception_ptr(42) : std::make_exception_ptr(std::runtime_error(detail.toStdString()));
  app::RegistryBatchCommitter committer(fixture.repository());
  const std::string root = "HKEY_LOCAL_MACHINE\\SOFTWARE\\ControlledRegistry";
  const std::vector<app::RegistryCommitTarget> targets{{root, "first"}, {root, "second"}};
  fixture.stageRegistryValues(targets);
  fixture.backend->seedRegistryValue(root, "existing-value", "before");
  fixture.backend->deleteRegistryValue(root, "existing-value");
  const auto verify = [&](const auto& work) {
    try {
      work();
      QFAIL("Batch preparation must retain its failure");
    } catch (const std::runtime_error& error) {
      QVERIFY(!unknown);
      QCOMPARE(QString::fromUtf8(error.what()), detail);
    } catch (int value) {
      QVERIFY(unknown);
      QCOMPARE(value, 42);
    }
  };
  verify([&] { (void)committer.commit(targets); });
  verify([&] { (void)committer.persistDeletions({{root, "existing-value"}}); });
  QVERIFY(fixture.state->registryCommits.empty());
  QVERIFY(fixture.state->registryDeletions.empty());
  QVERIFY(failureLogs().empty());
  auto errors = recentLogLines();
  std::erase_if(errors, [](const auto& line) { return line.find(" E ") == std::string::npos; });
  QCOMPARE(errors.size(), std::size_t{2});
  for (const auto& line : errors) {
    QVERIFY(line.find("registry commit batch read failed:") != std::string::npos);
    QVERIFY(line.find(unknown ? "non-standard-exception" : detail.toStdString()) != std::string::npos);
  }
  QVERIFY(errors.front().find("operation=commit ") != std::string::npos);
  QVERIFY(errors.back().find("operation=commit-deletion ") != std::string::npos);
  fixture.state->registryTargetFailure = {};
  const auto retried = committer.commit(targets);
  QVERIFY(retried.succeeded());
  QCOMPARE(retried.committed, targets.size());
  auto afterRetry = recentLogLines();
  std::erase_if(afterRetry, [](const auto& line) { return line.find(" E ") == std::string::npos; });
  QCOMPARE(afterRetry, errors);
}
void RegistryBatchCommitterTests::missingCommitValuesAreNotCreated_data() {
  QTest::addColumn<bool>("removedAfterPreparation");
  QTest::newRow("never-present") << false;
  QTest::newRow("removed-before-dispatch") << true;
}
void RegistryBatchCommitterTests::missingCommitValuesAreNotCreated() {
  QFETCH(bool, removedAfterPreparation);
  Fixture fixture;
  const std::string key = "HKEY_LOCAL_MACHINE\\SOFTWARE\\ControlledRegistry";
  const std::string name = "MissingValue";
  if (removedAfterPreparation) {
    fixture.stageRegistryValue(key, name, "pending contents");
    fixture.state->beforeRegistryCommit = [backend = fixture.backend, key, name] { backend->deleteRegistryValue(key, name); };
  }
  app::RegistryBatchCommitter committer(fixture.repository());
  const auto result = committer.commit({{key, name}});
  QCOMPARE(result.attempted, std::size_t{1});
  QCOMPARE(result.committed, std::size_t{0});
  QCOMPARE(result.failures.size(), std::size_t{1});
  QVERIFY(!fixture.backend->visibleRegistryValue(key, name));
  QVERIFY(!fixture.backend->readRegistryValue(key, name));
}

void RegistryBatchCommitterTests::currentExclusionCoveragePreservesRegistryDurability_data() {
  QTest::addColumn<QString>("pattern");
  QTest::addColumn<QString>("key");
  QTest::addColumn<bool>("managementSkipsCommit");
  QTest::addColumn<bool>("driverBypassesWrites");
  QTest::addColumn<bool>("deletion");
  QTest::addColumn<bool>("wholeKey");
  QTest::addColumn<std::uint32_t>("selectedControlSet");
  QTest::addColumn<bool>("controlSetRequired");
  struct Coverage {
    const char* name;
    const char16_t* patternLeaf;
    const char16_t* targetLeaf;
    bool management, driver;
  };
  constexpr std::array coverage{
      Coverage{"plain", u"Vendor", u"Vendor\\Child", true, true},
      Coverage{"one-tail", u"Vendor\\", u"Vendor\\Child", true, false},
      Coverage{"two-tails", u"Vendor\\\\", u"Vendor\\Child", false, false},
      Coverage{"ordinal-alias", u"\u00c4", u"\u00e4\\Child", false, true},
      Coverage{"ordinal-distinct", u"\u00b5", u"\u03bc\\Child", false, false},
  };
  struct Operation {
    const char* name;
    bool deletion, wholeKey;
  };
  constexpr std::array operations{Operation{"commit-values", false, false}, Operation{"persist-deleted-value", true, false},
                                  Operation{"persist-deleted-key", true, true}};
  const auto root = QStringLiteral("HKLM\\SOFTWARE\\CoverageFixtures\\");
  for (const auto& row : coverage)
    for (const auto& operation : operations)
      QTest::newRow(qPrintable(QStringLiteral("%1/%2").arg(QString::fromLatin1(row.name), QString::fromLatin1(operation.name))))
          << root + QString::fromUtf16(row.patternLeaf) << root + QString::fromUtf16(row.targetLeaf) << row.management << row.driver << operation.deletion
          << operation.wholeKey << std::uint32_t{1} << false;
  const std::array names{QStringLiteral("CurrentControlSet"), QStringLiteral("ControlSet001"), QStringLiteral("ControlSet002")};
  for (const std::uint32_t current : {1u, 2u}) {
    for (int pattern = 0; pattern < static_cast<int>(names.size()); ++pattern)
      for (int target = 0; target < static_cast<int>(names.size()); ++target)
        for (int tails = 0; tails <= 2; ++tails)
          for (const auto& operation : operations) {
            const auto stored =
                QStringLiteral("HKLM\\SYSTEM\\") + names[static_cast<std::size_t>(pattern)] + QStringLiteral("\\CoverageFixtures") + QString(tails, u'\\');
            const auto actual = QStringLiteral("HKLM\\SYSTEM\\") + names[static_cast<std::size_t>(target)] + QStringLiteral("\\CoverageFixtures\\Child");
            const auto patternSet = pattern == 0 ? current : static_cast<std::uint32_t>(pattern);
            const auto targetSet = target == 0 ? current : static_cast<std::uint32_t>(target);
            QTest::newRow(qPrintable(QStringLiteral("controlset-%1-pattern-%2-target-%3-tails-%4/%5")
                                         .arg(current)
                                         .arg(pattern)
                                         .arg(target)
                                         .arg(tails)
                                         .arg(QString::fromLatin1(operation.name))))
                << stored << actual << (pattern == target && tails <= 1) << (patternSet == targetSet && tails == 0) << operation.deletion << operation.wholeKey
                << current << (((pattern == 0) != (target == 0)) && tails == 0);
          }
    for (const bool actualCssExtra : {false, true})
      for (const auto& operation : operations) {
        const auto stored = QStringLiteral("HKLM\\SYSTEM\\CurrentControlSetExtra\\CoverageFixtures");
        const auto actual = (actualCssExtra ? QStringLiteral("HKLM\\SYSTEM\\CurrentControlSetExtra\\") : QStringLiteral("HKLM\\SYSTEM\\ControlSet001Extra\\")) +
                            QStringLiteral("CoverageFixtures\\Child");
        QTest::newRow(
            qPrintable(QStringLiteral("controlset-extra-%1-actual-alias-%2/%3").arg(current).arg(actualCssExtra).arg(QString::fromLatin1(operation.name))))
            << stored << actual << actualCssExtra << (!actualCssExtra && current == 1) << operation.deletion << operation.wholeKey << current
            << !actualCssExtra;
      }
  }
}

void RegistryBatchCommitterTests::currentExclusionCoveragePreservesRegistryDurability() {
  QFETCH(QString, pattern);
  QFETCH(QString, key);
  QFETCH(bool, managementSkipsCommit);
  QFETCH(bool, driverBypassesWrites);
  QFETCH(bool, deletion);
  QFETCH(bool, wholeKey);
  QFETCH(std::uint32_t, selectedControlSet);
  QFETCH(bool, controlSetRequired);
  using test::FakeUwfBackend;
  FakeUwfBackend::State initial;
  initial.current.filterEnabled = true;
  initial.runtime.currentControlSet = selectedControlSet;
  FakeUwfBackend::VolumeConfiguration systemVolume;
  systemVolume.volume = {Fixture::volumeId, "C:", true, false, false};
  initial.current.volumes.emplace(Fixture::volumeId, std::move(systemVolume));
  initial.runtime.systemVolumeId = Fixture::volumeId;
  initial.current.registryExclusions = {pattern.toStdString()};
  initial.next = initial.current;
  // Next deliberately suggests the opposite answer. Durability concerns the
  // running driver and must never be inferred from a future exclusion list.
  initial.next.registryExclusions = driverBypassesWrites ? std::vector<std::string>{} : std::vector<std::string>{"HKLM\\SOFTWARE\\CoverageFixtures"};
  const auto backend = std::make_shared<FakeUwfBackend>(std::move(initial));
  const auto target = key.toStdString();
  const std::string name = "Named";
  backend->seedRegistryValue(target, name, "before");

  app::RegistryBatchCommitter committer(app::uwfMaintenanceRepository(backend));
  std::vector<app::RegistryCommitTarget> targets;
  if (deletion) {
    // Model enhanced-mode removal: ordinary host deletion precedes the persistence batch.
    // Visible absence alone does not establish durability.
    if (wholeKey)
      backend->deleteRegistryKey(target);
    else
      backend->deleteRegistryValue(target, name);
    targets.push_back(wholeKey ? app::RegistryCommitTarget::wholeKey(target) : app::RegistryCommitTarget{target, name});
    QVERIFY(!backend->visibleRegistryValue(target, name));
    QCOMPARE(backend->readRegistryValue(target, name).has_value(), !driverBypassesWrites);
  } else {
    // Named and default values share the coverage rule, while the empty value
    // name remains a real value identity for ordinary commits.
    backend->seedRegistryValue(target, {}, "before");
    for (const auto& value : {std::string{}, name}) {
      backend->writeRegistryValue(target, value, "after");
      targets.push_back({target, value});
      const auto durable = backend->readRegistryValue(target, value);
      QVERIFY(durable);
      QCOMPARE(*durable, driverBypassesWrites ? std::string("after") : std::string("before"));
    }
  }
  const auto result = deletion ? committer.persistDeletions(targets) : committer.commit(targets);
  const bool persisted = driverBypassesWrites || (!deletion && !managementSkipsCommit);
  QCOMPARE(result.attempted, targets.size());
  QCOMPARE(result.succeeded(), persisted);
  QCOMPARE(result.committed, persisted ? targets.size() : std::size_t{0});
  QCOMPARE(result.failures.size(), persisted ? std::size_t{0} : targets.size());
  QCOMPARE(failureLogs().size(), result.failures.size());
  if (deletion) {
    QVERIFY(!backend->visibleRegistryValue(target, name));
    QCOMPARE(backend->readRegistryValue(target, name).has_value(), !persisted);
  } else {
    for (const auto& value : {std::string{}, name}) {
      const auto visible = backend->visibleRegistryValue(target, value);
      const auto durable = backend->readRegistryValue(target, value);
      QVERIFY(visible);
      QVERIFY(durable);
      QCOMPARE(*visible, std::string("after"));
      QCOMPARE(*durable, persisted ? std::string("after") : std::string("before"));
      QCOMPARE(backend->readRegistryValue(target, value, FakeUwfBackend::Storage::Overlay).has_value(), !persisted);
    }
  }
  const auto trace = backend->trace();
  std::size_t currentExclusionReads = 0;
  for (const auto& entry : trace)
    if (const auto* request = std::get_if<compat::GetRegistryExclusions>(&entry.request)) {
      QCOMPARE(request->session, compat::Session::Current);
      ++currentExclusionReads;
    }
  QCOMPARE(currentExclusionReads, std::size_t{0});

  if (controlSetRequired) QCOMPARE(backend->state().runtime.currentControlSet.value, std::optional{selectedControlSet});
  backend->simulateRestart();
  if (deletion) {
    QCOMPARE(backend->visibleRegistryValue(target, name).has_value(), !persisted);
    if (wholeKey) QCOMPARE(backend->visibleRegistryKey(target), !persisted);
  } else {
    for (const auto& value : {std::string{}, name})
      QCOMPARE(backend->visibleRegistryValue(target, value), std::optional<std::string>{persisted ? "after" : "before"});
  }
}
void RegistryBatchCommitterTests::controlSetReadFailuresCannotBecomeDurabilityProof_data() {
  QTest::addColumn<QString>("root");
  QTest::addColumn<QString>("targetRoot");
  QTest::addColumn<bool>("required");
  QTest::addColumn<bool>("deletion");
  for (const bool deletion : {false, true}) {
    const auto software = QStringLiteral("HKLM\\SOFTWARE\\ReaderProbe");
    const auto ordinary = QStringLiteral("HKLM\\SYSTEM\\Ordinary\\ReaderProbe");
    const auto css = QStringLiteral("HKLM\\SYSTEM\\CurrentControlSet\\ReaderProbe");
    QTest::newRow(deletion ? "software/delete" : "software/commit") << software << software << false << deletion;
    QTest::newRow(deletion ? "ordinary-system/delete" : "ordinary-system/commit") << ordinary << ordinary << false << deletion;
    QTest::newRow(deletion ? "same-current-control-set/delete" : "same-current-control-set/commit") << css << css << false << deletion;
    QTest::newRow(deletion ? "CSS-to-numbered-control-set/delete" : "CSS-to-numbered-control-set/commit")
        << css << QStringLiteral("HKLM\\SYSTEM\\ControlSet002\\ReaderProbe") << true << deletion;
  }
}
void RegistryBatchCommitterTests::controlSetReadFailuresCannotBecomeDurabilityProof() {
  QFETCH(QString, root);
  QFETCH(QString, targetRoot);
  QFETCH(bool, required);
  QFETCH(bool, deletion);
  auto initial = protectedRegistryState();
  initial.current.registryExclusions = {root.toStdString()};
  initial.runtime.currentControlSet = 2;
  initial.next = initial.current;
  const auto backend = std::make_shared<test::FakeUwfBackend>(initial);
  const auto key = (targetRoot + QStringLiteral("\\Child")).toStdString();
  const std::string value = "Named";
  backend->seedRegistryValue(key, value, "before");
  if (deletion)
    backend->deleteRegistryValue(key, value);
  else
    backend->writeRegistryValue(key, value, "after");

  const auto checkedSystem = std::make_shared<app::PrecheckedUwf>(backend);
  auto published = backend->state();
  published.runtime.currentControlSet = {};
  checkedSystem->updateState(std::move(published));
  app::RegistryBatchCommitter committer(app::uwfMaintenanceRepository(checkedSystem));
  const auto before = backend->snapshot();
  backend->clearTrace();
  const std::vector<app::RegistryCommitTarget> targets{{key, value}};
  const auto result = deletion ? committer.persistDeletions(targets) : committer.commit(targets);

  QCOMPARE(result.succeeded(), !required);
  QCOMPARE(result.committed, required ? std::size_t{0} : std::size_t{1});
  QCOMPARE(result.failures.size(), required ? std::size_t{1} : std::size_t{0});
  if (required) QVERIFY(result.failures.front().detail.find("control set is unavailable") != std::string::npos);
  QCOMPARE(failureLogs().size(), result.failures.size());
  QVERIFY(backend->snapshot() == before);
  const auto trace = backend->trace();
  QVERIFY(std::ranges::none_of(trace, [](const auto& entry) {
    return std::holds_alternative<compat::CommitRegistry>(entry.request) || std::holds_alternative<compat::CommitRegistryDeletion>(entry.request);
  }));
}

void RegistryBatchCommitterTests::controlSetNumericFormatGovernsEnvironmentReads_data() {
  QTest::addColumn<QString>("patternComponent");
  QTest::addColumn<QString>("targetComponent");
  QTest::addColumn<bool>("required");
  for (const auto& literal : {QStringLiteral("ControlSetOther"), QStringLiteral("ControlSet"), QStringLiteral("ControlSet0"), QStringLiteral("ControlSet00"),
                              QStringLiteral("ControlSet01")}) {
    QTest::newRow(qPrintable(QStringLiteral("CSS-to-%1").arg(literal))) << QStringLiteral("CurrentControlSet") << literal << false;
    QTest::newRow(qPrintable(QStringLiteral("%1-to-CSS").arg(literal))) << literal << QStringLiteral("CurrentControlSet") << false;
  }
  QTest::newRow("CSS-to-001") << QStringLiteral("CurrentControlSet") << QStringLiteral("ControlSet001") << true;
  QTest::newRow("CSS-to-002") << QStringLiteral("CurrentControlSet") << QStringLiteral("ControlSet002") << true;
  QTest::newRow("CSS-alpha-suffix") << QStringLiteral("CurrentControlSetExtra") << QStringLiteral("ControlSet002Extra") << true;
  QTest::newRow("CSS-numeric-suffix") << QStringLiteral("CurrentControlSet1") << QStringLiteral("ControlSet0021") << true;
  QTest::newRow("CSS-zero-numeric-suffix") << QStringLiteral("CurrentControlSet0") << QStringLiteral("ControlSet0000") << true;
  QTest::newRow("plain-CSS-cannot-match-alpha-suffix") << QStringLiteral("CurrentControlSet") << QStringLiteral("ControlSet002Extra") << false;
  QTest::newRow("alpha-suffix-cannot-match-complete-CSS") << QStringLiteral("ControlSet002Extra") << QStringLiteral("CurrentControlSet") << false;
  QTest::newRow("extra-leading-zero-is-not-a-formatted-id") << QStringLiteral("CurrentControlSet") << QStringLiteral("ControlSet0011") << false;
  QTest::newRow("numeric-suffix-mismatch") << QStringLiteral("CurrentControlSet1") << QStringLiteral("ControlSet0020") << false;
}
void RegistryBatchCommitterTests::controlSetNumericFormatGovernsEnvironmentReads() {
  QFETCH(QString, patternComponent);
  QFETCH(QString, targetComponent);
  QFETCH(bool, required);
  const auto pattern = QStringLiteral("HKLM\\SYSTEM\\") + patternComponent + QStringLiteral("\\FormatFixture");
  const auto target = (QStringLiteral("HKLM\\SYSTEM\\") + targetComponent + QStringLiteral("\\FormatFixture\\Child")).toStdString();
  auto initial = protectedRegistryState();
  initial.current.registryExclusions = {pattern.toStdString()};
  initial.runtime.currentControlSet = 2;
  initial.next = initial.current;
  const auto backend = std::make_shared<test::FakeUwfBackend>(initial);
  backend->seedRegistryValue(target, "Named", "before");
  backend->writeRegistryValue(target, "Named", "changed");

  const auto checkedSystem = std::make_shared<app::PrecheckedUwf>(backend);
  auto published = backend->state();
  published.runtime.currentControlSet = {};
  checkedSystem->updateState(std::move(published));
  app::RegistryBatchCommitter committer(app::uwfMaintenanceRepository(checkedSystem));
  const auto before = backend->snapshot();
  const auto result = committer.commit({{target, "Named"}});
  QCOMPARE(result.succeeded(), !required);
  QCOMPARE(result.committed, required ? std::size_t{0} : std::size_t{1});

  if (required) {
    QCOMPARE(result.failures.size(), std::size_t{1});
    QVERIFY(result.failures.front().detail.find("control set is unavailable") != std::string::npos);
    QVERIFY(backend->snapshot() == before);
  } else {
    QCOMPARE(backend->readRegistryValue(target, "Named"), std::optional<std::string>{"changed"});
    QVERIFY(!backend->readRegistryValue(target, "Named", test::FakeUwfBackend::Storage::Overlay));
    backend->simulateRestart();
    QCOMPARE(backend->visibleRegistryValue(target, "Named"), std::optional<std::string>{"changed"});
  }
  const auto trace = backend->trace();
  QCOMPARE(std::ranges::count_if(trace, [](const auto& entry) { return std::holds_alternative<compat::CommitRegistry>(entry.request); }), required ? 0 : 1);
}

}  // namespace

QTEST_APPLESS_MAIN(RegistryBatchCommitterTests)
#include "RegistryBatchCommitterTests.moc"
