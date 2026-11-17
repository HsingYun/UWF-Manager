// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QTest>
#include <algorithm>
#include <array>
#include <cstddef>
#include <memory>
#include <optional>
#include <stdexcept>
#include <utility>

#include "application/configuration/execution/CompositeConfigurationRepository.h"
#include "application/configuration/execution/ConfigurationExecutor.h"
#include "application/overlay/policy/OverlayPolicyRepository.h"
#include "application/uwf/UwfRequestPrecheck.h"
#include "application/uwf/adapters/UwfConfigurationRepository.h"
#include "application/uwf/adapters/UwfMaintenanceRepositoryFactory.h"
#include "application/uwf/adapters/UwfOverlayOperations.h"
#include "application/uwf/adapters/UwfReadState.h"
#include "infrastructure/uwf/routing/FallbackUwf.h"
#include "simulation/uwf/FakeUwfBackend.h"

enum class ObjectIdentityTarget { File, RegistryKey, RegistryValue };
Q_DECLARE_METATYPE(ObjectIdentityTarget)
enum class SystemProtection { Absent, Unprotected, Protected };
Q_DECLARE_METATYPE(SystemProtection)
enum class IndividualProviderOperation { File, DeleteFile, RegistryValue, DeleteRegistryValue, DeleteRegistryKey };
Q_DECLARE_METATYPE(IndividualProviderOperation)
enum class CommitSessionMode { Ordinary, NextRomOnly, CurrentRom, FullCommit };
Q_DECLARE_METATYPE(CommitSessionMode)

namespace {
using namespace uwf;
using Fake = uwf::simulation::FakeUwfBackend;
constexpr auto volumeC = "11111111-2222-3333-4444-555555555555";
constexpr auto volumeD = "66666666-7777-8888-9999-000000000000";
constexpr auto file = "C:\\data\\sample.txt";
constexpr auto relativeFile = "\\data\\sample.txt";
constexpr auto key = "HKEY_LOCAL_MACHINE\\SOFTWARE\\UwfFake";
constexpr auto valueName = "Answer";
// Opaque future flags must survive read/modify/write without acquiring meaning.
constexpr std::uint32_t unknownFlag = 0x80;
struct IdentitySpelling {
  const char* name;
  QString stored;
  QString alternate;
  bool configuredEqual;
};
auto identitySpellings() {
  return std::array{IdentitySpelling{"ascii", QStringLiteral("Entry"), QStringLiteral("eNTRY"), true},
                    IdentitySpelling{"latin", QStringLiteral("\u00C4"), QStringLiteral("\u00E4"), false},
                    IdentitySpelling{"greek", QStringLiteral("\u03A9"), QStringLiteral("\u03C9"), false},
                    IdentitySpelling{"different-utf8-length", QStringLiteral("\u023A"), QStringLiteral("\u2C65"), false}};
}

Fake::State configured(const bool enabled = false, const std::uint32_t type = static_cast<std::uint32_t>(compat::OverlayType::Ram),
                       const std::uint32_t flags = 0) {
  Fake::State state;
  state.runtime.systemVolumeId = volumeC;
  state.current.filterEnabled = enabled;
  state.current.overlayType = type;
  state.current.overlayFlags = flags;
  state.current.volumes.emplace(volumeC, Fake::VolumeConfiguration{.volume = {volumeC, "C:", true, false, false}, .fileExclusions = {}, .swapfileSize = 0});
  state.current.volumes.emplace(volumeD, Fake::VolumeConfiguration{.volume = {volumeD, "D:", true, false, false}, .fileExclusions = {}, .swapfileSize = 0});
  state.next = state.current;
  state.capturedSettings = state.next;
  state.runtime.fileMaintenancePrefixes = {{volumeC, "C:"}, {volumeD, "D:"}};
  return state;
}

Fake::State readObservationState() {
  Fake::State state;
  state.current.filterEnabled = true;
  state.current.servicingEnabled = true;
  state.current.persistDomainSecretKey = true;
  state.current.overlayType = 0;
  state.current.maximumOverlaySize = 2048;
  state.current.warningThreshold = 111;
  state.current.criticalThreshold = 222;
  state.current.overlayFlags = 4;
  state.current.resetPersistentStateSavedMode = 0;
  state.current.volumes.emplace(
      volumeC,
      Fake::VolumeConfiguration{.volume = {volumeC, "C:", true, false, std::nullopt}, .fileExclusions = {"\\CurrentOnly", "\\Case\\Entry"}, .swapfileSize = 0});
  state.current.registryExclusions = {"HKLM\\SOFTWARE\\Current"};
  state.next.filterEnabled = false;
  state.next.persistTscal = true;
  state.next.overlayType = 1;
  state.next.maximumOverlaySize = 4096;
  state.next.warningThreshold = 333;
  state.next.criticalThreshold = 444;
  state.next.overlayFlags = 3;
  state.next.resetPersistentStateSavedMode = 2;
  state.next.volumes.emplace(volumeC,
                             Fake::VolumeConfiguration{.volume = {volumeC, "D:", false, true, true}, .fileExclusions = {"\\NextOnly"}, .swapfileSize = 1});
  state.next.registryExclusions = {"HKEY_LOCAL_MACHINE\\SOFTWARE\\Next"};
  state.runtime.administrator = false;
  state.runtime.systemVolumeId = volumeC;
  state.runtime.controlDevicePresent = true;
  state.runtime.shutdownPending = true;
  state.runtime.romDisposition = 4;
  state.runtime.resetPersistentState = 1;
  state.runtime.usageOverride = compat::OverlayUsage{7, 121};
  state.runtime.diagnosticCandidateReports[volumeC] = {{"\\Candidate.bin:stream:$DATA", 4096}, {"\\$MFT", 8192}};
  return state;
}

struct ReadObservationCase {
  std::string id;
  compat::Request request;
  compat::Value expected;
};
const std::vector<ReadObservationCase>& readObservationCases() {
  using namespace compat;
  static const auto cases = [] {
    std::vector<ReadObservationCase> result;
    struct BooleanObservation {
      const char* name;
      BooleanProperty property;
      bool current;
      bool next;
    };
    // Literal expectations distinguish session configuration from global observations.
    constexpr std::array booleans{BooleanObservation{"filter", BooleanProperty::FilterEnabled, true, false},
                                  BooleanObservation{"shutdown", BooleanProperty::ShutdownPending, true, true},
                                  BooleanObservation{"horm", BooleanProperty::HormEnabled, false, false},
                                  BooleanObservation{"servicing", BooleanProperty::ServicingEnabled, true, false},
                                  BooleanObservation{"domain", BooleanProperty::PersistDomainSecretKey, true, false},
                                  BooleanObservation{"tscal", BooleanProperty::PersistTscal, false, true},
                                  BooleanObservation{"full-commit", BooleanProperty::OverlayCommit, true, true},
                                  BooleanObservation{"registry-persistence", BooleanProperty::RegistryChangesAlwaysPersisted, false, false}};
    static_assert(booleans.size() == static_cast<std::size_t>(BooleanProperty::Count));
    for (const auto& item : booleans) {
      result.push_back({std::string(item.name) + "-current", GetBoolean{item.property, Session::Current}, item.current});
      result.push_back({std::string(item.name) + "-next", GetBoolean{item.property, Session::Next}, item.next});
    }
    struct NumberObservation {
      const char* name;
      NumberProperty property;
      std::uint32_t current;
      std::uint32_t next;
    };
    constexpr std::array numbers{NumberObservation{"type", NumberProperty::OverlayType, 0, 1},
                                 NumberObservation{"maximum", NumberProperty::MaximumOverlaySize, 2048, 4096},
                                 NumberObservation{"warning", NumberProperty::WarningThreshold, 111, 111},
                                 NumberObservation{"critical", NumberProperty::CriticalThreshold, 222, 222},
                                 NumberObservation{"flags", NumberProperty::OverlayFlags, 4, 3},
                                 NumberObservation{"runtime-reset", NumberProperty::ResetPersistentState, 1, 1},
                                 NumberObservation{"saved-reset", NumberProperty::ResetPersistentStateSavedMode, 0, 2}};
    static_assert(numbers.size() == static_cast<std::size_t>(NumberProperty::Count));
    for (const auto& item : numbers) {
      result.push_back({std::string(item.name) + "-current", GetNumber{item.property, Session::Current}, item.current});
      result.push_back({std::string(item.name) + "-next", GetNumber{item.property, Session::Next}, item.next});
    }
    result.push_back({"usage-explicit-observation", GetOverlayUsage{}, OverlayUsage{7, 121}});
    result.push_back({"volumes-current", GetVolumes{Session::Current}, std::vector<Volume>{{volumeC, "C:", true, false, std::nullopt}}});
    result.push_back({"volumes-next", GetVolumes{Session::Next}, std::vector<Volume>{{volumeC, "D:", false, true, true}}});
    result.push_back({"protection-current", GetVolumeProperty{volumeC, VolumeProperty::Protected, Session::Current}, true});
    result.push_back({"protection-next", GetVolumeProperty{volumeC, VolumeProperty::Protected, Session::Next}, false});
    result.push_back({"binding-current", GetVolumeProperty{volumeC, VolumeProperty::BindByDriveLetter, Session::Current}, false});
    result.push_back({"binding-next", GetVolumeProperty{volumeC, VolumeProperty::BindByDriveLetter, Session::Next}, true});
    result.push_back({"file-list-current", GetFileExclusions{volumeC, Session::Current}, std::vector<std::string>{"\\Case\\Entry", "\\CurrentOnly"}});
    result.push_back({"file-list-next", GetFileExclusions{volumeC, Session::Next}, std::vector<std::string>{"\\NextOnly"}});
    result.push_back({"file-find-current", FindFileExclusion{volumeC, "\\case\\entry", Session::Current}, true});
    result.push_back({"file-find-next", FindFileExclusion{volumeC, "\\case\\entry", Session::Next}, false});
    result.push_back({"registry-list-current", GetRegistryExclusions{Session::Current}, std::vector<std::string>{"HKLM\\SOFTWARE\\Current"}});
    result.push_back({"registry-list-next", GetRegistryExclusions{Session::Next}, std::vector<std::string>{"HKEY_LOCAL_MACHINE\\SOFTWARE\\Next"}});
    result.push_back({"registry-find-current", FindRegistryExclusion{"hklm\\software\\current", Session::Current}, true});
    result.push_back({"registry-find-next", FindRegistryExclusion{"hklm\\software\\current", Session::Next}, false});
    result.push_back({"diagnostic-candidates", GetOverlayFiles{volumeC}, std::vector<OverlayFile>{{"\\$MFT", 8192}, {"\\Candidate.bin:stream:$DATA", 4096}}});
    result.push_back({"swap-marker-current", GetSwapfileSize{volumeC, Session::Current}, std::uint32_t{0}});
    result.push_back({"swap-marker-next", GetSwapfileSize{volumeC, Session::Next}, std::uint32_t{1}});
    return result;
  }();
  return cases;
}

struct Scenario {
  explicit Scenario(Fake::State state) {
    const auto simulated = Fake::createSystem(std::move(state));
    // Provider views share one simulated system state.
    for (std::size_t index = 0; index < views.size(); ++index)
      views[index] = std::make_shared<Fake>(simulated, std::array{"Fake-primary", "Fake-secondary", "Fake-tertiary"}[index]);
    std::vector<std::shared_ptr<compat::Backend>> raw;
    for (const auto& view : views) raw.push_back(view->rawBackend());
    auto operations = std::make_shared<app::PrecheckedUwf>(std::make_shared<compat::FallbackUwf>(std::move(raw)));
    operations->updateState(views.front()->state());
    system = std::move(operations);
    policy = app::uwfOverlayPolicy(system);
    repository = std::make_shared<app::CompositeConfigurationRepository>(
        std::vector<std::shared_ptr<app::ConfigurationRepository>>{app::uwfConfigurationRepository(system), app::overlayPolicyRepository(policy)});
  }
  app::ConfigurationApplyResult apply(const app::ConfigurationChanges& changes) const {
    const auto snapshot = app::publishedUwfSnapshot(system->state(), app::UwfCapability::Available, true, {});
    return app::ConfigurationExecutor(repository).apply(app::planApplicationConfiguration(changes, snapshot, policy->snapshot()));
  }
  std::size_t flagWrites() const {
    std::size_t count = 0;
    for (const auto& view : views)
      for (const auto& entry : view->trace())
        if (const auto* number = std::get_if<compat::SetNumber>(&entry.request); number && number->property == compat::NumberProperty::OverlayFlags) ++count;
    return count;
  }
  std::array<std::shared_ptr<Fake>, 3> views;
  std::shared_ptr<compat::Uwf> system;
  std::shared_ptr<app::OverlayPolicyActions> policy;
  std::shared_ptr<app::ConfigurationRepository> repository;
};
std::shared_ptr<app::UwfVolumeTarget> systemVolume(const std::shared_ptr<app::UwfMaintenanceRepository>& repository) {
  const auto targets = repository->prepareVolumeTargets();
  const auto found = std::ranges::find_if(targets, [](const auto& target) { return target->volume().driveLetter == "C:"; });
  if (found == targets.end()) throw std::logic_error("The system volume was not prepared");
  return *found;
}
}  // namespace

class UwfFakeIntegrationTests final : public QObject {
  Q_OBJECT
 private slots:
  void publicReadsKeepSelectedSessionAndObservedState_data() {
    QTest::addColumn<int>("caseIndex");
    QCOMPARE(readObservationCases().size(), std::size_t{48});
    for (std::size_t index = 0; index < readObservationCases().size(); ++index)
      QTest::newRow(readObservationCases()[index].id.c_str()) << static_cast<int>(index);
  }
  void publicReadsKeepSelectedSessionAndObservedState() {
    QFETCH(int, caseIndex);
    const auto& item = readObservationCases()[static_cast<std::size_t>(caseIndex)];
    Fake backend(readObservationState());
    const auto before = backend.snapshot();
    const auto capability = backend.probe(item.request);
    QVERIFY(capability.support == compat::Support::Supported || capability.support == compat::Support::Unknown);
    const auto response = backend.execute(item.request);
    QVERIFY2(response.succeeded(), response.failure ? response.failure->message.c_str() : "Successful observation expected");
    QVERIFY(response.value == item.expected);
    QCOMPARE(response.effect, compat::Effect::None);
    QVERIFY(backend.snapshot() == before);
    const auto trace = backend.trace();
    QCOMPARE(trace.size(), std::size_t{1});
    QVERIFY(!trace.front().capturedPrimitive);
  }
  void builtInPersistenceWritesConfirmOnlyNextSetting_data() {
    QTest::addColumn<bool>("domain");
    QTest::addColumn<bool>("before");
    QTest::addColumn<bool>("desired");
    for (const bool domain : {false, true})
      for (const bool before : {false, true})
        for (const bool desired : {false, true})
          QTest::newRow(
              qUtf8Printable(QStringLiteral("%1-before%2-target%3").arg(domain ? QStringLiteral("domain") : QStringLiteral("tscal")).arg(before).arg(desired)))
              << domain << before << desired;
  }
  void builtInPersistenceWritesConfirmOnlyNextSetting() {
    QFETCH(bool, domain);
    QFETCH(bool, before);
    QFETCH(bool, desired);
    auto state = configured();
    state.current.persistDomainSecretKey = true;
    state.current.persistTscal = true;
    state.next.persistDomainSecretKey = domain ? before : !before;
    state.next.persistTscal = domain ? !before : before;
    Fake backend(state);
    auto expected = backend.snapshot();
    if (domain)
      expected.next.persistDomainSecretKey = desired;
    else
      expected.next.persistTscal = desired;
    const compat::Request request =
        compat::SetBoolean{domain ? compat::BooleanProperty::PersistDomainSecretKey : compat::BooleanProperty::PersistTscal, desired};
    const auto response = backend.execute(request);
    QVERIFY(response.succeeded());
    // These settings dispatch even when their selected scalar already matches.
    // Confirmed acknowledges that dispatch, not a changed-field count.
    QCOMPARE(response.effect, compat::Effect::Confirmed);
    QVERIFY(backend.snapshot() == expected);
    const auto dispatched = backend.primitiveRequests();
    QCOMPARE(std::ranges::count(dispatched, request), std::ptrdiff_t{1});
  }
  void scalarAndMembershipWritesHaveIndependentPostconditions_data() {
    QTest::addColumn<int>("intent");
    QTest::addColumn<bool>("currentEnabled");
    QTest::addColumn<std::uint32_t>("overlayType");
    QTest::newRow("maximum/ram") << 0 << false << std::uint32_t{0};
    QTest::newRow("maximum/disk") << 0 << false << std::uint32_t{1};
    QTest::newRow("critical/current-off") << 1 << false << std::uint32_t{0};
    QTest::newRow("critical/current-on") << 1 << true << std::uint32_t{0};
    QTest::newRow("remove/current-off") << 2 << false << std::uint32_t{0};
    QTest::newRow("remove/current-on") << 2 << true << std::uint32_t{0};
  }
  void scalarAndMembershipWritesHaveIndependentPostconditions() {
    QFETCH(int, intent);
    QFETCH(bool, currentEnabled);
    QFETCH(std::uint32_t, overlayType);
    auto state = configured(currentEnabled, overlayType);
    state.next.maximumOverlaySize = 2048;
    state.next.volumes.at(volumeC).fileExclusions = {"\\SelectedOnly"};
    state.next.volumes.at(volumeC).swapfileSize = 1;
    Fake backend(state);
    backend.seedFile(volumeC, relativeFile, "durable content");
    backend.seedFile(volumeC, relativeFile, "unrelated pending content", Fake::Storage::Overlay);
    auto expected = backend.snapshot();
    const auto request = [&]() -> compat::Request {
      if (intent == 0) {
        expected.next.maximumOverlaySize = 3072;
        return compat::SetNumber{compat::NumberProperty::MaximumOverlaySize, 3072};
      }
      if (intent == 1) {
        expected.current.criticalThreshold = 900;
        expected.next.criticalThreshold = 900;
        return compat::SetNumber{compat::NumberProperty::CriticalThreshold, 900};
      }
      expected.next.volumes.erase(volumeC);
      return compat::RemoveVolume{volumeC};
    }();
    const auto response = backend.execute(request);
    QVERIFY(response.succeeded());
    QCOMPARE(response.effect, compat::Effect::Confirmed);
    QVERIFY(response.warnings.empty());
    QVERIFY(backend.snapshot() == expected);
    const auto dispatched = backend.primitiveRequests();
    QCOMPARE(std::ranges::count(dispatched, request), std::ptrdiff_t{1});
    QCOMPARE(std::ranges::count_if(dispatched, [](const auto& primitive) { return compat::isWrite(primitive); }), std::ptrdiff_t{1});
    const auto trace = backend.trace();
    QCOMPARE(trace.size(), std::size_t{1});
    QVERIFY(trace.front().request == request);
    QVERIFY(!trace.front().capturedPrimitive);
  }
  void updateWorkflowOutcomeNeverImpliesPowerOrTransportFailure_data() {
    QTest::addColumn<int>("outcome");
    QTest::newRow("completed") << 0;
    QTest::newRow("reboot-required") << 1;
    QTest::newRow("workflow-failed-value") << 2;
    QTest::newRow("malformed-outcome") << 3;
  }
  void updateWorkflowOutcomeNeverImpliesPowerOrTransportFailure() {
    QFETCH(int, outcome);
    auto state = configured();
    state.windowsUpdateOutcome = static_cast<compat::WindowsUpdateOutcome>(outcome);
    Fake backend(state);
    auto expected = backend.snapshot();
    ++expected.windowsUpdates;
    const auto response = backend.execute(compat::UpdateWindows{});
    if (outcome < 3) {
      QVERIFY(response.succeeded());
      QVERIFY(response.value == compat::Value{static_cast<compat::WindowsUpdateOutcome>(outcome)});
      QCOMPARE(response.effect, compat::Effect::Confirmed);
    } else {
      QVERIFY(response.failure);
      QCOMPARE(response.failure->kind, compat::ErrorKind::Protocol);
      QCOMPARE(response.effect, compat::Effect::Uncertain);
    }
    QVERIFY(backend.snapshot() == expected);
    QVERIFY(backend.snapshot().powerRequests.empty());
    QCOMPARE(backend.snapshot().restarts, std::uint32_t{0});
  }
  void existingRegistryPatternsCannotBypassAddSchema_data() {
    QTest::addColumn<QString>("pattern");
    QTest::newRow("hive-root") << QStringLiteral("HKLM");
    QTest::newRow("empty-relative") << QStringLiteral("HKLM\\\\");
    QTest::newRow("other-hive") << QStringLiteral("HKCU\\Software\\Legacy");
    QTest::newRow("reserved-root") << QStringLiteral("HKLM\\SYSTEM");
    QTest::newRow("reserved-current-control-set") << QStringLiteral("HKLM\\SYSTEM\\CurrentControlSet");
    QTest::newRow("component-256") << QStringLiteral("HKLM\\") + QString(256, u'X');
    QTest::newRow("utf16-256") << QStringLiteral("HKLM\\") + QString::fromUcs4(U"\U0001f642").repeated(128);
    QTest::newRow("components-513") << QStringLiteral("HKLM\\") + QStringLiteral("X\\").repeated(512) + u'X';
  }
  void existingRegistryPatternsCannotBypassAddSchema() {
    QFETCH(QString, pattern);
    const auto original = pattern.toStdString();
    constexpr auto unrelated = "HKLM\\SOFTWARE\\Unrelated";
    auto initial = configured();
    // This is an observed legacy user-list entry, not a newly accepted Add.
    initial.next.registryExclusions = {unrelated, original};
    Fake backend(initial);
    const auto before = backend.snapshot();
    const auto result = backend.execute(compat::EditRegistryExclusion{original, compat::ExclusionAction::Add});
    QVERIFY(result.failure);
    QCOMPARE(result.failure->kind, compat::ErrorKind::InvalidArgument);
    QCOMPARE(result.effect, compat::Effect::None);
    QVERIFY(backend.snapshot() == before);
    QVERIFY(compat::call(backend, compat::FindRegistryExclusion{original, compat::Session::Next}));
    const auto removed = backend.execute(compat::EditRegistryExclusion{original, compat::ExclusionAction::Remove});
    QVERIFY(removed.succeeded());
    QCOMPARE(removed.effect, compat::Effect::Confirmed);
    QCOMPARE(compat::call(backend, compat::GetRegistryExclusions{compat::Session::Next}), std::vector<std::string>{unrelated});
    QVERIFY(backend.snapshot().current == before.current);
    backend.simulateRestart();
    QCOMPARE(compat::call(backend, compat::GetRegistryExclusions{compat::Session::Current}), std::vector<std::string>{unrelated});
  }
  void fileAddNameValidationPreservesOriginalConfiguration_data() {
    QTest::addColumn<QString>("pattern");
    QTest::addColumn<bool>("allowed");
    QTest::addColumn<bool>("existing");
    const auto add = [](const char* label, const QString& pattern, bool allowed) {
      for (const bool existing : {false, true})
        QTest::newRow(qPrintable(QStringLiteral("%1/existing-%2").arg(label).arg(existing))) << pattern << allowed << existing;
    };
    add("ordinary-260", QStringLiteral("\\") + QString(259, u'X'), true);
    add("ordinary-261", QStringLiteral("\\") + QString(260, u'X'), false);
    const auto pair = QString::fromUcs4(U"\U0001F642");
    add("utf16-260", QStringLiteral("\\") + pair.repeated(129) + u'X', true);
    add("utf16-261", QStringLiteral("\\") + pair.repeated(130), false);
    add("extended-32767", QStringLiteral("\\\\?\\") + QString(32767, u'X'), true);
    add("extended-32768", QStringLiteral("\\\\?\\") + QString(32768, u'X'), false);
    add("device-prefix-does-not-extend-limit", QStringLiteral("\\\\.\\") + QString(257, u'X'), false);
    add("control-001e", QStringLiteral("\\Before") + QChar(0x1e) + QStringLiteral("After"), false);
    add("control-001f", QStringLiteral("\\Before") + QChar(0x1f) + QStringLiteral("After"), true);
    for (const QChar character : QStringView(u"<>:\"/|?*")) {
      const auto label = QStringLiteral("forbidden-%1").arg(static_cast<quint32>(character.unicode()), 4, 16, u'0');
      add(qPrintable(label), QStringLiteral("\\Before") + character + QStringLiteral("After"), false);
    }
  }
  void fileAddNameValidationPreservesOriginalConfiguration() {
    QFETCH(QString, pattern);
    QFETCH(bool, allowed);
    QFETCH(bool, existing);
    const auto original = pattern.toStdString();
    auto initial = configured();
    auto& entries = initial.next.volumes.at(volumeC).fileExclusions;
    entries = {"\\Unrelated"};
    if (existing) entries.push_back(original);
    Fake backend(initial);
    const auto before = backend.snapshot();
    const compat::Request add = compat::EditFileExclusion{volumeC, original, compat::ExclusionAction::Add};
    const auto result = backend.execute(add);
    QCOMPARE(result.succeeded(), allowed);
    if (!allowed) {
      QVERIFY(result.failure);
      QCOMPARE(result.failure->kind, compat::ErrorKind::InvalidArgument);
      QCOMPARE(result.effect, compat::Effect::None);
      QVERIFY(backend.snapshot() == before);
    } else {
      QCOMPARE(result.effect, existing ? compat::Effect::None : compat::Effect::Confirmed);
      QCOMPARE(backend.snapshot().next.volumes.at(volumeC).fileExclusions, (std::vector<std::string>{"\\Unrelated", original}));
      QVERIFY(backend.snapshot().current == before.current);
    }
    QCOMPARE(compat::call(backend, compat::FindFileExclusion{volumeC, original, compat::Session::Next}), allowed || existing);
    const auto removed = backend.execute(compat::EditFileExclusion{volumeC, original, compat::ExclusionAction::Remove});
    QVERIFY(removed.succeeded());
    QCOMPARE(removed.effect, allowed || existing ? compat::Effect::Confirmed : compat::Effect::None);
    QCOMPARE(compat::call(backend, compat::GetFileExclusions{volumeC, compat::Session::Next}), std::vector<std::string>{"\\Unrelated"});
    QVERIFY(backend.snapshot().current == before.current);
  }
  void invalidFileAddCannotRewriteOrReuseAnObservedPattern_data() {
    QTest::addColumn<std::string>("pattern");
    QTest::addColumn<bool>("existing");
    const std::array patterns{"\\Data/Cache", "/Data/Cache", "\\Data\\Cache/"};
    for (std::size_t index = 0; index < patterns.size(); ++index)
      for (const bool existing : {false, true})
        QTest::newRow(qPrintable(QStringLiteral("pattern-%1-existing-%2").arg(index).arg(existing))) << std::string(patterns[index]) << existing;
  }
  void invalidFileAddCannotRewriteOrReuseAnObservedPattern() {
    QFETCH(std::string, pattern);
    QFETCH(bool, existing);
    auto native = pattern;
    std::replace(native.begin(), native.end(), '/', '\\');
    auto initial = configured();
    auto& exclusions = initial.next.volumes.at(volumeC).fileExclusions;
    exclusions = {native};
    if (existing) exclusions.push_back(pattern);
    Fake backend(initial);
    const std::vector<std::string> observed = existing ? std::vector<std::string>{pattern, native} : std::vector<std::string>{native};
    QCOMPARE(compat::call(backend, compat::GetFileExclusions{volumeC, compat::Session::Next}), observed);
    QCOMPARE(compat::call(backend, compat::FindFileExclusion{volumeC, pattern, compat::Session::Next}), existing);
    QVERIFY(compat::call(backend, compat::FindFileExclusion{volumeC, native, compat::Session::Next}));
    const auto before = backend.snapshot();
    backend.clearTrace();
    const compat::Request add = compat::EditFileExclusion{volumeC, pattern, compat::ExclusionAction::Add};
    const auto rejected = backend.execute(add);
    QVERIFY(rejected.failure);
    QCOMPARE(rejected.failure->kind, compat::ErrorKind::InvalidArgument);
    QCOMPARE(rejected.effect, compat::Effect::None);
    QCOMPARE(backend.snapshot(), before);
    const auto trace = backend.trace();
    QCOMPARE(trace.size(), std::size_t{1});
    QVERIFY(trace.front().request == add);
    // Find/Remove preserve the observed identity, including legacy entries rejected by Add.
    const auto removed = backend.execute(compat::EditFileExclusion{volumeC, pattern, compat::ExclusionAction::Remove});
    QVERIFY(removed.succeeded());
    QCOMPARE(removed.effect, existing ? compat::Effect::Confirmed : compat::Effect::None);
    QCOMPARE(compat::call(backend, compat::GetFileExclusions{volumeC, compat::Session::Next}), std::vector<std::string>{native});
    QVERIFY(!compat::call(backend, compat::FindFileExclusion{volumeC, pattern, compat::Session::Next}));
  }
  void registryPersistenceFollowsTheSystemVolume_data() {
    QTest::addColumn<bool>("filter");
    QTest::addColumn<SystemProtection>("systemProtection");
    QTest::addColumn<bool>("otherProtected");
    QTest::addColumn<bool>("rom");
    const std::array protections{std::pair{"absent", SystemProtection::Absent}, std::pair{"unprotected", SystemProtection::Unprotected},
                                 std::pair{"protected", SystemProtection::Protected}};
    for (const bool filter : {false, true})
      for (const auto& [name, protection] : protections)
        for (const bool otherProtected : {false, true})
          for (const bool rom : {false, true})
            QTest::newRow(qPrintable(QStringLiteral("filter-%1/system-%2/other-%3/rom-%4").arg(filter).arg(name).arg(otherProtected).arg(rom)))
                << filter << protection << otherProtected << rom;
  }
  void registryPersistenceFollowsTheSystemVolume() {
    QFETCH(bool, filter);
    QFETCH(SystemProtection, systemProtection);
    QFETCH(bool, otherProtected);
    QFETCH(bool, rom);
    auto initial = configured(filter, static_cast<std::uint32_t>(compat::OverlayType::Ram), rom ? compat::flags::readOnlyMedia : 0);
    if (systemProtection == SystemProtection::Absent)
      initial.current.volumes.erase(volumeC);
    else
      initial.current.volumes.at(volumeC).volume.protectedVolume = systemProtection == SystemProtection::Protected;
    initial.current.volumes.at(volumeD).volume.protectedVolume = otherProtected;
    // Deliberately disagree with Current; the readonly fact is not Next policy.
    initial.next.filterEnabled = !filter;
    initial.next.volumes.at(volumeC).volume.protectedVolume = systemProtection != SystemProtection::Protected;
    Fake backend(initial);
    const bool persisted = !filter || systemProtection != SystemProtection::Protected;
    const auto before = backend.snapshot();
    for (const auto session : {compat::Session::Current, compat::Session::Next}) {
      const auto response = backend.execute(compat::GetBoolean{compat::BooleanProperty::RegistryChangesAlwaysPersisted, session});
      QVERIFY(response.succeeded());
      QCOMPARE(std::get<bool>(response.value), persisted);
      QCOMPARE(response.effect, compat::Effect::None);
    }
    QCOMPARE(backend.snapshot(), before);
    backend.seedRegistryValue(key, valueName, "old");
    backend.writeRegistryValue(key, valueName, "new");
    QCOMPARE(backend.visibleRegistryValue(key, valueName), std::optional<std::string>{"new"});
    QCOMPARE(backend.readRegistryValue(key, valueName, Fake::Storage::Durable), std::optional<std::string>{persisted ? "new" : "old"});
    backend.deleteRegistryValue(key, valueName);
    QVERIFY(!backend.visibleRegistryValue(key, valueName));
    QCOMPARE(backend.readRegistryValue(key, valueName, Fake::Storage::Durable).has_value(), !persisted);
    backend.simulateRestart();
    QCOMPARE(backend.visibleRegistryValue(key, valueName), persisted ? std::nullopt : std::optional<std::string>{"old"});
  }
  void registryPersistenceRequiresAnExplicitSystemIdentity() {
    auto initial = configured(true);
    initial.runtime.systemVolumeId.reset();
    Fake backend(initial);
    const auto before = backend.snapshot();
    const auto response = backend.execute(compat::GetBoolean{compat::BooleanProperty::RegistryChangesAlwaysPersisted});
    QVERIFY(response.failure);
    QCOMPARE(response.failure->kind, compat::ErrorKind::Unavailable);
    QCOMPARE(response.effect, compat::Effect::None);
    QCOMPARE(backend.snapshot(), before);
    // Disabling the global filter supplies a sufficient fact by itself.
    initial.current.filterEnabled = false;
    backend.seed(initial);
    QVERIFY(compat::call(backend, compat::GetBoolean{compat::BooleanProperty::RegistryChangesAlwaysPersisted}));
  }
  void registryPersistenceUsesPublishedStateWithoutAuxiliaryFallback() {
    auto first = std::make_shared<Fake>(configured(true), "first");
    auto secondState = configured(false);
    secondState.runtime.systemVolumeId.reset();  // Filter OFF needs no volume fact.
    auto second = std::make_shared<Fake>(secondState, "second");
    first->inject({.request = compat::GetVolumeProperty{volumeC, compat::VolumeProperty::Protected, compat::Session::Current},
                   .operation = {},
                   .timing = Fake::FaultTiming::Before,
                   .kind = compat::ErrorKind::Unavailable,
                   .message = "Current system-volume observation failed",
                   .code = 1722});
    const auto firstBefore = first->snapshot();
    const auto response = compat::FallbackUwf({first, second}).execute(compat::GetBoolean{compat::BooleanProperty::RegistryChangesAlwaysPersisted});
    QVERIFY(response.succeeded());
    QVERIFY(!std::get<bool>(response.value));
    QCOMPARE(response.effect, compat::Effect::None);
    QCOMPARE(first->snapshot(), firstBefore);
    QCOMPARE(second->snapshot(), secondState);
    QCOMPARE(first->trace().size(), std::size_t{1});
    QVERIFY(second->trace().empty());
    QVERIFY(std::holds_alternative<compat::GetBoolean>(first->trace().front().request));
  }
  void maintenanceNullTargetsCannotCommitOrDeleteAPrefix_data() {
    QTest::addColumn<ObjectIdentityTarget>("target");
    QTest::addColumn<bool>("deletion");
    QTest::addColumn<bool>("overrideSupported");
    for (const auto& [name, target] : {std::pair{"file", ObjectIdentityTarget::File}, std::pair{"registry-key", ObjectIdentityTarget::RegistryKey},
                                       std::pair{"registry-value", ObjectIdentityTarget::RegistryValue}})
      for (const bool deletion : {false, true})
        for (const bool supported : {false, true})
          QTest::newRow(qPrintable(QStringLiteral("%1/delete-%2/override-%3").arg(QString::fromLatin1(name)).arg(deletion).arg(supported)))
              << target << deletion << supported;
  }
  void maintenanceNullTargetsCannotCommitOrDeleteAPrefix() {
    QFETCH(ObjectIdentityTarget, target);
    QFETCH(bool, deletion);
    QFETCH(bool, overrideSupported);
    const auto backend = std::make_shared<Fake>(configured(true));
    const auto selectedFile = std::string(relativeFile) + '\0' + "After";
    const auto selectedKey = target == ObjectIdentityTarget::RegistryKey ? std::string(key) + '\0' + "After" : std::string(key);
    const auto selectedValue = target == ObjectIdentityTarget::RegistryValue ? std::string(valueName) + '\0' + "After" : std::string(valueName);
    backend->seedFile(volumeC, relativeFile, "file-base");
    backend->writeFile(volumeC, relativeFile, "file-pending");
    backend->seedRegistryValue(key, valueName, "registry-base");
    backend->writeRegistryValue(key, valueName, "registry-pending");
    const auto repository = app::uwfMaintenanceRepository(backend);
    const auto volume = systemVolume(repository);
    const auto registry = repository->prepareRegistryMaintenance();
    const compat::Request request = target == ObjectIdentityTarget::File ? (deletion ? compat::Request{compat::CommitFileDeletion{volumeC, selectedFile}}
                                                                                     : compat::Request{compat::CommitFile{volumeC, selectedFile}})
                                    : deletion                           ? compat::Request{compat::CommitRegistryDeletion{selectedKey, selectedValue}}
                                                                         : compat::Request{compat::CommitRegistry{selectedKey, selectedValue}};
    if (overrideSupported) backend->setCapability(request, {compat::Support::Supported, "Availability cannot change exact target semantics"});
    QCOMPARE(backend->probe(request).support, compat::Support::Unsupported);
    const std::string sentinel = "The prefix dispatch must remain pending";
    backend->inject({{}, compat::operationName(request), Fake::FaultTiming::Before, compat::ErrorKind::Failed, sentinel});
    backend->clearTrace();
    const auto before = backend->snapshot();
    bool rejected = false;
    try {
      if (target == ObjectIdentityTarget::File) {
        if (deletion)
          volume->commitFileDeletion(selectedFile);
        else
          volume->commitFile(selectedFile);
      } else if (deletion)
        registry->commitDeletion(selectedKey, selectedValue);
      else
        registry->commit(selectedKey, selectedValue);
    } catch (const app::OperationError& error) {
      rejected = true;
      QCOMPARE(error.failureKind(), app::OperationFailureKind::Unsupported);
      QCOMPARE(error.writeEffect(), app::WriteEffect::None);
    }
    QVERIFY(rejected);
    QVERIFY(backend->snapshot() == before);
    const compat::Request prefix = target == ObjectIdentityTarget::File ? (deletion ? compat::Request{compat::CommitFileDeletion{volumeC, relativeFile}}
                                                                                    : compat::Request{compat::CommitFile{volumeC, relativeFile}})
                                   : deletion                           ? compat::Request{compat::CommitRegistryDeletion{key, valueName}}
                                                                        : compat::Request{compat::CommitRegistry{key, valueName}};
    const auto pending = backend->execute(prefix);
    QVERIFY(pending.failure);
    QCOMPARE(pending.failure->kind, compat::ErrorKind::Failed);
    QCOMPARE(pending.failure->message, sentinel);
    QCOMPARE(pending.effect, compat::Effect::None);
    QVERIFY(backend->snapshot() == before);
    std::vector<Fake::TraceEntry> writes;
    for (const auto& entry : backend->trace())
      if (compat::isWrite(entry.request)) writes.push_back(entry);
    QCOMPARE(writes.size(), std::size_t{2});
    QVERIFY(writes.front().request == request);
    QCOMPARE(writes.front().response.failure->kind, compat::ErrorKind::Unsupported);
    QVERIFY(writes.back().request == prefix);
  }

  void compositeBeforeFaultsApplyToKnownAndUnknownCapabilities_data() {
    QTest::addColumn<bool>("unknown");
    QTest::addColumn<bool>("swapfile");
    for (const bool unknown : {false, true})
      for (const bool swapfile : {false, true})
        QTest::newRow(qPrintable(QStringLiteral("unknown-%1/swapfile-%2").arg(unknown).arg(swapfile))) << unknown << swapfile;
  }
  void compositeBeforeFaultsApplyToKnownAndUnknownCapabilities() {
    QFETCH(bool, unknown);
    QFETCH(bool, swapfile);
    Fake backend(configured());
    const compat::Request request = swapfile ? compat::Request{compat::CreateSwapfile{volumeD, "D:", true}}
                                             : compat::Request{compat::SetOverlayOption{compat::OverlayOption::Passthrough, true}};
    backend.setCapability(request, {unknown ? compat::Support::Unknown : compat::Support::Supported, "Explicit capability observation"});
    const auto before = backend.snapshot();
    const std::string sentinel = "The public composite failed before dispatch";
    backend.inject({request, {}, Fake::FaultTiming::Before, compat::ErrorKind::Failed, sentinel});
    const auto failed = backend.execute(request);
    QVERIFY(failed.failure);
    QCOMPARE(failed.failure->kind, compat::ErrorKind::Failed);
    QCOMPARE(failed.failure->message, sentinel);
    QCOMPARE(failed.effect, compat::Effect::None);
    QVERIFY(backend.snapshot() == before);
    const auto completed = backend.execute(request);
    QVERIFY2(completed.succeeded(), completed.failure ? completed.failure->message.c_str() : "");
    QCOMPARE(completed.effect, compat::Effect::Confirmed);
    const auto after = backend.snapshot();
    QCOMPARE(after.next.overlayFlags & compat::flags::passthrough, compat::flags::passthrough);
    QCOMPARE(after.next.overlayType, static_cast<std::uint32_t>(swapfile ? compat::OverlayType::Disk : compat::OverlayType::Ram));
    QCOMPARE(after.next.volumes.at(volumeD).swapfileSize, swapfile ? std::uint32_t{1} : std::uint32_t{0});
    QVERIFY(after.current == before.current);
    const auto trace = backend.trace();
    QCOMPARE(trace.size(), std::size_t{2});
    QVERIFY(trace.front().request == request);
    QVERIFY(trace.back().request == request);
  }

  void unicodeObjectIdentityPreservesExactCommitTargets_data() {
    QTest::addColumn<ObjectIdentityTarget>("target");
    QTest::addColumn<QString>("stored");
    QTest::addColumn<QString>("alternate");
    for (const auto& [name, target] : {std::pair{"file", ObjectIdentityTarget::File}, std::pair{"registry-key", ObjectIdentityTarget::RegistryKey},
                                       std::pair{"registry-value", ObjectIdentityTarget::RegistryValue}})
      for (const auto& spelling : identitySpellings()) {
        if (spelling.configuredEqual) continue;  // ASCII identity already has a separate baseline below.
        QTest::newRow(qPrintable(QStringLiteral("%1/%2").arg(QString::fromLatin1(name), QString::fromLatin1(spelling.name))))
            << target << spelling.stored << spelling.alternate;
      }
  }
  void unicodeObjectIdentityPreservesExactCommitTargets() {
    QFETCH(ObjectIdentityTarget, target);
    QFETCH(QString, stored);
    QFETCH(QString, alternate);
    Fake backend(configured(true));
    if (target == ObjectIdentityTarget::File) {
      const auto original = "\\Data\\" + stored.toStdString() + "\\Item.txt";
      const auto query = "\\data\\" + alternate.toStdString() + "\\item.TXT";
      backend.seedFile(volumeC, original, "base");
      backend.seedFile(volumeC, original + ".sibling", "unrelated", Fake::Storage::Overlay);
      backend.writeFile(volumeC, query, "changed");
      QCOMPARE(backend.visibleFile(volumeC, original), std::optional<std::string>{"changed"});
      (void)compat::call(backend, compat::CommitFile{volumeC, query});
      QCOMPARE(backend.readFile(volumeC, original), std::optional<std::string>{"changed"});
      QVERIFY(!backend.readFile(volumeC, original, Fake::Storage::Overlay));
      QCOMPARE(backend.readFile(volumeC, original + ".sibling", Fake::Storage::Overlay), std::optional<std::string>{"unrelated"});
    } else {
      const auto originalKey = target == ObjectIdentityTarget::RegistryKey ? std::string(key) + "\\" + stored.toStdString() : std::string(key);
      const auto queryKey = target == ObjectIdentityTarget::RegistryKey ? std::string(key) + "\\" + alternate.toStdString() : std::string(key);
      const auto originalValue = target == ObjectIdentityTarget::RegistryValue ? stored.toStdString() : std::string(valueName);
      const auto queryValue = target == ObjectIdentityTarget::RegistryValue ? alternate.toStdString() : std::string(valueName);
      backend.seedRegistryValue(originalKey, originalValue, "base");
      backend.seedRegistryValue(originalKey, originalValue + ".sibling", "unrelated", Fake::Storage::Overlay);
      backend.writeRegistryValue(queryKey, queryValue, "changed");
      QCOMPARE(backend.visibleRegistryValue(originalKey, originalValue), std::optional<std::string>{"changed"});
      (void)compat::call(backend, compat::CommitRegistry{queryKey, queryValue});
      QCOMPARE(backend.readRegistryValue(originalKey, originalValue), std::optional<std::string>{"changed"});
      QVERIFY(!backend.readRegistryValue(originalKey, originalValue, Fake::Storage::Overlay));
      QCOMPARE(backend.readRegistryValue(originalKey, originalValue + ".sibling", Fake::Storage::Overlay), std::optional<std::string>{"unrelated"});
    }
  }

  void managementExclusionIdentityDiffersFromDriverCoverage_data() {
    QTest::addColumn<bool>("registry");
    QTest::addColumn<QString>("stored");
    QTest::addColumn<QString>("alternate");
    QTest::addColumn<bool>("configuredEqual");
    QTest::addColumn<bool>("managementCovered");
    QTest::addColumn<bool>("driverCovered");
    for (const bool registry : {false, true})
      for (const auto& spelling : identitySpellings()) {
        const auto root = registry ? QString::fromLatin1(key) + u'\\' : QStringLiteral("\\Data\\");
        QTest::newRow(
            qPrintable(QStringLiteral("%1/%2").arg(registry ? QStringLiteral("registry") : QStringLiteral("file"), QString::fromLatin1(spelling.name))))
            << registry << root + spelling.stored << root + spelling.alternate << spelling.configuredEqual << spelling.configuredEqual << true;
      }
    for (const bool reverse : {false, true}) {
      const auto shortHive = QStringLiteral("HKLM\\SOFTWARE\\AliasIdentity");
      const auto longHive = QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\AliasIdentity");
      QTest::newRow(reverse ? "registry/long-to-short-hive" : "registry/short-to-long-hive")
          << true << (reverse ? longHive : shortHive) << (reverse ? shortHive : longHive) << false << true << true;
      const auto path = QStringLiteral("\\Data\\TrailingIdentity");
      QTest::newRow(reverse ? "file/without-to-with-trailing-separator" : "file/with-to-without-trailing-separator")
          << false << (reverse ? path : path + u'\\') << (reverse ? path + u'\\' : path) << false << true << reverse;
    }
    for (const bool registry : {false, true}) {
      const auto path = registry ? QStringLiteral("HKLM\\SOFTWARE\\TrailingIdentity") : QStringLiteral("\\Data\\TrailingIdentity");
      QTest::newRow(registry ? "registry/double-tail" : "file/double-tail") << registry << path + QStringLiteral("\\\\") << path << false << false << false;
      if (registry) QTest::newRow("registry/single-tail") << true << path + u'\\' << path << false << true << false;
    }
  }
  void managementExclusionIdentityDiffersFromDriverCoverage() {
    QFETCH(bool, registry);
    QFETCH(QString, stored);
    QFETCH(QString, alternate);
    QFETCH(bool, configuredEqual);
    QFETCH(bool, managementCovered);
    QFETCH(bool, driverCovered);
    const auto original = stored.toStdString();
    const auto query = alternate.toStdString();
    auto initial = configured(true);
    if (registry)
      initial.current.registryExclusions = initial.next.registryExclusions = {original};
    else
      initial.current.volumes.at(volumeC).fileExclusions = initial.next.volumes.at(volumeC).fileExclusions = {original};
    Fake backend(initial);
    const compat::Request find = registry ? compat::Request{compat::FindRegistryExclusion{query, compat::Session::Next}}
                                          : compat::Request{compat::FindFileExclusion{volumeC, query, compat::Session::Next}};
    auto found = backend.execute(find);
    QVERIFY(found.succeeded());
    QCOMPARE(std::get<bool>(found.value), configuredEqual);
    const compat::Request add = registry ? compat::Request{compat::EditRegistryExclusion{query, compat::ExclusionAction::Add}}
                                         : compat::Request{compat::EditFileExclusion{volumeC, query, compat::ExclusionAction::Add}};
    const auto added = backend.execute(add);
    QVERIFY(added.succeeded());
    QCOMPARE(added.effect, configuredEqual ? compat::Effect::None : compat::Effect::Confirmed);
    const auto afterAdd = backend.snapshot();
    QCOMPARE(registry ? afterAdd.next.registryExclusions.size() : afterAdd.next.volumes.at(volumeC).fileExclusions.size(),
             configuredEqual ? std::size_t{1} : std::size_t{2});
    const compat::Request remove = registry ? compat::Request{compat::EditRegistryExclusion{query, compat::ExclusionAction::Remove}}
                                            : compat::Request{compat::EditFileExclusion{volumeC, query, compat::ExclusionAction::Remove}};
    QVERIFY(backend.execute(remove).succeeded());
    const auto afterRemove = backend.snapshot();
    QCOMPARE(registry ? afterRemove.next.registryExclusions.size() : afterRemove.next.volumes.at(volumeC).fileExclusions.size(),
             configuredEqual ? std::size_t{0} : std::size_t{1});
    const auto list = registry ? compat::Request{compat::GetRegistryExclusions{compat::Session::Next}}
                               : compat::Request{compat::GetFileExclusions{volumeC, compat::Session::Next}};
    QCOMPARE(std::get<std::vector<std::string>>(backend.execute(list).value),
             configuredEqual ? std::vector<std::string>{} : std::vector<std::string>{original});
    QCOMPARE(std::get<bool>(backend.execute(find).value), false);

    const auto objectRoot = [](std::string path) {
      while (path.ends_with('\\')) path.pop_back();
      return path;
    };
    const auto originalRoot = objectRoot(original);
    const auto queryRoot = objectRoot(query);

    // Current driver coverage uses Unicode identity and component boundaries,
    // independently of those Next-session management-list operations.
    if (registry) {
      backend.seedRegistryValue(originalRoot + "\\Child", valueName, "base");
      backend.seedRegistryValue(queryRoot + "Sibling\\Child", valueName, "base");
      backend.writeRegistryValue(queryRoot + "\\child", valueName, "persisted");
      backend.writeRegistryValue(queryRoot + "Sibling\\Child", valueName, "pending");
      QCOMPARE(backend.readRegistryValue(originalRoot + "\\Child", valueName), std::optional<std::string>{driverCovered ? "persisted" : "base"});
      QCOMPARE(backend.readRegistryValue(queryRoot + "\\child", valueName, Fake::Storage::Overlay),
               driverCovered ? std::optional<std::string>{} : std::optional<std::string>{"persisted"});
      QCOMPARE(backend.readRegistryValue(queryRoot + "Sibling\\Child", valueName), std::optional<std::string>{"base"});
      QCOMPARE(backend.readRegistryValue(queryRoot + "Sibling\\Child", valueName, Fake::Storage::Overlay), std::optional<std::string>{"pending"});
    } else {
      backend.seedFile(volumeC, originalRoot + "\\Child.txt", "base");
      backend.seedFile(volumeC, queryRoot + "Sibling\\Child.txt", "base");
      backend.writeFile(volumeC, queryRoot + "\\child.TXT", "persisted");
      backend.writeFile(volumeC, queryRoot + "Sibling\\Child.txt", "pending");
      QCOMPARE(backend.readFile(volumeC, originalRoot + "\\Child.txt"), std::optional<std::string>{driverCovered ? "persisted" : "base"});
      QCOMPARE(backend.readFile(volumeC, queryRoot + "\\child.TXT", Fake::Storage::Overlay),
               driverCovered ? std::optional<std::string>{} : std::optional<std::string>{"persisted"});
      QCOMPARE(backend.readFile(volumeC, queryRoot + "Sibling\\Child.txt"), std::optional<std::string>{"base"});
      QCOMPARE(backend.readFile(volumeC, queryRoot + "Sibling\\Child.txt", Fake::Storage::Overlay), std::optional<std::string>{"pending"});
    }

    // ROM ignores driver exclusions, while the management precheck may still skip an ASCII-matching entry.
    auto romState = initial;
    romState.current.overlayFlags = compat::flags::readOnlyMedia;
    // The filesystem backing view is an independent fact, not implied by the ROM flag.
    romState.runtime.defaultFileCommitView = Fake::PersistentView::NotReady;
    Fake rom(std::move(romState));
    if (registry) {
      rom.seedRegistryValue(originalRoot + "\\Child", valueName, "base");
      rom.writeRegistryValue(queryRoot + "\\child", valueName, "pending");
      const compat::Request request = compat::CommitRegistry{queryRoot + "\\child", valueName};
      const auto publicReceipt = rom.execute(request);
      QVERIFY(publicReceipt.failure);
      QCOMPARE(publicReceipt.failure->kind, compat::ErrorKind::AccessDenied);
      QCOMPARE(publicReceipt.effect, compat::Effect::None);
      QCOMPARE(rom.executeCapturedPrimitiveForTest(request).succeeded(), managementCovered);
      QCOMPARE(rom.readRegistryValue(originalRoot + "\\Child", valueName), std::optional<std::string>{"base"});
    } else {
      rom.seedFile(volumeC, originalRoot + "\\Child.txt", "base");
      rom.writeFile(volumeC, queryRoot + "\\child.TXT", "pending");
      const compat::Request request = compat::CommitFile{volumeC, queryRoot + "\\child.TXT"};
      const auto publicReceipt = rom.execute(request);
      QVERIFY(publicReceipt.failure);
      QCOMPARE(publicReceipt.failure->kind, compat::ErrorKind::AccessDenied);
      QCOMPARE(publicReceipt.effect, compat::Effect::None);
      QCOMPARE(rom.executeCapturedPrimitiveForTest(request).succeeded(), managementCovered);
      QCOMPARE(rom.readFile(volumeC, originalRoot + "\\Child.txt"), std::optional<std::string>{"base"});
    }
  }

  void exactCommitsKeepCaseInsensitiveIdentityAndUnrelatedPendingChanges() {
    Fake backend(configured(true));
    backend.seedFile(volumeC, relativeFile, "base");
    backend.writeFile(volumeC, "\\DATA\\SAMPLE.TXT", "changed");
    backend.seedFile(volumeD, "\\other.bin", "other-change", Fake::Storage::Overlay);
    backend.seedRegistryValue(key, valueName, "base");
    backend.writeRegistryValue(key, "ANSWER", "changed");
    backend.seedRegistryValue(key, "Sibling", "sibling-change", Fake::Storage::Overlay);

    (void)compat::call(backend, compat::CommitFile{volumeC, "\\Data\\Sample.txt"});
    (void)compat::call(backend, compat::CommitRegistry{"hklm\\software\\uwffake", "answer"});

    QCOMPARE(backend.readFile(volumeC, relativeFile), std::optional<std::string>{"changed"});
    QVERIFY(!backend.readFile(volumeC, relativeFile, Fake::Storage::Overlay));
    QCOMPARE(backend.readFile(volumeD, "\\OTHER.BIN", Fake::Storage::Overlay), std::optional<std::string>{"other-change"});
    QCOMPARE(backend.readRegistryValue(key, valueName), std::optional<std::string>{"changed"});
    QVERIFY(!backend.readRegistryValue(key, valueName, Fake::Storage::Overlay));
    QCOMPARE(backend.readRegistryValue(key, "sibling", Fake::Storage::Overlay), std::optional<std::string>{"sibling-change"});
    const auto observed = backend.snapshot();
    QCOMPARE(observed.current.volumes.at(volumeC).volume.commitPending, std::optional<bool>{false});
    QCOMPARE(observed.current.volumes.at(volumeD).volume.commitPending, std::optional<bool>{true});
  }

  void configurationUsesFreshBackendState_data() {
    QTest::addColumn<bool>("toDisk");
    QTest::addColumn<bool>("explicitFlags");
    QTest::newRow("ram-to-disk-provider-default") << true << false;
    QTest::newRow("ram-to-disk-explicit-passthrough-off") << true << true;
    QTest::newRow("disk-to-ram-provider-default") << false << false;
    QTest::newRow("persistent-disk-to-ram-rom") << false << true;
  }
  void configurationUsesFreshBackendState() {
    QFETCH(bool, toDisk);
    QFETCH(bool, explicitFlags);
    const auto initialFlags = unknownFlag | (toDisk ? 0 : compat::flags::passthrough | (explicitFlags ? compat::flags::persistent : 0));
    Scenario scenario(configured(false, static_cast<std::uint32_t>(toDisk ? compat::OverlayType::Ram : compat::OverlayType::Disk), initialFlags));
    // One simulated OS is visible through all providers, while a missing WMI
    // operation routes only the corresponding request to another provider.
    scenario.views[0]->setOperationCapability(compat::operationName(compat::GetNumber{compat::NumberProperty::OverlayFlags}),
                                              {compat::Support::Unsupported, "This provider has no overlay flag entry point"});
    scenario.views[0]->setOperationCapability(compat::operationName(compat::SetNumber{compat::NumberProperty::OverlayFlags, 0}),
                                              {compat::Support::Unsupported, "This provider has no overlay flag entry point"});
    app::ConfigurationChanges changes;
    changes.settings.setOverlay.type = toDisk ? core::OverlayType::Disk : core::OverlayType::RAM;
    if (explicitFlags) {
      if (toDisk)
        changes.overlayPolicy.passthrough = false;
      else {
        changes.overlayPolicy.persistent = false;
        changes.overlayPolicy.readOnlyMedia = true;
      }
    }
    const auto result = scenario.apply(changes);
    const auto detail = std::ranges::find_if(result.steps, [](const auto& step) { return !step.description.empty(); });
    QVERIFY2(!result.failed(), detail == result.steps.end() ? "Configuration failed" : detail->description.c_str());
    const auto expected = unknownFlag | (toDisk ? (explicitFlags ? 0 : compat::flags::passthrough) : (explicitFlags ? compat::flags::readOnlyMedia : 0));
    for (const auto& view : scenario.views) {
      const auto observedState = view->snapshot();
      QCOMPARE(observedState.next.overlayFlags, expected);
      QCOMPARE(observedState.current.overlayFlags, initialFlags);
      QCOMPARE(observedState.current.overlayType, static_cast<std::uint32_t>(toDisk ? compat::OverlayType::Ram : compat::OverlayType::Disk));
    }
    QCOMPARE(scenario.flagWrites(), explicitFlags ? (toDisk ? std::size_t{1} : std::size_t{2}) : std::size_t{0});
    QVERIFY(result.observedOverlayPolicy);
    QVERIFY(result.observedOverlayPolicy->next.flags.value);
    QCOMPARE(result.observedOverlayPolicy->next.flags.value->raw, expected);
    const auto observed = app::publishedUwfSnapshot(scenario.system->state(), app::UwfCapability::Available, true, {});
    QCOMPARE(observed.next.overlay.type, toDisk ? core::OverlayType::Disk : core::OverlayType::RAM);
    QCOMPARE(scenario.views[1]->trace().empty(), !explicitFlags);
    for (const auto& view : scenario.views)
      for (const auto& entry : view->trace())
        if (const auto* number = std::get_if<compat::GetNumber>(&entry.request)) QVERIFY(number->property != compat::NumberProperty::OverlayFlags);
  }

  void registeredVolumesHaveNativeCreationDefaultsAndRejectDuplicates() {
    Fake backend;
    QVERIFY(backend.execute(compat::AddVolume{volumeC, {}}).succeeded());
    const auto after = backend.snapshot();
    const auto& added = after.next.volumes.at(volumeC).volume;
    QVERIFY(!added.protectedVolume);
    QVERIFY(!added.bindByDriveLetter);
    QVERIFY(added.driveLetter.empty());
    QVERIFY(after.current.volumes.empty());
    const auto duplicate = backend.execute(compat::AddVolume{volumeC, "C:"});
    QCOMPARE(duplicate.failure->kind, compat::ErrorKind::Conflict);
    QCOMPARE(duplicate.effect, compat::Effect::None);
    QVERIFY(backend.snapshot() == after);
  }

  void overlayTypeEffectsDistinguishRealTransitionsFromSameTypeCommands_data() {
    QTest::addColumn<bool>("fromDisk");
    QTest::addColumn<bool>("toDisk");
    for (const bool fromDisk : {false, true})
      for (const bool toDisk : {false, true})
        QTest::newRow(qUtf8Printable(QStringLiteral("%1-to-%2").arg(fromDisk ? "disk" : "ram", toDisk ? "disk" : "ram"))) << fromDisk << toDisk;
  }
  void overlayTypeEffectsDistinguishRealTransitionsFromSameTypeCommands() {
    QFETCH(bool, fromDisk);
    QFETCH(bool, toDisk);
    const auto initialFlags = unknownFlag | compat::flags::persistent | compat::flags::readOnlyMedia | (fromDisk ? 0 : compat::flags::passthrough);
    auto initial = configured(false, static_cast<std::uint32_t>(fromDisk ? compat::OverlayType::Disk : compat::OverlayType::Ram), initialFlags);
    initial.next.volumes.at(volumeC).swapfileSize = 1;
    Fake backend(initial);
    const auto response = backend.execute(
        compat::SetNumber{compat::NumberProperty::OverlayType, static_cast<std::uint32_t>(toDisk ? compat::OverlayType::Disk : compat::OverlayType::Ram)});
    QVERIFY(response.succeeded());
    const auto after = backend.snapshot();
    const auto expectedFlags = fromDisk == toDisk ? initialFlags
                               : toDisk           ? initialFlags | compat::flags::passthrough
                                                  : initialFlags & ~(compat::flags::passthrough | compat::flags::persistent);
    QCOMPARE(after.next.overlayFlags, expectedFlags);
    QCOMPARE(after.next.overlayType, static_cast<std::uint32_t>(toDisk ? compat::OverlayType::Disk : compat::OverlayType::Ram));
    // Type changes do not rewrite the configured swapfile host marker. That is
    // a separate configuration field, despite backing-file lifecycle changes.
    QCOMPARE(after.next.volumes.at(volumeC).swapfileSize, std::uint32_t{1});
    QVERIFY(after.current == initial.current);
  }

  void swapfileReselectionInheritsOnlyActualTypeTransitionEffects_data() {
    QTest::addColumn<bool>("fromDisk");
    QTest::addColumn<bool>("sameHost");
    for (const bool fromDisk : {false, true})
      for (const bool sameHost : {false, true})
        QTest::newRow(qUtf8Printable(QStringLiteral("%1-%2-host").arg(fromDisk ? "disk" : "ram", sameHost ? "same" : "other"))) << fromDisk << sameHost;
  }
  void swapfileReselectionInheritsOnlyActualTypeTransitionEffects() {
    QFETCH(bool, fromDisk);
    QFETCH(bool, sameHost);
    const auto initialFlags = unknownFlag | compat::flags::persistent | compat::flags::readOnlyMedia;
    auto initial = configured(false, static_cast<std::uint32_t>(fromDisk ? compat::OverlayType::Disk : compat::OverlayType::Ram), initialFlags);
    initial.next.volumes.at(volumeC).swapfileSize = 1;
    Fake backend(initial);
    const auto target = sameHost ? volumeC : volumeD;
    const auto response = backend.execute(compat::CreateSwapfile{target});
    QVERIFY(response.succeeded());
    const auto after = backend.snapshot();
    QCOMPARE(after.next.overlayType, static_cast<std::uint32_t>(compat::OverlayType::Disk));
    QCOMPARE(after.next.overlayFlags, (initialFlags & ~(fromDisk ? compat::flags::persistent : 0)) | compat::flags::passthrough);
    QCOMPARE(after.next.volumes.at(target).swapfileSize, std::uint32_t{1});
    QCOMPARE(after.next.volumes.at(sameHost ? volumeD : volumeC).swapfileSize, std::uint32_t{0});
    QVERIFY(after.current == initial.current);
  }

  void persistentResetKeepsCapturedDeviceAndNoDeviceBranchesDistinct_data() {
    QTest::addColumn<bool>("controlPresent");
    QTest::addColumn<bool>("nextDisk");
    QTest::addColumn<std::uint32_t>("value");
    QTest::addColumn<std::uint32_t>("currentSaved");
    QTest::addColumn<std::uint32_t>("nextSaved");
    for (const bool controlPresent : {false, true})
      for (const bool nextDisk : {false, true})
        for (const auto value : {0u, 1u, 2u, 255u})
          QTest::newRow(qUtf8Printable(QStringLiteral("%1-next%2-value%3").arg(controlPresent ? "device" : "no-device", nextDisk ? "disk" : "ram").arg(value)))
              << controlPresent << nextDisk << value << 1u << 1u;
    for (const bool controlPresent : {false, true})
      for (const bool nextDisk : {false, true})
        for (const auto [currentSaved, nextSaved] : std::array{std::pair{0u, 1u}, std::pair{1u, 0u}, std::pair{2u, 1u}})
          QTest::newRow(qUtf8Printable(QStringLiteral("%1-next%2-saved-current%3-next%4")
                                           .arg(controlPresent ? "device" : "no-device", nextDisk ? "disk" : "ram")
                                           .arg(currentSaved)
                                           .arg(nextSaved)))
              << controlPresent << nextDisk << 255u << currentSaved << nextSaved;
  }
  void persistentResetKeepsCapturedDeviceAndNoDeviceBranchesDistinct() {
    QFETCH(bool, controlPresent);
    QFETCH(bool, nextDisk);
    QFETCH(std::uint32_t, value);
    QFETCH(std::uint32_t, currentSaved);
    QFETCH(std::uint32_t, nextSaved);
    auto initial = configured(true, static_cast<std::uint32_t>(compat::OverlayType::Disk), unknownFlag | compat::flags::persistent);
    initial.next.overlayType = static_cast<std::uint32_t>(nextDisk ? compat::OverlayType::Disk : compat::OverlayType::Ram);
    initial.current.resetPersistentStateSavedMode = currentSaved;
    initial.next.resetPersistentStateSavedMode = nextSaved;
    initial.runtime.controlDevicePresent = controlPresent;
    initial.runtime.persistentRollbackState = 2;
    Fake backend(initial);
    const auto response = backend.execute(compat::SetNumber{compat::NumberProperty::ResetPersistentState, value});
    const auto resolved = value == 255 ? currentSaved : value;
    const bool invalidPublicSelector = value == 2 || (controlPresent && value == 255 && currentSaved > 1);
    const bool accepted = controlPresent && !invalidPublicSelector;
    QCOMPARE(response.succeeded(), accepted);
    if (!accepted) {
      QCOMPARE(response.failure->kind, invalidPublicSelector ? compat::ErrorKind::InvalidArgument : compat::ErrorKind::Unavailable);
      QCOMPARE(response.effect, controlPresent && value == 255 && currentSaved > 1 ? compat::Effect::Uncertain : compat::Effect::None);
      QCOMPARE(backend.snapshot(), initial);
      const auto primitives = backend.primitiveRequests();
      const compat::Request reset = compat::SetNumber{compat::NumberProperty::ResetPersistentState, value};
      const bool rejectedByRawSavedResolution = controlPresent && value == 255 && currentSaved > 1;
      QCOMPARE(std::ranges::count(primitives, reset), rejectedByRawSavedResolution ? std::ptrdiff_t{1} : std::ptrdiff_t{0});
      QVERIFY(std::ranges::none_of(primitives, [&](const auto& primitive) { return compat::isWrite(primitive) && primitive != reset; }));
    }
    const auto after = backend.snapshot();
    QVERIFY(after.current == initial.current);
    QCOMPARE(after.next.overlayType, initial.next.overlayType);
    QCOMPARE(after.next.overlayFlags, initial.next.overlayFlags);
    QCOMPARE(after.current.resetPersistentStateSavedMode, currentSaved);
    QCOMPARE(after.next.resetPersistentStateSavedMode, nextSaved);
    QCOMPARE(compat::call(backend, compat::GetNumber{compat::NumberProperty::ResetPersistentState}), controlPresent && !invalidPublicSelector ? resolved : 0u);
    QCOMPARE(after.runtime.persistentRollbackState, controlPresent && !invalidPublicSelector ? resolved : 2u);
    // Retain captured absent-device behavior for raw selector 2 and Saved resolving to 2, independently of public validation.
    Fake raw(initial);
    const auto rawReceipt = raw.executeCapturedPrimitiveForTest(compat::SetNumber{compat::NumberProperty::ResetPersistentState, value});
    QCOMPARE(rawReceipt.succeeded(), !controlPresent || resolved < 2);
    if (controlPresent && resolved == 2) {
      QCOMPARE(rawReceipt.failure->kind, compat::ErrorKind::InvalidArgument);
      QCOMPARE(rawReceipt.effect, compat::Effect::None);
    }
    QCOMPARE(raw.snapshot().next.overlayFlags, initial.next.overlayFlags | (!controlPresent && nextDisk ? compat::flags::passthrough : 0));
  }

  void savedRuntimeResetDoesNotRequirePublicSavedModeGetter_data() {
    QTest::addColumn<int>("savedGetterSupport");
    QTest::addColumn<std::uint32_t>("currentSaved");
    for (const auto support : {compat::Support::Supported, compat::Support::Unsupported, compat::Support::Unavailable})
      for (const auto currentSaved : {0u, 1u, 2u, 0xffffffffu})
        QTest::newRow(qUtf8Printable(QStringLiteral("saved-getter-%1-current%2").arg(static_cast<int>(support)).arg(currentSaved)))
            << static_cast<int>(support) << currentSaved;
  }
  void savedRuntimeResetDoesNotRequirePublicSavedModeGetter() {
    QFETCH(int, savedGetterSupport);
    QFETCH(std::uint32_t, currentSaved);
    auto initial = configured(true, static_cast<std::uint32_t>(compat::OverlayType::Disk), compat::flags::persistent);
    initial.current.resetPersistentStateSavedMode = currentSaved;
    initial.next.resetPersistentStateSavedMode = currentSaved == 0 ? 1u : 0u;
    initial.runtime.resetPersistentState = currentSaved == 0 ? 1u : 0u;
    initial.runtime.persistentRollbackState = 2;
    const compat::Request getter = compat::GetNumber{compat::NumberProperty::ResetPersistentStateSavedMode, compat::Session::Current};
    const compat::Request setter = compat::SetNumber{compat::NumberProperty::ResetPersistentState, 255};
    const auto support = static_cast<compat::Support>(savedGetterSupport);
    Fake backend(initial);
    backend.setCapability(getter, {support, "Independent public SavedMode getter capability"});
    const auto read = backend.execute(getter);
    QCOMPARE(read.succeeded(), support == compat::Support::Supported);
    if (read.failure)
      QCOMPARE(read.failure->kind, support == compat::Support::Unsupported ? compat::ErrorKind::Unsupported : compat::ErrorKind::Unavailable);
    else
      QCOMPARE(std::get<std::uint32_t>(read.value), currentSaved);
    backend.clearPrimitiveRequests();
    QCOMPARE(backend.probe(setter).support, compat::Support::Supported);
    const auto response = backend.execute(setter);
    const bool accepted = currentSaved < 2;
    QCOMPARE(response.succeeded(), accepted);
    QCOMPARE(response.effect, accepted ? compat::Effect::Confirmed : compat::Effect::Uncertain);
    if (!accepted) QCOMPARE(response.failure->kind, compat::ErrorKind::InvalidArgument);
    const auto primitives = backend.primitiveRequests();
    QCOMPARE(std::ranges::count(primitives, setter), std::ptrdiff_t{1});
    QCOMPARE(std::ranges::count(primitives, getter), std::ptrdiff_t{0});
    const auto after = backend.snapshot();
    QCOMPARE(after.current, initial.current);
    QCOMPARE(after.next, initial.next);
    QCOMPARE(after.runtime.resetPersistentState, accepted ? currentSaved : initial.runtime.resetPersistentState);
    QCOMPARE(after.runtime.persistentRollbackState, accepted ? currentSaved : initial.runtime.persistentRollbackState);
    if (!accepted) QCOMPARE(after, initial);

    // Captured selector 255 resolves Current SavedMode privately; the active-device path rejects 2 and UINT32_MAX.
    Fake raw(initial);
    raw.setCapability(getter, {support, "Independent public SavedMode getter capability"});
    const auto captured = raw.executeCapturedPrimitiveForTest(setter);
    QCOMPARE(captured.succeeded(), accepted);
    QCOMPARE(captured.effect, accepted ? compat::Effect::Confirmed : compat::Effect::None);
    if (!accepted) QCOMPARE(captured.failure->kind, compat::ErrorKind::InvalidArgument);
    QCOMPARE(raw.snapshot(), after);
    QCOMPARE(raw.primitiveRequests(), std::vector<compat::Request>{setter});
  }

  void changingFutureSavedModeDoesNotChangeCurrentResetSelection() {
    auto initial = configured(true, static_cast<std::uint32_t>(compat::OverlayType::Disk), compat::flags::passthrough | compat::flags::persistent);
    Fake backend(initial);
    backend.seedFile(volumeC, relativeFile, "base");
    backend.writeFile(volumeC, relativeFile, "pending");
    QVERIFY(backend.execute(compat::SetNumber{compat::NumberProperty::ResetPersistentStateSavedMode, 1}).succeeded());
    QCOMPARE(compat::call(backend, compat::GetNumber{compat::NumberProperty::ResetPersistentStateSavedMode, compat::Session::Current}), std::uint32_t{0});
    QCOMPARE(compat::call(backend, compat::GetNumber{compat::NumberProperty::ResetPersistentStateSavedMode, compat::Session::Next}), std::uint32_t{1});
    QVERIFY(backend.execute(compat::SetNumber{compat::NumberProperty::ResetPersistentState, 255}).succeeded());
    QCOMPARE(backend.snapshot().runtime.persistentRollbackState, std::uint32_t{0});
    backend.simulateRestart();
    QCOMPARE(backend.visibleFile(volumeC, relativeFile), std::optional<std::string>{"pending"});
    QVERIFY(backend.execute(compat::SetNumber{compat::NumberProperty::ResetPersistentState, 255}).succeeded());
    QCOMPARE(backend.snapshot().runtime.persistentRollbackState, std::uint32_t{1});
    backend.simulateRestart();
    QCOMPARE(backend.visibleFile(volumeC, relativeFile), std::optional<std::string>{"base"});
  }
  void unsupportedSavedModeProfileKeepsRuntimePolicyIndependent_data() {
    QTest::addColumn<int>("intent");
    QTest::newRow("saved-get-current") << 0;
    QTest::newRow("saved-get-next") << 1;
    QTest::newRow("saved-set-clear") << 2;
    QTest::newRow("runtime-saved-selector") << 3;
    QTest::newRow("runtime-preserve") << 4;
    QTest::newRow("runtime-clear") << 5;
  }
  void unsupportedSavedModeProfileKeepsRuntimePolicyIndependent() {
    QFETCH(int, intent);
    auto state = configured(true, 1u, compat::flags::persistent);
    state.current.resetPersistentStateSavedMode = 1;
    state.runtime.resetPersistentState = 1;
    state.runtime.persistentRollbackState = 1;
    Fake::Profile profile;
    profile.savedMode = Fake::SavedModeProfile::Unsupported;
    Fake backend(state, "No-SavedMode", profile);
    const std::array<compat::Request, 6> requests{compat::GetNumber{compat::NumberProperty::ResetPersistentStateSavedMode, compat::Session::Current},
                                                  compat::GetNumber{compat::NumberProperty::ResetPersistentStateSavedMode, compat::Session::Next},
                                                  compat::SetNumber{compat::NumberProperty::ResetPersistentStateSavedMode, 1},
                                                  compat::SetNumber{compat::NumberProperty::ResetPersistentState, 255},
                                                  compat::SetNumber{compat::NumberProperty::ResetPersistentState, 0},
                                                  compat::SetNumber{compat::NumberProperty::ResetPersistentState, 1}};
    const auto& request = requests.at(static_cast<std::size_t>(intent));
    const bool runtimePolicy = intent >= 4;
    QCOMPARE(backend.probe(request).support, runtimePolicy ? compat::Support::Supported : compat::Support::Unsupported);
    const auto response = backend.execute(request);
    auto expected = state;
    if (runtimePolicy) {
      QVERIFY(response.succeeded());
      QCOMPARE(response.effect, compat::Effect::Confirmed);
      expected.runtime.resetPersistentState = intent == 4 ? 0u : 1u;
      expected.runtime.persistentRollbackState = intent == 4 ? 0u : 1u;
      const auto dispatched = backend.primitiveRequests();
      QCOMPARE(std::ranges::count(dispatched, request), std::ptrdiff_t{1});
    } else {
      QVERIFY(response.failure);
      QCOMPARE(response.failure->kind, compat::ErrorKind::Unsupported);
      QCOMPARE(response.effect, compat::Effect::None);
      QVERIFY(backend.primitiveRequests().empty());
    }
    QVERIFY(backend.snapshot() == expected);
    const auto trace = backend.trace();
    QCOMPARE(trace.size(), std::size_t{1});
    QVERIFY(!trace.front().capturedPrimitive);
  }

  void powerRequestsNotifyOnlyTheCurrentActiveFilter_data() {
    QTest::addColumn<bool>("currentEnabled");
    QTest::addColumn<bool>("controlPresent");
    QTest::addColumn<bool>("restart");
    for (const bool currentEnabled : {false, true})
      for (const bool controlPresent : {false, true})
        for (const bool restart : {false, true})
          QTest::newRow(qUtf8Printable(QStringLiteral("current%1-device%2-%3").arg(currentEnabled).arg(controlPresent).arg(restart ? "restart" : "shutdown")))
              << currentEnabled << controlPresent << restart;
  }
  void powerRequestsNotifyOnlyTheCurrentActiveFilter() {
    QFETCH(bool, currentEnabled);
    QFETCH(bool, controlPresent);
    QFETCH(bool, restart);
    auto initial = configured(currentEnabled);
    initial.runtime.controlDevicePresent = controlPresent;
    initial.runtime.shutdownPending = !currentEnabled && controlPresent;
    Fake backend(initial);
    const auto action = restart ? compat::PowerAction::Restart : compat::PowerAction::Shutdown;
    const auto response = backend.execute(compat::Power{action});
    const bool accepted = !currentEnabled || controlPresent;
    QCOMPARE(response.succeeded(), accepted);
    QCOMPARE(response.effect, accepted ? compat::Effect::Confirmed : compat::Effect::None);
    const auto after = backend.snapshot();
    if (accepted) {
      QCOMPARE(after.powerRequests, std::vector{action});
      QCOMPARE(after.runtime.shutdownPending, currentEnabled || initial.runtime.shutdownPending);
      QVERIFY(after.current == initial.current);
      QVERIFY(after.next == initial.next);
    } else {
      QCOMPARE(response.failure->kind, compat::ErrorKind::Unavailable);
      QVERIFY(after == initial);
    }
  }

  void composedCommandsKeepPublicCapabilityFaultAndTraceBoundaries() {
    Fake backend(configured());
    const compat::Request command = compat::SetOverlayOption{compat::OverlayOption::Persistent, true};
    backend.setCapability(command, {compat::Support::Unsupported, "This provider has no persistent option"});
    QCOMPARE(backend.probe(command).support, compat::Support::Unsupported);
    QCOMPARE(backend.execute(command).failure->kind, compat::ErrorKind::Unsupported);
    QCOMPARE(backend.trace().size(), std::size_t{1});
    QVERIFY(backend.trace().front().request == command);
    backend.clearCapabilityOverrides();
    backend.clearTrace();
    backend.inject({command, {}, Fake::FaultTiming::AfterEffect, compat::ErrorKind::Failed, "The command response was lost"});
    const auto response = backend.execute(command);
    QCOMPARE(response.effect, compat::Effect::Uncertain);
    QCOMPARE(backend.snapshot().next.overlayFlags, compat::flags::persistent);
    QCOMPARE(backend.trace().size(), std::size_t{1});
    QVERIFY(backend.trace().front().request == command);
  }

  void clearExclusionEffectsFollowSubmittedDeletes_data() {
    QTest::addColumn<bool>("registry");
    QTest::addColumn<bool>("populated");
    QTest::addColumn<bool>("lostReply");
    for (const bool registry : {false, true})
      for (const bool populated : {false, true})
        for (const bool lostReply : {false, true}) {
          const auto row = std::string(registry ? "registry" : "file") + (populated ? "-populated" : "-empty") + (lostReply ? "-lost-reply" : "-ack");
          QTest::newRow(row.c_str()) << registry << populated << lostReply;
        }
  }
  void clearExclusionEffectsFollowSubmittedDeletes() {
    QFETCH(bool, registry);
    QFETCH(bool, populated);
    QFETCH(bool, lostReply);
    auto initial = configured();
    initial.current.volumes.at(volumeC).fileExclusions = {"\\current"};
    initial.current.registryExclusions = {"HKEY_LOCAL_MACHINE\\Software\\Current"};
    if (populated) {
      initial.next.volumes.at(volumeC).fileExclusions = {"\\first", "\\second"};
      initial.next.registryExclusions = {key, "HKEY_LOCAL_MACHINE\\Software\\Second"};
    }
    Fake backend(initial);
    const compat::Request command = registry ? compat::Request{compat::EditRegistryExclusion{{}, compat::ExclusionAction::Clear}}
                                             : compat::Request{compat::EditFileExclusion{volumeC, {}, compat::ExclusionAction::Clear}};
    if (lostReply) backend.inject({command, {}, Fake::FaultTiming::AfterEffect, compat::ErrorKind::Failed, "The clear reply was lost"});
    const auto response = backend.execute(command);
    QCOMPARE(response.succeeded(), !lostReply);
    QCOMPARE(response.effect, !populated ? compat::Effect::None : lostReply ? compat::Effect::Uncertain : compat::Effect::Confirmed);
    const auto after = backend.snapshot();
    QVERIFY(after.current == initial.current);
    if (registry) {
      QVERIFY(after.next.registryExclusions.empty());
      QCOMPARE(after.next.volumes.at(volumeC).fileExclusions, initial.next.volumes.at(volumeC).fileExclusions);
    } else {
      QVERIFY(after.next.volumes.at(volumeC).fileExclusions.empty());
      QCOMPARE(after.next.registryExclusions, initial.next.registryExclusions);
    }
    const auto repeated = backend.execute(command);
    QVERIFY(repeated.succeeded());
    QCOMPARE(repeated.effect, compat::Effect::None);
  }

  void servicingTransitionsPreserveRuntimeAndSavedFilterState_data() {
    QTest::addColumn<int>("status");
    QTest::addColumn<bool>("enabled");
    QTest::addColumn<bool>("nextFilter");
    QTest::addColumn<int>("savedFilter");
    QTest::addColumn<int>("expectedStatus");
    QTest::addColumn<bool>("expectedFilter");
    QTest::addColumn<int>("expectedSavedFilter");
    QTest::addColumn<bool>("success");
    QTest::addColumn<bool>("publicSuccess");
    QTest::newRow("enter-preserves-enabled-next-filter") << 0 << true << true << -1 << 1 << false << 1 << true << true;
    QTest::newRow("enter-preserves-disabled-next-filter") << 0 << true << false << -1 << 1 << false << 0 << true << true;
    QTest::newRow("not-started-disable-is-normalized") << 0 << false << true << -1 << 0 << true << -1 << true << true;
    QTest::newRow("started-enable-keeps-saved-filter") << 1 << true << false << 1 << 1 << false << 1 << true << true;
    QTest::newRow("exit-restores-enabled-filter") << 1 << false << false << 1 << 2 << true << 1 << true << true;
    QTest::newRow("exit-keeps-disabled-filter") << 1 << false << false << 0 << 2 << false << 0 << true << true;
    QTest::newRow("exit-saved-zero-does-not-disable-filter") << 1 << false << true << 0 << 2 << true << 0 << true << true;
    QTest::newRow("exit-missing-runtime-reports-failed-status") << 1 << false << false << -1 << 3 << false << -1 << true << false;
    QTest::newRow("failed-state-can-enter") << 3 << true << true << 0 << 1 << false << 1 << true << true;
    QTest::newRow("failed-state-can-exit") << 3 << false << false << 1 << 2 << true << 1 << true << true;
    QTest::newRow("completed-state-enable-rejected-after-static-write") << 2 << true << true << 1 << 2 << true << 1 << false << false;
    QTest::newRow("completed-state-disable-rejected-after-static-write") << 2 << false << true << 1 << 2 << true << 1 << false << false;
  }
  void servicingTransitionsPreserveRuntimeAndSavedFilterState() {
    QFETCH(int, status);
    QFETCH(bool, enabled);
    QFETCH(bool, nextFilter);
    QFETCH(int, savedFilter);
    QFETCH(int, expectedStatus);
    QFETCH(bool, expectedFilter);
    QFETCH(int, expectedSavedFilter);
    QFETCH(bool, success);
    QFETCH(bool, publicSuccess);
    auto initial = configured();
    initial.next.filterEnabled = nextFilter;
    initial.next.servicingEnabled = !enabled;
    initial.runtime.servicingStatus = static_cast<Fake::ServicingStatus>(status);
    if (savedFilter >= 0) initial.runtime.servicingSavedFilterEnabled = savedFilter != 0;
    Fake backend(initial);
    const compat::Request request = compat::SetBoolean{compat::BooleanProperty::ServicingEnabled, enabled};
    const auto response = backend.executeCapturedPrimitiveForTest(request);
    QCOMPARE(response.succeeded(), success);
    QCOMPARE(response.effect, success ? compat::Effect::Confirmed : compat::Effect::Uncertain);
    const auto after = backend.snapshot();
    QVERIFY(after.current == initial.current);
    QCOMPARE(after.next.servicingEnabled, enabled);
    QCOMPARE(after.next.filterEnabled, expectedFilter);
    QCOMPARE(static_cast<int>(after.runtime.servicingStatus), expectedStatus);
    QCOMPARE(after.runtime.servicingSavedFilterEnabled, expectedSavedFilter < 0 ? std::nullopt : std::optional<bool>{expectedSavedFilter != 0});
    if (!success) {
      QCOMPARE(response.failure->kind, compat::ErrorKind::Failed);
      QCOMPARE(response.failure->diagnostics.front().code, static_cast<std::int64_t>(static_cast<std::int32_t>(0x8007000du)));
    }
    Fake publicBackend(initial);
    const auto publicReceipt = publicBackend.execute(request);
    QCOMPARE(publicReceipt.succeeded(), publicSuccess);
    if (!publicSuccess) {
      QCOMPARE(publicReceipt.failure->kind, compat::ErrorKind::Conflict);
      QCOMPARE(publicReceipt.effect, compat::Effect::None);
      QCOMPARE(publicBackend.snapshot(), initial);
      QVERIFY(publicBackend.primitiveRequests().empty());
    } else
      QCOMPARE(publicBackend.snapshot(), after);
  }

  void servicingFilterChangesUseRomDispositionAndFailuresKeepEarlierWrites() {
    auto initial = configured(true, static_cast<std::uint32_t>(compat::OverlayType::Ram), compat::flags::readOnlyMedia);
    Fake backend(initial);
    const auto enabled = backend.execute(compat::SetBoolean{compat::BooleanProperty::ServicingEnabled, true});
    QVERIFY(enabled.succeeded());
    QCOMPARE(enabled.effect, compat::Effect::Confirmed);
    QCOMPARE(backend.snapshot().runtime.romDisposition, std::uint32_t{1});
    QVERIFY(!backend.snapshot().next.filterEnabled);
    const auto disabled = backend.execute(compat::SetBoolean{compat::BooleanProperty::ServicingEnabled, false});
    QVERIFY(disabled.succeeded());
    QVERIFY(backend.snapshot().next.filterEnabled);
    QCOMPARE(backend.snapshot().runtime.servicingStatus, Fake::ServicingStatus::Completed);

    initial.runtime.controlDevicePresent = false;
    Fake missingControl(initial);
    const auto failed = missingControl.execute(compat::SetBoolean{compat::BooleanProperty::ServicingEnabled, true});
    QVERIFY(failed.failure);
    QCOMPARE(failed.effect, compat::Effect::Uncertain);
    QVERIFY(missingControl.snapshot().next.servicingEnabled);
    QVERIFY(missingControl.snapshot().next.filterEnabled);
    QCOMPARE(missingControl.snapshot().runtime.servicingStatus, Fake::ServicingStatus::Failed);
    Fake raw(initial);
    const auto rawFailure = raw.executeCapturedPrimitiveForTest(compat::SetBoolean{compat::BooleanProperty::ServicingEnabled, true});
    QVERIFY(rawFailure.failure);
    QCOMPARE(rawFailure.effect, compat::Effect::Uncertain);
    QCOMPARE(missingControl.snapshot(), raw.snapshot());
  }

  void filterChangesRespectRomDispositionBeforeWritingNextConfiguration_data() {
    QTest::addColumn<bool>("currentEnabled");
    QTest::addColumn<bool>("rom");
    QTest::addColumn<std::uint32_t>("disposition");
    QTest::addColumn<bool>("success");
    QTest::addColumn<std::uint32_t>("expectedDisposition");
    QTest::newRow("active-rom-first-filter-change") << true << true << std::uint32_t{0} << true << std::uint32_t{1};
    QTest::newRow("active-rom-repeat-filter-change") << true << true << std::uint32_t{1} << true << std::uint32_t{1};
    QTest::newRow("active-rom-horm-prepared-rejects-change") << true << true << std::uint32_t{2} << false << std::uint32_t{2};
    QTest::newRow("active-rom-full-commit-retains-disposition") << true << true << std::uint32_t{4} << true << std::uint32_t{5};
    QTest::newRow("ordinary-mode-does-not-set-rom-disposition") << true << false << std::uint32_t{0} << true << std::uint32_t{0};
    QTest::newRow("disabled-filter-does-not-contact-rom") << false << true << std::uint32_t{0} << true << std::uint32_t{0};
  }
  void filterChangesRespectRomDispositionBeforeWritingNextConfiguration() {
    QFETCH(bool, currentEnabled);
    QFETCH(bool, rom);
    QFETCH(std::uint32_t, disposition);
    QFETCH(bool, success);
    QFETCH(std::uint32_t, expectedDisposition);
    auto initial = configured(currentEnabled, static_cast<std::uint32_t>(compat::OverlayType::Ram), rom ? compat::flags::readOnlyMedia : 0);
    initial.next.filterEnabled = true;
    initial.runtime.romDisposition = disposition;
    Fake backend(initial);
    backend.seedFile(volumeC, relativeFile, "base");
    backend.writeFile(volumeC, relativeFile, "changed");
    const auto before = backend.snapshot();
    const auto response = backend.execute(compat::SetBoolean{compat::BooleanProperty::FilterEnabled, false});
    QCOMPARE(response.succeeded(), success);
    QCOMPARE(response.effect, success ? compat::Effect::Confirmed : compat::Effect::Uncertain);
    const auto after = backend.snapshot();
    QVERIFY(after.current == before.current);
    QVERIFY(after.durable == before.durable);
    QVERIFY(after.overlay == before.overlay);
    QCOMPARE(after.next.filterEnabled, !success);
    QCOMPARE(after.runtime.romDisposition, expectedDisposition);
    if (!success) {
      QCOMPARE(response.failure->kind, compat::ErrorKind::Conflict);
      QVERIFY(after == before);
    }
    if (expectedDisposition == 1) {
      const auto fullCommit = backend.execute(compat::SetBoolean{compat::BooleanProperty::OverlayCommit, true});
      QVERIFY(fullCommit.failure);
      QCOMPARE(fullCommit.failure->kind, compat::ErrorKind::Conflict);
      QCOMPARE(fullCommit.effect, compat::Effect::None);
      QCOMPARE(backend.readFile(volumeC, relativeFile), std::optional<std::string>{"base"});
      QCOMPARE(backend.visibleFile(volumeC, relativeFile), std::optional<std::string>{"changed"});
    }
  }

  void exclusionAdmissionUsesCapturedRulesAndExplicitEnvironmentFacts_data() {
    QTest::addColumn<bool>("registry");
    QTest::addColumn<std::string>("path");
    QTest::addColumn<bool>("allowed");
    QTest::addColumn<bool>("environment");
    QTest::addColumn<bool>("rootDirectory");
    QTest::newRow("file-missing-object-allowed") << false << std::string("\\new\\missing") << true << false << false;
    QTest::newRow("file-empty") << false << std::string{} << false << false << false;
    QTest::newRow("file-forbidden-character") << false << std::string("\\bad?name") << false << false << false;
    QTest::newRow("file-control-character") << false << std::string("\\bad\x1e") << false << false << false;
    QTest::newRow("file-260-utf16-units") << false << ("\\" + std::string(259, 'a')) << true << false << false;
    QTest::newRow("file-261-utf16-units") << false << ("\\" + std::string(260, 'a')) << false << false << false;
    std::string supplementary = "\\";
    for (int index = 0; index < 130; ++index) supplementary += "\xf0\x9f\x98\x80";
    QTest::newRow("file-supplementary-utf16-length") << false << supplementary << false << false << false;
    QTest::newRow("file-system-fact-not-inferred") << false << std::string("\\Windows") << true << false << false;
    QTest::newRow("file-guid-prefix-does-not-match-drive-blacklist") << false << std::string("\\windows\\SYSTEM32") << true << true << false;
    QTest::newRow("file-system-descendant-not-blacklisted") << false << std::string("\\Windows\\Temp") << true << true << false;
    QTest::newRow("file-known-volume-root") << false << std::string("\\") << false << false << true;
    QTest::newRow("file-pagefile-is-not-commit-rule") << false << std::string("\\pagefile.sys") << true << true << false;
    QTest::newRow("registry-machine-descendant") << true << std::string("HKLM\\SOFTWARE\\MissingApplication") << true << false << false;
    QTest::newRow("registry-literal-slash") << true << std::string("HKLM\\SOFTWARE\\Name/Child") << true << false << false;
    QTest::newRow("registry-literal-control") << true << (std::string("HKLM\\SOFTWARE\\Name") + char{1} + "Suffix") << true << false << false;
    QTest::newRow("registry-repeated-and-trailing-separators") << true << std::string("HKLM\\SOFTWARE\\\\Name\\") << true << false << false;
    QTest::newRow("registry-other-hive") << true << std::string("HKCU\\Software\\Application") << false << false << false;
    QTest::newRow("registry-empty-relative") << true << std::string("HKLM\\") << false << false << false;
    for (const auto* blocked : {"SYSTEM", "SYSTEM\\CurrentControlSet", "SOFTWARE", "SAM", "SECURITY", "COMPONENTS"})
      QTest::newRow((std::string("registry-exact-") + blocked).c_str()) << true << (std::string("HKLM\\") + blocked) << false << false << false;
    QTest::newRow("registry-blacklist-is-not-prefix") << true << std::string("HKLM\\SECURITY\\Application") << true << false << false;
    QTest::newRow("registry-255-unit-component") << true << ("HKLM\\" + std::string(255, 'a')) << true << false << false;
    QTest::newRow("registry-256-unit-component") << true << ("HKLM\\" + std::string(256, 'a')) << false << false << false;
    std::string deep = "HKLM\\Application";
    for (int index = 1; index < 512; ++index) deep += "\\k";
    QTest::newRow("registry-512-components-not-512-total-chars") << true << deep << true << false << false;
    QTest::newRow("registry-513-components") << true << (deep + "\\k") << false << false << false;
  }
  void exclusionAdmissionUsesCapturedRulesAndExplicitEnvironmentFacts() {
    QFETCH(bool, registry);
    QFETCH(std::string, path);
    QFETCH(bool, allowed);
    QFETCH(bool, environment);
    QFETCH(bool, rootDirectory);
    auto initial = configured();
    if (environment) {
      initial.runtime.systemVolumeId = volumeC;
      initial.runtime.windowsDirectory = "\\Windows";
    }
    Fake backend(initial);
    if (rootDirectory) backend.seedDirectory(volumeC, "\\");
    const compat::Request command = registry ? compat::Request{compat::EditRegistryExclusion{path, compat::ExclusionAction::Add}}
                                             : compat::Request{compat::EditFileExclusion{volumeC, path, compat::ExclusionAction::Add}};
    const auto before = backend.snapshot();
    const auto response = backend.execute(command);
    QCOMPARE(response.succeeded(), allowed);
    if (!allowed) {
      QCOMPARE(response.failure->kind, compat::ErrorKind::InvalidArgument);
      QCOMPARE(response.effect, rootDirectory ? compat::Effect::Uncertain : compat::Effect::None);
      QVERIFY(backend.snapshot() == before);
    }
  }

  void fileCommitsRequireCurrentRegistrationAndPreserveExactTargets_data() {
    QTest::addColumn<SystemProtection>("currentProtection");
    QTest::addColumn<bool>("filterEnabled");
    QTest::addColumn<QString>("path");
    QTest::addColumn<bool>("exists");
    const std::array protections{std::pair{"absent", SystemProtection::Absent}, std::pair{"unprotected", SystemProtection::Unprotected},
                                 std::pair{"protected", SystemProtection::Protected}};
    const std::array paths{std::pair{"ordinary", QStringLiteral("\\Data\\Item.txt")}, std::pair{"literal-space", QStringLiteral("\\Data\\Item.txt ")},
                           std::pair{"literal-dot", QStringLiteral("\\Data\\Item.txt.")}};
    for (const auto& [membership, protection] : protections)
      for (const bool filterEnabled : {false, true})
        for (const auto& [spelling, path] : paths)
          for (const bool exists : {false, true})
            QTest::newRow(qUtf8Printable(
                QStringLiteral("%1/filter-%2/%3/%4").arg(membership, filterEnabled ? "on" : "off", spelling, exists ? "file-present" : "file-missing")))
                << protection << filterEnabled << path << exists;
  }
  void fileCommitsRequireCurrentRegistrationAndPreserveExactTargets() {
    QFETCH(SystemProtection, currentProtection);
    QFETCH(bool, filterEnabled);
    QFETCH(QString, path);
    QFETCH(bool, exists);
    Fake backend(configured(filterEnabled));
    if (currentProtection == SystemProtection::Absent) {
      (void)compat::call(backend, compat::RemoveVolume{volumeC});
      backend.simulateRestart();
      // A Next-only protected registration must not substitute for Current.
      (void)compat::call(backend, compat::SetVolume{volumeC, compat::VolumeProperty::Protected, true, "C:", true});
    } else {
      (void)compat::call(backend, compat::SetVolume{volumeC, compat::VolumeProperty::Protected, currentProtection == SystemProtection::Protected});
      backend.simulateRestart();
    }
    const auto original = path.toStdString();
    backend.seedFile(volumeC, original + "-sibling", "unchanged");
    if (exists) {
      backend.seedFile(volumeC, original, "base");
      backend.writeFile(volumeC, original, "changed");
    }
    backend.clearTrace();
    const auto before = backend.snapshot();
    const compat::Request request = compat::CommitFile{volumeC, original};
    const auto response = backend.execute(request);
    const bool success = currentProtection != SystemProtection::Absent && exists;
    QCOMPARE(response.succeeded(), success);
    QCOMPARE(response.effect, success ? compat::Effect::Confirmed : compat::Effect::None);
    if (!success) QCOMPARE(response.failure->kind, compat::ErrorKind::NotFound);
    auto expected = before;
    if (success && filterEnabled && currentProtection == SystemProtection::Protected) {
      const Fake::FileAddress target{volumeC, original};
      expected.durable.files.insert_or_assign(target, "changed");
      expected.overlay.files.erase(target);
      expected.current.volumes.at(volumeC).volume.commitPending = false;
      expected.fileCommitStages[target] = {Fake::FaultTiming::AfterDestinationEof, Fake::FaultTiming::AfterFileDataChunk, Fake::FaultTiming::AfterFileData,
                                           Fake::FaultTiming::AfterFileEa,         Fake::FaultTiming::AfterFileAds,       Fake::FaultTiming::AfterFileTime,
                                           Fake::FaultTiming::AfterFileSecurity};
    }
    QVERIFY(backend.snapshot() == expected);
    const auto trace = backend.trace();
    QCOMPARE(trace.size(), std::size_t{1});
    QVERIFY(trace.front().request == request);
    QCOMPARE(backend.readFile(volumeC, original + "-sibling"), std::optional<std::string>{"unchanged"});
    if (success) {
      backend.simulateRestart();
      QCOMPARE(backend.visibleFile(volumeC, original), std::optional<std::string>{"changed"});
    }
  }

  void fileCommitManagementBufferIsCheckedBeforeDispatch_data() {
    QTest::addColumn<bool>("deleting");
    QTest::addColumn<bool>("directory");
    QTest::addColumn<QString>("path");
    QTest::addColumn<bool>("excluded");
    QTest::addColumn<bool>("accepted");
    const auto prefix = QStringLiteral("\\Parent\\");
    const auto fits = prefix + QString(249, u'a');
    const auto tooLong = prefix + QString(250, u'a');
    for (const auto& [name, operation] : {std::pair{"commit-file", std::pair{false, false}}, std::pair{"delete-file", std::pair{true, false}},
                                          std::pair{"delete-directory", std::pair{true, true}}}) {
      QTest::newRow((std::string(name) + "/257").c_str()) << operation.first << operation.second << fits << false << true;
      QTest::newRow((std::string(name) + "/258").c_str()) << operation.first << operation.second << tooLong << false << false;
    }
    QTest::newRow("non-bmp-258-units") << false << false << (prefix + QString(248, u'a') + QString::fromUcs4(U"\U0001f600")) << false << false;
    QTest::newRow("excluded-no-op-257") << false << false << fits << true << true;
    QTest::newRow("excluded-no-op-258") << false << false << tooLong << true << false;
  }
  void fileCommitManagementBufferIsCheckedBeforeDispatch() {
    QFETCH(bool, deleting);
    QFETCH(bool, directory);
    QFETCH(QString, path);
    QFETCH(bool, excluded);
    QFETCH(bool, accepted);
    auto initial = configured(true);
    const auto original = path.toStdString();
    if (excluded) initial.current.volumes.at(volumeC).fileExclusions = {original};
    Fake backend(initial);
    if (directory)
      backend.seedDirectory(volumeC, original);
    else {
      backend.seedFile(volumeC, original, "base");
      backend.writeFile(volumeC, original, "changed");
    }
    backend.clearTrace();
    const auto before = backend.snapshot();
    const compat::Request request =
        deleting ? compat::Request{compat::CommitFileDeletion{volumeC, original}} : compat::Request{compat::CommitFile{volumeC, original}};
    const auto response = backend.execute(request);
    QCOMPARE(response.succeeded(), accepted);
    QCOMPARE(response.effect, accepted ? compat::Effect::Confirmed : compat::Effect::None);
    if (!accepted) QCOMPARE(response.failure->kind, compat::ErrorKind::InvalidArgument);
    if (!accepted || excluded) QVERIFY(backend.snapshot() == before);
    const auto trace = backend.trace();
    QCOMPARE(trace.size(), std::size_t{1});
    QVERIFY(trace.front().request == request);
    backend.simulateRestart();
    if (directory)
      QCOMPARE(backend.visibleDirectory(volumeC, original), !accepted);
    else
      QCOMPARE(backend.visibleFile(volumeC, original),
               accepted && deleting ? std::optional<std::string>{} : std::optional<std::string>{accepted || excluded ? "changed" : "base"});
  }

  void commitReservedFilesUsesItsOwnVolumeDependentRules() {
    auto initial = configured(true);
    initial.runtime.systemVolumeId = volumeC;
    Fake backend(initial);
    for (const auto* id : {volumeC, volumeD}) {
      for (const auto* name : {"\\pagefile.sys", "\\hiberfil.sys", "\\UWFSWAP.SYS", "\\swapfile.sys"}) {
        backend.seedFile(id, name, "base");
        backend.writeFile(id, name, "changed");
        const auto before = backend.snapshot();
        const bool reserved = std::string_view(name) == "\\pagefile.sys" || (std::string_view(id) == volumeC && std::string_view(name) != "\\swapfile.sys");
        const auto response = backend.execute(compat::CommitFile{id, name});
        QCOMPARE(response.succeeded(), !reserved);
        if (reserved) {
          QCOMPARE(response.failure->kind, compat::ErrorKind::InvalidArgument);
          QCOMPARE(response.effect, compat::Effect::None);
          QCOMPARE(backend.snapshot(), before);
          const auto raw = backend.executeCapturedPrimitiveForTest(compat::CommitFile{id, name});
          QVERIFY(raw.failure);
          QCOMPARE(raw.failure->kind, compat::ErrorKind::InvalidArgument);
          QCOMPARE(raw.effect, compat::Effect::None);
          QCOMPARE(backend.snapshot(), before);
        }
      }
    }
  }

  void configurationDispatchesVolumeRelativeExclusionPaths() {
    Scenario scenario(configured());
    app::ConfigurationChanges changes;
    changes.settings.addFileExclusions["C:"] = {"C:\\Data\\Selected"};
    QCOMPARE(scenario.apply(changes).failedOperations(), 0);
    changes.settings.clear();
    changes.settings.removeFileExclusions["C:"] = {"c:\\Data\\Selected"};
    QCOMPARE(scenario.apply(changes).failedOperations(), 0);
    std::size_t edits = 0;
    for (const auto& view : scenario.views)
      for (const auto& entry : view->trace())
        if (const auto* edit = std::get_if<compat::EditFileExclusion>(&entry.request)) {
          QCOMPARE(edit->path, std::string("\\Data\\Selected"));
          ++edits;
        }
    QCOMPARE(edits, std::size_t{2});
  }

  void maintenanceChangesSurviveDiscardedOverlay_data() {
    QTest::addColumn<bool>("registry");
    QTest::addColumn<bool>("deletion");
    QTest::addColumn<bool>("defaultValue");
    QTest::newRow("file-commit") << false << false << false;
    QTest::newRow("file-deletion") << false << true << false;
    QTest::newRow("registry-value-commit") << true << false << false;
    QTest::newRow("registry-default-value-commit") << true << false << true;
    QTest::newRow("registry-key-deletion") << true << true << false;
  }
  void maintenanceChangesSurviveDiscardedOverlay() {
    QFETCH(bool, registry);
    QFETCH(bool, deletion);
    QFETCH(bool, defaultValue);
    Scenario scenario(configured(true));
    auto& store = *scenario.views[0];
    const auto maintenance = app::uwfMaintenanceRepository(scenario.system);
    QCOMPARE(registry ? maintenance->registryCommitRequirement() : maintenance->fileCommitRequirement(volumeC), app::CommitRequirement::RequiresUwfCommit);
    const std::string selectedName = defaultValue ? "" : valueName;
    if (registry) {
      store.seedRegistryValue(key, selectedName, "base");
      store.seedRegistryValue(key, "Sibling", "sibling-base");
      store.seedRegistryValue(std::string(key) + "\\child", "Nested", "base");
      store.writeRegistryValue(key, selectedName, "changed");
      store.writeRegistryValue(key, "Sibling", "sibling-overlay");
      const auto target = maintenance->prepareRegistryMaintenance();
      QVERIFY(target);
      if (deletion) {
        QCOMPARE(store.readRegistryValue(key, selectedName), std::optional<std::string>{"base"});
        target->commitDeletion(std::string(key) + "\\child", std::nullopt);
        target->commitDeletion(key, std::nullopt);
      } else
        target->commit(key, selectedName);
      QCOMPARE(store.readRegistryValue(key, selectedName), deletion ? std::optional<std::string>{} : std::optional<std::string>{"changed"});
      QCOMPARE(store.readRegistryValue(key, "Sibling"), deletion ? std::optional<std::string>{} : std::optional<std::string>{"sibling-base"});
      if (deletion) QVERIFY(!store.readRegistryValue(std::string(key) + "\\child", "Nested"));
    } else {
      store.seedFile(volumeC, file, "base");
      store.writeFile(volumeC, file, "changed");
      const auto target = systemVolume(maintenance);
      if (deletion) {
        QCOMPARE(store.readFile(volumeC, file), std::optional<std::string>{"base"});
        target->commitFileDeletion(file);
      } else
        target->commitFile(file);
      QCOMPARE(store.readFile(volumeC, file), deletion ? std::optional<std::string>{} : std::optional<std::string>{"changed"});
      for (const auto& view : scenario.views)
        for (const auto& entry : view->trace()) {
          if (const auto* request = std::get_if<compat::CommitFile>(&entry.request)) QCOMPARE(request->path, std::string("\\data\\sample.txt"));
          if (const auto* request = std::get_if<compat::CommitFileDeletion>(&entry.request)) QCOMPARE(request->path, std::string("\\data\\sample.txt"));
        }
    }
    store.simulateRestart();
    const auto durable = registry ? store.readRegistryValue(key, selectedName) : store.readFile(volumeC, file);
    const auto visible = registry ? store.visibleRegistryValue(key, selectedName) : store.visibleFile(volumeC, file);
    QCOMPARE(visible, durable);
    QCOMPARE(durable, deletion ? std::optional<std::string>{} : std::optional<std::string>{"changed"});
    QVERIFY(store.snapshot().overlay.files.empty());
    QVERIFY(store.snapshot().overlay.registry.empty());
  }

  void registryDeletionPreservesIntentAndNonrecursiveBoundaries() {
    Scenario scenario(configured(true));
    auto& store = *scenario.views[0];
    store.seedRegistryValue(key, "", "default-base");
    store.seedRegistryValue(key, "Sibling", "sibling-base");
    const std::string child = std::string(key) + "\\child";
    store.seedRegistryValue(child, "Nested", "child-base");
    const auto before = store.snapshot();
    for (const compat::Request& request :
         {compat::Request{compat::CommitRegistryDeletion{key, std::string{}}}, compat::Request{compat::CommitRegistry{key, std::nullopt}}}) {
      QCOMPARE(store.probe(request).support, compat::Support::Unsupported);
      const auto response = store.execute(request);
      QCOMPARE(response.failure->kind, compat::ErrorKind::Unsupported);
      QCOMPARE(response.effect, compat::Effect::None);
      QVERIFY(store.snapshot() == before);
    }
    const auto nonrecursive = store.execute(compat::CommitRegistryDeletion{key, std::nullopt});
    QCOMPARE(nonrecursive.failure->kind, compat::ErrorKind::Conflict);
    QCOMPARE(nonrecursive.effect, compat::Effect::Uncertain);
    auto rejectedAfterBackingLoad = before;
    rejectedAfterBackingLoad.runtime.registryCommitHive = Fake::RegistryHive::Ready;
    QVERIFY(store.snapshot() == rejectedAfterBackingLoad);
    const auto target = app::uwfMaintenanceRepository(scenario.system)->prepareRegistryMaintenance();
    target->commitDeletion(child, std::nullopt);
    target->commitDeletion(key, std::nullopt);
    store.simulateRestart();
    QVERIFY(!store.visibleRegistryValue(key, ""));
    QVERIFY(!store.visibleRegistryValue(key, "Sibling"));
    QVERIFY(!store.visibleRegistryValue(child, "Nested"));
  }

  void registryImmediateCommandsRejectControlSetPrefixCollisions_data() {
    QTest::addColumn<QString>("target");
    QTest::addColumn<bool>("deletion");
    QTest::addColumn<int>("valueSelection");
    QTest::addColumn<bool>("overrideSupported");
    for (const auto& root : {QStringLiteral("HKLM\\SYSTEM\\CurrentControlSetExtra"), QStringLiteral("hkey_local_machine\\system\\currentcontrolsetBackup")})
      for (const bool child : {false, true})
        for (const bool deletion : {false, true})
          for (const int valueSelection : {0, 1, 2}) {
            const auto target = root + (child ? QStringLiteral("\\Fixture") : QString{});
            QTest::newRow(qPrintable(QStringLiteral("%1/delete-%2/value-%3").arg(target).arg(deletion).arg(valueSelection)))
                << target << deletion << valueSelection << false;
          }
    QTest::newRow("supported-override-cannot-bypass-exact-target-contract")
        << QStringLiteral("HKLM\\SYSTEM\\CurrentControlSetExtra\\Fixture") << true << 2 << true;
  }
  void registryImmediateCommandsRejectControlSetPrefixCollisions() {
    QFETCH(QString, target);
    QFETCH(bool, deletion);
    QFETCH(int, valueSelection);
    QFETCH(bool, overrideSupported);
    Fake backend(configured(true));
    const auto selected = valueSelection == 0   ? std::optional<std::string>{}
                          : valueSelection == 1 ? std::optional<std::string>{std::string{}}
                                                : std::optional<std::string>{valueName};
    const auto unsafeKey = target.toStdString();
    backend.seedRegistryValue(unsafeKey, valueName, "selected-base");
    backend.writeRegistryValue(unsafeKey, valueName, "selected-pending");
    backend.seedRegistryValue(unsafeKey, {}, "selected-default-base");
    backend.writeRegistryValue(unsafeKey, {}, "selected-default-pending");
    // A similarly spelled concrete object is independent. Rejecting the
    // request must preserve both it and the user's actual selected object.
    const std::string other = "HKLM\\SYSTEM\\ControlSet001Extra\\Fixture";
    backend.seedRegistryValue(other, valueName, "other-base");
    backend.writeRegistryValue(other, valueName, "other-pending");
    backend.seedRegistryValue(key, valueName, "safe-base");
    backend.writeRegistryValue(key, valueName, "safe-pending");
    const compat::Request request =
        deletion ? compat::Request{compat::CommitRegistryDeletion{unsafeKey, selected}} : compat::Request{compat::CommitRegistry{unsafeKey, selected}};
    const auto before = backend.snapshot();
    if (overrideSupported) backend.setCapability(request, {compat::Support::Supported, "Injected capability cannot change target semantics"});
    const std::string sentinel = "The immediate dispatch fault must remain pending";
    backend.inject({{}, compat::operationName(request), Fake::FaultTiming::Before, compat::ErrorKind::Failed, sentinel});
    QCOMPARE(backend.probe(request).support, compat::Support::Unsupported);
    const auto rejected = backend.execute(request);
    QVERIFY(rejected.failure);
    QCOMPARE(rejected.failure->kind, compat::ErrorKind::Unsupported);
    QCOMPARE(rejected.failure->effect, compat::Effect::None);
    QCOMPARE(rejected.effect, compat::Effect::None);
    QVERIFY(backend.snapshot() == before);
    // The unconsumed dispatch fault proves rejection occurred before the raw operation.
    const compat::Request safe = deletion ? compat::Request{compat::CommitRegistryDeletion{key, std::string{valueName}}}
                                          : compat::Request{compat::CommitRegistry{key, std::string{valueName}}};
    const auto next = backend.execute(safe);
    QVERIFY(next.failure);
    QCOMPARE(next.failure->kind, compat::ErrorKind::Failed);
    QCOMPARE(next.failure->message, sentinel);
    QCOMPARE(next.effect, compat::Effect::None);
    QVERIFY(backend.snapshot() == before);
    const auto trace = backend.trace();
    QCOMPARE(trace.size(), std::size_t{2});
    QVERIFY(trace.front().request == request);
    QVERIFY(trace.back().request == safe);
  }

  void registryImmediateCommandsKeepCompleteControlSetTargets_data() {
    QTest::addColumn<quint32>("selectedControlSet");
    QTest::addColumn<QString>("selector");
    QTest::addColumn<bool>("deletion");
    QTest::addColumn<int>("valueSelection");
    for (const quint32 current : {1u, 2u})
      for (const auto& selector : {QStringLiteral("CurrentControlSet"), QStringLiteral("ControlSet001"), QStringLiteral("ControlSet002")})
        for (const bool deletion : {false, true})
          for (const int valueSelection : {deletion ? 0 : 1, 2})
            QTest::newRow(qPrintable(QStringLiteral("current-%1/%2/delete-%3/value-%4").arg(current).arg(selector).arg(deletion).arg(valueSelection)))
                << current << selector << deletion << valueSelection;
  }
  void registryImmediateCommandsKeepCompleteControlSetTargets() {
    QFETCH(quint32, selectedControlSet);
    QFETCH(QString, selector);
    QFETCH(bool, deletion);
    QFETCH(int, valueSelection);
    auto initial = configured(true);
    initial.runtime.currentControlSet = selectedControlSet;
    Fake backend(std::move(initial));
    const auto target = (QStringLiteral("HKLM\\SYSTEM\\") + selector + QStringLiteral("\\ExactTargetFixture")).toStdString();
    const auto selected = valueSelection == 0   ? std::optional<std::string>{}
                          : valueSelection == 1 ? std::optional<std::string>{std::string{}}
                                                : std::optional<std::string>{valueName};
    const auto value = selected.value_or(valueName);
    backend.seedRegistryValue(target, value, "base");
    backend.writeRegistryValue(target, value, "pending");
    backend.seedRegistryValue(target + "Sibling", value, "unrelated-base");
    backend.writeRegistryValue(target + "Sibling", value, "unrelated-pending");
    const compat::Request request =
        deletion ? compat::Request{compat::CommitRegistryDeletion{target, selected}} : compat::Request{compat::CommitRegistry{target, selected}};
    QCOMPARE(backend.probe(request).support, compat::Support::Supported);
    const auto result = backend.execute(request);
    QVERIFY2(result.succeeded(), result.failure ? result.failure->message.c_str() : "");
    QCOMPARE(result.effect, compat::Effect::Confirmed);
    const auto persisted = deletion ? std::optional<std::string>{} : std::optional<std::string>{"pending"};
    QCOMPARE(backend.readRegistryValue(target, value), persisted);
    QCOMPARE(backend.visibleRegistryValue(target, value), persisted);
    QCOMPARE(backend.readRegistryValue(target + "Sibling", value), std::optional<std::string>{"unrelated-base"});
    QCOMPARE(backend.readRegistryValue(target + "Sibling", value, Fake::Storage::Overlay), std::optional<std::string>{"unrelated-pending"});
    backend.simulateRestart();
    QCOMPARE(backend.visibleRegistryValue(target, value), persisted);
    QCOMPARE(backend.visibleRegistryKey(target), !deletion || valueSelection != 0);
    QCOMPARE(backend.visibleRegistryValue(target + "Sibling", value), std::optional<std::string>{"unrelated-base"});
  }

  void visibleDeletionFailureDoesNotBecomeDurableSuccess() {
    Scenario scenario(configured(true));
    auto& store = *scenario.views[0];
    store.seedRegistryValue(key, valueName, "base");
    const compat::Request request = compat::CommitRegistryDeletion{key, std::string(valueName)};
    store.inject({request, {}, Fake::FaultTiming::AfterVisibleDeletion, compat::ErrorKind::AccessDenied, "Persistent hive deletion failed"});
    const auto target = app::uwfMaintenanceRepository(scenario.system)->prepareRegistryMaintenance();
    bool unknown = false;
    try {
      target->commitDeletion(key, std::string(valueName));
    } catch (const app::OperationError& error) {
      unknown = error.writeEffect() == app::WriteEffect::Uncertain;
    }
    QVERIFY(unknown);
    QVERIFY(!store.visibleRegistryValue(key, valueName));
    QCOMPARE(store.readRegistryValue(key, valueName), std::optional<std::string>{"base"});
    for (std::size_t index = 1; index < scenario.views.size(); ++index)
      for (const auto& entry : scenario.views[index]->trace()) QVERIFY(entry.request != request);
    store.simulateRestart();
    QCOMPARE(store.visibleRegistryValue(key, valueName), std::optional<std::string>{"base"});
    // A later explicit retry can succeed once the original target is visible.
    target->commitDeletion(key, std::string(valueName));
    store.simulateRestart();
    QVERIFY(!store.visibleRegistryValue(key, valueName));
  }

  void missingCurrentTargetsAreAttemptedWithoutInventingSuccess_data() {
    QTest::addColumn<bool>("registry");
    QTest::newRow("file-hidden-by-overlay") << false;
    QTest::newRow("registry-value-hidden-by-overlay") << true;
  }
  void missingCurrentTargetsAreAttemptedWithoutInventingSuccess() {
    QFETCH(bool, registry);
    Scenario scenario(configured(true));
    auto& store = *scenario.views[0];
    const compat::Request request = registry ? compat::Request{compat::CommitRegistryDeletion{key, std::string(valueName)}}
                                             : compat::Request{compat::CommitFileDeletion{volumeC, relativeFile}};
    if (registry) {
      store.seedRegistryValue(key, valueName, "base");
      store.deleteRegistryValue(key, valueName);
    } else {
      store.seedFile(volumeC, file, "base");
      store.deleteFile(volumeC, file);
    }
    const auto response = scenario.system->execute(request);
    QVERIFY(response.failure);
    QCOMPARE(response.failure->kind, compat::ErrorKind::NotFound);
    QCOMPARE(response.effect, compat::Effect::Uncertain);
    std::size_t attempts = 0;
    for (const auto& view : scenario.views)
      for (const auto& entry : view->trace())
        if (entry.request == request) ++attempts;
    QCOMPARE(attempts, std::size_t{1});
    store.simulateRestart();
    QCOMPARE(registry ? store.visibleRegistryValue(key, valueName) : store.visibleFile(volumeC, file), std::optional<std::string>{"base"});
  }

  void exactDirectoryAndKeyDeletionRequireReviewedPostorder() {
    Scenario scenario(configured(true));
    auto& store = *scenario.views[0];
    const auto maintenance = app::uwfMaintenanceRepository(scenario.system);
    const auto files = systemVolume(maintenance);
    store.seedDirectory(volumeC, "\\data");
    store.seedFile(volumeC, file, "base");
    QVERIFY_THROWS_EXCEPTION(app::OperationError, files->commitFile("C:\\data"));
    QVERIFY_THROWS_EXCEPTION(app::OperationError, files->commitFileDeletion("C:\\data"));
    QVERIFY(store.visibleDirectory(volumeC, "\\data"));
    QCOMPARE(store.visibleFile(volumeC, file), std::optional<std::string>{"base"});
    files->commitFileDeletion(file);
    files->commitFileDeletion("C:\\data");
    QVERIFY(!store.visibleDirectory(volumeC, "\\data"));
    const std::string emptyChild = std::string(key) + "\\empty-child";
    store.seedRegistryKey(key);
    store.seedRegistryKey(emptyChild);
    const auto registry = maintenance->prepareRegistryMaintenance();
    QVERIFY_THROWS_EXCEPTION(app::OperationError, registry->commitDeletion(key, std::nullopt));
    QVERIFY(store.visibleRegistryKey(key));
    QVERIFY(store.visibleRegistryKey(emptyChild));
    registry->commitDeletion(emptyChild, std::nullopt);
    registry->commitDeletion(key, std::nullopt);
    store.simulateRestart();
    QVERIFY(!store.visibleDirectory(volumeC, "\\data"));
    QVERIFY(!store.visibleRegistryKey(key));
    QVERIFY(!store.visibleRegistryKey(emptyChild));
  }

  void registryMaintenanceKeepsValueAndWholeKeyValidationSeparate_data() {
    QTest::addColumn<std::string>("targetKey");
    QTest::addColumn<bool>("value");
    QTest::addColumn<bool>("deletion");
    QTest::addColumn<bool>("allowed");
    QTest::newRow("commit-value-in-machine-root") << std::string("HKLM") << true << false << true;
    QTest::newRow("commit-value-in-reserved-key") << std::string("HKLM\\SYSTEM") << true << false << true;
    QTest::newRow("delete-value-in-user-hive") << std::string("HKCU\\Application") << true << true << false;
    QTest::newRow("commit-value-in-user-hive") << std::string("HKCU\\Application") << true << false << false;
    QTest::newRow("delete-user-child-key") << std::string("HKCU\\Application") << false << true << true;
    QTest::newRow("delete-hive-root") << std::string("HKCU") << false << true << false;
    QTest::newRow("delete-reserved-machine-key") << std::string("HKLM\\SYSTEM") << false << true << false;
    QTest::newRow("delete-ancestor-prefix") << std::string("HKLM\\SYS") << false << true << false;
    QTest::newRow("delete-descendant-is-not-blacklisted") << std::string("HKLM\\SYSTEM\\Application") << false << true << true;
  }
  void registryMaintenanceKeepsValueAndWholeKeyValidationSeparate() {
    QFETCH(std::string, targetKey);
    QFETCH(bool, value);
    QFETCH(bool, deletion);
    QFETCH(bool, allowed);
    Fake backend(configured(true));
    backend.seedRegistryValue(targetKey, valueName, "base");
    backend.writeRegistryValue(targetKey, valueName, "changed");
    const auto selected = value ? std::optional<std::string>{valueName} : std::nullopt;
    const compat::Request command =
        deletion ? compat::Request{compat::CommitRegistryDeletion{targetKey, selected}} : compat::Request{compat::CommitRegistry{targetKey, selected}};
    const auto response = backend.execute(command);
    QCOMPARE(response.succeeded(), allowed);
    if (!allowed) {
      QCOMPARE(response.failure->kind, compat::ErrorKind::InvalidArgument);
      QCOMPARE(response.effect, compat::Effect::None);
      QCOMPARE(backend.readRegistryValue(targetKey, valueName), std::optional<std::string>{"base"});
    }
  }

  void activeCommitPackingLimitsDoNotPretendToBeRegistryNameLimits() {
    Fake backend(configured(true));
    const auto longValue = std::string(256, 'v');
    backend.seedRegistryValue(key, longValue, "base");
    backend.writeRegistryValue(key, longValue, "changed");
    const auto result = backend.execute(compat::CommitRegistry{key, longValue});
    QCOMPARE(result.failure->kind, compat::ErrorKind::InvalidArgument);
    QCOMPARE(backend.readRegistryValue(key, longValue), std::optional<std::string>{"base"});
    const auto longFile = "\\" + std::string(1023, 'f');
    backend.seedFile(volumeC, longFile, "base");
    const auto fileResult = backend.execute(compat::CommitFileDeletion{volumeC, longFile});
    QCOMPARE(fileResult.failure->kind, compat::ErrorKind::InvalidArgument);
    QCOMPARE(backend.readFile(volumeC, longFile), std::optional<std::string>{"base"});
  }

  void runtimeExclusionsAndRomHaveSeparateCommitRules_data() {
    QTest::addColumn<bool>("rom");
    QTest::newRow("ordinary-mode-uses-current-exclusions") << false;
    QTest::newRow("rom-ignores-exclusions-and-keeps-horm-separate") << true;
  }
  void runtimeExclusionsAndRomHaveSeparateCommitRules() {
    QFETCH(bool, rom);
    auto initial = configured(true, static_cast<std::uint32_t>(compat::OverlayType::Ram), rom ? compat::flags::readOnlyMedia : 0);
    initial.current.volumes.at(volumeC).fileExclusions = {"\\data"};
    initial.current.registryExclusions = {key};
    Scenario scenario(initial);
    auto& store = *scenario.views[0];
    QVERIFY(compat::call(*scenario.system, compat::FindFileExclusion{volumeC, "\\DATA", compat::Session::Current}));
    QVERIFY(!compat::call(*scenario.system, compat::FindFileExclusion{volumeC, "\\data\\sample.txt", compat::Session::Current}));
    QVERIFY(!compat::call(*scenario.system, compat::FindFileExclusion{volumeC, "\\data", compat::Session::Next}));
    QVERIFY(compat::call(*scenario.system, compat::FindRegistryExclusion{key, compat::Session::Current}));
    QVERIFY(!compat::call(*scenario.system, compat::FindRegistryExclusion{std::string(key) + "\\child", compat::Session::Current}));
    QVERIFY(!compat::call(*scenario.system, compat::FindRegistryExclusion{key, compat::Session::Next}));
    store.seedFile(volumeC, file, "base");
    store.seedRegistryValue(key, valueName, "base");
    store.writeFile(volumeC, file, "changed");
    store.writeRegistryValue(key, valueName, "changed");
    const auto beforeCommit = std::optional<std::string>{rom ? "base" : "changed"};
    QCOMPARE(store.readFile(volumeC, file), beforeCommit);
    QCOMPARE(store.readRegistryValue(key, valueName), beforeCommit);
    // Application admission rejects ROM even when the raw management exclusion check would return S_OK.
    QVERIFY(store.executeCapturedPrimitiveForTest(compat::CommitFile{volumeC, relativeFile}).succeeded());
    QVERIFY(store.executeCapturedPrimitiveForTest(compat::CommitRegistry{key, valueName}).succeeded());
    // ROM suppresses runtime exclusions, but the management commit precheck
    // still treats configured exclusions as an already-persistent no-op.
    QCOMPARE(store.readFile(volumeC, file), std::optional<std::string>{rom ? "base" : "changed"});
    QCOMPARE(store.readRegistryValue(key, valueName), std::optional<std::string>{rom ? "base" : "changed"});
    QCOMPARE(store.visibleFile(volumeC, file), std::optional<std::string>{"changed"});
    QCOMPARE(store.visibleRegistryValue(key, valueName), std::optional<std::string>{"changed"});
  }

  void publicIndividualCommitsRejectCurrentRom_data() {
    QTest::addColumn<IndividualProviderOperation>("operation");
    QTest::addColumn<CommitSessionMode>("mode");
    const std::array operations{std::pair{"file", IndividualProviderOperation::File}, std::pair{"delete-file", IndividualProviderOperation::DeleteFile},
                                std::pair{"registry-value", IndividualProviderOperation::RegistryValue},
                                std::pair{"delete-registry-value", IndividualProviderOperation::DeleteRegistryValue},
                                std::pair{"delete-registry-key", IndividualProviderOperation::DeleteRegistryKey}};
    const std::array modes{std::pair{"ordinary", CommitSessionMode::Ordinary}, std::pair{"next-rom-only", CommitSessionMode::NextRomOnly},
                           std::pair{"current-rom", CommitSessionMode::CurrentRom}, std::pair{"full-commit", CommitSessionMode::FullCommit}};
    for (const auto& [operationName, operation] : operations)
      for (const auto& [modeName, mode] : modes) QTest::newRow((std::string(operationName) + '/' + modeName).c_str()) << operation << mode;
  }
  void publicIndividualCommitsRejectCurrentRom() {
    QFETCH(IndividualProviderOperation, operation);
    QFETCH(CommitSessionMode, mode);
    const bool files = operation == IndividualProviderOperation::File || operation == IndividualProviderOperation::DeleteFile;
    const bool deleting = operation != IndividualProviderOperation::File && operation != IndividualProviderOperation::RegistryValue;
    const bool currentRom = mode == CommitSessionMode::CurrentRom || mode == CommitSessionMode::FullCommit;
    auto initial = configured(true, static_cast<std::uint32_t>(compat::OverlayType::Ram), unknownFlag);
    if (mode != CommitSessionMode::Ordinary) initial.next.overlayFlags |= compat::flags::readOnlyMedia;
    if (currentRom) initial.current.overlayFlags |= compat::flags::readOnlyMedia;
    if (mode == CommitSessionMode::FullCommit) initial.runtime.romDisposition = 4;
    Fake backend(initial);
    backend.seedFile(volumeC, file, "base");
    backend.seedFile(volumeC, file, "changed", Fake::Storage::Overlay);
    backend.seedRegistryValue(key, valueName, "base");
    backend.seedRegistryValue(key, valueName, "changed", Fake::Storage::Overlay);
    const auto request = [&]() -> compat::Request {
      switch (operation) {
        case IndividualProviderOperation::File:
          return compat::CommitFile{volumeC, relativeFile};
        case IndividualProviderOperation::DeleteFile:
          return compat::CommitFileDeletion{volumeC, relativeFile};
        case IndividualProviderOperation::RegistryValue:
          return compat::CommitRegistry{key, std::string(valueName)};
        case IndividualProviderOperation::DeleteRegistryValue:
          return compat::CommitRegistryDeletion{key, std::string(valueName)};
        case IndividualProviderOperation::DeleteRegistryKey:
          return compat::CommitRegistryDeletion{key, std::nullopt};
      }
      throw std::logic_error("Unknown individual provider operation");
    }();
    const auto before = backend.snapshot();
    const auto response = backend.execute(request);
    const bool blocked = currentRom;
    QCOMPARE(response.succeeded(), !blocked);
    if (blocked) {
      QVERIFY(response.failure);
      QCOMPARE(response.failure->kind, compat::ErrorKind::AccessDenied);
      QCOMPARE(response.effect, compat::Effect::None);
      QCOMPARE(backend.snapshot().durable, before.durable);
      QCOMPARE(backend.snapshot(), before);
    } else {
      const auto expected = deleting ? std::optional<std::string>{} : std::optional<std::string>{mode == CommitSessionMode::FullCommit ? "base" : "changed"};
      QCOMPARE(files ? backend.readFile(volumeC, file) : backend.readRegistryValue(key, valueName), expected);
    }
  }

  void fullCommitAndHormKeepDriverDispositionAndReceiptSemantics() {
    for (const bool copyFails : {false, true}) {
      auto initial = configured(true, static_cast<std::uint32_t>(compat::OverlayType::Ram), compat::flags::readOnlyMedia);
      initial.runtime.fullCommitCopyFails = copyFails;
      Fake backend(initial);
      backend.seedFile(volumeC, file, "base");
      backend.seedFile(volumeD, "\\other.txt", "other-base");
      backend.seedRegistryValue(key, valueName, "registry-base");
      backend.writeFile(volumeC, file, "first-half");
      backend.writeFile(volumeD, "\\other.txt", "other-overlay");
      backend.writeRegistryValue(key, valueName, "registry-overlay");
      QVERIFY(backend.execute(compat::SetBoolean{compat::BooleanProperty::OverlayCommit, true}).succeeded());
      QVERIFY(compat::call(backend, compat::GetBoolean{compat::BooleanProperty::OverlayCommit}));
      QCOMPARE(backend.readFile(volumeC, file), std::optional<std::string>{copyFails ? "base" : "first-half"});
      QCOMPARE(backend.readFile(volumeD, "\\other.txt"), std::optional<std::string>{copyFails ? "other-base" : "other-overlay"});
      QCOMPARE(backend.readRegistryValue(key, valueName), std::optional<std::string>{copyFails ? "registry-base" : "registry-overlay"});
      QVERIFY(backend.snapshot().overlay.files.empty());
      // FullCommit forwards later writes even after a copy failure.
      // An append preserves the original prefix only if the initial copy saved it.
      const auto complete = backend.visibleFile(volumeC, file).value() + "+second-half";
      backend.writeFile(volumeC, file, complete);
      backend.writeFile(volumeD, "\\other.txt", "later-file-write");
      backend.writeRegistryValue(key, valueName, "later-registry-write");
      QCOMPARE(backend.readFile(volumeC, file), std::optional<std::string>{copyFails ? "base+second-half" : "first-half+second-half"});
      QCOMPARE(backend.readFile(volumeD, "\\other.txt"), std::optional<std::string>{"later-file-write"});
      QCOMPARE(backend.readRegistryValue(key, valueName), std::optional<std::string>{"later-registry-write"});
      QVERIFY(backend.snapshot().overlay.files.empty());
      QVERIFY(backend.snapshot().overlay.registry.empty());
      backend.simulateRestart();
      QVERIFY(!compat::call(backend, compat::GetBoolean{compat::BooleanProperty::OverlayCommit}));
      backend.writeFile(volumeC, file, "volatile-after-restart");
      backend.writeRegistryValue(key, valueName, "volatile-after-restart");
      QCOMPARE(backend.readFile(volumeC, file), std::optional<std::string>{complete});
      QCOMPARE(backend.readRegistryValue(key, valueName), std::optional<std::string>{"later-registry-write"});
    }
    Fake ordinary(configured(true));
    QVERIFY(ordinary.execute(compat::SetBoolean{compat::BooleanProperty::OverlayCommit, true}).succeeded());
    QVERIFY(!compat::call(ordinary, compat::GetBoolean{compat::BooleanProperty::OverlayCommit}));
    auto initial = configured(true, static_cast<std::uint32_t>(compat::OverlayType::Ram), compat::flags::readOnlyMedia);
    initial.current.registryExclusions = {key};
    Fake horm(initial);
    QVERIFY(horm.execute(compat::SetBoolean{compat::BooleanProperty::HormEnabled, true}).succeeded());
    QCOMPARE(horm.snapshot().runtime.romDisposition, std::uint32_t{2});
    const auto rejected = horm.execute(compat::SetBoolean{compat::BooleanProperty::OverlayCommit, true});
    QCOMPARE(rejected.failure->kind, compat::ErrorKind::Conflict);
    QVERIFY(horm.execute(compat::SetBoolean{compat::BooleanProperty::HormEnabled, false}).succeeded());
    QCOMPARE(horm.snapshot().runtime.romDisposition, std::uint32_t{3});
    horm.writeRegistryValue(key, valueName, "excluded-after-rom-release");
    QCOMPARE(horm.readRegistryValue(key, valueName), std::optional<std::string>{"excluded-after-rom-release"});
  }

  void builtInRegistryPersistenceDoesNotAlterUserExclusionQueries() {
    auto initial = configured(true);
    initial.current.persistDomainSecretKey = true;
    initial.current.persistTscal = true;
    initial.next = initial.current;
    Fake backend(initial);
    for (const auto* path : {"HKEY_LOCAL_MACHINE\\SECURITY\\Policy\\Secrets\\$MACHINE.ACC", "HKEY_LOCAL_MACHINE\\Software\\Microsoft\\MSLicensing"}) {
      backend.writeRegistryValue(path, "value", "durable");
      QCOMPARE(backend.readRegistryValue(path, "value"), std::optional<std::string>{"durable"});
      QVERIFY(!compat::call(backend, compat::FindRegistryExclusion{path}));
    }
    QVERIFY(compat::call(backend, compat::GetRegistryExclusions{}).empty());
  }

  void restartConsumesResetWithoutChangingSavedMode_data() {
    QTest::addColumn<bool>("persistent");
    QTest::addColumn<int>("reset");
    QTest::newRow("disk-discards-overlay") << false << -1;
    QTest::newRow("persistent-disk-retains-overlay") << true << -1;
    QTest::newRow("persistent-disk-clears-once") << true << 1;
    QTest::newRow("current-saved-selector-clears-once") << true << 255;
  }
  void restartConsumesResetWithoutChangingSavedMode() {
    QFETCH(bool, persistent);
    QFETCH(int, reset);
    auto initial =
        configured(true, static_cast<std::uint32_t>(compat::OverlayType::Disk), compat::flags::passthrough | (persistent ? compat::flags::persistent : 0));
    if (reset == 255) initial.current.resetPersistentStateSavedMode = 1;
    Scenario scenario(initial);
    auto& store = *scenario.views[0];
    store.seedFile(volumeC, file, "base");
    store.seedRegistryValue(key, valueName, "base");
    store.writeFile(volumeC, file, "pending");
    store.writeRegistryValue(key, valueName, "pending");
    if (persistent) {
      (void)compat::call(*scenario.system, compat::SetNumber{compat::NumberProperty::ResetPersistentStateSavedMode, 1});
      QCOMPARE(compat::call(*scenario.system, compat::GetNumber{compat::NumberProperty::ResetPersistentState}), std::uint32_t{0});
    }
    if (reset == 1) {
      app::ConfigurationChanges changes;
      changes.overlayPolicy.reset = app::PersistentReset::Clear;
      const auto result = scenario.apply(changes);
      QCOMPARE(result.failedOperations(), 0);
    } else if (reset == 255) {
      (void)compat::call(*scenario.system,
                         compat::SetNumber{compat::NumberProperty::ResetPersistentState, static_cast<std::uint32_t>(compat::ResetMode::Saved)});
    }
    store.simulateRestart();
    const auto expected = persistent && reset < 0 ? "pending" : "base";
    QCOMPARE(store.visibleFile(volumeC, file), std::optional<std::string>{expected});
    QCOMPARE(store.visibleRegistryValue(key, valueName), std::optional<std::string>{expected});
    QCOMPARE(store.readFile(volumeC, file), std::optional<std::string>{"base"});
    QCOMPARE(compat::call(*scenario.system, compat::GetNumber{compat::NumberProperty::ResetPersistentState}), std::uint32_t{0});
    if (persistent) {
      QCOMPARE(compat::call(*scenario.system, compat::GetNumber{compat::NumberProperty::ResetPersistentStateSavedMode, compat::Session::Next}),
               std::uint32_t{1});
      store.writeFile(volumeC, file, "later");
      store.simulateRestart();
      QCOMPARE(store.visibleFile(volumeC, file), std::optional<std::string>{"later"});
    }
  }

  void configurationFailureRemainsFailureAfterItsValueChanged_data() {
    QTest::addColumn<bool>("flags");
    QTest::newRow("filter-side-effect-before-later-failure") << false;
    QTest::newRow("overlay-flags-before-BCD-failure") << true;
  }
  void configurationFailureRemainsFailureAfterItsValueChanged() {
    QFETCH(bool, flags);
    Scenario scenario(configured());
    const compat::Request request = flags ? compat::Request{compat::SetNumber{compat::NumberProperty::OverlayFlags, compat::flags::persistent}}
                                          : compat::Request{compat::SetBoolean{compat::BooleanProperty::FilterEnabled, true}};
    auto& store = *scenario.views[0];
    store.inject({request, {}, Fake::FaultTiming::AfterEffect, compat::ErrorKind::Failed, "A later configuration step failed"});
    const compat::Request command = flags ? compat::Request{compat::SetOverlayOption{compat::OverlayOption::Persistent, true}} : request;
    const auto response = scenario.system->execute(command);
    QVERIFY(response.failure);
    QCOMPARE(response.effect, compat::Effect::Uncertain);
    if (flags)
      QCOMPARE(store.snapshot().next.overlayFlags, compat::flags::persistent);
    else
      QVERIFY(store.snapshot().next.filterEnabled);
    for (std::size_t index = 1; index < scenario.views.size(); ++index)
      for (const auto& entry : scenario.views[index]->trace()) QVERIFY(entry.request != command);
  }

  void runtimeResetKeepsControlReceiptSeparateFromFilterState() {
    Fake backend(configured(true));
    const auto response = backend.execute(compat::SetNumber{compat::NumberProperty::ResetPersistentState, 1});
    QCOMPARE(response.failure->kind, compat::ErrorKind::AccessDenied);
    QCOMPARE(compat::call(backend, compat::GetNumber{compat::NumberProperty::ResetPersistentState}), std::uint32_t{0});
    auto initial = configured(true, static_cast<std::uint32_t>(compat::OverlayType::Disk), compat::flags::persistent);
    initial.runtime.persistentRollbackState = 2;
    backend.seed(initial);
    QCOMPARE(compat::call(backend, compat::GetNumber{compat::NumberProperty::ResetPersistentState, compat::Session::Current}), std::uint32_t{0});
    backend.simulateRestart();
    QCOMPARE(compat::call(backend, compat::GetNumber{compat::NumberProperty::ResetPersistentState, compat::Session::Next}), std::uint32_t{0});
    QCOMPARE(backend.snapshot().runtime.persistentRollbackState, std::uint32_t{2});
    QVERIFY(backend.execute(compat::SetNumber{compat::NumberProperty::ResetPersistentState, 0}).succeeded());
    QCOMPARE(compat::call(backend, compat::GetNumber{compat::NumberProperty::ResetPersistentState}), std::uint32_t{0});
  }

  void unknownMaintenanceResultIsNotReplayed_data() {
    QTest::addColumn<bool>("registry");
    QTest::newRow("file-commit-receipt-lost") << false;
    QTest::newRow("registry-commit-receipt-lost") << true;
  }
  void unknownMaintenanceResultIsNotReplayed() {
    QFETCH(bool, registry);
    Scenario scenario(configured(true));
    auto& store = *scenario.views[0];
    const auto maintenance = app::uwfMaintenanceRepository(scenario.system);
    const compat::Request request =
        registry ? compat::Request{compat::CommitRegistry{key, std::string(valueName)}} : compat::Request{compat::CommitFile{volumeC, relativeFile}};
    store.inject({request, {}, Fake::FaultTiming::AfterEffect, compat::ErrorKind::Failed, "Provider failed after accepting the commit"});
    bool unknown = false;
    try {
      if (registry) {
        store.seedRegistryValue(key, valueName, "base");
        store.writeRegistryValue(key, valueName, "changed");
        maintenance->prepareRegistryMaintenance()->commit(key, valueName);
      } else {
        store.seedFile(volumeC, file, "base");
        store.writeFile(volumeC, file, "changed");
        systemVolume(maintenance)->commitFile(file);
      }
    } catch (const app::OperationError& error) {
      unknown = error.writeEffect() == app::WriteEffect::Uncertain;
    }
    QVERIFY(unknown);
    QCOMPARE(registry ? store.readRegistryValue(key, valueName) : store.readFile(volumeC, file), std::optional<std::string>{"changed"});
    for (std::size_t index = 1; index < scenario.views.size(); ++index)
      for (const auto& entry : scenario.views[index]->trace()) QVERIFY(entry.request != request);
    const auto attempts = store.trace();
    QCOMPARE(std::ranges::count_if(attempts, [&request](const auto& entry) { return entry.request == request; }), std::ptrdiff_t{1});
  }

  void swapfileDiskStepRejectsBeforeCapturedPartialFailure() {
    const auto initial = configured(true);
    Scenario scenario(initial);
    const auto response = scenario.system->execute(compat::CreateSwapfile{volumeD});
    QVERIFY(response.failure);
    QCOMPARE(response.failure->kind, compat::ErrorKind::AccessDenied);
    QCOMPARE(response.effect, compat::Effect::None);
    const auto state = scenario.views[0]->snapshot();
    QCOMPARE(state, initial);
    // Published Current state rejects at the UWF boundary, before routing.
    for (const auto& view : scenario.views) {
      const auto primitives = view->primitiveRequests();
      QVERIFY(std::ranges::none_of(primitives, [](const auto& primitive) { return compat::isWrite(primitive); }));
      QCOMPARE(view->snapshot(), initial);
      QVERIFY(view->trace().empty());
    }
    Fake raw(initial);
    const auto captured = raw.executeCapturedPrimitiveForTest(compat::CreateSwapfile{volumeD});
    QVERIFY(captured.failure);
    QCOMPARE(captured.failure->kind, compat::ErrorKind::AccessDenied);
    QCOMPARE(captured.effect, compat::Effect::Uncertain);
    QCOMPARE(raw.snapshot().next.overlayType, static_cast<std::uint32_t>(compat::OverlayType::Ram));
    QCOMPARE(raw.snapshot().next.volumes.at(volumeD).swapfileSize, std::uint32_t{1});
  }

  void knownUnprotectedSystemVolumeMakesRegistryWritesAlreadyDurable() {
    auto initial = configured(true);
    initial.runtime.systemVolumeId = volumeC;
    initial.current.volumes.at(volumeC).volume.protectedVolume = false;
    Fake backend(initial);
    backend.writeRegistryValue(key, valueName, "already durable");
    QCOMPARE(backend.readRegistryValue(key, valueName), std::optional<std::string>{"already durable"});
    QVERIFY(backend.execute(compat::CommitRegistry{key, std::string(valueName)}).succeeded());
    const auto deletion = backend.execute(compat::CommitRegistryDeletion{key, std::string(valueName)});
    QCOMPARE(deletion.failure->kind, compat::ErrorKind::Conflict);
    QCOMPARE(backend.readRegistryValue(key, valueName), std::optional<std::string>{"already durable"});
  }

  void swapfileAndCapturedResetChangeOnlyTheirModeledState() {
    auto initial = configured();
    initial.next.maximumOverlaySize = 2048;
    initial.capturedSettings = initial.next;
    Scenario scenario(initial);
    (void)compat::call(*scenario.system, compat::CreateSwapfile{volumeC});
    auto observed = scenario.views[2]->snapshot();
    QCOMPARE(observed.next.overlayType, static_cast<std::uint32_t>(compat::OverlayType::Disk));
    QCOMPARE(observed.next.overlayFlags, compat::flags::passthrough);
    (void)compat::call(*scenario.system, compat::SetOverlayOption{compat::OverlayOption::Persistent, true});
    (void)compat::call(*scenario.system, compat::CreateSwapfile{volumeD});
    observed = scenario.views[0]->snapshot();
    QCOMPARE(observed.next.volumes.at(volumeC).swapfileSize, std::uint32_t{0});
    QCOMPARE(observed.next.volumes.at(volumeD).swapfileSize, std::uint32_t{1});
    QCOMPARE(observed.next.overlayFlags, compat::flags::passthrough);
    (void)compat::call(*scenario.system, compat::SetNumber{compat::NumberProperty::OverlayType, static_cast<std::uint32_t>(compat::OverlayType::Ram)});
    QCOMPARE(scenario.views[1]->snapshot().next.overlayType, static_cast<std::uint32_t>(compat::OverlayType::Ram));
    (void)compat::call(*scenario.system, compat::SetNumber{compat::NumberProperty::WarningThreshold, 640});
    for (const auto session : {compat::Session::Current, compat::Session::Next})
      QCOMPARE(compat::call(*scenario.system, compat::GetNumber{compat::NumberProperty::WarningThreshold, session}), std::uint32_t{640});
    (void)compat::call(*scenario.system, compat::ResetSettings{});
    QVERIFY(scenario.views[0]->snapshot().next == *initial.capturedSettings);
    QVERIFY(scenario.views[2]->snapshot().current == initial.current);

    // Public Reset follows the active-persistent-disk prerequisite. Captured
    // no-device no-ops remain raw backend details, not the public command.
    Fake noDevice(configured());
    const auto denied = noDevice.execute(compat::SetNumber{compat::NumberProperty::ResetPersistentState, 1});
    QCOMPARE(denied.failure->kind, compat::ErrorKind::AccessDenied);
    for (const auto value : {0u, 2u}) {
      const auto rejected = noDevice.execute(compat::SetNumber{compat::NumberProperty::ResetPersistentStateSavedMode, value});
      QCOMPARE(rejected.failure->kind, compat::ErrorKind::InvalidArgument);
    }
    QVERIFY(noDevice.execute(compat::SetNumber{compat::NumberProperty::ResetPersistentStateSavedMode, 1}).succeeded());
    QCOMPARE(noDevice.probe(compat::SetNumber{compat::NumberProperty::OverlayFlags, 0}).support, compat::Support::Unsupported);
    QVERIFY(noDevice.execute(compat::SetOverlayOption{compat::OverlayOption::Persistent, true}).succeeded());
    QCOMPARE(noDevice.snapshot().next.overlayFlags, compat::flags::persistent);
  }
};
QTEST_APPLESS_MAIN(UwfFakeIntegrationTests)
#include "UwfFakeIntegrationTests.moc"
