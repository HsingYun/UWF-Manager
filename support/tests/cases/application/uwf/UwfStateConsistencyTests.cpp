// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QScopeGuard>
#include <QTest>
#include <algorithm>
#include <cstdint>
#include <exception>
#include <future>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "UwfStateConsistencyFixture.h"
#include "application/uwf/UwfRequestPrecheck.h"
#include "application/uwf/UwfStateUpdates.h"
#include "application/uwf/adapters/UwfReadState.h"
#include "simulation/uwf/FakeUwfBackend.h"

namespace {
using namespace uwf;
using namespace uwf::compat;
using Fake = uwf::simulation::FakeUwfBackend;
using namespace std::chrono_literals;
using namespace uwf::test::state_consistency;
#define VERIFY_STATE(fixture)                     \
  do {                                            \
    const auto mismatch = (fixture).difference(); \
    QVERIFY2(mismatch.empty(), mismatch.c_str()); \
  } while (false)

Fake::Fault rejected(const Request& request) {
  return {.request = request,
          .operation = {},
          .timing = Fake::FaultTiming::Before,
          .kind = ErrorKind::AccessDenied,
          .message = "Second primitive was rejected",
          .code = 0x80070005};
}

class UwfStateConsistencyTests final : public QObject {
  Q_OBJECT
 private slots:
  void overlayTransitionsPreservePartialFlagKnowledge_data() {
    QTest::addColumn<int>("beforeType");
    QTest::addColumn<int>("operation");
    QTest::addColumn<std::uint32_t>("knownMask");
    QTest::addColumn<std::uint32_t>("valueBits");
    QTest::addColumn<std::uint32_t>("expectedMask");
    QTest::addColumn<std::uint32_t>("expectedBits");
    constexpr std::uint32_t opaque = 0x80000000u;
    constexpr auto passthrough = flags::passthrough;
    constexpr auto persistent = flags::persistent;
    constexpr auto media = flags::readOnlyMedia;
    constexpr auto all = opaque | passthrough | persistent | media;
    const auto add = [&](const char* name, const int before, const int operation, const std::uint32_t known, const std::uint32_t value,
                         const std::uint32_t expectedKnown,
                         const std::uint32_t expectedValue) { QTest::newRow(name) << before << operation << known << value << expectedKnown << expectedValue; };
    // Operations 0/1 set RAM/Disk; 2 selects a storage host.
    add("ram-noop", 0, 0, opaque | persistent | media, opaque | persistent, opaque | persistent | media, opaque | persistent);
    add("disk-noop", 1, 1, all, opaque | persistent | media, all, opaque | persistent | media);
    add("disk-to-ram", 1, 0, opaque | media, opaque | media, all, opaque | media);
    add("ram-to-disk", 0, 1, opaque | media, opaque | media, opaque | passthrough | media, opaque | passthrough | media);
    add("storage-from-ram", 0, 2, opaque | persistent | media, opaque | persistent, all, opaque | passthrough | persistent);
    add("storage-from-disk", 1, 2, opaque | persistent | media, opaque | persistent, all, opaque | passthrough);
    add("unobserved-type", -1, 0, opaque | persistent | media, opaque | persistent, opaque | persistent | media, opaque | persistent);
    add("storage-from-unobserved-type", -1, 2, opaque | media, opaque | media, opaque | passthrough | media, opaque | passthrough | media);
    add("storage-from-unrecognized-type", 9, 2, opaque | persistent | media, opaque | persistent, all, opaque | passthrough | persistent);
  }
  void overlayTransitionsPreservePartialFlagKnowledge() {
    QFETCH(int, beforeType);
    QFETCH(int, operation);
    QFETCH(std::uint32_t, knownMask);
    QFETCH(std::uint32_t, valueBits);
    QFETCH(std::uint32_t, expectedMask);
    QFETCH(std::uint32_t, expectedBits);
    UwfStateFacts state;
    if (beforeType >= 0) state.next.overlayType = Fact<std::uint32_t>::known(static_cast<std::uint32_t>(beforeType));
    state.next.overlayFlags = {knownMask, valueBits};
    const bool storage = operation == 2;
    const Request request = storage ? Request{CreateSwapfile{volumeC}} : Request{SetNumber{NumberProperty::OverlayType, static_cast<std::uint32_t>(operation)}};
    const auto update = app::recordUwfState(state, request, Response::success());
    QCOMPARE(update, storage || beforeType < 0 ? app::UwfStateUpdate::RefreshRequired : app::UwfStateUpdate::Complete);
    QCOMPARE(state.next.overlayType.value, std::optional<std::uint32_t>{storage ? 1u : static_cast<std::uint32_t>(operation)});
    QCOMPARE(state.next.overlayFlags.knownMask, expectedMask);
    QCOMPARE(state.next.overlayFlags.valueBits, expectedBits);
    QVERIFY(state.next.overlayFlags.valid());
  }
  void filterChangesPublishTheirResetAndRomEffects_data() {
    QTest::addColumn<int>("mode");
    QTest::newRow("loaded-persistent-disk") << 0;
    QTest::newRow("loaded-rom") << 1;
    QTest::newRow("offline-disk-without-control") << 2;
  }
  void filterChangesPublishTheirResetAndRomEffects() {
    QFETCH(int, mode);
    auto initial = initialState();
    initial.current.filterEnabled = mode != 2;
    initial.current.overlayType = mode == 1 ? static_cast<std::uint32_t>(OverlayType::Ram) : static_cast<std::uint32_t>(OverlayType::Disk);
    initial.current.overlayFlags = mode == 1 ? flags::readOnlyMedia : mode == 2 ? 0x80000000u : 0x80000002u;
    initial.next = initial.current;
    initial.runtime.controlDevicePresent = mode != 2;
    Fixture fixture(std::move(initial));
    VERIFY_STATE(fixture);
    const bool desired = mode == 2;
    const auto first = fixture.system.execute(SetBoolean{BooleanProperty::FilterEnabled, desired});
    QVERIFY2(first.succeeded(), first.failure ? first.failure->message.c_str() : "");
    VERIFY_STATE(fixture);
    const auto second = fixture.system.execute(SetBoolean{BooleanProperty::FilterEnabled, !desired});
    QVERIFY2(second.succeeded(), second.failure ? second.failure->message.c_str() : "");
    VERIFY_STATE(fixture);
    QCOMPARE(fixture.refreshes->value, mode == 2 ? 0 : 2);
    if (mode == 2)
      for (const auto& request : fixture.native->primitiveRequests()) QVERIFY(isWrite(request));
  }
  void successfulResetAcknowledgementDoesNotInventTheDriverCache_data() {
    QTest::addColumn<bool>("filter");
    QTest::newRow("explicit-reset-clear") << false;
    QTest::newRow("filter-implicit-reset") << true;
  }
  void successfulResetAcknowledgementDoesNotInventTheDriverCache() {
    QFETCH(bool, filter);
    auto initial = initialState();
    initial.current.filterEnabled = true;
    initial.next = initial.current;
    initial.runtime.resetPersistentState = 0;
    initial.runtime.persistentRollbackState = 0;
    initial.runtime.persistentResetHeaderWriteFails = true;
    Fixture fixture(std::move(initial));
    const Request request = filter ? Request{SetBoolean{BooleanProperty::FilterEnabled, false}}
                                   : Request{SetNumber{NumberProperty::ResetPersistentState, static_cast<std::uint32_t>(ResetMode::Clear)}};
    const auto response = fixture.system.execute(request);
    QVERIFY2(response.succeeded(), response.failure ? response.failure->message.c_str() : "");
    QCOMPARE(response.effect, Effect::Confirmed);
    QCOMPARE(fixture.writes(), std::vector<Request>{request});
    QCOMPARE(fixture.refreshes->value, 1);
    const auto native = fixture.native->snapshot();
    // The filter changes its private in-memory intent before the header failure;
    // the global driver swallows that error and keeps its public cache at zero.
    QCOMPARE(native.runtime.persistentRollbackState, std::uint32_t{1});
    QCOMPARE(native.runtime.resetPersistentState, std::uint32_t{0});
    QCOMPARE(fixture.system.state().runtime.resetState.value, std::optional<std::uint32_t>{0});
    QVERIFY(!fixture.system.state().refreshFailure);
    QCOMPARE(std::ranges::count(fixture.native->primitiveRequests(), Request{GetNumber{NumberProperty::ResetPersistentState}}), 1);
    if (filter) QCOMPARE(fixture.system.state().next.filterEnabled.value, std::optional{false});
    VERIFY_STATE(fixture);
  }
  void failedResetObservationRetainsKnownStateAndRequiresARealRecoveryRefresh() {
    auto initial = initialState();
    initial.current.filterEnabled = true;
    initial.next = initial.current;
    initial.runtime.resetPersistentState = 0;
    Fixture fixture(std::move(initial));
    const Request reset = SetNumber{NumberProperty::ResetPersistentState, static_cast<std::uint32_t>(ResetMode::Clear)};
    fixture.native->inject(rejected(GetNumber{NumberProperty::ResetPersistentState}));
    const auto response = fixture.system.execute(reset);
    QVERIFY(response.failure);
    QCOMPARE(response.effect, Effect::Confirmed);
    QCOMPARE(response.failure->effect, Effect::Confirmed);
    QVERIFY(response.failure->message.find("write completed") != std::string::npos);
    QCOMPARE(fixture.native->snapshot().runtime.resetPersistentState, std::uint32_t{1});
    QCOMPARE(fixture.system.state().runtime.resetState.value, std::optional<std::uint32_t>{0});
    QVERIFY(fixture.system.state().refreshFailure);
    const auto before = fixture.native->trace().size();
    const auto blocked = fixture.system.execute(SetNumber{NumberProperty::CriticalThreshold, 900});
    QVERIFY(blocked.failure);
    QCOMPARE(blocked.effect, Effect::None);
    QCOMPARE(fixture.native->trace().size(), before);
    QVERIFY(fixture.system.refreshState().succeeded());
    QVERIFY(!fixture.system.state().refreshFailure);
    QCOMPARE(fixture.system.state().runtime.resetState.value, std::optional<std::uint32_t>{1});
    QVERIFY(fixture.system.execute(SetNumber{NumberProperty::CriticalThreshold, 900}).succeeded());
    QCOMPARE(fixture.writes(), (std::vector<Request>{reset, SetNumber{NumberProperty::CriticalThreshold, 900}}));
    VERIFY_STATE(fixture);
  }
  void hormDisablePreservesDispositionThreeAndBlocksFullCommitBeforeDispatch() {
    auto initial = initialState();
    initial.current.filterEnabled = true;
    initial.current.overlayType = static_cast<std::uint32_t>(OverlayType::Ram);
    initial.current.overlayFlags = flags::readOnlyMedia;
    for (auto& [id, volume] : initial.current.volumes) {
      (void)id;
      volume.volume.protectedVolume = true;
    }
    initial.next = initial.current;
    Fixture fixture(std::move(initial));
    VERIFY_STATE(fixture);
    QVERIFY(fixture.system.execute(SetBoolean{BooleanProperty::HormEnabled, true}).succeeded());
    VERIFY_STATE(fixture);
    QVERIFY(fixture.system.execute(SetBoolean{BooleanProperty::HormEnabled, false}).succeeded());
    VERIFY_STATE(fixture);
    QCOMPARE(fixture.system.state().runtime.romDisposition.valueBits, std::uint32_t{3});
    const auto before = fixture.native->primitiveRequests();
    const auto fullCommit = fixture.system.execute(SetBoolean{BooleanProperty::OverlayCommit, true});
    QVERIFY(fullCommit.failure);
    QCOMPARE(fullCommit.failure->kind, ErrorKind::Conflict);
    QCOMPARE(fullCommit.effect, Effect::None);
    QCOMPARE(fixture.native->primitiveRequests(), before);
    QCOMPARE(fixture.refreshes->value, 0);
    VERIFY_STATE(fixture);
  }
  void readingResetZeroDoesNotInventAnAvailableControlDevice() {
    auto initial = initialState();
    initial.runtime.controlDevicePresent = false;
    Fixture fixture(std::move(initial));
    const auto response = fixture.system.execute(GetNumber{NumberProperty::ResetPersistentState, Session::Current});
    QVERIFY(response.succeeded());
    QCOMPARE(std::get<std::uint32_t>(response.value), std::uint32_t{0});
    QCOMPARE(fixture.system.state().runtime.controlDevicePresent.value, std::optional{false});
    QCOMPARE(fixture.native->primitiveRequests().size(), std::size_t{1});
    QCOMPARE(fixture.refreshes->value, 0);
    VERIFY_STATE(fixture);
  }
  void fullCommitPublishesConfigurationAndExplicitUsageObservesNativeCopies() {
    auto initial = initialState();
    initial.current.filterEnabled = true;
    initial.current.overlayType = static_cast<std::uint32_t>(OverlayType::Ram);
    initial.current.overlayFlags = flags::readOnlyMedia;
    initial.next = initial.current;
    Fixture fixture(std::move(initial));
    const std::string contents(2 * 1024 * 1024 + 17, 'x');
    fixture.native->seedFile(volumeC, "\\dirty.bin", "before");
    fixture.native->writeFile(volumeC, "\\dirty.bin", contents);
    fixture.system.updateState(observedFacts(*fixture.native));
    VERIFY_STATE(fixture);
    const auto beforeUsage = fixture.system.execute(GetOverlayUsage{});
    QVERIFY(beforeUsage.succeeded());
    QVERIFY(std::get<OverlayUsage>(beforeUsage.value).consumptionMb > 0);
    const Request request = SetBoolean{BooleanProperty::OverlayCommit, true};
    const auto response = fixture.system.execute(request);
    QVERIFY2(response.succeeded(), response.failure ? response.failure->message.c_str() : "");
    QCOMPARE(fixture.refreshes->value, 1);
    QCOMPARE(fixture.writes(), std::vector<Request>{request});
    VERIFY_STATE(fixture);
    const auto state = fixture.system.state();
    QCOMPARE(state.runtime.fullCommit.value, std::optional{true});
    const auto afterUsage = fixture.system.execute(GetOverlayUsage{});
    QVERIFY(afterUsage.succeeded());
    QCOMPARE(std::get<OverlayUsage>(afterUsage.value).consumptionMb, std::uint32_t{0});
    QCOMPARE(std::ranges::count_if(fixture.native->trace(), [](const auto& entry) { return std::holds_alternative<GetOverlayUsage>(entry.request); }), 2);
    const auto durable = fixture.native->readFile(volumeC, "\\dirty.bin");
    QVERIFY(durable && *durable == contents);
  }
  void emptyCreationContextCannotReviveOldRegistrationMetadata_data() {
    QTest::addColumn<int>("route");
    QTest::newRow("remove-then-add") << 0;
    QTest::newRow("inventory-disappearance-then-add") << 1;
    QTest::newRow("remove-then-protect-composition") << 2;
  }
  void emptyCreationContextCannotReviveOldRegistrationMetadata() {
    QFETCH(int, route);
    auto initial = initialState();
    initial.next.volumes.at(volumeC).volume.bindByDriveLetter = true;
    initial.next.volumes.at(volumeC).swapfileSize = 9;
    initial.next.volumes.at(volumeC).fileExclusions = {"\\OldRegistration"};
    Fixture fixture(std::move(initial));
    std::vector<Request> expected;
    if (route == 1) {
      auto external = fixture.native->snapshot();
      external.next.volumes.erase(volumeC);
      fixture.native->seed(std::move(external));
      QVERIFY(fixture.system.execute(GetVolumes{Session::Next}).succeeded());
    } else {
      const Request remove = RemoveVolume{volumeC};
      QVERIFY(fixture.system.execute(remove).succeeded());
      expected.push_back(remove);
    }
    VERIFY_STATE(fixture);
    QCOMPARE(fixture.refreshes->value, 0);
    const Request add = AddVolume{volumeC, {}};
    if (route == 2) {
      const Request protect = SetVolume{volumeC, VolumeProperty::Protected, true};
      QVERIFY(fixture.system.execute(protect).succeeded());
      expected.push_back(add);
      expected.push_back(protect);
      expected.push_back(SetVolume{volumeC, VolumeProperty::BindByDriveLetter, false});
    } else {
      QVERIFY(fixture.system.execute(add).succeeded());
      expected.push_back(add);
    }
    QCOMPARE(fixture.writes(), expected);
    QCOMPARE(fixture.refreshes->value, 1);
    VERIFY_STATE(fixture);
    const auto published = fixture.system.state();
    const auto& registered = published.next.volumes.at(volumeC);
    QCOMPARE(registered.present.value, std::optional{true});
    QCOMPARE(registered.driveLetter.value, std::optional<std::string>{""});
    QCOMPARE(registered.physicalDriveLetter.value, std::optional<std::string>{"C:"});
    QCOMPARE(registered.bindByDriveLetter.value, std::optional{false});
    QCOMPARE(registered.swapfileMarker.value, std::optional<std::uint32_t>{0});
    QVERIFY(registered.fileExclusions.value && registered.fileExclusions.value->empty());
    core::UwfSnapshot projection;
    try {
      const auto sample = fixture.system.execute(GetOverlayUsage{});
      QVERIFY(sample.succeeded());
      projection = app::publishedUwfSnapshot(published, app::UwfCapability::Available, true, std::get<OverlayUsage>(sample.value));
    } catch (const std::exception& error) {
      QFAIL(error.what());
    }
    const auto row = std::ranges::find_if(projection.next.volumes, [](const auto& volume) { return normalizeVolumeId(volume.volumeName) == volumeC; });
    QVERIFY(row != projection.next.volumes.end());
    QVERIFY(row->driveLetter.empty());
    QVERIFY(projection.next.fileExclusions.at(row->volumeName).empty());
  }
  void resetDefaultsRefreshesEveryChangedPrimaryObservation() {
    auto initial = initialState();
    initial.next.filterEnabled = true;
    initial.next.persistDomainSecretKey = initial.next.persistTscal = true;
    initial.next.overlayFlags = 0x80000003u;
    initial.next.registryExclusions = {"HKEY_LOCAL_MACHINE\\SOFTWARE\\BeforeReset"};
    auto defaults = Fake::Configuration{};
    defaults.maximumOverlaySize = 512;
    defaults.warningThreshold = 10;
    defaults.criticalThreshold = 20;
    defaults.overlayFlags = 0x80;
    defaults.resetPersistentStateSavedMode = 1;
    defaults.volumes.emplace(volumeC, Fake::VolumeConfiguration{.volume = {volumeC, "C:", false, true, false}, .fileExclusions = {"\\Default"}});
    defaults.registryExclusions = {"HKLM\\SOFTWARE\\Defaults"};
    initial.capturedSettings = std::move(defaults);
    Fixture fixture(std::move(initial));
    VERIFY_STATE(fixture);
    const auto response = fixture.system.execute(ResetSettings{});
    QVERIFY2(response.succeeded(), response.failure ? response.failure->message.c_str() : "");
    QCOMPARE(fixture.refreshes->value, 1);
    VERIFY_STATE(fixture);
    QCOMPARE(fixture.system.state().runtime.warningThreshold.value, std::optional<std::uint32_t>{10});
    QCOMPARE(fixture.system.state().runtime.criticalThreshold.value, std::optional<std::uint32_t>{20});
    QCOMPARE(fixture.system.state().next.overlayFlags.valueBits, std::uint32_t{0x80});
  }
  void servicingRefreshesItsSavedFilterAndRestorationEffects() {
    auto initial = initialState();
    initial.next.filterEnabled = true;
    initial.runtime.controlDevicePresent = false;
    Fixture fixture(std::move(initial));
    QVERIFY(fixture.system.execute(SetBoolean{BooleanProperty::ServicingEnabled, true}).succeeded());
    QCOMPARE(fixture.refreshes->value, 1);
    VERIFY_STATE(fixture);
    QVERIFY(fixture.system.execute(SetBoolean{BooleanProperty::ServicingEnabled, false}).succeeded());
    QCOMPARE(fixture.refreshes->value, 2);
    VERIFY_STATE(fixture);
  }
  void partialClearKeepsAcknowledgedRemovalsAndRetriesOnlyRemainingEntries_data() {
    QTest::addColumn<bool>("registry");
    QTest::newRow("registry") << true;
    QTest::newRow("file") << false;
  }
  void partialClearKeepsAcknowledgedRemovalsAndRetriesOnlyRemainingEntries() {
    QFETCH(bool, registry);
    auto initial = initialState();
    const std::vector<std::string> entries = registry ? std::vector<std::string>{"HKLM\\SOFTWARE\\Alpha", "HKLM\\SOFTWARE\\Beta", "HKLM\\SOFTWARE\\Gamma"}
                                                      : std::vector<std::string>{"\\Alpha", "\\Beta", "\\Gamma"};
    if (registry)
      initial.next.registryExclusions = entries;
    else
      initial.next.volumes.at(volumeC).fileExclusions = entries;
    Fixture fixture(std::move(initial));
    const auto remove = [&](const std::string& entry) -> Request {
      return registry ? Request{EditRegistryExclusion{entry, ExclusionAction::Remove}} : Request{EditFileExclusion{volumeC, entry, ExclusionAction::Remove}};
    };
    const Request clear =
        registry ? Request{EditRegistryExclusion{{}, ExclusionAction::Clear}} : Request{EditFileExclusion{volumeC, {}, ExclusionAction::Clear}};
    fixture.native->inject(rejected(remove(entries[1])));
    const auto partial = fixture.system.execute(clear);
    QVERIFY(partial.failure);
    QCOMPARE(partial.failure->kind, ErrorKind::AccessDenied);
    QVERIFY(partial.failure->message.find("Second primitive was rejected") != std::string::npos);
    QCOMPARE(partial.effect, Effect::Confirmed);
    QCOMPARE(fixture.writes(), (std::vector<Request>{remove(entries[0]), remove(entries[1])}));
    QCOMPARE(std::ranges::count(fixture.native->primitiveRequests(), remove(entries[1])), 0);
    QCOMPARE(fixture.refreshes->value, 1);
    VERIFY_STATE(fixture);
    const auto retried = fixture.system.execute(clear);
    QVERIFY2(retried.succeeded(), retried.failure ? retried.failure->message.c_str() : "");
    QCOMPARE(fixture.writes(), (std::vector<Request>{remove(entries[0]), remove(entries[1]), remove(entries[1]), remove(entries[2])}));
    QCOMPARE(fixture.refreshes->value, 1);
    VERIFY_STATE(fixture);
  }
  void registrationSurvivesARejectedProtectionAndBindingIsNeverAttempted() {
    auto initial = initialState();
    initial.next.volumes.erase(volumeC);
    Fixture fixture(std::move(initial));
    const Request protection = SetVolume{volumeC, VolumeProperty::Protected, true};
    fixture.native->inject(rejected(protection));
    const Request intent = SetVolume{volumeC, VolumeProperty::Protected, true, "C:", true};
    const auto partial = fixture.system.execute(intent);
    QVERIFY(partial.failure);
    QCOMPARE(partial.failure->kind, ErrorKind::AccessDenied);
    QVERIFY(partial.failure->message.find("Second primitive was rejected") != std::string::npos);
    QCOMPARE(partial.effect, Effect::Confirmed);
    QCOMPARE(fixture.writes(), (std::vector<Request>{AddVolume{volumeC, "C:"}, protection}));
    QCOMPARE(std::ranges::count(fixture.native->primitiveRequests(), protection), 0);
    QCOMPARE(fixture.refreshes->value, 1);
    VERIFY_STATE(fixture);
    QCOMPARE(fixture.system.state().next.volumes.at(volumeC).present.value, std::optional{true});
    QCOMPARE(fixture.system.state().next.volumes.at(volumeC).protectedVolume.value, std::optional{false});
    QCOMPARE(fixture.system.state().next.volumes.at(volumeC).bindByDriveLetter.value, std::optional{false});
    QVERIFY(fixture.system.execute(intent).succeeded());
    QCOMPARE(fixture.writes(), (std::vector<Request>{AddVolume{volumeC, "C:"}, protection, intent}));
    QCOMPARE(fixture.refreshes->value, 1);
    VERIFY_STATE(fixture);
  }
  void synchronizedRefreshCannotOverwriteAConcurrentSetterWithItsOldCopy() {
    Fixture fixture(initialState());
    std::promise<void> copied;
    std::promise<void> setterAttempted;
    std::promise<void> publish;
    auto copiedFuture = copied.get_future();
    auto attemptedFuture = setterAttempted.get_future();
    auto release = publish.get_future().share();
    std::future<void> refresh;
    std::future<Response> setter;
    bool released = false;
    const auto finish = qScopeGuard([&] {
      if (!released) publish.set_value();
      if (refresh.valid()) refresh.wait();
      if (setter.valid()) setter.wait();
    });
    refresh = std::async(std::launch::async, [&] {
      synchronizedUwf(fixture.system, [&] {
        auto state = fixture.system.state();
        copied.set_value();
        release.wait();
        fixture.system.updateState(std::move(state));
      });
    });
    QVERIFY(copiedFuture.wait_for(5s) == std::future_status::ready);
    setter = std::async(std::launch::async, [&] {
      setterAttempted.set_value();
      return fixture.system.execute(SetNumber{NumberProperty::WarningThreshold, 777});
    });
    QVERIFY(attemptedFuture.wait_for(5s) == std::future_status::ready);
    QVERIFY(setter.wait_for(50ms) == std::future_status::timeout);
    QVERIFY(fixture.native->primitiveRequests().empty());
    publish.set_value();
    released = true;
    QVERIFY(refresh.wait_for(5s) == std::future_status::ready);
    refresh.get();
    QVERIFY(setter.wait_for(5s) == std::future_status::ready);
    const auto response = setter.get();
    QVERIFY(response.succeeded());
    QCOMPARE(fixture.system.state().runtime.warningThreshold.value, std::optional<std::uint32_t>{777});
    VERIFY_STATE(fixture);
  }
};
#undef VERIFY_STATE
}  // namespace

QTEST_APPLESS_MAIN(UwfStateConsistencyTests)
#include "UwfStateConsistencyTests.moc"
