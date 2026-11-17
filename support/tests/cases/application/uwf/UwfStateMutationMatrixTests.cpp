// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QTest>
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <utility>
#include <vector>

#include "UwfStateConsistencyFixture.h"

namespace {
using namespace uwf;
using namespace uwf::compat;
using namespace uwf::test::state_consistency;
constexpr auto filePath = "\\matrix.bin";
constexpr auto registryKey = "HKEY_LOCAL_MACHINE\\SOFTWARE\\StateMatrix";
constexpr auto registryValue = "Value";
constexpr auto originalRegistryExclusion = "HKEY_LOCAL_MACHINE\\SOFTWARE\\Existing";

struct Mutation {
  std::string name;
  Request request;
  Fake::State initial;
  int refreshes = 0;
  bool emptyClear = false;
};

std::vector<Mutation> mutations() {
  std::vector<Mutation> result;
  const auto add = [&](std::string name, Request request, Fake::State initial, const int refreshes = 0, const bool emptyClear = false) {
    result.push_back({std::move(name), std::move(request), std::move(initial), refreshes, emptyClear});
  };
  for (const auto property : {BooleanProperty::PersistDomainSecretKey, BooleanProperty::PersistTscal}) {
    for (const bool enabled : {false, true}) {
      auto state = initialState();
      (property == BooleanProperty::PersistDomainSecretKey ? state.next.persistDomainSecretKey : state.next.persistTscal) = !enabled;
      add(std::string(property == BooleanProperty::PersistDomainSecretKey ? "domain" : "tscal") + (enabled ? "-enable" : "-disable"),
          SetBoolean{property, enabled}, std::move(state));
    }
  }
  for (const auto type : {OverlayType::Ram, OverlayType::Disk}) {
    auto state = initialState();
    state.next.overlayType = static_cast<std::uint32_t>(type == OverlayType::Ram ? OverlayType::Disk : OverlayType::Ram);
    state.next.overlayFlags = type == OverlayType::Ram ? 0x80000003u : 0x80000000u;
    add(type == OverlayType::Ram ? "type-ram" : "type-disk", SetNumber{NumberProperty::OverlayType, static_cast<std::uint32_t>(type)}, std::move(state));
  }
  add("maximum-size", SetNumber{NumberProperty::MaximumOverlaySize, 6144}, initialState());
  add("warning-threshold", SetNumber{NumberProperty::WarningThreshold, 600}, initialState());
  add("critical-threshold", SetNumber{NumberProperty::CriticalThreshold, 900}, initialState());
  for (const auto mode : {ResetMode::Preserve, ResetMode::Clear, ResetMode::Saved}) {
    auto state = initialState();
    state.current.filterEnabled = true;
    state.next = state.current;
    state.current.resetPersistentStateSavedMode = 1;
    state.runtime.resetPersistentState = state.runtime.persistentRollbackState = mode == ResetMode::Preserve ? 1u : 0u;
    add(mode == ResetMode::Preserve ? "reset-preserve"
        : mode == ResetMode::Clear  ? "reset-clear"
                                    : "reset-saved",
        SetNumber{NumberProperty::ResetPersistentState, static_cast<std::uint32_t>(mode)}, std::move(state), 1);
  }
  add("saved-reset-mode", SetNumber{NumberProperty::ResetPersistentStateSavedMode, 1}, initialState());
  for (const auto option : {OverlayOption::Passthrough, OverlayOption::Persistent, OverlayOption::ReadOnlyMedia}) {
    const auto mask = option == OverlayOption::Passthrough  ? flags::passthrough
                      : option == OverlayOption::Persistent ? flags::persistent
                                                            : flags::readOnlyMedia;
    for (const bool enabled : {false, true}) {
      auto state = initialState();
      state.next.overlayType = static_cast<std::uint32_t>(option == OverlayOption::ReadOnlyMedia ? OverlayType::Ram : OverlayType::Disk);
      state.next.overlayFlags = 0x80000000u | (option == OverlayOption::Persistent ? flags::passthrough : 0u) | (enabled ? 0u : mask);
      const auto name = option == OverlayOption::Passthrough ? "passthrough" : option == OverlayOption::Persistent ? "persistent" : "rom";
      add(std::string(name) + (enabled ? "-enable" : "-disable"), SetOverlayOption{option, enabled}, std::move(state));
    }
  }
  add("volume-unprotect", SetVolume{volumeC, VolumeProperty::Protected, false}, initialState());
  add("volume-bind-by-letter", SetVolume{volumeC, VolumeProperty::BindByDriveLetter, true}, initialState());
  for (const bool registry : {false, true}) {
    for (const auto action : {ExclusionAction::Add, ExclusionAction::Remove, ExclusionAction::Clear}) {
      auto state = initialState();
      const auto name = action == ExclusionAction::Add ? "add" : action == ExclusionAction::Remove ? "remove" : "empty-clear";
      if (action == ExclusionAction::Clear) {
        if (registry)
          state.next.registryExclusions.clear();
        else
          state.next.volumes.at(volumeC).fileExclusions.clear();
      }
      Request request = registry ? Request{EditRegistryExclusion{action == ExclusionAction::Add ? "HKLM\\SOFTWARE\\Zed" : originalRegistryExclusion, action}}
                                 : Request{EditFileExclusion{volumeC, action == ExclusionAction::Add ? "\\Zed" : "\\Existing", action}};
      add(std::string(registry ? "registry-" : "file-") + name, std::move(request), std::move(state), 0, action == ExclusionAction::Clear);
    }
  }
  for (const auto type : {OverlayType::Ram, OverlayType::Disk}) {
    auto state = initialState();
    state.next.overlayType = static_cast<std::uint32_t>(type);
    state.next.overlayFlags = 0x80000002u;
    state.next.volumes.at(volumeC).swapfileSize = 9;
    state.next.volumes.at(volumeD).swapfileSize = 1;
    add(type == OverlayType::Ram ? "swap-existing-ram" : "swap-existing-disk", CreateSwapfile{volumeC}, std::move(state), 1);
  }
  for (const int operation : {0, 1, 2, 3}) {
    auto state = initialState();
    state.current.filterEnabled = true;
    state.next = state.current;
    Request request = CommitFile{volumeC, filePath};
    const char* name = "commit-file";
    if (operation == 1) {
      request = CommitFileDeletion{volumeC, filePath};
      name = "commit-file-deletion";
    }
    if (operation == 2) {
      request = CommitRegistry{registryKey, registryValue};
      name = "commit-registry";
    }
    if (operation == 3) {
      request = CommitRegistryDeletion{registryKey, registryValue};
      name = "commit-registry-deletion";
    }
    add(name, std::move(request), std::move(state));
  }
  for (const auto outcome : {WindowsUpdateOutcome::Completed, WindowsUpdateOutcome::RebootRequired, WindowsUpdateOutcome::Failed}) {
    auto state = initialState();
    state.windowsUpdateOutcome = outcome;
    add("windows-update-" + std::to_string(static_cast<std::uint32_t>(outcome)), UpdateWindows{}, std::move(state));
  }
  for (const auto action : {PowerAction::Shutdown, PowerAction::Restart}) {
    auto state = initialState();
    state.current.filterEnabled = true;
    state.next = state.current;
    add(action == PowerAction::Shutdown ? "shutdown" : "restart", Power{action}, std::move(state));
  }
  return result;
}

Request rawTarget(const Request& request, const Fake::State& state) {
  if (const auto* option = std::get_if<SetOverlayOption>(&request)) {
    const auto mask = option->option == OverlayOption::Passthrough  ? flags::passthrough
                      : option->option == OverlayOption::Persistent ? flags::persistent
                                                                    : flags::readOnlyMedia;
    return SetNumber{NumberProperty::OverlayFlags, option->value ? state.next.overlayFlags | mask : state.next.overlayFlags & ~mask};
  }
  return request;
}

bool dataCommit(const Request& request) {
  return std::holds_alternative<CommitFile>(request) || std::holds_alternative<CommitFileDeletion>(request) ||
         std::holds_alternative<CommitRegistry>(request) || std::holds_alternative<CommitRegistryDeletion>(request);
}

void prepareVisibleTarget(const Request& request, Fake& native) {
  if (std::holds_alternative<CommitFile>(request) || std::holds_alternative<CommitFileDeletion>(request)) {
    native.seedFile(volumeC, filePath, "before");
    native.writeFile(volumeC, filePath, "after");
  }
  if (std::holds_alternative<CommitRegistry>(request) || std::holds_alternative<CommitRegistryDeletion>(request)) {
    native.seedRegistryValue(registryKey, registryValue, "before");
    native.writeRegistryValue(registryKey, registryValue, "after");
  }
}

void verifyNativeMutation(const Request& request, const Fake::State& before, const Fixture& fixture, const Response& response) {
  const auto after = fixture.native->snapshot();
  if (const auto* boolean = std::get_if<SetBoolean>(&request)) {
    if (boolean->property == BooleanProperty::PersistDomainSecretKey) {
      QCOMPARE(after.next.persistDomainSecretKey, boolean->value);
      QCOMPARE(after.current.persistDomainSecretKey, before.current.persistDomainSecretKey);
    } else {
      QCOMPARE(after.next.persistTscal, boolean->value);
      QCOMPARE(after.current.persistTscal, before.current.persistTscal);
    }
  } else if (const auto* number = std::get_if<SetNumber>(&request)) {
    switch (number->property) {
      case NumberProperty::OverlayType:
        QCOMPARE(after.next.overlayType, number->value);
        if (number->value == static_cast<std::uint32_t>(OverlayType::Ram))
          QCOMPARE(after.next.overlayFlags & (flags::passthrough | flags::persistent), 0u);
        else
          QVERIFY((after.next.overlayFlags & flags::passthrough) != 0);
        break;
      case NumberProperty::MaximumOverlaySize:
        QCOMPARE(after.next.maximumOverlaySize, number->value);
        break;
      case NumberProperty::WarningThreshold:
        QCOMPARE(after.current.warningThreshold, number->value);
        QCOMPARE(after.next.warningThreshold, number->value);
        break;
      case NumberProperty::CriticalThreshold:
        QCOMPARE(after.current.criticalThreshold, number->value);
        QCOMPARE(after.next.criticalThreshold, number->value);
        break;
      case NumberProperty::ResetPersistentState: {
        const auto desired = number->value == static_cast<std::uint32_t>(ResetMode::Saved) ? before.current.resetPersistentStateSavedMode : number->value;
        QCOMPARE(after.runtime.resetPersistentState, desired);
        // The header is native model data, deliberately absent from GlobalFacts.
        QCOMPARE(after.runtime.persistentRollbackState, desired);
        break;
      }
      case NumberProperty::ResetPersistentStateSavedMode:
        QCOMPARE(after.next.resetPersistentStateSavedMode, number->value);
        QCOMPARE(after.current.resetPersistentStateSavedMode, before.current.resetPersistentStateSavedMode);
        QCOMPARE(after.runtime.resetPersistentState, before.runtime.resetPersistentState);
        break;
      default:
        QFAIL("Unexpected numeric mutation in the success matrix");
    }
  } else if (const auto* option = std::get_if<SetOverlayOption>(&request)) {
    const auto mask = option->option == OverlayOption::Passthrough  ? flags::passthrough
                      : option->option == OverlayOption::Persistent ? flags::persistent
                                                                    : flags::readOnlyMedia;
    QCOMPARE(after.next.overlayFlags, option->value ? before.next.overlayFlags | mask : before.next.overlayFlags & ~mask);
    QCOMPARE(after.current.overlayFlags, before.current.overlayFlags);
  } else if (const auto* volume = std::get_if<SetVolume>(&request)) {
    const auto& configured = after.next.volumes.at(volumeC).volume;
    QCOMPARE(volume->property == VolumeProperty::Protected ? configured.protectedVolume : configured.bindByDriveLetter, volume->value);
    QCOMPARE(after.current, before.current);
  } else if (const auto* file = std::get_if<EditFileExclusion>(&request)) {
    const auto& entries = after.next.volumes.at(volumeC).fileExclusions;
    if (file->action == ExclusionAction::Clear)
      QVERIFY(entries.empty());
    else
      QCOMPARE(std::ranges::find(entries, file->path) != entries.end(), file->action == ExclusionAction::Add);
    QCOMPARE(after.current, before.current);
  } else if (const auto* registry = std::get_if<EditRegistryExclusion>(&request)) {
    const auto& entries = after.next.registryExclusions;
    if (registry->action == ExclusionAction::Clear)
      QVERIFY(entries.empty());
    else
      QCOMPARE(std::ranges::find(entries, registry->key) != entries.end(), registry->action == ExclusionAction::Add);
    QCOMPARE(after.current, before.current);
  } else if (std::holds_alternative<CreateSwapfile>(request)) {
    QCOMPARE(after.next.overlayType, static_cast<std::uint32_t>(OverlayType::Disk));
    QCOMPARE(after.next.volumes.at(volumeC).swapfileSize, 1u);
    QCOMPARE(after.next.volumes.at(volumeD).swapfileSize, 0u);
    QVERIFY((after.next.overlayFlags & flags::passthrough) != 0);
    QCOMPARE(after.next.overlayFlags & flags::persistent,
             before.next.overlayType == static_cast<std::uint32_t>(OverlayType::Disk) ? 0u : before.next.overlayFlags & flags::persistent);
  } else if (std::holds_alternative<CommitFile>(request)) {
    QCOMPARE(fixture.native->readFile(volumeC, filePath), std::optional<std::string>{"after"});
    QCOMPARE(fixture.native->visibleFile(volumeC, filePath), std::optional<std::string>{"after"});
  } else if (std::holds_alternative<CommitFileDeletion>(request)) {
    QVERIFY(!fixture.native->readFile(volumeC, filePath));
    QVERIFY(!fixture.native->visibleFile(volumeC, filePath));
    const auto& stages = after.fileDeletions.at(Fake::FileAddress{volumeC, filePath});
    QVERIFY(stages.visibleDeleted && stages.persistentDeleted);
  } else if (std::holds_alternative<CommitRegistry>(request)) {
    QCOMPARE(fixture.native->readRegistryValue(registryKey, registryValue), std::optional<std::string>{"after"});
    QCOMPARE(fixture.native->visibleRegistryValue(registryKey, registryValue), std::optional<std::string>{"after"});
  } else if (std::holds_alternative<CommitRegistryDeletion>(request)) {
    QVERIFY(!fixture.native->readRegistryValue(registryKey, registryValue));
    QVERIFY(!fixture.native->visibleRegistryValue(registryKey, registryValue));
  } else if (std::holds_alternative<UpdateWindows>(request)) {
    if (response.succeeded()) QCOMPARE(std::get<WindowsUpdateOutcome>(response.value), before.windowsUpdateOutcome);
    QCOMPARE(after.windowsUpdates, before.windowsUpdates + 1);
  } else if (const auto* power = std::get_if<Power>(&request)) {
    QCOMPARE(after.powerRequests.size(), before.powerRequests.size() + 1);
    QCOMPARE(after.powerRequests.back(), power->action);
    QVERIFY(after.runtime.shutdownPending);
    QCOMPARE(after.current, before.current);
    QCOMPARE(after.next, before.next);
  } else {
    QFAIL("A write entry has no native effect assertion");
  }
}

class UwfStateMutationMatrixTests final : public QObject {
  Q_OBJECT
 private slots:
  void unobservedRelatedValuesAreRefreshedAfterSuccessfulWrites_data() {
    QTest::addColumn<int>("index");
    QTest::newRow("saved-reset-current-mode") << 0;
    QTest::newRow("type-previous-next-type") << 1;
    QTest::newRow("horm-previous-value") << 2;
    QTest::newRow("filter-current-servicing") << 3;
    QTest::newRow("file-exclusion-complete-list") << 4;
  }
  void unobservedRelatedValuesAreRefreshedAfterSuccessfulWrites() {
    QFETCH(int, index);
    auto initial = initialState();
    Request request = SetNumber{NumberProperty::ResetPersistentState, static_cast<std::uint32_t>(ResetMode::Saved)};
    std::function<void(UwfStateFacts&)> forget;
    switch (index) {
      case 0:
        initial.current.filterEnabled = true;
        initial.current.resetPersistentStateSavedMode = 1;
        initial.next = initial.current;
        forget = [](UwfStateFacts& facts) { facts.current.savedMode = Fact<std::uint32_t>::unknown(); };
        break;
      case 1:
        initial.next.overlayFlags = 0x80000003u;
        request = SetNumber{NumberProperty::OverlayType, static_cast<std::uint32_t>(OverlayType::Ram)};
        forget = [](UwfStateFacts& facts) { facts.next.overlayType = Fact<std::uint32_t>::unknown(); };
        break;
      case 2:
        initial.current.filterEnabled = true;
        initial.current.overlayType = static_cast<std::uint32_t>(OverlayType::Ram);
        initial.current.overlayFlags = flags::readOnlyMedia;
        for (auto& [id, volume] : initial.current.volumes) {
          (void)id;
          volume.volume.protectedVolume = true;
        }
        initial.next = initial.current;
        request = SetBoolean{BooleanProperty::HormEnabled, true};
        forget = [](UwfStateFacts& facts) { facts.runtime.horm = Fact<bool>::unknown(); };
        break;
      case 3:
        initial.current.filterEnabled = true;
        initial.current.servicingEnabled = true;
        initial.next.filterEnabled = false;
        initial.next.servicingEnabled = false;
        request = SetBoolean{BooleanProperty::FilterEnabled, true};
        forget = [](UwfStateFacts& facts) { facts.current.servicingEnabled = Fact<bool>::unknown(); };
        break;
      case 4:
        initial.next.volumes.at(volumeC).fileExclusions = {"\\Zulu", "\\Existing", "\\Existing", "\\alpha"};
        request = EditFileExclusion{volumeC, "\\New", ExclusionAction::Add};
        forget = [](UwfStateFacts& facts) { facts.next.volumes.at(volumeC).fileExclusions = Fact<std::vector<std::string>>::unknown(); };
        break;
      default:
        QFAIL("Unexpected unobserved input row");
    }
    Fixture fixture(std::move(initial));
    auto facts = fixture.system.state();
    forget(facts);
    fixture.system.updateState(std::move(facts));
    const auto response = fixture.system.execute(request);
    QVERIFY2(response.succeeded(), response.failure ? response.failure->message.c_str() : "");
    QCOMPARE(response.effect, Effect::Confirmed);
    QCOMPARE(fixture.refreshes->value, 1);
    QCOMPARE(fixture.writes(), std::vector<Request>{request});
    const auto calls = fixture.native->trace();
    QVERIFY(calls.size() > 1);
    // Missing related observations must not turn admission into a preflight Get.
    // The accepted write is followed by one configuration refresh.
    QCOMPARE(calls.front().request, request);
    for (std::size_t i = 1; i < calls.size(); ++i) QVERIFY(!isWrite(calls[i].request));
    const auto difference = fixture.difference();
    QVERIFY2(difference.empty(), difference.c_str());
    QCOMPARE(std::ranges::count_if(calls, [](const auto& call) { return std::holds_alternative<GetOverlayUsage>(call.request); }), 0);
  }
  void publicWriteVariantsMatchNativeStateAndReceipts_data() {
    QTest::addColumn<int>("index");
    QTest::addColumn<int>("faultMode");
    const auto cases = mutations();
    for (std::size_t index = 0; index < cases.size(); ++index) {
      QTest::newRow((cases[index].name + "-success").c_str()) << static_cast<int>(index) << 0;
      if (cases[index].emptyClear) continue;
      QTest::newRow((cases[index].name + "-before").c_str()) << static_cast<int>(index) << 1;
      QTest::newRow((cases[index].name + "-after-effect").c_str()) << static_cast<int>(index) << 2;
    }
  }
  void publicWriteVariantsMatchNativeStateAndReceipts() {
    QFETCH(int, index);
    QFETCH(int, faultMode);
    auto row = mutations().at(static_cast<std::size_t>(index));
    Fixture fixture(std::move(row.initial));
    prepareVisibleTarget(row.request, *fixture.native);
    const auto initialDifference = fixture.difference();
    QVERIFY2(initialDifference.empty(), initialDifference.c_str());
    const auto before = fixture.native->snapshot();
    const auto primitive = rawTarget(row.request, before);
    const auto faultMessage = "Native " + row.name + " receipt was lost";
    constexpr auto faultCode = std::int64_t{0x80070005};
    if (faultMode != 0)
      fixture.native->inject({.request = primitive,
                              .operation = {},
                              .timing = faultMode == 1 ? Fake::FaultTiming::Before : Fake::FaultTiming::AfterEffect,
                              .kind = ErrorKind::AccessDenied,
                              .message = faultMessage,
                              .code = faultCode});
    const auto response = fixture.system.execute(row.request);
    if (faultMode == 0) {
      QVERIFY2(response.succeeded(), response.failure ? response.failure->message.c_str() : "");
      QCOMPARE(response.effect, row.emptyClear ? Effect::None : Effect::Confirmed);
    } else {
      QVERIFY(response.failure);
      QCOMPARE(response.failure->kind, ErrorKind::AccessDenied);
      QCOMPARE(response.failure->message, faultMessage);
      QCOMPARE(response.effect, faultMode == 1 ? Effect::None : Effect::Uncertain);
      QCOMPARE(response.failure->effect, response.effect);
      QCOMPARE(response.failure->diagnostics.size(), std::size_t{1});
      const auto& diagnostic = response.failure->diagnostics.front();
      QCOMPARE(diagnostic.backend, fixture.native->name());
      QCOMPARE(diagnostic.operation, operationName(primitive));
      QCOMPARE(diagnostic.code, faultCode);
      QCOMPARE(diagnostic.detail, faultMessage);
    }
    const int expectedRefreshes = faultMode == 1 ? 0 : faultMode == 2 ? !dataCommit(row.request) : row.refreshes;
    QCOMPARE(fixture.refreshes->value, expectedRefreshes);
    const std::vector<Request> expectedWrites = row.emptyClear ? std::vector<Request>{} : std::vector<Request>{primitive};
    // A failed completion receipt must never replay the primitive. Before faults
    // are attempted once, but never enter the captured native implementation.
    QCOMPARE(fixture.writes(), expectedWrites);
    const auto primitives = fixture.native->primitiveRequests();
    QCOMPARE(std::ranges::count(primitives, primitive), faultMode == 1 || row.emptyClear ? 0 : 1);
    const auto difference = fixture.difference();
    QVERIFY2(difference.empty(), difference.c_str());
    if (faultMode == 1)
      QCOMPARE(fixture.native->snapshot(), before);
    else
      verifyNativeMutation(row.request, before, fixture, response);
    const auto calls = fixture.native->trace();
    if (expectedRefreshes == 0)
      for (const auto& call : calls) QVERIFY(isWrite(call.request));
    // Usage is an independent explicit sample, never a side effect of writes
    // or their configuration refresh. This covers all four data entry points.
    QCOMPARE(std::ranges::count_if(calls, [](const auto& call) { return std::holds_alternative<GetOverlayUsage>(call.request); }), 0);
  }
};
}  // namespace

QTEST_APPLESS_MAIN(UwfStateMutationMatrixTests)
#include "UwfStateMutationMatrixTests.moc"
