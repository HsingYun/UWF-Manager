// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "domain/uwf/contracts/Requests.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <set>
#include <string_view>
#include <type_traits>

#include "domain/uwf/policies/FileExclusionPolicy.h"
#include "domain/uwf/policies/RegistryCommitPolicy.h"
#include "domain/uwf/policies/RegistryExclusionPolicy.h"
#include "foundation/paths/RegistryPath.h"
#include "foundation/text/Utf8.h"

namespace uwf::compat {
std::string normalizeVolumeId(std::string value) {
  while (!value.empty() && (value.back() == '\\' || value.back() == '/')) value.pop_back();
  if (value.starts_with("\\\\?\\")) value.erase(0, 4);
  for (char& c : value) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  if (value.starts_with("volume{") && value.ends_with('}')) return value.substr(7, value.size() - 8);
  if (value.starts_with('{') && value.ends_with('}')) return value.substr(1, value.size() - 2);
  return value;
}
std::string volumePath(const std::string& id) { return "\\\\?\\Volume{" + normalizeVolumeId(id) + "}\\"; }
std::optional<std::string> parseVolumeId(std::string_view value) {
  // GUID syntax is ASCII, so this also rejects every malformed UTF-8 byte and
  // embedded NUL without ever exposing a truncated C-string to a provider.
  if (value.starts_with("\\\\?\\")) {
    value.remove_prefix(4);
    if (!value.ends_with('\\')) return std::nullopt;
    value.remove_suffix(1);
    if (value.size() != 44) return std::nullopt;
  }
  if (value.size() == 44) {
    constexpr std::string_view prefix = "volume{";
    for (std::size_t i = 0; i < prefix.size(); ++i)
      if (static_cast<char>(std::tolower(static_cast<unsigned char>(value[i]))) != prefix[i]) return std::nullopt;
    if (!value.ends_with('}')) return std::nullopt;
    value = value.substr(7, 36);
  } else if (value.size() == 38 && value.starts_with('{') && value.ends_with('}')) {
    value = value.substr(1, 36);
  }
  if (value.size() != 36) return std::nullopt;
  for (std::size_t i = 0; i < value.size(); ++i) {
    if (i == 8 || i == 13 || i == 18 || i == 23) {
      if (value[i] != '-') return std::nullopt;
    } else if (!((value[i] >= '0' && value[i] <= '9') || (value[i] >= 'a' && value[i] <= 'f') || (value[i] >= 'A' && value[i] <= 'F'))) {
      return std::nullopt;
    }
  }
  return normalizeVolumeId(std::string(value));
}

std::optional<Failure> requestInputFailure(const Request& request) {
  return std::visit(
      [](const auto& operation) -> std::optional<Failure> {
        using R = std::decay_t<decltype(operation)>;
        const auto invalid = [](const char* message) { return Failure{ErrorKind::InvalidArgument, Effect::None, message, {}}; };
        const auto unsupported = [](const char* message) { return Failure{ErrorKind::Unsupported, Effect::None, message, {}}; };
        if constexpr (requires { operation.session; }) {
          if (operation.session != Session::Current && operation.session != Session::Next) return invalid("Unknown UWF session");
        }
        if constexpr (std::is_same_v<R, GetBoolean> || std::is_same_v<R, SetBoolean>) {
          if (operation.property < BooleanProperty::FilterEnabled || operation.property > BooleanProperty::RegistryChangesAlwaysPersisted)
            return invalid("Unknown UWF boolean property");
          if constexpr (std::is_same_v<R, SetBoolean>) {
            if (operation.property == BooleanProperty::ShutdownPending || operation.property == BooleanProperty::RegistryChangesAlwaysPersisted)
              return unsupported("This UWF boolean property is read-only");
            if (operation.property == BooleanProperty::OverlayCommit && !operation.value) return invalid("OverlayCommit accepts only an enable request");
          }
        }
        if constexpr (std::is_same_v<R, GetNumber> || std::is_same_v<R, SetNumber>) {
          if (operation.property < NumberProperty::OverlayType || operation.property > NumberProperty::ResetPersistentStateSavedMode)
            return invalid("Unknown UWF number property");
          if constexpr (std::is_same_v<R, SetNumber>) {
            switch (operation.property) {
              case NumberProperty::OverlayType:
                if (operation.value > 1) return invalid("The overlay type must be RAM or Disk");
                break;
              case NumberProperty::MaximumOverlaySize:
                if (!operation.value) return invalid("The maximum overlay size must be greater than zero");
                break;
              case NumberProperty::OverlayFlags:
                // The UWF abstraction composes option intents into this raw write.
                break;
              case NumberProperty::ResetPersistentState:
                if (operation.value != 0 && operation.value != 1 && operation.value != 255)
                  return invalid("The portable runtime reset modes are Preserve, Clear and Saved");
                break;
              case NumberProperty::ResetPersistentStateSavedMode:
                if (operation.value != 1) return invalid("The supported saved reset policy is rollback");
                break;
              case NumberProperty::WarningThreshold:
              case NumberProperty::CriticalThreshold:
              case NumberProperty::Count:
                break;
            }
          }
        }
        if constexpr (std::is_same_v<R, SetOverlayOption>) {
          if (operation.option < OverlayOption::Passthrough || operation.option > OverlayOption::ReadOnlyMedia) return invalid("Unknown overlay option");
        }
        if constexpr (std::is_same_v<R, SetVolume> || std::is_same_v<R, GetVolumeProperty>) {
          if (operation.property != VolumeProperty::Protected && operation.property != VolumeProperty::BindByDriveLetter)
            return invalid("Unknown UWF volume property");
        }
        if constexpr (std::is_same_v<R, EditFileExclusion> || std::is_same_v<R, EditRegistryExclusion>) {
          if (operation.action < ExclusionAction::Add || operation.action > ExclusionAction::Clear) return invalid("Unknown exclusion action");
        }
        if constexpr (std::is_same_v<R, Power>) {
          if (operation.action != PowerAction::Shutdown && operation.action != PowerAction::Restart) return invalid("Unknown UWF power action");
        }
        const auto exact = [&](const std::string& text) -> std::optional<Failure> {
          if (text.find('\0') != std::string::npos)
            return Failure{ErrorKind::Unsupported, Effect::None, "The UWF protocol cannot preserve an embedded NUL in the exact target", {}};
          if (!utf8ToUtf16(text)) return invalid("The UWF target is not valid UTF-8");
          return std::nullopt;
        };
        if constexpr (requires { operation.volume; }) {
          if (!parseVolumeId(operation.volume)) return invalid("The UWF volume identity must be a complete GUID");
        }
        if constexpr (requires { operation.driveLetter; }) {
          if (operation.driveLetter.find('\0') != std::string::npos || !utf8ToUtf16(operation.driveLetter))
            return invalid("The UWF drive-letter context is not a valid UTF-8 C string");
        }
        if constexpr (std::is_same_v<R, CommitRegistry> || std::is_same_v<R, CommitRegistryDeletion>) {
          if (const auto error = exact(operation.key)) return error;
          if (operation.valueName) {
            if (const auto error = exact(*operation.valueName)) return error;
          }
          if (!core::preservesRegistryCommitTarget(operation.key))
            return Failure{ErrorKind::Unsupported, Effect::None, "The UWF protocol cannot preserve the exact registry target", {}};
          if constexpr (std::is_same_v<R, CommitRegistry>) {
            if (!operation.valueName)
              return Failure{ErrorKind::Unsupported, Effect::None, "Ordinary registry commit requires one exact value, not a whole key", {}};
          } else {
            if (operation.valueName && operation.valueName->empty())
              return Failure{ErrorKind::Unsupported, Effect::None, "The UWF deletion protocol cannot preserve default-value identity", {}};
          }
          if (!core::isRegistryCommitTargetAllowed(operation.key, operation.valueName.has_value()))
            return invalid("The exact registry maintenance target is reserved or uses an unsupported hive");
          if (utf8ToUtf16(regkey::expandHive(operation.key))->size() >= limits::registryKeyBufferUnits ||
              (operation.valueName && utf8ToUtf16(*operation.valueName)->size() >= limits::registryValueBufferUnits))
            return invalid("The exact registry maintenance target exceeds the captured key or value buffer");
        } else if constexpr (std::is_same_v<R, EditRegistryExclusion>) {
          if (operation.action != ExclusionAction::Clear) {
            if (const auto failure = exact(operation.key)) return failure;
            if (operation.action == ExclusionAction::Add && core::registryExclusionError(*utf8ToUtf16(operation.key)) != core::RegistryExclusionError::None)
              return invalid("The registry exclusion target is invalid");
          }
        } else if constexpr (std::is_same_v<R, FindRegistryExclusion>) {
          return exact(operation.key);
        } else if constexpr (std::is_same_v<R, EditFileExclusion>) {
          if (operation.action != ExclusionAction::Clear) {
            if (const auto failure = exact(operation.path)) return failure;
            if (operation.action == ExclusionAction::Add && core::fileExclusionError(*utf8ToUtf16(operation.path)) != core::FileExclusionError::None)
              return invalid("The file exclusion path has invalid characters or length");
          }
        } else if constexpr (std::is_same_v<R, CommitFile> || std::is_same_v<R, CommitFileDeletion>) {
          if (const auto failure = exact(operation.path)) return failure;
          if (operation.path.empty() || operation.path.front() != '\\' || operation.path.starts_with("\\\\") || operation.path.find(':') != std::string::npos ||
              operation.path.find('/') != std::string::npos)
            return invalid("A file maintenance target must be an exact rooted volume-relative path");
          if (utf8ToUtf16(operation.path)->size() >= limits::fileMaintenanceBufferUnits)
            return invalid("The file maintenance target cannot fit the captured management path buffer");
        } else if constexpr (std::is_same_v<R, FindFileExclusion>) {
          return exact(operation.path);
        }
        return std::nullopt;
      },
      request);
}
namespace {
template <class E, std::size_t N>
std::string enumName(E value, const std::array<std::string_view, N>& names) {
  const auto index = static_cast<std::size_t>(value);
  return index < names.size() ? std::string(names[index]) : "Invalid";
}
std::string property(BooleanProperty p) {
  constexpr std::array<std::string_view, 8> names{"FilterEnabled",          "ShutdownPending", "HormEnabled",   "ServicingEnabled",
                                                  "PersistDomainSecretKey", "PersistTscal",    "OverlayCommit", "RegistryChangesAlwaysPersisted"};
  return enumName(p, names);
}
std::string property(NumberProperty p) {
  constexpr std::array<std::string_view, 7> names{
      "OverlayType", "MaximumOverlaySize", "WarningThreshold", "CriticalThreshold", "OverlayFlags", "ResetPersistentState", "ResetPersistentStateSavedMode"};
  return enumName(p, names);
}
}  // namespace
std::string operationName(const Request& request) {
  return std::visit(
      [](const auto& r) -> std::string {
        using R = std::decay_t<decltype(r)>;
        if constexpr (std::is_same_v<R, GetBoolean>)
          return "Get" + property(r.property);
        else if constexpr (std::is_same_v<R, SetBoolean>)
          return "Set" + property(r.property);
        else if constexpr (std::is_same_v<R, GetNumber>)
          return "Get" + property(r.property);
        else if constexpr (std::is_same_v<R, SetNumber>)
          return "Set" + property(r.property);
        else if constexpr (std::is_same_v<R, SetOverlayOption>) {
          constexpr std::array<std::string_view, 3> names{"Passthrough", "Persistent", "ReadOnlyMedia"};
          return "Set" + enumName(r.option, names);
        } else if constexpr (std::is_same_v<R, GetOverlayUsage>)
          return "GetOverlayUsage";
        else if constexpr (std::is_same_v<R, GetVolumes>)
          return "GetVolumes";
        else if constexpr (std::is_same_v<R, AddVolume>)
          return "AddVolume";
        else if constexpr (std::is_same_v<R, RemoveVolume>)
          return "RemoveVolume";
        else if constexpr (std::is_same_v<R, SetVolume> || std::is_same_v<R, GetVolumeProperty>) {
          constexpr std::array<std::string_view, 2> names{"VolumeProtection", "VolumeBinding"};
          return std::string(std::is_same_v<R, SetVolume> ? "Set" : "Get") + enumName(r.property, names);
        } else if constexpr (std::is_same_v<R, GetFileExclusions>)
          return "GetFileExclusions";
        else if constexpr (std::is_same_v<R, EditFileExclusion>)
          return "EditFileExclusion";
        else if constexpr (std::is_same_v<R, FindFileExclusion>)
          return "FindFileExclusion";
        else if constexpr (std::is_same_v<R, GetRegistryExclusions>)
          return "GetRegistryExclusions";
        else if constexpr (std::is_same_v<R, EditRegistryExclusion>)
          return "EditRegistryExclusion";
        else if constexpr (std::is_same_v<R, FindRegistryExclusion>)
          return "FindRegistryExclusion";
        else if constexpr (std::is_same_v<R, CommitFile>)
          return "CommitFile";
        else if constexpr (std::is_same_v<R, CommitFileDeletion>)
          return "CommitFileDeletion";
        else if constexpr (std::is_same_v<R, CommitRegistry>)
          return "CommitRegistry";
        else if constexpr (std::is_same_v<R, CommitRegistryDeletion>)
          return "CommitRegistryDeletion";
        else if constexpr (std::is_same_v<R, GetOverlayFiles>)
          return "GetOverlayFiles";
        else if constexpr (std::is_same_v<R, GetSwapfileSize>)
          return "GetSwapfileSize";
        else if constexpr (std::is_same_v<R, CreateSwapfile>)
          return "CreateSwapfile";
        else if constexpr (std::is_same_v<R, ResetSettings>)
          return "ResetSettings";
        else if constexpr (std::is_same_v<R, UpdateWindows>)
          return "UpdateWindows";
        else {
          static_assert(std::is_same_v<R, Power>);
          constexpr std::array<std::string_view, 2> names{"Shutdown", "Restart"};
          return enumName(r.action, names);
        }
      },
      request);
}
bool isWrite(const Request& request) {
  return std::visit(
      [](const auto& r) {
        using R = std::decay_t<decltype(r)>;
        return std::is_same_v<R, SetBoolean> || std::is_same_v<R, SetNumber> || std::is_same_v<R, SetOverlayOption> || std::is_same_v<R, AddVolume> ||
               std::is_same_v<R, RemoveVolume> || std::is_same_v<R, SetVolume> || std::is_same_v<R, EditFileExclusion> ||
               std::is_same_v<R, EditRegistryExclusion> || std::is_same_v<R, CommitFile> || std::is_same_v<R, CommitFileDeletion> ||
               std::is_same_v<R, CommitRegistry> || std::is_same_v<R, CommitRegistryDeletion> || std::is_same_v<R, CreateSwapfile> ||
               std::is_same_v<R, ResetSettings> || std::is_same_v<R, UpdateWindows> || std::is_same_v<R, Power>;
      },
      request);
}
bool validValue(const Request& request, const Value& value) {
  return std::visit([&](const auto& r) { return std::holds_alternative<typename std::decay_t<decltype(r)>::Result>(value); }, request);
}
std::optional<Failure> normalizeValue(const Request& request, Value& value) {
  const auto protocol = [] { return Failure{ErrorKind::Protocol, Effect::None, "The UWF provider returned a malformed value or inventory", {}}; };
  if (!validValue(request, value)) return protocol();
  const auto exact = [](const std::string& text) { return text.find('\0') == std::string::npos && utf8ToUtf16(text).has_value(); };
  const auto textLess = [](const std::string& left, const std::string& right) {
    const auto fold = [](const unsigned char byte) { return byte >= 'A' && byte <= 'Z' ? static_cast<unsigned char>(byte - 'A' + 'a') : byte; };
    const auto count = std::min(left.size(), right.size());
    for (std::size_t index = 0; index < count; ++index) {
      const auto a = fold(static_cast<unsigned char>(left[index]));
      const auto b = fold(static_cast<unsigned char>(right[index]));
      if (a != b) return a < b;
    }
    return left.size() != right.size() ? left.size() < right.size() : left < right;
  };
  if (auto* volumes = std::get_if<std::vector<Volume>>(&value)) {
    std::set<std::string> identities;
    for (auto& volume : *volumes) {
      const auto id = parseVolumeId(volume.id);
      if (!id || !exact(volume.driveLetter) || !identities.insert(*id).second) return protocol();
      if (!volume.driveLetter.empty()) {
        if (volume.driveLetter.size() != 2 || volume.driveLetter[1] != ':' ||
            !((volume.driveLetter[0] >= 'A' && volume.driveLetter[0] <= 'Z') || (volume.driveLetter[0] >= 'a' && volume.driveLetter[0] <= 'z')))
          return protocol();
        if (volume.driveLetter[0] >= 'a' && volume.driveLetter[0] <= 'z') volume.driveLetter[0] = static_cast<char>(volume.driveLetter[0] - 'a' + 'A');
      }
      volume.id = *id;
    }
    std::ranges::sort(*volumes, {}, &Volume::id);
  } else if (auto* entries = std::get_if<std::vector<std::string>>(&value)) {
    for (const auto& entry : *entries)
      if (!exact(entry)) return protocol();
    std::ranges::sort(*entries, textLess);
  } else if (auto* files = std::get_if<std::vector<OverlayFile>>(&value)) {
    for (const auto& file : *files)
      if (!exact(file.path)) return protocol();
    std::ranges::sort(*files, [&](const OverlayFile& left, const OverlayFile& right) {
      return left.path == right.path ? left.bytes < right.bytes : textLess(left.path, right.path);
    });
  } else if (const auto* outcome = std::get_if<WindowsUpdateOutcome>(&value)) {
    if (*outcome != WindowsUpdateOutcome::Completed && *outcome != WindowsUpdateOutcome::RebootRequired && *outcome != WindowsUpdateOutcome::Failed)
      return protocol();
  }
  return std::nullopt;
}
}  // namespace uwf::compat
