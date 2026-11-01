// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QTest>
#include <algorithm>
#include <array>
#include <cstdint>
#include <limits>
#include <optional>
#include <set>
#include <string>
#include <type_traits>
#include <vector>

#include "domain/uwf/solvers/UwfFailureSolver.h"
#include "support/uwf/CapturedSolverOracle.h"
#include "support/uwf/UwfOperationCatalog.h"
namespace {
using namespace uwf::compat;
using namespace uwf::test::solver_catalog;
namespace oracle = uwf::test::solver_oracle;
constexpr std::array<std::optional<bool>, 3> truth{std::nullopt, false, true};
Fact<bool> boolean(std::size_t state) { return truth[state] ? Fact<bool>::known(*truth[state]) : Fact<bool>::unknown(); }
bool rejects(const Request& request, const UwfStateFacts& facts) { return std::holds_alternative<Reject>(solveUwfRequest(request, facts).decision); }
UwfStateFacts hormFacts() {
  UwfStateFacts state;
  state.administrator = Fact<bool>::known(true);
  state.runtime.horm = Fact<bool>::known(false);
  state.current.filterEnabled = Fact<bool>::known(true);
  state.current.overlayType = Fact<std::uint32_t>::known(0);
  state.current.overlayFlags = BitFacts::known(4);
  state.runtime.romDisposition = BitFacts::known(0);
  state.runtime.controlDevicePresent = Fact<bool>::known(true);
  state.runtime.hiberFile = Fact<bool>::known(true);
  state.runtime.hormVolumesProtected = Fact<bool>::known(true);
  return state;
}
struct Projection {
  bool witnessed = false, accepted = false;
};
template <std::size_t N>
Projection project(const BitFacts& partial, const std::array<std::uint32_t, N>& words, const std::array<char, N>& table) {
  Projection result;
  for (std::size_t i = 0; i < N; ++i)
    if (oracle::compatible(partial, words[i])) {
      result.witnessed = true;
      result.accepted = result.accepted || table[i] == 'A';
    }
  return result;
}
Projection combined(const BitFacts& flags, const BitFacts& disposition, const std::array<char, 16>& romTable) {
  Projection result;
  for (std::size_t f = 0; f < oracle::flags.size(); ++f)
    if (oracle::compatible(flags, oracle::flags[f]))
      for (std::size_t d = 0; d < oracle::dispositions.size(); ++d)
        if (oracle::compatible(disposition, oracle::dispositions[d])) {
          result.witnessed = true;
          result.accepted = result.accepted || (oracle::loadedRom[f] == 'N' || romTable[d] == 'A');
        }
  return result;
}
bool hasAccepted(const Fact<bool>& observed, const std::array<char, 2>& table) {
  for (const auto value : oracle::completions(observed))
    if (table[value ? 1 : 0] == 'A') return true;
  return false;
}
// Literal complete-path classification from CLI authorization and HORM's
// same-state branches. E=actual enable; D=actual disable; A=legal no-op.
constexpr std::array<std::array<char, 4>, 2> hormEntry{{{{'R', 'R', 'A', 'D'}}, {{'R', 'R', 'E', 'A'}}}};
bool hormHasSuccessfulCompletion(bool requested, const UwfStateFacts& state) {
  constexpr std::array<char, 2> necessaryTrue{'R', 'A'};
  constexpr std::array<char, 3> types{'A', 'R', 'R'};
  bool typeCanSucceed = !state.current.overlayType.value;
  if (state.current.overlayType.value) typeCanSucceed = *state.current.overlayType.value < types.size() && types[*state.current.overlayType.value] == 'A';
  for (const auto admin : oracle::completions(state.administrator))
    for (const auto before : oracle::completions(state.runtime.horm)) {
      const auto path = hormEntry[requested ? 1 : 0][(admin ? 2 : 0) + (before ? 1 : 0)];
      if (path == 'A') return true;
      if (path == 'D' && hasAccepted(state.runtime.controlDevicePresent, necessaryTrue)) return true;
      if (path == 'E' && typeCanSucceed && hasAccepted(state.current.filterEnabled, necessaryTrue) &&
          hasAccepted(state.runtime.controlDevicePresent, necessaryTrue) && hasAccepted(state.runtime.hiberFile, necessaryTrue) &&
          hasAccepted(state.runtime.hormVolumesProtected, necessaryTrue))
        return true;
    }
  return false;
}
}  // namespace
class UwfFailureSolverTests final : public QObject {
  Q_OBJECT
 private slots:
  void romConfigurationFailuresRequireCurrentProtectedSystemFacts() {
    auto facts = hormFacts();
    facts.systemVolumeId = Fact<std::string>::known(oracle::volume);
    auto& current = facts.current.volumes[oracle::volume];
    current.present = Fact<bool>::known(true);
    current.protectedVolume = Fact<bool>::known(true);
    const Request ordinary = SetBoolean{BooleanProperty::PersistDomainSecretKey, true};
    const Request threshold = SetNumber{NumberProperty::WarningThreshold, 128};
    const auto onlyModeRejects = [&](const Request& request) {
      const auto evaluation = solveUwfRequest(request, facts);
      const auto* failure = std::get_if<Reject>(&evaluation.decision);
      return failure && failure->rule == RuleId::RomConfigurationNotPersistent;
    };
    QVERIFY(onlyModeRejects(ordinary));
    QVERIFY(onlyModeRejects(threshold));
    facts.next.filterEnabled = Fact<bool>::known(false);
    facts.next.overlayFlags = BitFacts::known(0);
    facts.next.overlayType = Fact<std::uint32_t>::known(1);
    QVERIFY(onlyModeRejects(ordinary));
    current.protectedVolume = Fact<bool>::known(false);
    QVERIFY(!rejects(ordinary, facts));
    QVERIFY(!rejects(threshold, facts));
    current.protectedVolume = Fact<bool>::known(true);
    facts.current.filterEnabled = Fact<bool>::known(false);
    QVERIFY(!rejects(ordinary, facts));
    facts.current.filterEnabled = Fact<bool>::known(true);
    facts.runtime.romDisposition = BitFacts::known(3);
    QVERIFY(!rejects(ordinary, facts));
    QVERIFY(!rejects(threshold, facts));
    facts.runtime.romDisposition = BitFacts::known(4);
    facts.runtime.fullCommit = Fact<bool>::known(true);
    QVERIFY(!rejects(ordinary, facts));
    QVERIFY(onlyModeRejects(threshold));
    facts.runtime.romDisposition = BitFacts::known(0);
    facts.runtime.fullCommit = Fact<bool>::known(false);
    current.present = Fact<bool>::unknown();
    QVERIFY(!rejects(ordinary, facts));
    current.present = Fact<bool>::known(true);
    current.protectedVolume = Fact<bool>::unknown();
    QVERIFY(!rejects(ordinary, facts));
    current.protectedVolume = Fact<bool>::known(true);
    facts.systemVolumeId = Fact<std::string>::unknown();
    QVERIFY(!rejects(ordinary, facts));
    facts.systemVolumeId = Fact<std::string>::known(oracle::volume);
    facts.runtime.romDisposition = {};
    QVERIFY(!rejects(ordinary, facts));
    facts.runtime.romDisposition = BitFacts::known(0);
    facts.current.overlayFlags = {};
    QVERIFY(!rejects(ordinary, facts));
  }
  void filterAndHormTransitionsRetainTheirDedicatedRomPersistencePaths() {
    auto facts = hormFacts();
    facts.systemVolumeId = Fact<std::string>::known(oracle::volume);
    facts.current.volumes[oracle::volume].present = Fact<bool>::known(true);
    facts.current.volumes[oracle::volume].protectedVolume = Fact<bool>::known(true);
    facts.next.filterEnabled = Fact<bool>::known(true);
    QVERIFY(!rejects(SetBoolean{BooleanProperty::FilterEnabled, false}, facts));
    QVERIFY(!rejects(SetBoolean{BooleanProperty::HormEnabled, true}, facts));
    facts.runtime.horm = Fact<bool>::known(true);
    facts.runtime.romDisposition = BitFacts::known(2);
    QVERIFY(!rejects(SetBoolean{BooleanProperty::HormEnabled, false}, facts));
  }
  void allCapabilitySlotsWithNoFacts_data() {
    QTest::addColumn<int>("index");
    const auto entries = oracle::capabilitySlots();
    QCOMPARE(entries.size(), std::size_t{62});
    for (std::size_t i = 0; i < entries.size(); ++i) QTest::newRow(entries[i].id) << static_cast<int>(i);
  }
  void allCapabilitySlotsWithNoFacts() {
    QFETCH(int, index);
    const auto slot = oracle::capabilitySlots().at(static_cast<std::size_t>(index));
    const auto evaluated = solveUwfRequest(slot.request, {});
    QCOMPARE(std::holds_alternative<Reject>(evaluated.decision), slot.rejectWithNoFacts);
    QVERIFY(evaluated.arguments == slot.request);
    const auto identity = operationSlot(slot.request);
    QVERIFY(identity);
    const auto catalog = operationCatalog();
    const auto found = std::ranges::find_if(catalog, [&](const auto& entry) { return entry.slot == *identity; });
    QVERIFY(found != catalog.end());
    QCOMPARE(found->name, std::string_view(slot.id));
  }
  void exactSessionInventoryAndStaticInvalidity() {
    std::size_t signatures = 0, invalid = 0;
    std::set<OperationSlot> entries;
    for (const auto& slot : oracle::capabilitySlots()) {
      auto request = slot.request;
      const auto identity = operationSlot(request);
      QVERIFY(identity);
      entries.insert(*identity);
      std::visit(
          [&](auto& operation) {
            if constexpr (requires { operation.session; }) {
              for (const auto session : {Session::Current, Session::Next}) {
                operation.session = session;
                QCOMPARE(rejects(request, {}), slot.rejectWithNoFacts);
                ++signatures;
              }
              operation.session = static_cast<Session>(99);
              QVERIFY(rejects(request, {}));
              ++invalid;
            } else
              ++signatures;
          },
          request);
    }
    QCOMPARE(entries.size(), std::size_t{62});
    QCOMPARE(signatures, std::size_t{85});
    QCOMPARE(invalid, std::size_t{23});
  }
  void stateInventorySeparatesFactsAndFutureResults() {
    const auto axes = stateAxisCatalog();
    QCOMPARE(axes.size(), std::size_t{89});
    std::set<StateAxis> names;
    for (const auto& axis : axes) {
      QVERIFY(names.insert(axis.axis).second);
      QVERIFY(!axis.name.empty());
      QVERIFY(!axis.boundary.empty());
    }
    const std::array<StateAxis, 11> notFacts{StateAxis::ProviderTransport,       StateAxis::ResponseShape,        StateAxis::MutationStage,
                                             StateAxis::ProviderWriteEffect,     StateAxis::UpdateOutcome,        StateAxis::Concurrency,
                                             StateAxis::FullCommitFilterOutcome, StateAxis::DiagnosticCandidates, StateAxis::DiagnosticClusterSize,
                                             StateAxis::DiagnosticReportProfile, StateAxis::ProviderPrerequisites};
    for (const auto name : notFacts) {
      const auto found = std::ranges::find_if(axes, [name](const auto& axis) { return axis.axis == name; });
      QVERIFY(found != axes.end());
      QVERIFY(found->nature != AxisNature::ObservedFact);
      QVERIFY(found->nature != AxisNature::DerivedView);
    }
    for (const auto name : {StateAxis::ShutdownPending, StateAxis::FileSystem, StateAxis::VolumeBytes, StateAxis::AvailableBytes}) {
      const auto found = std::ranges::find_if(axes, [name](const auto& axis) { return axis.axis == name; });
      QVERIFY(found != axes.end());
      QCOMPARE(found->nature, AxisNature::ObservedFact);
    }
  }
  void flagsPartialMasksRespectExactHormRequirement() {
    const Request request = SetBoolean{BooleanProperty::HormEnabled, true};
    const auto masks = oracle::partialFlags();
    QCOMPARE(masks.size(), std::size_t{486});
    std::size_t witnesses = 0;
    for (std::size_t index = 0; index < masks.size(); ++index) {
      const auto expected = project(masks[index], oracle::flags, oracle::hormFlags);
      QVERIFY(expected.witnessed);
      auto state = hormFacts();
      state.current.overlayFlags = masks[index];
      if (rejects(request, state) == expected.accepted)
        QFAIL(qPrintable(
            QStringLiteral("HORM flags partial oracle mismatch at %1 mask=%2 value=%3").arg(index).arg(masks[index].knownMask).arg(masks[index].valueBits)));
      ++witnesses;
      if (!expected.accepted)
        for (const auto word : oracle::flags)
          if (oracle::compatible(masks[index], word)) {
            state.current.overlayFlags = BitFacts::known(word);
            QVERIFY(rejects(request, state));
          }
    }
    QCOMPARE(witnesses, std::size_t{486});
  }
  void dispositionPartialMasksRespectHormIdleRequirement() {
    constexpr std::array<char, 16> idle{'A', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R'};
    const Request request = SetBoolean{BooleanProperty::HormEnabled, true};
    const auto masks = oracle::partialDispositions();
    QCOMPARE(masks.size(), std::size_t{162});
    for (std::size_t index = 0; index < masks.size(); ++index) {
      const auto expected = project(masks[index], oracle::dispositions, idle);
      QVERIFY(expected.witnessed);
      auto state = hormFacts();
      state.runtime.romDisposition = masks[index];
      if (rejects(request, state) == expected.accepted) QFAIL(qPrintable(QStringLiteral("HORM disposition partial oracle mismatch at %1").arg(index)));
    }
  }
  void fullCommitPartialProductHasIndependentUniversalOracle() {
    const auto flags = oracle::bitInformationClasses(256);
    const auto dispositions = oracle::bitInformationClasses(8);
    const Request request = SetBoolean{BooleanProperty::OverlayCommit, true};
    constexpr std::array<bool, 3> canBeTrue{true, false, true};
    std::size_t checked = 0;
    for (std::size_t f = 0; f < flags.size(); ++f)
      for (std::size_t d = 0; d < dispositions.size(); ++d) {
        const auto branch = combined(flags[f].representative, dispositions[d].representative, oracle::fullCommitRom);
        QVERIFY(branch.witnessed);
        for (std::size_t filter = 0; filter < 3; ++filter)
          for (std::size_t control = 0; control < 3; ++control) {
            auto state = hormFacts();
            state.current.overlayFlags = flags[f].representative;
            state.runtime.romDisposition = dispositions[d].representative;
            state.current.filterEnabled = boolean(filter);
            state.runtime.controlDevicePresent = boolean(control);
            const bool expectedReject = !(branch.accepted && canBeTrue[filter] && canBeTrue[control]);
            if (rejects(request, state) != expectedReject)
              QFAIL(qPrintable(
                  QStringLiteral("FullCommit partial oracle mismatch flags=%1 disposition=%2 filter=%3 control=%4").arg(f).arg(d).arg(filter).arg(control)));
            ++checked;
          }
      }
    QCOMPARE(checked, std::size_t{60516});
  }
  void filterPartialProductPreservesPossibleNoOp() {
    const auto flags = oracle::bitInformationClasses(256);
    const auto dispositions = oracle::bitInformationClasses(8);
    const Request request = SetBoolean{BooleanProperty::FilterEnabled, true};
    constexpr std::array<bool, 3> canBeOffline{true, true, false};
    constexpr std::array<bool, 3> canMatchNext{true, false, true};
    std::size_t checked = 0;
    for (std::size_t f = 0; f < flags.size(); ++f)
      for (std::size_t d = 0; d < dispositions.size(); ++d) {
        const auto branch = combined(flags[f].representative, dispositions[d].representative, oracle::filterRom);
        QVERIFY(branch.witnessed);
        for (std::size_t current = 0; current < 3; ++current)
          for (std::size_t next = 0; next < 3; ++next) {
            auto state = hormFacts();
            state.runtime.horm = Fact<bool>::known(false);
            state.current.overlayFlags = flags[f].representative;
            state.runtime.romDisposition = dispositions[d].representative;
            state.current.filterEnabled = boolean(current);
            state.next.filterEnabled = boolean(next);
            state.next.overlayType = Fact<std::uint32_t>::known(0);
            const bool expectedReject = !(canMatchNext[next] || canBeOffline[current] || branch.accepted);
            if (rejects(request, state) != expectedReject)
              QFAIL(
                  qPrintable(QStringLiteral("Filter partial oracle mismatch flags=%1 disposition=%2 current=%3 next=%4").arg(f).arg(d).arg(current).arg(next)));
            ++checked;
          }
      }
    QCOMPARE(checked, std::size_t{60516});
  }
  void hormEntryBranchesAndUnknownsHaveIndependentOracle() {
    constexpr std::array<std::optional<std::uint32_t>, 4> types{std::nullopt, 0, 1, 2};
    std::size_t checked = 0;
    for (const bool requested : {false, true})
      for (std::size_t before = 0; before < 3; ++before)
        for (std::size_t admin = 0; admin < 3; ++admin)
          for (std::size_t filter = 0; filter < 3; ++filter)
            for (std::size_t control = 0; control < 3; ++control)
              for (std::size_t hiber = 0; hiber < 3; ++hiber)
                for (std::size_t volumes = 0; volumes < 3; ++volumes)
                  for (std::size_t type = 0; type < 4; ++type) {
                    auto state = hormFacts();
                    state.runtime.horm = boolean(before);
                    state.administrator = boolean(admin);
                    state.current.filterEnabled = boolean(filter);
                    state.runtime.controlDevicePresent = boolean(control);
                    state.runtime.hiberFile = boolean(hiber);
                    state.runtime.hormVolumesProtected = boolean(volumes);
                    state.current.overlayType = types[type] ? Fact<std::uint32_t>::known(*types[type]) : Fact<std::uint32_t>::unknown();
                    const bool expectedReject = !hormHasSuccessfulCompletion(requested, state);
                    if (rejects(SetBoolean{BooleanProperty::HormEnabled, requested}, state) != expectedReject)
                      QFAIL(qPrintable(
                          QStringLiteral("HORM branch oracle mismatch requested=%1 before=%2 admin=%3 filter=%4 control=%5 hiber=%6 volumes=%7 type=%8")
                              .arg(requested)
                              .arg(before)
                              .arg(admin)
                              .arg(filter)
                              .arg(control)
                              .arg(hiber)
                              .arg(volumes)
                              .arg(type)));
                    ++checked;
                  }
    QCOMPARE(checked, std::size_t{5832});
  }
  void independentRejectionSurvivesUnknownAndInvalidMask() {
    const Request request = SetNumber{NumberProperty::OverlayType, 0};
    UwfStateFacts state;
    state.current.filterEnabled = Fact<bool>::known(true);
    state.administrator = Fact<bool>::unknown();
    state.runtime.horm = Fact<bool>::unknown();
    state.next.maximumOverlaySize = Fact<std::uint32_t>::unknown();
    state.current.overlayFlags = {0, 4};
    QVERIFY(rejects(request, state));
    state.observation.applicable = false;
    QVERIFY(!rejects(request, state));
    state = hormFacts();
    state.current.overlayFlags = {0, 256};
    QVERIFY(!rejects(SetBoolean{BooleanProperty::HormEnabled, true}, state));
    state.current.filterEnabled = Fact<bool>::known(false);
    QVERIFY(rejects(SetBoolean{BooleanProperty::HormEnabled, true}, state));
  }
  void opaqueBitRenamingCannotHideRequiredFacts() {
    for (unsigned bit = 3; bit < 32; ++bit) {
      auto state = hormFacts();
      state.current.overlayFlags = BitFacts::known(4 | (std::uint32_t{1} << bit));
      QVERIFY(rejects(SetBoolean{BooleanProperty::HormEnabled, true}, state));
      state.current.overlayFlags = BitFacts::known(4);
      state.runtime.romDisposition = {4 | (std::uint32_t{1} << bit), std::uint32_t{1} << bit};
      QVERIFY(rejects(SetBoolean{BooleanProperty::OverlayCommit, true}, state));
      state.runtime.romDisposition = {std::uint32_t{1} << bit, std::uint32_t{1} << bit};
      QVERIFY(!rejects(SetBoolean{BooleanProperty::OverlayCommit, true}, state));
    }
  }
  void exactTargetFactsNeverLeakAcrossRequests() {
    const Request file = CommitFile{oracle::volume, "\\selected"};
    UwfStateFacts state;
    state.targets[oracle::volume].fileTarget = Fact<FileMaintenanceFacts>::known({10, FileMaintenanceKind::Missing, false});
    state.requestTarget = CommitFile{oracle::volume, "\\other"};
    QVERIFY(!rejects(file, state));
    state.requestTarget = file;
    QVERIFY(rejects(file, state));
    state = {};
    state.current.volumes[oracle::otherVolume].present = Fact<bool>::known(false);
    QVERIFY(!rejects(GetVolumeProperty{oracle::volume, VolumeProperty::Protected}, state));
    state.current.volumes[oracle::volume].present = Fact<bool>::known(false);
    QVERIFY(rejects(GetVolumeProperty{oracle::volume, VolumeProperty::Protected}, state));
  }

  void weightedBitClassesCoverTheCompleteRepresentationDomain() {
    const auto classes = oracle::bitInformationClasses(256);
    QCOMPARE(classes.size(), std::size_t{82});
    std::uint64_t valid = 0;
    for (std::size_t i = 0; i < 81; ++i) valid += classes[i].weight;
    QCOMPARE(valid, std::uint64_t{1853020188851841ULL});
    QCOMPARE(classes.back().weight, std::numeric_limits<std::uint64_t>::max() - valid + 1);
    for (unsigned bit = 0; bit < 32; ++bit) {
      auto state = hormFacts();
      state.current.overlayFlags = {~(std::uint32_t{1} << bit), 4 | (std::uint32_t{1} << bit)};
      state.runtime.romDisposition = BitFacts::known(3);
      // A malformed field is discarded, even if it also contains a known ROM
      // bit. It cannot turn an empty set of completions into arbitrary Reject.
      QVERIFY(!rejects(SetBoolean{BooleanProperty::OverlayCommit, true}, state));
    }
  }
  void capturedAuthorizationAppliesBeforeMatchingNoOps() {
    constexpr std::array<std::string_view, 18> governed{"SetBoolean.FilterEnabled",
                                                        "SetBoolean.ServicingEnabled",
                                                        "SetNumber.OverlayType",
                                                        "SetNumber.MaximumOverlaySize",
                                                        "SetNumber.WarningThreshold",
                                                        "SetNumber.CriticalThreshold",
                                                        "SetNumber.ResetPersistentState",
                                                        "SetNumber.ResetPersistentStateSavedMode",
                                                        "SetOverlayOption.Passthrough",
                                                        "SetOverlayOption.Persistent",
                                                        "SetVolume.Protected",
                                                        "EditFileExclusion.Add",
                                                        "EditFileExclusion.Remove",
                                                        "EditFileExclusion.Clear",
                                                        "EditRegistryExclusion.Add",
                                                        "EditRegistryExclusion.Remove",
                                                        "EditRegistryExclusion.Clear",
                                                        "ResetSettings"};
    constexpr std::array<std::string_view, 3> expected{"CCR", "RRR", "CCR"};
    std::size_t checked = 0;
    const auto entries = oracle::capabilitySlots();
    for (const auto name : governed) {
      const auto found = std::ranges::find_if(entries, [name](const auto& entry) { return name == entry.id; });
      QVERIFY(found != entries.end());
      for (std::size_t admin = 0; admin < 3; ++admin)
        for (std::size_t horm = 0; horm < 3; ++horm) {
          UwfStateFacts state;
          state.administrator = boolean(admin);
          state.runtime.horm = boolean(horm);
          QCOMPARE(rejects(found->request, state), expected[admin][horm] == 'R');
          ++checked;
        }
    }
    QCOMPARE(checked, std::size_t{162});
  }
  void offlineAndCapturedDefaultRulesDoNotWaitForOtherFacts() {
    const std::array<Request, 7> offline{SetNumber{NumberProperty::OverlayType, 0},
                                         SetNumber{NumberProperty::MaximumOverlaySize, 1024},
                                         SetOverlayOption{OverlayOption::Passthrough, true},
                                         SetOverlayOption{OverlayOption::Persistent, true},
                                         SetOverlayOption{OverlayOption::ReadOnlyMedia, true},
                                         CreateSwapfile{oracle::volume},
                                         ResetSettings{}};
    constexpr std::string_view expected = "CCR";
    for (const auto& request : offline)
      for (std::size_t current = 0; current < 3; ++current) {
        UwfStateFacts state;
        state.current.filterEnabled = boolean(current);
        QCOMPARE(rejects(request, state), expected[current] == 'R');
      }
    UwfStateFacts missing;
    missing.capturedSettingsAvailable = Fact<bool>::known(false);
    QVERIFY(rejects(ResetSettings{}, missing));
  }
  void numericConfigurationParametersUseIndependentBoundaryTables() {
    constexpr std::array<std::optional<std::uint32_t>, 8> sizes{std::nullopt, 0, 1, 1023, 1024, 2147483647u, 2147483648u, 4294967295u};
    constexpr std::array<std::string_view, 2> typeExpected{"CRCCCCCC", "CRRRCCRR"};
    for (std::uint32_t type = 0; type < 2; ++type)
      for (std::size_t size = 0; size < sizes.size(); ++size) {
        UwfStateFacts state;
        if (sizes[size]) state.next.maximumOverlaySize = Fact<std::uint32_t>::known(*sizes[size]);
        QCOMPARE(rejects(SetNumber{NumberProperty::OverlayType, type}, state), typeExpected[type][size] == 'R');
      }
    constexpr std::array<std::uint32_t, 6> requested{1, 1023, 1024, 2147483647u, 2147483648u, 4294967295u};
    constexpr std::array<std::optional<std::uint32_t>, 5> types{std::nullopt, 0, 1, 2, 4294967295u};
    constexpr std::array<std::string_view, 6> sizeExpected{"CCRRR", "CCRRR", "CCCRR", "CCCRR", "CCRRR", "CCRRR"};
    for (std::size_t size = 0; size < requested.size(); ++size)
      for (std::size_t type = 0; type < types.size(); ++type) {
        UwfStateFacts state;
        if (types[type]) state.next.overlayType = Fact<std::uint32_t>::known(*types[type]);
        QCOMPARE(rejects(SetNumber{NumberProperty::MaximumOverlaySize, requested[size]}, state), sizeExpected[size][type] == 'R');
      }
    // Create-swapfile has no capacity argument; its primitive uses the existing configuration.
    constexpr std::string_view swapExpected = "CCCCCCCC";
    for (std::size_t size = 0; size < sizes.size(); ++size) {
      UwfStateFacts state;
      if (sizes[size]) state.next.maximumOverlaySize = Fact<std::uint32_t>::known(*sizes[size]);
      QCOMPARE(rejects(CreateSwapfile{oracle::volume}, state), swapExpected[size] == 'R');
    }
  }
  void runtimeResetPartialFactsProjectCompleteNecessaryConditions() {
    constexpr std::array<std::optional<std::uint32_t>, 4> types{std::nullopt, 0, 1, 2};
    constexpr std::array<std::optional<std::uint32_t>, 6> saved{std::nullopt, 0, 1, 2, 255, 4294967295u};
    constexpr std::array<bool, 3> canBeTrue{true, false, true};
    constexpr std::array<bool, 4> canBeDisk{true, false, true, false};
    constexpr std::array<bool, 6> canResolve{true, true, true, false, false, false};
    std::size_t checked = 0;
    for (const auto mode : {0u, 1u, 255u})
      for (std::size_t filter = 0; filter < 3; ++filter)
        for (std::size_t type = 0; type < 4; ++type)
          for (std::size_t persistent = 0; persistent < 3; ++persistent)
            for (std::size_t control = 0; control < 3; ++control)
              for (std::size_t policy = 0; policy < 6; ++policy) {
                UwfStateFacts state;
                state.current.filterEnabled = boolean(filter);
                if (types[type]) state.current.overlayType = Fact<std::uint32_t>::known(*types[type]);
                state.current.overlayFlags = persistent == 0 ? BitFacts{} : BitFacts{2, persistent == 2 ? 2u : 0u};
                state.runtime.controlDevicePresent = boolean(control);
                if (saved[policy]) state.current.savedMode = Fact<std::uint32_t>::known(*saved[policy]);
                const bool possible =
                    canBeTrue[filter] && canBeDisk[type] && canBeTrue[persistent] && canBeTrue[control] && (mode != 255 || canResolve[policy]);
                if (rejects(SetNumber{NumberProperty::ResetPersistentState, mode}, state) == possible) QFAIL("Runtime reset partial oracle mismatch");
                ++checked;
              }
    QCOMPARE(checked, std::size_t{1944});
  }
  void targetMembershipAndPhysicalRulesHaveLiteralPartitions() {
    const std::array<Request, 7> requests{AddVolume{oracle::volume, {}},
                                          RemoveVolume{oracle::volume},
                                          SetVolume{oracle::volume, VolumeProperty::Protected, true},
                                          SetVolume{oracle::volume, VolumeProperty::Protected, false},
                                          SetVolume{oracle::volume, VolumeProperty::BindByDriveLetter, true},
                                          SetVolume{oracle::volume, VolumeProperty::BindByDriveLetter, false},
                                          CreateSwapfile{oracle::volume}};
    const std::array<std::optional<VolumeAdmission>, 4> physical{std::nullopt, VolumeAdmission{false, false}, VolumeAdmission{true, false},
                                                                 VolumeAdmission{true, true}};
    constexpr std::array<std::array<std::string_view, 3>, 7> expected{{{{"CRRC", "CRRC", "RRRR"}},
                                                                       {{"CCCC", "RRRR", "CCCC"}},
                                                                       {{"CCCC", "CRRC", "CCCC"}},
                                                                       {{"CCCC", "RRRR", "CCCC"}},
                                                                       {{"CRCC", "RRRR", "CRCC"}},
                                                                       {{"CRCC", "RRRR", "CRCC"}},
                                                                       {{"CCCC", "CRRC", "CCCC"}}}};
    std::size_t checked = 0;
    for (std::size_t operation = 0; operation < requests.size(); ++operation)
      for (std::size_t present = 0; present < 3; ++present)
        for (std::size_t medium = 0; medium < 4; ++medium) {
          UwfStateFacts state;
          state.next.volumes[oracle::volume].present = boolean(present);
          if (physical[medium]) state.targets[oracle::volume].volumeEligibility = Fact<VolumeAdmission>::known(*physical[medium]);
          QCOMPARE(rejects(requests[operation], state), expected[operation][present][medium] == 'R');
          ++checked;
        }
    QCOMPARE(checked, std::size_t{84});
  }
  void observationQueriesHonorExactSessionAbsence() {
    for (const auto session : {Session::Current, Session::Next})
      for (const auto present : {std::optional<bool>{}, std::optional<bool>{false}, std::optional<bool>{true}}) {
        UwfStateFacts state;
        auto& selected = session == Session::Current ? state.current : state.next;
        auto& other = session == Session::Current ? state.next : state.current;
        if (present) selected.volumes[oracle::volume].present = Fact<bool>::known(*present);
        other.volumes[oracle::volume].present = Fact<bool>::known(false);
        const std::array<Request, 5> requests{GetVolumeProperty{oracle::volume, VolumeProperty::Protected, session},
                                              GetVolumeProperty{oracle::volume, VolumeProperty::BindByDriveLetter, session},
                                              GetFileExclusions{oracle::volume, session}, FindFileExclusion{oracle::volume, "\\entry", session},
                                              GetSwapfileSize{oracle::volume, session}};
        for (const auto& request : requests) QCOMPARE(rejects(request, state), present.has_value() && !*present);
      }
  }
  void maintenancePartialFactsUseIndependentCompleteTruthTables() {
    constexpr std::array<std::string_view, 2> complete{"AAAAAAAAAAAAARRR", "RRRRRRRRRRRRARRR"};
    // Captured IsReadOnlyMediaUnsafe: disposition 1/2/3 suppress loaded ROM;
    // disposition 0/4/5/6/7 retain it. HORM independently blocks maintenance.
    constexpr std::string_view dispositionRom = "RAAARRRR";
    const std::array<Request, 4> requests{CommitFile{oracle::volume, "\\entry"}, CommitFileDeletion{oracle::volume, "\\entry"},
                                          CommitRegistry{oracle::key, std::string{"value"}}, CommitRegistryDeletion{oracle::key, std::string{"value"}}};
    std::size_t checked = 0;
    for (std::size_t operation = 0; operation < 4; ++operation)
      for (std::size_t filter = 0; filter < 3; ++filter)
        for (std::size_t protection = 0; protection < 3; ++protection)
          for (std::size_t horm = 0; horm < 3; ++horm)
            for (std::size_t rom = 0; rom < 3; ++rom)
              for (std::size_t disposition = 0; disposition < 9; ++disposition) {
                const auto f = boolean(filter), p = boolean(protection), h = boolean(horm), r = boolean(rom);
                bool possible = false;
                for (const auto fullF : oracle::completions(f))
                  for (const auto fullP : oracle::completions(p))
                    for (const auto fullH : oracle::completions(h))
                      for (const auto loadedRom : oracle::completions(r))
                        for (std::size_t d = 0; d < dispositionRom.size(); ++d) {
                          if (disposition != 0 && d != disposition - 1) continue;
                          const bool fullR = loadedRom && dispositionRom[d] == 'R';
                          const std::size_t index = (fullF ? 8U : 0U) + (fullP ? 4U : 0U) + (fullH ? 2U : 0U) + (fullR ? 1U : 0U);
                          possible = possible || complete[operation % 2][index] == 'A';
                        }
                UwfStateFacts state;
                state.current.filterEnabled = f;
                state.runtime.horm = h;
                state.current.overlayFlags = rom == 0 ? BitFacts{} : BitFacts{4, rom == 2 ? 4u : 0u};
                if (disposition != 0) state.runtime.romDisposition = BitFacts::known(static_cast<std::uint32_t>(disposition - 1));
                state.current.volumes[oracle::volume].present = Fact<bool>::known(true);
                state.current.volumes[oracle::volume].protectedVolume = p;
                state.systemVolumeId = Fact<std::string>::known(oracle::volume);
                if (rejects(requests[operation], state) == possible) QFAIL("Maintenance partial oracle mismatch");
                ++checked;
              }
    QCOMPARE(checked, std::size_t{2916});
  }
  void servicingPartialBranchesHaveIndependentLiteralOracle() {
    // T=filter transition, N=no transition, R=blocked. Rows are saved(Missing/false/true) x nextFilter(false/true).
    // Captured status 0/1/2/3/other cases use pre-operation facts, not the later status or raw S_OK.
    constexpr std::array<std::array<std::string_view, 5>, 2> branches{
        {{{"NNNNNN", "RRNNTT", "RRRRRR", "RRNNTT", "RRRRRR"}}, {{"TTTTTT", "NRNRNR", "RRRRRR", "TTTTTT", "RRRRRR"}}}};
    // currentFilter(false/true) x control(false/true) x nextType(RAM/Disk/other).
    constexpr std::string_view filterPath = "AARAARRRRAAR";
    constexpr std::string_view noFilterPath = "AARAAAAARAAA";
    const auto choices = [](std::size_t observed, std::size_t count) {
      std::vector<std::size_t> result;
      if (observed)
        result.push_back(observed - 1);
      else
        for (std::size_t i = 0; i < count; ++i) result.push_back(i);
      return result;
    };
    const auto missing = Fact<std::optional<bool>>::known(std::optional<bool>{});
    QVERIFY(missing.isKnown());
    QVERIFY(missing.value);
    QVERIFY(!missing.value->has_value());
    std::size_t checked = 0;
    for (std::size_t enabled = 0; enabled < 2; ++enabled)
      for (std::size_t status = 0; status < 6; ++status)
        for (std::size_t saved = 0; saved < 4; ++saved)
          for (std::size_t next = 0; next < 3; ++next)
            for (std::size_t current = 0; current < 3; ++current)
              for (std::size_t control = 0; control < 3; ++control)
                for (std::size_t type = 0; type < 4; ++type) {
                  bool possible = false;
                  for (const auto fullStatus : choices(status, 5))
                    for (const auto fullSaved : choices(saved, 3))
                      for (const auto fullNext : choices(next, 2))
                        for (const auto fullCurrent : choices(current, 2))
                          for (const auto fullControl : choices(control, 2))
                            for (const auto fullType : choices(type, 3)) {
                              const auto path = branches[enabled][fullStatus][fullSaved * 2 + fullNext];
                              const auto environment = fullCurrent * 6 + fullControl * 3 + fullType;
                              possible = possible || (path == 'T' && filterPath[environment] == 'A') || (path == 'N' && noFilterPath[environment] == 'A');
                            }
                  UwfStateFacts state;
                  state.administrator = Fact<bool>::known(true);
                  state.runtime.horm = Fact<bool>::known(false);
                  state.current.overlayFlags = BitFacts::known(0);
                  state.runtime.romDisposition = BitFacts::known(0);
                  state.current.filterEnabled = boolean(current);
                  state.next.filterEnabled = boolean(next);
                  state.runtime.controlDevicePresent = boolean(control);
                  if (status) state.runtime.servicingStatus = Fact<std::uint32_t>::known(static_cast<std::uint32_t>(status - 1));
                  if (saved == 1) state.runtime.savedServicingFilter = missing;
                  if (saved > 1) state.runtime.savedServicingFilter = Fact<std::optional<bool>>::known(saved == 3);
                  if (type) state.next.overlayType = Fact<std::uint32_t>::known(static_cast<std::uint32_t>(type - 1));
                  if (rejects(SetBoolean{BooleanProperty::ServicingEnabled, enabled != 0}, state) == possible) {
                    QFAIL(qPrintable(QStringLiteral("Servicing partial oracle mismatch enable=%1 status=%2 saved=%3 next=%4 current=%5 control=%6 type=%7")
                                         .arg(enabled)
                                         .arg(status)
                                         .arg(saved)
                                         .arg(next)
                                         .arg(current)
                                         .arg(control)
                                         .arg(type)));
                  }
                  ++checked;
                }
    QCOMPARE(checked, std::size_t{5184});
  }
  void malformedObservationIsNotMalformedStoredConfiguration() {
    UwfStateFacts state;
    state.next.volumes[oracle::volume].present = Fact<bool>::known(true);
    state.targets[oracle::volume].volumeEligibility = Fact<VolumeAdmission>::known({false, true});
    QVERIFY(!rejects(SetVolume{oracle::volume, VolumeProperty::BindByDriveLetter, true}, state));
    const Request request = CommitFile{oracle::volume, "\\entry"};
    state = {};
    state.requestTarget = request;
    state.targets[oracle::volume].fileTarget = Fact<FileMaintenanceFacts>::known({260, static_cast<FileMaintenanceKind>(99), true});
    QVERIFY(!rejects(request, state));
    state.current.filterEnabled = Fact<bool>::known(true);
    state.current.volumes[oracle::volume].protectedVolume = Fact<bool>::known(true);
    state.current.overlayFlags = BitFacts::known(4);
    QVERIFY(!rejects(request, state));
    state.runtime.romDisposition = BitFacts::known(0);
    QVERIFY(rejects(request, state));
    auto stored = hormFacts();
    stored.current.overlayType = Fact<std::uint32_t>::known(2);
    QVERIFY(rejects(SetBoolean{BooleanProperty::HormEnabled, true}, stored));
    QVERIFY(!rejects(GetNumber{NumberProperty::OverlayType}, stored));
  }

  void fileTargetPartitionsRespectScopeAndDoNotRejectMissingDeletion() {
    constexpr std::array<FileMaintenanceKind, 4> kinds{FileMaintenanceKind::Missing, FileMaintenanceKind::File, FileMaintenanceKind::EmptyDirectory,
                                                       FileMaintenanceKind::NonemptyDirectory};
    constexpr std::array<std::size_t, 4> lengths{5, 259, 260, 261};
    constexpr std::array<std::array<std::string_view, 4>, 2> expected{{{{"RCRR", "RCRR", "RRRR", "RRRR"}}, {{"CCCR", "CCCR", "RRRR", "RRRR"}}}};
    std::size_t checked = 0;
    for (std::size_t operation = 0; operation < 2; ++operation)
      for (std::size_t length = 0; length < lengths.size(); ++length)
        for (std::size_t kind = 0; kind < kinds.size(); ++kind)
          for (bool bound : {false, true}) {
            const auto path = std::string("\\") + std::string(lengths[length] - 3, 'x');
            const Request request = operation == 0 ? Request{CommitFile{oracle::volume, path}} : Request{CommitFileDeletion{oracle::volume, path}};
            UwfStateFacts state;
            state.requestTarget = bound ? std::optional<Request>{request} : std::optional<Request>{CommitFile{oracle::volume, "\\other"}};
            state.targets[oracle::volume].fileTarget = Fact<FileMaintenanceFacts>::known({lengths[length], kinds[kind], false});
            QCOMPARE(rejects(request, state), bound && expected[operation][length][kind] == 'R');
            ++checked;
          }
    QCOMPARE(checked, std::size_t{64});
    for (const auto& request : {Request{CommitFile{oracle::volume, "\\pagefile.sys"}}, Request{CommitFileDeletion{oracle::volume, "\\pagefile.sys"}}}) {
      UwfStateFacts state;
      state.requestTarget = request;
      state.targets[oracle::volume].fileTarget = Fact<FileMaintenanceFacts>::known({15, FileMaintenanceKind::File, true});
      QVERIFY(rejects(request, state));
    }
  }
  void staticArgumentsAndIgnoredClearFieldsAreIndependent() {
    const std::vector<Request> invalid{SetBoolean{BooleanProperty::OverlayCommit, false},
                                       SetNumber{NumberProperty::OverlayType, 2},
                                       SetNumber{NumberProperty::MaximumOverlaySize, 0},
                                       SetNumber{NumberProperty::ResetPersistentState, 2},
                                       SetNumber{NumberProperty::ResetPersistentStateSavedMode, 0},
                                       GetVolumeProperty{"invalid", VolumeProperty::Protected},
                                       CommitRegistry{oracle::key, std::nullopt},
                                       CommitRegistryDeletion{oracle::key, std::string{}},
                                       CommitRegistry{"HKLM\\SYSTEM\\CurrentControlSetExtra\\Child", std::string{"value"}},
                                       CommitFile{oracle::volume, std::string("\\x\0other", 8)}};
    for (const auto& request : invalid) {
      const auto result = solveUwfRequest(request, {});
      QVERIFY(std::holds_alternative<Reject>(result.decision));
      QVERIFY(!result.parameters.staticallyValid);
      QVERIFY(result.arguments == request);
    }
    for (const auto& slot : oracle::capabilitySlots()) {
      auto request = slot.request;
      std::visit(
          [&](auto& operation) {
            using R = std::decay_t<decltype(operation)>;
            if constexpr (std::is_same_v<R, EditFileExclusion>) {
              if (operation.action == ExclusionAction::Clear) operation.path = std::string("bad\0unused", 10);
            } else if constexpr (std::is_same_v<R, EditRegistryExclusion>) {
              if (operation.action == ExclusionAction::Clear) operation.key = std::string("bad\0unused", 10);
            }
          },
          request);
      const auto result = solveUwfRequest(request, {});
      QCOMPARE(std::holds_alternative<Reject>(result.decision), slot.rejectWithNoFacts);
      QVERIFY(result.arguments == request);
    }
  }
};
QTEST_APPLESS_MAIN(UwfFailureSolverTests)
#include "UwfFailureSolverTests.moc"
