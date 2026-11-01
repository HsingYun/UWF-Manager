// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "application/console/ConsoleCommandExecutor.h"

#include <algorithm>
#include <array>
#include <charconv>
#include <format>
#include <stdexcept>
#include <string_view>
#include <utility>

#include "application/configuration/commands/UwfmgrCommandParser.h"
#include "foundation/text/StringUtil.h"

namespace uwf::app {
namespace {
using namespace compat;

namespace exitCode {
constexpr int failed = 1;
constexpr int accessDenied = 5;
constexpr int resourceExhausted = 8;
constexpr int invalidData = 13;
constexpr int unsupported = 50;
constexpr int invalidArgument = 87;
constexpr int notFound = 1168;
constexpr int canceled = 1223;
constexpr int unavailable = 1722;
constexpr int rebootRequired = 3010;
constexpr int conflict = 5023;
}  // namespace exitCode

struct FailurePresentation {
  int exitCode;
  std::string_view summary;
};

constexpr FailurePresentation failurePresentation(const ErrorKind kind) {
  switch (kind) {
    case ErrorKind::Unsupported:
      return {exitCode::unsupported, "Unsupported operation"};
    case ErrorKind::Unavailable:
      return {exitCode::unavailable, "UWF operation unavailable"};
    case ErrorKind::AccessDenied:
      return {exitCode::accessDenied, "Access denied"};
    case ErrorKind::NotFound:
      return {exitCode::notFound, "Target not found"};
    case ErrorKind::InvalidArgument:
      return {exitCode::invalidArgument, "Invalid argument"};
    case ErrorKind::Canceled:
      return {exitCode::canceled, "Operation canceled"};
    case ErrorKind::ResourceExhausted:
      return {exitCode::resourceExhausted, "Insufficient resources"};
    case ErrorKind::Conflict:
      return {exitCode::conflict, "Operation conflicts with the current state"};
    case ErrorKind::Protocol:
      return {exitCode::invalidData, "Invalid UWF response"};
    case ErrorKind::Failed:
      return {exitCode::failed, "Operation failed"};
  }
  return {exitCode::failed, "Operation failed"};
}

UwfmgrCommand canonicalCommand(const UwfmgrCommand& command) {
  if (command.parseError != ParseError::None) throw Error({ErrorKind::InvalidArgument, Effect::None, uwfmgrParseErrorText(command), {}});
  std::vector<std::string> arguments;
  if (command.kind == UwfmgrKind::VolumeProtectAll) {
    arguments = {"volume", "protect", "all"};
  } else {
    const auto catalog = uwfmgrCommandCatalog();
    const auto entry = std::ranges::find(catalog, command.kind, &UwfmgrCommandDescriptor::kind);
    if (entry == catalog.end()) throw Error({ErrorKind::InvalidArgument, Effect::None, "Expected an executable UWF command", {}});
    if (!entry->family.empty()) arguments.emplace_back(entry->family);
    arguments.emplace_back(entry->verb);
  }
  arguments.insert(arguments.end(), command.args.begin(), command.args.end());
  auto parsed = parseUwfmgrArguments(arguments);
  if (parsed.parseError != ParseError::None) throw Error({ErrorKind::InvalidArgument, Effect::None, uwfmgrParseErrorText(parsed), {}});
  return parsed;
}

std::string booleanText(const bool value) { return value ? "Enabled" : "Disabled"; }
std::string sessionText(const Session session) { return session == Session::Current ? "Current" : "Next"; }
std::string resetPolicyText(const std::uint32_t value) {
  switch (static_cast<ResetMode>(value)) {
    case ResetMode::Preserve:
      return "Preserve";
    case ResetMode::Clear:
      return "Rollback";
    default:
      return "Unknown (" + std::to_string(value) + ')';
  }
}
enum class VolumeReport { Configuration, FileExclusions };

std::uint32_t numberArgument(const UwfmgrCommand& command) {
  const auto& argument = command.args.at(0);
  std::uint32_t value = 0;
  const auto parsed = std::from_chars(argument.data(), argument.data() + argument.size(), value);
  if (parsed.ec != std::errc{} || parsed.ptr != argument.data() + argument.size())
    throw Error({ErrorKind::InvalidArgument, Effect::None, "Expected an unsigned 32-bit decimal number", {}});
  return value;
}

std::vector<Session> selectedSessions(const std::vector<std::string>& arguments, const std::size_t selectorIndex = 0) {
  if (arguments.size() <= selectorIndex) return {Session::Current, Session::Next};
  const auto selector = toLowerAscii(arguments[selectorIndex]);
  if (selector == "current") return {Session::Current};
  if (selector == "next") return {Session::Next};
  throw Error({ErrorKind::InvalidArgument, Effect::None, "Expected current or next as the session selector", {}});
}

class Execution {
 public:
  Execution(const Uwf& system, const ConsoleEnvironment& environment, const Context& context, const AllVolumeProtection* allVolumes)
      : system(system), environment(environment), context(context), allVolumes(allVolumes) {}

  ConsoleCommandResult result;

  void canceled() const {
    if (context.stop.stop_requested()) throw Error({ErrorKind::Canceled, Effect::None, "The operation was canceled", {}});
  }

  void fail(Failure failure, std::string operation = "Command") {
    result.effect = combineEffects(result.effect, failure.effect);
    result.failures.push_back({std::move(operation), failure});
    if (!result.failure) result.failure = std::move(failure);
    result.failure->effect = result.effect;
    result.exitCode = consoleFailureExitCode(result.failure->kind);
  }

  Response request(const Request& operation) {
    canceled();
    auto response = [&] {
      try {
        return normalizeResponse(operation, system.execute(operation, context));
      } catch (const Error& error) {
        auto failed = Response::fail(error.failure().kind, error.failure().message, error.failure().effect, error.failure().diagnostics);
        failed.attempts = error.attempts();
        return failed;
      } catch (const std::exception& error) {
        return Response::fail(ErrorKind::Failed, error.what(), isWrite(operation) ? Effect::Uncertain : Effect::None);
      } catch (...) {
        return Response::fail(ErrorKind::Failed, "The UWF port raised an unknown error", isWrite(operation) ? Effect::Uncertain : Effect::None);
      }
    }();
    result.effect = combineEffects(result.effect, response.effect);
    result.attempts.insert(result.attempts.end(), response.attempts.begin(), response.attempts.end());
    result.warnings.insert(result.warnings.end(), response.warnings.begin(), response.warnings.end());
    if (!isWrite(operation)) {
      if (response.failure)
        ++result.failedReads;
      else
        ++result.successfulReads;
    }
    if (response.failure) fail(*response.failure, operationName(operation));
    return response;
  }

  template <class R>
  std::optional<typename R::Result> observe(const R& operation, ConsoleSection& section, const std::string& field) {
    auto response = request(operation);
    if (response.failure) {
      section.fields.push_back({field, {}, response.failure});
      return std::nullopt;
    }
    return std::move(std::get<typename R::Result>(response.value));
  }

  template <class R>
  void booleanField(const R& operation, ConsoleSection& section, const std::string& field) {
    if (const auto value = observe(operation, section, field)) section.fields.push_back({field, booleanText(*value)});
  }

  const Response& filterObservation(const Session session) {
    // Cache the complete receipt, including failure, for a coherent report.
    // Counters track actual requests rather than each use of this receipt.
    auto& observation = filterObservations[session == Session::Current ? 0U : 1U];
    if (!observation) observation = request(GetBoolean{BooleanProperty::FilterEnabled, session});
    return *observation;
  }

  std::optional<bool> filterField(const Session session, ConsoleSection& section, const std::string& field) {
    const auto& observation = filterObservation(session);
    if (observation.failure) {
      section.fields.push_back({field, {}, observation.failure});
      return std::nullopt;
    }
    const auto enabled = std::get<bool>(observation.value);
    section.fields.push_back({field, booleanText(enabled)});
    return enabled;
  }

  void sizeField(const GetNumber& operation, ConsoleSection& section, const std::string& field) {
    if (const auto value = observe(operation, section, field)) section.fields.push_back({field, std::to_string(*value) + " MB"});
  }

  void resetPolicyField(const GetNumber& operation, ConsoleSection& section, const std::string& field) {
    if (const auto value = observe(operation, section, field)) section.fields.push_back({field, resetPolicyText(*value)});
  }

  void mutate(const Request& operation, const std::string& message, const bool nextSession = false) {
    const auto response = request(operation);
    if (response.failure) return;
    result.sections.push_back({"Result", {{"Status", message}}, {}});
    if (nextSession) result.sections.back().fields.push_back({"Configuration", "Next session; restart Windows to load the saved configuration"});
  }

  ConsoleVolumeTarget volume(const std::string& operand) {
    canceled();
    try {
      return environment.resolveVolume(operand);
    } catch (const Error& error) {
      if (error.failure().kind == ErrorKind::InvalidArgument && error.failure().diagnostics.empty()) result.publicReason = error.failure().message;
      throw;
    }
  }

  ConsoleFileTarget file(const std::string& operand) {
    canceled();
    try {
      return environment.resolveFile(operand);
    } catch (const Error& error) {
      if (error.failure().kind == ErrorKind::InvalidArgument && error.failure().diagnostics.empty()) result.publicReason = error.failure().message;
      throw;
    }
  }

  static ConsoleSection& appendSection(std::vector<ConsoleSection>& sections, std::string title) {
    sections.push_back({std::move(title), {}, {}});
    return sections.back();
  }

  static std::string volumeTitle(const Volume& volume) { return volume.driveLetter.empty() ? volumePath(volume.id) : volume.driveLetter; }

  void runtimeConfig() {
    auto& section = appendSection(result.sections, "Runtime");
    booleanField(GetBoolean{BooleanProperty::HormEnabled}, section, "HORM");
    booleanField(GetBoolean{BooleanProperty::ShutdownPending}, section, "Shutdown pending");
    booleanField(GetBoolean{BooleanProperty::OverlayCommit}, section, "Full commit");
  }

  void servicingConfig(const Session session, ConsoleSection& section) {
    booleanField(GetBoolean{BooleanProperty::ServicingEnabled, session}, section, "Servicing");
  }

  void overlayConfig(const Session session, ConsoleSection& section, const bool includeFilter) {
    const auto& filter = filterObservation(session);
    const auto enabled = includeFilter    ? filterField(session, section, "UWF filter")
                         : filter.failure ? std::optional<bool>{}
                                          : std::optional{std::get<bool>(filter.value)};
    if (const auto type = observe(GetNumber{NumberProperty::OverlayType, session}, section, "Overlay type")) {
      const auto text = *type == static_cast<std::uint32_t>(OverlayType::Ram)    ? "RAM"
                        : *type == static_cast<std::uint32_t>(OverlayType::Disk) ? "Disk"
                                                                                 : "Unknown (" + std::to_string(*type) + ')';
      section.fields.push_back({"Overlay type", text});
    }
    sizeField(GetNumber{NumberProperty::MaximumOverlaySize, session}, section, "Maximum size");
    const auto overlayFlags = observe(GetNumber{NumberProperty::OverlayFlags, session}, section, "Overlay options");
    if (overlayFlags) {
      section.fields.push_back({"Free-space passthrough", booleanText((*overlayFlags & flags::passthrough) != 0)});
      section.fields.push_back({"Persistent overlay", booleanText((*overlayFlags & flags::persistent) != 0)});
      section.fields.push_back({"Read-only media mode", booleanText((*overlayFlags & flags::readOnlyMedia) != 0)});
      const auto additional = *overlayFlags & ~(flags::passthrough | flags::persistent | flags::readOnlyMedia);
      if (additional != 0) section.fields.push_back({"Additional flags", std::format("0x{:08X}", additional)});
      if (session == Session::Current && (*overlayFlags & flags::persistent) != 0) {
        if (enabled && *enabled)
          resetPolicyField(GetNumber{NumberProperty::ResetPersistentState}, section, "Reset on next boot");
        else if (enabled)
          section.fields.push_back({"Reset on next boot", "Not applicable (UWF is disabled)"});
      }
      if (session == Session::Next && (*overlayFlags & flags::persistent) != 0) {
        resetPolicyField(GetNumber{NumberProperty::ResetPersistentStateSavedMode, session}, section, "Default reset policy");
      }
    }
    if (session == Session::Current) {
      sizeField(GetNumber{NumberProperty::WarningThreshold}, section, "Warning threshold");
      sizeField(GetNumber{NumberProperty::CriticalThreshold}, section, "Critical threshold");
    }
  }

  void registryExclusions(const Session session, ConsoleSection& section) {
    if (auto values = observe(GetRegistryExclusions{session}, section, "Exclusions")) {
      section.fields.push_back({"Count", std::to_string(values->size())});
      section.items = std::move(*values);
    }
  }

  void fileExclusions(const std::string& id, const Session session, ConsoleSection& section, const bool includeVolume) {
    if (includeVolume) section.fields.push_back({"Volume GUID", volumePath(id)});
    if (auto values = observe(GetFileExclusions{id, session}, section, "Exclusions")) {
      section.fields.push_back({"Count", std::to_string(values->size())});
      section.items = std::move(*values);
    }
  }

  void volumeConfig(const Volume& entry, const Session session, ConsoleSection& section) {
    section.fields = {{"Volume GUID", volumePath(entry.id)},
                      {"Drive letter", entry.driveLetter.empty() ? "(none reported)" : entry.driveLetter},
                      {"Protection", booleanText(entry.protectedVolume)},
                      {"Binding", entry.bindByDriveLetter ? "Drive letter" : "Volume identity"}};
    if (entry.commitPending) section.fields.push_back({"Commit pending", booleanText(*entry.commitPending)});
    if (const auto marker = observe(GetSwapfileSize{entry.id, session}, section, "Swapfile host")) {
      const auto text = *marker == 0 ? "No" : *marker == 1 ? "Yes" : "Unknown (" + std::to_string(*marker) + ')';
      section.fields.push_back({"Swapfile host", text});
    }
    auto& exclusions = appendSection(section.children, "File exclusions");
    fileExclusions(entry.id, session, exclusions, false);
  }

  void volumeReports(const Session session, ConsoleSection& section, const VolumeReport report) {
    const auto inventory = observe(GetVolumes{session}, section, "Volumes");
    if (!inventory) return;
    section.fields.push_back({"Count", std::to_string(inventory->size())});
    for (const auto& entry : *inventory) {
      auto& child = appendSection(section.children, volumeTitle(entry));
      if (report == VolumeReport::Configuration)
        volumeConfig(entry, session, child);
      else
        fileExclusions(entry.id, session, child, true);
    }
  }

  void targetVolumeReports(const std::string& id, const std::vector<Session>& sessions, const VolumeReport report) {
    const auto identity = normalizeVolumeId(id);
    bool found = false;
    bool allObserved = true;
    for (const auto session : sessions) {
      auto& group = appendSection(result.sections, sessionText(session) + " session");
      auto& section = appendSection(group.children, volumePath(identity));
      section.fields.push_back({"Volume GUID", volumePath(identity)});
      // The session's authoritative inventory owns metadata and absence.
      const auto inventory = observe(GetVolumes{session}, section, "Configuration");
      if (!inventory) {
        allObserved = false;
        continue;
      }
      const auto entry = std::ranges::find_if(*inventory, [&](const Volume& candidate) { return normalizeVolumeId(candidate.id) == identity; });
      if (entry == inventory->end()) {
        section.fields.push_back({"Status", "Not configured"});
        continue;
      }
      found = true;
      section.title = volumeTitle(*entry);
      section.fields.clear();
      if (report == VolumeReport::Configuration)
        volumeConfig(*entry, session, section);
      else
        fileExclusions(entry->id, session, section, true);
    }
    if (allObserved && !found)
      fail({ErrorKind::NotFound, Effect::None, "No UWF configuration exists for this volume in the requested sessions", {}}, "FindVolumeConfiguration");
  }

  void allVolumeReports(const Session session, const VolumeReport report) {
    auto& group = appendSection(result.sections, sessionText(session) + " session");
    auto& volumes = appendSection(group.children, "Volumes");
    volumeReports(session, volumes, report);
  }

  void configurationSession(const Session session) {
    auto& group = appendSection(result.sections, sessionText(session) + " session");
    filterField(session, group, "UWF filter");
    servicingConfig(session, group);
    overlayConfig(session, appendSection(group.children, "Overlay"), false);
    volumeReports(session, appendSection(group.children, "Volumes"), VolumeReport::Configuration);
    registryExclusions(session, appendSection(group.children, "Registry exclusions"));
  }

  void protectAll() {
    canceled();
    if (!allVolumes) throw Error({ErrorKind::Unavailable, Effect::None, "The all-volume protection service is unavailable", {}});
    compat::AllVolumeProtectionResult receipt;
    try {
      receipt = allVolumes->execute(ProtectAll{}, context);
    } catch (const Error& error) {
      fail(error.failure(), "ProtectAllVolumes");
      result.attempts.insert(result.attempts.end(), error.attempts().begin(), error.attempts().end());
      return;
    } catch (const std::exception& error) {
      fail({ErrorKind::Failed, Effect::Uncertain, error.what(), {}}, "ProtectAllVolumes");
      return;
    } catch (...) {
      fail({ErrorKind::Failed, Effect::Uncertain, "The all-volume protection port raised an unknown error", {}}, "ProtectAllVolumes");
      return;
    }
    result.effect = combineEffects(result.effect, receipt.effect);
    ConsoleSection section{"Volume protection - Next", {{"Targets", std::to_string(receipt.items.size())}}, {}};
    for (const auto& item : receipt.items) {
      result.attempts.insert(result.attempts.end(), item.attempts.begin(), item.attempts.end());
      result.effect = combineEffects(result.effect, item.effect);
      section.items.push_back(volumePath(item.volume) + ": " +
                              (item.failure               ? "Error: " + std::string(consoleFailureSummary(item.failure->kind))
                               : item.completionConfirmed ? "Protected"
                                                          : "Unconfirmed"));
      if (item.failure) fail(*item.failure, "ProtectAllVolumes " + volumePath(item.volume));
    }
    if (receipt.failure) fail(*receipt.failure, "ProtectAllVolumes");
    if (!receipt.succeeded() && !result.failure)
      fail({ErrorKind::Failed, receipt.effect, "All-volume protection could not be confirmed", {}}, "ProtectAllVolumes");
    if (receipt.succeeded()) section.fields.push_back({"Configuration", "Next session; restart Windows to load the saved configuration"});
    result.sections.push_back(std::move(section));
  }

  void usage(const bool available) {
    ConsoleSection section{"Overlay - Current", {}, {}};
    const auto enabled = filterField(Session::Current, section, "UWF filter");
    const std::string field = available ? "Available space (MB)" : "Consumption (MB)";
    if (enabled && !*enabled)
      section.fields.push_back({field, "Not applicable (UWF is disabled)"});
    else if (enabled)
      if (const auto value = observe(GetOverlayUsage{}, section, "Overlay usage"))
        section.fields.push_back({field, std::to_string(available ? value->availableMb : value->consumptionMb)});
    result.sections.push_back(std::move(section));
  }

  void update() {
    const auto response = request(UpdateWindows{});
    if (response.failure) return;
    const auto outcome = std::get<WindowsUpdateOutcome>(response.value);
    switch (outcome) {
      case WindowsUpdateOutcome::Completed:
        result.sections.push_back({"Windows update", {{"Status", "Completed"}}, {}});
        return;
      case WindowsUpdateOutcome::RebootRequired:
        result.sections.push_back({"Windows update", {{"Status", "Completed; restart required"}}, {}});
        result.exitCode = exitCode::rebootRequired;
        return;
      case WindowsUpdateOutcome::Failed:
        fail({ErrorKind::Failed, response.effect, "The Windows update workflow reported failure", {}}, "UpdateWindows");
        return;
    }
  }

  void run(const UwfmgrCommand& command) {
    canceled();
    switch (command.kind) {
      case UwfmgrKind::FilterEnable:
      case UwfmgrKind::FilterDisable:
        mutate(SetBoolean{BooleanProperty::FilterEnabled, command.kind == UwfmgrKind::FilterEnable}, "Filter configuration saved", true);
        return;
      case UwfmgrKind::FilterEnableHorm:
      case UwfmgrKind::FilterDisableHorm:
        mutate(SetBoolean{BooleanProperty::HormEnabled, command.kind == UwfmgrKind::FilterEnableHorm}, "HORM operation completed");
        return;
      case UwfmgrKind::FilterResetSettings:
        mutate(ResetSettings{}, "UWF settings restored", true);
        return;
      case UwfmgrKind::OverlaySetSize:
        mutate(SetNumber{NumberProperty::MaximumOverlaySize, numberArgument(command)}, "Overlay maximum size saved", true);
        return;
      case UwfmgrKind::OverlaySetWarningThreshold:
      case UwfmgrKind::OverlaySetCriticalThreshold:
        mutate(SetNumber{command.kind == UwfmgrKind::OverlaySetWarningThreshold ? NumberProperty::WarningThreshold : NumberProperty::CriticalThreshold,
                         numberArgument(command)},
               "Overlay threshold updated");
        return;
      case UwfmgrKind::OverlaySetType:
        mutate(SetNumber{NumberProperty::OverlayType,
                         static_cast<std::uint32_t>(toLowerAscii(command.args.at(0)) == "ram" ? OverlayType::Ram : OverlayType::Disk)},
               "Overlay type saved", true);
        return;
      case UwfmgrKind::OverlaySetPassthrough:
      case UwfmgrKind::OverlaySetPersistent:
      case UwfmgrKind::OverlaySetReadOnlyMedia: {
        const auto option = command.kind == UwfmgrKind::OverlaySetPassthrough  ? OverlayOption::Passthrough
                            : command.kind == UwfmgrKind::OverlaySetPersistent ? OverlayOption::Persistent
                                                                               : OverlayOption::ReadOnlyMedia;
        mutate(SetOverlayOption{option, command.args.at(0) == "on"}, "Overlay option saved", true);
        return;
      }
      case UwfmgrKind::OverlayResetPersistentState: {
        const auto& argument = command.args.at(0);
        const auto mode = argument == "on" ? ResetMode::Clear : ResetMode::Preserve;
        mutate(SetNumber{NumberProperty::ResetPersistentState, static_cast<std::uint32_t>(mode)}, "Persistent-state reset selection updated");
        return;
      }
      case UwfmgrKind::OverlayCommit:
        mutate(SetBoolean{BooleanProperty::OverlayCommit, true}, "Full commit request acknowledged");
        return;
      case UwfmgrKind::VolumeProtect:
      case UwfmgrKind::VolumeUnprotect: {
        const auto target = volume(command.args.at(0));
        mutate(SetVolume{target.id, VolumeProperty::Protected, command.kind == UwfmgrKind::VolumeProtect, target.driveLetter, target.bindByDriveLetter},
               "Volume protection configuration saved", true);
        return;
      }
      case UwfmgrKind::VolumeProtectAll:
        protectAll();
        return;
      case UwfmgrKind::VolumeCreateSwapfile: {
        const auto target = volume(command.args.at(0));
        mutate(CreateSwapfile{target.id, target.driveLetter, target.bindByDriveLetter}, "Swapfile host configuration saved", true);
        return;
      }
      case UwfmgrKind::FileAddExclusion:
      case UwfmgrKind::FileRemoveExclusion: {
        const auto target = file(command.args.at(0));
        mutate(EditFileExclusion{target.volume, target.path, command.kind == UwfmgrKind::FileAddExclusion ? ExclusionAction::Add : ExclusionAction::Remove},
               "File exclusion configuration saved", true);
        return;
      }
      case UwfmgrKind::FileCommit:
      case UwfmgrKind::FileCommitDelete: {
        const auto target = file(command.args.at(0));
        if (command.kind == UwfmgrKind::FileCommit)
          mutate(CommitFile{target.volume, target.path}, "File commit completed");
        else
          mutate(CommitFileDeletion{target.volume, target.path}, "File deletion commit completed");
        return;
      }
      case UwfmgrKind::RegistryAddExclusion:
      case UwfmgrKind::RegistryRemoveExclusion:
        mutate(EditRegistryExclusion{command.args.at(0), command.kind == UwfmgrKind::RegistryAddExclusion ? ExclusionAction::Add : ExclusionAction::Remove},
               "Registry exclusion configuration saved", true);
        return;
      case UwfmgrKind::RegistryCommit:
        mutate(CommitRegistry{command.args.at(0), command.args.at(1)}, "Registry value commit completed");
        return;
      case UwfmgrKind::RegistryCommitDelete:
        mutate(CommitRegistryDeletion{command.args.at(0), command.args.size() > 1 ? std::optional{command.args[1]} : std::nullopt},
               "Registry deletion commit completed");
        return;
      case UwfmgrKind::ServicingEnable:
      case UwfmgrKind::ServicingDisable:
        mutate(SetBoolean{BooleanProperty::ServicingEnabled, command.kind == UwfmgrKind::ServicingEnable}, "Servicing transition completed", true);
        return;
      case UwfmgrKind::ServicingUpdateWindows:
        update();
        return;
      case UwfmgrKind::GetConfig:
        result.title = "UWF configuration";
        runtimeConfig();
        for (const auto session : selectedSessions(command.args)) {
          configurationSession(session);
          if (session == Session::Next) result.notes.emplace_back("Changes to next-session configuration take effect after restarting Windows.");
        }
        return;
      case UwfmgrKind::OverlayGetConfig:
        result.title = "Overlay configuration";
        for (const auto session : selectedSessions(command.args)) {
          overlayConfig(session, appendSection(result.sections, sessionText(session) + " session"), true);
          if (session == Session::Next) result.notes.emplace_back("Changes to next-session configuration take effect after restarting Windows.");
        }
        return;
      case UwfmgrKind::ServicingGetConfig:
        result.title = "Servicing configuration";
        for (const auto session : selectedSessions(command.args)) servicingConfig(session, appendSection(result.sections, sessionText(session) + " session"));
        return;
      case UwfmgrKind::VolumeGetConfig:
        result.title = "Volume configuration";
        if (command.args.empty() || toLowerAscii(command.args[0]) == "all") {
          for (const auto session : {Session::Current, Session::Next}) allVolumeReports(session, VolumeReport::Configuration);
        } else {
          const auto target = volume(command.args[0]);
          targetVolumeReports(target.id, {Session::Current, Session::Next}, VolumeReport::Configuration);
        }
        return;
      case UwfmgrKind::FileGetExclusions:
        result.title = "File exclusions";
        if (command.args.empty() || toLowerAscii(command.args[0]) == "all") {
          for (const auto session : selectedSessions(command.args, 1)) allVolumeReports(session, VolumeReport::FileExclusions);
        } else {
          const auto target = file(command.args[0]);
          targetVolumeReports(target.volume, selectedSessions(command.args, 1), VolumeReport::FileExclusions);
        }
        return;
      case UwfmgrKind::RegistryGetExclusions:
        result.title = "Registry exclusions";
        for (const auto session : selectedSessions(command.args))
          registryExclusions(session, appendSection(result.sections, sessionText(session) + " session"));
        return;
      case UwfmgrKind::OverlayGetAvailableSpace:
        usage(true);
        return;
      case UwfmgrKind::OverlayGetConsumption:
        usage(false);
        return;
      case UwfmgrKind::Help:
      case UwfmgrKind::Unknown:
        throw Error({ErrorKind::InvalidArgument, Effect::None, "Expected an executable UWF command", {}});
    }
  }

 private:
  const Uwf& system;
  const ConsoleEnvironment& environment;
  const Context& context;
  const AllVolumeProtection* allVolumes;
  std::array<std::optional<Response>, 2> filterObservations;
};
}  // namespace

int consoleFailureExitCode(const compat::ErrorKind kind) { return failurePresentation(kind).exitCode; }
std::string_view consoleFailureSummary(const compat::ErrorKind kind) { return failurePresentation(kind).summary; }

ConsoleCommandResult executeConsoleCommand(const UwfmgrCommand& command, const compat::Uwf& system, const ConsoleEnvironment& environment,
                                           const compat::Context& context, const compat::AllVolumeProtection* allVolumes) {
  Execution execution(system, environment, context, allVolumes);
  bool commandValidated = false;
  try {
    const auto canonical = canonicalCommand(command);
    commandValidated = true;
    execution.run(canonical);
  } catch (const compat::Error& error) {
    execution.result.interrupted = commandValidated;
    if (!commandValidated && error.failure().kind == ErrorKind::InvalidArgument) execution.result.publicReason = error.failure().message;
    execution.fail(error.failure(), commandValidated ? "Command" : "ValidateCommand");
    execution.result.attempts.insert(execution.result.attempts.end(), error.attempts().begin(), error.attempts().end());
  } catch (const std::out_of_range&) {
    execution.result.interrupted = commandValidated;
    execution.fail({ErrorKind::InvalidArgument, Effect::None, "A required command argument is missing", {}});
  } catch (const std::exception& error) {
    execution.result.interrupted = commandValidated;
    execution.fail({ErrorKind::Failed, Effect::None, error.what(), {}});
  } catch (...) {
    execution.result.interrupted = commandValidated;
    execution.fail({ErrorKind::Failed, Effect::None, "The command could not be completed because of an unknown error", {}});
  }
  return std::move(execution.result);
}
}  // namespace uwf::app
