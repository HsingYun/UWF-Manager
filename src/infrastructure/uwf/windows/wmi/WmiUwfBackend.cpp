// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "infrastructure/uwf/windows/wmi/WmiUwfBackend.h"

#include <array>
#include <exception>
#include <map>
#include <mutex>
#include <optional>
#include <set>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <utility>

#include "foundation/paths/DriveLetter.h"
#include "infrastructure/uwf/errors/WindowsUwfError.h"
#include "infrastructure/windows/management/wmi/WmiError.h"
#include "infrastructure/windows/management/wmi/WmiException.h"
#include "infrastructure/windows/management/wmi/WmiRowUtil.h"

namespace uwf::windows::backends {
struct WmiCurrentIdentities {
  std::mutex mutex;
  std::map<std::string, std::string> volumes;  // Configured Current GUID to object path.
  std::optional<std::string> registryPath;
};

namespace {
using namespace compat;
using wmi::WmiOperations;
using wmi::WmiRow;
using wmi::WmiValue;

constexpr std::string_view backendName = "WMI";
constexpr std::string_view filterClass = "UWF_Filter";
constexpr std::string_view overlayClass = "UWF_Overlay";
constexpr std::string_view overlayConfigClass = "UWF_OverlayConfig";
constexpr std::string_view volumeClass = "UWF_Volume";
constexpr std::string_view registryClass = "UWF_RegistryFilter";
constexpr std::string_view servicingClass = "UWF_Servicing";

struct EntryPoint {
  std::string_view className;
  std::string unsupported;
};

struct PropertyBinding {
  std::string_view className{};
  std::string_view field{};
  std::string_view nextField{};
  bool sessionInstance = false;
  std::string_view unsupported{};

  [[nodiscard]] std::optional<Session> instanceSession(const Session session) const { return sessionInstance ? std::optional{session} : std::nullopt; }
  [[nodiscard]] std::string fieldName(const Session session) const { return std::string(session == Session::Next && !nextField.empty() ? nextField : field); }
};
struct BooleanPropertyMapping {
  BooleanProperty property;
  PropertyBinding binding;
  std::string_view enableMethod{};
  std::string_view disableMethod{};
  std::string_view readOnlyReason{};
};
struct NumberPropertyMapping {
  NumberProperty property;
  PropertyBinding binding;
  std::string_view setter{};
  std::string_view parameter{};
};

// Session fields and session-selected instances are distinct WMI storage forms.
// Only static property metadata belongs here; HORM and persistence writes retain their own handlers.
constexpr std::array booleanProperties{
    BooleanPropertyMapping{.property = BooleanProperty::FilterEnabled,
                           .binding = {.className = filterClass, .field = "CurrentEnabled", .nextField = "NextEnabled"},
                           .enableMethod = "Enable",
                           .disableMethod = "Disable"},
    BooleanPropertyMapping{.property = BooleanProperty::ShutdownPending,
                           .binding = {.className = filterClass, .field = "ShutdownPending"},
                           .readOnlyReason = "ShutdownPending is read-only"},
    BooleanPropertyMapping{.property = BooleanProperty::HormEnabled, .binding = {.className = filterClass, .field = "HORMEnabled"}},
    BooleanPropertyMapping{.property = BooleanProperty::ServicingEnabled,
                           .binding = {.className = servicingClass, .field = "ServicingEnabled", .sessionInstance = true},
                           .enableMethod = "Enable",
                           .disableMethod = "Disable"},
    BooleanPropertyMapping{.property = BooleanProperty::PersistDomainSecretKey,
                           .binding = {.className = registryClass, .field = "PersistDomainSecretKey", .sessionInstance = true}},
    BooleanPropertyMapping{.property = BooleanProperty::PersistTscal,
                           .binding = {.className = registryClass, .field = "PersistTSCAL", .sessionInstance = true}},
    BooleanPropertyMapping{.property = BooleanProperty::OverlayCommit, .binding = {.unsupported = "WMI has no OverlayCommit entry point"}},
    BooleanPropertyMapping{.property = BooleanProperty::RegistryChangesAlwaysPersisted,
                           .binding = {.unsupported = "RegistryChangesAlwaysPersisted is a derived read-only observation"}}};
constexpr std::array numberProperties{
    NumberPropertyMapping{.property = NumberProperty::OverlayType,
                          .binding = {.className = overlayConfigClass, .field = "Type", .sessionInstance = true},
                          .setter = "SetType",
                          .parameter = "type"},
    NumberPropertyMapping{.property = NumberProperty::MaximumOverlaySize,
                          .binding = {.className = overlayConfigClass, .field = "MaximumSize", .sessionInstance = true},
                          .setter = "SetMaximumSize",
                          .parameter = "size"},
    NumberPropertyMapping{.property = NumberProperty::WarningThreshold,
                          .binding = {.className = overlayClass, .field = "WarningOverlayThreshold"},
                          .setter = "SetWarningThreshold",
                          .parameter = "size"},
    NumberPropertyMapping{.property = NumberProperty::CriticalThreshold,
                          .binding = {.className = overlayClass, .field = "CriticalOverlayThreshold"},
                          .setter = "SetCriticalThreshold",
                          .parameter = "size"},
    NumberPropertyMapping{.property = NumberProperty::OverlayFlags, .binding = {.unsupported = "WMI has no OverlayFlags entry point"}},
    NumberPropertyMapping{.property = NumberProperty::ResetPersistentState, .binding = {.unsupported = "WMI has no ResetPersistentState entry point"}},
    NumberPropertyMapping{.property = NumberProperty::ResetPersistentStateSavedMode,
                          .binding = {.unsupported = "WMI has no ResetPersistentStateSavedMode entry point"}}};

template <typename Property, typename Mapping, std::size_t Size>
consteval bool completePropertyMappings(const std::array<Mapping, Size>& mappings) {
  if (Size != static_cast<std::size_t>(Property::Count)) return false;
  for (std::size_t index = 0; index < Size; ++index)
    if (mappings[index].property != static_cast<Property>(index)) return false;
  return true;
}
static_assert(completePropertyMappings<BooleanProperty>(booleanProperties));
static_assert(completePropertyMappings<NumberProperty>(numberProperties));

template <typename Property, typename Mapping, std::size_t Size>
constexpr const Mapping* propertyMapping(const Property property, const std::array<Mapping, Size>& mappings) {
  const auto index = static_cast<std::size_t>(property);
  return index < Size ? &mappings[index] : nullptr;
}
template <typename Property, typename Mapping, std::size_t Size>
const Mapping& requirePropertyMapping(const Property property, const std::array<Mapping, Size>& mappings) {
  if (const auto* mapping = propertyMapping(property, mappings)) return *mapping;
  throw std::invalid_argument("Unknown WMI property");
}

EntryPoint booleanEntry(const BooleanProperty property, const bool writing) {
  const auto* mapping = propertyMapping(property, booleanProperties);
  if (!mapping) return {{}, "Unknown boolean property"};
  if (writing && !mapping->readOnlyReason.empty()) return {{}, std::string(mapping->readOnlyReason)};
  return {mapping->binding.className, std::string(mapping->binding.unsupported)};
}

EntryPoint numberEntry(const NumberProperty property) {
  const auto* mapping = propertyMapping(property, numberProperties);
  return mapping ? EntryPoint{mapping->binding.className, std::string(mapping->binding.unsupported)} : EntryPoint{{}, "Unknown numeric property"};
}

EntryPoint entryPoint(const Request& request) {
  return std::visit(
      [](const auto& value) -> EntryPoint {
        using R = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<R, GetBoolean>)
          return booleanEntry(value.property, false);
        else if constexpr (std::is_same_v<R, SetBoolean>)
          return booleanEntry(value.property, true);
        else if constexpr (std::is_same_v<R, GetNumber>)
          return numberEntry(value.property);
        else if constexpr (std::is_same_v<R, SetNumber>)
          return numberEntry(value.property);
        else if constexpr (std::is_same_v<R, GetOverlayUsage> || std::is_same_v<R, GetOverlayFiles>)
          return {overlayClass, {}};
        else if constexpr (std::is_same_v<R, GetVolumes>)
          return {{}, "WMI volume enumeration cannot establish a complete configured-volume inventory"};
        else if constexpr (std::is_same_v<R, RemoveVolume>)
          return {{}, "WMI has no volume removal entry point"};
        else if constexpr (std::is_same_v<R, GetSwapfileSize>)
          return {{}, "WMI has no swapfile size entry point"};
        else if constexpr (std::is_same_v<R, EditFileExclusion>) {
          if (value.action == ExclusionAction::Clear) return {{}, "Exclusion clearing is composed by the shared configuration command"};
          return {volumeClass, {}};
        } else if constexpr (std::is_same_v<R, EditRegistryExclusion>) {
          if (value.action == ExclusionAction::Clear) return {{}, "Exclusion clearing is composed by the shared configuration command"};
          return {registryClass, {}};
        } else if constexpr (std::is_same_v<R, CommitRegistry>) {
          if (!value.valueName) return {{}, "WMI CommitRegistry requires a value name; whole-key commit is not exposed"};
          return {registryClass, {}};
        } else if constexpr (std::is_same_v<R, CommitRegistryDeletion>) {
          if (value.valueName && value.valueName->empty()) return {{}, "WMI does not distinguish default-value deletion from whole-key deletion"};
          return {registryClass, {}};
        } else if constexpr (std::is_same_v<R, FindFileExclusion> || std::is_same_v<R, FindRegistryExclusion>)
          return {{}, "Exclusion lookup is derived from this backend's scoped list"};
        else if constexpr (std::is_same_v<R, GetRegistryExclusions>)
          return {registryClass, {}};
        else if constexpr (std::is_same_v<R, ResetSettings> || std::is_same_v<R, Power>)
          return {filterClass, {}};
        else if constexpr (std::is_same_v<R, UpdateWindows>)
          return {servicingClass, {}};
        else if constexpr (std::is_same_v<R, CreateSwapfile> || std::is_same_v<R, SetOverlayOption>)
          return {{}, "This command requires configuration composition"};
        else
          return {volumeClass, {}};
      },
      request);
}

std::string pathOf(const WmiRow& row) { return wmi::row::requireString(row, "__PATH", wmi::row::EmptyString::Reject); }

WmiRow readInstance(const WmiOperations& operations, const std::string_view className, const std::optional<Session> session = {}) {
  const auto rows = operations.queryInstances("SELECT * FROM " + std::string(className));
  std::optional<WmiRow> selected;
  for (const auto& row : rows) {
    if (session && wmi::row::requireBool(row, "CurrentSession") != (*session == Session::Current)) continue;
    if (selected) throw wmi::WmiProtocolError("read " + std::string(className), "provider returned duplicate instances");
    (void)pathOf(row);
    selected = row;
  }
  if (!selected) throw wmi::WmiProtocolError("read " + std::string(className), "enumeration returned no requested instance; it cannot establish absence");
  return std::move(*selected);
}

bool readBoolean(const WmiOperations& operations, const GetBoolean& request) {
  const auto& binding = requirePropertyMapping(request.property, booleanProperties).binding;
  const auto row = readInstance(operations, binding.className, binding.instanceSession(request.session));
  return wmi::row::requireBool(row, binding.fieldName(request.session));
}

std::uint32_t readNumber(const WmiOperations& operations, const GetNumber& request) {
  const auto& binding = requirePropertyMapping(request.property, numberProperties).binding;
  return wmi::row::requireUInt(readInstance(operations, binding.className, binding.instanceSession(request.session)), binding.fieldName(request.session));
}

struct VolumeLocator {
  WmiRow row;
  std::string id;
};

std::string decodeDriveLetter(const WmiRow& row) {
  const auto letter = row.find("DriveLetter");
  if (letter == row.end()) throw wmi::WmiDecodeError("decode UWF_Volume", "DriveLetter is missing");
  if (letter->second.isValid()) {
    const auto raw = wmi::row::requireString(row, "DriveLetter");
    if (!raw.empty() && (raw.size() != 2 || !((raw[0] >= 'A' && raw[0] <= 'Z') || (raw[0] >= 'a' && raw[0] <= 'z')) || raw[1] != ':'))
      throw wmi::WmiDecodeError("decode UWF_Volume", "DriveLetter is not an exact drive prefix");
    return drive::normalize(raw);
  }
  return {};
}

std::optional<VolumeLocator> decodeVolumeLocator(WmiRow row, const Session session, const std::optional<std::string>& expectedId = {}) {
  (void)pathOf(row);
  const auto identity = parseVolumeId(wmi::row::requireString(row, "VolumeName", wmi::row::EmptyString::Reject));
  if (wmi::row::requireBool(row, "CurrentSession") != (session == Session::Current) || !identity || (expectedId && *identity != *expectedId))
    throw wmi::WmiProtocolError("resolve UWF volume", "the locator returned an invalid/different volume identity or session");
  if (session == Session::Current) {
    const auto protection = row.find("Protected");
    if (protection == row.end()) throw wmi::WmiDecodeError("resolve UWF volume", "Current Protected presence is missing");
    // MI presence distinguishes configured Current instances from synthetic rows.
    if (!protection->second.isValid()) return {};
  }
  return VolumeLocator{std::move(row), *identity};
}

std::vector<VolumeLocator> readVolumeLocators(const WmiOperations& operations, const Session session, WmiCurrentIdentities* current) {
  const auto rows = operations.queryInstances("SELECT * FROM UWF_Volume");
  std::vector<VolumeLocator> result;
  for (const auto& row : rows) {
    if (wmi::row::requireBool(row, "CurrentSession") != (session == Session::Current)) continue;
    // Synthetic Current rows for unconfigured physical volumes omit Protected presence.
    if (const auto protectedValue = row.find("Protected"); session == Session::Current && protectedValue != row.end() && !protectedValue->second.isValid())
      continue;
    if (auto locator = decodeVolumeLocator(row, session)) result.push_back(std::move(*locator));
  }
  if (session == Session::Current && current) {
    std::map<std::string, std::string> observed;
    std::set<std::string> ambiguous;
    for (const auto& target : result) {
      if (!observed.emplace(target.id, pathOf(target.row)).second) ambiguous.insert(target.id);
    }
    for (const auto& id : ambiguous) observed.erase(id);
    const std::scoped_lock lock(current->mutex);
    current->volumes = std::move(observed);
  }
  return result;
}

std::optional<VolumeLocator> findVolume(const WmiOperations& operations, const std::string& id, const Session session,
                                        WmiCurrentIdentities* current = nullptr) {
  const auto normalized = normalizeVolumeId(id);
  if (session == Session::Current && current) {
    std::optional<std::string> cached;
    {
      const std::scoped_lock lock(current->mutex);
      if (const auto found = current->volumes.find(normalized); found != current->volumes.end()) cached = found->second;
    }
    if (cached) {
      try {
        auto observed = decodeVolumeLocator(operations.getObject(*cached), session, normalized);
        if (observed) return observed;
        const std::scoped_lock lock(current->mutex);
        current->volumes.erase(normalized);
        return {};
      } catch (const wmi::WmiException& error) {
        if (!error.hasCode(wmi::WmiErrorCode::NotFound)) throw;
        // A stale drive key does not prove GUID absence; resolve the identity again.
        const std::scoped_lock lock(current->mutex);
        current->volumes.erase(normalized);
      }
    }
  }
  std::optional<VolumeLocator> selected;
  for (auto& candidate : readVolumeLocators(operations, session, current)) {
    if (candidate.id != normalized) continue;
    if (selected) throw wmi::WmiProtocolError("resolve UWF volume", "multiple WMI instances identify the requested volume");
    selected = std::move(candidate);
  }
  if (selected) return selected;

  // Enumeration may omit rows after a provider failure. Use GetInstance to
  // establish exact GUID absence; all MOF keys are required, and empty
  // DriveLetter selects the VolumeName route instead of a mount lookup.
  const auto volumeName = "Volume{" + normalized + '}';
  std::string quotedName;
  for (const char ch : volumeName) {
    if (ch == '\0') throw std::invalid_argument("The volume identity contains a null character");
    if (ch == '\\' || ch == '"') quotedName += '\\';
    quotedName += ch;
  }
  const auto path =
      std::string(volumeClass) + ".CurrentSession=" + (session == Session::Current ? "TRUE" : "FALSE") + ",DriveLetter=\"\",VolumeName=\"" + quotedName + '"';
  WmiRow row;
  try {
    row = operations.getObject(path);
  } catch (const wmi::WmiException& error) {
    if (error.hasCode(wmi::WmiErrorCode::NotFound)) return {};
    throw;
  }
  // Explicitly unset Protected presence on targeted Current GetInstance proves
  // an unconfigured physical volume; a missing field is a protocol failure.
  return decodeVolumeLocator(std::move(row), session, normalized);
}

VolumeLocator requireVolume(const WmiOperations& operations, const std::string& id, const Session session, WmiCurrentIdentities* current = nullptr) {
  auto found = findVolume(operations, id, session, current);
  if (!found)
    throw wmi::WmiInfrastructureError(static_cast<std::int32_t>(wmi::WmiErrorCode::NotFound), "resolve UWF volume",
                                      "volume is not registered in the requested session");
  return std::move(*found);
}

std::string filePath(const VolumeLocator& volume, const std::string& path) {
  const auto parts = drive::split(path);
  if (parts.letter.empty()) return path;
  if (parts.letter != decodeDriveLetter(volume.row)) throw std::invalid_argument("File path belongs to a different volume");
  return parts.rest;
}

WmiRow fileInputs(const VolumeLocator& volume, const std::string& path) { return {{"FileName", WmiValue::fromString(filePath(volume, path))}}; }
WmiRow registryInputs(const std::string& key) { return {{"RegistryKey", WmiValue::fromString(key)}}; }

std::vector<std::string> exclusions(const WmiOperations& operations, const WmiRow& target, const bool registry) {
  const auto output = operations.callMethodRead(pathOf(target), "GetExclusions");
  return wmi::row::readArrayOutput<std::string>(output, registry ? "ExcludedKeys" : "ExcludedFiles", [registry](const WmiRow& item) {
    return wmi::row::requireEmbeddedString(item, registry ? "RegistryKey" : "FileName");
  });
}

Effect effectOf(const wmi::WriteEffect effect) {
  switch (effect) {
    case wmi::WriteEffect::None:
      return Effect::None;
    case wmi::WriteEffect::Uncertain:
      return Effect::Uncertain;
  }
  return Effect::Uncertain;
}
ReadCompletion completionOf(const wmi::ReadCompletion completion) {
  switch (completion) {
    case wmi::ReadCompletion::NotDispatched:
      return ReadCompletion::NotDispatched;
    case wmi::ReadCompletion::ProviderCompleted:
      return ReadCompletion::ProviderCompleted;
    case wmi::ReadCompletion::Unknown:
      return ReadCompletion::Unknown;
  }
  return ReadCompletion::Unknown;
}

ErrorKind errorKind(const wmi::WmiException& error, const WindowsUwfErrorContext context) {
  using Code = wmi::WmiErrorCode;
  if (dynamic_cast<const wmi::WmiProtocolError*>(&error) || dynamic_cast<const wmi::WmiDecodeError*>(&error)) return ErrorKind::Protocol;
  if (error.hasCode(Code::NotSupported) || error.hasCode(Code::InvalidClass) || error.hasCode(Code::InvalidMethod) ||
      error.hasCode(Code::MethodNotImplemented) || error.hasCode(Code::InvalidProperty) || error.hasCode(Code::MethodDisabled))
    return ErrorKind::Unsupported;
  if (error.hasCode(Code::AccessDenied) || error.hasCode(Code::PrivilegeNotHeld) || error.code() == std::errc::permission_denied)
    return ErrorKind::AccessDenied;
  if (error.hasCode(Code::NotFound)) return ErrorKind::NotFound;
  if (error.hasCode(Code::InvalidParameter) || error.hasCode(Code::InvalidMethodParameters) || error.hasCode(Code::ValueOutOfRange))
    return ErrorKind::InvalidArgument;
  if (error.hasCode(Code::CallCancelled) || error.code() == std::errc::operation_canceled) return ErrorKind::Canceled;
  if (error.hasCode(Code::OutOfMemory) || error.hasCode(Code::OutOfDiskSpace) || error.hasCode(Code::QuotaViolation)) return ErrorKind::ResourceExhausted;
  if (error.hasCode(Code::AlreadyExists)) return ErrorKind::Conflict;
  if (const auto* provider = dynamic_cast<const wmi::WmiProviderError*>(&error)) {
    if (const auto kind = classifyWindowsUwfError(provider->returnValue(), context)) return *kind;
  } else if (error.code().category() == wmi::wmiErrorCategory()) {
    if (const auto kind = classifyWindowsUwfError(static_cast<std::uint32_t>(error.code().value()), context)) return *kind;
  }
  if (dynamic_cast<const wmi::WmiTransportError*>(&error) || error.hasCode(Code::ProviderFailure) || error.hasCode(Code::ProviderNotFound) ||
      error.hasCode(Code::ProviderLoadFailure) || error.hasCode(Code::InvalidNamespace) || error.hasCode(Code::NotAvailable) ||
      error.hasCode(Code::RpcServerFault))
    return ErrorKind::Unavailable;
  return ErrorKind::Failed;
}

class Execution {
 public:
  Execution(WmiOperations& operations, const Context& context, WmiCurrentIdentities& current) : operations(operations), context(context), current(current) {}

  template <class Invoke>
  void write(Invoke&& invoke) {
    if (context.stop.stop_requested()) throw wmi::WmiCancelled("UWF operation canceled before provider dispatch");
    effect = Effect::Uncertain;
    try {
      std::forward<Invoke>(invoke)();
    } catch (const wmi::WmiException& error) {
      effect = dynamic_cast<const wmi::WmiProviderError*>(&error) ? Effect::Uncertain : effectOf(error.writeEffect());
      throw;
    }
    effect = Effect::Confirmed;
  }

  void invoke(const WmiRow& target, const std::string& method, const WmiRow& inputs = {}) {
    write([&] { operations.invokeMethod(pathOf(target), method, inputs); });
  }

  WmiOperations& operations;
  const Context& context;
  WmiCurrentIdentities& current;
  Effect effect = Effect::None;
  ReadCompletion readCompletion = ReadCompletion::NotDispatched;
};

WmiRow registryTarget(Execution& execution, const Session session) {
  if (session == Session::Current) {
    const std::scoped_lock lock(execution.current.mutex);
    if (execution.current.registryPath) return {{"__PATH", WmiValue::fromString(*execution.current.registryPath)}};
  }
  auto target = readInstance(execution.operations, registryClass, session);
  if (session == Session::Current) {
    const std::scoped_lock lock(execution.current.mutex);
    execution.current.registryPath = pathOf(target);
  }
  return target;
}

void setBoolean(Execution& execution, const SetBoolean& request) {
  const auto& mapping = requirePropertyMapping(request.property, booleanProperties);
  const auto& binding = mapping.binding;
  const auto targetSession = request.property == BooleanProperty::HormEnabled ? Session::Current : Session::Next;
  if (request.property == BooleanProperty::PersistDomainSecretKey || request.property == BooleanProperty::PersistTscal) {
    const WmiRow properties{{"CurrentSession", WmiValue::fromBool(false)}, {binding.fieldName(targetSession), WmiValue::fromBool(request.value)}};
    execution.write([&] { execution.operations.putInstance(std::string(binding.className), properties, wmi::WmiPutMode::UpdateOnly); });
    return;
  }
  const auto target = readInstance(execution.operations, binding.className, binding.instanceSession(targetSession));
  if (request.property == BooleanProperty::HormEnabled)
    execution.invoke(target, request.value ? "EnableHORM" : "DisableHORM");
  else
    execution.invoke(target, std::string(request.value ? mapping.enableMethod : mapping.disableMethod));
}

void setNumber(Execution& execution, const SetNumber& request) {
  const auto& mapping = requirePropertyMapping(request.property, numberProperties);
  const auto target = readInstance(execution.operations, mapping.binding.className, mapping.binding.instanceSession(Session::Next));
  if (request.property == NumberProperty::OverlayType && request.value > static_cast<std::uint32_t>(OverlayType::Disk))
    throw std::invalid_argument("Overlay type must be RAM or Disk");
  execution.invoke(target, std::string(mapping.setter), {{std::string(mapping.parameter), WmiValue::fromUInt(request.value)}});
}

void addVolume(Execution& execution, const AddVolume& request) {
  const auto id = normalizeVolumeId(request.volume);
  std::string letter = drive::normalize(request.driveLetter);
  if (!request.driveLetter.empty() && letter.empty()) throw std::invalid_argument("Volume drive letter is invalid");
  const std::string name = "Volume{" + id + '}';
  WmiRow properties{{"CurrentSession", WmiValue::fromBool(false)},
                    {"VolumeName", WmiValue::fromString(name)},
                    {"Protected", WmiValue::fromBool(false)},
                    {"BindByDriveLetter", WmiValue::fromBool(false)}};
  if (!letter.empty()) properties.emplace("DriveLetter", WmiValue::fromString(letter));
  execution.write([&] { execution.operations.putInstance(std::string(volumeClass), properties, wmi::WmiPutMode::CreateOnly); });
}

void setVolume(Execution& execution, const SetVolume& request) {
  const auto target = requireVolume(execution.operations, request.volume, Session::Next);
  std::string method;
  WmiRow inputs;
  switch (request.property) {
    case VolumeProperty::Protected:
      method = request.value ? "Protect" : "Unprotect";
      break;
    case VolumeProperty::BindByDriveLetter:
      method = "SetBindByDriveLetter";
      inputs.emplace("bBindByDriveLetter", WmiValue::fromBool(request.value));
      break;
    default:
      throw std::invalid_argument("Unknown volume property");
  }
  execution.invoke(target.row, method, inputs);
}

void editExclusion(Execution& execution, const WmiRow& target, const WmiRow& inputs, const ExclusionAction action) {
  if (action != ExclusionAction::Add && action != ExclusionAction::Remove) throw std::invalid_argument("Unknown exclusion action");
  const bool add = action == ExclusionAction::Add;
  execution.invoke(target, add ? "AddExclusion" : "RemoveExclusion", inputs);
}

template <class R>
typename R::Result run(Execution& execution, const R& request) {
  auto& operations = execution.operations;
  if constexpr (std::is_same_v<R, GetBoolean>)
    return readBoolean(operations, request);
  else if constexpr (std::is_same_v<R, SetBoolean>) {
    setBoolean(execution, request);
    return {};
  } else if constexpr (std::is_same_v<R, GetNumber>)
    return readNumber(operations, request);
  else if constexpr (std::is_same_v<R, SetNumber>) {
    setNumber(execution, request);
    return {};
  } else if constexpr (std::is_same_v<R, GetOverlayUsage>) {
    const auto row = readInstance(operations, overlayClass);
    return {wmi::row::requireUInt(row, "OverlayConsumption"), wmi::row::requireUInt(row, "AvailableSpace")};
  } else if constexpr (std::is_same_v<R, AddVolume>) {
    addVolume(execution, request);
    return {};
  } else if constexpr (std::is_same_v<R, SetVolume>) {
    setVolume(execution, request);
    return {};
  } else if constexpr (std::is_same_v<R, GetVolumeProperty>) {
    const auto target = requireVolume(operations, request.volume, request.session, &execution.current);
    const auto field = request.property == VolumeProperty::Protected ? "Protected" : "BindByDriveLetter";
    return wmi::row::requireBool(target.row, field);
  } else if constexpr (std::is_same_v<R, GetFileExclusions>)
    return exclusions(operations, requireVolume(operations, request.volume, request.session, &execution.current).row, false);
  else if constexpr (std::is_same_v<R, EditFileExclusion>) {
    const auto target = requireVolume(operations, request.volume, Session::Next);
    editExclusion(execution, target.row, fileInputs(target, request.path), request.action);
    return {};
  } else if constexpr (std::is_same_v<R, FindFileExclusion> || std::is_same_v<R, FindRegistryExclusion>) {
    throw std::logic_error("Exclusion lookup must use the backend's scoped-list derivation");
  } else if constexpr (std::is_same_v<R, GetRegistryExclusions>)
    return exclusions(operations, registryTarget(execution, request.session), true);
  else if constexpr (std::is_same_v<R, EditRegistryExclusion>) {
    editExclusion(execution, readInstance(operations, registryClass, Session::Next), registryInputs(request.key), request.action);
    return {};
  } else if constexpr (std::is_same_v<R, CommitFile> || std::is_same_v<R, CommitFileDeletion>) {
    const auto target = requireVolume(operations, request.volume, Session::Current, &execution.current);
    execution.invoke(target.row, std::is_same_v<R, CommitFile> ? "CommitFile" : "CommitFileDeletion", fileInputs(target, request.path));
    return {};
  } else if constexpr (std::is_same_v<R, CommitRegistry> || std::is_same_v<R, CommitRegistryDeletion>) {
    auto inputs = registryInputs(request.key);
    if (request.valueName) inputs.emplace("ValueName", WmiValue::fromString(*request.valueName));
    execution.invoke(registryTarget(execution, Session::Current), std::is_same_v<R, CommitRegistry> ? "CommitRegistry" : "CommitRegistryDeletion", inputs);
    return {};
  } else if constexpr (std::is_same_v<R, GetOverlayFiles>) {
    const auto target = readInstance(operations, overlayClass);
    const WmiRow inputs{{"Volume", WmiValue::fromString("Volume{" + normalizeVolumeId(request.volume) + '}')}};
    execution.readCompletion = ReadCompletion::Unknown;
    const auto output = [&] {
      try {
        return operations.callMethodReadOnce(pathOf(target), "GetOverlayFiles", inputs, execution.context.stop);
      } catch (const wmi::WmiException& error) {
        execution.readCompletion = completionOf(error.readCompletion());
        throw;
      }
    }();
    execution.readCompletion = ReadCompletion::ProviderCompleted;
    return wmi::row::readArrayOutput<OverlayFile>(output, "OverlayFiles", [](const WmiRow& row) {
      return OverlayFile{wmi::row::requireEmbeddedString(row, "FileName"), wmi::row::requireEmbeddedUInt64(row, "FileSize")};
    });
  } else if constexpr (std::is_same_v<R, ResetSettings>) {
    execution.invoke(readInstance(operations, filterClass), "ResetSettings");
    return {};
  } else if constexpr (std::is_same_v<R, UpdateWindows>) {
    const auto target = readInstance(operations, servicingClass, Session::Current);
    wmi::WmiMethodOutput output;
    execution.write([&] { output = operations.invokeMethod(pathOf(target), "UpdateWindows"); });
    // Outcome is independent of ReturnValue: the provider can complete the
    // method successfully while its update agent reports failure.
    execution.effect = Effect::Uncertain;
    const auto status = wmi::row::requireUInt(output.values, "UpdateStatus");
    if (status > static_cast<std::uint32_t>(WindowsUpdateOutcome::Failed))
      throw wmi::WmiProtocolError("decode Windows update outcome", "provider returned an unknown UpdateStatus");
    execution.effect = Effect::Confirmed;
    return static_cast<WindowsUpdateOutcome>(status);
  } else if constexpr (std::is_same_v<R, Power>) {
    if (request.action != PowerAction::Shutdown && request.action != PowerAction::Restart) throw std::invalid_argument("Unknown power action");
    execution.invoke(readInstance(operations, filterClass), request.action == PowerAction::Shutdown ? "ShutdownSystem" : "RestartSystem");
    return {};
  } else {
    // Unsupported variants are rejected by entryPoint before dispatch.
    throw std::logic_error("Request has no WMI entry point");
  }
}

}  // namespace

WmiUwfBackend::WmiUwfBackend(wmi::WmiOperations& operations) : WmiUwfBackend(operations.threadAccessor()) {}

WmiUwfBackend::WmiUwfBackend(Accessor operations) : m_operations(std::move(operations)), m_currentIdentities(std::make_shared<WmiCurrentIdentities>()) {
  if (!m_operations) throw std::invalid_argument("WMI thread accessor is required");
}

std::string WmiUwfBackend::name() const { return std::string(backendName); }

Capability WmiUwfBackend::probe(const Request& request) const {
  if (const auto failure = requestInputFailure(request)) return {Support::Unsupported, failure->message};
  return probeRaw(request);
}
Response WmiUwfBackend::execute(const Request& request, const Context& context) const {
  if (context.stop.stop_requested()) return Response::fail(ErrorKind::Canceled, "UWF operation canceled before dispatch");
  if (const auto failure = requestInputFailure(request)) return Response::fail(failure->kind, failure->message);
  return normalizeResponse(request, executeRaw(request, context));
}
Capability WmiUwfBackend::probeRaw(const Request& request) const {
  const auto entry = entryPoint(request);
  if (!entry.unsupported.empty()) return {Support::Unsupported, entry.unsupported};
  try {
    if (m_operations().classStatus(std::string(entry.className)) == wmi::WmiClassStatus::Missing) return {Support::Unsupported, "Required WMI class is absent"};
    // Class discovery provides no method metadata; class presence leaves support unknown.
    return {Support::Unknown, "WMI class is present; member availability is determined by the operation"};
  } catch (const wmi::WmiException& error) {
    if (error.hasCode(wmi::WmiErrorCode::InvalidNamespace) || error.hasCode(wmi::WmiErrorCode::InvalidClass)) return {Support::Unsupported, error.what()};
    return {Support::Unavailable, error.what()};
  } catch (const std::exception& error) {
    return {Support::Unavailable, error.what()};
  } catch (...) {
    return {Support::Unavailable, "WMI capability discovery failed"};
  }
}

Response WmiUwfBackend::executeRaw(const Request& request, const Context& context) const {
  const auto entry = entryPoint(request);
  if (!entry.unsupported.empty()) return Response::fail(ErrorKind::Unsupported, entry.unsupported);
  if (context.stop.stop_requested()) return Response::fail(ErrorKind::Canceled, "UWF operation canceled before execution");
  Effect observedEffect = Effect::None;
  ReadCompletion observedCompletion = ReadCompletion::NotDispatched;
  try {
    Execution execution(m_operations(), context, *m_currentIdentities);
    try {
      return std::visit(
          [&](const auto& value) {
            auto result = run(execution, value);
            return Response::success(std::move(result), execution.effect);
          },
          request);
    } catch (...) {
      observedEffect = execution.effect;
      observedCompletion = execution.readCompletion;
      throw;
    }
  } catch (const wmi::WmiException& error) {
    const auto native = error.diagnostic();
    const auto effect = effectOf(error.writeEffect()) == Effect::Uncertain ? Effect::Uncertain : observedEffect;
    return Response::fail(errorKind(error, isWrite(request) ? WindowsUwfErrorContext::Operation : WindowsUwfErrorContext::Observation), error.what(), effect,
                          {{std::string(backendName), error.operation(), native.result ? static_cast<std::int64_t>(native.result) : native.code, error.what()}},
                          observedCompletion);
  } catch (const std::invalid_argument& error) {
    return Response::fail(ErrorKind::InvalidArgument, error.what(), observedEffect, {}, observedCompletion);
  } catch (const std::exception& error) {
    return Response::fail(ErrorKind::Failed, error.what(), observedEffect, {{std::string(backendName), operationName(request), 0, error.what()}},
                          observedCompletion);
  } catch (...) {
    const std::string message = "WMI operation failed with an unknown exception";
    return Response::fail(ErrorKind::Failed, message, observedEffect, {{std::string(backendName), operationName(request), 0, message}}, observedCompletion);
  }
}

}  // namespace uwf::windows::backends
