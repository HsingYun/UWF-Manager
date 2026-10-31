// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QTest>
#include <algorithm>

#include "app/configuration/ConfigurationExecutor.h"
#include "app/wmi/WmiException.h"
#include "support/app/RecordingWmiOperations.h"

using namespace uwf;
namespace {
app::wmi::WmiRow registryRow(bool domain, bool license) {
  return {{"__PATH", app::wmi::WmiValue::fromString("registry-next")},
          {"CurrentSession", app::wmi::WmiValue::fromBool(false)},
          {"PersistDomainSecretKey", app::wmi::WmiValue::fromBool(domain)},
          {"PersistTSCAL", app::wmi::WmiValue::fromBool(license)}};
}
enum class Outcome { Success, Rejected, Uncertain, Unverified, ReadFailed };
enum class Coverage { RetainedRoot, RetainedAncestor, PlannedRoot, PlannedAncestor, ClearList, Alternatives };
class Store final : public app::FileStagingStore {
 public:
  QList<app::FileStagingEntry> entries;
  int writes = 0;
  QList<app::FileStagingEntry> load() const override { return entries; }
  void replace(const QList<app::FileStagingEntry>& value) override {
    entries = value;
    ++writes;
  }
};
app::wmi::WmiRow volumeRow(bool current, bool protectedVolume = false) {
  using app::wmi::WmiValue;
  return {{"__PATH", WmiValue::fromString(current ? "volume-current" : "volume-next")},
          {"CurrentSession", WmiValue::fromBool(current)},
          {"DriveLetter", WmiValue::fromString("C:")},
          {"VolumeName", WmiValue::fromString("Volume{guid}")},
          {"BindByDriveLetter", WmiValue::fromBool(true)},
          {"CommitPending", WmiValue::fromBool(false)},
          {"Protected", WmiValue::fromBool(protectedVolume)}};
}
}  // namespace

class ConfigurationExecutionTests final : public QObject {
  Q_OBJECT
 private slots:
  void stagingCompletionDependsOnlyOnItsExclusion_data() {
    QTest::addColumn<Coverage>("coverage");
    QTest::addColumn<bool>("exclusionFails");
    QTest::addColumn<bool>("licenseFails");
    // Retained exact / ancestor coverage, planned exact / ancestor coverage,
    // and cleanup of the last staged entry share the same completion contract.
    for (const auto coverage :
         {Coverage::RetainedRoot, Coverage::RetainedAncestor, Coverage::PlannedRoot, Coverage::PlannedAncestor, Coverage::ClearList, Coverage::Alternatives})
      for (const bool exclusionFails : {false, true})
        for (const bool licenseFails : {false, true})
          QTest::newRow(qPrintable(
              QStringLiteral("coverage-%1-exclusion-fails-%2-license-fails-%3").arg(static_cast<int>(coverage)).arg(exclusionFails).arg(licenseFails)))
              << coverage << exclusionFails << licenseFails;
  }
  void stagingCompletionDependsOnlyOnItsExclusion() {
    QFETCH(Coverage, coverage);
    const bool needsExclusion = coverage != Coverage::RetainedRoot && coverage != Coverage::RetainedAncestor;
    const bool alternatives = coverage == Coverage::Alternatives;
    QFETCH(bool, exclusionFails);
    QFETCH(bool, licenseFails);
    auto store = std::make_shared<Store>();
    const QList<app::FileStagingEntry> entries{{app::FileStagingKind::File, QStringLiteral("C:\\data.txt")}};
    if (coverage == Coverage::ClearList) store->entries = entries;
    auto draft = std::make_shared<app::FileStagingDraft>(store);
    draft->replace(coverage == Coverage::ClearList ? QList<app::FileStagingEntry>{} : entries);
    core::UwfSnapshot snapshot;
    const std::string root(app::kFileStagingRegistryRoot);
    if (coverage == Coverage::RetainedRoot || coverage == Coverage::ClearList) snapshot.next.registryExclusions = {root};
    if (coverage == Coverage::RetainedAncestor) snapshot.next.registryExclusions = {"HKEY_LOCAL_MACHINE\\SOFTWARE"};
    core::PendingChanges changes;
    changes.setPersistTSCAL = true;
    if (coverage == Coverage::PlannedAncestor) changes.addRegistryExclusions = {"HKEY_LOCAL_MACHINE\\SOFTWARE"};
    if (alternatives) changes.addRegistryExclusions = {root, "HKEY_LOCAL_MACHINE\\SOFTWARE"};
    test::application::RecordingWmiOperations transport;
    transport.queryResults.push_back({registryRow(false, false)});
    transport.invocationObserver = [&] {
      const auto call = transport.invocations.size();
      const bool exclusion = needsExclusion && call < (alternatives ? 2u : 1u);
      if (exclusion ? exclusionFails && call == 0 : licenseFails)
        transport.invocationFailure = std::make_exception_ptr(app::wmi::WmiProviderError(5, "write", "rejected"));
    };
    if (needsExclusion && !exclusionFails) transport.readResults.push_back({{{"bFound", app::wmi::WmiValue::fromBool(coverage != Coverage::ClearList)}}, {}});
    if (alternatives) transport.readResults.push_back({{{"bFound", app::wmi::WmiValue::fromBool(true)}}, {}});
    transport.objectResults.push_back(registryRow(false, true));
    app::ConfigurationExecutor executor(transport, [](const std::string&) { return std::vector<std::string>{}; }, draft, draft->changes());
    const auto result = executor.apply(changes, snapshot);
    const bool pending = needsExclusion && exclusionFails && !alternatives;
    QVERIFY(result.staging);
    QCOMPARE(bool(result.staging->failure), pending);
    QCOMPARE(bool(draft->changes()), pending);
    if (pending) QVERIFY(draft->changes()->persistencePending);
    QCOMPARE(store->writes, 1);
    QCOMPARE(store->entries, coverage == Coverage::ClearList ? QList<app::FileStagingEntry>{} : entries);
    QCOMPARE(result.failedOperations, int(licenseFails) + int(needsExclusion && exclusionFails));
  }
  void registrationEffectsRemainDistinctFromSkippedDependentWrites_data() {
    QTest::addColumn<Outcome>("outcome");
    for (const auto outcome : {Outcome::Success, Outcome::Rejected, Outcome::Uncertain, Outcome::Unverified})
      QTest::newRow(qPrintable(QString::number(static_cast<int>(outcome)))) << outcome;
  }
  void registrationEffectsRemainDistinctFromSkippedDependentWrites() {
    QFETCH(Outcome, outcome);
    test::application::RecordingWmiOperations transport;
    transport.queryResults = {{volumeRow(true)}, {volumeRow(true)}};
    if (outcome == Outcome::Rejected) transport.invocationFailure = std::make_exception_ptr(app::wmi::WmiProviderError(5, "register", "rejected"));
    if (outcome == Outcome::Uncertain) transport.invocationFailure = std::make_exception_ptr(app::wmi::WmiInvocationUncertain("register", "lost reply"));
    if (outcome == Outcome::Success) {
      transport.objectResults.push_back(volumeRow(false));
      transport.objectResults.push_back(volumeRow(false, true));
    }
    core::PendingChanges changes;
    changes.volumeProtect["C:"] = true;
    app::ConfigurationExecutor executor(transport, [](const std::string&) { return std::vector<std::string>{}; });
    const auto result = executor.apply(changes, {});
    const auto registration = std::ranges::find_if(
        result.steps, [](const auto& step) { return step.operation && step.operation->kind == app::ConfigurationOperationKind::VolumeRegistration; });
    QVERIFY(registration != result.steps.end());
    QCOMPARE(registration->writeEffect, outcome == Outcome::Success    ? app::WriteEffect::Confirmed
                                        : outcome == Outcome::Rejected ? app::WriteEffect::None
                                                                       : app::WriteEffect::Uncertain);
    const auto protection = std::ranges::find_if(
        result.steps, [](const auto& step) { return step.operation && step.operation->kind == app::ConfigurationOperationKind::VolumeProtection; });
    QVERIFY(protection != result.steps.end());
    QCOMPARE(protection->outcome, outcome == Outcome::Success ? app::ConfigurationStepOutcome::Succeeded : app::ConfigurationStepOutcome::Skipped);
    QCOMPARE(protection->writeEffect, outcome == Outcome::Success ? app::WriteEffect::Confirmed : app::WriteEffect::None);
    QCOMPARE(result.reconciliationRequired, outcome != Outcome::Rejected);
    QCOMPARE(result.anyWriteConfirmed, outcome == Outcome::Success);
  }
  void groupedWriteReportsEveryRequestedOperation_data() {
    QTest::addColumn<int>("selection");
    QTest::addColumn<Outcome>("outcome");
    for (int selection = 1; selection <= 3; ++selection)
      for (const auto outcome : {Outcome::Success, Outcome::Rejected, Outcome::Uncertain, Outcome::Unverified, Outcome::ReadFailed})
        QTest::newRow(qPrintable(QStringLiteral("switches-%1-outcome-%2").arg(selection).arg(static_cast<int>(outcome)))) << selection << outcome;
  }
  void groupedWriteReportsEveryRequestedOperation() {
    QFETCH(int, selection);
    QFETCH(Outcome, outcome);
    core::PendingChanges changes;
    if (selection & 1) changes.setPersistDomainSecretKey = true;
    if (selection & 2) changes.setPersistTSCAL = true;
    test::application::RecordingWmiOperations transport;
    if (outcome != Outcome::ReadFailed) transport.queryResults.push_back({registryRow(false, false)});
    if (outcome == Outcome::Rejected) transport.invocationFailure = std::make_exception_ptr(app::wmi::WmiProviderError(5, "persistence", "rejected"));
    if (outcome == Outcome::Uncertain) transport.invocationFailure = std::make_exception_ptr(app::wmi::WmiInvocationUncertain("persistence", "lost response"));
    transport.objectResults.push_back(outcome == Outcome::Success ? registryRow(selection & 1, selection & 2) : registryRow(false, false));
    app::ConfigurationExecutor executor(transport, [](const std::string&) { return std::vector<std::string>{}; });
    const auto result = executor.apply(changes, {});
    const auto expectedOutcome = outcome == Outcome::Success      ? app::ConfigurationStepOutcome::Succeeded
                                 : outcome == Outcome::ReadFailed ? app::ConfigurationStepOutcome::Skipped
                                                                  : app::ConfigurationStepOutcome::Failed;
    const auto expectedEffect = outcome == Outcome::Success                                       ? app::WriteEffect::Confirmed
                                : outcome == Outcome::Uncertain || outcome == Outcome::Unverified ? app::WriteEffect::Uncertain
                                                                                                  : app::WriteEffect::None;
    std::vector<app::ConfigurationOperation> expectedOperations;
    if (selection & 1) expectedOperations.push_back({app::ConfigurationOperationKind::DomainPersistence, {}});
    if (selection & 2) expectedOperations.push_back({app::ConfigurationOperationKind::LicensePersistence, {}});
    QCOMPARE(std::ranges::count_if(result.steps, [](const auto& step) { return step.operation.has_value(); }),
             static_cast<std::ptrdiff_t>(expectedOperations.size()));
    for (const auto& operation : expectedOperations) {
      QCOMPARE(std::ranges::count_if(result.steps, [&](const auto& step) { return step.operation == operation; }), 1);
      const auto step = std::ranges::find_if(result.steps, [&](const auto& value) { return value.operation == operation; });
      QCOMPARE(step->outcome, expectedOutcome);
      QCOMPARE(step->writeEffect, expectedEffect);
    }
    QCOMPARE(transport.invocations.size(), outcome == Outcome::ReadFailed ? std::size_t{0} : std::size_t{1});
    QCOMPARE(result.anyWriteConfirmed, outcome == Outcome::Success);
    QCOMPARE(result.reconciliationRequired, expectedEffect != app::WriteEffect::None);
  }
};
QTEST_APPLESS_MAIN(ConfigurationExecutionTests)
#include "ConfigurationExecutionTests.moc"
