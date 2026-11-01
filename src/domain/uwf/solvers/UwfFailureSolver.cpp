// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "domain/uwf/solvers/UwfFailureSolver.h"

#include <array>
#include <type_traits>

#include "domain/uwf/policies/IndividualCommitPolicy.h"
#include "domain/uwf/policies/ConfigurationModePolicy.h"
#include "domain/uwf/policies/UwfConstraints.h"

namespace uwf::compat {
namespace {
template <class T>
bool is(const Fact<T>& fact, const T value) {
  return fact.value && *fact.value == value;
}
bool yes(const std::optional<bool> value) { return value && *value; }
bool no(const std::optional<bool> value) { return value && !*value; }
struct Conclusions {
  std::vector<Reject> failures;
  void add(const bool proven, const RuleId rule, const ErrorKind kind, const char* reason) {
    if (proven) failures.push_back({rule, kind, reason});
  }
  void authorize(const UwfStateFacts& state, const bool allowHorm = false) {
    add(is(state.administrator, false), RuleId::AdministratorRequired, ErrorKind::AccessDenied, "This UWF operation requires an administrator");
    add(!allowHorm && is(state.runtime.horm, true), RuleId::HormBlocksConfiguration, ErrorKind::AccessDenied,
        "This UWF configuration operation is unavailable while HORM is enabled");
  }
  void offline(const UwfStateFacts& state) {
    add(is(state.current.filterEnabled, true), RuleId::CurrentFilterMustBeOff, ErrorKind::AccessDenied,
        "This UWF operation requires UWF disabled in the current session");
  }
  void control(const UwfStateFacts& state) {
    add(is(state.runtime.controlDevicePresent, false), RuleId::ControlDeviceRequired, ErrorKind::Unavailable, "The required UWF control device is unavailable");
  }
  void filterTransition(const UwfStateFacts& state) {
    add(state.next.overlayType.value && *state.next.overlayType.value > 1, RuleId::InvalidNextOverlayType, ErrorKind::Protocol,
        "The next-session overlay type is not recognized");
    if (is(state.current.filterEnabled, true)) {
      control(state);
      add(yes(state.current.overlayFlags.hasAny(flags::readOnlyMedia)) && no(state.runtime.romDisposition.equals(0)) &&
              no(state.runtime.romDisposition.hasAny(disposition::filterChange | disposition::fullCommit)),
          RuleId::FilterDispositionConflict, ErrorKind::Conflict, "The ROM disposition does not permit a filter change");
    }
  }
};
const VolumeStateFacts& volume(const SessionStateFacts& session, const std::string& id) {
  static const VolumeStateFacts unknown;
  const auto entry = session.volumes.find(normalizeVolumeId(id));
  return entry == session.volumes.end() ? unknown : entry->second;
}
const TargetStateFacts& target(const UwfStateFacts& state, const std::string& id) {
  static const TargetStateFacts unknown;
  const auto entry = state.targets.find(normalizeVolumeId(id));
  return entry == state.targets.end() ? unknown : entry->second;
}
const VolumeAdmission* eligibility(const UwfStateFacts& state, const std::string& id) {
  const auto& observed = target(state, id).volumeEligibility.value;
  // Eligibility includes fixed media. An inconsistent observation supplies no
  // compatible physical state from which to infer a business failure.
  return observed && (observed->fixedMedia || !observed->protectionEligible) ? &*observed : nullptr;
}
void targetMembership(Conclusions& out, const UwfStateFacts& state, const std::string& id, const bool registration, const bool binding) {
  const auto& next = volume(state.next, id);
  const auto* physical = eligibility(state, id);
  if (registration) {
    out.add(is(next.present, true), RuleId::ExistingNextVolume, ErrorKind::Conflict, "The UWF volume is already configured for the next session");
    out.add(physical && !physical->protectionEligible, RuleId::IneligibleVolume, ErrorKind::Conflict, "The physical volume cannot be registered with UWF");
  } else if (binding) {
    out.add(is(next.present, false), RuleId::MissingNextVolume, ErrorKind::NotFound, "The UWF volume is not configured for the next session");
    out.add(physical && !physical->fixedMedia, RuleId::FixedMediaRequired, ErrorKind::Conflict, "Volume binding requires fixed media");
  }
}
void fileTarget(Conclusions& out, const Request& request, const UwfStateFacts& state, const std::string& id, const bool deletion) {
  if (!state.requestTarget || *state.requestTarget != request) return;
  const auto& facts = target(state, id);
  out.add(requiresPersistentFileView(state, request, id) && core::individualCommitRestriction(true, true, {}, facts.persistentFileViewReady.value) ==
                                                                core::IndividualCommitRestriction::PersistentViewUnavailable,
          RuleId::PersistentFileViewUnavailable, ErrorKind::Unavailable, "The shared persistent file view is not ready");
  const auto& observed = facts.fileTarget;
  if (!observed.value) return;
  const auto& file = *observed.value;
  if (file.kind < FileMaintenanceKind::Missing || file.kind > FileMaintenanceKind::NonemptyDirectory) return;
  out.add(file.fullPathUtf16Units >= limits::fileMaintenanceBufferUnits, RuleId::FileTargetTooLong, ErrorKind::InvalidArgument,
          "The complete file target exceeds the management path buffer");
  out.add(file.reserved, RuleId::ReservedFileTarget, ErrorKind::InvalidArgument, "The exact file maintenance target is reserved by UWF");
  if (!deletion) {
    out.add(file.kind == FileMaintenanceKind::Missing, RuleId::MissingFileTarget, ErrorKind::NotFound, "The exact file maintenance target is missing");
    out.add(file.kind == FileMaintenanceKind::EmptyDirectory || file.kind == FileMaintenanceKind::NonemptyDirectory, RuleId::InvalidFileTargetKind,
            ErrorKind::InvalidArgument, "Ordinary file commit requires a file target");
  } else {
    out.add(file.kind == FileMaintenanceKind::NonemptyDirectory, RuleId::NonemptyDirectory, ErrorKind::Conflict,
            "Persistent directory deletion requires an empty directory");
  }
}
void maintenance(Conclusions& out, const UwfStateFacts& state, const std::optional<std::string>& id, const bool deletion) {
  Fact<bool> protection;
  Fact<bool> present;
  if (id) {
    const auto& current = volume(state.current, *id);
    protection = current.protectedVolume;
    present = current.present;
    out.add(is(present, false), RuleId::MissingCurrentVolume, ErrorKind::NotFound, "The file target volume is not configured for the current session");
  } else if (state.systemVolumeId.value) {
    if (const auto systemId = parseVolumeId(*state.systemVolumeId.value)) {
      const auto& current = volume(state.current, *systemId);
      protection = is(current.present, false) ? Fact<bool>::known(false) : current.protectedVolume;
    }
  }
  if (deletion) {
    out.add(is(state.current.filterEnabled, false) || is(protection, false), RuleId::MaintenanceNeedsProtection, ErrorKind::Conflict,
            "UWF persistent deletion requires an active protected Current target");
  }
  const bool requiresProtectedOperation = deletion || (is(state.current.filterEnabled, true) && is(protection, true));
  if (requiresProtectedOperation) {
    const auto restriction = core::individualCommitRestriction(true, true, individualCommitState(state));
    out.add(restriction == core::IndividualCommitRestriction::HormActive, RuleId::HormBlocksMaintenance, ErrorKind::AccessDenied,
            "Individual UWF maintenance is unavailable while HORM is enabled");
    out.add(restriction == core::IndividualCommitRestriction::ReadOnlyMediaActive, RuleId::RomBlocksMaintenance, ErrorKind::AccessDenied,
            "Individual UWF maintenance is unavailable in the loaded Read-Only Media overlay");
  }
}
// Prove failure only when every branch compatible with the partial observations is blocked.
void servicing(Conclusions& out, const UwfStateFacts& state, const bool enabled) {
  // One representative covers all unrecognized raw statuses.
  constexpr auto unrecognizedStatus = servicingStatus::failed + 1;
  constexpr std::array statuses{servicingStatus::notStarted, servicingStatus::started, servicingStatus::completed, servicingStatus::failed, unrecognizedStatus};
  std::vector<Reject> blocked;
  for (const auto status : statuses) {
    if (state.runtime.servicingStatus.value) {
      const auto actual = *state.runtime.servicingStatus.value;
      if (status != (actual > servicingStatus::failed ? unrecognizedStatus : actual)) continue;
    }
    Conclusions branch;
    branch.add(status == servicingStatus::completed || status == unrecognizedStatus, RuleId::InvalidServicingStatus, ErrorKind::Conflict,
               "The servicing runtime status does not permit this transition");
    branch.add(enabled && status == servicingStatus::started && is(state.next.filterEnabled, true), RuleId::ServicingFilterConflict, ErrorKind::Conflict,
               "The active servicing transition cannot confirm an enabled next-session filter");
    const bool exit = !enabled && (status == servicingStatus::started || status == servicingStatus::failed);
    const auto& saved = state.runtime.savedServicingFilter.value;
    branch.add(exit && saved && !saved->has_value(), RuleId::MissingServicingRestoreState, ErrorKind::Conflict,
               "The servicing transition cannot restore a missing saved filter state");
    if ((enabled && (status == servicingStatus::notStarted || status == servicingStatus::failed)) || (exit && saved && saved->value_or(false)))
      branch.filterTransition(state);
    if (branch.failures.empty()) return;
    blocked.push_back(branch.failures.front());
  }
  if (blocked.empty()) return;
  if (state.runtime.servicingStatus.value) {
    out.failures.push_back(std::move(blocked.front()));
  } else {
    out.add(true, RuleId::AllPathsRejected, ErrorKind::Conflict, "Every compatible servicing transition is blocked by the known facts");
  }
}
}  // namespace

SolverEvaluation solveUwfRequest(const Request& request, const UwfStateFacts& facts) {
  SolverEvaluation result{request, Continue{}, {operationName(request), true}};
  if (const auto* number = std::get_if<SetNumber>(&request); number && number->property == NumberProperty::OverlayFlags) {
    result.parameters.staticallyValid = false;
    result.decision = Reject{RuleId::InvalidRequest, ErrorKind::Unsupported, "Change one overlay option instead of writing raw flags"};
    return result;
  }
  if (const auto failure = requestInputFailure(request)) {
    result.parameters.staticallyValid = false;
    result.decision = Reject{RuleId::InvalidRequest, failure->kind, failure->message};
    return result;
  }
  if (!facts.observation.applicable) return result;
  Conclusions out;
  std::visit(
      [&](const auto& operation) {
        using R = std::decay_t<decltype(operation)>;
        if constexpr (std::is_same_v<R, SetBoolean>) {
          switch (operation.property) {
            case BooleanProperty::FilterEnabled:
              out.authorize(facts);
              if (facts.next.filterEnabled.value && *facts.next.filterEnabled.value != operation.value) out.filterTransition(facts);
              break;
            case BooleanProperty::HormEnabled:
              out.authorize(facts, true);
              if (facts.runtime.horm.value && *facts.runtime.horm.value != operation.value) {
                out.control(facts);
                if (operation.value) {
                  out.add(is(facts.current.filterEnabled, false), RuleId::HormEnableNeedsCurrentFilter, ErrorKind::Conflict,
                          "Enabling HORM requires an active UWF filter");
                  out.add(facts.current.overlayType.value && *facts.current.overlayType.value != 0, RuleId::HormEnableNeedsRam, ErrorKind::Conflict,
                          "Enabling HORM requires a loaded RAM overlay");
                  out.add(no(facts.current.overlayFlags.equals(flags::readOnlyMedia)), RuleId::HormEnableNeedsRomFlags, ErrorKind::Conflict,
                          "Enabling HORM requires exactly the loaded Read-Only Media overlay flags");
                  out.add(is(facts.runtime.hiberFile, false), RuleId::HormEnableNeedsHiberFile, ErrorKind::Conflict,
                          "Enabling HORM requires an existing hibernation file");
                  out.add(is(facts.runtime.hormVolumesProtected, false), RuleId::HormEnableNeedsProtectedVolumes, ErrorKind::Conflict,
                          "Enabling HORM requires protection of every eligible mounted fixed volume");
                  out.add(no(facts.runtime.romDisposition.equals(0)), RuleId::HormDispositionConflict, ErrorKind::Conflict,
                          "Enabling HORM requires an idle ROM disposition");
                }
              }
              break;
            case BooleanProperty::ServicingEnabled:
              out.authorize(facts);
              out.add(is(facts.runtime.controlDevicePresent, false) && facts.next.overlayType.value && *facts.next.overlayType.value > 1,
                      RuleId::InvalidNextOverlayType, ErrorKind::Protocol, "The servicing reset fallback cannot use an unrecognized next-session overlay type");
              servicing(out, facts, operation.value);
              break;
            case BooleanProperty::OverlayCommit:
              out.add(is(facts.current.filterEnabled, false), RuleId::FullCommitNeedsCurrentFilter, ErrorKind::Conflict,
                      "FullCommit requires UWF enabled in the current session");
              out.control(facts);
              out.add(yes(facts.current.overlayFlags.hasAny(flags::readOnlyMedia)) && no(facts.runtime.romDisposition.equals(0)) &&
                          no(facts.runtime.romDisposition.hasAny(disposition::fullCommit)),
                      RuleId::FullCommitDispositionConflict, ErrorKind::Conflict, "FullCommit requires an idle or already committed ROM disposition");
              break;
            case BooleanProperty::PersistDomainSecretKey:
            case BooleanProperty::PersistTscal:
            case BooleanProperty::ShutdownPending:
            case BooleanProperty::RegistryChangesAlwaysPersisted:
            case BooleanProperty::Count:
              break;
          }
        } else if constexpr (std::is_same_v<R, SetNumber>) {
          out.authorize(facts);
          switch (operation.property) {
            case NumberProperty::OverlayType:
            case NumberProperty::MaximumOverlaySize: {
              out.offline(facts);
              const auto type = operation.property == NumberProperty::OverlayType ? Fact<std::uint32_t>::known(operation.value) : facts.next.overlayType;
              const auto size =
                  operation.property == NumberProperty::MaximumOverlaySize ? Fact<std::uint32_t>::known(operation.value) : facts.next.maximumOverlaySize;
              out.add(type.value && size.value && !core::validOverlayTarget(static_cast<core::OverlayType>(*type.value), *size.value),
                      RuleId::IncompatibleOverlayConfiguration, ErrorKind::InvalidArgument, "The overlay type and maximum size are incompatible");
              break;
            }
            case NumberProperty::ResetPersistentState:
              out.add(is(facts.current.filterEnabled, false), RuleId::ResetNeedsCurrentFilter, ErrorKind::AccessDenied,
                      "Resetting the persistent overlay requires an active filter");
              out.add(facts.current.overlayType.value && *facts.current.overlayType.value != 1, RuleId::ResetNeedsDisk,
                      facts.current.overlayType.value && *facts.current.overlayType.value == 0 ? ErrorKind::AccessDenied : ErrorKind::Protocol,
                      "Resetting the persistent overlay requires a loaded disk overlay");
              out.add(no(facts.current.overlayFlags.hasAny(flags::persistent)), RuleId::ResetNeedsPersistence, ErrorKind::AccessDenied,
                      "Resetting the persistent overlay requires loaded Persistent flags");
              out.control(facts);
              out.add(operation.value == static_cast<std::uint32_t>(ResetMode::Saved) && facts.current.savedMode.value && *facts.current.savedMode.value > 1,
                      RuleId::InvalidSavedResetMode, ErrorKind::InvalidArgument, "The current saved reset mode cannot be executed by this UWF profile");
              break;
            case NumberProperty::ResetPersistentStateSavedMode:
            case NumberProperty::WarningThreshold:
            case NumberProperty::CriticalThreshold:
            case NumberProperty::OverlayFlags:
            case NumberProperty::Count:
              break;
          }
        } else if constexpr (std::is_same_v<R, SetOverlayOption>) {
          if (operation.option != OverlayOption::ReadOnlyMedia) out.authorize(facts);
          out.offline(facts);
        } else if constexpr (std::is_same_v<R, CreateSwapfile>) {
          out.offline(facts);
          if (is(volume(facts.next, operation.volume).present, false)) {
            const auto* physical = eligibility(facts, operation.volume);
            out.add(physical && !physical->protectionEligible, RuleId::IneligibleVolume, ErrorKind::Conflict,
                    "The physical volume cannot be registered with UWF");
          }
        } else if constexpr (std::is_same_v<R, ResetSettings>) {
          out.authorize(facts);
          out.offline(facts);
          out.add(is(facts.capturedSettingsAvailable, false), RuleId::CapturedDefaultsRequired, ErrorKind::Conflict,
                  "The captured UWF defaults are unavailable");
        } else if constexpr (std::is_same_v<R, AddVolume>) {
          targetMembership(out, facts, operation.volume, true, false);
        } else if constexpr (std::is_same_v<R, RemoveVolume>) {
          out.add(is(volume(facts.next, operation.volume).present, false), RuleId::MissingNextVolume, ErrorKind::NotFound,
                  "The UWF volume is not configured for the next session");
        } else if constexpr (std::is_same_v<R, SetVolume>) {
          if (operation.property == VolumeProperty::Protected) {
            out.authorize(facts);
            const auto& next = volume(facts.next, operation.volume);
            out.add(!operation.value && is(next.present, false), RuleId::MissingNextVolume, ErrorKind::NotFound,
                    "The UWF volume is not configured for the next session");
            if (operation.value && is(next.present, false)) {
              const auto* physical = eligibility(facts, operation.volume);
              out.add(physical && !physical->protectionEligible, RuleId::IneligibleVolume, ErrorKind::Conflict,
                      "The physical volume cannot be registered with UWF");
            }
          } else
            targetMembership(out, facts, operation.volume, false, true);
        } else if constexpr (std::is_same_v<R, EditFileExclusion> || std::is_same_v<R, EditRegistryExclusion>) {
          out.authorize(facts);
          if constexpr (std::is_same_v<R, EditFileExclusion>)
            out.add(operation.action == ExclusionAction::Add && is(volume(facts.next, operation.volume).present, false), RuleId::MissingNextVolume,
                    ErrorKind::AccessDenied, "Adding a file exclusion requires a configured next-session UWF volume");
        } else if constexpr (std::is_same_v<R, CommitFile> || std::is_same_v<R, CommitFileDeletion>) {
          constexpr bool deletion = std::is_same_v<R, CommitFileDeletion>;
          maintenance(out, facts, operation.volume, deletion);
          fileTarget(out, request, facts, operation.volume, deletion);
        } else if constexpr (std::is_same_v<R, CommitRegistry> || std::is_same_v<R, CommitRegistryDeletion>) {
          maintenance(out, facts, std::nullopt, std::is_same_v<R, CommitRegistryDeletion>);
        } else if constexpr (std::is_same_v<R, GetOverlayFiles>) {
          out.add(is(facts.current.filterEnabled, false), RuleId::OverlayEnumerationNeedsCurrentFilter, ErrorKind::Conflict,
                  "Overlay file enumeration requires an active Current filter");
          const auto& current = volume(facts.current, operation.volume);
          out.add(is(current.present, false), RuleId::MissingCurrentVolume, ErrorKind::NotFound, "The target volume is not configured for the current session");
          out.add(is(current.protectedVolume, false), RuleId::OverlayEnumerationNeedsProtection, ErrorKind::Conflict,
                  "Overlay file enumeration requires a protected Current volume");
        } else if constexpr (std::is_same_v<R, Power>) {
          if (is(facts.current.filterEnabled, true)) out.control(facts);
        } else if constexpr (std::is_same_v<R, GetVolumeProperty> || std::is_same_v<R, GetFileExclusions> || std::is_same_v<R, FindFileExclusion> ||
                             std::is_same_v<R, GetSwapfileSize>) {
          const auto& selected = operation.session == Session::Current ? facts.current : facts.next;
          out.add(is(volume(selected, operation.volume).present, false),
                  operation.session == Session::Current ? RuleId::MissingCurrentVolume : RuleId::MissingNextVolume, ErrorKind::NotFound,
                  "The target UWF volume is not configured for the selected session");
        } else if constexpr (std::is_same_v<R, GetBoolean> || std::is_same_v<R, GetNumber> || std::is_same_v<R, GetOverlayUsage> ||
                             std::is_same_v<R, GetVolumes> || std::is_same_v<R, GetRegistryExclusions> || std::is_same_v<R, FindRegistryExclusion> ||
                             std::is_same_v<R, UpdateWindows>) {
          // No additional universal failure rule applies after static parameter validation.
        } else
          static_assert(!sizeof(R), "Every abstract request must have a solver classification");
      },
      request);
  if (const auto write = configurationWrite(request)) {
    const auto restriction = core::configurationWriteRestriction(configurationModeState(facts), *write);
    out.add(restriction == core::ConfigurationWriteRestriction::HormActive, RuleId::HormBlocksConfiguration, ErrorKind::AccessDenied,
            core::configurationWriteReason(restriction));
    out.add(restriction == core::ConfigurationWriteRestriction::RomVolatile || restriction == core::ConfigurationWriteRestriction::RomCommitUnavailable,
            RuleId::RomConfigurationNotPersistent, ErrorKind::AccessDenied, core::configurationWriteReason(restriction));
  }
  if (!out.failures.empty()) result.decision = std::move(out.failures.front());
  return result;
}
std::string ruleName(const RuleId rule) {
  switch (rule) {
#define UWF_RULE_NAME(name) \
  case RuleId::name:        \
    return #name
    UWF_RULE_NAME(InvalidRequest);
    UWF_RULE_NAME(AdministratorRequired);
    UWF_RULE_NAME(HormBlocksConfiguration);
    UWF_RULE_NAME(CurrentFilterMustBeOff);
    UWF_RULE_NAME(FullCommitNeedsCurrentFilter);
    UWF_RULE_NAME(FullCommitDispositionConflict);
    UWF_RULE_NAME(HormEnableNeedsCurrentFilter);
    UWF_RULE_NAME(HormEnableNeedsRam);
    UWF_RULE_NAME(HormEnableNeedsRomFlags);
    UWF_RULE_NAME(HormEnableNeedsHiberFile);
    UWF_RULE_NAME(HormEnableNeedsProtectedVolumes);
    UWF_RULE_NAME(HormDispositionConflict);
    UWF_RULE_NAME(ControlDeviceRequired);
    UWF_RULE_NAME(FilterDispositionConflict);
    UWF_RULE_NAME(InvalidNextOverlayType);
    UWF_RULE_NAME(IncompatibleOverlayConfiguration);
    UWF_RULE_NAME(ResetNeedsCurrentFilter);
    UWF_RULE_NAME(ResetNeedsDisk);
    UWF_RULE_NAME(ResetNeedsPersistence);
    UWF_RULE_NAME(InvalidSavedResetMode);
    UWF_RULE_NAME(CapturedDefaultsRequired);
    UWF_RULE_NAME(ExistingNextVolume);
    UWF_RULE_NAME(MissingNextVolume);
    UWF_RULE_NAME(MissingCurrentVolume);
    UWF_RULE_NAME(IneligibleVolume);
    UWF_RULE_NAME(FixedMediaRequired);
    UWF_RULE_NAME(MaintenanceNeedsProtection);
    UWF_RULE_NAME(HormBlocksMaintenance);
    UWF_RULE_NAME(RomBlocksMaintenance);
    UWF_RULE_NAME(PersistentFileViewUnavailable);
    UWF_RULE_NAME(OverlayEnumerationNeedsCurrentFilter);
    UWF_RULE_NAME(OverlayEnumerationNeedsProtection);
    UWF_RULE_NAME(InvalidServicingStatus);
    UWF_RULE_NAME(MissingServicingRestoreState);
    UWF_RULE_NAME(ServicingFilterConflict);
    UWF_RULE_NAME(FileTargetTooLong);
    UWF_RULE_NAME(ReservedFileTarget);
    UWF_RULE_NAME(MissingFileTarget);
    UWF_RULE_NAME(InvalidFileTargetKind);
    UWF_RULE_NAME(NonemptyDirectory);
    UWF_RULE_NAME(AllPathsRejected);
    UWF_RULE_NAME(RomConfigurationNotPersistent);
#undef UWF_RULE_NAME
  }
  return "InvalidRule";
}
}  // namespace uwf::compat
