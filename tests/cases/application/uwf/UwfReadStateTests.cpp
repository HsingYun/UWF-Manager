// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QTest>
#include <algorithm>
#include <stdexcept>
#include <system_error>
#include <utility>

#include "application/operations/contracts/OperationError.h"
#include "application/uwf/adapters/UwfReadState.h"
#include "domain/uwf/policies/VolumeIdentityPolicy.h"
#include "support/uwf/FakeUwfBackend.h"

namespace {
using namespace uwf;
using Fake = test::FakeUwfBackend;
using Session = compat::Session;
constexpr auto cVolumeId = "11111111-2222-3333-4444-555555555555";
constexpr auto dVolumeId = "22222222-2222-3333-4444-555555555555";
constexpr auto mountedVolumeId = "44444444-2222-3333-4444-555555555555";
constexpr auto staleVolumeId = "55555555-2222-3333-4444-555555555555";
constexpr auto healthyVolumeId = "66666666-2222-3333-4444-555555555555";
constexpr auto configuredVolumeId = "77777777-2222-3333-4444-555555555555";
constexpr auto retiredVolumeId = "88888888-2222-3333-4444-555555555555";

Fake::VolumeConfiguration volume(std::string id, std::string drive, const bool protectedVolume = false, const bool bindByLetter = false,
                                 std::vector<std::string> exclusions = {}) {
  return {.volume = {std::move(id), std::move(drive), protectedVolume, bindByLetter, false}, .fileExclusions = std::move(exclusions)};
}
Fake::State conflicting(const std::string& stale = staleVolumeId) {
  Fake::State state;
  state.current.volumes.emplace(mountedVolumeId, volume(mountedVolumeId, "C:"));
  state.current.volumes.emplace(stale, volume(stale, "C:"));
  state.current.volumes.emplace(healthyVolumeId, volume(healthyVolumeId, "D:"));
  state.next = state.current;
  return state;
}

class UwfReadStateTests final : public QObject {
  Q_OBJECT
 private slots:
  void publishedSnapshotKeepsSessionDataAndResolvesPhysicalTopology() {
    auto initial = conflicting();
    initial.current.filterEnabled = true;
    initial.current.maximumOverlaySize = 2048;
    initial.current.warningThreshold = 400;
    initial.current.criticalThreshold = 700;
    initial.next = initial.current;
    initial.next.filterEnabled = false;
    initial.next.overlayType = static_cast<std::uint32_t>(compat::OverlayType::Disk);
    initial.next.maximumOverlaySize = 4096;
    initial.next.volumes.at(mountedVolumeId).fileExclusions = {"\\NextOnly"};
    Fake system(initial);
    auto facts = system.state();
    for (auto* session : {&facts.current, &facts.next}) {
      session->volumes[mountedVolumeId].physicalDriveLetter = compat::Fact<std::string>::known("C:");
      session->volumes[staleVolumeId].physicalDriveLetter = compat::Fact<std::string>::known("");
      session->volumes[healthyVolumeId].physicalDriveLetter = compat::Fact<std::string>::known("D:");
    }
    const auto snapshot = app::publishedUwfSnapshot(facts, app::UwfCapability::Available, true, {25, 2023});
    QVERIFY(snapshot.uwfAvailable);
    QVERIFY(snapshot.elevated);
    QVERIFY(snapshot.current.filter.enabled);
    QVERIFY(!snapshot.next.filter.enabled);
    QCOMPARE(snapshot.current.overlay.maximumSizeMb, std::uint32_t{2048});
    QCOMPARE(snapshot.next.overlay.type, core::OverlayType::Disk);
    QCOMPARE(snapshot.next.overlay.maximumSizeMb, std::uint32_t{4096});
    QCOMPARE(snapshot.runtime.currentConsumptionMb, std::uint32_t{25});
    QCOMPARE(snapshot.runtime.availableSpaceMb, std::uint32_t{2023});
    const auto resampled = app::publishedUwfSnapshot(facts, app::UwfCapability::Available, true, {30, 2018});
    QCOMPARE(resampled.runtime.currentConsumptionMb, std::uint32_t{30});
    QCOMPARE(resampled.runtime.availableSpaceMb, std::uint32_t{2018});
    QCOMPARE(snapshot.current.overlay.warningThresholdMb, std::uint32_t{400});
    QCOMPARE(snapshot.next.overlay.criticalThresholdMb, std::uint32_t{700});
    QCOMPARE(snapshot.current.volumes.size(), std::size_t{3});
    QCOMPARE(snapshot.next.volumes.size(), std::size_t{3});
    QCOMPARE(snapshot.next.fileExclusions.at("Volume{" + std::string(mountedVolumeId) + '}'), std::vector<std::string>{"\\NextOnly"});
    QVERIFY(system.trace().empty());
    facts.runtime.warningThreshold = {};
    QVERIFY_THROWS_EXCEPTION(app::OperationError, (void)app::publishedUwfSnapshot(facts, app::UwfCapability::Available, true, {25, 2023}));
    QVERIFY(!app::publishedUwfSnapshot(facts, app::UwfCapability::Unavailable, true, {}).uwfAvailable);
  }
  void physicalMountPublicationPreservesDistinctUwfDriveMetadata() {
    compat::UwfStateFacts state;
    state.current.volumesObserved = state.next.volumesObserved = true;
    state.current.volumes[cVolumeId].driveLetter = compat::Fact<std::string>::known("");
    state.next.volumes[cVolumeId].driveLetter = compat::Fact<std::string>::known("D:");
    core::VolumeInfo physical;
    physical.volumeName = compat::volumePath(cVolumeId);
    physical.driveLetter = "C:";
    app::publishUwfPhysicalState(state, {physical});
    QCOMPARE(state.current.volumes[cVolumeId].driveLetter.value, std::optional<std::string>{""});
    QCOMPARE(state.next.volumes[cVolumeId].driveLetter.value, std::optional<std::string>{"D:"});
    QCOMPARE(state.current.volumes[cVolumeId].physicalDriveLetter.value, std::optional<std::string>{"C:"});
    QCOMPARE(state.next.volumes[cVolumeId].physicalDriveLetter.value, std::optional<std::string>{"C:"});
    physical.driveLetter.clear();
    app::publishUwfPhysicalState(state, {physical});
    QCOMPARE(state.next.volumes[cVolumeId].physicalDriveLetter.value, std::optional<std::string>{""});
    QCOMPARE(state.next.volumes[cVolumeId].driveLetter.value, std::optional<std::string>{"D:"});
  }
  void savedLettersCannotSubstituteForUnobservedPhysicalAddresses() {
    const core::VolumeInfo unidentifiedPhysical{.driveLetter = "C:", .volumeName = {}, .fileSystem = {}, .label = {}};
    for (const auto binding : {core::VolumeBinding::DriveLetter, core::VolumeBinding::VolumeId})
      for (const auto& actual : {std::optional<std::string>{}, std::optional<std::string>{std::string{}}}) {
        core::SessionSnapshot snapshot;
        snapshot.volumes = {{"Volume{" + std::string(cVolumeId) + '}', "C:", true, binding, actual}};
        const auto* expected = &snapshot.volumes.front();
        QVERIFY(core::volumeConfigurationAddress(*expected).empty());
        QVERIFY(!core::findVolumeConfiguration(snapshot, "C:"));
        QVERIFY(!core::findVolumeConfiguration(snapshot, unidentifiedPhysical));
        QCOMPARE(core::findVolumeConfiguration(snapshot, cVolumeId), expected);
      }
  }
  void activeDriverAdmissionCannotBeOverwrittenByPhysicalMediaFacts() {
    compat::UwfStateFacts state;
    state.current.filterEnabled = compat::Fact<bool>::known(true);
    core::VolumeInfo physical;
    physical.volumeName = compat::volumePath(cVolumeId);
    physical.physical = {true, true};
    app::publishUwfPhysicalState(state, {physical});
    QVERIFY(!state.targets[cVolumeId].volumeEligibility.isKnown());
    state.targets[cVolumeId].volumeEligibility = compat::Fact<compat::VolumeAdmission>::known({true, false});
    app::publishUwfPhysicalState(state, {physical});
    QVERIFY(!state.targets[cVolumeId].volumeEligibility.value->protectionEligible);
    state.current.filterEnabled = compat::Fact<bool>::known(false);
    app::publishUwfPhysicalState(state, {physical});
    QVERIFY(state.targets[cVolumeId].volumeEligibility.value->protectionEligible);
  }
  void aliasesSelectTheConfiguredLetterForEachSessionRegardlessOfEnumerationOrder() {
    compat::UwfStateFacts state;
    state.current.filterEnabled = compat::Fact<bool>::known(true);
    state.current.volumes[cVolumeId].driveLetter = compat::Fact<std::string>::known("C:");
    state.next.volumes[cVolumeId].driveLetter = compat::Fact<std::string>::known("D:");
    core::VolumeInfo first;
    first.volumeName = compat::volumePath(cVolumeId);
    first.driveLetter = "C:";
    auto second = first;
    second.driveLetter = "D:";
    for (const auto& aliases : {std::vector{first, second}, std::vector{second, first}}) {
      app::publishUwfPhysicalState(state, aliases);
      QCOMPARE(state.current.volumes[cVolumeId].physicalDriveLetter.value, std::optional<std::string>{"C:"});
      QCOMPARE(state.next.volumes[cVolumeId].physicalDriveLetter.value, std::optional<std::string>{"D:"});
    }
  }
  void startupCapabilityDoesNotConfuseAnEntryPointWithAStateRead_data() {
    QTest::addColumn<compat::Support>("support");
    QTest::newRow("supported") << compat::Support::Supported;
    QTest::newRow("unknown") << compat::Support::Unknown;
    QTest::newRow("temporarily-unavailable") << compat::Support::Unavailable;
    QTest::newRow("unsupported") << compat::Support::Unsupported;
  }
  void startupCapabilityDoesNotConfuseAnEntryPointWithAStateRead() {
    QFETCH(compat::Support, support);
    Fake system;
    system.setCapability(compat::GetBoolean{compat::BooleanProperty::FilterEnabled}, {support, "Fixture capability"});
    QCOMPARE(app::probeUwfCapability(system), support == compat::Support::Unsupported ? app::UwfCapability::Unavailable : app::UwfCapability::Available);
    QVERIFY(system.trace().empty());
  }
  void displayedFilePathsBecomeVolumeRelativeWithoutChangingTheirContents() {
    for (const auto& path : {std::string("C:\\Data\\File "), std::string("c:\\Data\\File "), std::string("\\\\?\\C:\\Data\\File ")})
      QCOMPARE(app::toUwfFilePath(path, "C:"), std::string("\\Data\\File "));
    QCOMPARE(app::toUwfFilePath("\\Data\\File ", "C:"), std::string("\\Data\\File "));
    for (const auto& pattern : {std::string(R"(\\?\Entry)"), std::string(R"(\\.\Entry)"), std::string(R"(\\Server\Share\Entry)")})
      QCOMPARE(app::toUwfFilePath(pattern, "C:"), pattern);
    QVERIFY_THROWS_EXCEPTION(std::invalid_argument, (void)app::toUwfFilePath("D:\\Data\\File", "C:"));
  }
  void publishedExclusionsRetainProviderPatternsAndTheirOwningVolume() {
    const auto guidLookingPattern = compat::volumePath(dVolumeId) + "Data\\..\\File ";
    std::vector<std::string> patterns{"\\Data\\", guidLookingPattern, "C:\\LiteralProviderData"};
    Fake::State state;
    state.current.volumes.emplace(cVolumeId, volume(cVolumeId, "C:", true, false, patterns));
    state.next = state.current;
    const Fake system(state);
    auto facts = system.state();
    facts.current.volumes[cVolumeId].physicalDriveLetter = compat::Fact<std::string>::known("D:");
    facts.next.volumes[cVolumeId].physicalDriveLetter = compat::Fact<std::string>::known("");
    const auto snapshot = app::publishedUwfSnapshot(facts, app::UwfCapability::Available, true, {});
    core::sortExclusions(patterns);
    for (const auto* session : {&snapshot.current, &snapshot.next}) {
      QCOMPARE(session->fileExclusions.size(), std::size_t{1});
      QCOMPARE(session->fileExclusions.at("Volume{" + std::string(cVolumeId) + '}'), patterns);
      QVERIFY(!session->fileExclusions.contains("Volume{" + std::string(dVolumeId) + '}'));
    }
    QVERIFY(system.trace().empty());
  }
  void snapshotKeepsSessionsAndExclusionScopes() {
    Fake::State state;
    state.current.filterEnabled = true;
    state.current.overlayType = static_cast<std::uint32_t>(compat::OverlayType::Ram);
    state.current.maximumOverlaySize = 100;
    state.current.warningThreshold = 60;
    state.current.criticalThreshold = 80;
    state.current.volumes.emplace(cVolumeId, volume(cVolumeId, "C:", true, false, {"\\Current"}));
    state.current.volumes.emplace(dVolumeId, volume(dVolumeId, "D:", false, false, {"\\UnprotectedCurrent"}));
    state.current.registryExclusions = {"HKEY_LOCAL_MACHINE\\SOFTWARE\\Current"};
    state.current.persistDomainSecretKey = true;
    state.next = state.current;
    state.next.filterEnabled = false;
    state.next.overlayType = static_cast<std::uint32_t>(compat::OverlayType::Disk);
    state.next.maximumOverlaySize = 2048;
    state.next.volumes.at(cVolumeId).fileExclusions = {"\\Next"};
    state.next.volumes.at(dVolumeId).fileExclusions = {"\\PreparedWhileUnprotected"};
    state.next.registryExclusions = {"HKEY_LOCAL_MACHINE\\SOFTWARE\\Next"};
    state.next.persistDomainSecretKey = false;
    state.next.persistTscal = true;
    state.runtime.usageOverride = compat::OverlayUsage{25, 75};
    const Fake system(state);
    auto facts = system.state();
    for (auto* session : {&facts.current, &facts.next}) {
      session->volumes[cVolumeId].physicalDriveLetter = compat::Fact<std::string>::known("C:");
      session->volumes[dVolumeId].physicalDriveLetter = compat::Fact<std::string>::known("D:");
    }
    const auto snapshot = app::publishedUwfSnapshot(facts, app::UwfCapability::Available, true, {25, 75});
    QVERIFY(snapshot.uwfAvailable);
    QVERIFY(snapshot.elevated);
    QVERIFY(snapshot.current.filter.enabled);
    QVERIFY(!snapshot.next.filter.enabled);
    QCOMPARE(snapshot.current.overlay.type, core::OverlayType::RAM);
    QCOMPARE(snapshot.next.overlay.type, core::OverlayType::Disk);
    QCOMPARE(snapshot.current.overlay.maximumSizeMb, 100u);
    QCOMPARE(snapshot.next.overlay.maximumSizeMb, 2048u);
    QCOMPARE(snapshot.current.overlay.warningThresholdMb, 60u);
    QCOMPARE(snapshot.next.overlay.criticalThresholdMb, 80u);
    QCOMPARE(snapshot.runtime.currentConsumptionMb, 25u);
    QCOMPARE(snapshot.runtime.availableSpaceMb, 75u);
    QCOMPARE(snapshot.current.fileExclusions.at(std::string("Volume{") + cVolumeId + '}'), std::vector<std::string>{"\\Current"});
    QVERIFY(!snapshot.current.fileExclusions.contains(std::string("Volume{") + dVolumeId + '}'));
    QCOMPARE(snapshot.next.fileExclusions.at(std::string("Volume{") + dVolumeId + '}'), std::vector<std::string>{"\\PreparedWhileUnprotected"});
    QCOMPARE(snapshot.current.registryExclusions.front(), std::string("HKEY_LOCAL_MACHINE\\SOFTWARE\\Current"));
    QCOMPARE(snapshot.next.registryExclusions.front(), std::string("HKEY_LOCAL_MACHINE\\SOFTWARE\\Next"));
    QVERIFY(snapshot.current.persistDomainSecretKey);
    QVERIFY(!snapshot.next.persistDomainSecretKey);
    QVERIFY(snapshot.next.persistTSCAL);
    QVERIFY(system.trace().empty());
  }
  void publishedTopologyPreservesDistinctConfigurationsWithoutQueries() {
    const Fake system(conflicting());
    auto facts = system.state();
    for (auto* scope : {&facts.current, &facts.next}) {
      scope->volumes[mountedVolumeId].physicalDriveLetter = compat::Fact<std::string>::known("C:");
      scope->volumes[staleVolumeId].physicalDriveLetter = compat::Fact<std::string>::known("");
    }
    auto projected = app::publishedUwfSnapshot(facts, app::UwfCapability::Available, true, {});
    QCOMPARE(projected.current.volumes.size(), std::size_t{3});
    QCOMPARE(projected.next.volumes.size(), std::size_t{3});
    const auto* stale = core::findVolumeConfiguration(projected.next, staleVolumeId);
    QVERIFY(stale);
    QCOMPARE(stale->driveLetter, std::string("C:"));
    QCOMPARE(stale->physicalDriveLetter, std::optional<std::string>{""});
    QVERIFY(core::volumeConfigurationAddress(*stale).empty());
    QCOMPARE(core::findVolumeConfiguration(projected.next, "C:")->volumeName, "Volume{" + std::string(mountedVolumeId) + '}');
    facts.current.volumes[mountedVolumeId].physicalDriveLetter = compat::Fact<std::string>::known("");
    facts.current.volumes[staleVolumeId].physicalDriveLetter = compat::Fact<std::string>::known("C:");
    projected = app::publishedUwfSnapshot(facts, app::UwfCapability::Available, true, {});
    QCOMPARE(projected.current.volumes.size(), std::size_t{3});
    const auto moved = std::ranges::find_if(
        projected.current.volumes, [](const auto& record) { return compat::parseVolumeId(record.volumeName) == compat::parseVolumeId(staleVolumeId); });
    QVERIFY(moved != projected.current.volumes.end());
    QCOMPARE(core::volumeConfigurationAddress(*moved), std::string("C:"));
    // Projection preserves even conflicting addresses. Selection reports an
    // ambiguity instead of discarding one configuration's baseline.
    facts.next.volumes[staleVolumeId].physicalDriveLetter = compat::Fact<std::string>::known("C:");
    projected = app::publishedUwfSnapshot(facts, app::UwfCapability::Available, true, {});
    QCOMPARE(projected.next.volumes.size(), std::size_t{3});
    QVERIFY_THROWS_EXCEPTION(std::runtime_error, static_cast<void>(core::findVolumeConfiguration(projected.next, "C:")));
    QVERIFY(core::findVolumeConfiguration(projected.next, staleVolumeId));
    QVERIFY(system.trace().empty());
  }
  void offlineConfigurationsDoNotClaimTheirSavedDriveLetter() {
    Fake::State state;
    state.current.volumes.emplace(mountedVolumeId, volume(mountedVolumeId, "C:", true));
    state.next.volumes.emplace(configuredVolumeId, volume(configuredVolumeId, "C:", true, true));
    state.next.volumes.emplace(retiredVolumeId, volume(retiredVolumeId, "C:"));
    const Fake system(state);
    auto facts = system.state();
    facts.next.volumes[configuredVolumeId].physicalDriveLetter = compat::Fact<std::string>::known("");
    facts.next.volumes[retiredVolumeId].physicalDriveLetter = compat::Fact<std::string>::known("");
    const auto projected = app::publishedUwfSnapshot(facts, app::UwfCapability::Available, true, {});
    QCOMPARE(projected.current.volumes.front().volumeName, "Volume{" + std::string(mountedVolumeId) + '}');
    QCOMPARE(projected.next.volumes.size(), std::size_t{2});
    QVERIFY(!core::findVolumeConfiguration(projected.next, "C:"));
    for (const auto id : {configuredVolumeId, retiredVolumeId}) {
      const auto* selected = core::findVolumeConfiguration(projected.next, id);
      QVERIFY(selected);
      QCOMPARE(compat::parseVolumeId(selected->volumeName), compat::parseVolumeId(id));
      QVERIFY(core::volumeConfigurationAddress(*selected).empty());
    }
    QVERIFY(system.trace().empty());
  }
  void duplicateSavedLettersPreserveBothPhysicalConfigurationOwners() {
    for (const bool loose : {false, true}) {
      Fake::State state;
      state.current.volumes.emplace(configuredVolumeId, volume(configuredVolumeId, "D:", true, loose, {"\\FromA"}));
      state.current.volumes.emplace(mountedVolumeId, volume(mountedVolumeId, "D:", true, loose, {"\\FromB"}));
      state.next = state.current;
      const Fake system(state);
      auto facts = system.state();
      for (auto* session : {&facts.current, &facts.next}) {
        session->volumes[configuredVolumeId].physicalDriveLetter = compat::Fact<std::string>::known("E:");
        session->volumes[mountedVolumeId].physicalDriveLetter = compat::Fact<std::string>::known("D:");
      }
      const auto projected = app::publishedUwfSnapshot(facts, app::UwfCapability::Available, true, {});
      const core::VolumeInfo physicalA{.driveLetter = "E:", .volumeName = compat::volumePath(configuredVolumeId), .fileSystem = {}, .label = {}};
      const core::VolumeInfo physicalB{.driveLetter = "D:", .volumeName = compat::volumePath(mountedVolumeId), .fileSystem = {}, .label = {}};
      for (const auto* session : {&projected.current, &projected.next}) {
        QCOMPARE(session->volumes.size(), std::size_t{2});
        const auto* a = core::findVolumeConfiguration(*session, physicalA);
        const auto* b = core::findVolumeConfiguration(*session, physicalB);
        QVERIFY(a && b && a != b);
        QCOMPARE(compat::parseVolumeId(a->volumeName), compat::parseVolumeId(configuredVolumeId));
        QCOMPARE(compat::parseVolumeId(b->volumeName), compat::parseVolumeId(mountedVolumeId));
        QCOMPARE(core::findVolumeConfiguration(*session, "E:"), a);
        QCOMPARE(core::findVolumeConfiguration(*session, "D:"), b);
        QCOMPARE(a->driveLetter, std::string("D:"));
        QCOMPARE(core::volumeConfigurationAddress(*a), std::string("E:"));
        QCOMPARE(session->fileExclusions.at(a->volumeName), std::vector<std::string>{"\\FromA"});
        QCOMPARE(session->fileExclusions.at(b->volumeName), std::vector<std::string>{"\\FromB"});
      }
      QVERIFY(system.trace().empty());
    }
  }
  void anUnknownOverlayTypeIsNotPresentedAsRam() {
    Fake::State state;
    state.current.overlayType = 42;
    const Fake system(state);
    QVERIFY_THROWS_EXCEPTION(app::OperationError, (void)app::publishedUwfSnapshot(system.state(), app::UwfCapability::Available, false, {}));
  }
};
}  // namespace
QTEST_APPLESS_MAIN(UwfReadStateTests)
#include "UwfReadStateTests.moc"
