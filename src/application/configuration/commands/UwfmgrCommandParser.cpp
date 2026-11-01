// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "application/configuration/commands/UwfmgrCommandParser.h"

#include <algorithm>
#include <array>
#include <charconv>
#include <cstdint>
#include <optional>
#include <system_error>

#include "domain/uwf/contracts/Types.h"
#include "foundation/paths/DriveLetter.h"
#include "foundation/text/StringUtil.h"
#include "foundation/text/Utf8.h"
#include "uwf_version.h"

namespace uwf::app {
namespace {
using enum UwfmgrKind;
constexpr std::array catalog{
    UwfmgrCommandDescriptor{GetConfig, "", "get-config", "[current|next]", "Display all configuration information for the current and next sessions."},
    UwfmgrCommandDescriptor{FilterEnable, "filter", "enable", "", "Enable UWF for the next session, after restarting Windows."},
    UwfmgrCommandDescriptor{FilterDisable, "filter", "disable", "", "Disable UWF for the next session, after restarting Windows."},
    UwfmgrCommandDescriptor{FilterEnableHorm, "filter", "enable-horm", "", "Enable hibernate once, resume many (HORM) for the current session."},
    UwfmgrCommandDescriptor{FilterDisableHorm, "filter", "disable-horm", "", "Disable hibernate once, resume many (HORM) for the current session."},
    UwfmgrCommandDescriptor{FilterResetSettings, "filter", "reset-settings", "", "Reset UWF settings."},
    UwfmgrCommandDescriptor{VolumeProtect, "volume", "protect", "<volume|all>",
                            "Protect a volume by adding it to the protected-volume list for the next session, after restarting Windows."},
    UwfmgrCommandDescriptor{VolumeUnprotect, "volume", "unprotect", "<volume>", "Stop protecting a volume for the next session, after restarting Windows."},
    UwfmgrCommandDescriptor{VolumeCreateSwapfile, "volume", "create-swapfile", "<volume>",
                            "Create a swap file on the volume and set the overlay type to Disk."},
    UwfmgrCommandDescriptor{VolumeGetConfig, "volume", "get-config", "[volume|all]",
                            "Display volume configuration information for the current and next sessions."},
    UwfmgrCommandDescriptor{FileAddExclusion, "file", "add-exclusion", "<path>",
                            "Add a file or directory to the exclusion list of a protected volume for the next session, after restarting Windows."},
    UwfmgrCommandDescriptor{FileRemoveExclusion, "file", "remove-exclusion", "<path>",
                            "Remove a file or directory exclusion for the next session, after restarting Windows."},
    UwfmgrCommandDescriptor{FileGetExclusions, "file", "get-exclusions", "[volume-or-path|all] [current|next]",
                            "Display file exclusion configuration information for the current and next sessions."},
    UwfmgrCommandDescriptor{FileCommit, "file", "commit", "<path>", "Commit all changes to the specified file. Committing folders is not allowed."},
    UwfmgrCommandDescriptor{FileCommitDelete, "file", "commit-delete", "<path>",
                            "Commit deletion of the specified file. A directory is eligible only if it has remained empty since the last restart."},
    UwfmgrCommandDescriptor{OverlaySetSize, "overlay", "set-size", "<MB>",
                            "Set the protected system's overlay size in MB for the next session, after restarting Windows. "
                            "The driver preallocates the configured memory size at system startup."},
    UwfmgrCommandDescriptor{OverlaySetType, "overlay", "set-type", "<RAM|Disk>", "Set the overlay type for Next."},
    UwfmgrCommandDescriptor{OverlaySetPassthrough, "overlay", "set-passthrough", "<on|off>", "Turn free-space passthrough on or off."},
    UwfmgrCommandDescriptor{OverlaySetPersistent, "overlay", "set-persistent", "<on|off>", "Turn persistent overlay on or off."},
    UwfmgrCommandDescriptor{OverlaySetReadOnlyMedia, "overlay", "set-rom-mode", "<on|off>", "Turn read-only media mode on or off."},
    UwfmgrCommandDescriptor{OverlayResetPersistentState, "overlay", "reset-persistentstate", "<none|rollback>",
                            "Choose how to handle the persistent overlay at the next startup: none preserves it; rollback clears it."},
    UwfmgrCommandDescriptor{OverlaySetWarningThreshold, "overlay", "set-warningthreshold", "<MB>",
                            "Set the overlay size in MB at which the driver issues a warning notification for the current session."},
    UwfmgrCommandDescriptor{OverlaySetCriticalThreshold, "overlay", "set-criticalthreshold", "<MB>",
                            "Set the overlay size in MB at which the driver issues a critical notification for the current session."},
    UwfmgrCommandDescriptor{OverlayCommit, "overlay", "commit", "",
                            "Commit all accumulated and future writes to all protected volumes until the next restart. "
                            "Allowed only in read-only media mode."},
    UwfmgrCommandDescriptor{OverlayGetConfig, "overlay", "get-config", "[current|next]",
                            "Display overlay configuration information for the current and next sessions."},
    UwfmgrCommandDescriptor{OverlayGetAvailableSpace, "overlay", "get-availablespace", "",
                            "Display the remaining available overlay space for the current session in MB."},
    UwfmgrCommandDescriptor{OverlayGetConsumption, "overlay", "get-consumption", "", "Display the current overlay consumption in MB."},
    UwfmgrCommandDescriptor{RegistryAddExclusion, "registry", "add-exclusion", "<key>",
                            "Add a registry key to the protected system's exclusion list for the next session, after restarting Windows. "
                            "The excluded key should already exist before the system volume is protected."},
    UwfmgrCommandDescriptor{RegistryRemoveExclusion, "registry", "remove-exclusion", "<key>",
                            "Remove a registry key exclusion for the next session, after restarting Windows."},
    UwfmgrCommandDescriptor{RegistryGetExclusions, "registry", "get-exclusions", "[current|next]",
                            "Display excluded registry keys for the current and next sessions."},
    UwfmgrCommandDescriptor{RegistryCommit, "registry", "commit", "<key> <value>", "Commit changes to the specified registry value."},
    UwfmgrCommandDescriptor{RegistryCommitDelete, "registry", "commit-delete", "<key> [value]",
                            "Commit a whole-key deletion when value is omitted, or one named-value deletion when nonempty."},
    UwfmgrCommandDescriptor{ServicingEnable, "servicing", "enable", "", "Enable UWF servicing mode for the next session, after restarting Windows."},
    UwfmgrCommandDescriptor{ServicingDisable, "servicing", "disable", "", "Disable UWF servicing mode for the next session, after restarting Windows."},
    UwfmgrCommandDescriptor{ServicingUpdateWindows, "servicing", "update-windows", "", "Update the Windows system."},
    UwfmgrCommandDescriptor{ServicingGetConfig, "servicing", "get-config", "[current|next]", "Display servicing status for the current and next sessions."},
};
constexpr std::array<std::string_view, 6> families{"filter", "overlay", "volume", "file", "registry", "servicing"};

struct FamilyHelp {
  std::string_view name;
  std::string_view usage;
  std::string_view description;
  std::string_view parameters;
  std::string_view examples;
  std::string_view notes;
  bool listsHelp;
};

// Structure, command order and examples follow the captured Usage resources.
// Product-specific argument constraints are kept in the relevant family's notes.
constexpr std::array familyHelp{
    FamilyHelp{"filter", "filter [command]", "Configure filter-specific settings and global settings.", "",
               R"(    UWF.exe filter enable
    UWF.exe filter enable-HORM
)",
               "", true},
    FamilyHelp{"overlay", "overlay [command] [argument]", "Configure overlay settings.", "",
               R"(    UWF.exe overlay set-size 1024
    UWF.exe overlay set-type Disk
    UWF.exe overlay set-warningthreshold 256
    UWF.exe overlay get-config
    UWF.exe overlay set-passthrough on
)",
               R"(    get-config accepts current|next; omitting it displays both sessions.
    set-type, set-passthrough, set-persistent and set-rom-mode target Next.
    reset-persistentstate also accepts off=none and on=rollback.
)",
               false},
    FamilyHelp{"volume", "volume [command] [argument]", "Configure volume-specific Unified Write Filter (UWF) settings.",
               R"(    volume    A qualified volume name, such as:
                  A drive letter: C:
                  A device name: \\?\GLOBALROOT\Device\HarddiskVolume1
                  A GUID: \\?\Volume{4c1b02c1-d990-11dc-99ae-806e6f6e6963}
    all       All volumes in the protection list when displaying configuration.
)",
               R"(    UWF.exe volume protect c:
    UWF.exe volume unprotect \\?\Volume{4c1b02c1-d990-11dc-99ae-806e6f6e6963}
    UWF.exe volume create-swapfile \\?\Volume{4c1b02c1-d990-11dc-99ae-806e6f6e6963}
    UWF.exe volume get-config all
)",
               R"(    get-config without a target is equivalent to all; it displays both sessions.
    protect all protects all eligible local volumes.
    unprotect requires one volume.
)",
               false},
    FamilyHelp{"file", "file [command] [volumename][path][filename]", "Configure file and directory exclusion settings.",
               R"(    volumename    A qualified volume name: drive letter, device name
                  or volume GUID.
    path          The directory path of the file or directory to exclude.
    filename      The name of the file to exclude.
    all           All volumes in the protection list; only for get-exclusions.
)",
               R"(    UWF.exe file add-exclusion c:\dir1\dir2.txt
    UWF.exe file remove-exclusion \\?\Volume{4c1b02c1-d990-11dc-99ae-806e6f6e6963}\dir1\dir2.txt
    UWF.exe file get-exclusions all
    UWF.exe file commit c:\dir1\dir2.txt
    UWF.exe file commit-delete c:\dir1\dir2.txt
)",
               R"(    get-exclusions accepts current|next after the target.
    Omit the session to display both sessions.
    File paths must be local absolute paths with an explicit volume root.
    get-exclusions also accepts a volume root without a file path.
)",
               false},
    FamilyHelp{"registry", "registry [command] [key] [value]", "Configure registry exclusions or commit registry changes.",
               R"(    key      The fully qualified registry key name to exclude or commit.
    value    The value name to commit; used only by commit and commit-delete.
)",
               R"(    UWF.exe registry add-exclusion HKLM\Software\Microsoft\Windows\run
    UWF.exe registry remove-exclusion HKLM\Software\Microsoft\Windows\run
    UWF.exe registry get-exclusions
    UWF.exe registry commit HKLM\Software\Test TestValue
    UWF.exe registry commit-delete HKLM\Software\Test TestValue
)",
               R"(    get-exclusions accepts current|next; omitting it displays both sessions.
    registry commit <key> "": commit the default value.
    registry commit-delete <key>: commit deletion of the whole key.
    registry commit-delete <key> <nonempty-value>: delete one named value.
    registry commit-delete does not support deleting the default value;
    An explicit empty value name ("") is rejected;
    it is never treated as whole-key deletion.
)",
               false},
    FamilyHelp{"servicing", "servicing [command] [arguments]", "Configure servicing mode settings.", "",
               R"(    UWF.exe servicing enable
    UWF.exe servicing disable
    UWF.exe servicing get-config
    UWF.exe servicing help
    UWF.exe servicing ?
)",
               R"(    get-config accepts current|next; omitting it displays both sessions.
    update-windows returns 3010 when a restart is required.
    Commands do not automatically restart Windows.
)",
               true},
};

void appendHelpDescription(std::string& output, std::string_view text, const std::string_view indent) {
  constexpr std::size_t helpDescriptionWidth = 80;
  const auto available = helpDescriptionWidth - indent.size();
  while (!text.empty()) {
    auto count = std::min(text.size(), available);
    if (count < text.size()) {
      if (const auto space = text.rfind(' ', count); space != std::string_view::npos) count = space;
    }
    output += indent;
    output += text.substr(0, count);
    output += '\n';
    text.remove_prefix(count);
    while (text.starts_with(' ')) text.remove_prefix(1);
  }
}

bool isHelp(const std::string_view value) { return value == "help" || value == "?"; }

std::optional<std::string> volumeOperand(const std::string& input) {
  if (input.empty()) return std::nullopt;
  std::string_view body = input;
  if (body.starts_with("\\\\?\\") || body.starts_with("\\\\.\\")) body.remove_prefix(4);
  if ((body.size() == 1 || (body.size() == 2 && body[1] == ':') || (body.size() == 3 && body[1] == ':' && body[2] == '\\')) &&
      ((body[0] >= 'a' && body[0] <= 'z') || (body[0] >= 'A' && body[0] <= 'Z'))) {
    return drive::normalize(std::string(body));
  }
  const auto lower = toLowerAscii(input);
  if (lower.starts_with("\\\\?\\volume{")) {
    auto path = input;
    if (path.back() != '\\') path += '\\';
    if (const auto id = compat::parseVolumeId(path)) return compat::volumePath(*id);
  }
  constexpr std::string_view devicePrefix = "\\\\?\\globalroot\\device\\harddiskvolume";
  if (lower.starts_with(devicePrefix)) {
    auto number = std::string_view(lower).substr(devicePrefix.size());
    if (!number.empty() && number.back() == '\\') number.remove_suffix(1);
    if (!number.empty() && std::ranges::all_of(number, [](const char value) { return value >= '0' && value <= '9'; })) return input;
  }
  return std::nullopt;
}
}  // namespace

std::span<const UwfmgrCommandDescriptor> uwfmgrCommandCatalog() { return catalog; }

bool isUwfmgrConfigurationCommand(const UwfmgrKind kind) {
  switch (kind) {
    case FilterEnable:
    case FilterDisable:
    case FilterEnableHorm:
    case FilterDisableHorm:
    case OverlaySetType:
    case OverlaySetSize:
    case OverlaySetWarningThreshold:
    case OverlaySetCriticalThreshold:
    case OverlaySetPassthrough:
    case OverlaySetPersistent:
    case OverlaySetReadOnlyMedia:
    case OverlayResetPersistentState:
    case OverlayCommit:
    case VolumeProtect:
    case VolumeUnprotect:
    case VolumeProtectAll:
    case VolumeCreateSwapfile:
    case FileAddExclusion:
    case FileRemoveExclusion:
    case RegistryAddExclusion:
    case RegistryRemoveExclusion:
      return true;
    default:
      return false;
  }
}

UwfmgrCommand parseUwfmgrArguments(const std::vector<std::string>& arguments) {
  UwfmgrCommand result;
  const auto fail = [&](const ParseError error, const std::string& context = std::string{}) {
    result.parseError = error;
    result.parseErrorContext = context;
  };
  for (const auto& argument : arguments) {
    if (argument.find('\0') != std::string::npos || !utf8ToUtf16(argument)) {
      fail(ParseError::UnexpectedArgument, "Argument is not valid UTF-8 text");
      return result;
    }
  }
  if (arguments.empty()) {
    result.kind = Help;
    return result;
  }
  const auto first = toLowerAscii(arguments[0]);
  if (isHelp(first)) {
    result.kind = Help;
    if (arguments.size() > 1) fail(ParseError::UnexpectedArgument, arguments[1]);
    return result;
  }
  std::string family;
  std::string verb = first;
  std::size_t operandOffset = 1;
  if (std::ranges::find(families, first) != families.end()) {
    family = first;
    if (arguments.size() < 2) {
      result.kind = Help;
      result.args = {family};
      return result;
    }
    verb = toLowerAscii(arguments[1]);
    operandOffset = 2;
    if (isHelp(verb)) {
      result.kind = Help;
      result.args = {family};
      if (arguments.size() > operandOffset) fail(ParseError::UnexpectedArgument, arguments[operandOffset]);
      return result;
    }
  }
  const auto found = std::ranges::find_if(catalog, [&](const auto& descriptor) { return descriptor.family == family && descriptor.verb == verb; });
  if (found == catalog.end()) {
    fail(ParseError::Unsupported, family.empty() ? first : family + " " + verb);
    return result;
  }
  result.kind = found->kind;
  const auto count = arguments.size() - operandOffset;
  const auto at = [&](const std::size_t offset) -> const std::string& { return arguments[operandOffset + offset]; };
  const auto arity = [&](const std::size_t minimum, const std::size_t maximum, const ParseError missing) {
    if (count < minimum) {
      fail(missing);
      return false;
    }
    if (count > maximum) {
      fail(ParseError::UnexpectedArgument, at(maximum));
      return false;
    }
    return true;
  };
  const auto session = [&](const std::size_t offset) {
    const auto value = toLowerAscii(at(offset));
    if (value != "current" && value != "next") {
      fail(ParseError::UnexpectedArgument, at(offset));
      return false;
    }
    result.args.push_back(value);
    return true;
  };
  const auto requiredText = [&](const ParseError missing) {
    if (!arity(1, 1, missing)) return;
    if (at(0).empty())
      fail(missing);
    else
      result.args.push_back(at(0));
  };
  switch (result.kind) {
    case OverlaySetSize:
    case OverlaySetWarningThreshold:
    case OverlaySetCriticalThreshold: {
      if (!arity(1, 1, ParseError::MissingSizeArg)) break;
      std::uint32_t value = 0;
      const auto& text = at(0);
      const auto [end, error] = std::from_chars(text.data(), text.data() + text.size(), value);
      if (text.empty() || error != std::errc{} || end != text.data() + text.size() || (result.kind == OverlaySetSize && value == 0))
        fail(ParseError::InvalidSize, text);
      else
        result.args.push_back(std::to_string(value));
      break;
    }
    case OverlaySetType:
      if (arity(1, 1, ParseError::MissingTypeArg)) {
        const auto value = toLowerAscii(at(0));
        if (value == "ram")
          result.args.emplace_back("RAM");
        else if (value == "disk")
          result.args.emplace_back("Disk");
        else
          fail(ParseError::UnknownType, at(0));
      }
      break;
    case OverlaySetPassthrough:
    case OverlaySetPersistent:
    case OverlaySetReadOnlyMedia:
    case OverlayResetPersistentState:
      if (arity(1, 1, ParseError::Incomplete)) {
        auto value = toLowerAscii(at(0));
        if (result.kind == OverlayResetPersistentState) {
          if (value == "none") value = "off";
          if (value == "rollback") value = "on";
        }
        if (value == "on" || value == "off")
          result.args.push_back(value);
        else
          fail(ParseError::UnexpectedArgument, at(0));
      }
      break;
    case VolumeProtect:
    case VolumeUnprotect:
    case VolumeCreateSwapfile:
    case VolumeGetConfig: {
      if (!arity(result.kind == VolumeGetConfig ? 0U : 1U, 1, ParseError::MissingVolumeArg) || count == 0) break;
      if (toLowerAscii(at(0)) == "all") {
        if (result.kind == VolumeProtect)
          result.kind = VolumeProtectAll;
        else if (result.kind == VolumeGetConfig)
          result.args.emplace_back("all");
        else
          fail(ParseError::Unsupported, "all");
      } else if (auto volume = volumeOperand(at(0)))
        result.args.push_back(std::move(*volume));
      else
        fail(ParseError::InvalidVolume, at(0));
      break;
    }
    case GetConfig:
    case OverlayGetConfig:
    case RegistryGetExclusions:
    case ServicingGetConfig:
      if (arity(0, 1, ParseError::Incomplete) && count != 0) session(0);
      break;
    case FileGetExclusions:
      if (arity(0, 2, ParseError::MissingVolumeArg) && count != 0) {
        if (at(0).empty())
          fail(ParseError::MissingVolumeArg);
        else {
          result.args.push_back(toLowerAscii(at(0)) == "all" ? "all" : at(0));
          if (count == 2) session(1);
        }
      }
      break;
    case FileAddExclusion:
    case FileRemoveExclusion:
    case FileCommit:
    case FileCommitDelete:
      requiredText(ParseError::MissingPathArg);
      break;
    case RegistryAddExclusion:
    case RegistryRemoveExclusion:
      requiredText(ParseError::MissingRegistryKeyArg);
      break;
    case RegistryCommit:
    case RegistryCommitDelete:
      if (arity(result.kind == RegistryCommit ? 2U : 1U, 2, ParseError::Incomplete)) {
        if (at(0).empty())
          fail(ParseError::MissingRegistryKeyArg);
        else {
          result.args.push_back(at(0));
          if (count == 2) result.args.push_back(at(1));
        }
      }
      break;
    default:
      arity(0, 0, ParseError::Incomplete);
      break;
  }
  return result;
}

std::string uwfmgrHelpText(const std::string_view family) {
  std::string output = "UWF Manager version " UWF_VER_STRING "\n\n";
  if (family.empty()) {
    output += R"(Usage: UWF.exe <parameter> [commands] [arguments]

Description:
    Configure Unified Write Filter (UWF) lockdown settings.

Parameters:
    filter      Configure and display UWF settings, such as filter state.
    overlay     Configure and display overlay settings.
    volume      Configure and display volume filtering settings.
    file        Configure and display file exclusion settings.
    registry    Configure/display registry exclusions and commit changes.
    servicing   Configure and display servicing mode settings.
    get-config  Display all configuration for the current and next sessions.
    help or ?   Display help about the basic parameters.

Examples:
    UWF.exe get-config
    UWF.exe filter enable
    UWF.exe volume protect c:

For more information, put a parameter before help: UWF.exe filter help
)";
    return output;
  }
  const auto help = std::ranges::find(familyHelp, family, &FamilyHelp::name);
  if (help == familyHelp.end()) return output;
  output += "Usage: UWF.exe " + std::string(help->usage) + "\n\nDescription:\n";
  appendHelpDescription(output, help->description, "    ");
  output += "\nCommands:\n";
  for (const auto& descriptor : catalog) {
    if (descriptor.family != family) continue;
    output += "    ";
    output += descriptor.kind == FilterEnableHorm ? "enable-HORM" : descriptor.kind == FilterDisableHorm ? "disable-HORM" : descriptor.verb;
    if (!descriptor.operands.empty()) output += ' ' + std::string(descriptor.operands);
    output += '\n';
    appendHelpDescription(output, descriptor.description, "        ");
  }
  if (help->listsHelp) {
    output += "    help or ?\n        Display command usage and help.\n";
  }
  if (!help->parameters.empty()) output += "\nParameters:\n" + std::string(help->parameters);
  output += "\nExamples:\n" + std::string(help->examples);
  if (!help->notes.empty()) output += "\nNotes:\n" + std::string(help->notes);
  return output;
}

std::string uwfmgrParseErrorText(const UwfmgrCommand& command) {
  std::string message;
  switch (command.parseError) {
    case ParseError::None:
      return {};
    case ParseError::Comment:
      return "No command was specified.";
    case ParseError::Incomplete:
      message = "The command is incomplete.";
      break;
    case ParseError::MissingSizeArg:
      message = "A size in MB is required.";
      break;
    case ParseError::InvalidSize:
      message = "The size must be an unsigned 32-bit decimal integer; maximum overlay size must be positive.";
      break;
    case ParseError::MissingTypeArg:
      message = "An overlay type is required (RAM or Disk).";
      break;
    case ParseError::UnknownType:
      message = "Unknown overlay type; use RAM or Disk.";
      break;
    case ParseError::MissingVolumeArg:
      message = "A volume is required.";
      break;
    case ParseError::InvalidVolume:
      message = "Invalid volume name.";
      break;
    case ParseError::MissingPathArg:
      message = "A nonempty file path is required.";
      break;
    case ParseError::MissingRegistryKeyArg:
      message = "A nonempty registry key is required.";
      break;
    case ParseError::MalformedQuoting:
      message = "The command contains unmatched quotes.";
      break;
    case ParseError::UnexpectedArgument:
      message = "Invalid or unexpected argument.";
      break;
    case ParseError::Unsupported:
      message = "Unknown or unsupported command.";
      break;
  }
  if (!command.parseErrorContext.empty()) message += " Argument: " + command.parseErrorContext;
  return message;
}
}  // namespace uwf::app
