// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QScopeGuard>
#include <QTest>
#include <functional>
#include <future>
#include <map>
#include <memory>
#include <stdexcept>
#include <utility>

#include "application/uwf/adapters/UwfOverlayMaintenance.h"
#include "support/app/WorkerGate.h"

namespace {
using namespace uwf;
constexpr char firstVolume[] = "11111111-1111-1111-1111-111111111111";
constexpr char secondVolume[] = "22222222-2222-2222-2222-222222222222";

// Literal maintenance observations, independent of the UWF simulator.
class MaintenanceUwf final : public compat::Uwf {
 public:
  mutable bool filter = true;
  mutable bool horm = false;
  mutable bool commit = false;
  mutable std::uint32_t currentType = 0, nextType = 0, size = 4096, currentFlags = 4, nextFlags = 4;
  mutable std::map<std::string, std::uint32_t> currentHosts{{firstVolume, 1}, {secondVolume, 0}};
  mutable std::map<std::string, std::uint32_t> nextHosts{{firstVolume, 1}, {secondVolume, 0}};
  mutable std::map<std::string, bool> nextProtection{{firstVolume, true}, {secondVolume, false}};
  mutable compat::UwfStateFacts published;
  mutable int reads = 0;
  mutable int probes = 0;
  mutable int writes = 0;
  mutable int hormWrites = 0;
  mutable int sessionDepth = 0;
  mutable int sessions = 0;
  mutable int refreshes = 0;
  std::function<compat::Response()> onRefresh;
  bool requireSession = false;
  bool flagsUnsupported = false;
  bool hormUnknown = false;
  bool commitNoState = false;
  bool commitFailAfterState = false;
  bool disableOnly = false;
  bool nextVolumesUnknown = false;
  bool nextMarkerUnknown = false;
  MaintenanceUwf() {
    published.runtime.hiberFile = compat::Fact<bool>::known(true);
    published.runtime.hormVolumesProtected = compat::Fact<bool>::known(true);
    published.runtime.romDisposition = compat::BitFacts::known(0);
    published.systemVolumeId = compat::Fact<std::string>::known(firstVolume);
    published.runtime.protectScope = compat::Fact<std::vector<std::string>>::known({firstVolume, secondVolume});
  }
  compat::UwfStateSnapshot stateSnapshot() const override {
    using namespace compat;
    auto facts = published;
    facts.current.filterEnabled = Fact<bool>::known(filter);
    facts.runtime.horm = hormUnknown ? Fact<bool>{} : Fact<bool>::known(horm);
    facts.runtime.fullCommit = Fact<bool>::known(commit);
    facts.current.overlayType = Fact<std::uint32_t>::known(currentType);
    facts.next.overlayType = Fact<std::uint32_t>::known(nextType);
    facts.next.maximumOverlaySize = Fact<std::uint32_t>::known(size);
    facts.current.overlayFlags = flagsUnsupported ? BitFacts{} : BitFacts::known(currentFlags);
    facts.next.overlayFlags = flagsUnsupported ? BitFacts{} : BitFacts::known(nextFlags);
    facts.current.volumesObserved = true;
    facts.next.volumesObserved = !nextVolumesUnknown;
    for (const auto& [id, marker] : currentHosts) {
      auto& volume = facts.current.volumes[id];
      volume.present = Fact<bool>::known(true);
      volume.protectedVolume = Fact<bool>::known(id == firstVolume);
      volume.driveLetter = Fact<std::string>::known(id == firstVolume ? "C:" : "D:");
      volume.swapfileMarker = Fact<std::uint32_t>::known(marker);
    }
    for (const auto& [id, marker] : nextHosts) {
      auto& volume = facts.next.volumes[id];
      volume.present = Fact<bool>::known(true);
      volume.protectedVolume = Fact<bool>::known(nextProtection.at(id));
      volume.driveLetter = Fact<std::string>::known(id == firstVolume ? "C:" : "D:");
      volume.swapfileMarker = id == secondVolume && nextMarkerUnknown ? Fact<std::uint32_t>{} : Fact<std::uint32_t>::known(marker);
    }
    return std::make_shared<const compat::UwfStateFacts>(std::move(facts));
  }
  void updateState(compat::UwfStateFacts facts) const override {
    if (requireSession && sessionDepth == 0) throw std::logic_error("Observation publication escaped its synchronized session");
    published = std::move(facts);
  }
  void synchronize(const std::function<void()>& work) const override {
    ++sessions;
    ++sessionDepth;
    try {
      work();
    } catch (...) {
      --sessionDepth;
      throw;
    }
    --sessionDepth;
  }
  compat::Response refreshState(const compat::Context&) const override {
    ++refreshes;
    return onRefresh ? onRefresh() : compat::Response::success();
  }
  compat::Capability probe(const compat::Request& request) const override {
    ++probes;
    if (const auto* value = std::get_if<compat::SetBoolean>(&request);
        disableOnly && value && value->property == compat::BooleanProperty::HormEnabled && value->value)
      return {compat::Support::Unsupported, "Only HORM disable is available"};
    return {compat::Support::Supported, {}};
  }
  compat::Response execute(const compat::Request& request, const compat::Context&) const override {
    using namespace compat;
    if (requireSession && sessionDepth == 0) throw std::logic_error("A maintenance observation escaped its synchronized session");
    if (!isWrite(request)) ++reads;
    if (const auto* value = std::get_if<GetBoolean>(&request)) {
      if (value->property == BooleanProperty::FilterEnabled) return Response::success(filter);
      if (value->property == BooleanProperty::OverlayCommit) return Response::success(commit);
      if (value->property == BooleanProperty::HormEnabled)
        return hormUnknown ? Response::fail(ErrorKind::Unavailable, "HORM read unavailable") : Response::success(horm);
    }
    if (const auto* value = std::get_if<GetNumber>(&request)) {
      if (value->property == NumberProperty::OverlayType) return Response::success(value->session == Session::Current ? currentType : nextType);
      if (value->property == NumberProperty::MaximumOverlaySize) return Response::success(size);
      if (value->property == NumberProperty::OverlayFlags)
        return flagsUnsupported ? Response::fail(ErrorKind::Unsupported, "No public Flags getter")
                                : Response::success(value->session == Session::Current ? currentFlags : nextFlags);
    }
    if (const auto* value = std::get_if<GetVolumes>(&request)) {
      if (value->session == Session::Next && nextVolumesUnknown) return Response::fail(ErrorKind::Unavailable, "Next volume list could not be read");
      return Response::success(
          std::vector<Volume>{{firstVolume, "C:", value->session == Session::Current ? true : nextProtection.at(firstVolume), false, {}},
                              {secondVolume, "D:", value->session == Session::Current ? false : nextProtection.at(secondVolume), false, {}}});
    }
    if (const auto* value = std::get_if<GetSwapfileSize>(&request)) {
      if (value->session == Session::Next && value->volume == secondVolume && nextMarkerUnknown)
        return Response::fail(ErrorKind::Unavailable, "Next storage marker could not be read");
      return Response::success((value->session == Session::Current ? currentHosts : nextHosts).at(value->volume));
    }
    if (const auto* value = std::get_if<SetBoolean>(&request)) {
      ++writes;
      if (value->property == BooleanProperty::HormEnabled) {
        ++hormWrites;
        horm = value->value;
      } else if (value->property == BooleanProperty::OverlayCommit) {
        if (!commitNoState) commit = true;
        if (commitFailAfterState) return Response::fail(ErrorKind::AccessDenied, "Write failed after dispatch", Effect::Uncertain);
      }
      return Response::success(std::monostate{}, Effect::Confirmed);
    }
    if (const auto* value = std::get_if<CreateSwapfile>(&request)) {
      ++writes;
      for (auto& [id, marker] : nextHosts) marker = id == value->volume ? 1 : 0;
      nextType = 1;
      return Response::success(std::monostate{}, Effect::Confirmed);
    }
    return Response::fail(ErrorKind::Unsupported, "Unexpected maintenance request");
  }
};
class MaintenanceVolumes final : public app::VolumeRepository {
 public:
  std::string system = "S:";
  bool fail = false;
  std::vector<core::VolumeInfo> values{{"F:", compat::volumePath(firstVolume), "NTFS", "Candidate", 1ULL << 40, 1ULL << 39, core::VolumeSupport::Supported},
                                       {"S:", compat::volumePath(secondVolume), "NTFS", "System", 1ULL << 40, 1ULL << 39, core::VolumeSupport::Supported}};
  std::vector<core::VolumeInfo> read() const override {
    if (fail) throw std::runtime_error("Physical volume inventory unavailable");
    return values;
  }
  std::string driveForPath(const std::string&) const override { return {}; }
  std::vector<std::string> mountedLetters(const std::string& volume) const override {
    for (const auto& value : values)
      if (compat::parseVolumeId(value.volumeName) == compat::parseVolumeId(volume)) return {value.driveLetter};
    return {};
  }
  std::vector<std::string> mountedPaths(const std::string& volume) const override {
    std::vector<std::string> paths;
    for (const auto& letter : mountedLetters(volume)) paths.push_back(letter + '\\');
    return paths;
  }
  std::string systemDrive() const override { return system; }
};
app::OverlayMaintenanceReadOnlyPorts readyReadOnlyPorts() {
  return {.hormVolumeProtection = [](std::stop_token) { return app::OverlayHormVolumeProtection{true, {}, {}}; },
          .hibernationCapabilities = [](std::stop_token) { return app::OverlayHibernationCapabilities{true, true, app::HibernationFileType::Full}; }};
}
class LiteralAllVolumeProtection final : public app::AllVolumeProtectionActions {
 public:
  app::AllVolumeProtectionSnapshot state;
  bool changeObservedState = true;
  bool acknowledge = true;
  std::function<void(bool)> publish;
  int writes = 0;
  app::AllVolumeProtectionSnapshot observe(std::stop_token) override { return state; }
  std::optional<app::AllVolumeProtectionSnapshot> snapshot() const override { return state; }
  app::AllVolumeProtectionResult setProtected(const bool enabled, std::stop_token) override {
    ++writes;
    if (changeObservedState) {
      state.nextAllProtected = enabled;
      state.nextAllUnprotected = !enabled;
      if (publish) publish(enabled);
    }
    app::AllVolumeProtectionResult result;
    result.snapshot = state;
    result.effect = app::WriteEffect::Confirmed;
    result.completionConfirmed = acknowledge;
    result.items.push_back({QString::fromStdString(firstVolume), !enabled, enabled, {}, {}, app::WriteEffect::Confirmed, true});
    return result;
  }
};

class OverlayMaintenanceTests final : public QObject {
  Q_OBJECT
 private slots:
  void anUnconfirmedCompletionCannotBeRepairedByMatchingDisplayState() {
    auto system = std::make_shared<MaintenanceUwf>();
    auto bulk = std::make_shared<LiteralAllVolumeProtection>();
    bulk->acknowledge = false;
    const auto actions = app::uwfOverlayMaintenance(system, {}, readyReadOnlyPorts(), bulk);
    const auto result = actions->setAllVolumesProtected(true);
    QVERIFY(!result.completionConfirmed);
    QVERIFY(!result.succeeded());
    QCOMPARE(system->reads, 0);
    QCOMPARE(system->writes, 0);
  }
  void snapshotDoesNotEnterTheExecutionSessionOrProbeCapabilities() {
    auto system = std::make_shared<MaintenanceUwf>();
    system->requireSession = true;
    const auto actions = app::uwfOverlayMaintenance(system, std::make_shared<MaintenanceVolumes>(), readyReadOnlyPorts());
    const auto observed = actions->snapshot();
    QVERIFY(observed);
    QCOMPARE(observed->nextOverlayType, std::optional<std::int64_t>{0});
    QCOMPARE(system->sessions, 0);
    QCOMPARE(system->reads, 0);
    QCOMPARE(system->probes, 0);
  }
  void publishedConfigurationAndEligibilityAdvanceWithoutReadingHostDetailsAgain() {
    auto system = std::make_shared<MaintenanceUwf>();
    const auto actions = app::uwfOverlayMaintenance(system, std::make_shared<MaintenanceVolumes>(), readyReadOnlyPorts());
    const auto first = actions->observe();
    QCOMPARE(system->reads, 0);
    const auto probes = system->probes;
    const auto sessions = system->sessions;
    const auto refreshes = system->refreshes;
    system->nextType = 1;
    system->nextFlags = 1;
    system->nextHosts = {{firstVolume, 0}, {secondVolume, 1}};
    system->published.targets[firstVolume].volumeEligibility = compat::Fact<compat::VolumeAdmission>::known({true, false});
    system->published.targets[secondVolume].volumeEligibility = compat::Fact<compat::VolumeAdmission>::known({true, true});
    const auto observed = actions->snapshot();
    QVERIFY(observed);
    QCOMPARE(observed->nextOverlayType, std::optional<std::int64_t>{1});
    QCOMPARE(observed->nextFlags->raw, std::uint32_t{1});
    QCOMPARE(observed->nextStorage.volumeId, std::optional<QString>{QString::fromStdString(secondVolume)});
    QVERIFY(!observed->volumes[0].eligible);
    QVERIFY(observed->volumes[1].eligible);
    QCOMPARE(observed->volumes[0].label, first.volumes[0].label);
    QCOMPARE(observed->volumes[0].freeBytes, first.volumes[0].freeBytes);
    QCOMPARE(system->probes, probes);
    QCOMPARE(system->reads, 0);
    QCOMPARE(system->sessions, sessions);
    QCOMPARE(system->refreshes, refreshes);
  }
  void anIncompleteHostObservationCannotBlockOrPartiallyReplaceThePublishedView() {
    using namespace std::chrono_literals;
    auto system = std::make_shared<MaintenanceUwf>();
    auto gate = std::make_shared<test::WorkerGate>();
    bool delay = false;
    auto ports = readyReadOnlyPorts();
    ports.hibernationCapabilities = [gate, &delay](const std::stop_token stop) {
      if (delay) gate->wait(stop);
      return app::OverlayHibernationCapabilities{true, true, delay ? app::HibernationFileType::Reduced : app::HibernationFileType::Full};
    };
    const auto actions = app::uwfOverlayMaintenance(system, {}, std::move(ports));
    QVERIFY(actions->observe().hibernationAvailable == true);
    delay = true;
    auto observation = std::async(std::launch::async, [actions] { return actions->observe(); });
    const auto release = qScopeGuard([gate] { gate->release(); });
    QTRY_COMPARE(gate->started.load(), 1);
    auto projection = std::async(std::launch::async, [actions] { return actions->snapshot(); });
    const auto ready = projection.wait_for(200ms);
    // Release before an assertion exits: futures must not hide a blocking read.
    gate->release();
    QCOMPARE(ready, std::future_status::ready);
    const auto during = projection.get();
    QVERIFY(during && during->hibernationAvailable == true);
    const auto after = observation.get();
    QCOMPARE(after.hibernationAvailable, std::optional<bool>{false});
    QCOMPARE(actions->snapshot()->hibernationAvailable, std::optional<bool>{false});
    QCOMPARE(system->reads, 0);
    QCOMPARE(system->sessions, 0);
  }
  void failedHibernationFactDoesNotReuseCachedAvailability() {
    auto system = std::make_shared<MaintenanceUwf>();
    const auto actions = app::uwfOverlayMaintenance(system, {}, readyReadOnlyPorts());
    QVERIFY(actions->observe().hibernationAvailable == true);
    system->published.runtime.hiberFile = {};
    QVERIFY(!actions->snapshot()->hibernationAvailable.has_value());
    system->published.runtime.hiberFile = compat::Fact<bool>::known(true);
    QVERIFY(actions->snapshot()->hibernationAvailable == true);
  }
  void globalRefreshFailuresRemainVisibleInThePureProjection() {
    auto system = std::make_shared<MaintenanceUwf>();
    system->published.refreshFailure = compat::Failure{compat::ErrorKind::Unavailable, compat::Effect::None, "Global threshold read failed", {}};
    const auto actions = app::uwfOverlayMaintenance(system);
    const auto observed = actions->snapshot();
    QVERIFY(observed->readError.contains(QStringLiteral("Global threshold read failed")));
    QCOMPARE(observed->filterEnabled, std::optional<bool>{true});
    QCOMPARE(system->reads, 0);
    QCOMPARE(system->probes, 0);
    system->published.refreshFailure.reset();
    QVERIFY(actions->snapshot()->readError.isEmpty());
  }
  void explicitReadsRefreshSharedStateOnceBeforeHostDetails_data() {
    QTest::addColumn<bool>("initialFailure");
    QTest::addColumn<bool>("recover");
    QTest::newRow("healthy-state-is-refreshed") << false << true;
    QTest::newRow("global-refresh-recovers") << true << true;
    QTest::newRow("global-refresh-still-fails") << true << false;
  }
  void explicitReadsRefreshSharedStateOnceBeforeHostDetails() {
    QFETCH(bool, initialFailure);
    QFETCH(bool, recover);
    auto system = std::make_shared<MaintenanceUwf>();
    if (initialFailure) system->published.refreshFailure = compat::Failure{compat::ErrorKind::Unavailable, compat::Effect::None, "Global read failed", {}};
    system->onRefresh = [system, recover] {
      if (!recover) return compat::Response::fail(compat::ErrorKind::Unavailable, "Global read failed");
      system->published.refreshFailure.reset();
      return compat::Response::success();
    };
    int hostReads = 0;
    auto ports = readyReadOnlyPorts();
    ports.hibernationCapabilities = [&](std::stop_token) {
      ++hostReads;
      return app::OverlayHibernationCapabilities{true, true, app::HibernationFileType::Full};
    };
    const auto actions = app::uwfOverlayMaintenance(system, {}, std::move(ports));
    const auto observed = actions->observe();
    QCOMPARE(system->refreshes, 1);
    QCOMPARE(hostReads, recover ? 1 : 0);
    QCOMPARE(observed.readError.isEmpty(), recover);
    QCOMPARE(system->probes, recover ? 3 : 0);
    QCOMPARE(system->reads, 0);
    system->onRefresh = {};
  }
  void failedPhysicalReadDoesNotClearTheGlobalBarrier() {
    auto system = std::make_shared<MaintenanceUwf>();
    const auto volumes = std::make_shared<MaintenanceVolumes>();
    system->published.refreshFailure = compat::Failure{compat::ErrorKind::Unavailable, compat::Effect::None, "Global read failed", {}};
    system->onRefresh = [state = system.get()] {
      state->published.refreshFailure.reset();
      return compat::Response::success();
    };
    const auto actions = app::uwfOverlayMaintenance(system, volumes, readyReadOnlyPorts());
    volumes->fail = true;
    QVERIFY_THROWS_EXCEPTION(std::runtime_error, (void)actions->observe());
    QCOMPARE(system->refreshes, 0);
    QVERIFY(system->published.refreshFailure);
    volumes->fail = false;
    const auto recovered = actions->observe();
    QCOMPARE(system->refreshes, 1);
    QVERIFY(!system->published.refreshFailure);
    QVERIFY(recovered.volumesReadable);
    QVERIFY(recovered.readError.isEmpty());
  }
  void aDisabledCurrentFilterDoesNotReadActiveControlPrerequisites() {
    auto system = std::make_shared<MaintenanceUwf>();
    system->filter = false;
    system->published.runtime.romDisposition = {};
    system->published.runtime.hormVolumesProtected = {};
    int activeReads = 0;
    auto ports = readyReadOnlyPorts();

    ports.hormVolumeProtection = [&](std::stop_token) {
      ++activeReads;
      return app::OverlayHormVolumeProtection{};
    };
    const auto observed = app::uwfOverlayMaintenance(system, {}, std::move(ports))->observe();
    QCOMPARE(activeReads, 0);
    QVERIFY(observed.readError.isEmpty());
    QVERIFY(observed.allVolumesProtectedError.isEmpty());
    QCOMPARE(system->reads, 0);
  }
  void aPublishedHibernationFileChangeUpdatesTheCachedCapabilities() {
    const auto system = std::make_shared<MaintenanceUwf>();
    const auto actions = app::uwfOverlayMaintenance(system, {}, readyReadOnlyPorts());
    QVERIFY(actions->observe().hibernationCapabilities->hiberFilePresent);
    auto state = system->state();
    state.runtime.hiberFile = compat::Fact<bool>::known(false);
    system->updateState(std::move(state));
    const auto observed = actions->snapshot();
    QVERIFY(observed->hibernationCapabilities.has_value());
    QVERIFY(!observed->hibernationCapabilities->hiberFilePresent);
    QCOMPARE(observed->hibernationAvailable, std::optional<bool>{false});
  }
  void failedHostDetailsDoNotOverwriteRefreshedSharedFacts() {
    auto system = std::make_shared<MaintenanceUwf>();
    system->requireSession = true;
    system->published.runtime.hiberFile = compat::Fact<bool>::known(true);
    system->published.runtime.hormVolumesProtected = compat::Fact<bool>::known(true);
    system->published.runtime.romDisposition = compat::BitFacts::known(3);
    system->published.systemVolumeId = compat::Fact<std::string>::known(firstVolume);
    auto ports = readyReadOnlyPorts();
    ports.hibernationCapabilities = [](std::stop_token) -> app::OverlayHibernationCapabilities { throw std::runtime_error("Host read failed"); };

    ports.hormVolumeProtection = [](std::stop_token) -> app::OverlayHormVolumeProtection { throw std::runtime_error("Host read failed"); };

    const auto actions = app::uwfOverlayMaintenance(system, {}, std::move(ports));
    const auto observed = actions->observe();
    QCOMPARE(system->refreshes, 1);
    QCOMPARE(system->sessions, 0);
    QCOMPARE(system->sessionDepth, 0);
    QVERIFY(!observed.hibernationError.isEmpty());
    const auto state = system->state();
    QCOMPARE(state.runtime.hiberFile.value, std::optional<bool>{true});
    QCOMPARE(state.runtime.hormVolumesProtected.value, std::optional<bool>{true});
    QCOMPARE(state.runtime.romDisposition.valueBits, std::uint32_t{3});
    QCOMPARE(state.systemVolumeId.value, std::optional<std::string>{firstVolume});
    const auto projected = actions->snapshot();
    QVERIFY(projected.has_value());
    QCOMPARE(projected->romDisposition, std::optional<std::uint32_t>{3});
    QCOMPARE(system->sessions, 0);
    QCOMPARE(system->reads, 0);
    QCOMPARE(system->sessionDepth, 0);
  }
  void bulkProtectionDirectionsRemainIndependentAndKeepTheirReceipts() {
    auto system = std::make_shared<MaintenanceUwf>();
    auto bulk = std::make_shared<LiteralAllVolumeProtection>();
    bulk->state.protectSupported = bulk->state.unprotectSupported = true;
    bulk->state.protectScopeConfirmed = bulk->state.unprotectScopeConfirmed = true;
    bulk->state.nextAllProtected = bulk->state.nextAllUnprotected = false;
    bulk->publish = [system](const bool enabled) {
      for (auto& [id, protectedVolume] : system->nextProtection) {
        (void)id;
        protectedVolume = enabled;
      }
    };
    bulk->state.unprotectTargetVolumes = {QString::fromStdString(firstVolume), QString::fromStdString(secondVolume)};
    bulk->state.protectTargetVolumes = bulk->state.unprotectTargetVolumes;
    const auto actions = app::uwfOverlayMaintenance(system, {}, readyReadOnlyPorts(), bulk);
    const auto mixed = actions->observe();
    QCOMPARE(mixed.allVolumesNextProtected, std::optional<bool>{false});
    QCOMPARE(mixed.allVolumesNextUnprotected, std::optional<bool>{false});
    QVERIFY(mixed.unprotectScopeConfirmed);
    QCOMPARE(mixed.unprotectTargetVolumes, bulk->state.unprotectTargetVolumes);
    QVERIFY(system->state().runtime.protectScope.value.has_value());
    QCOMPARE(*system->state().runtime.protectScope.value, std::vector<std::string>({firstVolume, secondVolume}));
    const auto reads = system->reads;
    bulk->changeObservedState = false;
    const auto unconfirmed = actions->setAllVolumesProtected(false);
    QVERIFY(unconfirmed.succeeded());
    QVERIFY(unconfirmed.completionConfirmed);
    QCOMPARE(unconfirmed.effect, app::WriteEffect::Confirmed);
    QCOMPARE(unconfirmed.allVolumeItems.size(), std::size_t{1});
    bulk->changeObservedState = true;
    const auto unprotected = actions->setAllVolumesProtected(false);
    QVERIFY(unprotected.succeeded());
    QCOMPARE(unprotected.snapshot.allVolumesNextUnprotected, std::optional<bool>{true});
    QCOMPARE(unprotected.snapshot.allVolumesNextProtected, std::optional<bool>{false});
    QCOMPARE(unprotected.allVolumeItems[0].volumeId, QString::fromStdString(firstVolume));
    QCOMPARE(system->reads, reads);
    QCOMPARE(system->writes, 0);
  }
  void incompleteUnprotectScopeNeverExposesItsPartialInventoryForExport() {
    auto system = std::make_shared<MaintenanceUwf>();
    system->nextVolumesUnknown = true;
    auto bulk = std::make_shared<LiteralAllVolumeProtection>();
    bulk->state.unprotectSupported = true;
    bulk->state.unprotectScopeConfirmed = false;
    bulk->state.unprotectTargetVolumes = {QString::fromStdString(firstVolume)};
    bulk->state.nextAllUnprotected = false;
    const auto observed = app::uwfOverlayMaintenance(system, {}, readyReadOnlyPorts(), bulk)->observe();
    QVERIFY(!observed.unprotectScopeConfirmed);
    QVERIFY(observed.unprotectTargetVolumes.empty());
    QVERIFY(!observed.allVolumesNextUnprotected.has_value());
    QCOMPARE(system->writes, 0);
  }
  void aPhysicalScopeFailureDoesNotBlockTheIndependentUnprotectObservation() {
    auto system = std::make_shared<MaintenanceUwf>();
    system->published.runtime.protectScope = {};
    system->nextProtection = {{firstVolume, false}, {secondVolume, false}};
    auto bulk = std::make_shared<LiteralAllVolumeProtection>();
    bulk->state.protectSupported = false;
    bulk->state.protectError = QStringLiteral("Physical scope unavailable");
    bulk->state.unprotectSupported = true;
    bulk->state.unprotectScopeConfirmed = true;
    bulk->state.nextAllUnprotected = true;
    const auto observed = app::uwfOverlayMaintenance(system, {}, readyReadOnlyPorts(), bulk)->observe();
    QCOMPARE(observed.protectAllCapability.support, app::OverlayMaintenanceSupport::Unsupported);
    QCOMPARE(observed.unprotectAllCapability.support, app::OverlayMaintenanceSupport::Supported);
    QVERIFY(!observed.allVolumesNextProtected.has_value());
    QCOMPARE(observed.allVolumesNextUnprotected, std::optional<bool>{true});
    QVERIFY(!observed.protectAllError.isEmpty());
    QVERIFY(observed.unprotectAllError.isEmpty());
    QVERIFY(observed.readError.isEmpty());
    QCOMPARE(system->writes, 0);
  }
  void unknownReadsStayUnknownAndHostsRemainSessionScoped() {
    auto system = std::make_shared<MaintenanceUwf>();
    system->flagsUnsupported = true;
    system->hormUnknown = true;
    system->nextHosts = {{firstVolume, 0}, {secondVolume, 1}};
    const auto snapshot = app::uwfOverlayMaintenance(system)->observe();
    QVERIFY(!snapshot.hormEnabled.has_value());
    QVERIFY(!snapshot.currentFlags.has_value());
    QVERIFY(snapshot.hormCapability.canAttempt());
    QCOMPARE(snapshot.volumes.size(), std::size_t{2});
    QCOMPARE(snapshot.volumes[0].currentMarker, std::optional<std::uint32_t>{1});
    QCOMPARE(snapshot.volumes[0].nextMarker, std::optional<std::uint32_t>{0});
    QCOMPARE(system->writes, 0);
  }
  void fullCommitKeepsTheProviderReceiptWithoutReadback() {
    auto system = std::make_shared<MaintenanceUwf>();
    system->commitNoState = true;
    const auto result = app::uwfOverlayMaintenance(system)->fullCommit();
    QVERIFY(result.succeeded());
    QVERIFY(result.completionConfirmed);
    QCOMPARE(result.snapshot.overlayCommit, std::optional<bool>{false});
    QCOMPARE(result.effect, app::WriteEffect::Confirmed);
    QCOMPARE(system->writes, 1);
    QCOMPARE(system->reads, 0);
  }
  void matchingReadbackNeverErasesWriteFailure() {
    auto system = std::make_shared<MaintenanceUwf>();
    system->commitFailAfterState = true;
    const auto result = app::uwfOverlayMaintenance(system)->fullCommit();
    QVERIFY(!result.succeeded());
    QCOMPARE(result.snapshot.overlayCommit, std::optional<bool>{true});
    QVERIFY(result.failure.has_value());
    QCOMPARE(result.failure->kind, app::OperationFailureKind::AccessDenied);
    QCOMPARE(result.effect, app::WriteEffect::Uncertain);
    QCOMPARE(system->reads, 0);
  }
  void swapUsesTheSharedMemoryTargetWithoutRefresh() {
    auto system = std::make_shared<MaintenanceUwf>();
    system->filter = false;
    auto actions = app::uwfOverlayMaintenance(system);
    const auto result = actions->createSwap(QString::fromStdString(secondVolume));
    QVERIFY(result.succeeded());
    QCOMPARE(result.snapshot.nextOverlayType, std::optional<std::int64_t>{1});
    QCOMPARE(result.snapshot.volumes[0].nextMarker, std::optional<std::uint32_t>{0});
    QCOMPARE(result.snapshot.volumes[1].nextMarker, std::optional<std::uint32_t>{1});
    QCOMPARE(result.snapshot.volumes[0].currentMarker, std::optional<std::uint32_t>{1});
    QCOMPARE(system->writes, 1);
    QCOMPARE(system->reads, 0);
  }
  void completeHormOperationDoesNotRequirePublicFlags() {
    auto system = std::make_shared<MaintenanceUwf>();
    system->flagsUnsupported = true;
    auto actions = app::uwfOverlayMaintenance(system, {}, readyReadOnlyPorts());
    QVERIFY(actions->setHorm(true).succeeded());
    QVERIFY(system->horm);
    // Disable does not inherit enable prerequisites.
    system->filter = false;
    system->currentType = 1;
    system->currentFlags = 0;
    QVERIFY(actions->setHorm(false).succeeded());
    QVERIFY(!system->horm);
    QCOMPARE(system->hormWrites, 2);
  }
  void alreadyRequestedFullCommitIsNotWrittenAgain() {
    auto system = std::make_shared<MaintenanceUwf>();
    system->commit = true;
    const auto result = app::uwfOverlayMaintenance(system)->fullCommit();
    QVERIFY(result.succeeded());
    QCOMPARE(result.effect, app::WriteEffect::None);
    QCOMPARE(system->writes, 0);
  }
  void activeHormUsesDisableCapabilityAndCancellationDoesNotWrite() {
    auto system = std::make_shared<MaintenanceUwf>();
    system->horm = true;
    system->disableOnly = true;
    auto actions = app::uwfOverlayMaintenance(system);
    QVERIFY(actions->observe().hormCapability.canAttempt());
    std::stop_source source;
    source.request_stop();
    const auto result = actions->setHorm(false, source.get_token());
    QVERIFY(!result.succeeded());
    QCOMPARE(result.effect, app::WriteEffect::None);
    QCOMPARE(system->writes, 0);
    QVERIFY(system->horm);
  }
  void missingHostPortsStayUnknownInsteadOfUsingConfigurationAsPrerequisites() {
    auto system = std::make_shared<MaintenanceUwf>();
    system->published.runtime.hormVolumesProtected = {};
    system->published.runtime.romDisposition = {};
    const auto snapshot = app::uwfOverlayMaintenance(system)->observe();
    QVERIFY(!snapshot.hibernationAvailable.has_value());
    QVERIFY(!snapshot.allVolumesProtected.has_value());
    QVERIFY(!snapshot.romDisposition.has_value());
    QVERIFY(!snapshot.hibernationError.isEmpty());
    QVERIFY(!snapshot.allVolumesProtectedError.isEmpty());
    QVERIFY(snapshot.blockedVolumes.empty());
    QCOMPARE(system->writes, 0);
  }
  void hostFactsAndBlockingNamesAreIndependentReadOnlyObservations() {
    auto system = std::make_shared<MaintenanceUwf>();
    system->published.runtime.hormVolumesProtected = compat::Fact<bool>::known(false);
    system->published.runtime.romDisposition = compat::BitFacts::known(0x80000003);
    auto ports = readyReadOnlyPorts();
    int fileReads = 0, volumeReads = 0;
    ports.hibernationCapabilities = [&](std::stop_token) {
      ++fileReads;
      return app::OverlayHibernationCapabilities{true, true, app::HibernationFileType::Full};
    };
    ports.hormVolumeProtection = [&](std::stop_token) {
      ++volumeReads;
      return app::OverlayHormVolumeProtection{
          false, {{QString::fromStdString(firstVolume), QStringLiteral("Z:\\MountedData")}, {QString::fromStdString(secondVolume), QStringLiteral("V:")}}, {}};
    };

    const auto snapshot = app::uwfOverlayMaintenance(system, {}, std::move(ports))->observe();
    QCOMPARE(snapshot.hibernationAvailable, std::optional<bool>{true});
    QCOMPARE(snapshot.allVolumesProtected, std::optional<bool>{false});
    QCOMPARE(snapshot.romDisposition, std::optional<std::uint32_t>{0x80000003});
    QCOMPARE(snapshot.blockedVolumes, (std::vector<app::OverlayHormBlockedVolume>{{QString::fromStdString(firstVolume), QStringLiteral("Z:\\MountedData")},
                                                                                  {QString::fromStdString(secondVolume), QStringLiteral("V:")}}));
    QCOMPARE(fileReads, 1);
    QCOMPARE(volumeReads, 1);
    QCOMPARE(system->writes, 0);
    // The configured D: row is unprotected, but the independent runtime check
    // can establish that all volumes which actually need protection are safe.
    system->published.runtime.hormVolumesProtected = compat::Fact<bool>::known(true);
    const auto safe = app::uwfOverlayMaintenance(system, {}, readyReadOnlyPorts())->observe();
    QCOMPARE(safe.allVolumesProtected, std::optional<bool>{true});
    QVERIFY(safe.blockedVolumes.empty());
  }
  void prerequisiteReadErrorsRemainSeparateAndDoNotTurnIntoFalse() {
    auto system = std::make_shared<MaintenanceUwf>();
    system->published.runtime.hormVolumesProtected = {};
    system->published.runtime.romDisposition = {};
    auto ports = readyReadOnlyPorts();
    ports.hibernationCapabilities = [](std::stop_token) -> app::OverlayHibernationCapabilities {
      throw std::runtime_error("Hibernation-capability query failed");
    };
    ports.hormVolumeProtection = [](std::stop_token) -> app::OverlayHormVolumeProtection { throw std::runtime_error("Runtime volume query failed"); };

    const auto snapshot = app::uwfOverlayMaintenance(system, {}, std::move(ports))->observe();
    QVERIFY(!snapshot.hibernationAvailable.has_value());
    QVERIFY(!snapshot.allVolumesProtected.has_value());
    QVERIFY(!snapshot.romDisposition.has_value());
    QCOMPARE(snapshot.hibernationError, QStringLiteral("Hibernation-capability query failed"));
    QCOMPARE(snapshot.allVolumesProtectedError, QStringLiteral("Runtime volume query failed"));
    // Optional host facts must not turn unrelated operations or HORM disable
    // into a failed general refresh. Their own rows retain precise errors.
    QVERIFY(snapshot.readError.isEmpty());
    QCOMPARE(system->writes, 0);
  }
  void blockedVolumeIdentitiesRemainStructuredWhenNamesAreMissing() {
    auto system = std::make_shared<MaintenanceUwf>();
    system->published.runtime.hormVolumesProtected = compat::Fact<bool>::known(false);
    auto ports = readyReadOnlyPorts();
    ports.hormVolumeProtection = [](std::stop_token) {
      app::OverlayHormVolumeProtection result{false, {}, {}};
      result.blockedVolumes = {{QString::fromStdString(compat::volumePath(firstVolume)), QStringLiteral("D:")},
                               {QString::fromStdString(secondVolume), QStringLiteral("F:")},
                               {QStringLiteral("Volume{AAAAAAAA-BBBB-CCCC-DDDD-EEEEEEEEEEEE}"), {}}};
      return result;
    };
    const auto snapshot = app::uwfOverlayMaintenance(system, {}, std::move(ports))->observe();
    QCOMPARE(snapshot.allVolumesProtected, std::optional<bool>{false});
    QCOMPARE(snapshot.blockedVolumes.size(), std::size_t{3});
    QCOMPARE(snapshot.blockedVolumes[0].volumeId, QString::fromStdString(firstVolume));
    QCOMPARE(snapshot.blockedVolumes[0].name, QStringLiteral("D:"));
    QCOMPARE(snapshot.blockedVolumes[1].volumeId, QString::fromStdString(secondVolume));
    QCOMPARE(snapshot.blockedVolumes[1].name, QStringLiteral("F:"));
    QCOMPARE(snapshot.blockedVolumes[2].volumeId, QStringLiteral("aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeeee"));
    QVERIFY(snapshot.blockedVolumes[2].name.isEmpty());
    QVERIFY(snapshot.allVolumesProtectedError.isEmpty());
    QCOMPARE(system->writes, 0);
  }
  void blockedVolumeObservationsEnforceIdentityAndProtectionConsistency_data() {
    QTest::addColumn<bool>("allProtected");
    QTest::addColumn<bool>("validIdentity");
    for (const bool allProtected : {false, true})
      for (const bool validIdentity : {false, true})
        QTest::newRow(qPrintable(QStringLiteral("protected-%1/valid-identity-%2").arg(allProtected).arg(validIdentity))) << allProtected << validIdentity;
  }
  void blockedVolumeObservationsEnforceIdentityAndProtectionConsistency() {
    QFETCH(bool, allProtected);
    QFETCH(bool, validIdentity);
    auto system = std::make_shared<MaintenanceUwf>();
    system->published.runtime.hormVolumesProtected = compat::Fact<bool>::known(false);
    auto ports = readyReadOnlyPorts();
    ports.hormVolumeProtection = [=](std::stop_token) {
      return app::OverlayHormVolumeProtection{
          allProtected, {{validIdentity ? QString::fromStdString(firstVolume) : QStringLiteral("D:\\Mounted{cache}"), QStringLiteral("Display name")}}, {}};
    };
    const auto snapshot = app::uwfOverlayMaintenance(system, {}, std::move(ports))->observe();
    const bool accepted = !allProtected && validIdentity;
    QCOMPARE(snapshot.allVolumesProtected, std::optional<bool>{false});
    QCOMPARE(snapshot.allVolumesProtectedError.isEmpty(), accepted);
    QCOMPARE(snapshot.blockedVolumes.size(), accepted ? std::size_t{1} : std::size_t{0});
    if (accepted) QCOMPARE(snapshot.blockedVolumes.front().volumeId, QString::fromStdString(firstVolume));
    QCOMPARE(system->writes, 0);
  }
  void aHibernationFileAloneCannotProveHibernateIsAvailable() {
    auto system = std::make_shared<MaintenanceUwf>();
    auto ports = readyReadOnlyPorts();
    ports.hibernationCapabilities = {};

    const auto unknown = app::uwfOverlayMaintenance(system, {}, ports)->observe();
    QVERIFY(!unknown.hibernationAvailable.has_value());
    QVERIFY(!unknown.hibernationCapabilities.has_value());
    QVERIFY(!unknown.hibernationError.isEmpty());
    for (const auto capabilities : {app::OverlayHibernationCapabilities{false, true, app::HibernationFileType::Full},
                                    app::OverlayHibernationCapabilities{true, false, app::HibernationFileType::Full},
                                    app::OverlayHibernationCapabilities{true, true, app::HibernationFileType::Reduced},
                                    app::OverlayHibernationCapabilities{true, true, app::HibernationFileType::None}}) {
      ports.hibernationCapabilities = [capabilities](std::stop_token) { return capabilities; };
      system->published.runtime.hiberFile = compat::Fact<bool>::known(capabilities.hiberFilePresent);
      const auto unavailable = app::uwfOverlayMaintenance(system, {}, ports)->observe();
      QCOMPARE(unavailable.hibernationAvailable, std::optional<bool>{false});
      QVERIFY(unavailable.hibernationCapabilities.has_value());
      QVERIFY(unavailable.hibernationError.isEmpty());
    }
    QCOMPARE(system->writes, 0);
  }
  void hormCommandsDoNotRefreshTheirHostObservationPorts() {
    auto system = std::make_shared<MaintenanceUwf>();
    auto ports = readyReadOnlyPorts();
    int hostReads = 0;

    ports.hormVolumeProtection = [&](std::stop_token) {
      ++hostReads;
      return app::OverlayHormVolumeProtection{};
    };

    ports.hibernationCapabilities = [&](std::stop_token) {
      ++hostReads;
      return app::OverlayHibernationCapabilities{};
    };
    const auto result = app::uwfOverlayMaintenance(system, {}, std::move(ports))->setHorm(true);
    QVERIFY(result.succeeded());
    QCOMPARE(result.effect, app::WriteEffect::Confirmed);
    QCOMPARE(system->hormWrites, 1);
    QCOMPARE(system->reads, 0);
    QCOMPARE(hostReads, 0);
  }
  void disableDoesNotInheritFailedEnablePrerequisiteReads() {
    auto system = std::make_shared<MaintenanceUwf>();
    system->horm = true;
    auto ports = readyReadOnlyPorts();

    ports.hibernationCapabilities = [](std::stop_token) -> app::OverlayHibernationCapabilities { throw std::runtime_error("No power-capability observation"); };
    ports.hormVolumeProtection = [](std::stop_token) -> app::OverlayHormVolumeProtection { throw std::runtime_error("No volume observation"); };

    const auto result = app::uwfOverlayMaintenance(system, {}, std::move(ports))->setHorm(false);
    QVERIFY(result.succeeded());
    QCOMPARE(result.snapshot.hormEnabled, std::optional<bool>{false});
    QVERIFY(!result.snapshot.hibernationAvailable.has_value());
    QCOMPARE(system->hormWrites, 1);
  }
  void failedExplicitHostRefreshDoesNotCauseAnotherReadDuringWrite() {
    auto system = std::make_shared<MaintenanceUwf>();
    auto ports = readyReadOnlyPorts();
    int hostReads = 0;
    ports.hibernationCapabilities = [&](std::stop_token) -> app::OverlayHibernationCapabilities {
      ++hostReads;
      throw std::runtime_error("Hibernation capability observation failed");
    };
    const auto actions = app::uwfOverlayMaintenance(system, {}, std::move(ports));
    const auto observed = actions->observe();
    QVERIFY(observed.hibernationError.contains(QStringLiteral("Hibernation capability observation failed")));
    QCOMPARE(hostReads, 1);
    QCOMPARE(system->state().runtime.romDisposition.valueBits, std::uint32_t{0});
    const auto reads = system->reads;
    const auto result = actions->setHorm(true);
    QVERIFY(result.succeeded());
    QCOMPARE(result.effect, app::WriteEffect::Confirmed);
    QCOMPARE(system->hormWrites, 1);
    QCOMPARE(system->reads, reads);
    QCOMPARE(hostReads, 1);
  }
  void storageDefaultUsesTheObservedSystemDriveInsteadOfTheFirstCandidate() {
    auto system = std::make_shared<MaintenanceUwf>();
    system->currentType = system->nextType = 1;
    system->currentHosts = system->nextHosts = {{firstVolume, 0}, {secondVolume, 0}};
    system->published.systemVolumeId = compat::Fact<std::string>::known(secondVolume);
    auto volumes = std::make_shared<MaintenanceVolumes>();
    auto ports = readyReadOnlyPorts();

    const auto snapshot = app::uwfOverlayMaintenance(system, volumes, std::move(ports))->observe();
    QCOMPARE(snapshot.volumes.front().driveLetter, QStringLiteral("F:"));
    QCOMPARE(snapshot.systemDrive, QStringLiteral("S:"));
    QCOMPARE(snapshot.currentStorage.origin, app::OverlayStorageOrigin::SystemDriveDefault);
    QCOMPARE(snapshot.nextStorage.origin, app::OverlayStorageOrigin::SystemDriveDefault);
    QCOMPARE(snapshot.currentStorage.volumeId, std::optional<QString>{QString::fromStdString(secondVolume)});
    QCOMPARE(snapshot.nextStorage.driveLetter, QStringLiteral("S:"));
    QCOMPARE(system->writes, 0);
  }
  void configuredHostAndRamStateStayDistinctFromTheDefault() {
    auto system = std::make_shared<MaintenanceUwf>();
    system->currentType = 0;
    system->nextType = 1;
    system->nextHosts = {{firstVolume, 7}, {secondVolume, 0}};
    auto volumes = std::make_shared<MaintenanceVolumes>();
    auto ports = readyReadOnlyPorts();

    const auto snapshot = app::uwfOverlayMaintenance(system, volumes, std::move(ports))->observe();
    QCOMPARE(snapshot.currentStorage.origin, app::OverlayStorageOrigin::NotUsed);
    QVERIFY(!snapshot.currentStorage.volumeId.has_value());
    QCOMPARE(snapshot.nextStorage.origin, app::OverlayStorageOrigin::ConfiguredMarker);
    QCOMPARE(snapshot.nextStorage.driveLetter, QStringLiteral("F:"));
    QCOMPARE(snapshot.nextStorage.volumeId, std::optional<QString>{QString::fromStdString(firstVolume)});
    QCOMPARE(system->writes, 0);
  }
  void incompleteStorageReadsNeverBecomeAnAssumedDefault() {
    for (const bool listFailure : {false, true}) {
      auto system = std::make_shared<MaintenanceUwf>();
      system->currentType = system->nextType = 1;
      system->currentHosts = system->nextHosts = {{firstVolume, 0}, {secondVolume, 0}};
      system->nextVolumesUnknown = listFailure;
      system->nextMarkerUnknown = !listFailure;
      // A readable earlier host must not escape when a later marker is unknown.
      if (!listFailure) system->nextHosts.at(firstVolume) = 1;
      const auto snapshot = app::uwfOverlayMaintenance(system, std::make_shared<MaintenanceVolumes>(), readyReadOnlyPorts())->observe();
      QCOMPARE(snapshot.nextStorage.origin, app::OverlayStorageOrigin::Unknown);
      QCOMPARE(snapshot.nextStorage.issue, listFailure ? app::OverlayStorageIssue::ObservationUnavailable : app::OverlayStorageIssue::MarkerUnavailable);
      QVERIFY(!snapshot.nextStorage.diagnostics.isEmpty());
      QVERIFY(!snapshot.nextStorage.volumeId.has_value());
      QCOMPARE(system->writes, 0);
    }
  }
  void storageLocationFailuresKeepTheirMeaningSeparateFromDiagnostics() {
    for (const auto issue :
         {app::OverlayStorageIssue::OverlayTypeUnknown, app::OverlayStorageIssue::MultipleHosts, app::OverlayStorageIssue::SystemDriveUnavailable}) {
      auto system = std::make_shared<MaintenanceUwf>();
      if (issue == app::OverlayStorageIssue::SystemDriveUnavailable) system->published.systemVolumeId = {};
      system->nextType = issue == app::OverlayStorageIssue::OverlayTypeUnknown ? 2U : 1U;
      system->nextHosts = issue == app::OverlayStorageIssue::MultipleHosts ? std::map<std::string, std::uint32_t>{{firstVolume, 1}, {secondVolume, 1}}
                                                                           : std::map<std::string, std::uint32_t>{{firstVolume, 0}, {secondVolume, 0}};
      auto volumes = std::make_shared<MaintenanceVolumes>();
      volumes->system.clear();
      auto ports = readyReadOnlyPorts();

      const auto snapshot = app::uwfOverlayMaintenance(system, volumes, std::move(ports))->observe();
      QCOMPARE(snapshot.nextStorage.origin, app::OverlayStorageOrigin::Unknown);
      QCOMPARE(snapshot.nextStorage.issue, issue);
      QVERIFY(snapshot.nextStorage.diagnostics.isEmpty());
      QCOMPARE(system->writes, 0);
    }
  }
  void configuredHostsMissingFromPhysicalInventoryAreDisplayOnly() {
    auto system = std::make_shared<MaintenanceUwf>();
    system->nextType = 1;
    system->nextHosts = {{firstVolume, 0}, {secondVolume, 1}};
    auto volumes = std::make_shared<MaintenanceVolumes>();
    volumes->values.resize(1);
    const auto snapshot = app::uwfOverlayMaintenance(system, volumes, readyReadOnlyPorts())->observe();
    QCOMPARE(snapshot.volumes.size(), std::size_t{2});
    QVERIFY(snapshot.volumes[0].eligible);
    QVERIFY(!snapshot.volumes[1].eligible);
    QCOMPARE(snapshot.nextStorage.origin, app::OverlayStorageOrigin::ConfiguredMarker);
    QCOMPARE(snapshot.nextStorage.volumeId, std::optional<QString>{QString::fromStdString(secondVolume)});
    QCOMPARE(system->writes, 0);
    // The compatibility-only fixture has no physical inventory contract.
    const auto compatibilityOnly = app::uwfOverlayMaintenance(system)->observe();
    QVERIFY(compatibilityOnly.volumes[1].eligible);
  }
};
}  // namespace
QTEST_APPLESS_MAIN(OverlayMaintenanceTests)
#include "OverlayMaintenanceTests.moc"
