// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QTest>
#include <algorithm>
#include <array>
#include <functional>
#include <memory>
#include <optional>
#include <stop_token>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "application/uwf/UwfRequestPrecheck.h"
#include "support/uwf/CapturedSolverOracle.h"
#include "support/uwf/FakeUwfBackend.h"
namespace {
using namespace uwf;
using namespace compat;
using Fake = test::FakeUwfBackend;
namespace oracle = test::solver_oracle;

constexpr auto path = "\\selected";
constexpr auto excluded = "\\excluded";
enum class Scene {
  FullOff,
  FullDispositionConflict,
  FullCopy,
  FullCopyFailure,
  SwapMissingOn,
  ServicingCompleted,
  HormNoOp,
  FileNoCopy,
  DiagnosticCandidates,
  MissingDeletion,
  DefaultRegistryCommit,
  InvalidDefaultDeletion,
  FalseProtection,
  NextFilter,
  FilterInvalidType,
  FilterSameInvalidType,
  ServicingRestoreFailed,
  ServicingIdleDisableWithoutControl,
  ServicingStartedEnableWithoutControl,
  ServicingFailedDisableWithoutControl
};
struct Case {
  const char* id;
  Scene scene;
  bool solverReject;
  bool backendSuccess;
};
constexpr std::array<Case, 20> cases{{{"full-off", Scene::FullOff, true, false},
                                      {"full-disposition3", Scene::FullDispositionConflict, true, false},
                                      {"full-successful-copies", Scene::FullCopy, false, true},
                                      {"full-copy-errors-do-not-change-receipt", Scene::FullCopyFailure, false, true},
                                      {"swap-missing-target-late-offline-check", Scene::SwapMissingOn, true, false},
                                      {"servicing-completed-late-failure", Scene::ServicingCompleted, true, false},
                                      {"horm-same-enabled-without-device", Scene::HormNoOp, false, true},
                                      {"file-excluded-no-copy-with-unready-view", Scene::FileNoCopy, false, true},
                                      {"diagnostic-metadata-candidates", Scene::DiagnosticCandidates, false, true},
                                      {"delete-missing-visible-target", Scene::MissingDeletion, false, false},
                                      {"commit-default-registry-value", Scene::DefaultRegistryCommit, false, true},
                                      {"default-registry-deletion-not-expressible", Scene::InvalidDefaultDeletion, true, false},
                                      {"read-false-is-not-absence", Scene::FalseProtection, false, true},
                                      {"read-next-not-current", Scene::NextFilter, false, true},
                                      {"filter-invalid-next-type-after-disposition", Scene::FilterInvalidType, true, false},
                                      {"filter-matching-next-skips-invalid-type", Scene::FilterSameInvalidType, false, true},
                                      {"servicing-receipt-keeps-failed-runtime-status", Scene::ServicingRestoreFailed, false, true},
                                      {"servicing-idle-disable-without-control", Scene::ServicingIdleDisableWithoutControl, false, true},
                                      {"servicing-started-enable-without-control", Scene::ServicingStartedEnableWithoutControl, false, true},
                                      {"servicing-failed-disable-saved-false-without-control", Scene::ServicingFailedDisableWithoutControl, false, true}}};
Fake::State initial(bool enabled = false, std::uint32_t flags = 0) {
  Fake::State state;
  state.current.filterEnabled = enabled;
  state.current.overlayFlags = flags;
  state.current.volumes.emplace(oracle::volume,
                                Fake::VolumeConfiguration{.volume = {oracle::volume, "C:", true, false, false}, .fileExclusions = {}, .swapfileSize = 0});
  state.next = state.current;
  state.capturedSettings = state.next;
  state.runtime.systemVolumeId = oracle::volume;
  state.runtime.fileMaintenancePrefixes[oracle::volume] = "C:";
  return state;
}
struct Fixture {
  std::shared_ptr<Fake> backend;
  Request request;
  UwfStateFacts facts;
};
Fixture fixture(Scene scene) {
  auto state = initial();
  Request request = GetBoolean{BooleanProperty::FilterEnabled};
  UwfStateFacts facts;
  const auto active = [&](std::uint32_t flags = 0) {
    state = initial(true, flags);
    facts.current.filterEnabled = Fact<bool>::known(true);
    facts.current.overlayFlags = BitFacts::known(flags);
    facts.runtime.controlDevicePresent = Fact<bool>::known(true);
    facts.runtime.horm = Fact<bool>::known(false);
    facts.current.volumes[oracle::volume].present = Fact<bool>::known(true);
    facts.current.volumes[oracle::volume].protectedVolume = Fact<bool>::known(true);
    facts.current.volumes[oracle::volume].driveLetter = Fact<std::string>::known("C:");
    facts.current.volumes[oracle::volume].physicalDriveLetter = Fact<std::string>::known("C:");
  };
  switch (scene) {
    case Scene::FullOff:
      state = initial(false, 4);
      state.next.filterEnabled = true;
      request = SetBoolean{BooleanProperty::OverlayCommit, true};
      facts.current.filterEnabled = Fact<bool>::known(false);
      break;
    case Scene::FullDispositionConflict:
      active(4);
      state.runtime.romDisposition = 3;
      request = SetBoolean{BooleanProperty::OverlayCommit, true};
      facts.runtime.romDisposition = BitFacts::known(3);
      break;
    case Scene::FullCopy:
    case Scene::FullCopyFailure:
      active(4);
      state.runtime.fullCommitCopyFails = scene == Scene::FullCopyFailure;
      request = SetBoolean{BooleanProperty::OverlayCommit, true};
      facts.runtime.romDisposition = BitFacts::known(0);
      break;
    case Scene::SwapMissingOn:
      active();
      request = CreateSwapfile{oracle::otherVolume};
      facts.next.volumes[oracle::otherVolume].present = Fact<bool>::known(false);
      break;
    case Scene::ServicingCompleted:
      state = initial();
      state.runtime.servicingStatus = Fake::ServicingStatus::Completed;
      request = SetBoolean{BooleanProperty::ServicingEnabled, true};
      facts.runtime.servicingStatus = Fact<std::uint32_t>::known(2);
      break;
    case Scene::HormNoOp:
      state = initial(false, 4);
      state.runtime.hormEnabled = true;
      state.runtime.controlDevicePresent = false;
      request = SetBoolean{BooleanProperty::HormEnabled, true};
      facts.runtime.horm = Fact<bool>::known(true);
      facts.current.filterEnabled = Fact<bool>::known(false);
      facts.runtime.controlDevicePresent = Fact<bool>::known(false);
      break;
    case Scene::FileNoCopy:
      active();
      state.current.volumes.at(oracle::volume).fileExclusions = {excluded};
      state.runtime.defaultFileCommitView = Fake::PersistentView::NotReady;
      request = CommitFile{oracle::volume, excluded};
      facts.targets[oracle::volume].persistentFileViewReady = Fact<bool>::known(false);
      facts.current.volumes[oracle::volume].fileExclusions = Fact<std::vector<std::string>>::known({excluded});
      break;
    case Scene::DiagnosticCandidates:
      active();
      state.runtime.diagnosticCandidateReports[oracle::volume] = {{"\\clean", 8192}, {"\\invisible", 4096}};
      request = GetOverlayFiles{oracle::volume};
      break;
    case Scene::MissingDeletion:
      active();
      request = CommitFileDeletion{oracle::volume, path};
      facts.targets[oracle::volume].fileTarget = Fact<FileMaintenanceFacts>::known({11, FileMaintenanceKind::Missing, false});
      break;
    case Scene::DefaultRegistryCommit:
      active();
      request = CommitRegistry{oracle::key, std::string{}};
      facts.systemVolumeId = Fact<std::string>::known(oracle::volume);
      break;
    case Scene::InvalidDefaultDeletion:
      request = CommitRegistryDeletion{oracle::key, std::string{}};
      break;
    case Scene::FalseProtection:
      active();
      state.current.volumes.at(oracle::volume).volume.protectedVolume = false;
      request = GetVolumeProperty{volumePath(oracle::volume), VolumeProperty::Protected, Session::Current};
      facts.current.volumes[oracle::volume].protectedVolume = Fact<bool>::known(false);
      break;
    case Scene::NextFilter:
      state = initial(false);
      state.next.filterEnabled = true;
      request = GetBoolean{BooleanProperty::FilterEnabled, Session::Next};
      facts.current.filterEnabled = Fact<bool>::known(false);
      facts.next.filterEnabled = Fact<bool>::known(true);
      break;
    case Scene::FilterInvalidType:
      active(4);
      state.next.overlayType = 2;
      request = SetBoolean{BooleanProperty::FilterEnabled, false};
      facts.next.filterEnabled = Fact<bool>::known(true);
      facts.next.overlayType = Fact<std::uint32_t>::known(2);
      facts.administrator = Fact<bool>::known(true);
      break;
    case Scene::ServicingRestoreFailed:
      active();
      state.runtime.controlDevicePresent = false;
      state.runtime.servicingStatus = Fake::ServicingStatus::Started;
      state.runtime.servicingSavedFilterEnabled = true;
      state.next.servicingEnabled = true;
      state.next.filterEnabled = true;
      request = SetBoolean{BooleanProperty::ServicingEnabled, false};
      facts.runtime.controlDevicePresent = {};
      facts.runtime.servicingStatus = Fact<std::uint32_t>::known(1);
      facts.runtime.savedServicingFilter = Fact<std::optional<bool>>::known(true);
      facts.next.filterEnabled = Fact<bool>::known(true);
      facts.next.overlayType = Fact<std::uint32_t>::known(0);
      break;
    case Scene::ServicingIdleDisableWithoutControl:
      active();
      state.runtime.controlDevicePresent = false;
      state.runtime.servicingStatus = Fake::ServicingStatus::NotStarted;
      state.runtime.servicingSavedFilterEnabled = std::nullopt;
      state.next.servicingEnabled = true;
      state.next.filterEnabled = true;
      request = SetBoolean{BooleanProperty::ServicingEnabled, false};
      facts.administrator = Fact<bool>::known(true);
      facts.runtime.controlDevicePresent = Fact<bool>::known(false);
      facts.runtime.servicingStatus = Fact<std::uint32_t>::known(0);
      facts.runtime.savedServicingFilter = Fact<std::optional<bool>>::known(std::nullopt);
      facts.next.filterEnabled = Fact<bool>::known(true);
      facts.next.overlayType = Fact<std::uint32_t>::known(0);
      break;
    case Scene::ServicingStartedEnableWithoutControl:
      active();
      state.runtime.controlDevicePresent = false;
      state.runtime.servicingStatus = Fake::ServicingStatus::Started;
      state.runtime.servicingSavedFilterEnabled = true;
      state.next.servicingEnabled = false;
      state.next.filterEnabled = false;
      request = SetBoolean{BooleanProperty::ServicingEnabled, true};
      facts.administrator = Fact<bool>::known(true);
      facts.runtime.controlDevicePresent = Fact<bool>::known(false);
      facts.runtime.servicingStatus = Fact<std::uint32_t>::known(1);
      facts.runtime.savedServicingFilter = Fact<std::optional<bool>>::known(true);
      facts.next.filterEnabled = Fact<bool>::known(false);
      facts.next.overlayType = Fact<std::uint32_t>::known(0);
      break;
    case Scene::ServicingFailedDisableWithoutControl:
      active();
      state.runtime.controlDevicePresent = false;
      state.runtime.servicingStatus = Fake::ServicingStatus::Failed;
      state.runtime.servicingSavedFilterEnabled = false;
      state.next.servicingEnabled = true;
      state.next.filterEnabled = false;
      request = SetBoolean{BooleanProperty::ServicingEnabled, false};
      facts.administrator = Fact<bool>::known(true);
      facts.runtime.controlDevicePresent = Fact<bool>::known(false);
      facts.runtime.servicingStatus = Fact<std::uint32_t>::known(3);
      facts.runtime.savedServicingFilter = Fact<std::optional<bool>>::known(false);
      facts.next.filterEnabled = Fact<bool>::known(false);
      facts.next.overlayType = Fact<std::uint32_t>::known(0);
      break;
    case Scene::FilterSameInvalidType:
      active(4);
      state.next.overlayType = 2;
      state.runtime.romDisposition = 2;
      request = SetBoolean{BooleanProperty::FilterEnabled, true};
      facts.next.filterEnabled = Fact<bool>::known(true);
      facts.next.overlayType = Fact<std::uint32_t>::known(2);
      facts.runtime.romDisposition = BitFacts::known(2);
      facts.administrator = Fact<bool>::known(true);
      break;
  }
  facts.requestTarget = request;
  auto backend = std::make_shared<Fake>(state, "IndependentBackend");
  if (scene == Scene::FullCopy || scene == Scene::FullCopyFailure) {
    backend->seedFile(oracle::volume, path, "before");
    backend->writeFile(oracle::volume, path, "after");
  }
  if (scene == Scene::FileNoCopy) {
    backend->seedFile(oracle::volume, excluded, "before");
    backend->writeFile(oracle::volume, excluded, "after");
  }
  if (scene == Scene::DiagnosticCandidates) backend->seedFile(oracle::volume, "\\clean", "unchanged");
  if (scene == Scene::DefaultRegistryCommit) {
    backend->seedRegistryValue(oracle::key, "", "before");
    backend->writeRegistryValue(oracle::key, "", "after");
  }
  return {backend, std::move(request), std::move(facts)};
}
Value expectedValue(Scene scene) {
  if (scene == Scene::FalseProtection) return false;
  if (scene == Scene::NextFilter) return true;
  if (scene == Scene::DiagnosticCandidates) return std::vector<OverlayFile>{{"\\clean", 8192}, {"\\invisible", 4096}};
  return std::monostate{};
}
void compareSuccessfulEffects(Scene scene, const Fake& backend) {
  if (scene == Scene::FullCopy) {
    QCOMPARE(backend.snapshot().runtime.romDisposition, std::uint32_t{4});
    QCOMPARE(backend.readFile(oracle::volume, path), std::optional<std::string>{"after"});
  }
  if (scene == Scene::FullCopyFailure) {
    QCOMPARE(backend.snapshot().runtime.romDisposition, std::uint32_t{4});
    QCOMPARE(backend.readFile(oracle::volume, path), std::optional<std::string>{"before"});
  }
  if (scene == Scene::FileNoCopy) QCOMPARE(backend.readFile(oracle::volume, excluded), std::optional<std::string>{"after"});
  if (scene == Scene::DefaultRegistryCommit) QCOMPARE(backend.readRegistryValue(oracle::key, ""), std::optional<std::string>{"after"});
  if (scene == Scene::ServicingIdleDisableWithoutControl || scene == Scene::ServicingStartedEnableWithoutControl ||
      scene == Scene::ServicingFailedDisableWithoutControl) {
    const auto after = backend.snapshot();
    QVERIFY(after.current.filterEnabled);
    QCOMPARE(after.runtime.controlDevicePresent, std::optional<bool>{false});
    QCOMPARE(after.next.servicingEnabled, scene == Scene::ServicingStartedEnableWithoutControl);
    QCOMPARE(after.next.filterEnabled, scene == Scene::ServicingIdleDisableWithoutControl);
    if (scene == Scene::ServicingIdleDisableWithoutControl) {
      QCOMPARE(after.runtime.servicingStatus, Fake::ServicingStatus::NotStarted);
      QVERIFY(!after.runtime.servicingSavedFilterEnabled);
    } else if (scene == Scene::ServicingStartedEnableWithoutControl) {
      QCOMPARE(after.runtime.servicingStatus, Fake::ServicingStatus::Started);
      QCOMPARE(after.runtime.servicingSavedFilterEnabled, std::optional<bool>{true});
    } else {
      QCOMPARE(after.runtime.servicingStatus, Fake::ServicingStatus::Completed);
      QCOMPARE(after.runtime.servicingSavedFilterEnabled, std::optional<bool>{false});
    }
  }
}

class LiteralReadPort final : public Uwf {
 public:
  UwfStateFacts published;
  mutable int stateReads = 0;
  UwfStateSnapshot stateSnapshot() const override {
    ++stateReads;
    return std::make_shared<const UwfStateFacts>(published);
  }
  std::vector<std::pair<Request, Response>> replies;
  mutable std::vector<Request> calls;
  std::function<void(const Request&)> before;
  Capability probe(const Request&) const override { return {Support::Supported, {}}; }
  Response execute(const Request& request, const Context& = {}) const override {
    calls.push_back(request);
    if (before) before(request);
    const auto found = std::ranges::find_if(replies, [&](const auto& entry) { return entry.first == request; });
    return found == replies.end() ? Response::fail(ErrorKind::Unsupported, "No literal observation supplied") : found->second;
  }
};
}  // namespace
class UwfPrecheckDifferentialTests final : public QObject {
  Q_OBJECT
 private slots:
  void solverUsesOnlyThePublishedFileTarget_data() {
    QTest::addColumn<int>("scene");
    QTest::addColumn<bool>("reject");
    for (const auto& [name, scene, reject] :
         std::array{std::tuple{"missing", 0, true}, std::tuple{"reserved", 1, true}, std::tuple{"directory", 2, true}, std::tuple{"long-full-target", 3, true},
                    std::tuple{"nonempty-deletion", 4, true}, std::tuple{"ordinary-file", 5, false}, std::tuple{"unavailable-mapping", 6, false},
                    std::tuple{"malformed-kind", 7, true}, std::tuple{"unobserved-target", 8, false}})
      QTest::newRow(name) << scene << reject;
  }
  void solverUsesOnlyThePublishedFileTarget() {
    QFETCH(int, scene);
    QFETCH(bool, reject);
    auto source = std::make_shared<LiteralReadPort>();
    const std::string selectedPath = scene == 1 ? "\\pagefile.sys" : scene == 3 ? "\\" + std::string(257, 'a') : "\\selected";
    const Request request = scene == 4 ? Request{CommitFileDeletion{oracle::volume, selectedPath}} : Request{CommitFile{oracle::volume, selectedPath}};
    auto& published = source->published;
    published.current.filterEnabled = Fact<bool>::known(true);
    published.current.overlayFlags = BitFacts::known(0);
    published.runtime.horm = Fact<bool>::known(false);
    published.current.volumes[oracle::volume].present = Fact<bool>::known(true);
    published.current.volumes[oracle::volume].protectedVolume = Fact<bool>::known(true);
    published.current.volumes[oracle::volume].driveLetter = Fact<std::string>::known("C:");
    published.current.volumes[oracle::volume].physicalDriveLetter = Fact<std::string>::known("C:");
    published.requestTarget = request;
    if (scene != 6 && scene != 8)
      published.targets[oracle::volume].fileTarget = Fact<FileMaintenanceFacts>::known({scene == 3 ? std::size_t{260} : std::size_t{11},
                                                                                        scene == 0   ? FileMaintenanceKind::Missing
                                                                                        : scene == 2 ? FileMaintenanceKind::EmptyDirectory
                                                                                        : scene == 4 ? FileMaintenanceKind::NonemptyDirectory
                                                                                        : scene == 7 ? static_cast<FileMaintenanceKind>(99)
                                                                                                     : FileMaintenanceKind::File,
                                                                                        scene == 1});
    source->replies = {{request, Response::success(std::monostate{}, Effect::Confirmed)}};
    const app::PrecheckedUwf checked(source);
    const auto response = checked.execute(request);
    QCOMPARE(response.succeeded(), !reject);
    QCOMPARE(source->calls.size(), reject ? std::size_t{0} : std::size_t{1});
    if (reject) {
      QCOMPARE(response.effect, Effect::None);
      QVERIFY(response.failure);
      if (scene == 7) {
        QCOMPARE(response.failure->kind, ErrorKind::Protocol);
      } else {
        QVERIFY(!response.failure->diagnostics.empty());
        QCOMPARE(response.failure->diagnostics.front().backend, std::string{"UwfFailureSolver"});
      }
    } else {
      QCOMPARE(source->calls.front(), request);
    }
  }
  void sharedStateAccessDoesNotExecuteProvider() {
    auto source = std::make_shared<LiteralReadPort>();
    source->published.current.filterEnabled = Fact<bool>::known(false);
    source->published.runtime.horm = Fact<bool>::known(true);
    source->published.runtime.warningThreshold = Fact<std::uint32_t>::known(777);
    const app::PrecheckedUwf checked(source);
    source->stateReads = 0;
    source->published.runtime.warningThreshold = Fact<std::uint32_t>::known(888);
    const auto published = checked.state();
    QCOMPARE(published.current.filterEnabled.value, std::optional{false});
    QCOMPARE(published.runtime.horm.value, std::optional{true});
    QCOMPARE(published.runtime.warningThreshold.value, std::optional<std::uint32_t>{777});
    QCOMPARE(checked.stateSnapshot()->runtime.warningThreshold.value, std::optional<std::uint32_t>{777});
    QCOMPARE(source->stateReads, 0);
    QVERIFY(source->calls.empty());
  }
  void optionalSolverUsesIndependentContractCases_data() {
    QTest::addColumn<int>("index");
    for (std::size_t i = 0; i < cases.size(); ++i) QTest::newRow(cases[i].id) << static_cast<int>(i);
  }
  void optionalSolverUsesIndependentContractCases() {
    QFETCH(int, index);
    const auto vector = cases.at(static_cast<std::size_t>(index));
    auto bare = fixture(vector.scene);
    auto gated = fixture(vector.scene);
    const auto beforeGated = gated.backend->snapshot();
    app::PrecheckedUwf withoutInference(bare.backend->rawBackend(), {}, false);
    withoutInference.updateState(bare.facts);
    app::PrecheckedUwf system(gated.backend->rawBackend());
    system.updateState(gated.facts);
    const auto bareResult = withoutInference.execute(bare.request);
    const auto gatedResult = system.execute(gated.request);
    const bool refreshFails = vector.scene == Scene::ServicingRestoreFailed || vector.scene == Scene::ServicingIdleDisableWithoutControl ||
                              vector.scene == Scene::ServicingStartedEnableWithoutControl || vector.scene == Scene::ServicingFailedDisableWithoutControl;
    if (refreshFails) {
      const auto verifyAcknowledgedWrite = [&](const Fixture& fixture, const app::PrecheckedUwf& facade, const Response& response) {
        // These captured setter receipts are successful even though Current is
        // active without its control device. Observing that unhealthy runtime
        // must fail the public refresh without changing the native receipt.
        const auto trace = fixture.backend->trace();
        const auto write = std::ranges::find_if(trace, [&](const auto& entry) { return entry.request == fixture.request; });
        QVERIFY(write != trace.end());
        QCOMPARE(write->response.succeeded(), vector.backendSuccess);
        QCOMPARE(write->response.effect, Effect::Confirmed);
        QCOMPARE(std::ranges::count(trace, fixture.request, &Fake::TraceEntry::request), std::ptrdiff_t{1});
        const Request fullCommit = GetBoolean{BooleanProperty::OverlayCommit};
        const auto observation = std::ranges::find(trace, fullCommit, &Fake::TraceEntry::request);
        QVERIFY(observation != trace.end());
        QVERIFY(observation->response.failure);
        QCOMPARE(observation->response.failure->kind, ErrorKind::Failed);
        QCOMPARE(observation->response.failure->message, std::string{"The UWF volume control device is unavailable"});
        QCOMPARE(observation->response.failure->diagnostics.size(), std::size_t{1});
        QCOMPARE(observation->response.failure->diagnostics.front().code, static_cast<std::int32_t>(0x8000ffffu));
        QCOMPARE(std::ranges::count(trace, fullCommit, &Fake::TraceEntry::request), std::ptrdiff_t{1});
        QVERIFY(response.failure);
        QCOMPARE(response.failure->kind, observation->response.failure->kind);
        QCOMPARE(response.failure->diagnostics.size(), std::size_t{1});
        QCOMPARE(response.failure->diagnostics.front().code, observation->response.failure->diagnostics.front().code);
        QCOMPARE(response.failure->diagnostics.front().detail, observation->response.failure->diagnostics.front().detail);
        QCOMPARE(response.effect, Effect::Confirmed);
        QVERIFY(response.failure->message.find("write completed") != std::string::npos);
        const auto published = facade.state();
        QVERIFY(published.refreshFailure);
        QCOMPARE(published.refreshFailure->kind, observation->response.failure->kind);
        QCOMPARE(published.refreshFailure->message, observation->response.failure->message);
      };
      verifyAcknowledgedWrite(bare, withoutInference, bareResult);
      verifyAcknowledgedWrite(gated, system, gatedResult);
    }
    QCOMPARE(bareResult.succeeded(), vector.backendSuccess && !refreshFails);
    if (vector.solverReject) {
      QVERIFY(gatedResult.failure);
      QCOMPARE(gatedResult.effect, Effect::None);
      QVERIFY(gated.backend->trace().empty());
      QVERIFY(gated.backend->primitiveRequests().empty());
      QCOMPARE(gated.backend->snapshot(), beforeGated);
      QVERIFY(bareResult.failure);
    } else {
      QCOMPARE(gatedResult.succeeded(), vector.backendSuccess && !refreshFails);
      QCOMPARE(gatedResult.effect, bareResult.effect);
      QCOMPARE(gatedResult.failure.has_value(), bareResult.failure.has_value());
      if (gatedResult.failure) {
        QCOMPARE(gatedResult.failure->kind, bareResult.failure->kind);
        QCOMPARE(gatedResult.failure->effect, bareResult.failure->effect);
        QCOMPARE(gatedResult.failure->message, bareResult.failure->message);
        QCOMPARE(gatedResult.failure->diagnostics.size(), bareResult.failure->diagnostics.size());
        for (std::size_t diagnostic = 0; diagnostic < gatedResult.failure->diagnostics.size(); ++diagnostic) {
          const auto& actual = gatedResult.failure->diagnostics[diagnostic];
          const auto& expected = bareResult.failure->diagnostics[diagnostic];
          QCOMPARE(actual.backend, expected.backend);
          QCOMPARE(actual.operation, expected.operation);
          QCOMPARE(actual.code, expected.code);
          QCOMPARE(actual.detail, expected.detail);
        }
      }
      if (vector.backendSuccess) {
        QVERIFY(gatedResult.value == expectedValue(vector.scene));
        QVERIFY(bareResult.value == expectedValue(vector.scene));
        compareSuccessfulEffects(vector.scene, *bare.backend);
        compareSuccessfulEffects(vector.scene, *gated.backend);
      }
      QCOMPARE(gated.backend->snapshot(), bare.backend->snapshot());
    }
    if (isWrite(gated.request)) {
      const auto verifyTrace = [&](const Fake& backend, const Response& response) {
        const auto primitives = backend.primitiveRequests();
        std::vector<Request> writes;
        std::vector<Request> reads;
        bool refreshing = false;
        for (const auto& request : primitives) {
          if (isWrite(request)) {
            QVERIFY2(!refreshing, "The original write must not be replayed during or after state refresh");
            writes.push_back(request);
          } else {
            refreshing = true;
            reads.push_back(request);
          }
        }
        QCOMPARE(writes, response.effect == Effect::None ? std::vector<Request>{} : std::vector<Request>{gated.request});
        const auto* boolean = std::get_if<SetBoolean>(&gated.request);
        const bool observedWorkflow =
            boolean && (boolean->property == BooleanProperty::ServicingEnabled || boolean->property == BooleanProperty::OverlayCommit);
        const bool dataCommit = std::holds_alternative<CommitFile>(gated.request) || std::holds_alternative<CommitFileDeletion>(gated.request) ||
                                std::holds_alternative<CommitRegistry>(gated.request) || std::holds_alternative<CommitRegistryDeletion>(gated.request);
        const bool needsRefresh =
            (observedWorkflow && response.succeeded() && !writes.empty()) || (!dataCommit && response.failure && response.effect != Effect::None);
        if (!needsRefresh) {
          QVERIFY2(reads.empty(), "Ordinary successful writes and side-effect-free rejections must not query other capabilities");
        } else {
          // One explicit global refresh follows the original receipt. Repeated
          // checkpoints would reveal another refresh or per-field confirmation.
          for (const auto& checkpoint : std::vector<Request>{
                   GetBoolean{BooleanProperty::FilterEnabled, Session::Current}, GetBoolean{BooleanProperty::FilterEnabled, Session::Next},
                   GetBoolean{BooleanProperty::ShutdownPending}, GetNumber{NumberProperty::WarningThreshold}, GetNumber{NumberProperty::CriticalThreshold}})
            QCOMPARE(std::ranges::count(reads, checkpoint), std::ptrdiff_t{1});
          for (const auto& read : reads) QCOMPARE(std::ranges::count(reads, read), std::ptrdiff_t{1});
        }
      };
      verifyTrace(*gated.backend, gatedResult);
      verifyTrace(*bare.backend, bareResult);
    }
    if (vector.scene == Scene::ServicingRestoreFailed) {
      QCOMPARE(bareResult.effect, Effect::Confirmed);
      QCOMPARE(gatedResult.effect, Effect::Confirmed);
      QCOMPARE(gated.backend->snapshot().runtime.servicingStatus, Fake::ServicingStatus::Failed);
    }
    if (vector.scene == Scene::FilterSameInvalidType) {
      QCOMPARE(bareResult.effect, Effect::None);
      QCOMPARE(gated.backend->snapshot(), beforeGated);
    }
  }
  void unavailableFactsNeverTriggerAProviderLookup() {
    auto source = std::make_shared<LiteralReadPort>();
    const Request request = GetBoolean{BooleanProperty::FilterEnabled};
    source->replies = {{request, Response::success(false)}};
    const app::PrecheckedUwf checked(source);
    const auto response = checked.execute(request);
    QVERIFY(response.succeeded());
    QCOMPARE(std::get<bool>(response.value), false);
    QCOMPARE(source->calls, std::vector<Request>{request});
  }
  void cancellationNeverSubmitsTheTarget() {
    auto source = std::make_shared<LiteralReadPort>();
    const app::PrecheckedUwf checked(source);
    std::stop_source stop;
    stop.request_stop();
    const auto result = checked.execute(GetBoolean{BooleanProperty::FilterEnabled}, {stop.get_token()});
    QVERIFY(result.failure);
    QCOMPARE(result.failure->kind, ErrorKind::Canceled);
    QCOMPARE(result.effect, Effect::None);
    QVERIFY(source->calls.empty());
  }
  void contextFactsOverrideThePublishedSnapshotWithoutRefreshing() {
    auto source = std::make_shared<LiteralReadPort>();
    const Request request = CommitFile{oracle::volume, path};
    source->published.current.filterEnabled = Fact<bool>::known(false);
    source->published.requestTarget = request;
    source->published.targets[oracle::volume].fileTarget = Fact<FileMaintenanceFacts>::known({11, FileMaintenanceKind::Missing, false});
    source->replies = {{request, Response::success()}};
    auto prepared = source->published;
    prepared.targets.at(oracle::volume).fileTarget.value->kind = FileMaintenanceKind::File;
    int factsReads = 0;
    int refreshes = 0;
    const app::PrecheckedUwf checked(
        source,
        [&](const Request&, const Context&) {
          ++factsReads;
          return source->published;
        },
        true, [&](const Uwf&, const Context&) {
          ++refreshes;
          return Response::success();
        });
    source->stateReads = 0;

    const auto response = checked.execute(request, {{}, &prepared});
    QVERIFY(response.succeeded());
    QCOMPARE(source->calls, std::vector<Request>{request});
    QCOMPARE(factsReads, 0);
    QCOMPARE(refreshes, 0);
    QCOMPARE(source->stateReads, 0);
    QCOMPARE(checked.state().targets.at(oracle::volume).fileTarget.value->kind, FileMaintenanceKind::Missing);

    const auto withoutOverride = checked.execute(request);
    QVERIFY(withoutOverride.failure);
    QCOMPARE(withoutOverride.failure->kind, ErrorKind::NotFound);
    QCOMPARE(withoutOverride.effect, Effect::None);
    QCOMPARE(source->calls, std::vector<Request>{request});
    QCOMPARE(factsReads, 1);
    QCOMPARE(refreshes, 0);
    QCOMPARE(source->stateReads, 0);
  }
  void probeDoesNotObserveOrSolve() {
    auto backend = std::make_shared<Fake>(initial());
    const Request request = GetBoolean{BooleanProperty::FilterEnabled};
    backend->setCapability(request, {Support::Unavailable, "literal fixture capability"});
    int observations = 0;
    app::PrecheckedUwf system(backend, [&](const Request&, const Context&) {
      ++observations;
      return UwfStateFacts{};
    });
    const auto capability = system.probe(request);
    QCOMPARE(capability.support, Support::Unavailable);
    QCOMPARE(observations, 0);
    QVERIFY(backend->trace().empty());
  }
};
QTEST_APPLESS_MAIN(UwfPrecheckDifferentialTests)
#include "UwfPrecheckDifferentialTests.moc"
