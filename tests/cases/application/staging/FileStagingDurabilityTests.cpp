// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QTest>
#include <algorithm>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <system_error>
#include <utility>
#include <variant>
#include <vector>

#include "application/commit/CommitWorkflow.h"
#include "application/commit/contracts/CommitTraversalSource.h"
#include "application/configuration/execution/ConfigurationExecutor.h"
#include "application/operations/contracts/OperationError.h"
#include "application/staging/editing/FileStagingDraft.h"
#include "application/staging/execution/FileStagingTask.h"
#include "application/staging/persistence/DurableFileStagingStore.h"
#include "application/uwf/UwfRequestPrecheck.h"
#include "application/uwf/adapters/UwfConfigurationRepository.h"
#include "application/uwf/adapters/UwfMaintenanceRepositoryFactory.h"
#include "application/uwf/adapters/UwfReadState.h"
#include "support/app/UwfMaintenanceFixture.h"

using namespace uwf;
namespace {
// Use the backend's visible registry with opaque staging snapshots; codec behavior has separate tests.
class SimulatedRegistryStorage final : public app::FileStagingStorage {
 public:
  explicit SimulatedRegistryStorage(std::shared_ptr<test::FakeUwfBackend> backend) : m_backend(std::move(backend)) {}
  void seed(const QList<app::FileStagingEntry>& entries) {
    m_backend->seedRegistryKey(std::string(app::kFileStagingRegistryRoot));
    m_backend->seedRegistryValue(std::string(app::kFileStagingRegistryKey), std::string(app::kFileStagingEntriesValue), encode(entries));
  }
  QList<app::FileStagingEntry> load() const override {
    const auto contents = m_backend->visibleRegistryValue(std::string(app::kFileStagingRegistryKey), std::string(app::kFileStagingEntriesValue));
    if (!contents || contents->empty()) return {};
    const auto snapshot = std::ranges::find_if(m_snapshots, [&](const auto& item) { return item.first == *contents; });
    if (snapshot == m_snapshots.end()) throw std::runtime_error("unknown simulated staging snapshot");
    return snapshot->second;
  }
  void write(const QList<app::FileStagingEntry>& entries) override {
    ++writes;
    // Model RegCreateKeyEx before setting Entries so recreation follows the driver's write policy.
    m_backend->createRegistryKey(std::string(app::kFileStagingRegistryRoot));
    m_backend->writeRegistryValue(std::string(app::kFileStagingRegistryKey), std::string(app::kFileStagingEntriesValue), encode(entries));
  }
  void erase() override {
    ++erasures;
    for (const auto& target : deletionPlan()) m_backend->deleteRegistryKey(target.key);
  }
  std::vector<app::RegistryCommitTarget> deletionPlan() const override {
    std::vector<app::RegistryCommitTarget> targets;
    for (const auto key : {app::kFileStagingRegistryKey, app::kFileStagingRegistryRoot})
      if (m_backend->visibleRegistryKey(std::string(key))) targets.push_back(app::RegistryCommitTarget::wholeKey(std::string(key)));
    return targets;
  }
  int writes = 0;
  int erasures = 0;

 private:
  std::string encode(const QList<app::FileStagingEntry>& entries) {
    if (entries.isEmpty()) return {};
    const auto found = std::ranges::find_if(m_snapshots, [&](const auto& item) { return item.second == entries; });
    if (found != m_snapshots.end()) return found->first;
    const auto token = "staging-snapshot-" + std::to_string(m_snapshots.size());
    m_snapshots.emplace_back(token, entries);
    return token;
  }
  std::shared_ptr<test::FakeUwfBackend> m_backend;
  std::vector<std::pair<std::string, QList<app::FileStagingEntry>>> m_snapshots;
};

class StagingRegistrySource final : public app::CommitTraversalSource {
 public:
  explicit StagingRegistrySource(std::shared_ptr<test::FakeUwfBackend> backend) : m_backend(std::move(backend)) {}
  std::optional<app::CommitNode> inspect(const app::CommitNode& target) const override { return exists(target) ? std::optional{target} : std::nullopt; }
  bool exists(const app::CommitNode& target) const override {
    return target.kind == app::CommitTargetKind::RegistryKey
               ? m_backend->visibleRegistryKey(target.path.toStdString())
               : m_backend->visibleRegistryValue(target.path.toStdString(), target.valueName.toStdString()).has_value();
  }
  std::string driveLetterForPath(const QString&) const override { return {}; }
  app::FileCommitTarget resolveFileTarget(const QString&) const override { throw std::logic_error("Registry source has no file identity"); }
  QString enumerateChildren(const app::CommitNode& target, const std::stop_token stop, const app::CommitNodeVisitor& discover) const override {
    if (target.kind != app::CommitTargetKind::RegistryKey || stop.stop_requested()) return {};
    if (target.path.toStdString() == app::kFileStagingRegistryRoot) {
      const app::CommitNode child{app::CommitTargetKind::RegistryKey, QString::fromUtf8(app::kFileStagingRegistryKey), {}, {}};
      if (exists(child)) discover(child);
    } else if (target.path.toStdString() == app::kFileStagingRegistryKey) {
      const app::CommitNode value{app::CommitTargetKind::RegistryValue, target.path, QString::fromUtf8(app::kFileStagingEntriesValue), {}};
      if (exists(value)) discover(value);
    }
    return {};
  }

 private:
  std::shared_ptr<test::FakeUwfBackend> m_backend;
};

test::FakeUwfBackend::State protectedStagingState() {
  test::FakeUwfBackend::State state;
  state.current.filterEnabled = true;
  const std::string systemVolume(test::application::UwfMaintenanceFixture::volumeId);
  test::FakeUwfBackend::VolumeConfiguration volume;
  volume.volume = {systemVolume, "C:", true, false, false};
  state.current.volumes.emplace(systemVolume, std::move(volume));
  state.runtime.systemVolumeId = systemVolume;
  state.runtime.fileMaintenancePrefixes[systemVolume] = "C:";
  state.next = state.current;
  state.next.registryExclusions = {std::string(app::kFileStagingRegistryRoot)};
  return state;
}
std::shared_ptr<test::FakeUwfBackend> protectedStagingBackend() {
  auto state = protectedStagingState();
  return std::make_shared<test::FakeUwfBackend>(std::move(state));
}

enum class MaintenanceMode { Ordinary, ReadOnlyMedia, Horm, FullCommit };
enum class StagingWrite { Replace, Clear, Reconcile };
enum class MaintenanceObservation { RegistryPersistence, FilterEnabled, HormEnabled, OverlayFlags };
enum class ReconciliationFailure { NextRead, NextWrite, AfterNextWrite };

class DetailedPersistenceFailure final : public app::OperationError {
 public:
  explicit DetailedPersistenceFailure(const app::WriteEffect effect)
      : OperationError(std::make_error_code(std::errc::io_error), "generic category must not replace provider detail", effect) {}
  const char* what() const noexcept override { return "Persistence target was not found\nOriginal provider diagnostic"; }
  app::OperationFailureKind failureKind() const noexcept override { return app::OperationFailureKind::NotFound; }
  app::OperationDiagnostic diagnostic() const noexcept override { return {1234, 5678}; }
};

void applyStagingWrite(app::DurableFileStagingStore& store, const StagingWrite operation, const QList<app::FileStagingEntry>& replacement) {
  switch (operation) {
    case StagingWrite::Replace:
      store.replace(replacement);
      break;
    case StagingWrite::Clear:
      store.replace({});
      break;
    case StagingWrite::Reconcile:
      store.reconcileDurability();
      break;
  }
}

std::shared_ptr<test::FakeUwfBackend> stagingBackend(const MaintenanceMode mode, const bool excluded, const bool systemProtected = true) {
  auto state = protectedStagingState();
  if (mode != MaintenanceMode::Ordinary) state.current.overlayFlags = compat::flags::readOnlyMedia;
  state.runtime.hormEnabled = mode == MaintenanceMode::Horm;
  state.runtime.romDisposition = mode == MaintenanceMode::FullCommit ? compat::disposition::fullCommit : 0;
  auto& volume = state.current.volumes.at(test::application::UwfMaintenanceFixture::volumeId);
  volume.volume.protectedVolume = systemProtected;
  if (excluded) {
    volume.fileExclusions = {"\\data"};
    state.current.registryExclusions = {std::string(app::kFileStagingRegistryRoot)};
  }
  if (!systemProtected) {
    constexpr auto otherVolume = "66666666-7777-8888-9999-aaaaaaaaaaaa";
    test::FakeUwfBackend::VolumeConfiguration other;
    other.volume = {otherVolume, "D:", true, false, false};
    state.current.volumes.emplace(otherVolume, std::move(other));
  }
  state.next = state.current;
  state.next.registryExclusions = {std::string(app::kFileStagingRegistryRoot)};
  return std::make_shared<test::FakeUwfBackend>(std::move(state));
}

class StagedFileSource final : public app::CommitTraversalSource {
 public:
  std::optional<app::CommitNode> inspect(const app::CommitNode& node) const override { return node; }
  bool exists(const app::CommitNode&) const override { return true; }
  std::string driveLetterForPath(const QString&) const override { return "C:"; }
  app::FileCommitTarget resolveFileTarget(const QString& path) const override { return test::application::memoryFileTarget(path); }
  QString enumerateChildren(const app::CommitNode&, std::stop_token, const app::CommitNodeVisitor&) const override { return {}; }
};
}  // namespace
class FileStagingDurabilityTests final : public QObject {
  Q_OBJECT
 private slots:
  void aHiddenCurrentRootDoesNotProveThePersistentListIsEmpty_data() {
    QTest::addColumn<bool>("recreateStore");
    QTest::addColumn<bool>("failBarrier");
    QTest::addColumn<bool>("removeThroughPlan");
    for (const bool recreate : {false, true})
      for (const bool fail : {false, true})
        for (const bool throughPlan : {false, true})
          QTest::newRow(qPrintable(QStringLiteral("recreate-%1-barrier-failure-%2-plan-%3").arg(recreate).arg(fail).arg(throughPlan)))
              << recreate << fail << throughPlan;
  }
  void aHiddenCurrentRootDoesNotProveThePersistentListIsEmpty() {
    QFETCH(bool, recreateStore);
    QFETCH(bool, failBarrier);
    QFETCH(bool, removeThroughPlan);
    const auto backend = protectedStagingBackend();
    const auto storage = std::make_shared<SimulatedRegistryStorage>(backend);
    const QList<app::FileStagingEntry> original{{app::FileStagingKind::File, QStringLiteral("C:\\old.txt")}};
    storage->seed(original);
    const auto originalData = backend->readRegistryValue(std::string(app::kFileStagingRegistryKey), std::string(app::kFileStagingEntriesValue));
    QVERIFY(originalData && !originalData->empty());
    const auto repository = app::uwfMaintenanceRepository(backend);
    auto store = std::make_shared<app::DurableFileStagingStore>(storage, repository, app::UwfCapability::Available);
    app::CommitRequest request;
    std::shared_ptr<app::CommitBackend> maintenance;
    const auto source = std::make_shared<StagingRegistrySource>(backend);
    app::CommitWorkflow workflow(repository, {[&](const auto& submitted, auto prepared) {
                                   request = submitted;
                                   maintenance = std::move(prepared);
                                 }},
                                 source);
    workflow.commitRegistryDeletionKey(QString::fromUtf8(app::kFileStagingRegistryRoot), {});
    QVERIFY(maintenance);
    const auto plan = maintenance->prepare(request, {}, {}, {});
    QCOMPARE(plan.progress().failed, 0);
    const auto targets = plan.targets();
    QCOMPARE(targets.size(), 2);
    QCOMPARE(targets.front().path, QString::fromUtf8(app::kFileStagingRegistryKey));
    QCOMPARE(targets.back().path, QString::fromUtf8(app::kFileStagingRegistryRoot));
    const compat::Request childDeletion = compat::CommitRegistryDeletion{std::string(app::kFileStagingRegistryKey), std::nullopt};
    backend->inject(
        {childDeletion, {}, test::FakeUwfBackend::FaultTiming::AfterVisibleDeletion, compat::ErrorKind::AccessDenied, "The persistent child deletion failed"});
    for (const auto& target : targets) {
      const auto deleted = maintenance->execute(target, request.operation);
      QVERIFY(!deleted.succeeded);
      QCOMPARE(deleted.failureEffect, app::WriteEffect::Uncertain);
    }
    QVERIFY(!backend->visibleRegistryKey(std::string(app::kFileStagingRegistryRoot)));
    QCOMPARE(backend->readRegistryValue(std::string(app::kFileStagingRegistryKey), std::string(app::kFileStagingEntriesValue)), originalData);
    QVERIFY(storage->load().isEmpty());
    if (recreateStore) store = std::make_shared<app::DurableFileStagingStore>(storage, repository, app::UwfCapability::Available);
    const auto draft = std::make_shared<app::FileStagingDraft>(store);
    draft->reload();
    QVERIFY(draft->load().isEmpty());
    QVERIFY(!draft->changes());
    app::ConfigurationExecutor executor(app::uwfConfigurationRepository(backend, draft));
    const auto removeExclusion = [&] {
      core::PendingChanges changes;
      changes.removeRegistryExclusions = {std::string(app::kFileStagingRegistryRoot)};
      const auto reviewed =
          app::planConfiguration(changes, app::publishedUwfSnapshot(backend->state(), app::UwfCapability::Available, true, {}), draft->changes());
      if (!reviewed.staging || !reviewed.staging->after.isEmpty() || !reviewed.staging->persistencePending)
        throw std::logic_error("Removing the staging exclusion must first confirm its persistent list is empty");
      return executor.apply(reviewed);
    };
    if (failBarrier) {
      const compat::Request commit = compat::CommitRegistry{std::string(app::kFileStagingRegistryKey), std::string(app::kFileStagingEntriesValue)};
      backend->inject({commit, {}, test::FakeUwfBackend::FaultTiming::Before, compat::ErrorKind::AccessDenied, "The empty-list barrier was not committed"});
      if (removeThroughPlan) {
        const auto result = removeExclusion();
        QCOMPARE(result.failedOperations(), 1);
        QVERIFY(result.staging() && result.staging()->failure);
        const auto skippedRemoval = std::ranges::find_if(
            result.steps, [](const auto& step) { return step.operation && step.operation->kind == app::ConfigurationOperationKind::RemoveRegistryExclusion; });
        QVERIFY(skippedRemoval != result.steps.end());
        QCOMPARE(skippedRemoval->outcome, app::ConfigurationStepOutcome::NotAttempted);
        QCOMPARE(skippedRemoval->writeEffect, app::WriteEffect::None);
      } else {
        try {
          store->replace({});
          QFAIL("An absent current root must still require the persistent empty-list barrier");
        } catch (const app::OperationError& error) {
          QCOMPARE(error.writeEffect(), app::WriteEffect::Uncertain);
          QVERIFY(QString::fromUtf8(error.what()).contains(QStringLiteral("empty-list barrier")));
        }
      }
      QCOMPARE(backend->snapshot().next.registryExclusions, std::vector<std::string>{std::string(app::kFileStagingRegistryRoot)});
      QCOMPARE(backend->readRegistryValue(std::string(app::kFileStagingRegistryKey), std::string(app::kFileStagingEntriesValue)), originalData);
    }
    if (removeThroughPlan)
      QCOMPARE(removeExclusion().failedOperations(), 0);
    else
      store->replace({});
    QCOMPARE(storage->writes, failBarrier ? 2 : 1);
    QCOMPARE(storage->erasures, 0);
    QVERIFY(!backend->visibleRegistryKey(std::string(app::kFileStagingRegistryRoot)));
    // Only after the clearing protocol succeeds may its Next dependency go.
    if (!removeThroughPlan)
      QVERIFY(backend->execute(compat::EditRegistryExclusion{std::string(app::kFileStagingRegistryRoot), compat::ExclusionAction::Remove}).succeeded());
    backend->simulateRestart();
    QVERIFY(store->load().isEmpty());
    QVERIFY(!backend->visibleRegistryKey(std::string(app::kFileStagingRegistryRoot)));
    QVERIFY(backend->snapshot().next.registryExclusions.empty());
  }
  void unrelatedControlSetExclusionsDoNotReadEnvironmentFacts_data() {
    QTest::addColumn<bool>("rootCovered");
    QTest::addColumn<bool>("reverse");
    QTest::addColumn<bool>("reconcile");
    for (const bool root : {false, true})
      for (const bool reverse : {false, true})
        for (const bool reconcile : {false, true})
          QTest::newRow(qPrintable(QStringLiteral("root-%1-reverse-%2-reconcile-%3").arg(root).arg(reverse).arg(reconcile))) << root << reverse << reconcile;
  }
  void unrelatedControlSetExclusionsDoNotReadEnvironmentFacts() {
    QFETCH(bool, rootCovered);
    QFETCH(bool, reverse);
    QFETCH(bool, reconcile);
    auto initial = protectedStagingState();
    initial.current.registryExclusions = {"HKLM\\SYSTEM\\CurrentControlSet\\Fixture", "HKLM\\SYSTEM\\ControlSet001\\Fixture"};
    if (rootCovered) initial.current.registryExclusions.push_back(std::string(app::kFileStagingRegistryRoot));
    if (reverse) std::ranges::reverse(initial.current.registryExclusions);
    initial.next = initial.current;
    const auto backend = std::make_shared<test::FakeUwfBackend>(initial);
    const auto storage = std::make_shared<SimulatedRegistryStorage>(backend);
    const QList<app::FileStagingEntry> original{{app::FileStagingKind::File, QStringLiteral("C:\\old.txt")}};
    const QList<app::FileStagingEntry> replacement{{app::FileStagingKind::File, QStringLiteral("C:\\new.txt")}};
    storage->seed(original);
    app::DurableFileStagingStore store(storage, app::uwfMaintenanceRepository(backend), app::UwfCapability::Available);
    if (reconcile)
      store.reconcileDurability();
    else
      store.replace(replacement);
    const auto trace = backend->trace();
    QCOMPARE(std::ranges::count_if(trace, [](const auto& entry) { return std::holds_alternative<compat::CommitRegistry>(entry.request); }),
             rootCovered ? 0 : 1);
    backend->simulateRestart();
    QCOMPARE(store.load(), reconcile ? original : replacement);
  }
  void listKeyCoverageDoesNotProveParentMetadataDurability() {
    auto initial = protectedStagingState();
    initial.current.registryExclusions = {std::string(app::kFileStagingRegistryKey)};
    initial.next = initial.current;
    const auto backend = std::make_shared<test::FakeUwfBackend>(initial);
    const auto storage = std::make_shared<SimulatedRegistryStorage>(backend);
    storage->seed({{app::FileStagingKind::File, QStringLiteral("C:\\old.txt")}});
    app::DurableFileStagingStore store(storage, app::uwfMaintenanceRepository(backend), app::UwfCapability::Available);
    try {
      store.replace({});
      QFAIL("List-key coverage cannot acknowledge deletion of its protected parent metadata");
    } catch (const app::OperationError& error) {
      QCOMPARE(error.writeEffect(), app::WriteEffect::Uncertain);
    }
    QCOMPARE(storage->writes, 1);
    QCOMPARE(storage->erasures, 0);
    QVERIFY(store.load().isEmpty());
    backend->simulateRestart();
    QVERIFY(store.load().isEmpty());
    QVERIFY(backend->visibleRegistryKey(std::string(app::kFileStagingRegistryRoot)));
  }
  void exclusionPrechecksCannotSubstituteForListDurability_data() {
    QTest::addColumn<int>("tails");
    QTest::addColumn<int>("operation");
    for (const int tails : {0, 1, 2})
      for (const auto operation : {StagingWrite::Replace, StagingWrite::Clear, StagingWrite::Reconcile})
        QTest::newRow(qPrintable(QStringLiteral("tails-%1-operation-%2").arg(tails).arg(int(operation)))) << tails << int(operation);
  }
  void exclusionPrechecksCannotSubstituteForListDurability() {
    QFETCH(int, tails);
    QFETCH(int, operation);
    auto initial = protectedStagingState();
    initial.current.registryExclusions = {std::string(app::kFileStagingRegistryRoot) + std::string(static_cast<std::size_t>(tails), '\\')};
    initial.next = initial.current;
    const auto backend = std::make_shared<test::FakeUwfBackend>(initial);
    const auto storage = std::make_shared<SimulatedRegistryStorage>(backend);
    const QList<app::FileStagingEntry> original{{app::FileStagingKind::File, QStringLiteral("C:\\old.txt")}};
    const QList<app::FileStagingEntry> replacement{{app::FileStagingKind::File, QStringLiteral("C:\\new.txt")}};
    storage->seed(original);
    app::DurableFileStagingStore store(storage, app::uwfMaintenanceRepository(backend), app::UwfCapability::Available);
    const auto write = [&] { applyStagingWrite(store, static_cast<StagingWrite>(operation), replacement); };
    if (tails == 1) {
      try {
        write();
        QFAIL("A management-only exclusion must not acknowledge persistent storage");
      } catch (const app::OperationError& error) {
        QCOMPARE(error.writeEffect(), app::WriteEffect::None);
        QVERIFY(QString::fromUtf8(error.what()).contains(QStringLiteral("reliable commit")));
      }
      QCOMPARE(storage->writes, 0);
      QCOMPARE(storage->erasures, 0);
      for (const auto& entry : backend->trace()) QVERIFY(!compat::isWrite(entry.request));
    } else {
      write();
    }
    backend->simulateRestart();
    const auto expected = tails == 1 || static_cast<StagingWrite>(operation) == StagingWrite::Reconcile ? original
                          : static_cast<StagingWrite>(operation) == StagingWrite::Clear                 ? QList<app::FileStagingEntry>{}
                                                                                                        : replacement;
    QCOMPARE(store.load(), expected);
  }
  void registryWriteAndHistoryAdmissionPrecedesStorageChanges_data() {
    QTest::addColumn<int>("mode");
    QTest::addColumn<bool>("excluded");
    QTest::addColumn<int>("operation");
    QTest::addColumn<bool>("systemProtected");
    for (const auto& [modeName, mode] : {std::pair{"ordinary", MaintenanceMode::Ordinary}, std::pair{"rom", MaintenanceMode::ReadOnlyMedia},
                                         std::pair{"horm", MaintenanceMode::Horm}, std::pair{"full-commit", MaintenanceMode::FullCommit}})
      for (const bool excluded : {false, true})
        for (const bool protectedSystem : {false, true})
          for (const auto& [operationName, operation] :
               {std::pair{"replace", StagingWrite::Replace}, std::pair{"clear", StagingWrite::Clear}, std::pair{"reconcile", StagingWrite::Reconcile}})
            QTest::newRow((std::string(modeName) + (excluded ? "/excluded/" : "/not-excluded/") +
                           (protectedSystem ? "system-protected/" : "only-D-protected/") + operationName)
                              .c_str())
                << int(mode) << excluded << int(operation) << protectedSystem;
  }
  void registryWriteAndHistoryAdmissionPrecedesStorageChanges() {
    QFETCH(int, mode);
    QFETCH(bool, excluded);
    QFETCH(int, operation);
    QFETCH(bool, systemProtected);
    const auto backend = stagingBackend(static_cast<MaintenanceMode>(mode), excluded, systemProtected);
    const auto storage = std::make_shared<SimulatedRegistryStorage>(backend);
    const QList<app::FileStagingEntry> original{{app::FileStagingKind::File, QStringLiteral("C:\\old.txt")}};
    const QList<app::FileStagingEntry> replacement{{app::FileStagingKind::File, QStringLiteral("C:\\new.txt")}};
    storage->seed(original);
    const auto before = backend->snapshot();
    app::DurableFileStagingStore durable(storage, app::uwfMaintenanceRepository(backend), app::UwfCapability::Available);
    const auto write = [&] { applyStagingWrite(durable, static_cast<StagingWrite>(operation), replacement); };
    const bool fullCommit = static_cast<MaintenanceMode>(mode) == MaintenanceMode::FullCommit;
    const bool historical = static_cast<StagingWrite>(operation) == StagingWrite::Reconcile;
    if (systemProtected && static_cast<MaintenanceMode>(mode) != MaintenanceMode::Ordinary && (!fullCommit || historical)) {
      QVERIFY_THROWS_EXCEPTION(app::OperationError, write());
      QCOMPARE(storage->writes, 0);
      QCOMPARE(storage->erasures, 0);
      QCOMPARE(durable.load(), original);
      const auto after = backend->snapshot();
      QCOMPARE(after.durable, before.durable);
      QCOMPARE(after.overlay, before.overlay);
      QCOMPARE(after.next, before.next);
      for (const auto& entry : backend->trace()) QVERIFY(!compat::isWrite(entry.request));
      backend->simulateRestart();
      QCOMPARE(durable.load(), original);
      return;
    }
    write();
    const auto expected = static_cast<StagingWrite>(operation) == StagingWrite::Replace ? replacement
                          : static_cast<StagingWrite>(operation) == StagingWrite::Clear ? QList<app::FileStagingEntry>{}
                                                                                        : original;
    QCOMPARE(durable.load(), expected);
    if (!systemProtected || fullCommit) {
      const auto trace = backend->trace();
      QVERIFY(std::ranges::none_of(trace, [](const auto& entry) {
        if (std::holds_alternative<compat::CommitRegistry>(entry.request) || std::holds_alternative<compat::CommitRegistryDeletion>(entry.request)) return true;
        if (const auto* boolean = std::get_if<compat::GetBoolean>(&entry.request)) return boolean->property == compat::BooleanProperty::HormEnabled;
        if (const auto* exclusions = std::get_if<compat::GetRegistryExclusions>(&entry.request)) return exclusions->session == compat::Session::Current;
        return false;
      }));
      QCOMPARE(storage->erasures, static_cast<StagingWrite>(operation) == StagingWrite::Clear ? 1 : 0);
    }
    backend->simulateRestart();
    QCOMPARE(durable.load(), expected);
  }
  void preparedStorageRechecksNewWritesBeforeMutating_data() {
    QTest::addColumn<bool>("clear");
    QTest::addColumn<bool>("horm");
    for (const bool clear : {false, true})
      for (const bool horm : {false, true})
        QTest::newRow((std::string(clear ? "clear/" : "replace/") + (horm ? "horm" : "full-commit")).c_str()) << clear << horm;
  }
  void preparedStorageRechecksNewWritesBeforeMutating() {
    QFETCH(bool, clear);
    QFETCH(bool, horm);
    test::application::UwfMaintenanceFixture fixture;
    const auto repository = fixture.repository();
    const auto storage = std::make_shared<SimulatedRegistryStorage>(fixture.backend);
    const QList<app::FileStagingEntry> original{{app::FileStagingKind::File, QStringLiteral("C:\\old.txt")}};
    const QList<app::FileStagingEntry> replacement{{app::FileStagingKind::File, QStringLiteral("C:\\new.txt")}};
    storage->seed(original);
    fixture.state->beforeRegistryTarget = [&] {
      auto state = fixture.backend->snapshot();
      state.current.overlayFlags = compat::flags::readOnlyMedia;
      state.runtime.hormEnabled = horm;
      state.runtime.romDisposition = horm ? compat::disposition::horm : compat::disposition::fullCommit;
      fixture.backend->seed(std::move(state));
    };
    app::DurableFileStagingStore durable(storage, repository, app::UwfCapability::Available);
    const auto write = [&] { durable.replace(clear ? QList<app::FileStagingEntry>{} : replacement); };
    if (horm) {
      QVERIFY_THROWS_EXCEPTION(app::OperationError, write());
      QCOMPARE(storage->writes, 0);
      QCOMPARE(storage->erasures, 0);
      QCOMPARE(durable.load(), original);
    } else {
      write();
      QCOMPARE(storage->writes, clear ? 0 : 1);
      QCOMPARE(storage->erasures, clear ? 1 : 0);
      fixture.backend->simulateRestart();
      QCOMPARE(durable.load(), clear ? QList<app::FileStagingEntry>{} : replacement);
    }
    QVERIFY(fixture.state->registryCommits.empty());
    QVERIFY(fixture.state->registryDeletions.empty());
    QVERIFY(fixture.backend->trace().empty());
  }
  void fullCommitDoesNotCertifyTheExistingStagingList() {
    const auto backend = protectedStagingBackend();
    const auto storage = std::make_shared<SimulatedRegistryStorage>(backend);
    const QList<app::FileStagingEntry> original{{app::FileStagingKind::File, QStringLiteral("C:\\old.txt")}};
    const QList<app::FileStagingEntry> pending{{app::FileStagingKind::File, QStringLiteral("C:\\pending.txt")}};
    storage->seed(original);
    storage->write(pending);
    storage->writes = 0;
    auto state = backend->snapshot();
    state.current.overlayFlags = compat::flags::readOnlyMedia;
    state.runtime.romDisposition = compat::disposition::fullCommit;
    state.next.registryExclusions.clear();
    backend->seed(state);
    app::DurableFileStagingStore durable(storage, app::uwfMaintenanceRepository(backend), app::UwfCapability::Available);
    QVERIFY_THROWS_EXCEPTION(app::OperationError, durable.reconcileDurability());
    QCOMPARE(storage->writes, 0);
    QCOMPARE(storage->erasures, 0);
    QCOMPARE(durable.load(), pending);
    QCOMPARE(backend->snapshot(), state);
    QVERIFY(backend->trace().empty());
    backend->simulateRestart();
    QCOMPARE(durable.load(), original);
  }
  void unavailableMaintenanceKeepsTheConfiguredStagingExclusion_data() {
    QTest::addColumn<int>("mode");
    QTest::newRow("rom") << int(MaintenanceMode::ReadOnlyMedia);
    QTest::newRow("horm") << int(MaintenanceMode::Horm);
  }
  void unusedGetterFailuresDoNotOverridePublishedAdmission_data() {
    QTest::addColumn<int>("observation");
    QTest::addColumn<int>("operation");
    for (const auto& [name, observation] :
         {std::pair{"registry-persistence", MaintenanceObservation::RegistryPersistence}, std::pair{"filter", MaintenanceObservation::FilterEnabled},
          std::pair{"horm", MaintenanceObservation::HormEnabled}, std::pair{"flags", MaintenanceObservation::OverlayFlags}})
      for (const auto& [operationName, operation] :
           {std::pair{"replace", StagingWrite::Replace}, std::pair{"clear", StagingWrite::Clear}, std::pair{"reconcile", StagingWrite::Reconcile}})
        QTest::newRow((std::string(name) + '/' + operationName).c_str()) << int(observation) << int(operation);
  }
  void unusedGetterFailuresDoNotOverridePublishedAdmission() {
    QFETCH(int, observation);
    QFETCH(int, operation);
    const auto backend = stagingBackend(MaintenanceMode::Ordinary, true);
    const auto storage = std::make_shared<SimulatedRegistryStorage>(backend);
    const QList<app::FileStagingEntry> original{{app::FileStagingKind::File, QStringLiteral("C:\\old.txt")}};
    storage->seed(original);
    const auto selected = static_cast<MaintenanceObservation>(observation);
    const compat::Request read =
        selected == MaintenanceObservation::RegistryPersistence ? compat::Request{compat::GetBoolean{compat::BooleanProperty::RegistryChangesAlwaysPersisted}}
        : selected == MaintenanceObservation::FilterEnabled     ? compat::Request{compat::GetBoolean{compat::BooleanProperty::FilterEnabled}}
        : selected == MaintenanceObservation::HormEnabled ? compat::Request{compat::GetBoolean{compat::BooleanProperty::HormEnabled}}
                                                          : compat::Request{compat::GetNumber{compat::NumberProperty::OverlayFlags, compat::Session::Current}};
    backend->inject({read, {}, test::FakeUwfBackend::FaultTiming::Before, compat::ErrorKind::Unavailable, "Maintenance state is unavailable"});
    app::DurableFileStagingStore durable(storage, app::uwfMaintenanceRepository(backend), app::UwfCapability::Available);
    const QList<app::FileStagingEntry> replacement{{app::FileStagingKind::File, QStringLiteral("C:\\new.txt")}};
    const auto selectedWrite = static_cast<StagingWrite>(operation);
    applyStagingWrite(durable, selectedWrite, replacement);
    QCOMPARE(storage->writes, selectedWrite == StagingWrite::Replace ? 1 : 0);
    QCOMPARE(storage->erasures, selectedWrite == StagingWrite::Clear ? 1 : 0);
    QCOMPARE(durable.load(), selectedWrite == StagingWrite::Replace ? replacement
                             : selectedWrite == StagingWrite::Clear ? QList<app::FileStagingEntry>{}
                                                                    : original);
    QVERIFY(backend->trace().empty());
  }
  void unknownPublishedAdmissionLeavesStorageUntouched_data() { unusedGetterFailuresDoNotOverridePublishedAdmission_data(); }
  void unknownPublishedAdmissionLeavesStorageUntouched() {
    QFETCH(int, observation);
    QFETCH(int, operation);
    const auto backend = stagingBackend(MaintenanceMode::Ordinary, true);
    const auto storage = std::make_shared<SimulatedRegistryStorage>(backend);
    const QList<app::FileStagingEntry> original{{app::FileStagingKind::File, QStringLiteral("C:\\old.txt")}};
    storage->seed(original);
    const auto before = backend->snapshot();
    const auto system = std::make_shared<app::PrecheckedUwf>(backend);
    auto published = system->state();
    switch (static_cast<MaintenanceObservation>(observation)) {
      case MaintenanceObservation::RegistryPersistence:
        published.systemVolumeId = {};
        break;
      case MaintenanceObservation::FilterEnabled:
        published.current.filterEnabled = {};
        break;
      case MaintenanceObservation::HormEnabled:
        published.runtime.horm = {};
        break;
      case MaintenanceObservation::OverlayFlags:
        published.current.overlayFlags = {};
        break;
    }
    system->updateState(std::move(published));
    app::DurableFileStagingStore durable(storage, app::uwfMaintenanceRepository(system), app::UwfCapability::Available);
    bool caught = false;
    try {
      applyStagingWrite(durable, static_cast<StagingWrite>(operation), {{app::FileStagingKind::File, QStringLiteral("C:\\new.txt")}});
    } catch (const app::OperationError& error) {
      caught = true;
      QCOMPARE(error.code(), std::make_error_code(std::errc::state_not_recoverable));
      QCOMPARE(error.writeEffect(), app::WriteEffect::None);
    }
    QVERIFY(caught);
    QCOMPARE(storage->writes, 0);
    QCOMPARE(storage->erasures, 0);
    QCOMPARE(durable.load(), original);
    QCOMPARE(backend->snapshot(), before);
    QVERIFY(backend->trace().empty());
  }
  void unavailableMaintenanceKeepsTheConfiguredStagingExclusion() {
    QFETCH(int, mode);
    const auto backend = stagingBackend(static_cast<MaintenanceMode>(mode), true);
    const auto storage = std::make_shared<SimulatedRegistryStorage>(backend);
    const QList<app::FileStagingEntry> original{{app::FileStagingKind::File, QStringLiteral("C:\\data.txt")}};
    storage->seed(original);
    const auto durable = std::make_shared<app::DurableFileStagingStore>(storage, app::uwfMaintenanceRepository(backend), app::UwfCapability::Available);
    const auto draft = std::make_shared<app::FileStagingDraft>(durable);
    QCOMPARE(draft->load(), original);
    draft->replace({});
    const auto snapshot = app::publishedUwfSnapshot(backend->state(), app::UwfCapability::Available, true, {});
    bool rejected = false;
    try {
      const auto repository = app::uwfConfigurationRepository(backend, draft);
      const auto result = app::ConfigurationExecutor(repository).apply(app::planConfiguration({}, snapshot, draft->changes()));
      QVERIFY(result.failed());
      QVERIFY(result.staging() && result.staging()->failure);
      QVERIFY(!result.anyWriteConfirmed());
      QVERIFY(!result.reconciliationRequired());
      std::rethrow_exception(result.staging()->failure);
    } catch (const app::OperationError& error) {
      rejected = true;
      QCOMPARE(error.failureKind(), app::OperationFailureKind::AccessDenied);
      QCOMPARE(error.writeEffect(), app::WriteEffect::None);
    }
    QVERIFY(rejected);
    QCOMPARE(durable->load(), original);
    QCOMPARE(backend->snapshot().next.registryExclusions, std::vector<std::string>{std::string(app::kFileStagingRegistryRoot)});
    for (const auto& entry : backend->trace()) QVERIFY(!compat::isWrite(entry.request));
    backend->simulateRestart();
    QCOMPARE(durable->load(), original);
  }
  void stagingAdmissionIsSharedByDirectAndHeadlessExecution_data() {
    QTest::addColumn<int>("mode");
    QTest::addColumn<bool>("excluded");
    QTest::addColumn<bool>("headless");
    for (const auto& [name, mode] :
         {std::pair{"ordinary", MaintenanceMode::Ordinary}, std::pair{"rom", MaintenanceMode::ReadOnlyMedia}, std::pair{"horm", MaintenanceMode::Horm}})
      for (const bool excluded : {false, true})
        for (const bool headless : {false, true})
          QTest::newRow((std::string(name) + (excluded ? "/excluded/" : "/protected/") + (headless ? "headless" : "direct")).c_str())
              << int(mode) << excluded << headless;
  }
  void stagingAdmissionIsSharedByDirectAndHeadlessExecution() {
    QFETCH(int, mode);
    QFETCH(bool, excluded);
    QFETCH(bool, headless);
    const auto backend = stagingBackend(static_cast<MaintenanceMode>(mode), excluded);
    constexpr auto volume = test::application::UwfMaintenanceFixture::volumeId;
    constexpr auto path = "\\data\\value.txt";
    backend->seedFile(volume, path, "base");
    backend->writeFile(volume, path, "changed");
    const auto before = backend->snapshot();
    const auto repository = app::uwfMaintenanceRepository(backend);
    const auto source = std::make_shared<StagedFileSource>();
    const QList<app::FileStagingEntry> entries{{app::FileStagingKind::File, QStringLiteral("C:\\data\\value.txt")}};
    app::FileStagingCommitResult result;
    const bool paused = static_cast<MaintenanceMode>(mode) != MaintenanceMode::Ordinary;
    if (headless) {
      app::FileStagingTask task(repository, entries, app::UwfCapability::Available, source);
      QVERIFY(!task.pollPreparation());
      task.waitForWorker();
      QVERIFY(task.pollPreparation());
      while (!task.finished()) {
        (void)task.advance();
        task.waitForWorker();
      }
      result = task.result();
    } else {
      app::FileStagingCommitter committer(repository, source);
      auto operation = committer.beginCommit(committer.prepare(app::FileStagingCommitter::scan(committer.prepareScan(entries))));
      while (!operation.finished()) (void)operation.advance();
      result = operation.result();
    }
    if (paused) {
      QVERIFY(result.succeeded());
      QCOMPARE(result.pausedEntries, std::size_t{1});
      QCOMPARE(result.committedFiles, std::size_t{0});
      QVERIFY(result.automaticCommit.paused());
      const auto after = backend->snapshot();
      QCOMPARE(after.durable, before.durable);
      QCOMPARE(after.overlay, before.overlay);
      for (const auto& entry : backend->trace()) QVERIFY(!compat::isWrite(entry.request));
    } else {
      QVERIFY(result.succeeded());
      QCOMPARE(result.committedFiles, excluded ? std::size_t{0} : std::size_t{1});
      QCOMPARE(result.skippedEntries, excluded ? std::size_t{1} : std::size_t{0});
      QCOMPARE(result.writeEffect(), excluded ? app::WriteEffect::None : app::WriteEffect::Confirmed);
      QCOMPARE(backend->readFile(volume, path), std::optional<std::string>{"changed"});
    }
  }
  void cleanupRetriesAcrossStoreRecreationNeverRestoreTheList_data() {
    QTest::addColumn<bool>("rootTarget");
    QTest::addColumn<int>("timing");
    for (const bool root : {false, true})
      for (const auto& [name, timing] :
           {std::pair{"before", test::FakeUwfBackend::FaultTiming::Before}, std::pair{"after-visible", test::FakeUwfBackend::FaultTiming::AfterVisibleDeletion},
            std::pair{"after-effect", test::FakeUwfBackend::FaultTiming::AfterEffect}})
        QTest::newRow(qPrintable(QStringLiteral("%1/%2").arg(root ? QStringLiteral("root") : QStringLiteral("list"), QString::fromLatin1(name))))
            << root << int(timing);
  }
  void cleanupRetriesAcrossStoreRecreationNeverRestoreTheList() {
    QFETCH(bool, rootTarget);
    QFETCH(int, timing);
    const auto faultTiming = static_cast<test::FakeUwfBackend::FaultTiming>(timing);
    const auto backend = protectedStagingBackend();
    const auto storage = std::make_shared<SimulatedRegistryStorage>(backend);
    const QList<app::FileStagingEntry> entries{{app::FileStagingKind::File, QStringLiteral("C:\\data.txt")}};
    storage->seed(entries);
    const compat::Request deletion =
        compat::CommitRegistryDeletion{std::string(rootTarget ? app::kFileStagingRegistryRoot : app::kFileStagingRegistryKey), std::nullopt};
    backend->inject({deletion, {}, faultTiming, compat::ErrorKind::AccessDenied, "Persistent staging deletion failed"});

    for (int attempt = 0; attempt < 2; ++attempt) {
      // Reopening must recover from persisted state without an in-memory cleanup journal.
      const auto durable = std::make_shared<app::DurableFileStagingStore>(storage, app::uwfMaintenanceRepository(backend), app::UwfCapability::Available);
      const auto draft = std::make_shared<app::FileStagingDraft>(durable);
      const auto before = draft->load();
      draft->replace({});
      const auto repository = app::uwfConfigurationRepository(backend, draft);
      const auto snapshot = app::publishedUwfSnapshot(backend->state(), app::UwfCapability::Available, true, {});

      const app::FileStagingChanges changes{before, {}, true};
      const auto result = app::ConfigurationExecutor(repository).apply(app::planConfiguration({}, snapshot, changes));
      QVERIFY(result.staging());
      if (attempt > 0) {
        QCOMPARE(result.failedOperations(), 0);
        QVERIFY(!result.staging()->failure);
        QVERIFY(backend->snapshot().next.registryExclusions.empty());
        break;
      }
      QVERIFY(result.failed());
      QVERIFY(result.staging()->failure);
      QVERIFY(storage->load().isEmpty());
      const auto persisted = backend->readRegistryValue(std::string(app::kFileStagingRegistryKey), std::string(app::kFileStagingEntriesValue));
      QVERIFY(!persisted || persisted->empty());
      QCOMPARE(backend->snapshot().next.registryExclusions, std::vector<std::string>{std::string(app::kFileStagingRegistryRoot)});
      for (const auto& entry : backend->trace()) QVERIFY(!std::holds_alternative<compat::EditRegistryExclusion>(entry.request));
    }
    backend->simulateRestart();
    QVERIFY(storage->load().isEmpty());
  }
  void emptyListCommitFailureStopsMetadataDeletionAndRetainsNextExclusion_data() {
    QTest::addColumn<bool>("afterEffect");
    QTest::newRow("before") << false;
    QTest::newRow("after-effect") << true;
  }
  void emptyListCommitFailureStopsMetadataDeletionAndRetainsNextExclusion() {
    QFETCH(bool, afterEffect);
    const auto backend = protectedStagingBackend();
    const auto storage = std::make_shared<SimulatedRegistryStorage>(backend);
    const QList<app::FileStagingEntry> entries{{app::FileStagingKind::File, QStringLiteral("C:\\data.txt")}};
    storage->seed(entries);
    {
      const auto durable = std::make_shared<app::DurableFileStagingStore>(storage, app::uwfMaintenanceRepository(backend), app::UwfCapability::Available);
      const auto draft = std::make_shared<app::FileStagingDraft>(durable);
      QCOMPARE(draft->load(), entries);
      draft->replace({});
      const auto repository = app::uwfConfigurationRepository(backend, draft);
      const compat::Request commit = compat::CommitRegistry{std::string(app::kFileStagingRegistryKey), std::string(app::kFileStagingEntriesValue)};
      backend->inject({commit,
                       {},
                       afterEffect ? test::FakeUwfBackend::FaultTiming::AfterEffect : test::FakeUwfBackend::FaultTiming::Before,
                       compat::ErrorKind::AccessDenied,
                       "Empty list could not be confirmed durable"});
      const auto snapshot = app::publishedUwfSnapshot(backend->state(), app::UwfCapability::Available, true, {});

      const auto result = app::ConfigurationExecutor(repository).apply(app::planConfiguration({}, snapshot, draft->changes()));
      QVERIFY(result.failed());
      QVERIFY(result.staging() && result.staging()->failure);
      QCOMPARE(backend->snapshot().next.registryExclusions, std::vector<std::string>{std::string(app::kFileStagingRegistryRoot)});
      for (const auto& entry : backend->trace()) {
        QVERIFY(!std::holds_alternative<compat::CommitRegistryDeletion>(entry.request));
        QVERIFY(!std::holds_alternative<compat::EditRegistryExclusion>(entry.request));
      }
    }
    // Reopening may retry the clear, but may not infer persistence from the
    // already-empty visible value left by the failed attempt.
    app::DurableFileStagingStore reopened(storage, app::uwfMaintenanceRepository(backend), app::UwfCapability::Available);
    reopened.replace({});
    backend->simulateRestart();
    QVERIFY(reopened.load().isEmpty());
  }
  void metadataFailureCanBeSupersededByDurableReplacement() {
    const auto backend = protectedStagingBackend();
    const auto storage = std::make_shared<SimulatedRegistryStorage>(backend);
    storage->seed({{app::FileStagingKind::File, QStringLiteral("C:\\old.txt")}});
    app::DurableFileStagingStore durable(storage, app::uwfMaintenanceRepository(backend), app::UwfCapability::Available);
    const compat::Request deletion = compat::CommitRegistryDeletion{std::string(app::kFileStagingRegistryKey), std::nullopt};
    backend->inject({deletion, {}, test::FakeUwfBackend::FaultTiming::AfterVisibleDeletion, compat::ErrorKind::AccessDenied, "Persistent deletion failed"});
    QVERIFY_THROWS_EXCEPTION(app::OperationError, durable.replace({}));
    QVERIFY(storage->load().isEmpty());
    const QList<app::FileStagingEntry> replacement{{app::FileStagingKind::File, QStringLiteral("C:\\new.txt")}};
    durable.replace(replacement);
    QCOMPARE(durable.load(), replacement);
    durable.replace({});
    backend->simulateRestart();
    QVERIFY(durable.load().isEmpty());
  }

  void persistenceFailuresAggregateEffectsWithoutLosingDiagnostics_data() {
    QTest::addColumn<int>("effect");
    QTest::addColumn<bool>("clear");
    for (const auto& [name, effect] :
         {std::pair{"none", app::WriteEffect::None}, std::pair{"confirmed", app::WriteEffect::Confirmed}, std::pair{"uncertain", app::WriteEffect::Uncertain}})
      for (const bool clear : {false, true}) QTest::newRow((std::string(name) + (clear ? "-clear" : "-preserve")).c_str()) << static_cast<int>(effect) << clear;
  }
  void persistenceFailuresAggregateEffectsWithoutLosingDiagnostics() {
    QFETCH(int, effect);
    QFETCH(bool, clear);
    test::application::UwfMaintenanceFixture fixture;
    auto storage = std::make_shared<SimulatedRegistryStorage>(fixture.backend);
    const QList<app::FileStagingEntry> original{{app::FileStagingKind::File, QStringLiteral("C:\\data.txt")}};
    storage->seed(original);
    const DetailedPersistenceFailure originalFailure(static_cast<app::WriteEffect>(effect));
    fixture.state->registryCommitFailure = std::make_exception_ptr(originalFailure);
    app::DurableFileStagingStore store(storage, fixture.repository(), app::UwfCapability::Available);
    bool caught = false;
    try {
      if (clear)
        store.replace({});
      else
        store.reconcileDurability();
    } catch (const app::OperationError& error) {
      caught = true;
      QCOMPARE(error.failureKind(), app::OperationFailureKind::NotFound);
      QCOMPARE(error.writeEffect(), app::WriteEffect::Uncertain);
      QCOMPARE(error.code(), originalFailure.code());
      QCOMPARE(QString::fromUtf8(error.what()), QString::fromUtf8(originalFailure.what()));
      QCOMPARE(error.diagnostic().code, originalFailure.diagnostic().code);
      QCOMPARE(error.diagnostic().result, originalFailure.diagnostic().result);
    }
    QVERIFY(caught);
    QCOMPARE(storage->load(), clear ? QList<app::FileStagingEntry>{} : original);
    QCOMPARE(fixture.state->registryTargets, 1);
    QCOMPARE(fixture.state->registryCommits.size(), std::size_t{1});
    QVERIFY(fixture.state->registryDeletions.empty());
    for (const auto& entry : fixture.backend->trace()) QVERIFY(!std::holds_alternative<compat::EditRegistryExclusion>(entry.request));
  }
  void reconciliationAggregatesCurrentAndNextWrites_data() {
    QTest::addColumn<bool>("currentCovered");
    QTest::addColumn<int>("failure");
    for (const bool covered : {false, true})
      for (const auto& [name, failure] : {std::pair{"next-read", ReconciliationFailure::NextRead}, std::pair{"next-write", ReconciliationFailure::NextWrite},
                                          std::pair{"after-next-write", ReconciliationFailure::AfterNextWrite}})
        QTest::newRow((std::string(covered ? "covered/" : "protected/") + name).c_str()) << covered << int(failure);
  }
  void reconciliationAggregatesCurrentAndNextWrites() {
    QFETCH(bool, currentCovered);
    QFETCH(int, failure);
    const auto stage = static_cast<ReconciliationFailure>(failure);
    const auto backend = stagingBackend(MaintenanceMode::Ordinary, currentCovered);
    auto state = backend->snapshot();
    state.next.registryExclusions.clear();
    backend->seed(std::move(state));
    const auto storage = std::make_shared<SimulatedRegistryStorage>(backend);
    const QList<app::FileStagingEntry> original{{app::FileStagingKind::File, QStringLiteral("C:\\data.txt")}};
    storage->seed(original);
    constexpr std::int64_t diagnosticCode = 0x80070005;
    const auto system = std::make_shared<app::PrecheckedUwf>(backend);
    if (stage == ReconciliationFailure::NextRead) {
      auto published = system->state();
      published.next.registryExclusions = {};
      system->updateState(std::move(published));
    } else {
      backend->inject(
          {compat::EditRegistryExclusion{std::string(app::kFileStagingRegistryRoot), compat::ExclusionAction::Add},
           {},
           stage == ReconciliationFailure::AfterNextWrite ? test::FakeUwfBackend::FaultTiming::AfterEffect : test::FakeUwfBackend::FaultTiming::Before,
           compat::ErrorKind::AccessDenied,
           "Next-session staging persistence failed",
           diagnosticCode});
    }
    app::DurableFileStagingStore store(storage, app::uwfMaintenanceRepository(system), app::UwfCapability::Available);
    bool caught = false;
    try {
      store.reconcileDurability();
    } catch (const app::OperationError& error) {
      caught = true;
      const bool attemptedWrite = !currentCovered || stage != ReconciliationFailure::NextRead;
      QCOMPARE(error.writeEffect(), attemptedWrite ? app::WriteEffect::Uncertain : app::WriteEffect::None);
      if (stage == ReconciliationFailure::NextRead) {
        QCOMPARE(error.code(), std::make_error_code(std::errc::state_not_recoverable));
        QVERIFY(QString::fromUtf8(error.what()).contains(QStringLiteral("global Next registry exclusions are unavailable")));
      } else {
        QCOMPARE(error.failureKind(), app::OperationFailureKind::AccessDenied);
        QCOMPARE(static_cast<std::uint32_t>(error.diagnostic().code), static_cast<std::uint32_t>(diagnosticCode));
        QVERIFY(QString::fromUtf8(error.what()).contains(QStringLiteral("Next-session staging persistence failed")));
      }
    }
    QVERIFY(caught);
    QCOMPARE(storage->load(), original);
    QCOMPARE(storage->writes, 0);
    QCOMPARE(storage->erasures, 0);
  }
  void failedClearKeepsNextExclusionThroughTheConfigurationExecutor_data() {
    QTest::addColumn<int>("effect");
    for (const auto& [name, effect] :
         {std::pair{"none", app::WriteEffect::None}, std::pair{"confirmed", app::WriteEffect::Confirmed}, std::pair{"uncertain", app::WriteEffect::Uncertain}})
      QTest::newRow(name) << static_cast<int>(effect);
  }
  void failedClearKeepsNextExclusionThroughTheConfigurationExecutor() {
    QFETCH(int, effect);
    test::application::UwfMaintenanceFixture fixture;
    auto storage = std::make_shared<SimulatedRegistryStorage>(fixture.backend);
    storage->seed({{app::FileStagingKind::File, QStringLiteral("C:\\data.txt")}});
    const auto maintenance = fixture.repository();
    auto state = fixture.backend->snapshot();
    state.next.registryExclusions = {std::string(app::kFileStagingRegistryRoot)};
    fixture.backend->seed(std::move(state));
    fixture.state->registryCommitFailure = std::make_exception_ptr(app::OperationError(
        std::make_error_code(std::errc::no_such_file_or_directory), "Persistence target was not found", static_cast<app::WriteEffect>(effect)));
    const auto durable = std::make_shared<app::DurableFileStagingStore>(storage, maintenance, app::UwfCapability::Available);
    const auto draft = std::make_shared<app::FileStagingDraft>(durable);
    static_cast<void>(draft->load());
    draft->replace({});
    const auto repository = app::uwfConfigurationRepository(fixture.backend, draft);
    const auto snapshot = app::publishedUwfSnapshot(fixture.backend->state(), app::UwfCapability::Available, true, {});

    const auto plan = app::planConfiguration({}, snapshot, draft->changes());
    QVERIFY(std::ranges::find(plan.changes.removeRegistryExclusions, std::string(app::kFileStagingRegistryRoot)) !=
            plan.changes.removeRegistryExclusions.end());
    const auto result = app::ConfigurationExecutor(repository).apply(plan);
    QVERIFY(result.failed());
    QVERIFY(result.staging() && result.staging()->failure);
    QVERIFY(storage->load().isEmpty());
    QCOMPARE(fixture.backend->snapshot().next.registryExclusions, std::vector<std::string>{std::string(app::kFileStagingRegistryRoot)});
    QVERIFY(fixture.state->registryDeletions.empty());
    for (const auto& entry : fixture.backend->trace()) QVERIFY(!std::holds_alternative<compat::EditRegistryExclusion>(entry.request));
  }
  void writesOnlyTheCurrentSession_data() {
    QTest::addColumn<bool>("filterEnabled");
    QTest::addColumn<bool>("covered");
    QTest::addColumn<bool>("clear");
    for (const bool enabled : {false, true})
      for (const bool covered : {false, true})
        for (const bool clear : {false, true})
          QTest::newRow(qPrintable(QStringLiteral("filter-%1-covered-%2-clear-%3").arg(enabled).arg(covered).arg(clear))) << enabled << covered << clear;
  }
  void writesOnlyTheCurrentSession() {
    QFETCH(bool, filterEnabled);
    QFETCH(bool, covered);
    QFETCH(bool, clear);
    test::application::UwfMaintenanceFixture fixture;
    auto storage = std::make_shared<SimulatedRegistryStorage>(fixture.backend);
    storage->seed({{app::FileStagingKind::File, QStringLiteral("C:\\data.txt")}});
    fixture.state->filterEnabled = filterEnabled;
    const auto repository = fixture.repository();
    auto configuration = fixture.backend->snapshot();
    if (covered) configuration.current.registryExclusions = {"HKEY_LOCAL_MACHINE\\SOFTWARE\\HsingYun"};
    const auto next = configuration.next;
    fixture.backend->seed(std::move(configuration));
    app::DurableFileStagingStore store(storage, repository, app::UwfCapability::Available);
    const auto target = clear ? QList<app::FileStagingEntry>{} : storage->load();
    const auto deletionPlan = storage->deletionPlan();
    store.replace(target);
    QCOMPARE(store.load(), target);
    const bool needsCommit = filterEnabled && !covered;
    QCOMPARE(fixture.state->registryCommits.size(), needsCommit ? std::size_t{1} : std::size_t{0});
    QCOMPARE(fixture.state->registryDeletions.size(), needsCommit && clear ? deletionPlan.size() : std::size_t{0});
    if (needsCommit && clear) {
      for (std::size_t index = 0; index < deletionPlan.size(); ++index) {
        QCOMPARE(fixture.state->registryDeletions[index].key, deletionPlan[index].key);
        QCOMPARE(fixture.state->registryDeletions[index].valueName, deletionPlan[index].deletionValueName());
      }
    }
    QCOMPARE(fixture.backend->snapshot().next, next);
    for (const auto& call : fixture.backend->trace()) QVERIFY(!std::holds_alternative<compat::EditRegistryExclusion>(call.request));
  }
};
QTEST_APPLESS_MAIN(FileStagingDurabilityTests)
#include "FileStagingDurabilityTests.moc"
