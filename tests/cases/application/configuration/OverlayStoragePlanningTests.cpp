// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QTest>
#include <algorithm>
#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "application/configuration/contracts/ConfigurationPlan.h"
#include "application/configuration/execution/CompositeConfigurationRepository.h"
#include "application/configuration/execution/ConfigurationExecutor.h"
#include "application/overlay/policy/OverlayPolicyRepository.h"
#include "application/overlay/policy/OverlayPolicyService.h"
#include "application/uwf/adapters/UwfConfigurationRepository.h"
#include "application/uwf/adapters/UwfOverlayMaintenance.h"
#include "application/uwf/adapters/UwfOverlayOperations.h"
#include "application/uwf/adapters/UwfReadState.h"
#include "support/uwf/FakeUwfBackend.h"

namespace {
using namespace uwf;
using namespace uwf::app;
using Fake = test::FakeUwfBackend;
constexpr auto storageVolume = "11111111-1111-1111-1111-111111111111";
constexpr auto oldStorageVolume = "22222222-2222-2222-2222-222222222222";
constexpr std::uint32_t opaqueFlag = 0x40;

std::vector<compat::Request> writes(const Fake& system) {
  std::vector<compat::Request> result;
  for (const auto& request : system.primitiveRequests())
    if (compat::isWrite(request)) result.push_back(request);
  return result;
}

compat::Request storageWrite() { return compat::CreateSwapfile{storageVolume}; }
compat::Request flagsWrite(const std::uint32_t flags) { return compat::SetNumber{compat::NumberProperty::OverlayFlags, flags}; }

const ConfigurationStepResult& receipt(const ConfigurationApplyResult& result, const ConfigurationOperationKind kind) {
  const auto found = std::ranges::find_if(result.steps, [&](const auto& step) { return step.operation && step.operation->kind == kind; });
  if (found == result.steps.end()) throw std::logic_error("Missing planned operation receipt");
  return *found;
}

int receiptCount(const ConfigurationApplyResult& result, const ConfigurationOperationKind kind) {
  return static_cast<int>(std::ranges::count_if(result.steps, [&](const auto& step) { return step.operation && step.operation->kind == kind; }));
}

struct ProductionFixture {
  std::shared_ptr<Fake> system;
  std::shared_ptr<OverlayPolicyService> policy;
  std::shared_ptr<OverlayMaintenanceActions> maintenance;
  std::shared_ptr<ConfigurationRepository> repository;

  ProductionFixture(const std::uint32_t type, const std::uint32_t flags, const std::uint32_t capacity = 4096) {
    Fake::State state;
    state.current.filterEnabled = state.next.filterEnabled = false;
    state.current.overlayType = state.next.overlayType = type;
    state.current.overlayFlags = state.next.overlayFlags = flags;
    state.current.maximumOverlaySize = state.next.maximumOverlaySize = capacity;
    state.current.warningThreshold = state.next.warningThreshold = 317;
    state.current.criticalThreshold = state.next.criticalThreshold = 613;
    state.runtime.controlDevicePresent = true;
    state.runtime.resetPersistentState = 0;
    state.current.volumes[storageVolume].volume = {storageVolume, "D:", false, false, {}};
    state.current.volumes[oldStorageVolume].volume = {oldStorageVolume, "E:", false, false, {}};
    state.current.volumes[oldStorageVolume].swapfileSize = 1;
    state.next.volumes = state.current.volumes;
    system = std::make_shared<Fake>(std::move(state));
    policy = std::make_shared<OverlayPolicyService>(std::make_shared<UwfOverlayOperations>(system));
    maintenance = uwfOverlayMaintenance(system);
    repository = std::make_shared<CompositeConfigurationRepository>(std::vector<std::shared_ptr<ConfigurationRepository>>{
        uwfConfigurationRepository(system), overlayPolicyRepository(policy)});
  }

  [[nodiscard]] ConfigurationPlan plan(const ConfigurationChanges& changes, const bool review = false) const {
    const auto snapshot = publishedUwfSnapshot(system->state(), UwfCapability::Available, true, {});
    return review ? reviewApplicationConfiguration(changes, snapshot, policy->snapshot(), {}, maintenance->snapshot())
                  : planApplicationConfiguration(changes, snapshot, policy->snapshot(), {}, maintenance->snapshot());
  }

  [[nodiscard]] ConfigurationApplyResult apply(const ConfigurationPlan& plan) const {
    return ConfigurationExecutor(repository, {}, maintenance).apply(plan);
  }
};

ConfigurationChanges selectStorage() {
  ConfigurationChanges changes;
  changes.maintenance.storageVolume = QString::fromLatin1(storageVolume);
  return changes;
}

// A faulty maintenance port acknowledges the request without performing it.
// Every other operation and every observation still comes from the production
// adapter over the memory backend; no host UWF or power operation is involved.
class UnfulfilledStorageAcknowledgement final : public OverlayMaintenanceActions {
 public:
  explicit UnfulfilledStorageAcknowledgement(std::shared_ptr<OverlayMaintenanceActions> real) : m_real(std::move(real)) {}
  int calls = 0;
  OverlayMaintenanceSnapshot observe(const std::stop_token stop) override { return m_real->observe(stop); }
  std::optional<OverlayMaintenanceSnapshot> snapshot() const override { return m_real->snapshot(); }
  OverlayMaintenanceResult createSwap(const QString&, std::stop_token) override {
    ++calls;
    return {*m_real->snapshot(), {}, {}, WriteEffect::None, true};
  }
  OverlayMaintenanceResult fullCommit(const std::stop_token stop) override { return m_real->fullCommit(stop); }
  OverlayMaintenanceResult setHorm(const bool enabled, const std::stop_token stop) override { return m_real->setHorm(enabled, stop); }
  OverlayMaintenanceResult setAllVolumesProtected(const bool enabled, const std::stop_token stop) override {
    return m_real->setAllVolumesProtected(enabled, stop);
  }

 private:
  std::shared_ptr<OverlayMaintenanceActions> m_real;
};

class OverlayStoragePlanningTests final : public QObject {
  Q_OBJECT
 private slots:
  void storageAndPolicyIntentsShareTheFinalProductionState_data() {
    QTest::addColumn<int>("initialType");
    QTest::addColumn<bool>("initialPersistent");
    QTest::addColumn<int>("options");
    QTest::addColumn<bool>("explicitDisk");
    for (const int type : {0, 1})
      for (const bool persistent : {false, true})
        for (const int options : {0, 1, 2, 3})
          for (const bool explicitDisk : {false, true}) {
            const auto row = QStringLiteral("%1/persistent-%2/options-%3/explicit-Disk-%4")
                                 .arg(type == 0 ? "RAM" : "Disk")
                                 .arg(persistent)
                                 .arg(options)
                                 .arg(explicitDisk);
            QTest::newRow(qPrintable(row)) << type << persistent << options << explicitDisk;
          }
  }
  void storageAndPolicyIntentsShareTheFinalProductionState() {
    QFETCH(int, initialType);
    QFETCH(bool, initialPersistent);
    QFETCH(int, options);
    QFETCH(bool, explicitDisk);
    const auto initialFlags = opaqueFlag | OverlayFlags::Passthrough | (initialPersistent ? OverlayFlags::Persistent : 0u);
    ProductionFixture fixture(static_cast<std::uint32_t>(initialType), initialFlags);
    const auto before = fixture.system->snapshot();
    auto changes = selectStorage();
    if (explicitDisk) changes.settings.setOverlay.type = core::OverlayType::Disk;
    if ((options & 1) != 0) changes.overlayPolicy.persistent = true;
    if ((options & 2) != 0) changes.overlayPolicy.passthrough = false;
    const auto result = fixture.apply(fixture.plan(changes));
    QVERIFY(!result.failed());

    // Native storage selection calls RAM before editing markers and Disk last.
    // RAM's equal-type shortcut preserves a pre-existing Persistent bit; a real
    // Disk-to-RAM transition clears bits 0x3. Unknown bits survive both paths.
    auto expectedFlags = initialType == 0 ? initialFlags | OverlayFlags::Passthrough
                                         : (initialFlags & ~(OverlayFlags::Passthrough | OverlayFlags::Persistent)) | OverlayFlags::Passthrough;
    std::vector<compat::Request> expected{storageWrite()};
    if (explicitDisk) expected.emplace_back(compat::SetNumber{compat::NumberProperty::OverlayType, 1});
    if (changes.overlayPolicy.passthrough) {
      expectedFlags &= ~OverlayFlags::Passthrough;
      expected.push_back(flagsWrite(expectedFlags));
    }
    if (changes.overlayPolicy.persistent) {
      expectedFlags |= OverlayFlags::Persistent;
      expected.push_back(flagsWrite(expectedFlags));
    }
    QCOMPARE(writes(*fixture.system), expected);
    const auto after = fixture.system->snapshot();
    QCOMPARE(after.next.overlayType, std::uint32_t{1});
    QCOMPARE(after.next.overlayFlags, expectedFlags);
    QCOMPARE(after.next.volumes.at(storageVolume).swapfileSize, std::uint32_t{1});
    QCOMPARE(after.next.volumes.at(oldStorageVolume).swapfileSize, std::uint32_t{0});
    QCOMPARE(after.current, before.current);
    QCOMPARE(after.next.warningThreshold, before.next.warningThreshold);
    QCOMPARE(after.next.criticalThreshold, before.next.criticalThreshold);
    QCOMPARE(after.runtime.resetPersistentState, before.runtime.resetPersistentState);
    QCOMPARE(after.runtime.persistentRollbackState, before.runtime.persistentRollbackState);
    const auto published = fixture.system->stateSnapshot();
    QCOMPARE(published->next.overlayType.value, std::optional<std::uint32_t>{1});
    QCOMPARE(published->next.overlayFlags.knownMask, std::numeric_limits<std::uint32_t>::max());
    QCOMPARE(published->next.overlayFlags.valueBits, expectedFlags);
    QCOMPARE(published->next.volumes.at(storageVolume).swapfileMarker.value, std::optional<std::uint32_t>{1});
    QCOMPARE(published->next.volumes.at(oldStorageVolume).swapfileMarker.value, std::optional<std::uint32_t>{0});
    QVERIFY(result.observedOverlayPolicy);
    QCOMPARE(result.observedOverlayPolicy->next.overlayType.value, std::optional<std::int64_t>{1});
    QCOMPARE(result.observedOverlayPolicy->next.flags.value, std::optional{OverlayFlags{expectedFlags}});
    QVERIFY(result.observedOverlayMaintenance);
    QCOMPARE(result.observedOverlayMaintenance->nextOverlayType, std::optional<std::int64_t>{1});
    QCOMPARE(result.observedOverlayMaintenance->nextFlags, std::optional{OverlayFlags{expectedFlags}});
    QCOMPARE(receiptCount(result, ConfigurationOperationKind::OverlayStorage), 1);
    QCOMPARE(receipt(result, ConfigurationOperationKind::OverlayStorage).outcome, ConfigurationStepOutcome::Succeeded);
    QCOMPARE(receiptCount(result, ConfigurationOperationKind::OverlayType), explicitDisk ? 1 : 0);
    if (explicitDisk) {
      const auto& type = receipt(result, ConfigurationOperationKind::OverlayType);
      QCOMPARE(type.outcome, ConfigurationStepOutcome::Succeeded);
    }
    if (changes.overlayPolicy.persistent)
      QCOMPARE(receipt(result, ConfigurationOperationKind::OverlayPersistent).outcome, ConfigurationStepOutcome::Succeeded);
    if (changes.overlayPolicy.passthrough)
      QCOMPARE(receipt(result, ConfigurationOperationKind::OverlayPassthrough).outcome, ConfigurationStepOutcome::Succeeded);
  }

  void explicitRomRemovalPrecedesImplicitDiskStorage_data() {
    QTest::addColumn<int>("initialType");
    QTest::newRow("RAM") << 0;
    QTest::newRow("Disk") << 1;
  }
  void explicitRomRemovalPrecedesImplicitDiskStorage() {
    QFETCH(int, initialType);
    const auto initialFlags = opaqueFlag | OverlayFlags::ReadOnlyMedia;
    ProductionFixture fixture(static_cast<std::uint32_t>(initialType), initialFlags);
    auto changes = selectStorage();
    changes.overlayPolicy.readOnlyMedia = false;
    changes.overlayPolicy.persistent = true;
    const auto result = fixture.apply(fixture.plan(changes));
    QVERIFY(!result.failed());
    const std::vector<compat::Request> expected{flagsWrite(opaqueFlag), storageWrite(),
                                               flagsWrite(opaqueFlag | OverlayFlags::Passthrough | OverlayFlags::Persistent)};
    QCOMPARE(writes(*fixture.system), expected);
    QCOMPARE(fixture.system->snapshot().next.overlayType, std::uint32_t{1});
    QCOMPARE(fixture.system->snapshot().next.overlayFlags, opaqueFlag | OverlayFlags::Passthrough | OverlayFlags::Persistent);
    QCOMPARE(receipt(result, ConfigurationOperationKind::OverlayReadOnlyMedia).outcome, ConfigurationStepOutcome::Succeeded);
    QCOMPARE(receiptCount(result, ConfigurationOperationKind::OverlayType), 0);
  }

  void storageAndExplicitRamConflictRejectsTheWholeBatchBeforeWrites() {
    ProductionFixture fixture(1, opaqueFlag | OverlayFlags::Passthrough);
    const auto before = fixture.system->snapshot();
    auto changes = selectStorage();
    changes.settings.setOverlay.type = core::OverlayType::RAM;
    changes.settings.setOverlay.warningThresholdMb = 318;
    const auto reviewed = fixture.plan(changes, true);
    bool rejected = false;
    try {
      rejected = fixture.apply(reviewed).failed();
    } catch (const OperationError&) {
      rejected = true;
    }
    QVERIFY(rejected);
    QVERIFY(writes(*fixture.system).empty());
    QCOMPARE(fixture.system->snapshot(), before);
  }

  void explicitCapacityIncreasePrecedesStorageSelection() {
    ProductionFixture fixture(0, opaqueFlag, 768);
    auto changes = selectStorage();
    changes.settings.setOverlay.maximumSizeMb = 2048;
    changes.overlayPolicy.persistent = true;
    const auto result = fixture.apply(fixture.plan(changes));
    QVERIFY(!result.failed());
    const std::vector<compat::Request> expected{compat::SetNumber{compat::NumberProperty::MaximumOverlaySize, 2048}, storageWrite(),
                                               flagsWrite(opaqueFlag | OverlayFlags::Passthrough | OverlayFlags::Persistent)};
    QCOMPARE(writes(*fixture.system), expected);
    QCOMPARE(fixture.system->snapshot().next.maximumOverlaySize, std::uint32_t{2048});
    QCOMPARE(fixture.system->snapshot().next.overlayType, std::uint32_t{1});
    QCOMPARE(receipt(result, ConfigurationOperationKind::OverlaySize).outcome, ConfigurationStepOutcome::Succeeded);
    QCOMPARE(receipt(result, ConfigurationOperationKind::OverlayStorage).outcome, ConfigurationStepOutcome::Succeeded);
    QCOMPARE(receipt(result, ConfigurationOperationKind::OverlayPersistent).outcome, ConfigurationStepOutcome::Succeeded);
  }

  void storageSelectionKeepsItsOwnCapacityContract_data() {
    QTest::addColumn<bool>("initialDisk");
    QTest::addColumn<quint32>("requestedSize");
    QTest::addColumn<bool>("explicitDisk");
    QTest::addColumn<bool>("allowed");
    QTest::newRow("RAM512-storage-only") << false << quint32{0} << false << true;
    QTest::newRow("RAM512-size768-storage") << false << quint32{768} << false << true;
    QTest::newRow("Disk-explicit-size512-storage") << true << quint32{512} << false << false;
    QTest::newRow("RAM512-storage-explicit-Disk") << false << quint32{0} << true << false;
    QTest::newRow("RAM512-size2048-storage-explicit-Disk") << false << quint32{2048} << true << true;
  }
  void storageSelectionKeepsItsOwnCapacityContract() {
    QFETCH(bool, initialDisk);
    QFETCH(quint32, requestedSize);
    QFETCH(bool, explicitDisk);
    QFETCH(bool, allowed);
    ProductionFixture fixture(initialDisk ? 1u : 0u, opaqueFlag, initialDisk ? 2048u : 512u);
    const auto before = fixture.system->snapshot();
    auto changes = selectStorage();
    if (requestedSize) changes.settings.setOverlay.maximumSizeMb = requestedSize;
    if (explicitDisk) changes.settings.setOverlay.type = core::OverlayType::Disk;
    const auto review = fixture.plan(changes, true);
    if (!allowed) {
      QVERIFY_THROWS_EXCEPTION(std::invalid_argument, static_cast<void>(fixture.plan(changes)));
      QVERIFY_THROWS_EXCEPTION(std::invalid_argument, static_cast<void>(configurationPhases(review)));
      ConfigurationExecutor executor(fixture.repository, {}, fixture.maintenance);
      QVERIFY_THROWS_EXCEPTION(std::invalid_argument, static_cast<void>(executor.prepareApply(review)));
      QVERIFY(writes(*fixture.system).empty());
      QVERIFY(fixture.system->snapshot() == before);
      return;
    }

    const auto plan = fixture.plan(changes);
    const auto target = configurationOverlayTarget(plan).overlay;
    const auto capacity = requestedSize ? requestedSize : std::uint32_t{512};
    QCOMPARE(target.type, core::OverlayType::Disk);
    QCOMPARE(target.maximumSizeMb, capacity);
    const auto result = fixture.apply(plan);
    QVERIFY(!result.failed());
    std::vector<compat::Request> expectedWrites;
    std::vector<ConfigurationOperationKind> expectedReceipts;
    if (requestedSize) {
      expectedWrites.emplace_back(compat::SetNumber{compat::NumberProperty::MaximumOverlaySize, requestedSize});
      expectedReceipts.push_back(ConfigurationOperationKind::OverlaySize);
    }
    expectedWrites.push_back(storageWrite());
    expectedReceipts.push_back(ConfigurationOperationKind::OverlayStorage);
    if (explicitDisk) {
      expectedWrites.emplace_back(compat::SetNumber{compat::NumberProperty::OverlayType, 1});
      expectedReceipts.push_back(ConfigurationOperationKind::OverlayType);
    }
    QCOMPARE(writes(*fixture.system), expectedWrites);
    std::vector<ConfigurationOperationKind> observedReceipts;
    for (const auto& step : result.steps)
      if (step.operation) {
        QCOMPARE(step.outcome, ConfigurationStepOutcome::Succeeded);
        observedReceipts.push_back(step.operation->kind);
      }
    QCOMPARE(observedReceipts, expectedReceipts);
    QCOMPARE(result.batchFailures(), 0);
    QCOMPARE(result.writeEffect(), WriteEffect::Confirmed);
    const auto after = fixture.system->snapshot();
    QCOMPARE(after.next.overlayType, std::uint32_t{1});
    QCOMPARE(after.next.maximumOverlaySize, capacity);
    QCOMPARE(after.next.volumes.at(storageVolume).swapfileSize, std::uint32_t{1});
    QVERIFY(after.current == before.current);
  }

  void storageFailureStopsFollowingFlagWritesAndRetainsItsReceipt_data() {
    QTest::addColumn<bool>("afterEffect");
    QTest::newRow("before") << false;
    QTest::newRow("after-effect") << true;
  }
  void storageFailureStopsFollowingFlagWritesAndRetainsItsReceipt() {
    QFETCH(bool, afterEffect);
    ProductionFixture fixture(1, opaqueFlag | OverlayFlags::Passthrough);
    auto changes = selectStorage();
    changes.overlayPolicy.persistent = true;
    fixture.system->inject({.request = storageWrite(),
                            .operation = {},
                            .timing = afterEffect ? Fake::FaultTiming::AfterEffect : Fake::FaultTiming::Before,
                            .kind = compat::ErrorKind::Failed,
                            .message = "storage selection fault",
                            .code = 0});
    const auto result = fixture.apply(fixture.plan(changes));
    QVERIFY(result.failed());
    QCOMPARE(writes(*fixture.system), std::vector<compat::Request>{storageWrite()});
    const auto& storage = receipt(result, ConfigurationOperationKind::OverlayStorage);
    QCOMPARE(storage.outcome, ConfigurationStepOutcome::Failed);
    QCOMPARE(storage.writeEffect, afterEffect ? WriteEffect::Uncertain : WriteEffect::None);
    QVERIFY(storage.failure);
    QCOMPARE(receipt(result, ConfigurationOperationKind::OverlayPersistent).outcome, ConfigurationStepOutcome::NotAttempted);
    QCOMPARE(result.reconciliationRequired(), afterEffect);
  }

  void flagFailureKeepsTheSuccessfulStorageReceiptAndStopsRemainingFlags_data() {
    QTest::addColumn<bool>("afterEffect");
    QTest::newRow("before") << false;
    QTest::newRow("after-effect") << true;
  }
  void flagFailureKeepsTheSuccessfulStorageReceiptAndStopsRemainingFlags() {
    QFETCH(bool, afterEffect);
    ProductionFixture fixture(1, opaqueFlag | OverlayFlags::Passthrough);
    auto changes = selectStorage();
    changes.overlayPolicy.passthrough = false;
    changes.overlayPolicy.persistent = true;
    fixture.system->inject({.request = flagsWrite(opaqueFlag),
                            .operation = {},
                            .timing = afterEffect ? Fake::FaultTiming::AfterEffect : Fake::FaultTiming::Before,
                            .kind = compat::ErrorKind::Failed,
                            .message = "overlay option fault",
                            .code = 0});
    const auto result = fixture.apply(fixture.plan(changes));
    QVERIFY(result.failed());
    const std::vector<compat::Request> expected{storageWrite(), flagsWrite(opaqueFlag)};
    QCOMPARE(writes(*fixture.system), expected);
    const auto& storage = receipt(result, ConfigurationOperationKind::OverlayStorage);
    QCOMPARE(storage.outcome, ConfigurationStepOutcome::Succeeded);
    QCOMPARE(storage.writeEffect, WriteEffect::Confirmed);
    const auto& passthrough = receipt(result, ConfigurationOperationKind::OverlayPassthrough);
    QCOMPARE(passthrough.outcome, ConfigurationStepOutcome::Failed);
    QCOMPARE(passthrough.writeEffect, afterEffect ? WriteEffect::Uncertain : WriteEffect::None);
    QVERIFY(passthrough.policyFailure);
    QCOMPARE(receipt(result, ConfigurationOperationKind::OverlayPersistent).outcome, ConfigurationStepOutcome::NotAttempted);
    QVERIFY(result.reconciliationRequired());
    QVERIFY(result.anyWriteConfirmed());
    QCOMPARE(fixture.system->snapshot().next.volumes.at(storageVolume).swapfileSize, std::uint32_t{1});
    QCOMPARE(fixture.system->snapshot().next.overlayFlags, afterEffect ? opaqueFlag : opaqueFlag | OverlayFlags::Passthrough);
  }

  void anAcknowledgedStoragePhaseCannotHideAnUnfulfilledFinalTarget_data() {
    QTest::addColumn<int>("initialType");
    QTest::newRow("RAM/type-and-host-unfulfilled") << 0;
    QTest::newRow("Disk/only-host-unfulfilled") << 1;
  }
  void anAcknowledgedStoragePhaseCannotHideAnUnfulfilledFinalTarget() {
    QFETCH(int, initialType);
    ProductionFixture fixture(static_cast<std::uint32_t>(initialType), opaqueFlag | OverlayFlags::Passthrough);
    auto untrusted = std::make_shared<UnfulfilledStorageAcknowledgement>(fixture.maintenance);
    fixture.maintenance = untrusted;
    const auto result = fixture.apply(fixture.plan(selectStorage()));
    QCOMPARE(untrusted->calls, 1);
    QVERIFY(result.failed());
    QVERIFY(std::ranges::any_of(result.steps, [](const auto& step) { return step.outcome == ConfigurationStepOutcome::VerificationFailed; }));
    QCOMPARE(receipt(result, ConfigurationOperationKind::OverlayStorage).outcome, ConfigurationStepOutcome::Succeeded);
    QCOMPARE(fixture.system->snapshot().next.overlayType, static_cast<std::uint32_t>(initialType));
    QCOMPARE(fixture.system->snapshot().next.overlayFlags, opaqueFlag | OverlayFlags::Passthrough);
    QCOMPARE(fixture.system->snapshot().next.volumes.at(storageVolume).swapfileSize, std::uint32_t{0});
    QCOMPARE(fixture.system->snapshot().next.volumes.at(oldStorageVolume).swapfileSize, std::uint32_t{1});
    QVERIFY(writes(*fixture.system).empty());
  }
};
}  // namespace

QTEST_APPLESS_MAIN(OverlayStoragePlanningTests)
#include "OverlayStoragePlanningTests.moc"
