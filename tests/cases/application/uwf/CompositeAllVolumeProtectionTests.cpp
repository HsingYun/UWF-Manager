// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QTest>
#include <functional>
#include <map>
#include <set>

#include "application/uwf/CompositeAllVolumeProtection.h"
#include "application/uwf/adapters/UwfOverlayMaintenance.h"

namespace {
using namespace uwf;
constexpr char a[] = "11111111-1111-1111-1111-111111111111";
constexpr char b[] = "22222222-2222-2222-2222-222222222222";
constexpr char partition[] = "33333333-3333-3333-3333-333333333333";
class MemoryUwf final : public compat::Uwf {
 public:
  mutable std::map<std::string, bool> next{{a, false}, {b, false}};
  mutable std::vector<compat::SetVolume> writes;
  bool horm = false;
  bool administrator = true;
  bool forbidReads = false;
  mutable int reads = 0;
  mutable compat::UwfStateFacts published;
  bool enumerationFails = false;
  std::set<std::string> unreadable;
  std::string failAfterWrite, throwOnReadAfterWrite;
  mutable bool readThrown = false;
  std::function<void()> afterWrite;
  compat::UwfStateSnapshot stateSnapshot() const override {
    auto facts = published;
    if (!facts.administrator.value) facts.administrator = compat::Fact<bool>::known(administrator);
    facts.runtime.horm = compat::Fact<bool>::known(horm);
    facts.next.volumesObserved = !enumerationFails;
    for (auto& [id, volume] : facts.next.volumes) {
      (void)id;
      volume.present = compat::Fact<bool>::known(false);
    }
    for (const auto& [id, protectedVolume] : next) {
      auto& volume = facts.next.volumes[id];
      volume.present = compat::Fact<bool>::known(true);
      volume.protectedVolume = unreadable.contains(id) ? compat::Fact<bool>{} : compat::Fact<bool>::known(protectedVolume);
    }
    return std::make_shared<const compat::UwfStateFacts>(std::move(facts));
  }
  void updateState(compat::UwfStateFacts facts) const override { published = std::move(facts); }
  compat::Capability probe(const compat::Request&) const override { return {compat::Support::Supported, {}}; }
  compat::Response execute(const compat::Request& request, const compat::Context& context) const override {
    if (context.stop.stop_requested()) return compat::Response::fail(compat::ErrorKind::Canceled, "Canceled");
    if (!compat::isWrite(request)) {
      ++reads;
      if (forbidReads) throw std::logic_error("A bulk mutation issued a provider observation");
    }
    if (const auto* read = std::get_if<compat::GetBoolean>(&request)) {
      return compat::Response::success(read->property == compat::BooleanProperty::HormEnabled ? horm : true);
    }
    if (std::holds_alternative<compat::GetVolumes>(request)) {
      if (enumerationFails) return compat::Response::fail(compat::ErrorKind::Unavailable, "Next enumeration failed");
      std::vector<compat::Volume> values;
      for (const auto& [id, value] : next) values.push_back({id, {}, value, false, {}});
      return compat::Response::success(std::move(values));
    }
    if (const auto* read = std::get_if<compat::GetVolumeProperty>(&request)) {
      if (!readThrown && read->volume == throwOnReadAfterWrite && !writes.empty()) {
        readThrown = true;
        throw std::runtime_error("Post-write custom reader failed");
      }
      if (unreadable.contains(read->volume)) return compat::Response::fail(compat::ErrorKind::AccessDenied, "Protection getter denied");
      const auto value = next.find(read->volume);
      if (value == next.end()) return compat::Response::fail(compat::ErrorKind::NotFound, "No Next registration");
      return compat::Response::success(value->second);
    }
    if (const auto* write = std::get_if<compat::SetVolume>(&request)) {
      writes.push_back(*write);
      next[write->volume] = write->value;
      if (afterWrite) afterWrite();
      if (write->volume == failAfterWrite)
        return compat::Response::fail(compat::ErrorKind::AccessDenied, "A provider step failed after changing protection", compat::Effect::Uncertain,
                                      {{"InjectedUwf", "SetProtected", static_cast<std::int64_t>(0x80070005U), "Original native failure"}});
      return compat::Response::success(std::monostate{}, compat::Effect::Confirmed);
    }
    return compat::Response::fail(compat::ErrorKind::Unsupported, "Unexpected interface request");
  }
};
class CachedUwf final : public compat::Uwf {
 public:
  MemoryUwf provider;
  mutable compat::UwfStateFacts published = provider.state();
  compat::UwfStateSnapshot stateSnapshot() const override { return std::make_shared<const compat::UwfStateFacts>(published); }
  void updateState(compat::UwfStateFacts facts) const override { published = std::move(facts); }
  compat::Capability probe(const compat::Request& request) const override { return provider.probe(request); }
  compat::Response execute(const compat::Request& request, const compat::Context& context) const override { return provider.execute(request, context); }
};
compat::AllVolumeProtectionEnvironment environment(std::vector<std::string> scope = {a, b, partition}) {
  return {.isAdministrator = [](const compat::Context&) { return true; },
          .protectScope = [scope = std::move(scope)](bool, const compat::Context&) { return scope; }};
}
class CompositeAllVolumeProtectionTests final : public QObject {
  Q_OBJECT
 private slots:
  void publishedSnapshotsShareScopeAndUnknownState_data() {
    QTest::addColumn<int>("scenario");
    QTest::addColumn<int>("protectedValue");
    QTest::addColumn<int>("unprotectedValue");
    QTest::newRow("complete-mixed") << 0 << 0 << 0;
    QTest::newRow("known-protect-scope-without-next-enumeration") << 1 << 1 << -1;
    QTest::newRow("protected-and-unknown") << 2 << -1 << -1;
    QTest::newRow("mixed-and-unknown") << 3 << -1 << -1;
  }
  void publishedSnapshotsShareScopeAndUnknownState() {
    QFETCH(int, scenario);
    QFETCH(int, protectedValue);
    QFETCH(int, unprotectedValue);
    const auto system = std::make_shared<MemoryUwf>();
    system->next = {{a, true}, {b, false}};
    std::vector<std::string> scope{a, b};
    if (scenario == 1) {
      system->enumerationFails = true;
      system->next[b] = true;
    } else if (scenario == 2) {
      system->unreadable.insert(b);
    } else if (scenario == 3) {
      system->next = {{a, false}, {b, true}, {partition, false}};
      system->unreadable.insert(partition);
      scope.push_back(partition);
    }
    system->published.runtime.protectScope = compat::Fact<std::vector<std::string>>::known(scope);
    system->forbidReads = true;
    const auto operations = app::composeAllVolumeProtection(
        system,
        {.isAdministrator = [](const compat::Context&) -> bool { throw std::logic_error("A snapshot queried administrator access"); },
         .protectScope = [](bool, const compat::Context&) -> std::vector<std::string> { throw std::logic_error("A snapshot queried the physical scope"); }});
    const auto bulk = app::allVolumeProtectionActions(operations);
    const auto published = bulk->snapshot();
    QVERIFY(published);
    const auto expected = [](const int value) -> std::optional<bool> { return value < 0 ? std::nullopt : std::optional{value != 0}; };
    std::vector<QString> targets;
    for (const auto& id : scope) targets.push_back(QString::fromStdString(id));
    QCOMPARE(published->nextAllProtected, expected(protectedValue));
    QCOMPARE(published->nextAllUnprotected, expected(unprotectedValue));
    QVERIFY(published->protectScopeConfirmed);
    QCOMPARE(published->unprotectScopeConfirmed, !system->enumerationFails);
    QCOMPARE(published->protectTargetVolumes, targets);
    QCOMPARE(published->unprotectTargetVolumes, system->enumerationFails ? std::vector<QString>{} : targets);
    for (const auto& maintenance : {app::uwfOverlayMaintenance(system), app::uwfOverlayMaintenance(system, {}, {}, bulk)}) {
      const auto snapshot = maintenance->snapshot();
      QVERIFY(snapshot);
      QCOMPARE(snapshot->allVolumesNextProtected, published->nextAllProtected);
      QCOMPARE(snapshot->allVolumesNextUnprotected, published->nextAllUnprotected);
      QCOMPARE(snapshot->protectScopeConfirmed, published->protectScopeConfirmed);
      QCOMPARE(snapshot->unprotectScopeConfirmed, published->unprotectScopeConfirmed);
      QCOMPARE(snapshot->protectTargetVolumes, published->protectTargetVolumes);
      QCOMPARE(snapshot->unprotectTargetVolumes, published->unprotectTargetVolumes);
    }
    QCOMPARE(system->reads, 0);
    QVERIFY(system->writes.empty());
  }
  void failedGlobalRefreshRejectsEvenMatchingBulkTargets_data() {
    QTest::addColumn<bool>("protect");
    QTest::newRow("protect") << true;
    QTest::newRow("unprotect") << false;
  }
  void failedGlobalRefreshRejectsEvenMatchingBulkTargets() {
    QFETCH(bool, protect);
    const auto system = std::make_shared<MemoryUwf>();
    system->next = {{a, protect}, {b, protect}};
    system->published.runtime.protectScope = compat::Fact<std::vector<std::string>>::known({a, b});
    system->published.refreshFailure = compat::Failure{compat::ErrorKind::AccessDenied, compat::Effect::None, "Refresh denied", {}};
    system->forbidReads = true;
    const auto feature = app::composeAllVolumeProtection(system, {});
    const auto execute = [&] { return protect ? feature->execute(compat::ProtectAll{}) : feature->execute(compat::UnprotectAll{}); };
    const auto rejected = execute();
    QVERIFY(rejected.failure);
    QCOMPARE(rejected.failure->kind, compat::ErrorKind::Unavailable);
    QCOMPARE(rejected.effect, compat::Effect::None);
    QVERIFY(!rejected.completionConfirmed);
    QVERIFY(system->writes.empty());
    QCOMPARE(system->reads, 0);
    system->published.refreshFailure.reset();
    const auto recovered = execute();
    QVERIFY(recovered.succeeded());
    QVERIFY(recovered.completionConfirmed);
    QVERIFY(system->writes.empty());
    QCOMPARE(system->reads, 0);
  }
  void failedPhysicalScopeReplacesTheOldScopeWithoutBlockingUnprotect_data() {
    QTest::addColumn<int>("failure");
    QTest::newRow("observation-failed") << 0;
    QTest::newRow("empty-scope") << 1;
    QTest::newRow("invalid-scope") << 2;
  }
  void failedPhysicalScopeReplacesTheOldScopeWithoutBlockingUnprotect() {
    QFETCH(int, failure);
    const auto system = std::make_shared<CachedUwf>();
    bool failScope = false;
    auto ports = environment();
    ports.protectScope = [&](bool, const compat::Context&) -> std::vector<std::string> {
      if (!failScope) return {a, b};
      if (failure == 0) throw std::runtime_error("Physical scope unavailable");
      return failure == 1 ? std::vector<std::string>{} : std::vector<std::string>{a, "invalid-volume"};
    };
    const auto feature = app::composeAllVolumeProtection(system, ports);
    QVERIFY(feature->read(compat::GetAllProtected{}).scopeConfirmed);
    QVERIFY(system->published.runtime.protectScope.isKnown());
    failScope = true;
    QVERIFY(feature->read(compat::GetAllProtected{}).failure);
    QVERIFY(!system->published.runtime.protectScope.isKnown());
    QVERIFY(!system->published.refreshFailure);
    const auto reads = system->provider.reads;
    system->provider.forbidReads = true;
    const auto rejected = feature->execute(compat::ProtectAll{});
    QVERIFY(rejected.failure);
    QCOMPARE(rejected.effect, compat::Effect::None);
    QVERIFY(!rejected.completionConfirmed);
    QVERIFY(system->provider.writes.empty());
    const auto unprotected = feature->snapshot(compat::GetAllUnprotected{});
    QVERIFY(unprotected.scopeConfirmed);
    QCOMPARE(unprotected.value, std::optional<bool>{true});
    const auto independent = feature->execute(compat::UnprotectAll{});
    QVERIFY(independent.succeeded());
    QVERIFY(independent.completionConfirmed);
    QCOMPARE(independent.effect, compat::Effect::None);
    QCOMPARE(system->provider.reads, reads);
    QVERIFY(system->provider.writes.empty());
  }
  void failedProtectionReadReplacesTheOldTargetObservation_data() {
    QTest::addColumn<bool>("protect");
    QTest::addColumn<bool>("registered");
    QTest::newRow("protect-existing") << true << true;
    QTest::newRow("protect-previously-absent") << true << false;
    QTest::newRow("unprotect-existing") << false << true;
  }
  void failedProtectionReadReplacesTheOldTargetObservation() {
    QFETCH(bool, protect);
    QFETCH(bool, registered);
    const auto system = std::make_shared<CachedUwf>();
    system->provider.next = registered ? std::map<std::string, bool>{{a, !protect}} : std::map<std::string, bool>{};
    const auto feature = app::composeAllVolumeProtection(system, environment({a}));
    const auto read = [&] { return protect ? feature->read(compat::GetAllProtected{}) : feature->read(compat::GetAllUnprotected{}); };
    const auto initial = read();
    QVERIFY(initial.scopeConfirmed);
    QCOMPARE(initial.value, std::optional<bool>{false});
    QVERIFY(system->published.next.volumes.at(a).protectedVolume.isKnown());
    system->provider.next[a] = !protect;
    system->provider.unreadable.insert(a);
    const auto failed = read();
    QVERIFY(failed.failure);
    QVERIFY(failed.scopeConfirmed);
    QVERIFY(!failed.value);
    QVERIFY(!system->published.next.volumes.at(a).protectedVolume.isKnown());
    QCOMPARE(system->published.next.volumes.at(a).present.isKnown(), !protect);
    const auto reads = system->provider.reads;
    system->provider.forbidReads = true;
    const auto result = protect ? feature->execute(compat::ProtectAll{}) : feature->execute(compat::UnprotectAll{});
    QVERIFY(result.failure);
    QCOMPARE(result.effect, compat::Effect::None);
    QVERIFY(!result.completionConfirmed);
    QCOMPARE(system->provider.reads, reads);
    QVERIFY(system->provider.writes.empty());
  }
  void mutationsRequireGlobalScopeWithoutCallingObservationPorts() {
    const auto system = std::make_shared<MemoryUwf>();
    system->enumerationFails = true;
    auto ports = environment();
    int scopeReads = 0, administratorReads = 0;
    ports.protectScope = [&](bool, const compat::Context&) {
      ++scopeReads;
      return std::vector<std::string>{a, b};
    };
    ports.isAdministrator = [&](const compat::Context&) {
      ++administratorReads;
      return true;
    };
    const auto feature = app::composeAllVolumeProtection(system, ports);
    QVERIFY(!feature->execute(compat::ProtectAll{}).succeeded());
    QVERIFY(!feature->execute(compat::UnprotectAll{}).succeeded());
    QCOMPARE(scopeReads, 0);
    QCOMPARE(administratorReads, 0);
    QCOMPARE(system->reads, 0);
    QVERIFY(system->writes.empty());
    system->enumerationFails = false;
    QVERIFY(feature->read(compat::GetAllProtected{}).scopeConfirmed);
    QCOMPARE(scopeReads, 1);
    QCOMPARE(administratorReads, 1);
    const auto reads = system->reads;
    system->forbidReads = true;
    QVERIFY(feature->execute(compat::ProtectAll{}).succeeded());
    QCOMPARE(system->reads, reads);
    QCOMPARE(scopeReads, 1);
    QCOMPARE(administratorReads, 1);
  }
  void protectsCompleteInjectedScopeAndCreatesByGuidWithoutRebindingExistingRows() {
    const auto system = std::make_shared<MemoryUwf>();
    auto feature = app::composeAllVolumeProtection(system, environment());
    QVERIFY(feature->read(compat::GetAllProtected{}).scopeConfirmed);
    const auto reads = system->reads;
    system->forbidReads = true;
    const auto result = feature->execute(compat::ProtectAll{});
    QVERIFY(result.succeeded());
    QCOMPARE(result.effect, compat::Effect::Confirmed);
    QCOMPARE(result.items.size(), std::size_t{3});
    QCOMPARE(system->writes.size(), std::size_t{3});
    QVERIFY(system->next.at(partition));
    for (const auto& write : system->writes) {
      QCOMPARE(write.property, compat::VolumeProperty::Protected);
      QVERIFY(write.value);
      QVERIFY(write.driveLetter.empty());
      QVERIFY(!write.bindByDriveLetterOnCreate);
    }
    const auto second = feature->execute(compat::ProtectAll{});
    QVERIFY(second.succeeded());
    QCOMPARE(second.effect, compat::Effect::None);
    QCOMPARE(system->writes.size(), std::size_t{3});
    QCOMPARE(system->reads, reads);
  }
  void unprotectIncludesOfflineRegistrationsAndNeverNeedsThePhysicalScope() {
    const auto system = std::make_shared<MemoryUwf>();
    system->next = {{a, true}, {partition, true}};
    auto ports = environment();
    int physicalReads = 0;
    ports.protectScope = [&](bool, const compat::Context&) -> std::vector<std::string> {
      ++physicalReads;
      throw compat::Error({compat::ErrorKind::Unsupported, compat::Effect::None, "Physical scope unavailable", {}});
    };
    auto feature = app::composeAllVolumeProtection(system, ports);
    QVERIFY(feature->read(compat::GetAllUnprotected{}).scopeConfirmed);
    system->forbidReads = true;
    const auto result = feature->execute(compat::UnprotectAll{});
    QVERIFY(result.succeeded());
    QCOMPARE(physicalReads, 0);
    QCOMPARE(system->next.size(), std::size_t{2});
    QVERIFY(!system->next.at(partition));
    QCOMPARE(system->writes.size(), std::size_t{2});
    system->forbidReads = false;
    const auto snapshot = app::allVolumeProtectionActions(feature)->observe();
    QVERIFY(!snapshot.protectSupported);
    QVERIFY(snapshot.unprotectSupported);
    QVERIFY(!snapshot.nextAllProtected);
    QCOMPARE(snapshot.nextAllUnprotected, std::optional<bool>{true});
  }
  void unreadableAndMatchingVolumesAreNotWrittenWhileIndependentChangesContinue_data() {
    QTest::addColumn<bool>("protect");
    QTest::newRow("protect") << true;
    QTest::newRow("unprotect") << false;
  }
  void unreadableAndMatchingVolumesAreNotWrittenWhileIndependentChangesContinue() {
    QFETCH(bool, protect);
    const auto system = std::make_shared<MemoryUwf>();
    system->next = {{a, !protect}, {b, !protect}, {partition, protect}};
    system->unreadable.insert(a);
    auto feature = app::composeAllVolumeProtection(system, environment());
    const auto observed = protect ? feature->read(compat::GetAllProtected{}) : feature->read(compat::GetAllUnprotected{});
    QVERIFY(observed.scopeConfirmed);
    system->forbidReads = true;
    const auto result = protect ? feature->execute(compat::ProtectAll{}) : feature->execute(compat::UnprotectAll{});
    QVERIFY(!result.succeeded());
    QVERIFY(!result.observation.value.has_value());
    QCOMPARE(result.items.size(), std::size_t{3});
    QVERIFY(result.items[0].failure.has_value());
    QCOMPARE(result.items[0].effect, compat::Effect::None);
    QVERIFY(result.items[1].completionConfirmed);
    QCOMPARE(result.items[1].effect, compat::Effect::Confirmed);
    QVERIFY(result.items[2].completionConfirmed);
    QCOMPARE(result.items[2].effect, compat::Effect::None);
    QCOMPARE(system->writes.size(), std::size_t{1});
    QCOMPARE(system->writes[0].volume, b);
    QCOMPARE(system->writes[0].value, protect);
    QCOMPARE(system->next.at(a), !protect);
    QCOMPARE(system->next.at(b), protect);
    QCOMPARE(system->next.at(partition), protect);
  }
  void matchingReadbackNeverRepairsFailedWorkflowAndDiagnosticsReachTheApp() {
    const auto system = std::make_shared<MemoryUwf>();
    system->failAfterWrite = a;
    auto actions = app::allVolumeProtectionActions(app::composeAllVolumeProtection(system, environment({a, b})));
    (void)actions->observe();
    const auto reads = system->reads;
    system->forbidReads = true;
    const auto result = actions->setProtected(true);
    QCOMPARE(system->reads, reads);
    QVERIFY(!result.succeeded());
    QCOMPARE(result.snapshot.nextAllProtected, std::optional<bool>{true});
    QCOMPARE(result.effect, app::WriteEffect::Uncertain);
    QCOMPARE(result.items.size(), std::size_t{2});
    QVERIFY(result.items[0].failure.has_value());
    QCOMPARE(static_cast<std::uint32_t>(result.items[0].failure->diagnostic.code), 0x80070005U);
    QVERIFY(result.items[0].error.contains(QStringLiteral("InjectedUwf")));
    QVERIFY(result.items[1].completionConfirmed);
    QCOMPARE(system->writes.size(), std::size_t{2});
  }
  void aCustomPostwriteReaderIsNeverInvokedByMutation() {
    const auto system = std::make_shared<MemoryUwf>();
    system->throwOnReadAfterWrite = a;
    const auto feature = app::composeAllVolumeProtection(system, environment({a, b}));
    QVERIFY(feature->read(compat::GetAllProtected{}).scopeConfirmed);
    system->forbidReads = true;
    const auto result = feature->execute(compat::ProtectAll{});
    QVERIFY(result.succeeded());
    QCOMPARE(result.effect, compat::Effect::Confirmed);
    QCOMPARE(result.items.size(), std::size_t{2});
    QCOMPARE(result.items[0].effect, compat::Effect::Confirmed);
    QVERIFY(!result.items[0].failure);
    QVERIFY(result.items[1].completionConfirmed);
    QVERIFY(!system->readThrown);
    QCOMPARE(result.observation.value, std::optional<bool>{true});
  }
  void emptyNextInventoryIsAnExplicitUnprotectNoopAndUnreadableInventoryIsUnknown() {
    const auto system = std::make_shared<MemoryUwf>();
    system->next.clear();
    auto feature = app::composeAllVolumeProtection(system, environment({}));
    QVERIFY(!feature->read(compat::GetAllProtected{}).value);
    QCOMPARE(feature->read(compat::GetAllUnprotected{}).value, std::optional<bool>{true});
    const auto result = feature->execute(compat::UnprotectAll{});
    QVERIFY(result.succeeded());
    QCOMPARE(result.effect, compat::Effect::None);
    QVERIFY(system->writes.empty());
    system->enumerationFails = true;
    QVERIFY(!feature->read(compat::GetAllUnprotected{}).value);
    QVERIFY(!feature->execute(compat::UnprotectAll{}).succeeded());
    QVERIFY(system->writes.empty());
  }
  void fatalSessionChangesAndCancellationStopFurtherWrites() {
    const auto system = std::make_shared<MemoryUwf>();
    std::stop_source canceled;
    system->afterWrite = [&] { canceled.request_stop(); };
    const auto feature = app::composeAllVolumeProtection(system, environment({a, b}));
    QVERIFY(feature->read(compat::GetAllProtected{}).scopeConfirmed);
    system->forbidReads = true;
    const auto result = feature->execute(compat::ProtectAll{}, {canceled.get_token()});
    QVERIFY(!result.succeeded());
    QCOMPARE(system->writes.size(), std::size_t{1});
    QCOMPARE(result.effect, compat::Effect::Confirmed);
    const auto hormSystem = std::make_shared<MemoryUwf>();
    hormSystem->afterWrite = [target = hormSystem.get()] { target->horm = true; };
    const auto hormFeature = app::composeAllVolumeProtection(hormSystem, environment({a, b}));
    QVERIFY(hormFeature->read(compat::GetAllProtected{}).scopeConfirmed);
    hormSystem->forbidReads = true;
    const auto changed = hormFeature->execute(compat::ProtectAll{});
    QVERIFY(!changed.succeeded());
    QCOMPARE(hormSystem->writes.size(), std::size_t{1});
    const auto deniedSystem = std::make_shared<MemoryUwf>();
    auto deniedPorts = environment();
    deniedPorts.isAdministrator = [](const compat::Context&) { return false; };
    const auto deniedFeature = app::composeAllVolumeProtection(deniedSystem, deniedPorts);
    QVERIFY(deniedFeature->read(compat::GetAllUnprotected{}).scopeConfirmed);
    deniedSystem->forbidReads = true;
    QVERIFY(!deniedFeature->execute(compat::UnprotectAll{}).succeeded());
    QVERIFY(deniedSystem->writes.empty());
  }
};
}  // namespace
QTEST_APPLESS_MAIN(CompositeAllVolumeProtectionTests)
#include "CompositeAllVolumeProtectionTests.moc"
