// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QTest>
#include <algorithm>
#include <array>
#include <memory>
#include <optional>
#include <stdexcept>
#include <stop_token>
#include <string>
#include <utility>
#include <vector>

#include "application/configuration/commands/UwfmgrCommandParser.h"
#include "application/console/ConsoleCommandExecutor.h"
#include "application/uwf/CompositeAllVolumeProtection.h"
#include "simulation/uwf/FakeUwfBackend.h"

namespace {
using namespace uwf;
using Fake = uwf::simulation::FakeUwfBackend;
constexpr auto volumeId = "11111111-2222-3333-4444-555555555555";
constexpr auto otherVolumeId = "22222222-2222-3333-4444-555555555555";
constexpr auto registryKey = "HKLM\\SOFTWARE\\UwfConsoleTest";

struct Environment final : app::ConsoleEnvironment {
  mutable std::vector<std::string> volumeOperands;
  mutable std::vector<std::string> fileOperands;
  std::string resolvedDriveLetter = "C:";
  app::ConsoleVolumeTarget resolveVolume(const std::string& operand) const override {
    volumeOperands.push_back(operand);
    return {volumeId, resolvedDriveLetter, true};
  }
  app::ConsoleFileTarget resolveFile(const std::string& operand) const override {
    fileOperands.push_back(operand);
    return {volumeId, operand.starts_with("C:") ? operand.substr(2) : operand};
  }
};

Fake::State initialState(const int profile = 0) {
  Fake::State state;
  state.current.volumes[volumeId] = {.volume = {volumeId, "C:", false, false, false}, .fileExclusions = {}};
  state.runtime.systemVolumeId = volumeId;
  state.runtime.fileMaintenancePrefixes[volumeId] = "C:";
  if (profile != 0) {
    state.current.filterEnabled = true;
    state.current.volumes.at(volumeId).volume.protectedVolume = true;
  }
  if (profile == 1 || profile == 2 || profile == 4) state.current.overlayFlags = compat::flags::readOnlyMedia;
  if (profile == 2) {
    state.runtime.hormEnabled = true;
    state.runtime.romDisposition = compat::disposition::horm;
  }
  if (profile == 3) {
    state.current.overlayType = static_cast<std::uint32_t>(compat::OverlayType::Disk);
    state.current.overlayFlags = compat::flags::persistent;
  }
  state.next = state.current;
  return state;
}

app::UwfmgrCommand parsed(const QStringList& input) {
  std::vector<std::string> arguments;
  for (const auto& argument : input) arguments.push_back(argument.toUtf8().toStdString());
  return app::parseUwfmgrArguments(arguments);
}

const app::ConsoleSection* findSection(const std::vector<app::ConsoleSection>& sections, const std::string& title) {
  for (const auto& section : sections) {
    if (section.title == title) return &section;
    if (const auto* nested = findSection(section.children, title)) return nested;
  }
  return nullptr;
}
const app::ConsoleSection* findSection(const app::ConsoleCommandResult& result, const std::string& title) { return findSection(result.sections, title); }
const app::ConsoleField* findField(const app::ConsoleSection& section, const std::string& name) {
  const auto field = std::ranges::find(section.fields, name, &app::ConsoleField::name);
  return field == section.fields.end() ? nullptr : &*field;
}
bool fieldEquals(const app::ConsoleSection& section, const std::string& name, const std::string& value) {
  const auto* field = findField(section, name);
  return field && !field->failure && field->value == value;
}
bool fieldEquals(const app::ConsoleCommandResult& result, const std::string& title, const std::string& name, const std::string& value) {
  const auto* section = findSection(result, title);
  return section && fieldEquals(*section, name, value);
}
bool fieldFails(const app::ConsoleCommandResult& result, const std::string& title, const std::string& name, const compat::ErrorKind kind) {
  const auto* section = findSection(result, title);
  const auto* field = section ? findField(*section, name) : nullptr;
  return field && field->value.empty() && field->failure && field->failure->kind == kind;
}
bool hasStatus(const app::ConsoleSection& section, const std::string& status) {
  if (fieldEquals(section, "Status", status)) return true;
  return std::ranges::any_of(section.children, [&](const auto& child) { return hasStatus(child, status); });
}
bool sessionStatus(const app::ConsoleCommandResult& result, const std::string& session, const std::string& status) {
  const auto* group = findSection(result, session + " session");
  return group && hasStatus(*group, status);
}
std::vector<std::pair<std::string, std::string>> fields(const app::ConsoleSection& section) {
  std::vector<std::pair<std::string, std::string>> result;
  for (const auto& field : section.fields) result.emplace_back(field.name, field.value);
  return result;
}

class ConsoleCommandExecutorTests final : public QObject {
  Q_OBJECT
 private slots:
  void everyPublicCommandUsesTheCompletePort_data();
  void everyPublicCommandUsesTheCompletePort();
  void reportsKeepSessionAndOriginalCollectionIdentity();
  void aReadFailureIsVisibleAndNeverBecomesAFalseValue();
  void fileAndRegistryMaintenanceUseExactTargets();
  void permissionFailureDoesNotBecomeSuccessfulConfiguration();
  void updateOutcomeControlsExitStatus_data();
  void updateOutcomeControlsExitStatus();
  void cancellationStopsBeforeDispatchAndBetweenReportReads();
  void allProtectionUsesTheExistingCompositeContract();
  void allProtectionExceptionsRetainTheirEffectBoundary();
  void originalFailureDiagnosticsAndUncertainEffectsSurvive();
  void failurePresentationUsesStableEnglishAndExitCodes_data();
  void failurePresentationUsesStableEnglishAndExitCodes();
  void invalidCommandsCannotReachTheBackendOrResolver();
  void bareVolumeSelectorsDoNotBecomeRootWriteIntents();
  void targetedReportsUseAuthoritativeSessionInventory_data();
  void targetedReportsUseAuthoritativeSessionInventory();
  void targetInventoryAndDetailErrorsNeverBecomeAbsence_data();
  void targetInventoryAndDetailErrorsNeverBecomeAbsence();
  void cancellationBetweenTargetInventoryAndDetailsStopsReads();
  void usageDistinguishesInactiveZeroAndFailedObservations_data();
  void usageDistinguishesInactiveZeroAndFailedObservations();
  void unconfirmedEmptyUsageIsReportedAsUnavailable_data();
  void unconfirmedEmptyUsageIsReportedAsUnavailable();
  void overlayReportsKeepScopeUnitsAndRuntimePolicy_data();
  void overlayReportsKeepScopeUnitsAndRuntimePolicy();
  void fullReportReusesFailedFilterObservation();
  void overlayReportsPreserveOpaqueOptionsWithoutRawFlagNoise();
  void readFailuresRemainIndependentAndCountsTrackRequests_data();
  void readFailuresRemainIndependentAndCountsTrackRequests();
  void fullReportsGroupSessionsAndKeepEveryCollectionEntry_data();
  void fullReportsGroupSessionsAndKeepEveryCollectionEntry();
  void swapfileHostMeaningIsSharedAcrossReports_data();
  void swapfileHostMeaningIsSharedAcrossReports();
  void interruptionTracksControlFlowWithoutInventingReadFailures_data();
  void interruptionTracksControlFlowWithoutInventingReadFailures();
  void publicInputReasonsAreLimitedToTheResolutionBoundary_data();
  void publicInputReasonsAreLimitedToTheResolutionBoundary();
};

void ConsoleCommandExecutorTests::everyPublicCommandUsesTheCompletePort_data() {
  QTest::addColumn<QStringList>("arguments");
  QTest::addColumn<int>("profile");
  QTest::addColumn<QString>("operation");
  const auto row = [](const char* name, const QStringList& arguments, const int profile, const char* operation) {
    QTest::newRow(name) << arguments << profile << QString::fromLatin1(operation);
  };
  row("filter-enable", {"filter", "enable"}, 0, "SetFilterEnabled");
  row("filter-disable", {"filter", "disable"}, 0, "SetFilterEnabled");
  row("horm-enable", {"filter", "enable-horm"}, 1, "SetHormEnabled");
  row("horm-disable", {"filter", "disable-horm"}, 2, "SetHormEnabled");
  row("reset-settings", {"filter", "reset-settings"}, 0, "ResetSettings");
  row("volume-protect", {"volume", "protect", "C:"}, 0, "SetVolumeProtection");
  row("volume-unprotect", {"volume", "unprotect", "C:"}, 0, "SetVolumeProtection");
  row("swapfile", {"volume", "create-swapfile", "C:"}, 0, "CreateSwapfile");
  row("volume-report", {"volume", "get-config"}, 0, "GetVolumes");
  row("file-add", {"file", "add-exclusion", "C:\\Data\\Cache"}, 0, "EditFileExclusion");
  row("file-remove", {"file", "remove-exclusion", "C:\\Data\\Cache"}, 0, "EditFileExclusion");
  row("file-list", {"file", "get-exclusions"}, 0, "GetVolumes");
  row("file-commit", {"file", "commit", "C:\\Data\\File"}, 5, "CommitFile");
  row("file-delete", {"file", "commit-delete", "C:\\Data\\File"}, 5, "CommitFileDeletion");
  row("overlay-size", {"overlay", "set-size", "2048"}, 0, "SetMaximumOverlaySize");
  row("overlay-type", {"overlay", "set-type", "Disk"}, 0, "SetOverlayType");
  row("overlay-passthrough", {"overlay", "set-passthrough", "on"}, 0, "SetPassthrough");
  row("overlay-persistent", {"overlay", "set-persistent", "on"}, 0, "SetPersistent");
  row("overlay-rom", {"overlay", "set-rom-mode", "on"}, 0, "SetReadOnlyMedia");
  row("overlay-reset", {"overlay", "reset-persistentstate", "rollback"}, 3, "SetResetPersistentState");
  row("overlay-warning", {"overlay", "set-warningthreshold", "0"}, 0, "SetWarningThreshold");
  row("overlay-critical", {"overlay", "set-criticalthreshold", "1024"}, 0, "SetCriticalThreshold");
  row("overlay-report", {"overlay", "get-config"}, 0, "GetFilterEnabled");
  row("overlay-available", {"overlay", "get-availablespace"}, 5, "GetFilterEnabled");
  row("overlay-consumption", {"overlay", "get-consumption"}, 5, "GetFilterEnabled");
  row("overlay-commit", {"overlay", "commit"}, 4, "SetOverlayCommit");
  row("registry-add", {"registry", "add-exclusion", registryKey}, 0, "EditRegistryExclusion");
  row("registry-remove", {"registry", "remove-exclusion", registryKey}, 0, "EditRegistryExclusion");
  row("registry-list", {"registry", "get-exclusions"}, 0, "GetRegistryExclusions");
  row("registry-commit", {"registry", "commit", registryKey, "Value"}, 5, "CommitRegistry");
  row("registry-delete", {"registry", "commit-delete", registryKey, "Value"}, 5, "CommitRegistryDeletion");
  row("servicing-enable", {"servicing", "enable"}, 0, "SetServicingEnabled");
  row("servicing-disable", {"servicing", "disable"}, 0, "SetServicingEnabled");
  row("windows-update", {"servicing", "update-windows"}, 0, "UpdateWindows");
  row("servicing-report", {"servicing", "get-config"}, 0, "GetServicingEnabled");
  row("full-report", {"get-config"}, 0, "GetHormEnabled");
}

void ConsoleCommandExecutorTests::everyPublicCommandUsesTheCompletePort() {
  QFETCH(QStringList, arguments);
  QFETCH(int, profile);
  QFETCH(QString, operation);
  const auto command = parsed(arguments);
  QCOMPARE(command.parseError, app::ParseError::None);
  Fake system(initialState(profile));
  system.seedFile(volumeId, "\\Data\\File", "Before");
  system.seedRegistryValue(registryKey, "Value", "Before");
  if (profile == 5) {
    system.writeFile(volumeId, "\\Data\\File", "After");
    system.writeRegistryValue(registryKey, "Value", "After");
  }
  Environment environment;
  const auto result = app::executeConsoleCommand(command, system, environment);
  const auto trace = system.trace();
  QVERIFY(!trace.empty());
  QCOMPARE(QString::fromStdString(compat::operationName(trace.front().request)), operation);
  QVERIFY2(!result.failure, result.failure ? result.failure->message.c_str() : "");
  QCOMPARE(result.exitCode, 0);
  QVERIFY(!result.sections.empty());
}

void ConsoleCommandExecutorTests::reportsKeepSessionAndOriginalCollectionIdentity() {
  auto state = initialState();
  state.current.volumes.at(volumeId).fileExclusions = {"\\Current\\", "\\Current\\"};
  state.next.volumes.at(volumeId).fileExclusions = {"\\Next"};
  state.current.registryExclusions = {"HKLM\\SOFTWARE\\Current"};
  state.next.registryExclusions = {"HKLM\\SOFTWARE\\Next"};
  state.next.servicingEnabled = true;
  Fake system(state);
  Environment environment;
  auto result = app::executeConsoleCommand(parsed({"file", "get-exclusions", "C:", "current"}), system, environment);
  QVERIFY(!result.failure);
  const auto* exclusions = findSection(result, "C:");
  QVERIFY(exclusions);
  QCOMPARE(exclusions->items, (std::vector<std::string>{"\\Current\\", "\\Current\\"}));
  const auto trace = system.trace();
  QCOMPARE(trace.size(), std::size_t{2});
  QCOMPARE(trace.front().request, (compat::Request{compat::GetVolumes{compat::Session::Current}}));
  QCOMPARE(trace.back().request, (compat::Request{compat::GetFileExclusions{volumeId, compat::Session::Current}}));
  system.clearTrace();
  result = app::executeConsoleCommand(parsed({"servicing", "get-config", "next"}), system, environment);
  QVERIFY(fieldEquals(result, "Next session", "Servicing", "Enabled"));
  QCOMPARE(system.trace().size(), std::size_t{1});
  QCOMPARE(system.trace().front().request, (compat::Request{compat::GetBoolean{compat::BooleanProperty::ServicingEnabled, compat::Session::Next}}));
  result = app::executeConsoleCommand(parsed({"registry", "get-exclusions", "next"}), system, environment);
  QCOMPARE(result.sections.front().items, state.next.registryExclusions);
}

void ConsoleCommandExecutorTests::aReadFailureIsVisibleAndNeverBecomesAFalseValue() {
  Fake system(initialState());
  system.inject({.request = compat::GetNumber{compat::NumberProperty::OverlayFlags, compat::Session::Current},
                 .operation = {},
                 .kind = compat::ErrorKind::AccessDenied,
                 .message = "Flags are unreadable",
                 .code = 0x80070005});
  Environment environment;
  const auto result = app::executeConsoleCommand(parsed({"overlay", "get-config"}), system, environment);
  QVERIFY(result.failure.has_value());
  QCOMPARE(result.failure->kind, compat::ErrorKind::AccessDenied);
  QCOMPARE(result.exitCode, 5);
  QVERIFY(fieldFails(result, "Current session", "Overlay options", compat::ErrorKind::AccessDenied));
  QCOMPARE(result.failure->message, std::string("Flags are unreadable"));
  QVERIFY(result.publicReason.empty());
  QVERIFY(!fieldEquals(result, "Current session", "Read-only media mode", "Disabled"));
  QVERIFY(fieldEquals(result, "Next session", "Read-only media mode", "Disabled"));
  QCOMPARE(result.effect, compat::Effect::None);
  QVERIFY(!result.failure->diagnostics.empty());
}

void ConsoleCommandExecutorTests::fileAndRegistryMaintenanceUseExactTargets() {
  Fake system(initialState(5));
  system.seedFile(volumeId, "\\Data\\资料 %TEMP% !name!", "Before");
  system.writeFile(volumeId, "\\Data\\资料 %TEMP% !name!", "After");
  system.seedRegistryValue(registryKey, "", "Before");
  system.writeRegistryValue(registryKey, "", "After");
  Environment environment;
  auto result = app::executeConsoleCommand(parsed({"file", "commit", QStringLiteral("C:\\Data\\资料 %TEMP% !name!")}), system, environment);
  QVERIFY2(!result.failure, result.failure ? result.failure->message.c_str() : "");
  QCOMPARE(environment.fileOperands.back(), std::string("C:\\Data\\资料 %TEMP% !name!"));
  QCOMPARE(system.readFile(volumeId, "\\Data\\资料 %TEMP% !name!"), std::optional<std::string>{"After"});
  result = app::executeConsoleCommand(parsed({"registry", "commit", registryKey, ""}), system, environment);
  QVERIFY(!result.failure);
  QCOMPARE(system.readRegistryValue(registryKey, ""), std::optional<std::string>{"After"});
  const auto deletedDefault = parsed({"registry", "commit-delete", registryKey, ""});
  result = app::executeConsoleCommand(deletedDefault, system, environment);
  QVERIFY(result.failure.has_value());
  QCOMPARE(result.failure->kind, compat::ErrorKind::Unsupported);
  QCOMPARE(system.visibleRegistryValue(registryKey, ""), std::optional<std::string>{"After"});
  system.seedRegistryKey("HKLM\\SOFTWARE\\UwfConsoleEmptyKey");
  result = app::executeConsoleCommand(parsed({"registry", "commit-delete", "HKLM\\SOFTWARE\\UwfConsoleEmptyKey"}), system, environment);
  QVERIFY2(!result.failure, result.failure ? result.failure->message.c_str() : "");
  QVERIFY(!system.visibleRegistryKey("HKLM\\SOFTWARE\\UwfConsoleEmptyKey"));
}

void ConsoleCommandExecutorTests::permissionFailureDoesNotBecomeSuccessfulConfiguration() {
  auto state = initialState();
  state.runtime.administrator = false;
  Fake system(state);
  Environment environment;
  const auto result = app::executeConsoleCommand(parsed({"overlay", "set-size", "2048"}), system, environment);
  QVERIFY(result.failure.has_value());
  QCOMPARE(result.exitCode, 5);
  QCOMPARE(result.effect, compat::Effect::None);
  QCOMPARE(system.snapshot().next.maximumOverlaySize, state.next.maximumOverlaySize);
  QVERIFY(result.sections.empty());
}

void ConsoleCommandExecutorTests::updateOutcomeControlsExitStatus_data() {
  QTest::addColumn<compat::WindowsUpdateOutcome>("outcome");
  QTest::addColumn<int>("exitCode");
  QTest::addColumn<bool>("failure");
  QTest::newRow("completed") << compat::WindowsUpdateOutcome::Completed << 0 << false;
  QTest::newRow("restart-required") << compat::WindowsUpdateOutcome::RebootRequired << 3010 << false;
  QTest::newRow("reported-failed") << compat::WindowsUpdateOutcome::Failed << 1 << true;
}

void ConsoleCommandExecutorTests::updateOutcomeControlsExitStatus() {
  QFETCH(compat::WindowsUpdateOutcome, outcome);
  QFETCH(int, exitCode);
  QFETCH(bool, failure);
  auto state = initialState();
  state.windowsUpdateOutcome = outcome;
  Fake system(state);
  Environment environment;
  const auto result = app::executeConsoleCommand(parsed({"servicing", "update-windows"}), system, environment);
  QCOMPARE(result.exitCode, exitCode);
  QCOMPARE(result.failure.has_value(), failure);
  QCOMPARE(system.snapshot().windowsUpdates, std::uint32_t{1});
  QVERIFY(system.snapshot().powerRequests.empty());
}

void ConsoleCommandExecutorTests::cancellationStopsBeforeDispatchAndBetweenReportReads() {
  Fake system(initialState());
  Environment environment;
  std::stop_source stop;
  stop.request_stop();
  auto result = app::executeConsoleCommand(parsed({"file", "commit", "C:\\Data\\File"}), system, environment, {stop.get_token()});
  QCOMPARE(result.exitCode, 1223);
  QVERIFY(system.trace().empty());
  QVERIFY(environment.fileOperands.empty());

  class CancelAfterFirstRead final : public compat::Uwf {
   public:
    Fake backend{initialState()};
    mutable std::stop_source stop;
    compat::Capability probe(const compat::Request& request) const override { return backend.probe(request); }
    compat::Response execute(const compat::Request& request, const compat::Context& context) const override {
      const auto response = backend.execute(request, context);
      stop.request_stop();
      return response;
    }
  } canceled;
  result = app::executeConsoleCommand(parsed({"overlay", "get-config"}), canceled, environment, {canceled.stop.get_token()});
  QCOMPARE(result.exitCode, 1223);
  QCOMPARE(canceled.backend.trace().size(), std::size_t{1});
  QCOMPARE(result.effect, compat::Effect::None);
}

void ConsoleCommandExecutorTests::allProtectionUsesTheExistingCompositeContract() {
  auto state = initialState();
  state.current.volumes[otherVolumeId] = {.volume = {otherVolumeId, "D:", false, false, false}, .fileExclusions = {}};
  state.next = state.current;
  auto system = std::make_shared<Fake>(state);
  int administratorReads = 0;
  int scopeReads = 0;
  auto all = app::composeAllVolumeProtection(system, {.isAdministrator =
                                                          [&](const compat::Context&) {
                                                            ++administratorReads;
                                                            return true;
                                                          },
                                                      .protectScope =
                                                          [&](const bool, const compat::Context&) {
                                                            ++scopeReads;
                                                            return std::vector<std::string>{volumeId, otherVolumeId};
                                                          }});
  Environment environment;
  auto result = app::executeConsoleCommand(parsed({"volume", "protect", "all"}), *system, environment, {}, all.get());
  QVERIFY(result.failure);
  QCOMPARE(result.failure->kind, compat::ErrorKind::Unavailable);
  QCOMPARE(result.effect, compat::Effect::None);
  QVERIFY(system->primitiveRequests().empty());
  QCOMPARE(administratorReads, 0);
  QCOMPARE(scopeReads, 0);

  // Console startup publishes this complete physical scope before executing writes.
  auto facts = system->state();
  facts.runtime.protectScope = compat::Fact<std::vector<std::string>>::known({volumeId, otherVolumeId});
  system->updateState(std::move(facts));
  result = app::executeConsoleCommand(parsed({"volume", "protect", "all"}), *system, environment, {}, all.get());
  QVERIFY2(!result.failure, result.failure ? result.failure->message.c_str() : "");
  QVERIFY(system->snapshot().next.volumes.at(volumeId).volume.protectedVolume);
  QVERIFY(system->snapshot().next.volumes.at(otherVolumeId).volume.protectedVolume);
  QVERIFY(!system->snapshot().current.volumes.at(volumeId).volume.protectedVolume);
  QVERIFY(environment.volumeOperands.empty());
  QCOMPARE(result.sections.front().items.size(), std::size_t{2});
  QCOMPARE(administratorReads, 0);
  QCOMPARE(scopeReads, 0);
  const auto primitives = system->primitiveRequests();
  QCOMPARE(primitives.size(), std::size_t{2});
  QVERIFY(std::ranges::all_of(primitives, [](const auto& request) {
    const auto* protection = std::get_if<compat::SetVolume>(&request);
    return protection && protection->property == compat::VolumeProperty::Protected && protection->value;
  }));
  result = app::executeConsoleCommand(parsed({"volume", "protect", "all"}), *system, environment);
  QCOMPARE(result.exitCode, 1722);
}

void ConsoleCommandExecutorTests::allProtectionExceptionsRetainTheirEffectBoundary() {
  class ThrowingProtection final : public compat::AllVolumeProtection {
   public:
    bool typed = false;
    compat::AllVolumeProtectionObservation read(const compat::GetAllProtected&, const compat::Context&) const override { return {}; }
    compat::AllVolumeProtectionObservation read(const compat::GetAllUnprotected&, const compat::Context&) const override { return {}; }
    compat::AllVolumeProtectionResult execute(const compat::UnprotectAll&, const compat::Context&) const override { return {}; }
    compat::AllVolumeProtectionResult execute(const compat::ProtectAll&, const compat::Context&) const override {
      if (typed)
        throw compat::Error(
            {compat::ErrorKind::AccessDenied, compat::Effect::None, "Admission rejected before writes", {{"Fake", "ProtectAll", 5, "Access denied"}}});
      throw std::runtime_error("The mutating protection port raised an unexpected exception");
    }
  } protection;
  Fake system(initialState());
  Environment environment;
  auto result = app::executeConsoleCommand(parsed({"volume", "protect", "all"}), system, environment, {}, &protection);
  QCOMPARE(result.exitCode, 1);
  QCOMPARE(result.effect, compat::Effect::Uncertain);
  QVERIFY(result.failure.has_value());
  QCOMPARE(result.failure->effect, compat::Effect::Uncertain);
  protection.typed = true;
  result = app::executeConsoleCommand(parsed({"volume", "protect", "all"}), system, environment, {}, &protection);
  QCOMPARE(result.exitCode, 5);
  QCOMPARE(result.effect, compat::Effect::None);
  QVERIFY(result.failure.has_value());
  QCOMPARE(result.failure->diagnostics.front().detail, std::string("Access denied"));
  QVERIFY(system.trace().empty());
}

void ConsoleCommandExecutorTests::originalFailureDiagnosticsAndUncertainEffectsSurvive() {
  class FailedPort final : public compat::Uwf {
   public:
    compat::Capability probe(const compat::Request&) const override { return {compat::Support::Supported, {}}; }
    compat::Response execute(const compat::Request&, const compat::Context&) const override {
      return compat::Response::fail(compat::ErrorKind::Failed, "The write could not be confirmed", compat::Effect::Uncertain,
                                    {{"CSP", "SetFilterEnabled", 0x80070005, "Original transport detail"}});
    }
  } system;
  Environment environment;
  const auto result = app::executeConsoleCommand(parsed({"filter", "enable"}), system, environment);
  QCOMPARE(result.exitCode, 1);
  QCOMPARE(result.effect, compat::Effect::Uncertain);
  QVERIFY(result.failure.has_value());
  QCOMPARE(result.failure->effect, compat::Effect::Uncertain);
  QCOMPARE(result.failure->diagnostics.front().code, std::int64_t{0x80070005});
  QCOMPARE(result.failure->diagnostics.front().detail, std::string("Original transport detail"));
  QVERIFY(result.sections.empty());
}

void ConsoleCommandExecutorTests::failurePresentationUsesStableEnglishAndExitCodes_data() {
  QTest::addColumn<compat::ErrorKind>("kind");
  QTest::addColumn<int>("exitCode");
  QTest::addColumn<QString>("summary");
  QTest::newRow("unsupported") << compat::ErrorKind::Unsupported << 50 << QStringLiteral("Unsupported operation");
  QTest::newRow("unavailable") << compat::ErrorKind::Unavailable << 1722 << QStringLiteral("UWF operation unavailable");
  QTest::newRow("access-denied") << compat::ErrorKind::AccessDenied << 5 << QStringLiteral("Access denied");
  QTest::newRow("not-found") << compat::ErrorKind::NotFound << 1168 << QStringLiteral("Target not found");
  QTest::newRow("invalid-argument") << compat::ErrorKind::InvalidArgument << 87 << QStringLiteral("Invalid argument");
  QTest::newRow("canceled") << compat::ErrorKind::Canceled << 1223 << QStringLiteral("Operation canceled");
  QTest::newRow("resources") << compat::ErrorKind::ResourceExhausted << 8 << QStringLiteral("Insufficient resources");
  QTest::newRow("conflict") << compat::ErrorKind::Conflict << 5023 << QStringLiteral("Operation conflicts with the current state");
  QTest::newRow("protocol") << compat::ErrorKind::Protocol << 13 << QStringLiteral("Invalid UWF response");
  QTest::newRow("failed") << compat::ErrorKind::Failed << 1 << QStringLiteral("Operation failed");
}

void ConsoleCommandExecutorTests::failurePresentationUsesStableEnglishAndExitCodes() {
  QFETCH(compat::ErrorKind, kind);
  QFETCH(int, exitCode);
  QFETCH(QString, summary);
  QCOMPARE(app::consoleFailureExitCode(kind), exitCode);
  QCOMPARE(QString::fromStdString(std::string(app::consoleFailureSummary(kind))), summary);
}

void ConsoleCommandExecutorTests::invalidCommandsCannotReachTheBackendOrResolver() {
  Fake system(initialState());
  Environment environment;
  for (const auto& input : {QStringList{"registry", "commit", registryKey}, QStringList{"file", "commit"}, QStringList{"overlay", "set-size", "4294967296"},
                            QStringList{"filter", "enable", "extra"}}) {
    const auto result = app::executeConsoleCommand(parsed(input), system, environment);
    QCOMPARE(result.exitCode, 87);
    QVERIFY(!result.publicReason.empty());
    QVERIFY(!result.interrupted);
    QCOMPARE(result.successfulReads, std::size_t{0});
    QCOMPARE(result.failedReads, std::size_t{0});
  }
  for (const auto kind : {app::UwfmgrKind::OverlaySetType, app::UwfmgrKind::OverlaySetPersistent, app::UwfmgrKind::OverlayResetPersistentState}) {
    app::UwfmgrCommand command;
    command.kind = kind;
    command.args = {"unknown"};
    QCOMPARE(app::executeConsoleCommand(command, system, environment).exitCode, 87);
  }
  QVERIFY(system.trace().empty());
  QVERIFY(environment.volumeOperands.empty());
  QVERIFY(environment.fileOperands.empty());
}
void ConsoleCommandExecutorTests::bareVolumeSelectorsDoNotBecomeRootWriteIntents() {
  for (const auto* verb : {"add-exclusion", "commit", "commit-delete"}) {
    Fake system(initialState());
    const auto before = system.snapshot();
    Environment environment;
    const auto result = app::executeConsoleCommand(parsed({"file", QString::fromLatin1(verb), "C:"}), system, environment);
    QVERIFY(result.failure.has_value());
    QCOMPARE(result.failure->kind, compat::ErrorKind::InvalidArgument);
    QCOMPARE(result.effect, compat::Effect::None);
    QVERIFY(result.publicReason.empty());
    QVERIFY(system.snapshot() == before);
  }
}

void ConsoleCommandExecutorTests::targetedReportsUseAuthoritativeSessionInventory_data() {
  QTest::addColumn<bool>("files");
  QTest::addColumn<bool>("guid");
  QTest::addColumn<int>("presence");
  for (const bool files : {false, true})
    for (const bool guid : {false, true})
      for (const int presence : {0, 1, 2, 3})
        QTest::newRow(qPrintable(QStringLiteral("%1-%2-current%3-next%4")
                                     .arg(files ? "files" : "volume", guid ? "guid" : "physical-letter")
                                     .arg((presence & 1) != 0)
                                     .arg((presence & 2) != 0)))
            << files << guid << presence;
}

void ConsoleCommandExecutorTests::targetedReportsUseAuthoritativeSessionInventory() {
  QFETCH(bool, files);
  QFETCH(bool, guid);
  QFETCH(int, presence);
  auto state = initialState();
  state.current.volumes.clear();
  state.next.volumes.clear();
  if ((presence & 1) != 0)
    state.current.volumes[volumeId] = {.volume = {volumeId, "C:", true, false, true}, .fileExclusions = {"\\Current\\", "\\Current\\"}, .swapfileSize = 7};
  if ((presence & 2) != 0) state.next.volumes[volumeId] = {.volume = {volumeId, "D:", false, true, false}, .fileExclusions = {"\\Next"}, .swapfileSize = 1};
  for (auto* configuration : {&state.current, &state.next})
    configuration->volumes[otherVolumeId] = {.volume = {otherVolumeId, "E:", true, false, {}}, .fileExclusions = {"\\Unrelated"}};
  Fake system(state);
  const auto before = system.snapshot();
  Environment environment;
  environment.resolvedDriveLetter = guid ? "" : "Z:";
  const auto operand = guid ? QString::fromStdString(compat::volumePath(volumeId)) : QStringLiteral("Z:");
  const auto result = app::executeConsoleCommand(parsed({files ? "file" : "volume", files ? "get-exclusions" : "get-config", operand}), system, environment);
  QCOMPARE(result.exitCode, presence == 0 ? 1168 : 0);
  QCOMPARE(result.failure.has_value(), presence == 0);
  QVERIFY(!result.interrupted);
  if (result.failure) QCOMPARE(result.failure->kind, compat::ErrorKind::NotFound);
  const auto trace = system.trace();
  QCOMPARE(std::ranges::count_if(trace, [](const auto& entry) { return std::holds_alternative<compat::GetVolumes>(entry.request); }), 2);
  QVERIFY(std::ranges::none_of(trace, [](const auto& entry) { return compat::isWrite(entry.request); }));
  for (const auto session : {compat::Session::Current, compat::Session::Next}) {
    const bool current = session == compat::Session::Current;
    const bool present = (presence & (current ? 1 : 2)) != 0;
    const std::string scope = current ? "Current" : "Next";
    QCOMPARE(sessionStatus(result, scope, "Not configured"), !present);
    if (!present) {
      QVERIFY(std::ranges::none_of(trace, [&](const auto& entry) {
        if (const auto* request = std::get_if<compat::GetFileExclusions>(&entry.request)) return request->volume == volumeId && request->session == session;
        if (const auto* request = std::get_if<compat::GetSwapfileSize>(&entry.request)) return request->volume == volumeId && request->session == session;
        if (const auto* request = std::get_if<compat::GetVolumeProperty>(&entry.request)) return request->volume == volumeId && request->session == session;
        return false;
      }));
      continue;
    }
    const auto all = app::executeConsoleCommand(parsed({files ? "file" : "volume", files ? "get-exclusions" : "get-config", "all"}), system, environment);
    QVERIFY(!all.failure);
    const auto* targetedSession = findSection(result, scope + " session");
    const auto* completeSession = findSection(all, scope + " session");
    QVERIFY(targetedSession);
    QVERIFY(completeSession);
    const auto* target = findSection(targetedSession->children, current ? "C:" : "D:");
    const auto* complete = findSection(completeSession->children, current ? "C:" : "D:");
    QVERIFY(target);
    QVERIFY(complete);
    QCOMPARE(fields(*target), fields(*complete));
    QCOMPARE(target->items, complete->items);
    QVERIFY(fieldEquals(*target, "Volume GUID", compat::volumePath(volumeId)));
    if (!files) {
      QVERIFY(fieldEquals(*target, "Drive letter", current ? "C:" : "D:"));
      QVERIFY(fieldEquals(*target, "Binding", current ? "Volume identity" : "Drive letter"));
      QVERIFY(fieldEquals(*target, "Commit pending", current ? "Enabled" : "Disabled"));
      QVERIFY(fieldEquals(*target, "Swapfile host", current ? "Unknown (7)" : "Yes"));
      const auto* exclusions = findSection(target->children, "File exclusions");
      const auto* allExclusions = findSection(complete->children, "File exclusions");
      QVERIFY(exclusions);
      QVERIFY(allExclusions);
      QCOMPARE(exclusions->items, allExclusions->items);
      QCOMPARE(exclusions->items, current ? std::vector<std::string>({"\\Current\\", "\\Current\\"}) : std::vector<std::string>{"\\Next"});
      QVERIFY(!findField(*exclusions, "Volume GUID"));
    }
  }
  QCOMPARE(system.snapshot(), before);
  QCOMPARE(result.effect, compat::Effect::None);
}

void ConsoleCommandExecutorTests::targetInventoryAndDetailErrorsNeverBecomeAbsence_data() {
  QTest::addColumn<bool>("files");
  QTest::addColumn<int>("failurePhase");
  QTest::addColumn<compat::ErrorKind>("failureKind");
  for (const bool files : {false, true})
    for (const int phase : {0, 1, 2}) {
      if (files && phase == 2) continue;
      for (const auto kind : {compat::ErrorKind::AccessDenied, compat::ErrorKind::NotFound})
        QTest::newRow(qPrintable(
            QStringLiteral("%1-phase%2-%3").arg(files ? "files" : "volume").arg(phase).arg(kind == compat::ErrorKind::NotFound ? "missing" : "denied")))
            << files << phase << kind;
    }
}

void ConsoleCommandExecutorTests::targetInventoryAndDetailErrorsNeverBecomeAbsence() {
  QFETCH(bool, files);
  QFETCH(int, failurePhase);
  QFETCH(compat::ErrorKind, failureKind);
  Fake system(initialState());
  const compat::Request failed = failurePhase == 0   ? compat::Request{compat::GetVolumes{compat::Session::Current}}
                                 : failurePhase == 1 ? compat::Request{compat::GetFileExclusions{volumeId, compat::Session::Current}}
                                                     : compat::Request{compat::GetSwapfileSize{volumeId, compat::Session::Current}};
  system.inject({.request = failed, .operation = {}, .kind = failureKind, .message = "Original scope/detail read failure", .code = 5});
  Environment environment;
  const auto before = system.snapshot();
  const auto result = app::executeConsoleCommand(parsed({files ? "file" : "volume", files ? "get-exclusions" : "get-config", "C:"}), system, environment);
  QCOMPARE(result.exitCode, failureKind == compat::ErrorKind::NotFound ? 1168 : 5);
  QVERIFY(result.failure.has_value());
  QCOMPARE(result.failure->kind, failureKind);
  QCOMPARE(result.failure->message, std::string("Original scope/detail read failure"));
  QVERIFY(!sessionStatus(result, "Current", "Not configured"));
  QVERIFY(!sessionStatus(result, "Next", "Not configured"));
  const auto* nextSession = findSection(result, "Next session");
  QVERIFY(nextSession);
  QVERIFY(findSection(nextSession->children, "C:"));
  if (failurePhase == 0) {
    const auto trace = system.trace();
    QVERIFY(std::ranges::none_of(trace, [](const auto& entry) {
      if (const auto* request = std::get_if<compat::GetFileExclusions>(&entry.request)) return request->session == compat::Session::Current;
      if (const auto* request = std::get_if<compat::GetSwapfileSize>(&entry.request)) return request->session == compat::Session::Current;
      return false;
    }));
  }
  QCOMPARE(result.effect, compat::Effect::None);
  QCOMPARE(system.snapshot(), before);
}

void ConsoleCommandExecutorTests::cancellationBetweenTargetInventoryAndDetailsStopsReads() {
  class CancelAfterInventory final : public compat::Uwf {
   public:
    Fake backend{initialState()};
    mutable std::stop_source stop;
    compat::Capability probe(const compat::Request& request) const override { return backend.probe(request); }
    compat::Response execute(const compat::Request& request, const compat::Context& context) const override {
      auto response = backend.execute(request, context);
      if (std::holds_alternative<compat::GetVolumes>(request)) stop.request_stop();
      return response;
    }
  };
  for (const bool files : {false, true}) {
    CancelAfterInventory system;
    Environment environment;
    const auto result = app::executeConsoleCommand(parsed({files ? "file" : "volume", files ? "get-exclusions" : "get-config", "C:"}), system, environment,
                                                   {system.stop.get_token()});
    QCOMPARE(result.exitCode, 1223);
    QCOMPARE(system.backend.trace().size(), std::size_t{1});
    QVERIFY(!sessionStatus(result, "Current", "Not configured"));
    QVERIFY(!sessionStatus(result, "Next", "Not configured"));
    QCOMPARE(result.effect, compat::Effect::None);
  }
}

void ConsoleCommandExecutorTests::usageDistinguishesInactiveZeroAndFailedObservations_data() {
  QTest::addColumn<bool>("available");
  QTest::addColumn<int>("observation");
  for (const bool available : {false, true})
    for (const int observation : {0, 1, 2, 3, 4})
      QTest::newRow(qPrintable(QStringLiteral("%1-observation%2").arg(available ? "available" : "consumption").arg(observation))) << available << observation;
}

void ConsoleCommandExecutorTests::usageDistinguishesInactiveZeroAndFailedObservations() {
  QFETCH(bool, available);
  QFETCH(int, observation);
  auto state = initialState(observation == 0 ? 0 : 5);
  const auto capacity = state.current.maximumOverlaySize;
  const auto zeroUsage = available ? compat::OverlayUsage{capacity, 0} : compat::OverlayUsage{0, capacity};
  state.runtime.usageOverride = observation == 1 ? zeroUsage : compat::OverlayUsage{7, 93};
  Fake system(state);
  if (observation == 3 || observation == 4)
    system.inject({.request = observation == 3 ? compat::Request{compat::GetBoolean{compat::BooleanProperty::FilterEnabled, compat::Session::Current}}
                                               : compat::Request{compat::GetOverlayUsage{}},
                   .operation = {},
                   .kind = compat::ErrorKind::AccessDenied,
                   .message = "Original usage observation failure",
                   .code = 5});
  Environment environment;
  const auto result = app::executeConsoleCommand(parsed({"overlay", available ? "get-availablespace" : "get-consumption"}), system, environment);
  const auto trace = system.trace();
  QVERIFY(!trace.empty());
  QCOMPARE(trace.front().request, (compat::Request{compat::GetBoolean{compat::BooleanProperty::FilterEnabled, compat::Session::Current}}));
  const auto usageCalls = std::ranges::count_if(trace, [](const auto& entry) { return std::holds_alternative<compat::GetOverlayUsage>(entry.request); });
  QCOMPARE(usageCalls, observation == 0 || observation == 3 ? 0 : 1);
  const auto label = available ? "Available space (MB)" : "Consumption (MB)";
  if (observation >= 3) {
    QCOMPARE(result.exitCode, 5);
    QVERIFY(result.failure.has_value());
    QCOMPARE(result.failure->message, std::string("Original usage observation failure"));
    QVERIFY(!fieldEquals(result, "Overlay - Current", label, "Not applicable (UWF is disabled)"));
    QVERIFY(!fieldEquals(result, "Overlay - Current", label, "0"));
  } else {
    QVERIFY(!result.failure);
    QCOMPARE(result.exitCode, 0);
    const auto expected = observation == 0 ? "Not applicable (UWF is disabled)" : observation == 1 ? "0" : available ? "93" : "7";
    QVERIFY(fieldEquals(result, "Overlay - Current", label, expected));
  }
  QCOMPARE(result.effect, compat::Effect::None);
  QVERIFY(std::ranges::none_of(trace, [](const auto& entry) { return compat::isWrite(entry.request); }));
}

void ConsoleCommandExecutorTests::unconfirmedEmptyUsageIsReportedAsUnavailable_data() {
  QTest::addColumn<bool>("available");
  QTest::newRow("consumption") << false;
  QTest::newRow("available") << true;
}

void ConsoleCommandExecutorTests::unconfirmedEmptyUsageIsReportedAsUnavailable() {
  QFETCH(bool, available);
  auto state = initialState(5);
  state.runtime.usageOverride = compat::OverlayUsage{0, 0};
  Fake system(state);
  Environment environment;
  const auto result = app::executeConsoleCommand(parsed({"overlay", available ? "get-availablespace" : "get-consumption"}), system, environment);
  QVERIFY(result.failure);
  QCOMPARE(result.failure->kind, compat::ErrorKind::Unavailable);
  QCOMPARE(result.exitCode, app::consoleFailureExitCode(compat::ErrorKind::Unavailable));
  QVERIFY(fieldFails(result, "Overlay - Current", "Overlay usage", compat::ErrorKind::Unavailable));
  const auto label = available ? "Available space (MB)" : "Consumption (MB)";
  QVERIFY(!fieldEquals(result, "Overlay - Current", label, "0"));
  QVERIFY(!fieldEquals(result, "Overlay - Current", label, "Not applicable (UWF is disabled)"));
  const auto trace = system.trace();
  QCOMPARE(std::ranges::count_if(trace, [](const auto& entry) { return std::holds_alternative<compat::GetOverlayUsage>(entry.request); }), 1);
  QCOMPARE(result.effect, compat::Effect::None);
  QVERIFY(std::ranges::none_of(trace, [](const auto& entry) { return compat::isWrite(entry.request); }));
}

void ConsoleCommandExecutorTests::overlayReportsKeepScopeUnitsAndRuntimePolicy_data() {
  QTest::addColumn<int>("selection");
  QTest::addColumn<int>("filterState");
  QTest::addColumn<bool>("unknownPolicy");
  for (const int selection : {0, 1, 2})
    for (const int filter : {0, 1, 2})
      for (const bool unknown : {false, true})
        QTest::newRow(qPrintable(QStringLiteral("scope%1-filter%2-policy%3").arg(selection).arg(filter).arg(unknown ? "unknown" : "known")))
            << selection << filter << unknown;
}

void ConsoleCommandExecutorTests::overlayReportsKeepScopeUnitsAndRuntimePolicy() {
  QFETCH(int, selection);
  QFETCH(int, filterState);
  QFETCH(bool, unknownPolicy);
  auto state = initialState(filterState == 0 ? 0 : 5);
  state.current.overlayType = static_cast<std::uint32_t>(compat::OverlayType::Disk);
  state.current.maximumOverlaySize = 1024;
  state.current.warningThreshold = 64;
  state.current.criticalThreshold = 128;
  state.current.overlayFlags = compat::flags::passthrough | compat::flags::persistent;
  state.next.filterEnabled = false;
  state.next.overlayType = static_cast<std::uint32_t>(compat::OverlayType::Ram);
  state.next.maximumOverlaySize = 2048;
  state.next.warningThreshold = 777;
  state.next.criticalThreshold = 999;
  state.next.overlayFlags = compat::flags::persistent | compat::flags::readOnlyMedia;
  state.runtime.resetPersistentState = unknownPolicy ? 7U : 1U;
  state.next.resetPersistentStateSavedMode = unknownPolicy ? 9U : 1U;
  Fake system(state);
  if (filterState == 2)
    system.inject({.request = compat::GetBoolean{compat::BooleanProperty::FilterEnabled, compat::Session::Current},
                   .operation = {},
                   .kind = compat::ErrorKind::AccessDenied,
                   .message = "Current filter observation is unavailable",
                   .code = 5});
  Environment environment;
  QStringList arguments{"overlay", "get-config"};
  if (selection != 0) arguments.append(selection == 1 ? "current" : "next");
  const auto result = app::executeConsoleCommand(parsed(arguments), system, environment);
  const bool current = selection != 2;
  const bool next = selection != 1;
  QCOMPARE(result.failure.has_value(), current && filterState == 2);
  QCOMPARE(result.exitCode, current && filterState == 2 ? 5 : 0);
  QCOMPARE(result.title, std::string("Overlay configuration"));
  QCOMPARE(findSection(result, "Current session") != nullptr, current);
  QCOMPARE(findSection(result, "Next session") != nullptr, next);
  if (current) {
    QVERIFY(fieldEquals(result, "Current session", "Overlay type", "Disk"));
    QVERIFY(fieldEquals(result, "Current session", "Maximum size", "1024 MB"));
    QVERIFY(fieldEquals(result, "Current session", "Warning threshold", "64 MB"));
    QVERIFY(fieldEquals(result, "Current session", "Critical threshold", "128 MB"));
    QVERIFY(fieldEquals(result, "Current session", "Free-space passthrough", "Enabled"));
    QVERIFY(fieldEquals(result, "Current session", "Persistent overlay", "Enabled"));
    if (filterState == 1)
      QVERIFY(fieldEquals(result, "Current session", "Reset on next boot", unknownPolicy ? "Unknown (7)" : "Rollback"));
    else if (filterState == 0)
      QVERIFY(fieldEquals(result, "Current session", "Reset on next boot", "Not applicable (UWF is disabled)"));
    else
      QVERIFY(!fieldEquals(result, "Current session", "Reset on next boot", "Not applicable (UWF is disabled)"));
  }
  if (next) {
    QVERIFY(fieldEquals(result, "Next session", "UWF filter", "Disabled"));
    QVERIFY(fieldEquals(result, "Next session", "Overlay type", "RAM"));
    QVERIFY(fieldEquals(result, "Next session", "Maximum size", "2048 MB"));
    QVERIFY(fieldEquals(result, "Next session", "Read-only media mode", "Enabled"));
    QVERIFY(fieldEquals(result, "Next session", "Default reset policy", unknownPolicy ? "Unknown (9)" : "Rollback"));
    QVERIFY(!fieldEquals(result, "Next session", "Warning threshold", "64 MB"));
    QVERIFY(!fieldEquals(result, "Next session", "Critical threshold", "128 MB"));
    QVERIFY(std::ranges::find(result.notes, "Changes to next-session configuration take effect after restarting Windows.") != result.notes.end());
  }
  const auto trace = system.trace();
  const auto count = [&](const compat::NumberProperty property) {
    return std::ranges::count_if(trace, [&](const auto& entry) {
      const auto* request = std::get_if<compat::GetNumber>(&entry.request);
      return request && request->property == property;
    });
  };
  QCOMPARE(count(compat::NumberProperty::WarningThreshold), current ? 1 : 0);
  QCOMPARE(count(compat::NumberProperty::CriticalThreshold), current ? 1 : 0);
  QCOMPARE(count(compat::NumberProperty::ResetPersistentState), current && filterState == 1 ? 1 : 0);
  QCOMPARE(count(compat::NumberProperty::ResetPersistentStateSavedMode), next ? 1 : 0);
  QCOMPARE(result.effect, compat::Effect::None);
}

void ConsoleCommandExecutorTests::fullReportReusesFailedFilterObservation() {
  class FilterReadFailure final : public compat::Uwf {
   public:
    Fake backend{initialState(3)};
    mutable int filterReads = 0;
    compat::Capability probe(const compat::Request& request) const override { return backend.probe(request); }
    compat::Response execute(const compat::Request& request, const compat::Context& context) const override {
      if (request == compat::Request{compat::GetBoolean{compat::BooleanProperty::FilterEnabled, compat::Session::Current}}) {
        ++filterReads;
        return compat::Response::fail(compat::ErrorKind::AccessDenied, "Original filter observation failure");
      }
      return backend.execute(request, context);
    }
  } system;
  Environment environment;
  const auto result = app::executeConsoleCommand(parsed({"get-config", "current"}), system, environment);
  QCOMPARE(result.exitCode, 5);
  QCOMPARE(system.filterReads, 1);
  QVERIFY(fieldFails(result, "Current session", "UWF filter", compat::ErrorKind::AccessDenied));
  const auto* overlay = findSection(result, "Overlay");
  QVERIFY(overlay);
  QVERIFY(!findField(*overlay, "UWF filter"));
  QVERIFY(!fieldEquals(*overlay, "Reset on next boot", "Not applicable (UWF is disabled)"));
  QCOMPARE(result.failedReads, std::size_t{1});
  QCOMPARE(result.failures.size(), std::size_t{1});
  QCOMPARE(result.failures.front().operation, std::string("GetFilterEnabled"));
  const auto trace = system.backend.trace();
  QVERIFY(std::ranges::none_of(trace, [](const auto& entry) {
    const auto* number = std::get_if<compat::GetNumber>(&entry.request);
    return number && number->property == compat::NumberProperty::ResetPersistentState;
  }));
  QCOMPARE(result.effect, compat::Effect::None);
}

void ConsoleCommandExecutorTests::overlayReportsPreserveOpaqueOptionsWithoutRawFlagNoise() {
  struct Options {
    std::uint32_t additional;
    const char* display;
  };
  for (const auto& options : {Options{0, ""}, Options{8, "0x00000008"}, Options{0x80000000U, "0x80000000"}})
    for (const bool next : {false, true}) {
      auto state = initialState();
      (next ? state.next : state.current).overlayFlags = compat::flags::passthrough | compat::flags::readOnlyMedia | options.additional;
      Fake system(state);
      Environment environment;
      const auto result = app::executeConsoleCommand(parsed({"overlay", "get-config", next ? "next" : "current"}), system, environment);
      QVERIFY(!result.failure);
      const auto title = next ? "Next session" : "Current session";
      const auto* section = findSection(result, title);
      QVERIFY(section);
      QVERIFY(std::ranges::none_of(section->fields, [](const auto& field) { return field.name == "Flags"; }));
      const auto additional = std::ranges::find(section->fields, std::string("Additional flags"), &app::ConsoleField::name);
      QCOMPARE(additional != section->fields.end(), options.additional != 0);
      if (additional != section->fields.end()) QCOMPARE(additional->value, std::string(options.display));
      QVERIFY(fieldEquals(result, title, "Free-space passthrough", "Enabled"));
      QVERIFY(fieldEquals(result, title, "Read-only media mode", "Enabled"));
    }
}

void ConsoleCommandExecutorTests::readFailuresRemainIndependentAndCountsTrackRequests_data() {
  QTest::addColumn<int>("failureMask");
  QTest::newRow("complete") << 0;
  QTest::newRow("size-failed") << 1;
  QTest::newRow("options-failed") << 2;
  QTest::newRow("independent-size-and-options-failures") << 3;
  QTest::newRow("canceled-receipt-without-interruption") << 4;
}

void ConsoleCommandExecutorTests::readFailuresRemainIndependentAndCountsTrackRequests() {
  QFETCH(int, failureMask);
  class ReadFailures final : public compat::Uwf {
   public:
    Fake backend{initialState()};
    int mask = 0;
    mutable std::vector<compat::Request> requests;
    compat::Capability probe(const compat::Request& request) const override { return backend.probe(request); }
    compat::Response execute(const compat::Request& request, const compat::Context& context) const override {
      requests.push_back(request);
      if (((mask & 1) != 0 || mask == 4) && request == compat::Request{compat::GetNumber{compat::NumberProperty::MaximumOverlaySize, compat::Session::Current}})
        return compat::Response::fail(mask == 4 ? compat::ErrorKind::Canceled : compat::ErrorKind::AccessDenied, "Original size failure", compat::Effect::None,
                                      {{"WMI", "ReadMaximumSize", mask == 4 ? 0x800704C7 : 0x80070005, "Untranslated native size detail"}});
      if ((mask & 2) != 0 && request == compat::Request{compat::GetNumber{compat::NumberProperty::OverlayFlags, compat::Session::Current}}) {
        auto response = compat::Response::fail(compat::ErrorKind::Failed, "Original option failure", compat::Effect::None,
                                               {{"CSP", "ReadOverlayFlags", 0x8000FFFF, "<provider-error>Original XML detail</provider-error>"}});
        response.warnings = {"Original provider warning"};
        return response;
      }
      return backend.execute(request, context);
    }
  } system;
  system.mask = failureMask;
  Environment environment;
  const auto result = app::executeConsoleCommand(parsed({"overlay", "get-config", "current"}), system, environment);
  const std::vector<compat::Request> expected{compat::GetBoolean{compat::BooleanProperty::FilterEnabled, compat::Session::Current},
                                              compat::GetNumber{compat::NumberProperty::OverlayType, compat::Session::Current},
                                              compat::GetNumber{compat::NumberProperty::MaximumOverlaySize, compat::Session::Current},
                                              compat::GetNumber{compat::NumberProperty::OverlayFlags, compat::Session::Current},
                                              compat::GetNumber{compat::NumberProperty::WarningThreshold, compat::Session::Current},
                                              compat::GetNumber{compat::NumberProperty::CriticalThreshold, compat::Session::Current}};
  QCOMPARE(system.requests, expected);
  const bool sizeFailed = (failureMask & 1) != 0 || failureMask == 4;
  const auto failures = static_cast<std::size_t>(sizeFailed) + static_cast<std::size_t>((failureMask & 2) != 0);
  QCOMPARE(result.failedReads, failures);
  QCOMPARE(result.successfulReads, expected.size() - failures);
  QCOMPARE(result.failures.size(), failures);
  QCOMPARE(result.failure.has_value(), failureMask != 0);
  QCOMPARE(result.exitCode, failureMask == 4 ? 1223 : (failureMask & 1) != 0 ? 5 : (failureMask & 2) != 0 ? 1 : 0);
  QVERIFY(!result.interrupted);
  QCOMPARE(result.effect, compat::Effect::None);
  QVERIFY(result.publicReason.empty());
  QCOMPARE(result.warnings, (failureMask & 2) != 0 ? std::vector<std::string>{"Original provider warning"} : std::vector<std::string>{});
  std::size_t index = 0;
  if (sizeFailed) {
    const auto& failure = result.failures[index++];
    QCOMPARE(failure.operation, std::string("GetMaximumOverlaySize"));
    QCOMPARE(failure.failure.message, std::string("Original size failure"));
    QCOMPARE(failure.failure.diagnostics.size(), std::size_t{1});
    QCOMPARE(failure.failure.diagnostics.front().backend, std::string("WMI"));
    QCOMPARE(failure.failure.diagnostics.front().operation, std::string("ReadMaximumSize"));
    QCOMPARE(failure.failure.diagnostics.front().code, failureMask == 4 ? std::int64_t{0x800704C7} : std::int64_t{0x80070005});
    QCOMPARE(failure.failure.diagnostics.front().detail, std::string("Untranslated native size detail"));
    QVERIFY(fieldFails(result, "Current session", "Maximum size", failureMask == 4 ? compat::ErrorKind::Canceled : compat::ErrorKind::AccessDenied));
  }
  if ((failureMask & 2) != 0) {
    const auto& failure = result.failures[index++];
    QCOMPARE(failure.operation, std::string("GetOverlayFlags"));
    QCOMPARE(failure.failure.message, std::string("Original option failure"));
    QCOMPARE(failure.failure.diagnostics.size(), std::size_t{1});
    QCOMPARE(failure.failure.diagnostics.front().backend, std::string("CSP"));
    QCOMPARE(failure.failure.diagnostics.front().operation, std::string("ReadOverlayFlags"));
    QCOMPARE(failure.failure.diagnostics.front().code, std::int64_t{0x8000FFFF});
    QCOMPARE(failure.failure.diagnostics.front().detail, std::string("<provider-error>Original XML detail</provider-error>"));
    QVERIFY(fieldFails(result, "Current session", "Overlay options", compat::ErrorKind::Failed));
  }
  QCOMPARE(index, failures);
  for (const auto& failure : result.failures) {
    QCOMPARE(failure.failure.effect, compat::Effect::None);
    for (const auto& diagnostic : failure.failure.diagnostics) {
      QVERIFY(!diagnostic.backend.empty());
      QVERIFY(diagnostic.operation != "ConsoleCommand");
      QVERIFY(diagnostic.code != 0);
    }
  }
  if (result.failure) {
    // The primary failure keeps only its original diagnostics. Later failures
    // remain independent rather than being packed into an anonymous record.
    QCOMPARE(result.failure->diagnostics.size(), std::size_t{1});
    QCOMPARE(result.failure->message, result.failures.front().failure.message);
    QCOMPARE(result.failure->diagnostics.front().detail, result.failures.front().failure.diagnostics.front().detail);
  }
  QVERIFY(fieldEquals(result, "Current session", "UWF filter", "Disabled"));
  QVERIFY(fieldEquals(result, "Current session", "Warning threshold", "512 MB"));
  QVERIFY(fieldEquals(result, "Current session", "Critical threshold", "768 MB"));
}

void ConsoleCommandExecutorTests::fullReportsGroupSessionsAndKeepEveryCollectionEntry_data() {
  QTest::addColumn<int>("selection");
  QTest::newRow("both") << 0;
  QTest::newRow("current") << 1;
  QTest::newRow("next") << 2;
}

void ConsoleCommandExecutorTests::fullReportsGroupSessionsAndKeepEveryCollectionEntry() {
  QFETCH(int, selection);
  auto state = initialState(5);
  state.current.volumes.at(volumeId).fileExclusions = {"\\Current\\", "\\Current\\"};
  state.next.volumes.at(volumeId).volume.driveLetter = "D:";
  state.next.volumes.at(volumeId).volume.bindByDriveLetter = true;
  state.next.volumes.at(volumeId).fileExclusions = {"\\Next"};
  state.current.volumes[otherVolumeId] = {.volume = {otherVolumeId, {}, false, false, {}}, .fileExclusions = {}};
  state.next.volumes[otherVolumeId] = state.current.volumes.at(otherVolumeId);
  state.current.registryExclusions = {"HKLM\\SOFTWARE\\Current"};
  state.next.registryExclusions = {"HKLM\\SOFTWARE\\Next"};
  state.next.servicingEnabled = true;
  Fake system(state);
  const auto before = system.snapshot();
  Environment environment;
  QStringList arguments{"get-config"};
  if (selection != 0) arguments.append(selection == 1 ? "current" : "next");
  const auto result = app::executeConsoleCommand(parsed(arguments), system, environment);
  QVERIFY2(!result.failure, result.failure ? result.failure->message.c_str() : "");
  QCOMPARE(result.title, std::string("UWF configuration"));
  QCOMPARE(result.sections.size(), selection == 0 ? std::size_t{3} : std::size_t{2});
  QCOMPARE(result.sections.front().title, std::string("Runtime"));
  QCOMPARE(result.sections.front().fields.size(), std::size_t{3});
  const auto trace = system.trace();
  QCOMPARE(result.successfulReads, trace.size());
  QCOMPARE(result.failedReads, std::size_t{0});
  QVERIFY(result.failures.empty());
  for (const auto session : {compat::Session::Current, compat::Session::Next}) {
    const bool current = session == compat::Session::Current;
    const bool included = selection == 0 || current == (selection == 1);
    const auto* group = findSection(result, current ? "Current session" : "Next session");
    QCOMPARE(group != nullptr, included);
    const auto filterReads = std::ranges::count_if(
        trace, [session](const auto& entry) { return entry.request == compat::Request{compat::GetBoolean{compat::BooleanProperty::FilterEnabled, session}}; });
    QCOMPARE(filterReads, included ? 1 : 0);
    if (!included) continue;
    QCOMPARE(group->fields.size(), std::size_t{2});
    QVERIFY(fieldEquals(*group, "UWF filter", "Enabled"));
    QVERIFY(fieldEquals(*group, "Servicing", current ? "Disabled" : "Enabled"));
    QCOMPARE(group->children.size(), std::size_t{3});
    QCOMPARE(group->children[0].title, std::string("Overlay"));
    QCOMPARE(group->children[1].title, std::string("Volumes"));
    QCOMPARE(group->children[2].title, std::string("Registry exclusions"));
    const auto& overlay = group->children[0];
    QVERIFY(!findField(overlay, "UWF filter"));
    QCOMPARE(findField(overlay, "Warning threshold") != nullptr, current);
    const auto& volumes = group->children[1];
    QVERIFY(fieldEquals(volumes, "Count", "2"));
    QCOMPARE(volumes.children.size(), std::size_t{2});
    const auto* volume = findSection(volumes.children, current ? "C:" : "D:");
    QVERIFY(volume);
    QVERIFY(fieldEquals(*volume, "Volume GUID", compat::volumePath(volumeId)));
    QVERIFY(fieldEquals(*volume, "Drive letter", current ? "C:" : "D:"));
    QVERIFY(fieldEquals(*volume, "Binding", current ? "Volume identity" : "Drive letter"));
    const auto* exclusions = findSection(volume->children, "File exclusions");
    QVERIFY(exclusions);
    QVERIFY(fieldEquals(*exclusions, "Count", current ? "2" : "1"));
    QCOMPARE(exclusions->items, current ? std::vector<std::string>({"\\Current\\", "\\Current\\"}) : std::vector<std::string>{"\\Next"});
    QVERIFY(!findField(*exclusions, "Volume GUID"));
    const auto* letterless = findSection(volumes.children, compat::volumePath(otherVolumeId));
    QVERIFY(letterless);
    QVERIFY(fieldEquals(*letterless, "Volume GUID", compat::volumePath(otherVolumeId)));
    QVERIFY(fieldEquals(*letterless, "Drive letter", "(none reported)"));
    const auto* empty = findSection(letterless->children, "File exclusions");
    QVERIFY(empty);
    QVERIFY(fieldEquals(*empty, "Count", "0"));
    QVERIFY(empty->items.empty());
    const auto& registry = group->children[2];
    QVERIFY(fieldEquals(registry, "Count", "1"));
    QCOMPARE(registry.items, current ? state.current.registryExclusions : state.next.registryExclusions);
  }
  QCOMPARE(result.effect, compat::Effect::None);
  QCOMPARE(system.snapshot(), before);
}

void ConsoleCommandExecutorTests::swapfileHostMeaningIsSharedAcrossReports_data() {
  QTest::addColumn<int>("report");
  QTest::addColumn<bool>("current");
  QTest::addColumn<int>("observation");
  constexpr std::array reports{"target-volume", "all-volumes", "full-configuration"};
  constexpr std::array observations{"zero", "one", "unknown", "read-failed"};
  for (std::size_t report = 0; report < reports.size(); ++report)
    for (const bool current : {false, true})
      for (std::size_t observation = 0; observation < observations.size(); ++observation)
        QTest::newRow(qPrintable(
            QStringLiteral("%1/%2/%3").arg(QString::fromLatin1(reports[report]), current ? "Current" : "Next", QString::fromLatin1(observations[observation]))))
            << static_cast<int>(report) << current << static_cast<int>(observation);
}

void ConsoleCommandExecutorTests::swapfileHostMeaningIsSharedAcrossReports() {
  QFETCH(int, report);
  QFETCH(bool, current);
  QFETCH(int, observation);
  auto state = initialState();
  const auto marker = observation < 2 ? static_cast<std::uint32_t>(observation) : std::uint32_t{7};
  (current ? state.current : state.next).volumes.at(volumeId).swapfileSize = marker;
  Fake system(state);
  const auto before = system.snapshot();
  const auto session = current ? compat::Session::Current : compat::Session::Next;
  if (observation == 3)
    system.inject({.request = compat::GetSwapfileSize{volumeId, session},
                   .operation = {},
                   .kind = compat::ErrorKind::AccessDenied,
                   .message = "Original host marker read failure",
                   .code = 0x80070005});
  Environment environment;
  const QStringList arguments = report == 0   ? QStringList{"volume", "get-config", "C:"}
                                : report == 1 ? QStringList{"volume", "get-config", "all"}
                                              : QStringList{"get-config", current ? "current" : "next"};
  const auto result = app::executeConsoleCommand(parsed(arguments), system, environment);
  const auto* group = findSection(result, current ? "Current session" : "Next session");
  QVERIFY(group);
  const auto* volume = findSection(group->children, "C:");
  QVERIFY(volume);
  const auto* host = findField(*volume, "Swapfile host");
  QVERIFY(host);
  QVERIFY(!findField(*volume, "Swapfile host marker"));
  if (observation == 3) {
    QVERIFY(host->failure);
    QVERIFY(host->value.empty());
    QCOMPARE(host->failure->kind, compat::ErrorKind::AccessDenied);
    QCOMPARE(host->failure->message, std::string("Original host marker read failure"));
    QCOMPARE(host->failure->diagnostics.front().code, std::int64_t{0x80070005});
    QCOMPARE(result.exitCode, 5);
    QCOMPARE(result.failedReads, std::size_t{1});
    QCOMPARE(result.failures.size(), std::size_t{1});
    QVERIFY(result.publicReason.empty());
  } else {
    QVERIFY(!host->failure);
    QCOMPARE(host->value, std::string(observation == 0 ? "No" : observation == 1 ? "Yes" : "Unknown (7)"));
    QVERIFY(host->value.find("MB") == std::string::npos);
    QVERIFY(!result.failure);
    QCOMPARE(result.exitCode, 0);
    QCOMPARE(result.failedReads, std::size_t{0});
  }
  QCOMPARE(result.successfulReads + result.failedReads, system.trace().size());
  QCOMPARE(result.effect, compat::Effect::None);
  QCOMPARE(system.snapshot(), before);
}

void ConsoleCommandExecutorTests::interruptionTracksControlFlowWithoutInventingReadFailures_data() {
  QTest::addColumn<int>("stopPoint");
  QTest::newRow("before-first-request") << 0;
  QTest::newRow("after-successful-field") << 1;
  QTest::newRow("after-failed-field") << 2;
}

void ConsoleCommandExecutorTests::interruptionTracksControlFlowWithoutInventingReadFailures() {
  QFETCH(int, stopPoint);
  class InterruptedRead final : public compat::Uwf {
   public:
    Fake backend{initialState()};
    int stopPoint = 0;
    mutable std::stop_source stop;
    mutable int requests = 0;
    compat::Capability probe(const compat::Request& request) const override { return backend.probe(request); }
    compat::Response execute(const compat::Request& request, const compat::Context& context) const override {
      ++requests;
      auto response = stopPoint == 2 ? compat::Response::fail(compat::ErrorKind::AccessDenied, "Original field failure before cancellation",
                                                              compat::Effect::None, {{"WMI", "GetFilterEnabled", 0x80070005, "Original access diagnosis"}})
                                     : backend.execute(request, context);
      stop.request_stop();
      return response;
    }
  } system;
  system.stopPoint = stopPoint;
  if (stopPoint == 0) system.stop.request_stop();
  Environment environment;
  const auto result = app::executeConsoleCommand(parsed({"overlay", "get-config", "current"}), system, environment, {system.stop.get_token()});
  QVERIFY(result.interrupted);
  QCOMPARE(system.requests, stopPoint == 0 ? 0 : 1);
  QCOMPARE(result.successfulReads, stopPoint == 1 ? std::size_t{1} : std::size_t{0});
  QCOMPARE(result.failedReads, stopPoint == 2 ? std::size_t{1} : std::size_t{0});
  QCOMPARE(result.successfulReads + result.failedReads, static_cast<std::size_t>(system.requests));
  QCOMPARE(result.failures.size(), stopPoint == 2 ? std::size_t{2} : std::size_t{1});
  QVERIFY(result.failure);
  QCOMPARE(result.failure->kind, stopPoint == 2 ? compat::ErrorKind::AccessDenied : compat::ErrorKind::Canceled);
  QCOMPARE(result.exitCode, stopPoint == 2 ? 5 : 1223);
  QCOMPARE(result.failures.back().failure.kind, compat::ErrorKind::Canceled);
  if (stopPoint == 1) QVERIFY(fieldEquals(result, "Current session", "UWF filter", "Disabled"));
  if (stopPoint == 2) {
    QVERIFY(fieldFails(result, "Current session", "UWF filter", compat::ErrorKind::AccessDenied));
    QCOMPARE(result.failure->message, std::string("Original field failure before cancellation"));
    QCOMPARE(result.failure->diagnostics.size(), std::size_t{1});
    QCOMPARE(result.failure->diagnostics.front().detail, std::string("Original access diagnosis"));
    QCOMPARE(result.failures.front().operation, std::string("GetFilterEnabled"));
  }
}

void ConsoleCommandExecutorTests::publicInputReasonsAreLimitedToTheResolutionBoundary_data() {
  QTest::addColumn<bool>("volume");
  QTest::addColumn<bool>("nativeDiagnostic");
  for (const bool volume : {false, true})
    for (const bool diagnostic : {false, true})
      QTest::newRow(qPrintable(QStringLiteral("%1/%2").arg(volume ? "volume" : "file", diagnostic ? "native-diagnostic" : "input-contract")))
          << volume << diagnostic;
}

void ConsoleCommandExecutorTests::publicInputReasonsAreLimitedToTheResolutionBoundary() {
  QFETCH(bool, volume);
  QFETCH(bool, nativeDiagnostic);
  class InvalidTarget final : public app::ConsoleEnvironment {
   public:
    bool nativeDiagnostic = false;
    [[noreturn]] void reject() const {
      std::vector<compat::Diagnostic> diagnostics;
      if (nativeDiagnostic) diagnostics.push_back({"Windows", "Resolve", 87, "Original native resolver diagnostic"});
      throw compat::Error(
          {compat::ErrorKind::InvalidArgument, compat::Effect::None, "A local absolute path with an explicit volume is required", std::move(diagnostics)});
    }
    app::ConsoleVolumeTarget resolveVolume(const std::string&) const override { reject(); }
    app::ConsoleFileTarget resolveFile(const std::string&) const override { reject(); }
  } environment;
  environment.nativeDiagnostic = nativeDiagnostic;
  Fake system(initialState());
  const auto result = app::executeConsoleCommand(parsed(volume ? QStringList{"volume", "protect", "C:"} : QStringList{"file", "commit", "C:\\Data\\file"}),
                                                 system, environment);
  QVERIFY(result.failure);
  QCOMPARE(result.failure->kind, compat::ErrorKind::InvalidArgument);
  QCOMPARE(result.exitCode, 87);
  QCOMPARE(result.publicReason, nativeDiagnostic ? std::string{} : std::string("A local absolute path with an explicit volume is required"));
  QCOMPARE(result.failure->diagnostics.size(), nativeDiagnostic ? std::size_t{1} : std::size_t{0});
  QCOMPARE(result.successfulReads, std::size_t{0});
  QCOMPARE(result.failedReads, std::size_t{0});
  QVERIFY(system.trace().empty());
}

}  // namespace
QTEST_APPLESS_MAIN(ConsoleCommandExecutorTests)
#include "ConsoleCommandExecutorTests.moc"
