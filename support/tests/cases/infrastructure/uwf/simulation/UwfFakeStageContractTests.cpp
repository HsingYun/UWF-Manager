// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QTest>
#include <algorithm>
#include <array>
#include <cstdint>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "infrastructure/uwf/routing/FallbackUwf.h"
#include "simulation/uwf/FakeUwfBackend.h"
#include "support/uwf/CapturedUwfContractVectors.h"

namespace {
using namespace uwf;
using Fake = uwf::simulation::FakeUwfBackend;
namespace captured = test::captured;
constexpr auto volumeA = "11111111-2222-3333-4444-555555555555";
constexpr auto volumeB = "66666666-7777-8888-9999-000000000000";
constexpr auto path = "\\contract.bin";
constexpr auto sibling = "\\unrelated.bin";
constexpr auto key = "HKEY_LOCAL_MACHINE\\SOFTWARE\\CapturedContract";

Fake::State initialState(bool rom = false) {
  Fake::State state;
  state.current.filterEnabled = true;
  state.current.overlayType = 0;
  state.current.overlayFlags = rom ? 4 : 0;
  state.current.volumes.emplace(volumeA, Fake::VolumeConfiguration{.volume = {volumeA, "C:", true, false, false}, .fileExclusions = {}, .swapfileSize = 0});
  state.current.volumes.emplace(volumeB, Fake::VolumeConfiguration{.volume = {volumeB, "D:", true, false, false}, .fileExclusions = {}, .swapfileSize = 0});
  state.next = state.current;
  state.runtime.systemVolumeId = volumeA;
  state.runtime.fileMaintenancePrefixes = {{volumeA, "C:"}, {volumeB, "D:"}};
  return state;
}
Fake::FaultTiming copyStage(captured::CopyCheckpoint point) {
  switch (point) {
    case captured::CopyCheckpoint::Eof:
      return Fake::FaultTiming::AfterDestinationEof;
    case captured::CopyCheckpoint::FirstDataChunk:
      return Fake::FaultTiming::AfterFileDataChunk;
    case captured::CopyCheckpoint::Data:
      return Fake::FaultTiming::AfterFileData;
    case captured::CopyCheckpoint::Ea:
      return Fake::FaultTiming::AfterFileEa;
    case captured::CopyCheckpoint::Ads:
      return Fake::FaultTiming::AfterFileAds;
    case captured::CopyCheckpoint::Time:
      return Fake::FaultTiming::AfterFileTime;
    case captured::CopyCheckpoint::Security:
      return Fake::FaultTiming::AfterFileSecurity;
  }
  throw std::logic_error("Unknown captured copy checkpoint");
}
void compareReceipt(const compat::Response& actual, const captured::ExpectedReceipt& expected) {
  QCOMPARE(actual.effect, expected.effect);
  QCOMPARE(actual.failure.has_value(), expected.failure.has_value());
  if (expected.failure && actual.failure) QCOMPARE(actual.failure->kind, *expected.failure);
  if (expected.value) QVERIFY(actual.value == *expected.value);
}
struct ManagedHormCase {
  const char* id;
  bool currentFilter;
  std::uint32_t type, flags;
  bool hiberfile;
  Fake::SafetyObservation physical;
  std::uint32_t disposition;
  Fake::SafetyObservation finalSafety;
  bool controlKnown, control;
  captured::ExpectedReceipt expected;
  std::uint32_t afterDisposition;
  captured::ExpectedReceipt portableExpected;
};
std::vector<ManagedHormCase> managedHormCases() {
  using S = Fake::SafetyObservation;
  using E = compat::ErrorKind;
  using F = compat::Effect;
  // Captured managed-input partitions: uwfcfgmgmt 18a20, C22265-22361.
  // Final safety is an execution scenario; expected receipts are literal and independent of production policy.
  return {
      {"ready", true, 0, 4, true, S::Satisfied, 0, S::Satisfied, true, true, {{}, F::Confirmed, {}}, 2, {{}, F::Confirmed, {}}},
      {"no-hiberfile", true, 0, 4, false, S::Satisfied, 0, S::Satisfied, true, true, {E::NotFound, F::None, {}}, 0, {E::Conflict, F::Uncertain, {}}},
      {"no-rom", true, 0, 0, true, S::Satisfied, 0, S::Satisfied, true, true, {E::Conflict, F::None, {}}, 0, {E::Conflict, F::Uncertain, {}}},
      {"rom-with-passthrough", true, 0, 5, true, S::Satisfied, 0, S::Satisfied, true, true, {E::Conflict, F::None, {}}, 0, {E::Conflict, F::Uncertain, {}}},
      {"rom-with-opaque-bit", true, 0, 0x84, true, S::Satisfied, 0, S::Satisfied, true, true, {E::Conflict, F::None, {}}, 0, {E::Conflict, F::Uncertain, {}}},
      {"filter-off", false, 0, 4, true, S::Satisfied, 0, S::Satisfied, true, true, {E::Conflict, F::None, {}}, 0, {E::Conflict, F::Uncertain, {}}},
      {"disk", true, 1, 4, true, S::Satisfied, 0, S::Satisfied, true, true, {E::Conflict, F::None, {}}, 0, {E::Conflict, F::Uncertain, {}}},
      {"invalid-type", true, 2, 4, true, S::Satisfied, 0, S::Satisfied, true, true, {E::Conflict, F::None, {}}, 0, {E::Conflict, F::Uncertain, {}}},
      {"physical-rejected", true, 0, 4, true, S::Rejected, 0, S::Satisfied, true, true, {E::Conflict, F::None, {}}, 0, {E::Conflict, F::Uncertain, {}}},
      {"physical-unknown", true, 0, 4, true, S::Unknown, 0, S::Satisfied, true, true, {E::Unavailable, F::None, {}}, 0, {E::Unavailable, F::Uncertain, {}}},
      {"disposition-filter", true, 0, 4, true, S::Satisfied, 1, S::Satisfied, true, true, {E::Conflict, F::None, {}}, 1, {E::Conflict, F::Uncertain, {}}},
      {"disposition-prepared", true, 0, 4, true, S::Satisfied, 2, S::Satisfied, true, true, {E::Conflict, F::None, {}}, 2, {E::Conflict, F::Uncertain, {}}},
      {"disposition-committed", true, 0, 4, true, S::Satisfied, 4, S::Satisfied, true, true, {E::Conflict, F::None, {}}, 4, {E::Conflict, F::Uncertain, {}}},
      {"final-rejected", true, 0, 4, true, S::Satisfied, 0, S::Rejected, true, true, {E::Conflict, F::Uncertain, {}}, 2, {E::Conflict, F::Uncertain, {}}},
      {"final-unknown", true, 0, 4, true, S::Satisfied, 0, S::Unknown, true, true, {E::Unavailable, F::Uncertain, {}}, 2, {E::Unavailable, F::Uncertain, {}}},
      {"control-absent", true, 0, 4, true, S::Satisfied, 0, S::Satisfied, true, false, {E::Unavailable, F::None, {}}, 0, {E::Conflict, F::Uncertain, {}}},
      {"control-unknown", true, 0, 4, true, S::Satisfied, 0, S::Satisfied, false, true, {E::Unavailable, F::None, {}}, 0, {E::Unavailable, F::Uncertain, {}}},
      {"control-before-hiber",
       true,
       0,
       4,
       false,
       S::Satisfied,
       0,
       S::Satisfied,
       true,
       false,
       {E::Unavailable, F::None, {}},
       0,
       {E::Conflict, F::Uncertain, {}}},
      {"hiber-before-flags", true, 0, 0, false, S::Satisfied, 0, S::Satisfied, true, true, {E::NotFound, F::None, {}}, 0, {E::Conflict, F::Uncertain, {}}},
      {"flags-before-physical", true, 0, 0, true, S::Unknown, 0, S::Satisfied, true, true, {E::Conflict, F::None, {}}, 0, {E::Conflict, F::Uncertain, {}}},
      {"physical-before-disposition",
       true,
       0,
       4,
       true,
       S::Unknown,
       4,
       S::Satisfied,
       true,
       true,
       {E::Unavailable, F::None, {}},
       4,
       {E::Unavailable, F::Uncertain, {}}},
      {"filter-before-physical", false, 0, 4, true, S::Unknown, 0, S::Satisfied, true, true, {E::Conflict, F::None, {}}, 0, {E::Conflict, F::Uncertain, {}}},
      {"type-before-physical", true, 1, 4, true, S::Unknown, 0, S::Satisfied, true, true, {E::Conflict, F::None, {}}, 0, {E::Conflict, F::Uncertain, {}}}};
}
}  // namespace

class UwfFakeStageContractTests final : public QObject {
  Q_OBJECT
 private slots:
  void rawFilterValidatesNextTypeAfterItsRuntimeRequest_data() {
    QTest::addColumn<int>("row");
    for (int row = 0; row < 8; ++row) QTest::newRow(qPrintable(QStringLiteral("filter-runtime-before-type/%1").arg(row))) << row;
  }
  void rawFilterValidatesNextTypeAfterItsRuntimeRequest() {
    QFETCH(int, row);
    const bool current = row / 4 != 0;
    const bool rom = (row / 2) % 2 != 0;
    const auto invalidType = row % 2 == 0 ? std::uint32_t{2} : std::uint32_t{0xffffffffu};
    auto state = initialState(rom);
    state.current.filterEnabled = current;
    state.next.filterEnabled = true;
    state.next.overlayType = invalidType;
    Fake backend(state);
    const auto before = backend.snapshot();
    const compat::Request request = compat::SetBoolean{compat::BooleanProperty::FilterEnabled, false};
    const auto result = backend.execute(request);
    QVERIFY(result.failure);
    QCOMPARE(result.failure->kind, compat::ErrorKind::Failed);
    QCOMPARE(result.effect, compat::Effect::Uncertain);
    Fake capturedBackend(state);
    const auto capturedReceipt = capturedBackend.executeCapturedPrimitiveForTest(request);
    compareReceipt(capturedReceipt, {compat::ErrorKind::Protocol, current ? compat::Effect::Uncertain : compat::Effect::None, {}});
    QCOMPARE(capturedBackend.snapshot(), backend.snapshot());
    QCOMPARE(backend.snapshot().runtime.romDisposition, current && rom ? std::uint32_t{1} : std::uint32_t{0});
    QCOMPARE(backend.snapshot().next, before.next);
    const auto primitives = backend.primitiveRequests();
    QCOMPARE(std::ranges::count(primitives, request), std::ptrdiff_t{1});
    Fake matching(state);
    const auto unchanged = matching.snapshot();
    const compat::Request same = compat::SetBoolean{compat::BooleanProperty::FilterEnabled, true};
    compareReceipt(matching.execute(same), {{}, compat::Effect::None, {}});
    QCOMPARE(matching.snapshot(), unchanged);
    const auto noOpCalls = matching.primitiveRequests();
    QCOMPARE(std::ranges::count(noOpCalls, same), std::ptrdiff_t{0});
  }
  void servicingNoFilterPathRetainsMissingDeviceFallback_data() {
    QTest::addColumn<int>("row");
    for (int row = 0; row < 6; ++row) QTest::newRow(qPrintable(QStringLiteral("servicing-no-filter-path/%1").arg(row))) << row;
  }
  void filterControlFailureKeepsCapturedAndPublicReceiptSeparate_data() {
    QTest::addColumn<bool>("rom");
    QTest::newRow("ordinary") << false;
    QTest::newRow("rom") << true;
  }
  void filterControlFailureKeepsCapturedAndPublicReceiptSeparate() {
    QFETCH(bool, rom);
    auto state = initialState(rom);
    state.runtime.controlDevicePresent = false;
    const compat::Request request = compat::SetBoolean{compat::BooleanProperty::FilterEnabled, false};
    Fake publicBackend(state);
    Fake capturedBackend(state);
    const auto receipt = publicBackend.execute(request);
    const auto capturedReceipt = capturedBackend.executeCapturedPrimitiveForTest(request);
    compareReceipt(receipt, {compat::ErrorKind::Failed, compat::Effect::Uncertain, {}});
    compareReceipt(capturedReceipt, {compat::ErrorKind::Unavailable, compat::Effect::None, {}});
    QCOMPARE(receipt.failure->diagnostics.back().code, std::int64_t{static_cast<std::int32_t>(0x8000ffffu)});
    QCOMPARE(capturedReceipt.failure->diagnostics.back().code, std::int64_t{static_cast<std::int32_t>(0x8000ffffu)});
    QCOMPARE(publicBackend.snapshot(), state);
    QCOMPARE(capturedBackend.snapshot(), state);
  }
  void servicingNoFilterPathRetainsMissingDeviceFallback() {
    QFETCH(int, row);
    const auto branch = row / 2;
    const bool invalidType = row % 2 != 0;
    const std::array<Fake::ServicingStatus, 3> before{Fake::ServicingStatus::NotStarted, Fake::ServicingStatus::Started, Fake::ServicingStatus::Failed};
    constexpr std::array<bool, 3> enable{false, true, false};
    constexpr std::array<bool, 3> nextFilter{true, false, false};
    const std::array<std::optional<bool>, 3> saved{std::nullopt, true, false};
    auto state = initialState();
    state.runtime.controlDevicePresent = false;
    state.runtime.servicingStatus = before[static_cast<std::size_t>(branch)];
    state.runtime.servicingSavedFilterEnabled = saved[static_cast<std::size_t>(branch)];
    state.next.filterEnabled = nextFilter[static_cast<std::size_t>(branch)];
    state.next.overlayType = invalidType ? 2 : 0;
    Fake backend(state);
    const auto unchanged = backend.snapshot();
    const compat::Request request = compat::SetBoolean{compat::BooleanProperty::ServicingEnabled, enable[static_cast<std::size_t>(branch)]};
    const auto result = backend.execute(request);
    if (invalidType) {
      compareReceipt(result, {compat::ErrorKind::Failed, compat::Effect::Uncertain, {}});
      QCOMPARE(backend.snapshot(), unchanged);
    } else {
      QVERIFY(result.succeeded());
      QCOMPARE(result.effect, compat::Effect::Confirmed);
      QCOMPARE(backend.snapshot().next.servicingEnabled, enable[static_cast<std::size_t>(branch)]);
    }
    const auto primitives = backend.primitiveRequests();
    QCOMPARE(std::ranges::count(primitives, request), std::ptrdiff_t{1});
  }

  void defaultUsageDoesNotMeasureNextOrUnprotectedVolumes_data() {
    QTest::addColumn<int>("row");
    for (int row = 0; row < 24; ++row) QTest::newRow(qPrintable(QStringLiteral("usage-current-next/%1").arg(row))) << row;
  }
  void defaultUsageDoesNotMeasureNextOrUnprotectedVolumes() {
    QFETCH(int, row);
    auto coordinate = static_cast<std::size_t>(row);
    const bool overrideProfile = coordinate % 2 != 0;
    coordinate /= 2;
    const bool nextProtected = coordinate % 2 != 0;
    coordinate /= 2;
    const auto currentProtection = coordinate % 3;
    coordinate /= 3;
    const bool currentEnabled = coordinate != 0;
    auto state = initialState();
    state.current.filterEnabled = currentEnabled;
    state.current.volumes.erase(volumeB);
    if (currentProtection == 0)
      state.current.volumes.erase(volumeA);
    else
      state.current.volumes.at(volumeA).volume.protectedVolume = currentProtection == 2;
    state.next.volumes.at(volumeA).volume.protectedVolume = nextProtected;
    state.next.volumes.erase(volumeB);
    if (overrideProfile) state.runtime.usageOverride = compat::OverlayUsage{7, 11};
    Fake backend(state);
    const auto before = backend.snapshot();
    const auto actual = compat::call(backend, compat::GetOverlayUsage{});
    const compat::OverlayUsage expected = !currentEnabled          ? compat::OverlayUsage{}
                                          : overrideProfile        ? compat::OverlayUsage{7, 11}
                                          : currentProtection == 2 ? compat::OverlayUsage{0, 1024}
                                                                   : compat::OverlayUsage{};
    QCOMPARE(actual, expected);
    QCOMPARE(backend.snapshot(), before);
  }

  void diagnosticCandidatesAreIndependentFromFileContent() {
    auto state = initialState();
    state.runtime.diagnosticCandidateReports[volumeA] = {{"\\clean-metadata-only.bin", 8192}, {"\\no-visible-file.bin", 4096}};
    Fake backend(state);
    backend.seedFile(volumeA, "\\clean-metadata-only.bin", "three");
    const auto before = backend.snapshot();
    const auto files = compat::call(backend, compat::GetOverlayFiles{volumeA});
    const std::vector<compat::OverlayFile> expected{{"\\clean-metadata-only.bin", 8192}, {"\\no-visible-file.bin", 4096}};
    QCOMPARE(files, expected);
    QVERIFY(backend.snapshot().overlay.files.empty());
    QCOMPARE(backend.readFile(volumeA, "\\clean-metadata-only.bin"), std::optional<std::string>{"three"});
    QVERIFY(!backend.visibleFile(volumeA, "\\no-visible-file.bin"));
    QCOMPARE(backend.snapshot(), before);
    const auto missing = backend.execute(compat::CommitFile{volumeA, "\\no-visible-file.bin"});
    compareReceipt(missing, {compat::ErrorKind::NotFound, compat::Effect::None, {}});
    state.runtime.diagnosticCandidateReports[volumeA].clear();
    backend.seed(state);
    backend.writeFile(volumeA, path, "unreported pending content");
    QVERIFY(!backend.snapshot().overlay.files.empty());
    QVERIFY(compat::call(backend, compat::GetOverlayFiles{volumeA}).empty());
  }
  void derivedDiagnosticBytesUseExplicitClusterCoverage_data() {
    QTest::addColumn<int>("length");
    QTest::addColumn<quint64>("expectedBytes");
    QTest::newRow("empty") << 0 << quint64{0};
    QTest::newRow("one-byte") << 1 << quint64{4096};
    QTest::newRow("one-cluster") << 4096 << quint64{4096};
    QTest::newRow("two-clusters") << 4097 << quint64{8192};
  }
  void derivedDiagnosticBytesUseExplicitClusterCoverage() {
    QFETCH(int, length);
    QFETCH(quint64, expectedBytes);
    auto state = initialState();
    state.runtime.diagnosticClusterSizeBytes = 4096;
    Fake backend(state);
    backend.seedFile(volumeA, path, std::string(static_cast<std::size_t>(length), 'x'), Fake::Storage::Overlay);
    const auto before = backend.snapshot();
    const auto report = compat::call(backend, compat::GetOverlayFiles{volumeA});
    QCOMPARE(report, (std::vector<compat::OverlayFile>{{path, expectedBytes}}));
    QCOMPARE(backend.snapshot(), before);
  }
  void unknownDiagnosticGeometryCannotBecomeContentBytes() {
    auto state = initialState();
    state.runtime.diagnosticClusterSizeBytes.reset();
    Fake backend(state);
    backend.writeFile(volumeA, path, "changed");
    compareReceipt(backend.execute(compat::GetOverlayFiles{volumeA}), {compat::ErrorKind::Unavailable, compat::Effect::None, {}});
    state.runtime.diagnosticClusterSizeBytes = 0;
    backend.seed(state);
    compareReceipt(backend.execute(compat::GetOverlayFiles{volumeA}), {compat::ErrorKind::Protocol, compat::Effect::None, {}});
  }
  void fileExclusionPagingContextIsIndependentFromTargetName_data() {
    QTest::addColumn<int>("row");
    for (int row = 0; row < 12; ++row) QTest::newRow(qPrintable(QStringLiteral("paging-context/%1").arg(row))) << row;
  }
  void fileExclusionPagingContextIsIndependentFromTargetName() {
    QFETCH(int, row);
    const auto observation = static_cast<std::size_t>(row) / 4;
    const auto spelling = static_cast<std::size_t>(row) % 4;
    constexpr std::array<const char*, 4> targets{"\\pagefile.sys", "\\Windows", "\\Windows\\System32", "\\ordinary"};
    constexpr std::array<Fake::SafetyObservation, 3> contexts{Fake::SafetyObservation::Unknown, Fake::SafetyObservation::Rejected,
                                                              Fake::SafetyObservation::Satisfied};
    auto state = initialState();
    state.current.filterEnabled = false;
    state.runtime.windowsDirectory = "\\Windows";
    state.runtime.pagingFilesValidation = contexts[observation];
    Fake backend(state);
    const auto before = backend.snapshot();
    const auto response = backend.execute(compat::EditFileExclusion{volumeA, targets[spelling], compat::ExclusionAction::Add});
    if (observation == 2) {
      QVERIFY(response.succeeded());
      QCOMPARE(response.effect, compat::Effect::Confirmed);
      QCOMPARE(backend.snapshot().next.volumes.at(volumeA).fileExclusions, std::vector<std::string>{targets[spelling]});
    } else {
      compareReceipt(response, {observation == 0 ? compat::ErrorKind::Unavailable : compat::ErrorKind::InvalidArgument, compat::Effect::Uncertain, {}});
      QCOMPARE(backend.snapshot(), before);
    }
  }
  void unavailableControlObservationNeverBecomesFalseOrZero_data() {
    QTest::addColumn<bool>("rom");
    QTest::newRow("ordinary-mode-rejects-inside-primitive") << false;
    QTest::newRow("rom-mode-rejects-at-disposition-read") << true;
  }
  void unavailableControlObservationNeverBecomesFalseOrZero() {
    QFETCH(bool, rom);
    auto state = initialState(rom);
    state.runtime.controlDeviceObservationAvailable = false;
    Fake backend(state);
    const auto before = backend.snapshot();
    for (const compat::Request& request : {compat::Request{compat::GetBoolean{compat::BooleanProperty::ShutdownPending}},
                                           compat::Request{compat::GetBoolean{compat::BooleanProperty::OverlayCommit}},
                                           compat::Request{compat::GetNumber{compat::NumberProperty::ResetPersistentState}}})
      compareReceipt(backend.executeCapturedPrimitiveForTest(request), {compat::ErrorKind::Unavailable, compat::Effect::None, {}});
    const compat::Request fullCommit = compat::SetBoolean{compat::BooleanProperty::OverlayCommit, true};
    compareReceipt(backend.execute(fullCommit), {compat::ErrorKind::Unavailable, rom ? compat::Effect::None : compat::Effect::Uncertain, {}});
    QCOMPARE(backend.snapshot(), before);
    QVERIFY(backend.stages().empty());
    const auto primitives = backend.primitiveRequests();
    // ROM rejects an unreadable disposition before dispatch. Ordinary mode reaches the opaque setter,
    // whose failure remains uncertain despite its private pre-mutation rejection.
    QCOMPARE(std::ranges::count(primitives, fullCommit), rom ? std::ptrdiff_t{0} : std::ptrdiff_t{1});
    QVERIFY(std::ranges::none_of(primitives, [&](const auto& primitive) { return compat::isWrite(primitive) && primitive != fullCommit; }));
  }
  void capturedPowerPrivilegePrecedesUwfNotification_data() {
    QTest::addColumn<int>("row");
    for (int row = 0; row < 4; ++row) QTest::newRow(qPrintable(QStringLiteral("power-preflight/%1").arg(row))) << row;
  }
  void capturedPowerPrivilegePrecedesUwfNotification() {
    QFETCH(int, row);
    auto state = initialState();
    state.current.filterEnabled = row / 2 != 0;
    state.runtime.shutdownPrivilegeAvailable = row % 2 == 0 ? std::optional<bool>{} : std::optional<bool>{false};
    Fake backend(state);
    const auto before = backend.snapshot();
    for (const auto action : {compat::PowerAction::Shutdown, compat::PowerAction::Restart}) {
      compareReceipt(backend.execute(compat::Power{action}),
                     {row % 2 == 0 ? compat::ErrorKind::Unavailable : compat::ErrorKind::Unsupported, compat::Effect::None, {}});
      QCOMPARE(backend.snapshot(), before);
      QVERIFY(backend.stages().empty());
    }
    const auto primitives = backend.primitiveRequests();
    QVERIFY(std::ranges::none_of(primitives, [](const auto& primitive) { return compat::isWrite(primitive); }));
  }
  void publicOfflineRuntimeReadsDoNotDispatchMissingDeviceGetters() {
    auto state = initialState(true);
    state.current.filterEnabled = false;
    state.next.filterEnabled = true;
    state.runtime.controlDevicePresent = false;
    state.runtime.romDisposition = 4;
    state.runtime.usageOverride = compat::OverlayUsage{7, 9};
    Fake backend(state);
    const auto before = backend.snapshot();
    compareReceipt(backend.execute(compat::GetBoolean{compat::BooleanProperty::OverlayCommit}), {{}, compat::Effect::None, compat::Value{false}});
    compareReceipt(backend.execute(compat::GetOverlayUsage{}), {{}, compat::Effect::None, compat::Value{compat::OverlayUsage{0, 0}}});
    for (const auto& request : backend.primitiveRequests()) {
      QVERIFY(!std::holds_alternative<compat::GetOverlayUsage>(request));
      if (const auto* boolean = std::get_if<compat::GetBoolean>(&request)) QVERIFY(boolean->property != compat::BooleanProperty::OverlayCommit);
      QVERIFY(!compat::isWrite(request));
    }
    QCOMPARE(backend.snapshot(), before);
  }
  void publicFullCommitRejectsBeforeTargetDispatch_data() {
    QTest::addColumn<int>("row");
    for (int row = 0; row < 120; ++row) QTest::newRow(qPrintable(QStringLiteral("full-commit-public/%1").arg(row))) << row;
  }
  void publicFullCommitRejectsBeforeTargetDispatch() {
    QFETCH(int, row);
    auto coordinate = static_cast<std::size_t>(row);
    const auto disposition = coordinate % 10;
    coordinate /= 10;
    const auto flags = coordinate % 3;
    coordinate /= 3;
    const auto nextFilter = coordinate % 2;
    coordinate /= 2;
    const auto currentFilter = coordinate;
    constexpr std::array<std::uint32_t, 10> dispositions{0, 1, 2, 3, 4, 5, 6, 7, 8, 12};
    constexpr std::array<std::uint32_t, 3> loadedFlags{0, 4, 260};
    // A=acknowledged dispatch, K=pre-dispatch conflict, N=existing mask 0x4 no-op; none asserts copy durability.
    constexpr std::array<std::string_view, 3> expectedOnline{"AAAAAAAAAA", "AKKKNNNNKN", "AKKKNNNNKN"};
    const char expected = currentFilter ? expectedOnline[flags][disposition] : 'K';
    auto state = initialState();
    state.current.filterEnabled = currentFilter != 0;
    state.current.overlayFlags = loadedFlags[flags];
    state.next.filterEnabled = nextFilter != 0;
    state.runtime.romDisposition = dispositions[disposition];
    Fake backend(state);
    const auto before = backend.snapshot();
    const compat::Request request = compat::SetBoolean{compat::BooleanProperty::OverlayCommit, true};
    const auto response = backend.execute(request);
    compareReceipt(
        response,
        {expected == 'K' ? std::optional{compat::ErrorKind::Conflict} : std::nullopt, expected == 'A' ? compat::Effect::Confirmed : compat::Effect::None, {}});
    const auto primitives = backend.primitiveRequests();
    const auto targetCalls = std::ranges::count(primitives, request);
    QCOMPARE(targetCalls, expected == 'A' ? 1 : 0);
    if (expected != 'A') {
      QCOMPARE(backend.snapshot(), before);
      QVERIFY(std::ranges::none_of(primitives, [](const auto& primitive) { return compat::isWrite(primitive); }));
    }
    QVERIFY(backend.stages().empty());
  }
  void physicalAdmissionPrecedesRegistrationAndBinding_data() {
    QTest::addColumn<int>("row");
    for (int row = 0; row < 20; ++row) QTest::newRow(qPrintable(QStringLiteral("physical-public/%1").arg(row))) << row;
  }
  void physicalAdmissionPrecedesRegistrationAndBinding() {
    QFETCH(int, row);
    const auto operation = static_cast<std::size_t>(row) / 5;
    const auto physical = static_cast<std::size_t>(row) % 5;
    constexpr std::array<std::string_view, 4> expected{"UKPKA", "UKPKA", "UKKAA", "UKPKA"};
    const char receipt = expected[operation][physical];
    const std::array<std::optional<compat::VolumeAdmission>, 5> observations{std::nullopt, compat::VolumeAdmission{false, false},
                                                                             compat::VolumeAdmission{false, true}, compat::VolumeAdmission{true, false},
                                                                             compat::VolumeAdmission{true, true}};
    auto state = initialState();
    state.current.filterEnabled = false;
    state.runtime.defaultVolumeEligibility.reset();
    if (observations[physical]) state.runtime.physicalVolumes.emplace(volumeA, *observations[physical]);
    if (operation != 2) state.next.volumes.erase(volumeA);
    Fake backend(state);
    const auto before = backend.snapshot();
    const std::array<compat::Request, 4> requests{compat::AddVolume{volumeA, {}}, compat::SetVolume{volumeA, compat::VolumeProperty::Protected, true},
                                                  compat::SetVolume{volumeA, compat::VolumeProperty::BindByDriveLetter, true}, compat::CreateSwapfile{volumeA}};
    const auto response = backend.execute(requests[operation]);
    if (receipt == 'A') {
      QVERIFY(response.succeeded());
      QCOMPARE(response.effect, compat::Effect::Confirmed);
      const auto primitives = backend.primitiveRequests();
      QVERIFY(std::ranges::all_of(primitives, [](const auto& primitive) { return compat::isWrite(primitive); }));
    } else {
      compareReceipt(response, {receipt == 'U'   ? compat::ErrorKind::Unavailable
                                : receipt == 'P' ? compat::ErrorKind::Protocol
                                                 : compat::ErrorKind::Conflict,
                                compat::Effect::None,
                                {}});
      QCOMPARE(backend.snapshot(), before);
      const auto primitives = backend.primitiveRequests();
      QVERIFY(primitives.empty());
    }
  }
  void publishedFileTargetRejectionsPrecedeRawDispatch_data() {
    QTest::addColumn<int>("scenario");
    const std::array names{"missing-file",
                           "directory",
                           "nonempty-directory",
                           "nonempty-deletion",
                           "full-path-buffer",
                           "reserved-target",
                           "invalid-target-kind",
                           "persistent-view-unavailable",
                           "known-missing-current-volume",
                           "complete-inventory-miss"};
    for (std::size_t index = 0; index < names.size(); ++index) QTest::newRow(names[index]) << int(index);
  }
  void publishedFileTargetRejectionsPrecedeRawDispatch() {
    QFETCH(int, scenario);
    Fake backend(initialState());
    backend.seedFile(volumeA, path, "overlay content", Fake::Storage::Overlay);
    const auto before = backend.snapshot();
    const compat::Request request =
        scenario == 3 ? compat::Request{compat::CommitFileDeletion{volumeA, path}} : compat::Request{compat::CommitFile{volumeA, path}};
    auto facts = backend.state();
    facts.requestTarget = request;
    auto& target = facts.targets[volumeA];
    compat::FileMaintenanceFacts file{15, compat::FileMaintenanceKind::File, false};
    switch (scenario) {
      case 0:
        file.kind = compat::FileMaintenanceKind::Missing;
        break;
      case 1:
        file.kind = compat::FileMaintenanceKind::EmptyDirectory;
        break;
      case 2:
      case 3:
        file.kind = compat::FileMaintenanceKind::NonemptyDirectory;
        break;
      case 4:
        file.fullPathUtf16Units = compat::limits::fileMaintenanceBufferUnits;
        break;
      case 5:
        file.reserved = true;
        break;
      case 6:
        file.kind = static_cast<compat::FileMaintenanceKind>(255);
        break;
      case 7:
        target.persistentFileViewReady = compat::Fact<bool>::known(false);
        break;
      case 8:
        facts.current.volumes.at(volumeA).present = compat::Fact<bool>::known(false);
        break;
      case 9:
        facts.current.volumes.erase(volumeA);
        facts.current.volumesObserved = true;
        break;
    }
    target.fileTarget = compat::Fact<compat::FileMaintenanceFacts>::known(file);
    const auto result = backend.execute(request, {{}, &facts});
    const std::array errors{compat::ErrorKind::NotFound, compat::ErrorKind::InvalidArgument, compat::ErrorKind::InvalidArgument,
                            compat::ErrorKind::Conflict, compat::ErrorKind::InvalidArgument, compat::ErrorKind::InvalidArgument,
                            compat::ErrorKind::Protocol, compat::ErrorKind::Unavailable,     compat::ErrorKind::NotFound,
                            compat::ErrorKind::NotFound};
    compareReceipt(result, {errors.at(static_cast<std::size_t>(scenario)), compat::Effect::None, {}});
    QCOMPARE(backend.snapshot(), before);
    QVERIFY(backend.primitiveRequests().empty());
  }
  void unknownOrDifferentFileTargetFactsDoNotAuthorizeAnotherTarget_data() {
    QTest::addColumn<bool>("unknown");
    QTest::newRow("different-exact-target") << false;
    QTest::newRow("target-not-observed") << true;
  }
  void unknownOrDifferentFileTargetFactsDoNotAuthorizeAnotherTarget() {
    QFETCH(bool, unknown);
    Fake backend(initialState());
    backend.seedFile(volumeA, path, "overlay content", Fake::Storage::Overlay);
    const compat::Request request = compat::CommitFile{volumeA, path};
    auto facts = backend.state();
    facts.requestTarget = unknown ? request : compat::Request{compat::CommitFile{volumeA, sibling}};
    if (!unknown) facts.targets[volumeA].fileTarget = compat::Fact<compat::FileMaintenanceFacts>::known({15, compat::FileMaintenanceKind::Missing, true});
    const auto result = backend.execute(request, {{}, &facts});
    QVERIFY(result.succeeded());
    QCOMPARE(result.effect, compat::Effect::Confirmed);
    QCOMPARE(backend.primitiveRequests(), std::vector<compat::Request>{request});
    QCOMPARE(backend.readFile(volumeA, path), std::optional<std::string>{"overlay content"});
  }
  void missingCapturedSettingsRejectBeforeResetDispatch() {
    auto state = initialState();
    state.current.filterEnabled = false;
    state.capturedSettings.reset();
    Fake backend(state);
    const auto before = backend.snapshot();
    compareReceipt(backend.execute(compat::ResetSettings{}), {compat::ErrorKind::Conflict, compat::Effect::None, {}});
    QCOMPARE(backend.snapshot(), before);
    const auto primitives = backend.primitiveRequests();
    QVERIFY(std::ranges::none_of(primitives, [](const auto& primitive) { return compat::isWrite(primitive); }));
  }
  void capturedControlDeviceReadBoundaries_data() {
    QTest::addColumn<bool>("devicePresent");
    QTest::addColumn<bool>("cachedValue");
    QTest::addColumn<bool>("shutdownExpected");
    // Literal observations from GetSystemShutdownState 0x17a10 and
    // GetOverlayCommitState 0x1a130/CDevice::SendIOCTL 0x15c70. The former
    // accepts absent-device false; the latter returns E_UNEXPECTED.
    QTest::newRow("absent/cache-false") << false << false << false;
    QTest::newRow("absent/cache-true") << false << true << false;
    QTest::newRow("present/cache-false") << true << false << false;
    QTest::newRow("present/cache-true") << true << true << true;
  }
  void capturedControlDeviceReadBoundaries() {
    QFETCH(bool, devicePresent);
    QFETCH(bool, cachedValue);
    QFETCH(bool, shutdownExpected);
    auto state = initialState();
    state.runtime.controlDevicePresent = devicePresent;
    state.runtime.shutdownPending = cachedValue;
    state.runtime.romDisposition = cachedValue ? 4 : 0;
    Fake backend(state);
    const auto before = backend.snapshot();
    for (const auto session : {compat::Session::Current, compat::Session::Next}) {
      const auto shutdown = backend.executeCapturedPrimitiveForTest(compat::GetBoolean{compat::BooleanProperty::ShutdownPending, session});
      compareReceipt(shutdown, {{}, compat::Effect::None, compat::Value{shutdownExpected}});
      const auto commit = backend.executeCapturedPrimitiveForTest(compat::GetBoolean{compat::BooleanProperty::OverlayCommit, session});
      if (devicePresent) {
        compareReceipt(commit, {{}, compat::Effect::None, compat::Value{cachedValue}});
      } else {
        compareReceipt(commit, {compat::ErrorKind::Unavailable, compat::Effect::None, {}});
        QVERIFY(commit.failure);
        QVERIFY(!commit.failure->diagnostics.empty());
        QCOMPARE(commit.failure->diagnostics.front().code, static_cast<std::int64_t>(static_cast<std::int32_t>(0x8000ffffu)));
      }
      Fake publicBackend(state);
      const auto publicCommit = publicBackend.execute(compat::GetBoolean{compat::BooleanProperty::OverlayCommit, session});
      compareReceipt(publicCommit, devicePresent ? captured::ExpectedReceipt{{}, compat::Effect::None, compat::Value{cachedValue}}
                                                 : captured::ExpectedReceipt{compat::ErrorKind::Failed, compat::Effect::None, {}});
      if (!devicePresent) QCOMPARE(publicCommit.failure->diagnostics.back().code, static_cast<std::int64_t>(static_cast<std::int32_t>(0x8000ffffu)));
      QCOMPARE(publicBackend.snapshot(), state);
    }
    QCOMPARE(backend.snapshot(), before);
    QVERIFY(backend.stages().empty());
  }
  void capturedFullCommitMissingDevicePreservesNativeCode() {
    auto state = initialState(true);
    state.runtime.controlDevicePresent = false;
    Fake backend(state);
    const auto before = backend.snapshot();
    const auto response = backend.executeCapturedPrimitiveForTest(compat::SetBoolean{compat::BooleanProperty::OverlayCommit, true});
    compareReceipt(response, {compat::ErrorKind::Unavailable, compat::Effect::None, {}});
    QVERIFY(response.failure);
    QVERIFY(!response.failure->diagnostics.empty());
    QCOMPARE(response.failure->diagnostics.front().code, static_cast<std::int64_t>(static_cast<std::int32_t>(0x8000ffffu)));
    QCOMPARE(backend.snapshot(), before);
    QVERIFY(backend.stages().empty());
  }
  void publicNonRomFullCommitMissingDeviceKeepsOpaqueFailure() {
    auto state = initialState();
    state.runtime.controlDevicePresent = false;
    Fake backend(state);
    const auto response = backend.execute(compat::SetBoolean{compat::BooleanProperty::OverlayCommit, true});
    compareReceipt(response, {compat::ErrorKind::Failed, compat::Effect::Uncertain, {}});
    QCOMPARE(response.failure->diagnostics.back().code, static_cast<std::int64_t>(static_cast<std::int32_t>(0x8000ffffu)));
    QCOMPARE(backend.snapshot(), state);
  }
  void frozenSessionObservations_data() {
    QTest::addColumn<int>("index");
    const auto vectors = captured::filterSessionReads();
    for (std::size_t i = 0; i < vectors.size(); ++i) QTest::newRow(vectors[i].id) << static_cast<int>(i);
  }
  void frozenSessionObservations() {
    QFETCH(int, index);
    auto state = initialState();
    state.current.filterEnabled = false;
    state.next.filterEnabled = true;
    Fake backend(state);
    const auto vector = captured::filterSessionReads().at(static_cast<std::size_t>(index));
    compareReceipt(backend.execute(vector.request), vector.expected);
  }
  void frozenExactTargetRejections_data() {
    QTest::addColumn<int>("index");
    const auto vectors = captured::exactTargetRejections();
    for (std::size_t i = 0; i < vectors.size(); ++i) QTest::newRow(vectors[i].id) << static_cast<int>(i);
  }
  void frozenExactTargetRejections() {
    QFETCH(int, index);
    Fake backend(initialState());
    const auto before = backend.snapshot();
    const auto vector = captured::exactTargetRejections().at(static_cast<std::size_t>(index));
    compareReceipt(backend.execute(vector.request), vector.expected);
    QCOMPARE(backend.snapshot(), before);
    QVERIFY(backend.stages().empty());
  }
  void frozenHormTransitions_data() {
    QTest::addColumn<int>("index");
    const auto vectors = captured::hormTransitions();
    for (std::size_t i = 0; i < vectors.size(); ++i) QTest::newRow(vectors[i].id) << static_cast<int>(i);
  }
  void frozenHormTransitions() {
    QFETCH(int, index);
    const auto vector = captured::hormTransitions().at(static_cast<std::size_t>(index));
    auto state = initialState(true);
    state.runtime.hormEnabled = vector.beforeEnabled;
    state.runtime.romDisposition = vector.beforeDisposition;
    state.runtime.controlDevicePresent = vector.controlPresent;
    Fake backend(state);
    compareReceipt(backend.execute(compat::SetBoolean{compat::BooleanProperty::HormEnabled, vector.requestedEnabled}), vector.expected);
    const auto after = backend.snapshot();
    QCOMPARE(after.runtime.hormEnabled, vector.afterEnabled);
    QCOMPARE(after.runtime.romDisposition, vector.afterDisposition);
    QCOMPARE(after.current, state.current);
    QCOMPARE(after.next, state.next);
    if (std::string(vector.id) == "disable-or-one") {
      // OR-ing mask 0x1 permits the subsequent filter change.
      QVERIFY(backend.execute(compat::SetBoolean{compat::BooleanProperty::FilterEnabled, false}).succeeded());
      QCOMPARE(backend.snapshot().runtime.romDisposition, std::uint32_t{3});
    }
  }
  void managedHormOwnsItsPrivateObservations_data() {
    QTest::addColumn<int>("flagsSupport");
    QTest::addColumn<int>("typeSupport");
    QTest::addColumn<int>("caseIndex");
    const auto cases = managedHormCases();
    QCOMPARE(cases.size(), std::size_t{23});
    for (const auto flags : {compat::Support::Supported, compat::Support::Unsupported, compat::Support::Unavailable})
      for (const auto type : {compat::Support::Supported, compat::Support::Unsupported, compat::Support::Unavailable})
        for (std::size_t index = 0; index < cases.size(); ++index)
          QTest::newRow(qUtf8Printable(QStringLiteral("flags%1-type%2-%3").arg(static_cast<int>(flags)).arg(static_cast<int>(type)).arg(cases[index].id)))
              << static_cast<int>(flags) << static_cast<int>(type) << static_cast<int>(index);
  }
  void managedHormOwnsItsPrivateObservations() {
    QFETCH(int, flagsSupport);
    QFETCH(int, typeSupport);
    QFETCH(int, caseIndex);
    const auto entry = managedHormCases().at(static_cast<std::size_t>(caseIndex));
    auto state = initialState(true);
    state.current.filterEnabled = entry.currentFilter;
    state.current.overlayType = entry.type;
    state.current.overlayFlags = entry.flags;
    state.runtime.hiberfileExists = entry.hiberfile;
    state.runtime.hormFixedVolumeSafety = entry.physical;
    state.runtime.romDisposition = entry.disposition;
    state.runtime.hormFinalSafety = entry.finalSafety;
    state.runtime.controlDeviceObservationAvailable = entry.controlKnown;
    state.runtime.controlDevicePresent = entry.control;
    const compat::Request flagsGetter = compat::GetNumber{compat::NumberProperty::OverlayFlags, compat::Session::Current};
    const compat::Request typeGetter = compat::GetNumber{compat::NumberProperty::OverlayType, compat::Session::Current};
    const compat::Request request = compat::SetBoolean{compat::BooleanProperty::HormEnabled, true};
    const auto restrictGetters = [&](Fake& backend) {
      backend.setCapability(flagsGetter, {static_cast<compat::Support>(flagsSupport), "Independent public Flags getter capability"});
      backend.setCapability(typeGetter, {static_cast<compat::Support>(typeSupport), "Independent public Type getter capability"});
    };
    Fake backend(state);
    restrictGetters(backend);
    QCOMPARE(backend.probe(flagsGetter).support, static_cast<compat::Support>(flagsSupport));
    QCOMPARE(backend.probe(typeGetter).support, static_cast<compat::Support>(typeSupport));
    QCOMPARE(backend.probe(request).support, compat::Support::Supported);
    const auto publicReceipt = backend.execute(request);
    compareReceipt(publicReceipt, entry.portableExpected);
    const auto after = backend.snapshot();
    QCOMPARE(after.current, state.current);
    QCOMPARE(after.next, state.next);
    QCOMPARE(after.runtime.hormEnabled, !entry.expected.failure.has_value());
    QCOMPARE(after.runtime.romDisposition, entry.afterDisposition);
    if (entry.expected.effect == compat::Effect::None) QCOMPARE(after, state);
    const auto primitives = backend.primitiveRequests();
    QCOMPARE(std::ranges::count(primitives, request), std::ptrdiff_t{1});
    QCOMPARE(std::ranges::count(primitives, flagsGetter), std::ptrdiff_t{0});
    QCOMPARE(std::ranges::count(primitives, typeGetter), std::ptrdiff_t{0});
    Fake raw(state);
    restrictGetters(raw);
    const auto capturedReceipt = raw.executeCapturedPrimitiveForTest(request);
    compareReceipt(capturedReceipt, entry.expected);
    if (entry.physical == Fake::SafetyObservation::Satisfied && entry.disposition != 0) {
      QCOMPARE(capturedReceipt.failure->diagnostics.back().code, std::int64_t{0xc0000184});
      QCOMPARE(publicReceipt.failure->diagnostics.back().code, std::int64_t{static_cast<std::int32_t>(0x80070016u)});
    }
    QCOMPARE(raw.snapshot(), after);
    QCOMPARE(raw.primitiveRequests(), std::vector<compat::Request>{request});
  }
  void capturedHormControlGatePrecedesItsOwnSameStateBranch() {
    for (const bool enabled : {false, true}) {
      auto state = initialState(true);
      state.runtime.hormEnabled = enabled;
      state.runtime.controlDevicePresent = false;
      const compat::Request request = compat::SetBoolean{compat::BooleanProperty::HormEnabled, enabled};
      Fake backend(state);
      compareReceipt(backend.execute(request), {{}, compat::Effect::None, {}});
      QVERIFY(backend.snapshot() == state);
      const auto primitives = backend.primitiveRequests();
      QCOMPARE(std::ranges::count(primitives, request), std::ptrdiff_t{0});
      Fake raw(state);
      const auto captured = raw.executeCapturedPrimitiveForTest(request);
      compareReceipt(captured, {compat::ErrorKind::Unavailable, compat::Effect::None, {}});
      QVERIFY(captured.failure);
      QCOMPARE(captured.failure->diagnostics.back().code, std::int64_t{static_cast<std::int32_t>(0x8007070du)});
      QCOMPARE(raw.snapshot(), state);
    }
  }
  void hormSafetyIsAnIndependentObservation_data() {
    QTest::addColumn<bool>("finalCheck");
    QTest::addColumn<bool>("unknown");
    for (const bool finalCheck : {false, true})
      for (const bool unknown : {false, true})
        QTest::newRow(qPrintable(QStringLiteral("%1/%2").arg(finalCheck ? "final" : "fixed-volumes").arg(unknown ? "unknown" : "rejected")))
            << finalCheck << unknown;
  }
  void hormSafetyIsAnIndependentObservation() {
    QFETCH(bool, finalCheck);
    QFETCH(bool, unknown);
    auto state = initialState(true);
    const auto safety = unknown ? Fake::SafetyObservation::Unknown : Fake::SafetyObservation::Rejected;
    if (finalCheck)
      state.runtime.hormFinalSafety = safety;
    else
      state.runtime.hormFixedVolumeSafety = safety;
    Fake backend(state);
    const auto response = backend.execute(compat::SetBoolean{compat::BooleanProperty::HormEnabled, true});
    QVERIFY(response.failure);
    QCOMPARE(response.failure->kind, unknown ? compat::ErrorKind::Unavailable : compat::ErrorKind::Conflict);
    QCOMPARE(response.effect, compat::Effect::Uncertain);
    QCOMPARE(backend.snapshot().runtime.romDisposition, finalCheck ? 2u : 0u);
    QVERIFY(!backend.snapshot().runtime.hormEnabled);
  }
  void copyFailureKeepsOnlyProvenBytesAndMetadata_data() {
    QTest::addColumn<int>("index");
    const auto vectors = captured::copyFailures();
    for (std::size_t i = 0; i < vectors.size(); ++i) QTest::newRow(vectors[i].id) << static_cast<int>(i);
  }
  void copyFailureKeepsOnlyProvenBytesAndMetadata() {
    QFETCH(int, index);
    const auto vector = captured::copyFailures().at(static_cast<std::size_t>(index));
    const auto shared = Fake::createSystem(initialState());
    const auto backend = std::make_shared<Fake>(shared, "Copy-source");
    const auto fallback = std::make_shared<Fake>(shared, "Must-not-replay");
    compat::FallbackUwf system({backend, fallback});
    const std::string contents(65537, 'N');
    backend->seedFile(volumeA, path, "old base");
    backend->writeFile(volumeA, path, contents);
    backend->seedFile(volumeA, sibling, "unrelated base");
    backend->writeFile(volumeA, sibling, "unrelated overlay");
    const compat::Request request = compat::CommitFile{volumeA, path};
    backend->inject({request, {}, copyStage(vector.checkpoint), compat::ErrorKind::Unavailable, "original copy failure", 1722});
    const auto response = system.execute(request);
    QVERIFY(response.failure);
    QCOMPARE(response.failure->kind, compat::ErrorKind::Unavailable);
    QCOMPARE(response.effect, compat::Effect::Uncertain);
    QVERIFY(!response.failure->diagnostics.empty());
    QCOMPARE(response.failure->diagnostics.back().code, std::int64_t{1722});
    const auto state = backend->snapshot();
    const Fake::FileAddress address{volumeA, path};
    if (vector.exactDataKnown) {
      QCOMPARE(backend->readFile(volumeA, path), std::optional<std::string>{contents});
      QVERIFY(!state.partialPersistentFiles.contains(address));
    } else {
      const auto& partial = state.partialPersistentFiles.at(address);
      QCOMPARE(partial.length, std::optional<std::size_t>{65537});
      QCOMPARE(partial.completedPrefix, contents.substr(0, vector.completedPrefixBytes));
      QVERIFY_THROWS_EXCEPTION(std::logic_error, (void)backend->readFile(volumeA, path));
    }
    constexpr std::array metadata{Fake::FaultTiming::AfterFileEa, Fake::FaultTiming::AfterFileAds, Fake::FaultTiming::AfterFileTime,
                                  Fake::FaultTiming::AfterFileSecurity};
    const auto& stages = state.fileCommitStages.at(address);
    QCOMPARE(static_cast<std::size_t>(std::ranges::count_if(stages,
                                                            [](auto stage) {
                                                              return stage == Fake::FaultTiming::AfterFileEa || stage == Fake::FaultTiming::AfterFileAds ||
                                                                     stage == Fake::FaultTiming::AfterFileTime || stage == Fake::FaultTiming::AfterFileSecurity;
                                                            })),
             vector.completedMetadataHelpers);
    std::vector<Fake::FaultTiming> completedMetadata;
    for (const auto stage : stages)
      if (std::ranges::find(metadata, stage) != metadata.end()) completedMetadata.push_back(stage);
    QVERIFY(std::equal(completedMetadata.begin(), completedMetadata.end(), metadata.begin()));
    QCOMPARE(backend->visibleFile(volumeA, path), std::optional<std::string>{contents});
    QCOMPARE(backend->readFile(volumeA, sibling), std::optional<std::string>{"unrelated base"});
    QCOMPARE(backend->visibleFile(volumeA, sibling), std::optional<std::string>{"unrelated overlay"});
    QVERIFY(fallback->trace().empty());
  }
  void deletionFailurePreservesOriginalAndCleanupFacts_data() {
    QTest::addColumn<bool>("directory");
    QTest::addColumn<int>("stage");
    QTest::addColumn<bool>("compensationFails");
    QTest::addColumn<bool>("restoreFails");
    QTest::addColumn<bool>("visibleAfter");
    QTest::addColumn<bool>("baseAfter");
    for (const bool directory : {false, true}) {
      const auto row = [&](const char* name, Fake::FaultTiming stage, bool compensationFails, bool restoreFails, bool visibleAfter, bool baseAfter) {
        QTest::newRow(qPrintable(QStringLiteral("%1/%2").arg(directory ? QStringLiteral("directory") : QStringLiteral("file"), QString::fromLatin1(name))))
            << directory << static_cast<int>(stage) << compensationFails << restoreFails << visibleAfter << baseAfter;
      };
      row("access-adjusted", Fake::FaultTiming::AfterFileAccessAdjusted, false, false, true, true);
      row("visible-compensated", Fake::FaultTiming::AfterVisibleDeletion, false, false, true, true);
      row("visible-compensation-fails", Fake::FaultTiming::AfterVisibleDeletion, true, false, false, true);
      row("both-cleanups-fail", Fake::FaultTiming::BeforePersistentDeletion, true, true, false, true);
      row("persistent-done-receipt-lost", Fake::FaultTiming::AfterPersistentDeletion, false, false, false, false);
    }
  }
  void completedShortStreamBytesAreKnownEvenWhenReceiptFails_data() {
    QTest::addColumn<int>("length");
    QTest::newRow("empty-eof") << 0;
    QTest::newRow("one-byte-write") << 1;
    QTest::newRow("one-full-chunk") << 65536;
  }
  void completedShortStreamBytesAreKnownEvenWhenReceiptFails() {
    QFETCH(int, length);
    Fake backend(initialState());
    backend.seedFile(volumeA, path, "old longer contents");
    const std::string contents(static_cast<std::size_t>(length), 'N');
    backend.writeFile(volumeA, path, contents);
    const compat::Request request = compat::CommitFile{volumeA, path};
    backend.inject({request,
                    {},
                    length == 0 ? Fake::FaultTiming::AfterDestinationEof : Fake::FaultTiming::AfterFileDataChunk,
                    compat::ErrorKind::Failed,
                    "receipt after proven data",
                    1722});
    const auto response = backend.execute(request);
    QVERIFY(response.failure);
    QCOMPARE(response.effect, compat::Effect::Uncertain);
    QCOMPARE(backend.readFile(volumeA, path), std::optional<std::string>{contents});
    QVERIFY(backend.snapshot().partialPersistentFiles.empty());
  }
  void deletionFailurePreservesOriginalAndCleanupFacts() {
    QFETCH(bool, directory);
    QFETCH(int, stage);
    QFETCH(bool, compensationFails);
    QFETCH(bool, restoreFails);
    QFETCH(bool, visibleAfter);
    QFETCH(bool, baseAfter);
    const auto checkpoint = static_cast<Fake::FaultTiming>(stage);
    Fake backend(initialState());
    if (directory)
      backend.seedDirectory(volumeA, path);
    else
      backend.seedFile(volumeA, path, "base");
    backend.seedFile(volumeA, sibling, "sibling");
    const compat::Request request = compat::CommitFileDeletion{volumeA, path};
    backend.inject({request, {}, checkpoint, compat::ErrorKind::AccessDenied, "original persistent deletion error", 5});
    if (compensationFails)
      backend.inject({request, {}, Fake::FaultTiming::DuringDeleteCompensation, compat::ErrorKind::Unavailable, "cleanup must not replace original", 1722});
    if (restoreFails)
      backend.inject({request, {}, Fake::FaultTiming::DuringDeletePermissionsRestore, compat::ErrorKind::Failed, "restore must not replace original", 87});
    const auto response = backend.execute(request);
    QVERIFY(response.failure);
    QCOMPARE(response.failure->kind, compat::ErrorKind::AccessDenied);
    QCOMPARE(response.failure->message, std::string("original persistent deletion error"));
    QCOMPARE(response.failure->diagnostics.back().code, std::int64_t{5});
    QCOMPARE(response.effect, compat::Effect::Uncertain);
    const auto state = backend.snapshot();
    const Fake::FileAddress address{volumeA, path};
    QCOMPARE(directory ? backend.visibleDirectory(volumeA, path) : backend.visibleFile(volumeA, path).has_value(), visibleAfter);
    QCOMPARE(directory ? state.durable.directories.contains(address) : backend.readFile(volumeA, path).has_value(), baseAfter);
    const auto& progress = state.fileDeletions.at(address);
    if (checkpoint == Fake::FaultTiming::AfterPersistentDeletion) {
      QVERIFY(progress.persistentDeleted);
      QVERIFY(!progress.compensationAttempted);
      QVERIFY(!progress.permissionsRestoreAttempted);
    } else {
      QCOMPARE(progress.compensationAttempted, checkpoint != Fake::FaultTiming::AfterFileAccessAdjusted);
      if (progress.compensationAttempted) QCOMPARE(progress.compensationSucceeded, !compensationFails);
      QVERIFY(progress.permissionsRestoreAttempted);
      QCOMPARE(progress.permissionsRestoreSucceeded, !restoreFails);
    }
    QCOMPARE(backend.readFile(volumeA, sibling), std::optional<std::string>{"sibling"});
  }
  void successfulDeletionBypassesRestore() {
    Fake backend(initialState());
    backend.seedFile(volumeA, path, "base");
    QVERIFY(backend.execute(compat::CommitFileDeletion{volumeA, path}).succeeded());
    const auto progress = backend.snapshot().fileDeletions.at({volumeA, path});
    QVERIFY(progress.visibleDeleted);
    QVERIFY(progress.persistentDeleted);
    QVERIFY(!progress.compensationAttempted);
    QVERIFY(!progress.permissionsRestoreAttempted);
  }
  void driverOnlyExclusionDeletesTheBaseSourceBeforeAnySecondDelete() {
    auto state = initialState();
    state.current.volumes.at(volumeA).fileExclusions = {"\\\xC3\x84"};
    Fake backend(state);
    const std::string selected = "\\\xC3\xA4\\file";
    backend.seedFile(volumeA, selected, "base");
    const compat::Request request = compat::CommitFileDeletion{volumeA, selected};
    backend.inject({request, {}, Fake::FaultTiming::AfterVisibleDeletion, compat::ErrorKind::Failed, "lost source-delete receipt", 1722});
    const auto response = backend.execute(request);
    QVERIFY(response.failure);
    QCOMPARE(response.effect, compat::Effect::Uncertain);
    QVERIFY(!backend.readFile(volumeA, selected));
    QVERIFY(!backend.visibleFile(volumeA, selected));
    const auto progress = backend.snapshot().fileDeletions.at({volumeA, selected});
    QVERIFY(progress.persistentDeleted);
    QVERIFY(!progress.compensationAttempted);
  }
  void fullCommitAllowsMixedFilterOutcomes_data() {
    QTest::addColumn<bool>("failA");
    QTest::addColumn<bool>("failB");
    QTest::addColumn<QString>("expectedA");
    QTest::addColumn<QString>("expectedB");
    QTest::newRow("both-complete") << false << false << QStringLiteral("new-A") << QStringLiteral("new-B");
    QTest::newRow("first-fails") << true << false << QStringLiteral("old-A") << QStringLiteral("new-B");
    QTest::newRow("last-fails") << false << true << QStringLiteral("new-A") << QStringLiteral("old-B");
    QTest::newRow("both-fail") << true << true << QStringLiteral("old-A") << QStringLiteral("old-B");
  }
  void fullCommitAllowsMixedFilterOutcomes() {
    QFETCH(bool, failA);
    QFETCH(bool, failB);
    QFETCH(QString, expectedA);
    QFETCH(QString, expectedB);
    auto state = initialState(true);
    Fake::FullCommitFilterOutcome outcomeA;
    outcomeA.copyFails = failA;
    Fake::FullCommitFilterOutcome outcomeB;
    outcomeB.copyFails = failB;
    state.runtime.fullCommitFilters.emplace(volumeA, outcomeA);
    state.runtime.fullCommitFilters.emplace(volumeB, outcomeB);
    Fake backend(state);
    backend.seedFile(volumeA, path, "old-A");
    backend.seedFile(volumeB, path, "old-B");
    backend.writeFile(volumeA, path, "new-A");
    backend.writeFile(volumeB, path, "new-B");
    const auto receipt = backend.execute(compat::SetBoolean{compat::BooleanProperty::OverlayCommit, true});
    QVERIFY(receipt.succeeded());
    QCOMPARE(receipt.effect, compat::Effect::Confirmed);
    QCOMPARE(backend.readFile(volumeA, path), std::optional<std::string>{expectedA.toStdString()});
    QCOMPARE(backend.readFile(volumeB, path), std::optional<std::string>{expectedB.toStdString()});
    QVERIFY(backend.snapshot().overlay.files.empty());
    QCOMPARE(backend.snapshot().runtime.romDisposition, std::uint32_t{4});
    backend.writeFile(volumeA, path, "later-A");
    backend.writeFile(volumeB, path, "later-B");
    QCOMPARE(backend.readFile(volumeA, path), std::optional<std::string>{"later-A"});
    QCOMPARE(backend.readFile(volumeB, path), std::optional<std::string>{"later-B"});
  }
  void fullCommitFailureCanKeepWitnessesAndUnknownRanges() {
    auto state = initialState(true);
    state.runtime.fullCommitFilters.emplace(
        volumeA,
        Fake::FullCommitFilterOutcome{.copyFails = true, .completedFilePaths = {path}, .indeterminateFilePaths = {sibling}, .registryCopyCompleted = true});
    Fake backend(state);
    backend.seedFile(volumeA, path, "old");
    backend.seedFile(volumeA, sibling, "old sibling");
    backend.writeFile(volumeA, path, "completed");
    backend.writeFile(volumeA, sibling, "partially copied");
    backend.seedRegistryValue(key, "Value", "old");
    backend.writeRegistryValue(key, "Value", "completed registry");
    QVERIFY(backend.execute(compat::SetBoolean{compat::BooleanProperty::OverlayCommit, true}).succeeded());
    QCOMPARE(backend.readFile(volumeA, path), std::optional<std::string>{"completed"});
    QCOMPARE(backend.readRegistryValue(key, "Value"), std::optional<std::string>{"completed registry"});
    QVERIFY_THROWS_EXCEPTION(std::logic_error, (void)backend.readFile(volumeA, sibling));
    QVERIFY(!backend.snapshot().partialPersistentFiles.at({volumeA, sibling}).length);
    QVERIFY(backend.snapshot().overlay.files.empty());
  }
  void explicitReadinessDiffersFromPublicRomAdmission_data() {
    QTest::addColumn<bool>("rom");
    QTest::addColumn<int>("view");
    for (const bool rom : {false, true})
      for (const auto view : {Fake::PersistentView::Unknown, Fake::PersistentView::NotReady, Fake::PersistentView::Ready})
        QTest::newRow(qPrintable(QStringLiteral("rom-%1/view-%2").arg(rom).arg(static_cast<int>(view)))) << rom << static_cast<int>(view);
  }
  void managementExcludedFilesDoNotRequireAPersistentView_data() {
    QTest::addColumn<int>("scenario");
    QTest::newRow("exact-management-exclusion") << 0;
    QTest::newRow("descendant-management-exclusion") << 1;
    QTest::newRow("driver-only-coverage-still-requires-view") << 2;
  }
  void managementExcludedFilesDoNotRequireAPersistentView() {
    QFETCH(int, scenario);
    const std::array<std::string, 3> patterns{path, "\\Scope", "\\\xC3\x84"};
    const std::array<std::string, 3> targets{path, "\\sCOPE\\entry", "\\\xC3\xA4"};
    auto state = initialState();
    state.current.volumes.at(volumeA).fileExclusions = {patterns.at(static_cast<std::size_t>(scenario))};
    state.runtime.defaultFileCommitView = Fake::PersistentView::NotReady;
    Fake backend(state);
    const auto& target = targets.at(static_cast<std::size_t>(scenario));
    backend.seedFile(volumeA, target, "visible content", Fake::Storage::Overlay);
    const auto before = backend.snapshot();
    const compat::Request request = compat::CommitFile{volumeA, target};
    auto facts = backend.state();
    facts.requestTarget = request;
    facts.targets[volumeA].fileTarget = compat::Fact<compat::FileMaintenanceFacts>::known({15, compat::FileMaintenanceKind::File, false});
    facts.targets[volumeA].persistentFileViewReady = compat::Fact<bool>::known(false);
    const auto response = backend.execute(request, {{}, &facts});
    if (scenario != 2) {
      QVERIFY(response.succeeded());
      QCOMPARE(response.effect, compat::Effect::Confirmed);
      QCOMPARE(backend.primitiveRequests(), std::vector<compat::Request>{request});
    } else {
      compareReceipt(response, {compat::ErrorKind::Unavailable, compat::Effect::None, {}});
      QVERIFY(backend.primitiveRequests().empty());
    }
    QCOMPARE(backend.snapshot(), before);
  }
  void explicitReadinessDiffersFromPublicRomAdmission() {
    QFETCH(bool, rom);
    QFETCH(int, view);
    auto state = initialState(rom);
    state.runtime.fileCommitViews[volumeA] = static_cast<Fake::PersistentView>(view);
    Fake backend(state);
    backend.seedFile(volumeA, path, "base");
    backend.seedFile(volumeA, path, "overlay", Fake::Storage::Overlay);
    const compat::Request request = compat::CommitFile{volumeA, path};
    const auto raw = backend.executeCapturedPrimitiveForTest(request);
    QCOMPARE(raw.succeeded(), view == static_cast<int>(Fake::PersistentView::Ready));
    QVERIFY(backend.trace().front().capturedPrimitive);
    if (!raw.succeeded()) {
      QCOMPARE(raw.effect, compat::Effect::None);
      QCOMPARE(raw.failure->diagnostics.back().code, view == static_cast<int>(Fake::PersistentView::NotReady) ? std::int64_t{0xc00000a3} : std::int64_t{0});
    }
    if (!rom) {
      Fake publicBackend(state);
      publicBackend.seedFile(volumeA, path, "base");
      publicBackend.seedFile(volumeA, path, "overlay", Fake::Storage::Overlay);
      const auto publicReceipt = publicBackend.execute(request);
      QCOMPARE(publicReceipt.succeeded(), view == static_cast<int>(Fake::PersistentView::Ready));
      if (!publicReceipt.succeeded()) {
        QCOMPARE(publicReceipt.failure->kind, compat::ErrorKind::Unavailable);
        const bool knownNotReady = view == static_cast<int>(Fake::PersistentView::NotReady);
        QCOMPARE(publicReceipt.effect, knownNotReady ? compat::Effect::None : compat::Effect::Uncertain);
        if (knownNotReady) {
          QVERIFY(publicBackend.primitiveRequests().empty());
          QVERIFY(publicReceipt.failure->diagnostics.empty());
        } else {
          QCOMPARE(publicBackend.primitiveRequests(), std::vector<compat::Request>{request});
          QCOMPARE(publicReceipt.failure->diagnostics.back().code, std::int64_t{0});
        }
        QCOMPARE(publicBackend.snapshot(), backend.snapshot());
      }
    }
    if (rom) {
      const auto before = backend.snapshot();
      const auto publicReceipt = backend.execute(request);
      QVERIFY(publicReceipt.failure);
      QCOMPARE(publicReceipt.failure->kind, compat::ErrorKind::AccessDenied);
      QCOMPARE(publicReceipt.effect, compat::Effect::None);
      QCOMPARE(backend.snapshot(), before);
      QVERIFY(!backend.trace().back().capturedPrimitive);
    }
  }
  void cachedRegistryHiveIsNotInferredFromRom_data() {
    QTest::addColumn<bool>("rom");
    QTest::addColumn<int>("hive");
    QTest::addColumn<bool>("deletion");
    QTest::addColumn<bool>("rawAccepted");
    for (const bool deletion : {false, true}) {
      const auto row = [&](const char* name, bool rom, Fake::RegistryHive hive, bool accepted) {
        QTest::newRow(qPrintable(QStringLiteral("%1/%2").arg(deletion ? QStringLiteral("delete") : QStringLiteral("copy"), QString::fromLatin1(name))))
            << rom << static_cast<int>(hive) << deletion << accepted;
      };
      row("ordinary-unknown", false, Fake::RegistryHive::Unknown, false);
      row("ordinary-uncached", false, Fake::RegistryHive::Uncached, true);
      row("ordinary-cached", false, Fake::RegistryHive::Ready, true);
      row("rom-unknown", true, Fake::RegistryHive::Unknown, false);
      row("rom-uncached", true, Fake::RegistryHive::Uncached, false);
      row("rom-cached", true, Fake::RegistryHive::Ready, true);
    }
  }
  void cachedRegistryHiveIsNotInferredFromRom() {
    QFETCH(bool, rom);
    QFETCH(int, hive);
    QFETCH(bool, deletion);
    QFETCH(bool, rawAccepted);
    auto state = initialState(rom);
    state.runtime.registryCommitHive = static_cast<Fake::RegistryHive>(hive);
    Fake backend(state);
    backend.seedRegistryValue(key, "Value", "base");
    backend.seedRegistryValue(key, "Value", "overlay", Fake::Storage::Overlay);
    const compat::Request request = deletion ? compat::Request{compat::CommitRegistryDeletion{key, std::string("Value")}}
                                             : compat::Request{compat::CommitRegistry{key, std::string("Value")}};
    QCOMPARE(backend.executeCapturedPrimitiveForTest(request).succeeded(), rawAccepted);
    if (rom) {
      const auto publicReceipt = backend.execute(request);
      QVERIFY(publicReceipt.failure);
      QCOMPARE(publicReceipt.failure->kind, compat::ErrorKind::AccessDenied);
      QCOMPARE(publicReceipt.effect, compat::Effect::None);
    }
  }
  void powerNotificationAndOsSubmissionAreSeparate_data() {
    QTest::addColumn<bool>("active");
    QTest::addColumn<bool>("restart");
    QTest::addColumn<int>("stage");
    for (const bool active : {false, true})
      for (const bool restart : {false, true}) {
        if (active)
          QTest::newRow(qPrintable(QStringLiteral("active/%1/notification").arg(restart)))
              << active << restart << static_cast<int>(Fake::FaultTiming::AfterUwfNotification);
        QTest::newRow(qPrintable(QStringLiteral("active-%1/%2/os-rejected").arg(active).arg(restart)))
            << active << restart << static_cast<int>(Fake::FaultTiming::BeforeOsPowerRequest);
      }
  }
  void powerNotificationAndOsSubmissionAreSeparate() {
    QFETCH(bool, active);
    QFETCH(bool, restart);
    QFETCH(int, stage);
    auto state = initialState();
    state.current.filterEnabled = active;
    Fake backend(state);
    const compat::Request request = compat::Power{restart ? compat::PowerAction::Restart : compat::PowerAction::Shutdown};
    backend.inject({request, {}, static_cast<Fake::FaultTiming>(stage), compat::ErrorKind::AccessDenied, "OS privilege/request rejected", 1314});
    const auto receipt = backend.execute(request);
    QVERIFY(receipt.failure);
    QCOMPARE(receipt.failure->diagnostics.back().code, std::int64_t{1314});
    QCOMPARE(receipt.effect, compat::Effect::Uncertain);
    QCOMPARE(backend.snapshot().runtime.shutdownPending, active);
    QVERIFY(backend.snapshot().powerRequests.empty());
    QCOMPARE(backend.snapshot().restarts, std::uint32_t{0});
  }
  void injectedFailureKindsAreAlreadyPortable_data() {
    QTest::addColumn<int>("kind");
    QTest::addColumn<qint64>("code");
    QTest::addColumn<bool>("beforeInvocation");
    constexpr std::array kinds{compat::ErrorKind::Unavailable, compat::ErrorKind::Conflict, compat::ErrorKind::NotFound, compat::ErrorKind::AccessDenied};
    constexpr std::array<std::int64_t, 4> codes{static_cast<std::int32_t>(0x80070005u), static_cast<std::int32_t>(0x8007000du),
                                                static_cast<std::int32_t>(0x8007070du), 0xc0000184LL};
    for (std::size_t index = 0; index < kinds.size(); ++index)
      for (const bool before : {false, true})
        QTest::newRow(qPrintable(QStringLiteral("typed-fault-%1/before-%2").arg(index).arg(before)))
            << static_cast<int>(kinds[index]) << static_cast<qint64>(codes[index]) << before;
  }
  void injectedFailureKindsAreAlreadyPortable() {
    QFETCH(int, kind);
    QFETCH(qint64, code);
    QFETCH(bool, beforeInvocation);
    auto state = initialState();
    state.current.filterEnabled = false;
    const compat::Request request = compat::Power{compat::PowerAction::Shutdown};
    const auto timing = beforeInvocation ? Fake::FaultTiming::Before : Fake::FaultTiming::BeforeOsPowerRequest;
    Fake publicBackend(state);
    Fake capturedBackend(state);
    for (auto* backend : {&publicBackend, &capturedBackend})
      backend->inject({request, {}, timing, static_cast<compat::ErrorKind>(kind), "Typed fault diagnostic remains opaque", code});
    const auto receipt = publicBackend.execute(request);
    const auto capturedReceipt = capturedBackend.executeCapturedPrimitiveForTest(request);
    compareReceipt(receipt, {static_cast<compat::ErrorKind>(kind), beforeInvocation ? compat::Effect::None : compat::Effect::Uncertain, {}});
    compareReceipt(capturedReceipt, {static_cast<compat::ErrorKind>(kind), compat::Effect::None, {}});
    QCOMPARE(receipt.failure->diagnostics.back().code, static_cast<std::int64_t>(code));
    QCOMPARE(capturedReceipt.failure->diagnostics.back().code, static_cast<std::int64_t>(code));
    QCOMPARE(publicBackend.snapshot(), state);
    QCOMPARE(capturedBackend.snapshot(), state);
  }
  void swapfilePrimitiveKeepsNativeDiagnostic_data() {
    QTest::addColumn<qint64>("code");
    QTest::newRow("rpc-status") << qint64{1722};
    QTest::newRow("hresult-access-denied") << qint64{-2147024891};
  }
  void swapfilePrimitiveKeepsNativeDiagnostic() {
    QFETCH(qint64, code);
    auto state = initialState();
    state.current.filterEnabled = false;
    state.next.filterEnabled = false;
    const auto shared = Fake::createSystem(state);
    const auto source = std::make_shared<Fake>(shared, "Swapfile-source");
    const auto other = std::make_shared<Fake>(shared, "Must-not-replay");
    compat::FallbackUwf system({source, other});
    const compat::Request request = compat::CreateSwapfile{volumeA};
    source->inject({request, {}, Fake::FaultTiming::AfterEffect, compat::ErrorKind::Unavailable, "original swapfile error", code});
    const auto result = system.execute(request);
    QVERIFY(result.failure);
    QCOMPARE(result.effect, compat::Effect::Uncertain);
    QCOMPARE(result.failure->message, std::string("original swapfile error"));
    QVERIFY(!result.failure->diagnostics.empty());
    // Routing may append readback diagnostics; identify the original diagnostic by its fields.
    const auto original = std::ranges::find_if(result.failure->diagnostics, [code](const compat::Diagnostic& diagnostic) {
      return diagnostic.backend == "Swapfile-source" && diagnostic.operation == "CreateSwapfile" && diagnostic.code == code &&
             diagnostic.detail == "original swapfile error";
    });
    QVERIFY(original != result.failure->diagnostics.end());
    const auto sourceTrace = source->trace();
    const auto submitted = std::ranges::find_if(sourceTrace, [&request](const auto& entry) { return entry.request == request; });
    QVERIFY(submitted != sourceTrace.end());
    QVERIFY(submitted->response.failure);
    QCOMPARE(submitted->response.failure->diagnostics.back().code, static_cast<std::int64_t>(code));
    QCOMPARE(submitted->response.failure->diagnostics.back().backend, std::string("Swapfile-source"));
    QCOMPARE(source->snapshot().next.volumes.at(volumeA).swapfileSize, std::uint32_t{1});
    QVERIFY(other->trace().empty());
  }
};

QTEST_APPLESS_MAIN(UwfFakeStageContractTests)
#include "UwfFakeStageContractTests.moc"
