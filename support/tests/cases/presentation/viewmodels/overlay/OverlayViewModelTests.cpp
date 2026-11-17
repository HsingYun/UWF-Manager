// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QCoreApplication>
#include <QScopeGuard>
#include <QSignalSpy>
#include <QTest>
#include <QThreadPool>
#include <QTimer>
#include <algorithm>
#include <atomic>
#include <cstddef>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include "cases/presentation/viewmodels/common/ViewModelStateFixture.h"
#include "presentation/viewmodels/configuration/GlobalConfigurationViewModel.h"
#include "presentation/viewmodels/overlay/OverlayViewModel.h"
#include "support/app/WorkerGate.h"

namespace {
using namespace uwf;
using namespace uwf::test::viewmodels;

class FakeUsageReader final : public app::OverlayUsageReader {
 public:
  app::OverlayUsageState next{app::OverlayUsageDisabled{}};
  mutable int calls = 0;
  bool fail = false;
  app::OverlayUsageState read() const override {
    ++calls;
    if (fail) throw std::runtime_error("incomplete read");
    return next;
  }
};
class ControlledUsageReader final : public app::OverlayUsageReader {
 public:
  struct Read {
    std::shared_ptr<test::WorkerGate> gate;
    app::OverlayUsageState result;
  };
  std::vector<Read> reads;
  mutable std::atomic_int calls{0};
  app::OverlayUsageState read() const override {
    const auto call = calls.fetch_add(1);
    if (call >= static_cast<int>(reads.size())) throw std::logic_error("unexpected usage read");
    const auto& read = reads[static_cast<std::size_t>(call)];
    // Model a synchronous provider which cannot be interrupted by the caller.
    if (read.gate) read.gate->wait({}, false);
    return read.result;
  }
};
app::OverlayUsageState usageWithConsumption(const std::uint32_t used) {
  auto state = snapshot();
  state.runtime.currentConsumptionMb = used;
  return app::OverlayUsageEnabled{state.runtime, state.current.overlay};
}
class OverlayViewModelTests final : public QObject {
  Q_OBJECT
 private slots:
  void currentUsageIsIndependentOfPendingConfiguration();
  void currentUsageUsesObservedCapacity();
  void configurationProjectionPreservesTheLatestSampleAndItsObservation();
  void configurationProjectionKeepsAnInFlightPollAndItsLatestConfig();
  void aProjectionWithoutASampleStillHonorsAvailabilityTransitions();
  void overlayPublishesOnlyCompleteSuccessfulReads_data();
  void overlayPublishesOnlyCompleteSuccessfulReads();
  void observationPauseSuppressesAutomaticAndManualReadsUntilTheFinalLease();
  void observationPauseDiscardsOldResultsWithoutJoiningOtherWork();
  void observationPauseLeaseCanOutliveItsOwner();
  void releasingAnObservationPauseRespectsTheLatestSnapshot_data();
  void releasingAnObservationPauseRespectsTheLatestSnapshot();
};

void OverlayViewModelTests::configurationProjectionPreservesTheLatestSampleAndItsObservation() {
  auto reader = std::make_shared<FakeUsageReader>();
  presentation::OverlayViewModel model(reader);
  auto state = snapshot();
  state.runtime.currentConsumptionMb = 100;
  model.applySnapshot(state);
  reader->next = usageWithConsumption(300);
  model.refreshUsage();
  QTRY_VERIFY(!model.refreshing());
  reader->fail = true;
  model.refreshUsage();
  QTRY_VERIFY(!model.refreshing());
  const auto previous = std::get<app::OverlayUsageEnabled>(model.usage());
  QVERIFY(previous.error && previous.lastSuccess);
  state.current.overlay.warningThresholdMb = 777;
  state.current.overlay.criticalThresholdMb = 999;
  model.applySnapshot(state, false);
  const auto projected = std::get<app::OverlayUsageEnabled>(model.usage());
  QCOMPARE(projected.runtime.currentConsumptionMb, std::uint32_t{300});
  QCOMPARE(projected.config.warningThresholdMb, std::uint32_t{777});
  QCOMPARE(projected.config.criticalThresholdMb, std::uint32_t{999});
  QCOMPARE(projected.lastSuccess, previous.lastSuccess);
  QCOMPARE(projected.error, previous.error);
  QCOMPARE(reader->calls, 2);
}

void OverlayViewModelTests::configurationProjectionKeepsAnInFlightPollAndItsLatestConfig() {
  auto gate = std::make_shared<test::WorkerGate>();
  const auto release = qScopeGuard([gate] { gate->release(); });
  auto reader = std::make_shared<ControlledUsageReader>();
  reader->reads = {{gate, usageWithConsumption(456)}};
  presentation::OverlayViewModel model(reader);
  auto state = snapshot();
  model.applySnapshot(state);
  model.refreshUsage();
  QTRY_COMPARE(gate->started.load(), 1);
  QVERIFY(model.refreshing());
  state.runtime.currentConsumptionMb = 17;
  state.current.overlay.warningThresholdMb = 777;
  state.current.overlay.criticalThresholdMb = 999;
  model.applySnapshot(state, false);
  QVERIFY(model.refreshing());
  QCOMPARE(gate->finished.load(), 0);
  gate->release();
  QTRY_VERIFY(!model.refreshing());
  const auto observed = std::get<app::OverlayUsageEnabled>(model.usage());
  QCOMPARE(observed.runtime.currentConsumptionMb, std::uint32_t{456});
  QCOMPARE(observed.config.warningThresholdMb, std::uint32_t{777});
  QCOMPARE(observed.config.criticalThresholdMb, std::uint32_t{999});
  QVERIFY(observed.lastSuccess && !observed.error);
  QCOMPARE(reader->calls.load(), 1);
}

void OverlayViewModelTests::aProjectionWithoutASampleStillHonorsAvailabilityTransitions() {
  auto reader = std::make_shared<FakeUsageReader>();
  presentation::OverlayViewModel model(reader);
  auto state = snapshot();
  model.applySnapshot(state);
  const auto lastSuccess = app::overlayUsageObservation(model.usage()).lastSuccess;
  state.current.filter.enabled = false;
  model.applySnapshot(state, false);
  QVERIFY(std::holds_alternative<app::OverlayUsageDisabled>(model.usage()));
  QCOMPARE(app::overlayUsageObservation(model.usage()).lastSuccess, lastSuccess);
  state.uwfAvailable = false;
  model.applySnapshot(state, false);
  QVERIFY(std::holds_alternative<app::OverlayUsageUnavailable>(model.usage()));
  QCOMPARE(app::overlayUsageObservation(model.usage()).lastSuccess, lastSuccess);
  presentation::OverlayViewModel initial(reader);
  state.uwfAvailable = true;
  state.current.filter.enabled = true;
  initial.applySnapshot(state, false);
  QVERIFY(std::holds_alternative<app::OverlayUsageUnavailable>(initial.usage()));
  QVERIFY(!app::overlayUsageObservation(initial.usage()).lastSuccess);
  QCOMPARE(reader->calls, 0);
}

void OverlayViewModelTests::currentUsageIsIndependentOfPendingConfiguration() {
  presentation::GlobalConfigurationViewModel model(16384);
  auto current = snapshot().current;
  current.overlay = {core::OverlayType::RAM, 8192, 100, 200};
  model.setCurrentSession(current, {8000, 192});
  model.reset(current);
  QSignalSpy changes(&model, &presentation::GlobalConfigurationViewModel::usageChanged);
  QCOMPARE(model.usage().severity, core::OverlayUsageSeverity::Warning);
  const auto original = model.usage();
  model.importOverlayWarnMb(400);
  model.importOverlayCritMb(600);
  model.finishImport();
  QCOMPARE(model.usage().warningLegend, original.warningLegend);
  QCOMPARE(model.usage().criticalLegend, original.criticalLegend);
  QCOMPARE(model.usage().severity, original.severity);
  const auto pending = model.pendingOverlay();
  model.applyUsageState(app::OverlayUsageEnabled{{7992, 200}, current.overlay});
  QCOMPARE(changes.count(), 1);
  QCOMPARE(model.usage().severity, core::OverlayUsageSeverity::Critical);
  QVERIFY(!model.usage().warningText.isEmpty());
  QCOMPARE(model.pendingOverlay().warningThresholdMb, pending.warningThresholdMb);
  QCOMPARE(model.pendingOverlay().criticalThresholdMb, pending.criticalThresholdMb);
  const auto maximum = std::numeric_limits<uint32_t>::max();
  model.applyUsageState(app::OverlayUsageEnabled{{maximum, maximum}, current.overlay});
  QCOMPARE(model.usage().valueText, QStringLiteral("4,294,967,295 / 8,589,934,590 MB"));
  QCOMPARE(model.usage().percentageText, QStringLiteral("50%"));
  QCOMPARE(model.usage().totalMb, uint64_t{8589934590});
  auto liveConfig = current.overlay;
  liveConfig.warningThresholdMb = 1000;
  liveConfig.criticalThresholdMb = 2000;
  model.applyUsageState(app::OverlayUsageEnabled{{7800, 392}, liveConfig});
  QCOMPARE(model.usage().severity, core::OverlayUsageSeverity::Normal);
  QCOMPARE(model.usage().warningMb, uint32_t{1000});
  QCOMPARE(model.pendingOverlay().warningThresholdMb, pending.warningThresholdMb);
  model.applyUsageState(app::OverlayUsageDisabled{});
  QVERIFY(!model.usage().enabled && model.usage().warningText.isEmpty());
  QCOMPARE(model.pendingOverlay().criticalThresholdMb, pending.criticalThresholdMb);
  current.filter.enabled = false;
  model.setCurrentSession(current, {});
  QVERIFY(!model.usage().enabled && model.usage().warningText.isEmpty());
  QCOMPARE(model.usage().percentageText, QStringLiteral("—"));
  const auto beforeUnavailable = changes.count();
  model.setAvailable(false);
  QCOMPARE(changes.count(), beforeUnavailable + 1);
  QVERIFY(!model.usage().enabled && model.usage().warningText.isEmpty());
  QCOMPARE(model.usage().valueText, QStringLiteral("Usage unavailable"));
}

void OverlayViewModelTests::currentUsageUsesObservedCapacity() {
  presentation::GlobalConfigurationViewModel model(4096);
  auto current = snapshot().current;
  current.overlay = {core::OverlayType::RAM, 8192, 3000, 3500};
  model.reset(current);
  model.setCurrentSession(current, {2048, 2048});
  const auto usage = model.usage();
  QCOMPARE(usage.currentMb, uint32_t{2048});
  QCOMPARE(usage.totalMb, uint64_t{4096});
  QCOMPARE(usage.valueText, QStringLiteral("2,048 / 4,096 MB"));
  QCOMPARE(usage.percentageText, QStringLiteral("50%"));
  QCOMPARE(usage.warningMb, uint32_t{3000});
  QCOMPARE(usage.criticalMb, uint32_t{3500});
}

void OverlayViewModelTests::overlayPublishesOnlyCompleteSuccessfulReads_data() {
  QTest::addColumn<bool>("initialEnabled");
  QTest::newRow("last-observed-enabled") << true;
  QTest::newRow("last-observed-disabled") << false;
}

void OverlayViewModelTests::overlayPublishesOnlyCompleteSuccessfulReads() {
  QFETCH(bool, initialEnabled);
  auto reader = std::make_shared<FakeUsageReader>();
  presentation::OverlayViewModel model(reader);
  model.refreshUsage();
  QTRY_VERIFY(!model.refreshing());
  QCOMPARE(reader->calls, 0);
  auto state = snapshot();
  state.current.filter.enabled = initialEnabled;
  model.applySnapshot(state);
  const auto lastSuccess = app::overlayUsageObservation(model.usage()).lastSuccess;
  QVERIFY(lastSuccess);
  QSignalSpy changes(&model, &presentation::OverlayViewModel::usageChanged);
  reader->fail = true;
  model.refreshUsage();
  QTRY_VERIFY(!model.refreshing());
  QCOMPARE(changes.count(), 1);
  QCOMPARE(std::holds_alternative<app::OverlayUsageEnabled>(model.usage()), initialEnabled);
  if (initialEnabled)
    QCOMPARE(std::get<app::OverlayUsageEnabled>(model.usage()).runtime.currentConsumptionMb, state.runtime.currentConsumptionMb);
  else
    QVERIFY(std::holds_alternative<app::OverlayUsageDisabled>(model.usage()));
  const auto failed = app::overlayUsageObservation(model.usage());
  QVERIFY(failed.stale());
  QCOMPARE(failed.lastSuccess, lastSuccess);
  QCOMPARE(*failed.error, std::string("incomplete read"));
  QVERIFY(presentation::overlayUsageToolTip(model.usage()).contains(QStringLiteral("incomplete read")));
  model.refreshUsage();
  QTRY_VERIFY(!model.refreshing());
  QCOMPARE(changes.count(), 2);
  QCOMPARE(app::overlayUsageObservation(model.usage()).lastSuccess, lastSuccess);
  reader->fail = false;
  auto runtime = state.runtime;
  runtime.currentConsumptionMb = 2048;
  reader->next = app::OverlayUsageEnabled{runtime, state.current.overlay};
  model.refreshUsage();
  QTRY_VERIFY(!model.refreshing());
  QCOMPARE(changes.count(), 3);
  QCOMPARE(std::get<app::OverlayUsageEnabled>(model.usage()).runtime.currentConsumptionMb, uint64_t{2048});
  QVERIFY(!app::overlayUsageObservation(model.usage()).stale());
  QVERIFY(app::overlayUsageObservation(model.usage()).lastSuccess);
  QVERIFY(!presentation::overlayUsageToolTip(model.usage()).contains(QStringLiteral("incomplete read")));
}

void OverlayViewModelTests::observationPauseSuppressesAutomaticAndManualReadsUntilTheFinalLease() {
  auto gate = std::make_shared<test::WorkerGate>();
  const auto unblock = qScopeGuard([gate] { gate->release(); });
  auto reader = std::make_shared<ControlledUsageReader>();
  reader->reads = {{gate, usageWithConsumption(123)}};
  // Use a short interval to exercise automatic polling and observation leases.
  constexpr auto period = std::chrono::milliseconds{50};
  presentation::OverlayViewModel model(reader, period);
  model.applySnapshot(snapshot());
  QSignalSpy changed(&model, &presentation::OverlayViewModel::usageChanged);
  // Pause on delivery so another timer tick cannot race the QTRY assertion.
  presentation::ObservationPause afterDelivery;
  connect(&model, &presentation::OverlayViewModel::usageChanged, &model, [&] { afterDelivery = model.pauseObservation(); });
  auto first = model.pauseObservation();
  auto second = model.pauseObservation();
  auto moved = std::move(second);
  second = {};
  QVERIFY(model.observationPaused());
  model.refreshUsage();
  QCOMPARE(reader->calls.load(), 0);

  // Cross two polling periods to detect reads while observation is paused.
  QTimer barrier;
  barrier.setSingleShot(true);
  barrier.setTimerType(Qt::PreciseTimer);
  bool crossedPollingPeriod = false;
  connect(&barrier, &QTimer::timeout, &barrier, [&] { crossedPollingPeriod = true; });
  barrier.start(2 * period + std::chrono::milliseconds{1});
  const auto timeout = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(period).count()) + 2000;
  QTRY_VERIFY_WITH_TIMEOUT(crossedPollingPeriod, timeout);
  QCOMPARE(reader->calls.load(), 0);
  QVERIFY(changed.isEmpty());

  first = {};
  QVERIFY(model.observationPaused());
  model.refreshUsage();
  QCOMPARE(reader->calls.load(), 0);
  moved = {};
  QVERIFY(!model.observationPaused());
  // No explicit refresh: the last lease must restore automatic observation.
  QTRY_COMPARE_WITH_TIMEOUT(reader->calls.load(), 1, timeout);
  QTRY_COMPARE(gate->started.load(), 1);
  model.refreshUsage();
  QCOMPARE(reader->calls.load(), 1);
  gate->release();
  QTRY_VERIFY(!model.refreshing());
  QCOMPARE(changed.count(), 1);
  QCOMPARE(std::get<app::OverlayUsageEnabled>(model.usage()).runtime.currentConsumptionMb, std::uint64_t{123});
}

void OverlayViewModelTests::observationPauseDiscardsOldResultsWithoutJoiningOtherWork() {
  auto* pool = QThreadPool::globalInstance();
  const auto previousMaximum = pool->maxThreadCount();
  pool->setMaxThreadCount(std::max(2, previousMaximum));
  const auto restorePool = qScopeGuard([pool, previousMaximum] { pool->setMaxThreadCount(previousMaximum); });
  auto gate = std::make_shared<test::WorkerGate>();
  const auto unblock = qScopeGuard([gate] { gate->release(); });
  auto reader = std::make_shared<ControlledUsageReader>();
  reader->reads = {{gate, usageWithConsumption(999)}, {{}, usageWithConsumption(456)}};
  presentation::OverlayViewModel model(reader);
  model.applySnapshot(snapshot());
  QSignalSpy changed(&model, &presentation::OverlayViewModel::usageChanged);
  model.refreshUsage();
  QTRY_COMPARE(gate->started.load(), 1);
  auto pause = model.pauseObservation();
  QVERIFY(model.observationPaused());
  QVERIFY(!model.refreshing());
  QCOMPARE(gate->finished.load(), 0);  // pause returned while the provider is blocked.

  auto independentReader = std::make_shared<ControlledUsageReader>();
  independentReader->reads = {{{}, usageWithConsumption(77)}};
  presentation::OverlayViewModel independent(independentReader);
  independent.applySnapshot(snapshot());
  QSignalSpy independentChanged(&independent, &presentation::OverlayViewModel::usageChanged);
  independent.refreshUsage();
  QTRY_COMPARE(independentChanged.count(), 1);
  QCOMPARE(gate->finished.load(), 0);

  auto fresh = snapshot();
  fresh.runtime.currentConsumptionMb = 222;
  model.applySnapshot(fresh);
  QCOMPARE(changed.count(), 1);
  gate->release();
  QTRY_COMPARE(gate->finished.load(), 1);
  // Wait for promise publication, then deliver watcher events before checking that the stale result was discarded.
  QTRY_VERIFY(pool->waitForDone(0));
  QCoreApplication::sendPostedEvents();
  QCoreApplication::processEvents();
  QCOMPARE(changed.count(), 1);
  QCOMPARE(std::get<app::OverlayUsageEnabled>(model.usage()).runtime.currentConsumptionMb, std::uint64_t{222});
  model.refreshUsage();
  QCOMPARE(reader->calls.load(), 1);
  pause = {};
  model.refreshUsage();
  QTRY_COMPARE(changed.count(), 2);
  QCOMPARE(reader->calls.load(), 2);
  QCOMPARE(std::get<app::OverlayUsageEnabled>(model.usage()).runtime.currentConsumptionMb, std::uint64_t{456});
}

void OverlayViewModelTests::observationPauseLeaseCanOutliveItsOwner() {
  auto gate = std::make_shared<test::WorkerGate>();
  const auto unblock = qScopeGuard([gate] { gate->release(); });
  auto reader = std::make_shared<ControlledUsageReader>();
  reader->reads = {{gate, usageWithConsumption(999)}};
  auto model = std::make_unique<presentation::OverlayViewModel>(reader);
  model->applySnapshot(snapshot());
  QSignalSpy changed(model.get(), &presentation::OverlayViewModel::usageChanged);
  model->refreshUsage();
  QTRY_COMPARE(gate->started.load(), 1);
  auto pause = model->pauseObservation();
  model.reset();
  QCOMPARE(gate->finished.load(), 0);  // Destruction did not join the provider.
  pause = {};                          // Its QPointer release target is already gone.
  QCOMPARE(gate->finished.load(), 0);
  gate->release();
  QTRY_VERIFY(QThreadPool::globalInstance()->waitForDone(0));
  QCoreApplication::sendPostedEvents();
  QCoreApplication::processEvents();
  QVERIFY(changed.isEmpty());
}

void OverlayViewModelTests::releasingAnObservationPauseRespectsTheLatestSnapshot_data() {
  QTest::addColumn<bool>("available");
  QTest::newRow("snapshot-remains-available") << true;
  QTest::newRow("snapshot-became-unavailable") << false;
}

void OverlayViewModelTests::releasingAnObservationPauseRespectsTheLatestSnapshot() {
  QFETCH(bool, available);
  auto reader = std::make_shared<ControlledUsageReader>();
  reader->reads = {{{}, usageWithConsumption(123)}};
  presentation::OverlayViewModel model(reader);
  auto pause = model.pauseObservation();
  auto state = snapshot();
  state.uwfAvailable = available;
  model.applySnapshot(state);
  QVERIFY(model.observationPaused());
  model.refreshUsage();
  QCOMPARE(reader->calls.load(), 0);
  pause = {};
  QVERIFY(!model.observationPaused());
  model.refreshUsage();
  QTRY_VERIFY(!model.refreshing());
  QCOMPARE(reader->calls.load(), available ? 1 : 0);
}

}  // namespace

QTEST_GUILESS_MAIN(OverlayViewModelTests)
#include "OverlayViewModelTests.moc"
