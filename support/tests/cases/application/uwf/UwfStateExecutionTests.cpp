// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QScopeGuard>
#include <QTest>
#include <algorithm>
#include <atomic>
#include <functional>
#include <future>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "application/uwf/UwfRequestPrecheck.h"
#include "application/uwf/UwfStateUpdates.h"
#include "infrastructure/uwf/routing/FallbackUwf.h"

namespace {
using namespace uwf;
using namespace uwf::compat;
using namespace std::chrono_literals;
constexpr auto volumeId = "11111111-2222-3333-4444-555555555555";

UwfStateFacts state() {
  UwfStateFacts facts;
  facts.administrator = Fact<bool>::known(true);
  facts.capturedSettingsAvailable = Fact<bool>::known(true);
  facts.systemVolumeId = Fact<std::string>::known(volumeId);
  facts.current.filterEnabled = Fact<bool>::known(true);
  facts.next.filterEnabled = Fact<bool>::known(false);
  facts.current.servicingEnabled = facts.next.servicingEnabled = Fact<bool>::known(false);
  facts.current.overlayType = facts.next.overlayType = Fact<std::uint32_t>::known(1);
  facts.current.maximumOverlaySize = facts.next.maximumOverlaySize = Fact<std::uint32_t>::known(2048);
  facts.current.overlayFlags = facts.next.overlayFlags = BitFacts::known(0);
  facts.runtime.horm = Fact<bool>::known(false);
  facts.runtime.controlDevicePresent = Fact<bool>::known(true);
  facts.runtime.romDisposition = BitFacts::known(0);
  facts.runtime.warningThreshold = Fact<std::uint32_t>::known(512);
  facts.runtime.criticalThreshold = Fact<std::uint32_t>::known(768);
  facts.current.volumesObserved = facts.next.volumesObserved = true;
  auto& current = facts.current.volumes[volumeId];
  current.present = current.protectedVolume = Fact<bool>::known(true);
  current.bindByDriveLetter = Fact<bool>::known(false);
  current.driveLetter = Fact<std::string>::known("C:");
  current.physicalDriveLetter = Fact<std::string>::known("C:");
  current.fileExclusions = Fact<std::vector<std::string>>::known({});
  facts.next.volumes[volumeId] = current;
  facts.next.volumes[volumeId].protectedVolume = Fact<bool>::known(false);
  facts.current.registryExclusions = facts.next.registryExclusions = Fact<std::vector<std::string>>::known({});
  return facts;
}
struct Recording {
  std::vector<Request> calls;
  std::vector<UwfStateFacts> facts;
  std::function<Response(const Request&, const Context&)> action;
  Response execute(const Request& request, const Context& context) {
    calls.push_back(request);
    if (context.facts) facts.push_back(*context.facts);
    if (action) return action(request, context);
    if (!isWrite(request)) return Response::fail(ErrorKind::Failed, "Unexpected prerequisite or readback query");
    return Response::success(std::monostate{}, Effect::Confirmed);
  }
};
class RecordingUwf final : public Uwf {
 public:
  std::shared_ptr<Recording> recording = std::make_shared<Recording>();
  Capability probe(const Request&) const override { return {Support::Supported, {}}; }
  Response execute(const Request& request, const Context& context) const override { return recording->execute(request, context); }
};
class RecordingBackend final : public Backend {
 public:
  RecordingBackend(std::string label, std::shared_ptr<Recording> recording, std::function<bool(const Request&)> accepts)
      : m_name(std::move(label)), m_recording(std::move(recording)), m_accepts(std::move(accepts)) {}
  std::string name() const override { return m_name; }
  Capability probe(const Request& request) const override { return {m_accepts(request) ? Support::Supported : Support::Unsupported, {}}; }
  Response execute(const Request& request, const Context& context) const override { return m_recording->execute(request, context); }

 private:
  std::string m_name;
  std::shared_ptr<Recording> m_recording;
  std::function<bool(const Request&)> m_accepts;
};

class UwfStateExecutionTests final : public QObject {
  Q_OBJECT
 private slots:
  void completedWritesRetainCapabilityRefreshWarnings() {
    auto raw = std::make_shared<RecordingUwf>();
    app::PrecheckedUwf system(raw, {}, false, [](const Uwf&, const Context&) {
      auto response = Response::success();
      response.warnings.push_back("Captured settings: observation unavailable");
      return response;
    });
    auto facts = state();
    facts.current.filterEnabled = Fact<bool>::known(false);
    system.updateState(std::move(facts));
    const auto response = system.execute(ResetSettings{});
    QVERIFY(response.succeeded());
    QCOMPARE(response.effect, Effect::Confirmed);
    QCOMPARE(response.warnings, std::vector<std::string>{"Captured settings: observation unavailable"});
    QVERIFY(!system.state().refreshFailure);
  }
  void writesRequireTheirBranchDependenciesWithOrWithoutInference_data() {
    QTest::addColumn<int>("scenario");
    QTest::addColumn<int>("observation");
    QTest::addColumn<bool>("inference");
    const std::vector<const char*> scenarios{"servicing-enable-status", "servicing-disable-status", "servicing-started-restore", "servicing-failed-restore",
                                             "captured-defaults"};
    const std::vector<const char*> observations{"unknown", "unsatisfied", "satisfied"};
    for (std::size_t scenario = 0; scenario < scenarios.size(); ++scenario)
      for (std::size_t observation = 0; observation < observations.size(); ++observation)
        for (const bool inference : {false, true})
          QTest::newRow(qUtf8Printable(QStringLiteral("%1/%2/inference-%3").arg(scenarios[scenario]).arg(observations[observation]).arg(inference)))
              << static_cast<int>(scenario) << static_cast<int>(observation) << inference;
  }
  void writesRequireTheirBranchDependenciesWithOrWithoutInference() {
    QFETCH(int, scenario);
    QFETCH(int, observation);
    QFETCH(bool, inference);
    auto raw = std::make_shared<RecordingUwf>();
    int refreshes = 0;
    app::PrecheckedUwf system(raw, {}, inference, [&](const Uwf&, const Context&) {
      ++refreshes;
      return Response::success();
    });
    auto facts = state();
    Request request = ResetSettings{};
    if (scenario <= 1) {
      facts.runtime.servicingStatus =
          observation == 0 ? Fact<std::uint32_t>{} : Fact<std::uint32_t>::known(observation == 1 ? servicingStatus::completed : servicingStatus::notStarted);
      request = SetBoolean{BooleanProperty::ServicingEnabled, scenario == 0};
    } else if (scenario <= 3) {
      facts.runtime.servicingStatus = Fact<std::uint32_t>::known(scenario == 2 ? servicingStatus::started : servicingStatus::failed);
      facts.runtime.savedServicingFilter =
          observation == 0 ? Fact<std::optional<bool>>{} : Fact<std::optional<bool>>::known(observation == 1 ? std::nullopt : std::optional{false});
      request = SetBoolean{BooleanProperty::ServicingEnabled, false};
    } else {
      facts.current.filterEnabled = Fact<bool>::known(false);
      facts.capturedSettingsAvailable = observation == 0 ? Fact<bool>{} : Fact<bool>::known(observation == 2);
    }
    system.updateState(facts);
    const auto response = system.execute(request);
    if (observation != 2) {
      QVERIFY(response.failure);
      QCOMPARE(response.failure->kind, observation == 0 ? ErrorKind::Unavailable : ErrorKind::Conflict);
      QCOMPARE(response.effect, Effect::None);
      QCOMPARE(response.failure->effect, Effect::None);
      QVERIFY(raw->recording->calls.empty());
      QCOMPARE(refreshes, 0);
    } else {
      QVERIFY2(response.succeeded(), response.failure ? response.failure->message.c_str() : "");
      QCOMPARE(response.effect, Effect::Confirmed);
      QCOMPARE(raw->recording->calls, std::vector<Request>{request});
      QCOMPARE(refreshes, 1);
    }
  }
  void hormDisableAndRepeatedSettingsIgnoreEnablePrerequisites_data() {
    QTest::addColumn<bool>("current");
    QTest::addColumn<bool>("target");
    QTest::addColumn<bool>("knownUnsatisfied");
    QTest::addColumn<bool>("inference");
    for (const bool current : {false, true})
      for (const bool target : {false, true}) {
        if (!current && target) continue;
        for (const bool knownUnsatisfied : {false, true})
          for (const bool inference : {false, true})
            QTest::newRow(qUtf8Printable(QStringLiteral("%1-to-%2/prerequisites-%3/inference-%4")
                                             .arg(current)
                                             .arg(target)
                                             .arg(knownUnsatisfied ? "unsatisfied" : "unknown")
                                             .arg(inference)))
                << current << target << knownUnsatisfied << inference;
      }
  }
  void hormDisableAndRepeatedSettingsIgnoreEnablePrerequisites() {
    QFETCH(bool, current);
    QFETCH(bool, target);
    QFETCH(bool, knownUnsatisfied);
    QFETCH(bool, inference);
    auto raw = std::make_shared<RecordingUwf>();
    app::PrecheckedUwf system(raw, {}, inference);
    auto facts = state();
    facts.runtime.horm = Fact<bool>::known(current);
    facts.runtime.hiberFile = facts.runtime.hormVolumesProtected = knownUnsatisfied ? Fact<bool>::known(false) : Fact<bool>{};
    system.updateState(facts);
    const Request request = SetBoolean{BooleanProperty::HormEnabled, target};
    const auto response = system.execute(request);
    QVERIFY2(response.succeeded(), response.failure ? response.failure->message.c_str() : "");
    QCOMPARE(response.effect, current == target ? Effect::None : Effect::Confirmed);
    QCOMPARE(raw->recording->calls, current == target ? std::vector<Request>{} : std::vector<Request>{request});
    QCOMPARE(system.state().runtime.horm.value, std::optional{target});
    QVERIFY(!system.state().refreshFailure);
  }
  void servicingOnlyRequiresSavedFilterWhenLeavingAnActiveTransition_data() {
    QTest::addColumn<quint32>("status");
    QTest::addColumn<bool>("enabled");
    QTest::addColumn<bool>("confirmedAbsent");
    QTest::addColumn<bool>("inference");
    for (const auto status : {servicingStatus::notStarted, servicingStatus::started, servicingStatus::failed})
      for (const bool enabled : {false, true}) {
        if (!enabled && status != servicingStatus::notStarted) continue;
        for (const bool confirmedAbsent : {false, true})
          for (const bool inference : {false, true})
            QTest::newRow(qUtf8Printable(QStringLiteral("status-%1/enabled-%2/saved-%3/inference-%4")
                                             .arg(status)
                                             .arg(enabled)
                                             .arg(confirmedAbsent ? "absent" : "unknown")
                                             .arg(inference)))
                << static_cast<quint32>(status) << enabled << confirmedAbsent << inference;
      }
  }
  void servicingOnlyRequiresSavedFilterWhenLeavingAnActiveTransition() {
    QFETCH(quint32, status);
    QFETCH(bool, enabled);
    QFETCH(bool, confirmedAbsent);
    QFETCH(bool, inference);
    auto raw = std::make_shared<RecordingUwf>();
    int refreshes = 0;
    app::PrecheckedUwf system(raw, {}, inference, [&](const Uwf&, const Context&) {
      ++refreshes;
      return Response::success();
    });
    auto facts = state();
    facts.runtime.servicingStatus = Fact<std::uint32_t>::known(status);
    facts.runtime.savedServicingFilter = confirmedAbsent ? Fact<std::optional<bool>>::known(std::nullopt) : Fact<std::optional<bool>>{};
    system.updateState(facts);
    const Request request = SetBoolean{BooleanProperty::ServicingEnabled, enabled};
    const auto response = system.execute(request);
    QVERIFY2(response.succeeded(), response.failure ? response.failure->message.c_str() : "");
    QCOMPARE(response.effect, Effect::Confirmed);
    QCOMPARE(raw->recording->calls, std::vector<Request>{request});
    QCOMPARE(refreshes, 1);
  }
  void individualFileDispatchRequiresThePublishedPhysicalMapping_data() {
    QTest::addColumn<int>("scenario");
    QTest::addColumn<bool>("deletion");
    QTest::addColumn<bool>("inference");
    const std::vector<const char*> scenarios{"matching",          "configured-unknown", "configured-empty",  "physical-unknown",
                                             "physical-empty",    "mismatched",         "case-insensitive",  "not-a-drive-prefix",
                                             "excluded-mismatch", "filter-off",         "target-unprotected"};
    for (std::size_t index = 0; index < scenarios.size(); ++index)
      for (const bool deletion : {false, true})
        for (const bool inference : {false, true})
          QTest::newRow(qUtf8Printable(QStringLiteral("%1/delete-%2/inference-%3").arg(scenarios[index]).arg(deletion).arg(inference)))
              << static_cast<int>(index) << deletion << inference;
  }
  void individualFileDispatchRequiresThePublishedPhysicalMapping() {
    QFETCH(int, scenario);
    QFETCH(bool, deletion);
    QFETCH(bool, inference);
    auto raw = std::make_shared<RecordingUwf>();
    app::PrecheckedUwf system(raw, {}, inference);
    auto facts = state();
    auto& current = facts.current.volumes[volumeId];
    std::optional<ErrorKind> expected;
    switch (scenario) {
      case 1:
        current.driveLetter = {};
        expected = ErrorKind::Unavailable;
        break;
      case 2:
        current.driveLetter = Fact<std::string>::known("");
        expected = ErrorKind::Unsupported;
        break;
      case 3:
        current.physicalDriveLetter = {};
        expected = ErrorKind::Unavailable;
        break;
      case 4:
        current.physicalDriveLetter = Fact<std::string>::known("");
        expected = ErrorKind::Unsupported;
        break;
      case 5:
      case 8:
        current.physicalDriveLetter = Fact<std::string>::known("D:");
        if (scenario == 8) current.fileExclusions = Fact<std::vector<std::string>>::known({"\\Data"});
        expected = ErrorKind::Conflict;
        break;
      case 6:
        current.driveLetter = Fact<std::string>::known("c:");
        break;
      case 7:
        current.driveLetter = Fact<std::string>::known("C:\\Other");
        expected = ErrorKind::Protocol;
        break;
      case 9:
      case 10:
        if (scenario == 9)
          facts.current.filterEnabled = Fact<bool>::known(false);
        else
          current.protectedVolume = Fact<bool>::known(false);
        current.driveLetter = current.physicalDriveLetter = {};
        if (deletion) expected = ErrorKind::Conflict;
        break;
      default:
        break;
    }
    QVERIFY(!facts.requestTarget);
    system.updateState(std::move(facts));
    const Request request = deletion ? Request{CommitFileDeletion{volumeId, "\\Data\\same.bin"}} : Request{CommitFile{volumeId, "\\Data\\same.bin"}};
    const auto response = system.execute(request);
    QCOMPARE(response.failure.has_value(), expected.has_value());
    if (expected) {
      QCOMPARE(response.failure->kind, *expected);
      QCOMPARE(response.effect, Effect::None);
      QVERIFY(raw->recording->calls.empty());
    } else {
      QCOMPARE(raw->recording->calls, std::vector<Request>{request});
    }
  }
  void batchCommitsReuseThePublishedStateWithoutGetters_data() {
    QTest::addColumn<bool>("inference");
    QTest::newRow("inference-on") << true;
    QTest::newRow("inference-off") << false;
  }
  void batchCommitsReuseThePublishedStateWithoutGetters() {
    QFETCH(bool, inference);
    auto raw = std::make_shared<RecordingUwf>();
    app::PrecheckedUwf system(raw, {}, inference);
    system.updateState(state());
    constexpr int total = 100;
    for (int index = 0; index < total; ++index) {
      const auto response = system.execute(CommitFile{volumeId, "\\file-" + std::to_string(index) + ".txt"});
      QVERIFY2(response.succeeded(), response.failure ? response.failure->message.c_str() : "");
    }
    QCOMPARE(raw->recording->calls.size(), std::size_t{total});
    QCOMPARE(raw->recording->facts.size(), std::size_t{total});
    for (const auto& request : raw->recording->calls) QVERIFY(std::holds_alternative<CommitFile>(request));
    for (const auto& facts : raw->recording->facts) {
      QCOMPARE(facts.current.filterEnabled.value, std::optional{true});
      QCOMPARE(facts.runtime.horm.value, std::optional{false});
      QCOMPARE(facts.current.volumes.at(volumeId).protectedVolume.value, std::optional{true});
    }
  }
  void readersRetainImmutablePublicationsAcrossWritesAndNestedUpdates() {
    auto raw = std::make_shared<RecordingUwf>();
    app::PrecheckedUwf system(raw);
    system.updateState(state());
    const auto original = system.stateSnapshot();
    QCOMPARE(original.get(), system.stateSnapshot().get());
    QVERIFY(system.execute(CommitFile{volumeId, "\\same.bin"}).succeeded());
    QCOMPARE(original.get(), system.stateSnapshot().get());
    QVERIFY(system.execute(SetNumber{NumberProperty::WarningThreshold, 600}).succeeded());
    const auto changed = system.stateSnapshot();
    QVERIFY(changed != original);
    QCOMPARE(original->runtime.warningThreshold.value, std::optional<std::uint32_t>{512});
    QCOMPARE(changed->runtime.warningThreshold.value, std::optional<std::uint32_t>{600});
    UwfStateSnapshot privateBefore;
    system.synchronize([&] {
      privateBefore = system.stateSnapshot();
      auto updated = system.state();
      updated.runtime.warningThreshold = Fact<std::uint32_t>::known(700);
      system.updateState(std::move(updated));
      QCOMPARE(privateBefore->runtime.warningThreshold.value, std::optional<std::uint32_t>{600});
      QCOMPARE(system.stateSnapshot()->runtime.warningThreshold.value, std::optional<std::uint32_t>{700});
    });
    QCOMPARE(privateBefore->runtime.warningThreshold.value, std::optional<std::uint32_t>{600});
    QCOMPARE(changed->runtime.warningThreshold.value, std::optional<std::uint32_t>{600});
    QCOMPARE(system.stateSnapshot()->runtime.warningThreshold.value, std::optional<std::uint32_t>{700});
    QCOMPARE(raw->recording->calls.size(), std::size_t{2});
  }
  void overlayUsageReadsRemainIndependentSamples() {
    auto raw = std::make_shared<RecordingUwf>();
    app::PrecheckedUwf system(raw);
    system.updateState(state());
    std::uint32_t samples = 0;
    raw->recording->action = [&](const Request& request, const Context&) {
      if (!std::holds_alternative<GetOverlayUsage>(request)) return Response::fail(ErrorKind::Unsupported, "Unexpected capability");
      ++samples;
      return Response::success(OverlayUsage{samples, 2048 - samples});
    };
    const auto first = system.execute(GetOverlayUsage{});
    const auto second = system.execute(GetOverlayUsage{});
    QVERIFY(first.succeeded() && second.succeeded());
    QCOMPARE(std::get<OverlayUsage>(first.value), (OverlayUsage{1, 2047}));
    QCOMPARE(std::get<OverlayUsage>(second.value), (OverlayUsage{2, 2046}));
    QCOMPARE(raw->recording->calls, (std::vector<Request>{GetOverlayUsage{}, GetOverlayUsage{}}));
  }
  void overlayUsageValidatesEmptySamplesWithCurrentFacts_data() {
    QTest::addColumn<int>("scenario");
    QTest::addColumn<quint32>("consumption");
    QTest::addColumn<quint32>("available");
    QTest::addColumn<bool>("rejected");
    QTest::addColumn<bool>("dispatched");
    QTest::addColumn<bool>("inference");
    struct Case {
      const char* name;
      int scenario;
      quint32 consumption, available;
      bool rejected, dispatched = true;
    };
    const Case cases[]{
        {"active-disk-empty", 0, 0, 0, true},
        {"active-ram-empty", 1, 0, 0, true},
        {"current-filter-off", 2, 0, 0, false, false},
        {"current-filter-unknown", 3, 0, 0, false},
        {"current-capacity-unknown-next-known", 4, 0, 0, false},
        {"current-capacity-zero-next-positive", 5, 0, 0, false},
        {"only-next-volume-protected", 6, 0, 0, false},
        {"current-volume-absent", 7, 0, 0, false},
        {"current-membership-unknown", 8, 0, 0, false},
        {"current-volume-unprotected", 9, 0, 0, false},
        {"current-protection-unknown", 10, 0, 0, false},
        {"another-current-volume-protected", 11, 0, 0, true},
        {"no-published-facts", 12, 0, 0, false},
        {"unused-ram-limited-by-physical-memory", 1, 0, 1024, false},
        {"pool-full", 0, 2048, 0, false},
        {"consumption-above-configured-limit", 0, 3072, 0, false},
        {"independent-scalar-samples", 0, 1800, 500, false},
    };
    for (const auto& value : cases)
      for (const bool inference : {false, true})
        QTest::newRow(qUtf8Printable(QStringLiteral("%1/inference-%2").arg(QString::fromLatin1(value.name)).arg(inference)))
            << value.scenario << value.consumption << value.available << value.rejected << value.dispatched << inference;
  }
  void overlayUsageValidatesEmptySamplesWithCurrentFacts() {
    QFETCH(int, scenario);
    QFETCH(quint32, consumption);
    QFETCH(quint32, available);
    QFETCH(bool, rejected);
    QFETCH(bool, dispatched);
    QFETCH(bool, inference);
    auto raw = std::make_shared<RecordingUwf>();
    app::PrecheckedUwf system(raw, {}, inference);
    auto facts = state();
    switch (scenario) {
      case 1:
        facts.current.overlayType = Fact<std::uint32_t>::known(0);
        break;
      case 2:
        facts.current.filterEnabled = Fact<bool>::known(false);
        break;
      case 3:
        facts.current.filterEnabled = {};
        break;
      case 4:
        facts.current.maximumOverlaySize = {};
        break;
      case 5:
        facts.current.maximumOverlaySize = Fact<std::uint32_t>::known(0);
        break;
      case 6:
        facts.current.volumes.clear();
        facts.next.volumes[volumeId].protectedVolume = Fact<bool>::known(true);
        break;
      case 7:
        facts.current.volumes[volumeId].present = Fact<bool>::known(false);
        break;
      case 8:
        facts.current.volumes[volumeId].present = {};
        break;
      case 9:
        facts.current.volumes[volumeId].protectedVolume = Fact<bool>::known(false);
        break;
      case 10:
        facts.current.volumes[volumeId].protectedVolume = {};
        break;
      case 11: {
        auto protectedVolume = facts.current.volumes[volumeId];
        facts.current.volumes[volumeId].protectedVolume = Fact<bool>::known(false);
        facts.current.volumes["22222222-3333-4444-5555-666666666666"] = std::move(protectedVolume);
        break;
      }
      case 12:
        facts = {};
        break;
    }
    system.updateState(std::move(facts));
    raw->recording->action = [=](const Request&, const Context&) { return Response::success(OverlayUsage{consumption, available}); };
    const auto response = system.execute(GetOverlayUsage{});
    QCOMPARE(response.succeeded(), !rejected);
    QCOMPARE(response.effect, Effect::None);
    QCOMPARE(raw->recording->calls, dispatched ? std::vector<Request>{GetOverlayUsage{}} : std::vector<Request>{});
    if (rejected) {
      QVERIFY(response.failure);
      QCOMPARE(response.failure->kind, ErrorKind::Unavailable);
      QVERIFY(!system.state().refreshFailure);
      raw->recording->action = [](const Request&, const Context&) { return Response::success(OverlayUsage{0, 2048}); };
      const auto recovered = system.execute(GetOverlayUsage{});
      QVERIFY(recovered.succeeded());
      QCOMPARE(std::get<OverlayUsage>(recovered.value), (OverlayUsage{0, 2048}));
      QCOMPARE(raw->recording->calls, (std::vector<Request>{GetOverlayUsage{}, GetOverlayUsage{}}));
    } else {
      QCOMPARE(std::get<OverlayUsage>(response.value), (OverlayUsage{consumption, available}));
    }
  }
  void configurationRefreshDoesNotSampleOverlayUsage() {
    auto raw = std::make_shared<RecordingUwf>();
    raw->recording->action = [](const Request& request, const Context&) {
      if (std::holds_alternative<GetBoolean>(request)) return Response::success(false);
      if (std::holds_alternative<GetNumber>(request)) return Response::success(std::uint32_t{0});
      if (std::holds_alternative<GetVolumes>(request)) return Response::success(std::vector<Volume>{});
      if (std::holds_alternative<GetRegistryExclusions>(request)) return Response::success(std::vector<std::string>{});
      return Response::fail(ErrorKind::Unsupported, "Unexpected refresh capability");
    };
    const auto refreshed = app::refreshUwfState(*raw);
    QVERIFY2(refreshed.succeeded(), refreshed.failure ? refreshed.failure->message.c_str() : "");
    QVERIFY(std::ranges::none_of(raw->recording->calls, [](const auto& request) { return std::holds_alternative<GetOverlayUsage>(request); }));
  }
  void disabledCurrentRefreshDoesNotRequireTheFullCommitControlGetter() {
    auto raw = std::make_shared<RecordingUwf>();
    raw->recording->action = [](const Request& request, const Context&) {
      if (const auto* boolean = std::get_if<GetBoolean>(&request)) {
        if (boolean->property == BooleanProperty::OverlayCommit) return Response::fail(ErrorKind::Unavailable, "The native control handle is unavailable");
        return Response::success(false);
      }
      if (std::holds_alternative<GetNumber>(request)) return Response::success(std::uint32_t{0});
      if (std::holds_alternative<GetVolumes>(request)) return Response::success(std::vector<Volume>{});
      if (std::holds_alternative<GetRegistryExclusions>(request)) return Response::success(std::vector<std::string>{});
      return Response::fail(ErrorKind::Unsupported, "Unexpected refresh capability");
    };
    // The basic refresher itself skips the inapplicable getter, independently
    // of PrecheckedUwf's equivalent derived-read shortcut.
    QVERIFY(app::refreshUwfState(*raw).succeeded());
    app::PrecheckedUwf system(raw);
    auto facts = state();
    facts.current.filterEnabled = Fact<bool>::known(false);
    facts.runtime.fullCommit = Fact<bool>::known(true);
    facts.runtime.controlDevicePresent = Fact<bool>::known(false);
    system.updateState(std::move(facts));
    QVERIFY(system.refreshState().succeeded());
    QCOMPARE(system.state().runtime.fullCommit.value, std::optional{false});
    QCOMPARE(system.state().runtime.controlDevicePresent.value, std::optional{false});
    QCOMPARE(std::ranges::count(raw->recording->calls, Request{GetBoolean{BooleanProperty::OverlayCommit}}), std::ptrdiff_t{0});
  }
  void publishedStateDoesNotWaitForNativeIo() {
    auto raw = std::make_shared<RecordingUwf>();
    app::PrecheckedUwf system(raw);
    system.updateState(state());
    std::promise<void> entered;
    std::promise<void> release;
    auto enteredFuture = entered.get_future();
    const auto released = release.get_future().share();
    raw->recording->action = [&](const Request&, const Context&) {
      entered.set_value();
      released.wait();
      return Response::success(std::monostate{}, Effect::Confirmed);
    };
    std::future<Response> writer;
    std::future<UwfStateSnapshot> reader;
    bool finished = false;
    const auto cleanup = qScopeGuard([&] {
      if (!finished) release.set_value();
      if (writer.valid()) writer.wait();
      if (reader.valid()) reader.wait();
    });
    writer = std::async(std::launch::async, [&] { return system.execute(SetNumber{NumberProperty::WarningThreshold, 600}); });
    QVERIFY(enteredFuture.wait_for(5s) == std::future_status::ready);
    reader = std::async(std::launch::async, [&] { return system.stateSnapshot(); });
    QVERIFY2(reader.wait_for(1s) == std::future_status::ready, "Published state must remain readable while native I/O is blocked");
    QCOMPARE(reader.get()->runtime.warningThreshold.value, std::optional<std::uint32_t>{512});
    release.set_value();
    finished = true;
    QVERIFY(writer.wait_for(5s) == std::future_status::ready);
    QVERIFY(writer.get().succeeded());
    QCOMPARE(system.state().runtime.warningThreshold.value, std::optional<std::uint32_t>{600});
  }
  void refreshPublishesOneCompleteSnapshotAndNestedSessionsSeeWorkingState() {
    auto raw = std::make_shared<RecordingUwf>();
    app::PrecheckedUwf other(raw);
    std::promise<void> updated;
    std::promise<void> release;
    auto updatedFuture = updated.get_future();
    const auto released = release.get_future().share();
    std::atomic<bool> internalLatest = false;
    raw->recording->action = [](const Request& request, const Context&) {
      const auto* number = std::get_if<GetNumber>(&request);
      if (!number) return Response::fail(ErrorKind::Unsupported, "Unexpected native operation");
      return Response::success(number->property == NumberProperty::WarningThreshold ? std::uint32_t{600} : std::uint32_t{900});
    };
    app::PrecheckedUwf system(raw, {}, true, [&](const Uwf& owner, const Context& context) {
      const auto warning = owner.execute(GetNumber{NumberProperty::WarningThreshold}, context);
      if (warning.failure) return warning;
      other.synchronize([&] {
        const auto critical = owner.execute(GetNumber{NumberProperty::CriticalThreshold}, context);
        if (critical.failure) return;
        auto working = owner.state();
        internalLatest = working.runtime.warningThreshold.value == 600 && working.runtime.criticalThreshold.value == 900;
        working.runtime.shutdownPending = Fact<bool>::known(true);
        owner.updateState(std::move(working));
      });
      updated.set_value();
      released.wait();
      return Response::success();
    });
    system.updateState(state());
    std::future<Response> refresh;
    std::future<UwfStateFacts> reader;
    bool finished = false;
    const auto cleanup = qScopeGuard([&] {
      if (!finished) release.set_value();
      if (refresh.valid()) refresh.wait();
      if (reader.valid()) reader.wait();
    });
    refresh = std::async(std::launch::async, [&] { return system.refreshState(); });
    QVERIFY(updatedFuture.wait_for(5s) == std::future_status::ready);
    reader = std::async(std::launch::async, [&] { return system.state(); });
    QVERIFY(reader.wait_for(1s) == std::future_status::ready);
    const auto before = reader.get();
    QCOMPARE(before.runtime.warningThreshold.value, std::optional<std::uint32_t>{512});
    QCOMPARE(before.runtime.criticalThreshold.value, std::optional<std::uint32_t>{768});
    QVERIFY(!before.runtime.shutdownPending.value);
    QVERIFY(internalLatest.load());
    release.set_value();
    finished = true;
    QVERIFY(refresh.wait_for(5s) == std::future_status::ready);
    QVERIFY(refresh.get().succeeded());
    const auto after = system.state();
    QCOMPARE(after.runtime.warningThreshold.value, std::optional<std::uint32_t>{600});
    QCOMPARE(after.runtime.criticalThreshold.value, std::optional<std::uint32_t>{900});
    QCOMPARE(after.runtime.shutdownPending.value, std::optional{true});
  }
  void failedRefreshPreservesTheWriteEffectAndBlocksWritesUntilRecovery_data() {
    QTest::addColumn<int>("failureMode");
    QTest::newRow("returned-failure") << 0;
    QTest::newRow("standard-exception") << 1;
    QTest::newRow("unknown-exception") << 2;
  }
  void failedRefreshPreservesTheWriteEffectAndBlocksWritesUntilRecovery() {
    QFETCH(int, failureMode);
    auto raw = std::make_shared<RecordingUwf>();
    auto healthy = state();
    healthy.current.filterEnabled = Fact<bool>::known(false);
    bool failing = true;
    int refreshes = 0;
    raw->recording->action = [](const Request& request, const Context&) {
      if (isWrite(request)) return Response::success(std::monostate{}, Effect::Confirmed);
      return Response::success(std::uint32_t{600});
    };
    app::PrecheckedUwf system(
        raw, [healthy](const Request&, const Context&) { return healthy; }, true,
        [&](const Uwf& owner, const Context& context) {
          ++refreshes;
          const auto read = owner.execute(GetNumber{NumberProperty::WarningThreshold}, context);
          if (read.failure) return read;
          if (failing) {
            if (failureMode == 1) throw std::logic_error("State observation failed");
            if (failureMode == 2) throw 42;
            return Response::fail(ErrorKind::AccessDenied, "State observation failed", Effect::None, {{"fixture", "RefreshState", 5, "original error"}});
          }
          return Response::success();
        });
    system.updateState(healthy);
    const auto response = system.execute(ResetSettings{});
    QVERIFY(response.failure);
    QCOMPARE(response.effect, Effect::Confirmed);
    QCOMPARE(response.failure->effect, Effect::Confirmed);
    QVERIFY(response.failure->message.find("write completed") != std::string::npos);
    QVERIFY(system.state().refreshFailure);
    QCOMPARE(system.state().runtime.warningThreshold.value, std::optional<std::uint32_t>{600});
    auto replacement = system.state();
    replacement.refreshFailure.reset();
    system.updateState(std::move(replacement));
    QVERIFY(system.state().refreshFailure);
    const auto calls = raw->recording->calls.size();
    const auto blocked = system.execute(SetNumber{NumberProperty::CriticalThreshold, 900}, {{}, &healthy});
    QVERIFY(blocked.failure);
    QCOMPARE(blocked.effect, Effect::None);
    QCOMPARE(raw->recording->calls.size(), calls);
    QVERIFY(system.execute(GetNumber{NumberProperty::WarningThreshold}).succeeded());
    QVERIFY(system.state().refreshFailure);
    failing = false;
    QVERIFY(system.refreshState().succeeded());
    QVERIFY(!system.state().refreshFailure);
    QVERIFY(system.execute(SetNumber{NumberProperty::CriticalThreshold, 900}).succeeded());
    QCOMPARE(refreshes, 2);
    std::vector<Request> writes;
    for (const auto& request : raw->recording->calls)
      if (isWrite(request)) writes.push_back(request);
    QCOMPARE(writes, (std::vector<Request>{ResetSettings{}, SetNumber{NumberProperty::CriticalThreshold, 900}}));
  }
  void thresholdWriteUsesOnlyItsOwnPrimitiveAndUpdatesMemory() {
    auto raw = std::make_shared<RecordingUwf>();
    app::PrecheckedUwf system(raw);
    system.updateState(state());
    const Request request = SetNumber{NumberProperty::WarningThreshold, 600};
    QVERIFY(system.execute(request).succeeded());
    QCOMPARE(raw->recording->calls, std::vector<Request>{request});
    QCOMPARE(system.state().runtime.warningThreshold.value, std::optional<std::uint32_t>{600});
    QCOMPARE(system.state().runtime.criticalThreshold.value, std::optional<std::uint32_t>{768});
  }
  void resetGetterDoesNotInventAControlDevice() {
    auto raw = std::make_shared<RecordingUwf>();
    app::PrecheckedUwf system(raw);
    auto facts = state();
    facts.runtime.controlDevicePresent = Fact<bool>::known(false);
    facts.runtime.resetState = Fact<std::uint32_t>::known(1);
    system.updateState(facts);
    raw->recording->action = [](const Request&, const Context&) { return Response::success(std::uint32_t{0}); };
    QVERIFY(system.execute(GetNumber{NumberProperty::ResetPersistentState}).succeeded());
    QCOMPARE(system.state().runtime.resetState.value, std::optional<std::uint32_t>{0});
    QCOMPARE(system.state().runtime.controlDevicePresent.value, std::optional{false});
    QCOMPARE(raw->recording->calls.size(), std::size_t{1});
  }
  void filterWritesPublishKnownEffectsAndObserveTheOpaqueReset_data() {
    QTest::addColumn<int>("scenario");
    QTest::newRow("loaded-rom-disposition") << 0;
    QTest::newRow("persistent-runtime-reset") << 1;
    QTest::newRow("no-device-disk-passthrough") << 2;
  }
  void filterWritesPublishKnownEffectsAndObserveTheOpaqueReset() {
    QFETCH(int, scenario);
    auto raw = std::make_shared<RecordingUwf>();
    app::PrecheckedUwf system(raw);
    auto facts = state();
    facts.current.servicingEnabled = facts.next.servicingEnabled = Fact<bool>::known(false);
    facts.next.filterEnabled = Fact<bool>::known(true);
    facts.runtime.resetState = Fact<std::uint32_t>::known(0);
    if (scenario == 0) facts.current.overlayFlags = BitFacts::known(flags::readOnlyMedia);
    if (scenario == 1) facts.current.overlayFlags = BitFacts::known(flags::persistent);
    if (scenario == 2) {
      facts.current.filterEnabled = facts.next.filterEnabled = Fact<bool>::known(false);
      facts.runtime.controlDevicePresent = Fact<bool>::known(false);
    }
    system.updateState(facts);
    const Request request = SetBoolean{BooleanProperty::FilterEnabled, scenario == 2};
    const auto response = system.execute(request);
    QCOMPARE(response.succeeded(), scenario == 2);
    QCOMPARE(response.effect, Effect::Confirmed);
    const auto published = system.state();
    QCOMPARE(std::ranges::count(raw->recording->calls, request), std::ptrdiff_t{1});
    QCOMPARE(std::ranges::count(raw->recording->calls, Request{GetNumber{NumberProperty::ResetPersistentState}}),
             scenario == 2 ? std::ptrdiff_t{0} : std::ptrdiff_t{1});
    QCOMPARE(published.refreshFailure.has_value(), scenario != 2);
    QCOMPARE(published.next.filterEnabled.value, std::optional{scenario == 2});
    QCOMPARE(published.current.filterEnabled.value, facts.current.filterEnabled.value);
    QCOMPARE(published.runtime.romDisposition.valueBits, scenario == 0 ? disposition::filterChange : 0u);
    // A successful setter cannot supply the driver's reset-cache value. Failed
    // observation retains the previous value and marks the snapshot unhealthy.
    QCOMPARE(published.runtime.resetState.value, std::optional<std::uint32_t>{0});
    QCOMPARE(published.next.overlayFlags.valueBits, scenario == 2 ? flags::passthrough : 0u);
  }
  void hormTransitionsPublishTheirDispositionBeforeAnotherCommand() {
    auto raw = std::make_shared<RecordingUwf>();
    app::PrecheckedUwf system(raw);
    auto facts = state();
    facts.current.overlayType = Fact<std::uint32_t>::known(static_cast<std::uint32_t>(OverlayType::Ram));
    facts.current.overlayFlags = BitFacts::known(flags::readOnlyMedia);
    facts.runtime.hiberFile = facts.runtime.hormVolumesProtected = Fact<bool>::known(true);
    system.updateState(facts);
    QVERIFY(system.execute(SetBoolean{BooleanProperty::HormEnabled, true}).succeeded());
    QCOMPARE(system.state().runtime.horm.value, std::optional{true});
    QCOMPARE(system.state().runtime.romDisposition.valueBits, disposition::horm);
    QVERIFY(system.execute(SetBoolean{BooleanProperty::HormEnabled, false}).succeeded());
    QCOMPARE(system.state().runtime.horm.value, std::optional{false});
    QCOMPARE(system.state().runtime.romDisposition.valueBits, disposition::horm | disposition::filterChange);
    const auto commit = system.execute(SetBoolean{BooleanProperty::OverlayCommit, true});
    QVERIFY(commit.failure);
    QCOMPARE(commit.failure->kind, ErrorKind::Conflict);
    QCOMPARE(commit.effect, Effect::None);
    QCOMPARE(raw->recording->calls.size(), std::size_t{2});
  }
  void resettingDefaultsRefreshesLiveValuesWithoutLosingTheWriteReceipt_data() {
    QTest::addColumn<bool>("failCritical");
    QTest::newRow("complete-refresh") << false;
    QTest::newRow("failed-read-preserves-published-value") << true;
  }
  void resettingDefaultsRefreshesLiveValuesWithoutLosingTheWriteReceipt() {
    QFETCH(bool, failCritical);
    auto raw = std::make_shared<RecordingUwf>();
    app::PrecheckedUwf system(raw);
    auto facts = state();
    facts.current.filterEnabled = Fact<bool>::known(false);
    facts.runtime.warningThreshold = Fact<std::uint32_t>::known(600);
    facts.runtime.criticalThreshold = Fact<std::uint32_t>::known(900);
    facts.runtime.controlDevicePresent = Fact<bool>::known(false);
    system.updateState(facts);
    raw->recording->action = [failCritical](const Request& request, const Context&) {
      if (std::holds_alternative<ResetSettings>(request)) return Response::success(std::monostate{}, Effect::Confirmed);
      if (std::holds_alternative<GetBoolean>(request)) return Response::success(false);
      if (const auto* number = std::get_if<GetNumber>(&request)) {
        if (number->property == NumberProperty::CriticalThreshold && failCritical)
          return Response::fail(ErrorKind::AccessDenied, "Critical threshold refresh denied", Effect::None,
                                {{"fixture", "GetCriticalThreshold", 5, "original read error"}});
        const auto value = number->property == NumberProperty::MaximumOverlaySize  ? 1024u
                           : number->property == NumberProperty::WarningThreshold  ? 256u
                           : number->property == NumberProperty::CriticalThreshold ? 512u
                                                                                   : 0u;
        return Response::success(value);
      }
      if (const auto* inventory = std::get_if<GetVolumes>(&request))
        return Response::success(inventory->session == Session::Current ? std::vector<Volume>{{volumeId, "C:", true, false, {}}} : std::vector<Volume>{});
      if (std::holds_alternative<GetFileExclusions>(request) || std::holds_alternative<GetRegistryExclusions>(request))
        return Response::success(std::vector<std::string>{});
      if (std::holds_alternative<GetSwapfileSize>(request)) return Response::success(std::uint32_t{0});
      return Response::fail(ErrorKind::Unsupported, "Unexpected refresh capability");
    };
    const auto response = system.execute(ResetSettings{});
    QCOMPARE(response.succeeded(), !failCritical);
    QCOMPARE(response.effect, Effect::Confirmed);
    QCOMPARE(response.warnings.empty(), !failCritical);
    const auto published = system.state();
    QCOMPARE(published.refreshFailure.has_value(), failCritical);
    QCOMPARE(published.runtime.warningThreshold.value, std::optional<std::uint32_t>{256});
    QCOMPARE(published.runtime.criticalThreshold.value, std::optional<std::uint32_t>{failCritical ? 900u : 512u});
    QCOMPARE(published.runtime.controlDevicePresent.value, std::optional{false});
    QCOMPARE(published.current.volumes.at(volumeId).present.value, std::optional{true});
    QCOMPARE(published.next.volumes.at(volumeId).present.value, std::optional{false});
    QCOMPARE(raw->recording->calls.front(), Request{ResetSettings{}});
    QCOMPARE(std::ranges::count_if(raw->recording->calls, [](const auto& request) { return isWrite(request); }), std::ptrdiff_t{1});
  }
  void sameTypeWritePreservesFlagsForTheNextBitEdit() {
    auto raw = std::make_shared<RecordingUwf>();
    app::PrecheckedUwf system(raw);
    auto facts = state();
    facts.current.filterEnabled = Fact<bool>::known(false);
    facts.next.overlayType = Fact<std::uint32_t>::known(static_cast<std::uint32_t>(OverlayType::Disk));
    facts.next.overlayFlags = BitFacts::known(0x80000000u);
    system.updateState(facts);
    QVERIFY(system.execute(SetNumber{NumberProperty::OverlayType, static_cast<std::uint32_t>(OverlayType::Disk)}).succeeded());
    QCOMPARE(system.state().next.overlayFlags.valueBits, 0x80000000u);
    QVERIFY(system.execute(SetOverlayOption{OverlayOption::Persistent, true}).succeeded());
    QCOMPARE(raw->recording->calls, (std::vector<Request>{SetNumber{NumberProperty::OverlayType, static_cast<std::uint32_t>(OverlayType::Disk)},
                                                          SetNumber{NumberProperty::OverlayFlags, 0x80000002u}}));
  }
  void swapfileCapacityObservationsPreservePrimitiveReceipts_data() {
    QTest::addColumn<bool>("capacityKnown");
    QTest::addColumn<quint32>("capacity");
    QTest::addColumn<bool>("inference");
    QTest::addColumn<int>("receipt");
    struct Case {
      const char* name;
      bool known;
      quint32 capacity;
    };
    const Case cases[]{
        {"ram-512", true, 512}, {"unknown", false, 0}, {"zero", true, 0}, {"signed-high-bit", true, 0x80000000u}, {"dword-max", true, 0xffffffffu}};
    constexpr const char* receipts[]{"success", "failed-before-effect", "failed-after-effect"};
    for (const auto& value : cases)
      for (const bool inference : {false, true})
        for (int receipt = 0; receipt < 3; ++receipt)
          QTest::newRow(qUtf8Printable(
              QStringLiteral("%1/inference-%2/%3").arg(QString::fromLatin1(value.name)).arg(inference).arg(QString::fromLatin1(receipts[receipt]))))
              << value.known << value.capacity << inference << receipt;
  }
  void swapfileCapacityObservationsPreservePrimitiveReceipts() {
    QFETCH(bool, capacityKnown);
    QFETCH(quint32, capacity);
    QFETCH(bool, inference);
    QFETCH(int, receipt);
    auto raw = std::make_shared<RecordingUwf>();
    int refreshes = 0;
    app::PrecheckedUwf system(raw, {}, inference, [&](const Uwf&, const Context&) {
      ++refreshes;
      return Response::success();
    });
    auto facts = state();
    facts.current.filterEnabled = Fact<bool>::known(false);
    facts.next.overlayType = Fact<std::uint32_t>::known(static_cast<std::uint32_t>(OverlayType::Ram));
    facts.next.maximumOverlaySize = capacityKnown ? Fact<std::uint32_t>::known(capacity) : Fact<std::uint32_t>{};
    system.updateState(std::move(facts));
    const auto primitive =
        receipt == 0 ? Response::success(std::monostate{}, Effect::Confirmed)
                     : Response::fail(ErrorKind::InvalidArgument, "Native swapfile file operation failed", receipt == 1 ? Effect::None : Effect::Uncertain,
                                      {{"Native", "UwfCfgSetVolumeSwapfileSize", static_cast<std::int32_t>(0x80070057u), "Native file operation result"}});
    raw->recording->action = [primitive](const Request&, const Context&) { return primitive; };
    const auto response = system.execute(CreateSwapfile{volumeId});
    QCOMPARE(raw->recording->calls, (std::vector<Request>{CreateSwapfile{volumeId}}));
    QCOMPARE(response.succeeded(), primitive.succeeded());
    QCOMPARE(response.effect, primitive.effect);
    QCOMPARE(refreshes, receipt == 1 ? 0 : 1);
    QVERIFY(!system.state().refreshFailure);
    if (primitive.failure) {
      QVERIFY(response.failure);
      QCOMPARE(response.failure->kind, primitive.failure->kind);
      QCOMPARE(response.failure->effect, primitive.failure->effect);
      QCOMPARE(response.failure->message, primitive.failure->message);
      QCOMPARE(response.failure->diagnostics.size(), std::size_t{1});
      const auto& diagnostic = response.failure->diagnostics.front();
      const auto& expected = primitive.failure->diagnostics.front();
      QCOMPARE(diagnostic.backend, expected.backend);
      QCOMPARE(diagnostic.operation, expected.operation);
      QCOMPARE(diagnostic.code, expected.code);
      QCOMPARE(diagnostic.detail, expected.detail);
    } else {
      QCOMPARE(system.state().next.overlayType.value, std::optional<std::uint32_t>{static_cast<std::uint32_t>(OverlayType::Disk)});
      QCOMPARE(system.state().next.volumes.at(volumeId).swapfileMarker.value, std::optional<std::uint32_t>{1});
    }
  }
  void selectingSwapfileKeepsItsReceiptWhenConfigurationRefreshFails_data() {
    QTest::addColumn<bool>("wasDisk");
    QTest::newRow("ram-to-disk") << false;
    QTest::newRow("disk-via-ram-to-disk") << true;
  }
  void selectingSwapfileKeepsItsReceiptWhenConfigurationRefreshFails() {
    QFETCH(bool, wasDisk);
    auto raw = std::make_shared<RecordingUwf>();
    app::PrecheckedUwf system(raw);
    auto facts = state();
    facts.current.filterEnabled = Fact<bool>::known(false);
    facts.next.overlayType = Fact<std::uint32_t>::known(static_cast<std::uint32_t>(wasDisk ? OverlayType::Disk : OverlayType::Ram));
    facts.next.overlayFlags = BitFacts::known(0x80000002u);
    system.updateState(facts);
    raw->recording->action = [](const Request& request, const Context&) {
      return isWrite(request) ? Response::success(std::monostate{}, Effect::Confirmed)
                              : Response::fail(ErrorKind::Unavailable, "Configuration observation unavailable");
    };
    const auto response = system.execute(CreateSwapfile{volumeId});
    QVERIFY(response.failure);
    QVERIFY(response.failure->message.find("write completed") != std::string::npos);
    QCOMPARE(response.effect, Effect::Confirmed);
    QVERIFY(!response.warnings.empty());
    std::vector<Request> writes;
    for (const auto& request : raw->recording->calls)
      if (isWrite(request)) writes.push_back(request);
    QCOMPARE(writes, (std::vector<Request>{CreateSwapfile{volumeId}}));
    for (const auto session : {Session::Current, Session::Next}) {
      QCOMPARE(std::ranges::count(raw->recording->calls, Request{GetBoolean{BooleanProperty::FilterEnabled, session}}), std::ptrdiff_t{1});
      QCOMPARE(std::ranges::count(raw->recording->calls, Request{GetNumber{NumberProperty::OverlayFlags, session}}), std::ptrdiff_t{1});
    }
    const auto published = system.state();
    QVERIFY(published.refreshFailure);
    QCOMPARE(published.next.overlayType.value, std::optional<std::uint32_t>{static_cast<std::uint32_t>(OverlayType::Disk)});
    QCOMPARE(published.next.overlayFlags.knownMask, facts.next.overlayFlags.knownMask);
    QCOMPARE(published.next.overlayFlags.valueBits, wasDisk ? 0x80000001u : 0x80000003u);
    QCOMPARE(published.next.volumes.at(volumeId).swapfileMarker.value, std::optional<std::uint32_t>{1});
  }
  void partialSwapfileCreationKeepsEachAcknowledgedPrimitive() {
    auto raw = std::make_shared<RecordingUwf>();
    app::PrecheckedUwf system(raw);
    auto facts = state();
    facts.current.filterEnabled = Fact<bool>::known(false);
    facts.next.volumes.clear();
    facts.targets[volumeId].volumeEligibility = Fact<VolumeAdmission>::known({true, true});
    system.updateState(facts);
    raw->recording->action = [](const Request& request, const Context&) {
      if (!isWrite(request)) return Response::fail(ErrorKind::Unavailable, "Refresh data unavailable");
      if (std::holds_alternative<CreateSwapfile>(request)) return Response::fail(ErrorKind::Failed, "Marker write failed", Effect::None);
      return Response::success(std::monostate{}, Effect::Confirmed);
    };
    const auto response = system.execute(CreateSwapfile{volumeId});
    QVERIFY(response.failure);
    QCOMPARE(response.effect, Effect::Confirmed);
    QCOMPARE(std::ranges::count_if(raw->recording->calls, [](const auto& request) { return isWrite(request); }), std::ptrdiff_t{4});
    QCOMPARE(std::ranges::count(raw->recording->calls, Request{CreateSwapfile{volumeId}}), std::ptrdiff_t{1});
    const auto published = system.state();
    QCOMPARE(published.next.volumes.at(volumeId).present.value, std::optional{true});
    QCOMPARE(published.next.volumes.at(volumeId).protectedVolume.value, std::optional{false});
    QCOMPARE(published.next.volumes.at(volumeId).bindByDriveLetter.value, std::optional{false});
    QCOMPARE(published.next.volumes.at(volumeId).swapfileMarker.value, std::optional<std::uint32_t>{0});
    QCOMPARE(published.next.overlayType.value, facts.next.overlayType.value);
    QCOMPARE(published.next.overlayFlags.valueBits, facts.next.overlayFlags.valueBits);
  }
  void leavingServicingDoesNotSkipASubsequentFilterWrite() {
    auto raw = std::make_shared<RecordingUwf>();
    int refreshes = 0;
    app::PrecheckedUwf system(raw, {}, true, [&](const Uwf& owner, const Context& context) {
      ++refreshes;
      const auto filter = owner.execute(GetBoolean{BooleanProperty::FilterEnabled, Session::Next}, context);
      if (filter.failure) return filter;
      const auto reset = owner.execute(GetNumber{NumberProperty::ResetPersistentState}, context);
      if (reset.failure) return reset;
      auto facts = owner.state();
      facts.runtime.servicingStatus = Fact<std::uint32_t>::known(servicingStatus::completed);
      owner.updateState(std::move(facts));
      return Response::success();
    });
    auto facts = state();
    facts.runtime.servicingStatus = Fact<std::uint32_t>::known(servicingStatus::started);
    facts.runtime.savedServicingFilter = Fact<std::optional<bool>>::known(true);
    system.updateState(facts);
    bool nativeNextFilter = false;
    raw->recording->action = [&](const Request& request, const Context&) {
      if (const auto* value = std::get_if<SetBoolean>(&request)) {
        nativeNextFilter = value->property == BooleanProperty::FilterEnabled ? value->value : true;
        return Response::success(std::monostate{}, Effect::Confirmed);
      }
      const auto* value = std::get_if<GetBoolean>(&request);
      if (value && value->property == BooleanProperty::FilterEnabled) return Response::success(nativeNextFilter);
      if (std::holds_alternative<GetNumber>(request)) return Response::success(std::uint32_t{0});
      return Response::fail(ErrorKind::Unavailable, "This fixture publishes only the restored filter");
    };
    QVERIFY(system.execute(SetBoolean{BooleanProperty::ServicingEnabled, false}).succeeded());
    QCOMPARE(system.state().next.filterEnabled.value, std::optional{true});
    QCOMPARE(system.state().runtime.servicingStatus.value, std::optional<std::uint32_t>{servicingStatus::completed});
    QVERIFY(system.execute(SetBoolean{BooleanProperty::FilterEnabled, false}).succeeded());
    std::vector<Request> writes;
    for (const auto& request : raw->recording->calls)
      if (isWrite(request)) writes.push_back(request);
    QCOMPARE(writes, (std::vector<Request>{SetBoolean{BooleanProperty::ServicingEnabled, false}, SetBoolean{BooleanProperty::FilterEnabled, false}}));
    QCOMPARE(refreshes, 2);
    QVERIFY(std::ranges::find(raw->recording->calls, Request{GetBoolean{BooleanProperty::FilterEnabled, Session::Next}}) != raw->recording->calls.end());
  }
  void optionIntentComposesOneRawWriteFromThePublishedOpaqueFlags() {
    auto raw = std::make_shared<RecordingUwf>();
    app::PrecheckedUwf system(raw);
    auto facts = state();
    facts.current.filterEnabled = Fact<bool>::known(false);
    facts.next.overlayFlags = BitFacts::known(0x80000001u);
    system.updateState(facts);
    QVERIFY(system.execute(SetOverlayOption{OverlayOption::Persistent, true}).succeeded());
    QCOMPARE(raw->recording->calls, (std::vector<Request>{SetNumber{NumberProperty::OverlayFlags, 0x80000003u}}));
    QCOMPARE(system.state().next.overlayFlags.valueBits, 0x80000003u);
  }
  void clearUsesOnlyOneRemovalPerPublishedEntryAndKeepsPartialEffects_data() {
    QTest::addColumn<bool>("failSecond");
    QTest::newRow("all-removed") << false;
    QTest::newRow("second-fails") << true;
  }
  void clearUsesOnlyOneRemovalPerPublishedEntryAndKeepsPartialEffects() {
    QFETCH(bool, failSecond);
    auto raw = std::make_shared<RecordingUwf>();
    app::PrecheckedUwf system(raw);
    auto facts = state();
    facts.next.registryExclusions = Fact<std::vector<std::string>>::known(
        {"HKEY_LOCAL_MACHINE\\SOFTWARE\\One", "HKEY_LOCAL_MACHINE\\SOFTWARE\\Two", "HKEY_LOCAL_MACHINE\\SOFTWARE\\Three"});
    system.updateState(facts);
    raw->recording->action = [raw, failSecond](const Request& request, const Context&) {
      if (!isWrite(request)) return Response::fail(ErrorKind::Unavailable, "Refresh data unavailable");
      if (failSecond && raw->recording->calls.size() == 2)
        return Response::fail(ErrorKind::Failed, "Second removal failed", Effect::None, {{"fixture", "Remove", 42, "Original failure"}});
      return Response::success(std::monostate{}, Effect::Confirmed);
    };
    const auto response = system.execute(EditRegistryExclusion{{}, ExclusionAction::Clear});
    QCOMPARE(response.succeeded(), !failSecond);
    QCOMPARE(response.effect, Effect::Confirmed);
    QCOMPARE(std::ranges::count_if(raw->recording->calls, [](const auto& request) { return isWrite(request); }),
             failSecond ? std::ptrdiff_t{2} : std::ptrdiff_t{3});
    for (const auto& request : raw->recording->calls)
      if (isWrite(request)) QCOMPARE(std::get<EditRegistryExclusion>(request).action, ExclusionAction::Remove);
    if (failSecond) {
      QCOMPARE(response.failure->diagnostics.back().code, std::int64_t{42});
      QCOMPARE(system.state().next.registryExclusions.value,
               (std::optional<std::vector<std::string>>{{"HKEY_LOCAL_MACHINE\\SOFTWARE\\Two", "HKEY_LOCAL_MACHINE\\SOFTWARE\\Three"}}));
    } else {
      QVERIFY(system.state().next.registryExclusions.value->empty());
    }
    raw->recording->action = {};
  }
  void missingVolumeCompositionCanRouteEachPrimitiveToADifferentBackend() {
    auto registration = std::make_shared<Recording>();
    auto protection = std::make_shared<Recording>();
    auto binding = std::make_shared<Recording>();
    auto first =
        std::make_shared<RecordingBackend>("registration", registration, [](const Request& request) { return std::holds_alternative<AddVolume>(request); });
    auto second = std::make_shared<RecordingBackend>("protection", protection, [](const Request& request) {
      const auto* value = std::get_if<SetVolume>(&request);
      return value && value->property == VolumeProperty::Protected;
    });
    auto third = std::make_shared<RecordingBackend>("binding", binding, [](const Request& request) {
      const auto* value = std::get_if<SetVolume>(&request);
      return value && value->property == VolumeProperty::BindByDriveLetter;
    });
    auto router = std::make_shared<FallbackUwf>(std::vector<std::shared_ptr<Backend>>{first, second, third});
    app::PrecheckedUwf system(router);
    auto facts = state();
    facts.current.filterEnabled = Fact<bool>::known(false);
    facts.next.volumes.clear();
    facts.targets[volumeId].volumeEligibility = Fact<VolumeAdmission>::known({true, true});
    system.updateState(facts);
    const auto response = system.execute(SetVolume{volumeId, VolumeProperty::Protected, true, "C:", true});
    QVERIFY(response.failure);
    QVERIFY(system.state().refreshFailure);
    QCOMPARE(response.effect, Effect::Confirmed);
    QCOMPARE(registration->calls, (std::vector<Request>{AddVolume{volumeId, "C:"}}));
    QCOMPARE(protection->calls, (std::vector<Request>{SetVolume{volumeId, VolumeProperty::Protected, true}}));
    QCOMPARE(binding->calls, (std::vector<Request>{SetVolume{volumeId, VolumeProperty::BindByDriveLetter, true}}));
    QCOMPARE(system.state().next.volumes.at(volumeId).present.value, std::optional{true});
    QCOMPARE(system.state().next.volumes.at(volumeId).bindByDriveLetter.value, std::optional{true});
  }
  void derivedFindAndRegistryPersistenceArePureStateOperations() {
    auto raw = std::make_shared<RecordingUwf>();
    app::PrecheckedUwf system(raw);
    auto facts = state();
    facts.current.volumes[volumeId].fileExclusions = Fact<std::vector<std::string>>::known({"\\Data"});
    facts.current.registryExclusions = Fact<std::vector<std::string>>::known({"HKEY_LOCAL_MACHINE\\SOFTWARE\\Example"});
    system.updateState(facts);
    const auto file = system.execute(FindFileExclusion{volumeId, "\\DATA"});
    const auto registry = system.execute(FindRegistryExclusion{"hkey_local_machine\\software\\example"});
    const auto durability = system.execute(GetBoolean{BooleanProperty::RegistryChangesAlwaysPersisted});
    QVERIFY(file.succeeded() && std::get<bool>(file.value));
    QVERIFY(registry.succeeded() && std::get<bool>(registry.value));
    QVERIFY(durability.succeeded() && !std::get<bool>(durability.value));
    facts.current.filterEnabled = Fact<bool>::known(false);
    system.updateState(facts);
    const auto persistent = system.execute(GetBoolean{BooleanProperty::RegistryChangesAlwaysPersisted});
    QVERIFY(persistent.succeeded() && std::get<bool>(persistent.value));
    QVERIFY(raw->recording->calls.empty());
  }
  void unknownCompositionInputsFailWithoutProbingForTheirValues() {
    auto raw = std::make_shared<RecordingUwf>();
    app::PrecheckedUwf system(raw);
    system.updateState({});
    const std::vector<Request> requests{SetOverlayOption{OverlayOption::Persistent, true},
                                        EditRegistryExclusion{{}, ExclusionAction::Clear},
                                        CreateSwapfile{volumeId},
                                        SetVolume{volumeId, VolumeProperty::Protected, true},
                                        FindRegistryExclusion{"HKEY_LOCAL_MACHINE\\SOFTWARE\\Example"},
                                        GetBoolean{BooleanProperty::RegistryChangesAlwaysPersisted}};
    for (const auto& request : requests) {
      const auto response = system.execute(request);
      QVERIFY(response.failure.has_value());
      QCOMPARE(response.effect, Effect::None);
    }
    QVERIFY(raw->recording->calls.empty());
  }
  void explicitReadsPublishTheirValuesAndFailedReadsPreserveTheKnownField() {
    auto raw = std::make_shared<RecordingUwf>();
    app::PrecheckedUwf system(raw);
    system.updateState(state());
    bool fail = false;
    raw->recording->action = [&](const Request& request, const Context&) {
      if (fail) return Response::fail(ErrorKind::Failed, "Explicit read failed");
      if (std::holds_alternative<GetBoolean>(request)) return Response::success(false);
      return Response::success(std::uint32_t{123});
    };
    const Request number = GetNumber{NumberProperty::WarningThreshold};
    QVERIFY(system.execute(number).succeeded());
    QCOMPARE(system.state().runtime.warningThreshold.value, std::optional<std::uint32_t>{123});
    fail = true;
    QVERIFY(system.execute(number).failure.has_value());
    QCOMPARE(system.state().runtime.warningThreshold.value, std::optional<std::uint32_t>{123});
    QCOMPARE(system.state().runtime.criticalThreshold.value, std::optional<std::uint32_t>{768});
    fail = false;
    const Request boolean = GetBoolean{BooleanProperty::FilterEnabled};
    QVERIFY(system.execute(boolean).succeeded());
    QCOMPARE(system.state().current.filterEnabled.value, std::optional{false});
    fail = true;
    QVERIFY(system.execute(boolean).failure.has_value());
    QCOMPARE(system.state().current.filterEnabled.value, std::optional{false});
    QCOMPARE(raw->recording->calls, (std::vector<Request>{number, number, boolean, boolean}));
  }
  void removingAnExclusionFromAKnownMissingVolumeIsAPureNoOp() {
    auto raw = std::make_shared<RecordingUwf>();
    app::PrecheckedUwf system(raw);
    auto published = state();
    published.next.volumes.clear();
    system.updateState(std::move(published));
    const auto removed = system.execute(EditFileExclusion{volumeId, "\\missing", ExclusionAction::Remove});
    QVERIFY(removed.succeeded());
    QCOMPARE(removed.effect, Effect::None);
    QVERIFY(raw->recording->calls.empty());
    const auto unprotected = system.execute(SetVolume{volumeId, VolumeProperty::Protected, false});
    QVERIFY(unprotected.failure);
    QCOMPARE(unprotected.failure->kind, ErrorKind::NotFound);
    QCOMPARE(unprotected.effect, Effect::None);
    QVERIFY(raw->recording->calls.empty());
  }
  void uncertainWriteRefreshesStateWithoutReplayingOnAnotherBackend() {
    auto failed = std::make_shared<Recording>();
    auto other = std::make_shared<Recording>();
    failed->action = [](const Request& request, const Context&) {
      return isWrite(request) ? Response::fail(ErrorKind::Failed, "Lost write receipt", Effect::Uncertain)
                              : Response::fail(ErrorKind::Unavailable, "Refresh data unavailable");
    };
    const auto accepts = [](const Request&) { return true; };
    auto first = std::make_shared<RecordingBackend>("failed", failed, accepts);
    auto second = std::make_shared<RecordingBackend>("other", other, accepts);
    auto router = std::make_shared<FallbackUwf>(std::vector<std::shared_ptr<Backend>>{first, second});
    app::PrecheckedUwf system(router);
    system.updateState(state());
    const Request request = SetNumber{NumberProperty::WarningThreshold, 600};
    const auto response = system.execute(request);
    QVERIFY(response.failure.has_value());
    QCOMPARE(response.effect, Effect::Uncertain);
    QCOMPARE(std::ranges::count_if(failed->calls, [](const auto& candidate) { return isWrite(candidate); }), std::ptrdiff_t{1});
    QCOMPARE(failed->calls.front(), request);
    QVERIFY(std::ranges::none_of(other->calls, [](const auto& candidate) { return isWrite(candidate); }));
    QVERIFY(failed->calls.size() > 1);
    QCOMPARE(system.state().runtime.warningThreshold.value, std::optional<std::uint32_t>{512});
  }
};
}  // namespace

QTEST_APPLESS_MAIN(UwfStateExecutionTests)
#include "UwfStateExecutionTests.moc"
