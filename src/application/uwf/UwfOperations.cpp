// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "application/uwf/UwfOperations.h"

#include <algorithm>
#include <exception>
#include <limits>
#include <type_traits>
#include <utility>

#include "domain/uwf/policies/ExclusionCoverage.h"
#include "domain/uwf/policies/ConfigurationModePolicy.h"
#include "domain/uwf/policies/IndividualCommitPolicy.h"
#include "domain/uwf/policies/UwfConstraints.h"
#include "foundation/paths/DriveLetter.h"
#include "foundation/paths/RegistryPath.h"
#include "foundation/text/StringUtil.h"
#include "foundation/text/Utf8.h"

namespace uwf::app {
namespace {
using namespace compat;
const SessionStateFacts& sessionFacts(const UwfStateFacts& facts, const Session session) { return session == Session::Current ? facts.current : facts.next; }
const VolumeStateFacts* volumeFacts(const SessionStateFacts& session, const std::string& volume) {
  const auto found = session.volumes.find(normalizeVolumeId(volume));
  return found == session.volumes.end() ? nullptr : &found->second;
}
std::optional<bool> presence(const SessionStateFacts& session, const std::string& volume) {
  if (const auto* target = volumeFacts(session, volume)) return target->present.value;
  return session.volumesObserved ? std::optional{false} : std::nullopt;
}
const Fact<std::vector<std::string>>& exclusionFacts(const UwfStateFacts& facts, const GetFileExclusions& request) {
  if (const auto* target = volumeFacts(sessionFacts(facts, request.session), request.volume)) return target->fileExclusions;
  static const Fact<std::vector<std::string>> unknown;
  return unknown;
}
Capability combineCapability(Capability first, const Capability& next) {
  if (first.support == Support::Unsupported || first.support == Support::Unavailable) return first;
  if (next.support == Support::Unsupported || next.support == Support::Unavailable) return next;
  return first.support == Support::Unknown ? first : next.support == Support::Unknown ? next : first;
}

class Operation {
 public:
  Operation(const Uwf& downstream, const Context& context) : downstream(downstream), context(context), facts(context.facts ? *context.facts : unknownFacts) {}
  [[noreturn]] void fail(const ErrorKind kind, std::string message) const { throw Error({kind, effect, std::move(message), {}, readCompletion}, attempts); }
  template <class T>
  const T& require(const Fact<T>& fact, const char* description) const {
    if (!fact.value) fail(ErrorKind::Unavailable, std::string("The shared UWF state has no ") + description);
    return *fact.value;
  }
  void authorize(const bool allowHorm = false) const {
    if (facts.administrator.value == false) fail(ErrorKind::AccessDenied, "This UWF operation requires an administrator");
    if (!allowHorm && facts.runtime.horm.value == true) fail(ErrorKind::AccessDenied, "This UWF operation is unavailable while HORM is enabled");
  }
  void offline() const {
    if (facts.current.filterEnabled.value == true) fail(ErrorKind::AccessDenied, "This UWF operation requires UWF disabled in the current session");
  }
  void eligible(const std::string& volume, const bool binding) const {
    const auto found = facts.targets.find(normalizeVolumeId(volume));
    if (found == facts.targets.end()) fail(ErrorKind::Unavailable, "The shared UWF state has no physical volume admission");
    const auto& value = require(found->second.volumeEligibility, "physical volume admission");
    if (!binding && value.protectionEligible && !value.fixedMedia) fail(ErrorKind::Protocol, "The shared physical volume admission is inconsistent");
    if (binding ? !value.fixedMedia : !value.protectionEligible)
      fail(ErrorKind::Conflict, binding ? "Volume binding requires fixed media" : "The physical volume cannot be registered with UWF");
  }
  void fileIdentity(const std::string& volume) const {
    const auto* current = volumeFacts(facts.current, volume);
    if (facts.current.filterEnabled.value != true || !current || current->protectedVolume.value != true) return;
    // The management entry points route through Current's configured letter.
    // A GUID argument alone does not keep an empty or stale letter on target.
    const auto& configured = require(current->driveLetter, "Current configured drive letter");
    if (configured.empty()) fail(ErrorKind::Unsupported, "Individual file maintenance requires an existing drive-letter mapping");
    const auto& physical = require(current->physicalDriveLetter, "physical drive letter");
    if (physical.empty()) fail(ErrorKind::Unsupported, "Individual file maintenance requires an existing drive-letter mapping");
    const auto configuredDrive = drive::normalize(configured);
    const auto physicalDrive = drive::normalize(physical);
    if (configured.size() != 2 || physical.size() != 2 || configuredDrive.empty() || physicalDrive.empty())
      fail(ErrorKind::Protocol, "The shared file maintenance mapping is not an exact drive prefix");
    if (configuredDrive != physicalDrive) fail(ErrorKind::Conflict, "The configured Current drive letter does not identify the selected physical volume");
  }
  void fileTarget(const Request& request, const std::string& volume, const bool deletion) const {
    if (!facts.requestTarget || *facts.requestTarget != request) return;
    const auto found = facts.targets.find(normalizeVolumeId(volume));
    if (found == facts.targets.end()) return;
    const auto& observed = found->second;
    if (observed.fileTarget.value) {
      const auto& target = *observed.fileTarget.value;
      if (target.kind < FileMaintenanceKind::Missing || target.kind > FileMaintenanceKind::NonemptyDirectory)
        fail(ErrorKind::Protocol, "The shared file target kind is not recognized");
      if (target.fullPathUtf16Units >= limits::fileMaintenanceBufferUnits)
        fail(ErrorKind::InvalidArgument, "The complete file target exceeds the management path buffer");
      if (target.reserved) fail(ErrorKind::InvalidArgument, "The exact file maintenance target is reserved by UWF");
      if (!deletion) {
        if (target.kind == FileMaintenanceKind::Missing) fail(ErrorKind::NotFound, "The exact file maintenance target is missing");
        if (target.kind != FileMaintenanceKind::File) fail(ErrorKind::InvalidArgument, "Ordinary file commit requires a file target");
      } else if (target.kind == FileMaintenanceKind::NonemptyDirectory) {
        fail(ErrorKind::Conflict, "Persistent directory deletion requires an empty directory");
      }
    }
    if (requiresPersistentFileView(facts, request, volume) && core::individualCommitRestriction(true, true, {}, observed.persistentFileViewReady.value) ==
                                                                  core::IndividualCommitRestriction::PersistentViewUnavailable)
      fail(ErrorKind::Unavailable, "The shared persistent file view is not ready");
  }
  Response send(const Request& request) {
    if (context.stop.stop_requested()) fail(ErrorKind::Canceled, "UWF operation canceled before dispatch");
    if (const auto write = configurationWrite(request)) {
      const auto restriction = core::configurationWriteRestriction(configurationModeState(facts), *write);
      if (restriction != core::ConfigurationWriteRestriction::None && restriction != core::ConfigurationWriteRestriction::Unknown)
        fail(ErrorKind::AccessDenied, core::configurationWriteReason(restriction));
    }
    Response response;
    try {
      const auto wire = std::visit(
          [](const auto& operation) -> Request {
            using R = std::decay_t<decltype(operation)>;
            if constexpr (std::is_same_v<R, CommitRegistry> || std::is_same_v<R, CommitRegistryDeletion>) {
              auto canonical = operation;
              canonical.key = regkey::expandHive(canonical.key);
              return canonical;
            }
            return operation;
          },
          request);
      const bool enumeration = std::holds_alternative<GetOverlayFiles>(request);
      if (enumeration) readCompletion = ReadCompletion::Unknown;
      auto raw = downstream.execute(wire, context);
      if (enumeration) readCompletion = raw.failure ? raw.failure->readCompletion : ReadCompletion::ProviderCompleted;
      response = normalizeResponse(request, std::move(raw));
    } catch (const Error& error) {
      if (std::holds_alternative<GetOverlayFiles>(request)) readCompletion = error.failure().readCompletion;
      auto failure = error.failure();
      failure.effect = combineEffects(effect, failure.effect);
      auto history = attempts;
      history.insert(history.end(), error.attempts().begin(), error.attempts().end());
      throw Error(std::move(failure), std::move(history));
    } catch (const std::exception& error) {
      if (isWrite(request)) effect = combineEffects(effect, Effect::Uncertain);
      if (std::holds_alternative<GetOverlayFiles>(request)) throw Error({ErrorKind::Failed, effect, error.what(), {}, readCompletion}, attempts);
      fail(ErrorKind::Failed, error.what());
    } catch (...) {
      if (isWrite(request)) effect = combineEffects(effect, Effect::Uncertain);
      if (std::holds_alternative<GetOverlayFiles>(request))
        throw Error({ErrorKind::Failed, effect, "UWF primitive raised an unknown exception", {}, readCompletion}, attempts);
      fail(ErrorKind::Failed, "UWF primitive raised an unknown exception");
    }
    effect = combineEffects(effect, response.effect);
    attempts.insert(attempts.end(), response.attempts.begin(), response.attempts.end());
    warnings.insert(warnings.end(), response.warnings.begin(), response.warnings.end());
    if (response.failure) {
      auto failure = std::move(*response.failure);
      failure.effect = combineEffects(effect, failure.effect);
      throw Error(std::move(failure), attempts);
    }
    return response;
  }
  Response success() const {
    auto response = Response::success(std::monostate{}, effect);
    response.attempts = attempts;
    response.warnings = warnings;
    return response;
  }
  void create(const std::string& volume, const std::string& driveLetter, const bool protect, const bool bindByLetter) {
    eligible(volume, false);
    eligible(volume, true);
    (void)send(AddVolume{volume, driveLetter});
    (void)send(SetVolume{volume, VolumeProperty::Protected, protect});
    (void)send(SetVolume{volume, VolumeProperty::BindByDriveLetter, bindByLetter});
  }
  void maintenance(const std::optional<std::string>& fileVolume, const bool deletion) const {
    const auto enabled = facts.current.filterEnabled.value;
    std::optional<bool> protectedVolume;
    if (fileVolume) {
      if (presence(facts.current, *fileVolume) == false) fail(ErrorKind::NotFound, "The file target volume is not configured for the current session");
      if (const auto* volume = volumeFacts(facts.current, *fileVolume)) protectedVolume = volume->protectedVolume.value;
    } else if (facts.systemVolumeId.value) {
      if (const auto* volume = volumeFacts(facts.current, *facts.systemVolumeId.value)) protectedVolume = volume->protectedVolume.value;
      if (presence(facts.current, *facts.systemVolumeId.value) == false) protectedVolume = false;
    }
    if (enabled == false || protectedVolume == false) {
      if (deletion) fail(ErrorKind::Conflict, "UWF persistent deletion requires an active protected Current target");
      return;
    }
    const auto restriction = core::individualCommitRestriction(true, true, individualCommitState(facts));
    if (restriction == core::IndividualCommitRestriction::HormActive)
      fail(ErrorKind::AccessDenied, "Individual UWF maintenance is unavailable while HORM is enabled");
    if (restriction == core::IndividualCommitRestriction::ReadOnlyMediaActive)
      fail(ErrorKind::AccessDenied, "Individual UWF maintenance is unavailable in the loaded Read-Only Media overlay");
  }
  // Missing observations belong to this operation, avoiding a dynamically
  // initialized process-wide sentinel and its shutdown-order dependencies.
  const UwfStateFacts unknownFacts{};
  const Uwf& downstream;
  const Context& context;
  const UwfStateFacts& facts;
  Effect effect = Effect::None;
  ReadCompletion readCompletion = ReadCompletion::NotDispatched;
  std::vector<Attempt> attempts;
  std::vector<std::string> warnings;
};

void run(Operation& command, const SetBoolean& request) {
  const auto& facts = command.facts;
  switch (request.property) {
    case BooleanProperty::FilterEnabled:
      command.authorize();
      if (facts.next.filterEnabled.value == request.value) return;
      break;
    case BooleanProperty::HormEnabled:
      command.authorize(true);
      if (facts.runtime.horm.value == request.value) return;
      break;
    case BooleanProperty::ServicingEnabled: {
      command.authorize();
      const auto status = command.require(facts.runtime.servicingStatus, "servicing runtime status");
      if (status >= servicingStatus::completed && status != servicingStatus::failed)
        command.fail(ErrorKind::Conflict, "The servicing runtime status cannot start this transition");
      if (!request.value && (status == servicingStatus::started || status == servicingStatus::failed) &&
          !command.require(facts.runtime.savedServicingFilter, "saved servicing filter state"))
        command.fail(ErrorKind::Conflict, "The servicing transition has no saved filter state");
      break;
    }
    case BooleanProperty::OverlayCommit:
      if (facts.current.filterEnabled.value == false) command.fail(ErrorKind::Conflict, "FullCommit requires UWF enabled in the current session");
      if (facts.current.overlayFlags.hasAny(flags::readOnlyMedia) == true) {
        if (!facts.runtime.romDisposition.valid() || facts.runtime.romDisposition.knownMask != std::numeric_limits<std::uint32_t>::max())
          command.fail(ErrorKind::Unavailable, "The shared UWF state has no ROM disposition");
        const auto runtime = facts.runtime.romDisposition.valueBits;
        if ((runtime & disposition::fullCommit) != 0) return;
        if (runtime != 0) command.fail(ErrorKind::Conflict, "FullCommit requires an idle ROM disposition");
      }
      break;
    default:
      command.authorize();
      break;
  }
  (void)command.send(request);
}
void run(Operation& command, const SetNumber& request) {
  command.authorize();
  const auto& facts = command.facts;
  if (request.property == NumberProperty::OverlayType) {
    command.offline();
    if (facts.next.maximumOverlaySize.value && !core::validOverlayTarget(static_cast<core::OverlayType>(request.value), *facts.next.maximumOverlaySize.value))
      command.fail(ErrorKind::InvalidArgument, "The overlay type and maximum size are incompatible");
  } else if (request.property == NumberProperty::MaximumOverlaySize) {
    command.offline();
    if (facts.next.overlayType.value && !core::validOverlayTarget(static_cast<core::OverlayType>(*facts.next.overlayType.value), request.value))
      command.fail(ErrorKind::InvalidArgument, "The overlay type and maximum size are incompatible");
    if (facts.runtime.criticalThreshold.value && request.value < *facts.runtime.criticalThreshold.value)
      command.warnings.emplace_back("The maximum overlay size is below the critical threshold");
    if (facts.runtime.warningThreshold.value && request.value < *facts.runtime.warningThreshold.value)
      command.warnings.emplace_back("The maximum overlay size is below the warning threshold");
  } else if (request.property == NumberProperty::WarningThreshold || request.property == NumberProperty::CriticalThreshold) {
    const bool warning = request.property == NumberProperty::WarningThreshold;
    const auto& other = warning ? facts.runtime.criticalThreshold : facts.runtime.warningThreshold;
    if (facts.next.maximumOverlaySize.value && request.value > *facts.next.maximumOverlaySize.value)
      command.warnings.emplace_back("The overlay threshold exceeds the next-session maximum overlay size");
    if (other.value && (warning ? request.value > *other.value : request.value < *other.value))
      command.warnings.emplace_back("The warning threshold exceeds the critical threshold");
  } else if (request.property == NumberProperty::ResetPersistentState) {
    if (facts.current.filterEnabled.value == false) command.fail(ErrorKind::AccessDenied, "Resetting the persistent overlay requires an active filter");
    if (facts.current.overlayType.value && *facts.current.overlayType.value != static_cast<std::uint32_t>(OverlayType::Disk))
      command.fail(ErrorKind::AccessDenied, "Resetting the persistent overlay requires a loaded disk overlay");
    if (facts.current.overlayFlags.hasAny(flags::persistent) == false)
      command.fail(ErrorKind::AccessDenied, "Resetting the persistent overlay requires loaded Persistent flags");
    if (!command.require(facts.runtime.controlDevicePresent, "control-device availability"))
      command.fail(ErrorKind::Unavailable, "The UWF control device is unavailable");
    // Saved is an argument of the native reset capability. Its entry point
    // resolves that selector; do not compose a separate policy getter here.
  }
  (void)command.send(request);
}
void run(Operation& command, const SetOverlayOption& request) {
  if (request.option != OverlayOption::ReadOnlyMedia) command.authorize();
  command.offline();
  const auto& observed = command.facts.next.overlayFlags;
  if (!observed.valid() || observed.knownMask != std::numeric_limits<std::uint32_t>::max())
    command.fail(ErrorKind::Unavailable, "The shared UWF state has no complete Next overlay flags");
  const auto mask = request.option == OverlayOption::Passthrough  ? flags::passthrough
                    : request.option == OverlayOption::Persistent ? flags::persistent
                                                                  : flags::readOnlyMedia;
  (void)command.send(SetNumber{NumberProperty::OverlayFlags, request.value ? observed.valueBits | mask : observed.valueBits & ~mask});
}
void run(Operation& command, const SetVolume& request) {
  if (request.property == VolumeProperty::Protected) command.authorize();
  const auto present = presence(command.facts.next, request.volume);
  if (!present) command.fail(ErrorKind::Unavailable, "The shared UWF state has no Next volume membership");
  if (present == false) {
    if (request.property != VolumeProperty::Protected || !request.value)
      command.fail(ErrorKind::NotFound, "The UWF volume is not configured for the next session");
    command.create(request.volume, request.driveLetter, true, request.bindByDriveLetterOnCreate);
    return;
  }
  if (request.property == VolumeProperty::Protected) {
    if (const auto* volume = volumeFacts(command.facts.next, request.volume); volume && volume->protectedVolume.value == request.value) return;
  } else {
    command.eligible(request.volume, true);
  }
  (void)command.send(request);
}
void run(Operation& command, const CreateSwapfile& request) {
  const auto present = presence(command.facts.next, request.volume);
  if (!present) command.fail(ErrorKind::Unavailable, "The shared UWF state has no Next volume membership");
  if (!*present) command.create(request.volume, request.driveLetter, false, request.bindByDriveLetterOnCreate);
  command.offline();
  (void)command.send(request);
}
template <class R>
void editExclusion(Operation& command, const R& request) {
  command.authorize();
  constexpr bool file = std::is_same_v<R, EditFileExclusion>;
  const Fact<std::vector<std::string>>* observed;
  if constexpr (file) {
    observed = &exclusionFacts(command.facts, GetFileExclusions{request.volume, Session::Next});
    if (request.action == ExclusionAction::Add && presence(command.facts.next, request.volume) == false)
      command.fail(ErrorKind::AccessDenied, "Adding a file exclusion requires a configured next-session UWF volume");
    if (request.action == ExclusionAction::Remove && presence(command.facts.next, request.volume) == false) return;
  } else {
    observed = &command.facts.next.registryExclusions;
  }
  if (request.action == ExclusionAction::Clear) {
    const auto& entries = command.require(*observed, "Next exclusion list");
    // Validate the complete plan before its first mutation.
    for (const auto& entry : entries) {
      auto remove = request;
      remove.action = ExclusionAction::Remove;
      if constexpr (file)
        remove.path = entry;
      else
        remove.key = entry;
      if (const auto failure = requestInputFailure(Request{remove})) command.fail(failure->kind, failure->message);
    }
    for (const auto& entry : entries) {
      auto remove = request;
      remove.action = ExclusionAction::Remove;
      if constexpr (file)
        remove.path = entry;
      else
        remove.key = entry;
      (void)command.send(remove);
    }
    return;
  }
  if (observed->value) {
    const auto& target = [&]() -> const std::string& {
      if constexpr (file)
        return request.path;
      else
        return request.key;
    }();
    const bool present = std::ranges::any_of(*observed->value, [&](const auto& entry) { return equalAsciiInsensitive(entry, target); });
    if (present == (request.action == ExclusionAction::Add)) return;
  }
  (void)command.send(request);
}
}  // namespace

compat::Capability probeUwfOperation(const compat::Uwf& downstream, const compat::Request& request, const compat::Context& context) {
  using namespace compat;
  if (const auto failure = requestInputFailure(request)) return {Support::Unsupported, failure->message};
  if (const auto* number = std::get_if<SetNumber>(&request); number && number->property == NumberProperty::OverlayFlags)
    return {Support::Unsupported, "Change one overlay option instead of writing raw flags"};
  if (std::holds_alternative<FindFileExclusion>(request) || std::holds_alternative<FindRegistryExclusion>(request)) return {Support::Supported, {}};
  if (const auto* boolean = std::get_if<GetBoolean>(&request); boolean && boolean->property == BooleanProperty::RegistryChangesAlwaysPersisted)
    return {Support::Supported, {}};
  if (std::holds_alternative<SetOverlayOption>(request)) return downstream.probe(SetNumber{NumberProperty::OverlayFlags, 0});
  auto primitive = request;
  if (auto* file = std::get_if<EditFileExclusion>(&primitive); file && file->action == ExclusionAction::Clear) file->action = ExclusionAction::Remove;
  if (auto* registry = std::get_if<EditRegistryExclusion>(&primitive); registry && registry->action == ExclusionAction::Clear)
    registry->action = ExclusionAction::Remove;
  auto support = downstream.probe(primitive);
  std::visit(
      [&](const auto& operation) {
        using R = std::decay_t<decltype(operation)>;
        if constexpr (std::is_same_v<R, CreateSwapfile> || std::is_same_v<R, SetVolume>) {
          bool creates = false;
          if constexpr (std::is_same_v<R, CreateSwapfile>)
            creates = true;
          else
            creates = operation.property == VolumeProperty::Protected && operation.value;
          if (creates && context.facts && presence(context.facts->next, operation.volume) == false) {
            support = combineCapability(std::move(support), downstream.probe(AddVolume{operation.volume, operation.driveLetter}));
            support = combineCapability(std::move(support), downstream.probe(SetVolume{operation.volume, VolumeProperty::Protected, false}));
            support = combineCapability(std::move(support), downstream.probe(SetVolume{operation.volume, VolumeProperty::BindByDriveLetter, false}));
          }
        }
      },
      request);
  return support;
}

compat::Response executeUwfOperation(const compat::Uwf& downstream, const compat::Request& request, const compat::Context& context) {
  using namespace compat;
  if (context.stop.stop_requested()) return Response::fail(ErrorKind::Canceled, "UWF operation canceled before dispatch");
  if (const auto failure = requestInputFailure(request)) return Response::fail(failure->kind, failure->message);
  if (const auto* boolean = std::get_if<SetBoolean>(&request); boolean && boolean->property == BooleanProperty::OverlayCommit && !boolean->value)
    return Response::fail(ErrorKind::InvalidArgument, "OverlayCommit accepts only an enable request");
  if (const auto* number = std::get_if<SetNumber>(&request); number && number->property == NumberProperty::OverlayFlags)
    return Response::fail(ErrorKind::Unsupported, "Change one overlay option instead of writing raw flags");
  Operation command(downstream, context);
  try {
    return std::visit(
        [&](const auto& operation) -> Response {
          using R = std::decay_t<decltype(operation)>;
          const auto& facts = command.facts;
          if constexpr (std::is_same_v<R, GetBoolean>) {
            if (operation.property == BooleanProperty::RegistryChangesAlwaysPersisted) {
              if (!command.require(facts.current.filterEnabled, "Current filter state")) return Response::success(true);
              const auto& id = command.require(facts.systemVolumeId, "system volume identity");
              if (presence(facts.current, id) == false) return Response::success(true);
              const auto* volume = volumeFacts(facts.current, id);
              if (!volume) command.fail(ErrorKind::Unavailable, "The shared UWF state has no system-volume protection");
              return Response::success(!command.require(volume->protectedVolume, "system-volume protection"));
            }
            if (operation.property == BooleanProperty::OverlayCommit && facts.current.filterEnabled.value == false) return Response::success(false);
            return command.send(operation);
          } else if constexpr (std::is_same_v<R, GetOverlayUsage>) {
            if (facts.current.filterEnabled.value == false) return Response::success(OverlayUsage{});
            auto response = command.send(operation);
            const auto& usage = std::get<OverlayUsage>(response.value);
            // Raw getters can retain zero defaults or sample the two scalars
            // across a usage change. Known active protection and nonzero
            // capacity make this pair insufficient to confirm a usage sample.
            if (usage.consumptionMb == 0 && usage.availableMb == 0 && facts.current.filterEnabled.value == true &&
                facts.current.maximumOverlaySize.value && *facts.current.maximumOverlaySize.value > 0 &&
                std::ranges::any_of(facts.current.volumes, [](const auto& entry) {
                  return entry.second.present.value == true && entry.second.protectedVolume.value == true;
                }))
              command.fail(ErrorKind::Unavailable, "The current overlay usage sample could not be confirmed");
            return response;
          } else if constexpr (std::is_same_v<R, FindFileExclusion> || std::is_same_v<R, FindRegistryExclusion>) {
            const Fact<std::vector<std::string>>* list;
            const std::string* target;
            if constexpr (std::is_same_v<R, FindFileExclusion>) {
              if (presence(sessionFacts(facts, operation.session), operation.volume) == false)
                command.fail(ErrorKind::NotFound, "The UWF volume is not registered");
              list = &exclusionFacts(facts, GetFileExclusions{operation.volume, operation.session});
              target = &operation.path;
            } else {
              list = &sessionFacts(facts, operation.session).registryExclusions;
              target = &operation.key;
            }
            const auto& entries = command.require(*list, "exclusion list");
            return Response::success(std::ranges::any_of(entries, [&](const auto& entry) { return equalAsciiInsensitive(entry, *target); }));
          } else if constexpr (std::is_same_v<R, SetBoolean> || std::is_same_v<R, SetNumber> || std::is_same_v<R, SetOverlayOption> ||
                               std::is_same_v<R, SetVolume> || std::is_same_v<R, CreateSwapfile>) {
            run(command, operation);
            return command.success();
          } else if constexpr (std::is_same_v<R, AddVolume>) {
            if (presence(facts.next, operation.volume) == true) command.fail(ErrorKind::Conflict, "The UWF volume is already registered");
            command.eligible(operation.volume, false);
            return command.send(operation);
          } else if constexpr (std::is_same_v<R, RemoveVolume>) {
            if (presence(facts.next, operation.volume) == false) command.fail(ErrorKind::NotFound, "The UWF volume is not registered");
            return command.send(operation);
          } else if constexpr (std::is_same_v<R, EditFileExclusion> || std::is_same_v<R, EditRegistryExclusion>) {
            editExclusion(command, operation);
            return command.success();
          } else if constexpr (std::is_same_v<R, CommitFile> || std::is_same_v<R, CommitFileDeletion>) {
            command.maintenance(operation.volume, std::is_same_v<R, CommitFileDeletion>);
            command.fileIdentity(operation.volume);
            command.fileTarget(request, operation.volume, std::is_same_v<R, CommitFileDeletion>);
            return command.send(operation);
          } else if constexpr (std::is_same_v<R, CommitRegistry> || std::is_same_v<R, CommitRegistryDeletion>) {
            command.maintenance(std::nullopt, std::is_same_v<R, CommitRegistryDeletion>);
            return command.send(operation);
          } else if constexpr (std::is_same_v<R, GetOverlayFiles>) {
            const auto* volume = volumeFacts(facts.current, operation.volume);
            if (facts.current.filterEnabled.value == false || presence(facts.current, operation.volume) == false ||
                (volume && volume->protectedVolume.value == false))
              command.fail(ErrorKind::Conflict, "Overlay enumeration requires an active protected Current volume");
            return command.send(operation);
          } else if constexpr (std::is_same_v<R, ResetSettings>) {
            command.authorize();
            command.offline();
            if (!command.require(facts.capturedSettingsAvailable, "captured UWF defaults"))
              command.fail(ErrorKind::Conflict, "Captured UWF defaults are unavailable");
            return command.send(operation);
          } else if constexpr (std::is_same_v<R, Power>) {
            if (!command.require(facts.runtime.shutdownPrivilegeAvailable, "shutdown privilege"))
              command.fail(ErrorKind::Unsupported, "The caller has no shutdown privilege");
            if (facts.current.filterEnabled.value == true &&
                (!facts.runtime.romDisposition.valid() || facts.runtime.romDisposition.knownMask != std::numeric_limits<std::uint32_t>::max()))
              command.fail(ErrorKind::Unavailable, "The shared UWF state has no ROM disposition");
            return command.send(operation);
          } else {
            return command.send(operation);
          }
        },
        request);
  } catch (const Error& error) {
    auto response = Response::fail(error.failure().kind, error.what(), combineEffects(command.effect, error.failure().effect), error.failure().diagnostics,
                                   error.failure().readCompletion);
    response.attempts = error.attempts();
    response.warnings = std::move(command.warnings);
    return response;
  } catch (const std::exception& error) {
    return Response::fail(ErrorKind::Failed, error.what(), command.effect, {}, command.readCompletion);
  } catch (...) {
    return Response::fail(ErrorKind::Failed, "UWF operation composition failed", command.effect, {}, command.readCompletion);
  }
}
}  // namespace uwf::app
