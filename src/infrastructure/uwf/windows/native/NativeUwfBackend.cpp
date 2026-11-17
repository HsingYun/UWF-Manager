// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "infrastructure/uwf/windows/native/NativeUwfBackend.h"

// clang-format off
#include <Windows.h>
#include <OleAuto.h>
// clang-format on

#include <algorithm>
#include <array>
#include <cstddef>
#include <initializer_list>
#include <memory>
#include <new>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include "infrastructure/uwf/errors/WindowsUwfError.h"
#include "infrastructure/uwf/windows/native/NativeUwfAbi.h"
#include "infrastructure/windows/errors/WindowsErrorText.h"
#include "infrastructure/windows/errors/WindowsSystemError.h"
#include "infrastructure/windows/resources/SystemLibrary.h"
#include "infrastructure/windows/text/UnicodeConversion.h"

namespace uwf::windows::backends {
namespace {
using namespace compat;
using ExportMap = std::unordered_map<std::string_view, NativeUwfProcedure>;
constexpr std::string_view backendName = "Native";
constexpr std::string_view arrayFreeImport = "free";
constexpr std::string_view arrayDeleteImport = "??_V@YAXPEAX@Z";
static_assert(sizeof(HRESULT) == 4 && sizeof(ULONG) == 4 && sizeof(bool) == 1 && sizeof(wchar_t) == 2);

struct PropertyApi {
  std::string_view get;
  std::string_view set;
  bool sessionGetter;
};
constexpr std::array booleanApis{PropertyApi{"UwfCfgGetFilterEnabled", "UwfCfgSetFilterEnabled", true},
                                 PropertyApi{"UwfCfgGetShutdownPending", {}, false},
                                 PropertyApi{"UwfCfgGetHormEnabled", "UwfCfgSetHormEnabled", false},
                                 PropertyApi{"UwfCfgGetServicingModeEnabled", "UwfCfgSetServicingModeEnabled", true},
                                 PropertyApi{"UwfCfgGetPersistDomainSecretKey", "UwfCfgSetPersistDomainSecretKey", true},
                                 PropertyApi{"UwfCfgGetPersistTSCAL", "UwfCfgSetPersistTSCAL", true},
                                 PropertyApi{"UwfCfgGetOverlayCommitState", "UwfCfgSetOverlayCommitState", false},
                                 PropertyApi{{}, {}, false}};  // Shared read-only registry-persistence observation.
constexpr std::array numberApis{PropertyApi{"UwfCfgGetOverlayType", "UwfCfgSetOverlayType", true},
                                PropertyApi{"UwfCfgGetOverlayMaximumSize", "UwfCfgSetOverlayMaximumSize", true},
                                PropertyApi{"UwfCfgGetWarningThreshold", "UwfCfgSetWarningThreshold", false},
                                PropertyApi{"UwfCfgGetCriticalThreshold", "UwfCfgSetCriticalThreshold", false},
                                PropertyApi{"UwfCfgGetOverlayFlags", "UwfCfgSetOverlayFlags", true},
                                PropertyApi{"UwfCfgGetResetPersistentOverlay", "UwfCfgSetResetPersistentOverlay", false},
                                PropertyApi{"UwfCfgGetResetPersistentOverlaySavedMode", "UwfCfgSetResetPersistentOverlaySavedMode", true}};
static_assert(booleanApis.size() == static_cast<std::size_t>(BooleanProperty::Count));
static_assert(numberApis.size() == static_cast<std::size_t>(NumberProperty::Count));
constexpr std::array otherExports{"UwfCfgGetOverlayInformation",      "UwfCfgGetVolumes",        "UwfCfgGetDriveLetter",         "UwfCfgGetVolumeProtected",
                                  "UwfCfgGetVolumeBindByDriveLetter", "UwfCfgAddVolume",         "UwfCfgRemoveVolume",           "UwfCfgSetVolumeProtected",
                                  "UwfCfgSetVolumeBindByDriveLetter", "UwfCfgGetFileExclusions", "UwfCfgAddFileExclusion",       "UwfCfgRemoveFileExclusion",
                                  "UwfCfgGetRegExclusions",           "UwfCfgAddRegExclusion",   "UwfCfgRemoveRegExclusion",     "UwfCfgCommitFile",
                                  "UwfCfgCommitFileDeletion",         "UwfCfgCommitRegistry",    "UwfCfgCommitRegistryDeletion", "UwfCfgGetVolumeSwapfileSize",
                                  "UwfCfgSetVolumeSwapfileSize",      "UwfCfgResetSettings",     "UwfCfgShutdownSystem",         "UwfCfgRestartSystem"};

template <class Property, std::size_t Count>
const PropertyApi& propertyApi(const Property property, const std::array<PropertyApi, Count>& apis) {
  const auto index = static_cast<std::size_t>(property);
  if (index >= apis.size()) throw std::invalid_argument("Unknown UWF property");
  return apis[index];
}
std::string_view volumeSetter(const VolumeProperty property) {
  switch (property) {
    case VolumeProperty::Protected:
      return "UwfCfgSetVolumeProtected";
    case VolumeProperty::BindByDriveLetter:
      return "UwfCfgSetVolumeBindByDriveLetter";
    case VolumeProperty::Count:
      break;
  }
  throw std::invalid_argument("Unknown UWF volume property");
}
std::string_view powerExport(const PowerAction action) {
  switch (action) {
    case PowerAction::Shutdown:
      return "UwfCfgShutdownSystem";
    case PowerAction::Restart:
      return "UwfCfgRestartSystem";
    case PowerAction::Count:
      break;
  }
  throw std::invalid_argument("Unknown UWF power action");
}
std::string_view exclusionExport(const ExclusionAction action, const bool registry) {
  switch (action) {
    case ExclusionAction::Add:
      return registry ? "UwfCfgAddRegExclusion" : "UwfCfgAddFileExclusion";
    case ExclusionAction::Remove:
      return registry ? "UwfCfgRemoveRegExclusion" : "UwfCfgRemoveFileExclusion";
    case ExclusionAction::Clear:
    case ExclusionAction::Count:
      break;
  }
  throw std::invalid_argument("Unknown UWF exclusion action");
}

struct Requirements {
  Requirements(std::initializer_list<std::string_view> symbols, const bool array = false, const bool drive = false, std::string reason = {})
      : exports(symbols), stringArray(array), driveLetter(drive), unsupported(std::move(reason)) {}
  std::vector<std::string_view> exports;
  bool stringArray = false;
  bool driveLetter = false;
  std::string unsupported;
};
Requirements requirements(const Request& request) {
  return std::visit(
      []<class T>(const T& item) -> Requirements {
        if constexpr (std::is_same_v<T, GetBoolean>)
          return {{propertyApi(item.property, booleanApis).get}};
        else if constexpr (std::is_same_v<T, SetBoolean>) {
          if (item.property == BooleanProperty::ServicingEnabled)
            return {{}, false, false, "The native servicing flag does not implement the complete servicing transition"};
          const auto symbol = propertyApi(item.property, booleanApis).set;
          return symbol.empty() ? Requirements{{}, false, false, "The native API does not expose a setter for this property"} : Requirements{{symbol}};
        } else if constexpr (std::is_same_v<T, GetNumber>)
          return {{propertyApi(item.property, numberApis).get}};
        else if constexpr (std::is_same_v<T, SetNumber>)
          return {{propertyApi(item.property, numberApis).set}};
        else if constexpr (std::is_same_v<T, GetOverlayUsage>)
          return {{"UwfCfgGetOverlayInformation"}};
        else if constexpr (std::is_same_v<T, GetVolumes>)
          return {{"UwfCfgGetVolumes", "UwfCfgGetDriveLetter", "UwfCfgGetVolumeProtected", "UwfCfgGetVolumeBindByDriveLetter"}, true, true};
        else if constexpr (std::is_same_v<T, AddVolume>)
          return {{"UwfCfgAddVolume"}};
        else if constexpr (std::is_same_v<T, RemoveVolume>)
          return {{"UwfCfgRemoveVolume"}};
        else if constexpr (std::is_same_v<T, SetVolume>)
          return {{volumeSetter(item.property)}};
        else if constexpr (std::is_same_v<T, GetVolumeProperty>)
          return {{item.property == VolumeProperty::Protected ? "UwfCfgGetVolumeProtected" : "UwfCfgGetVolumeBindByDriveLetter"}};
        else if constexpr (std::is_same_v<T, SetOverlayOption>)
          return {{}, false, false, "Overlay options are composed by the shared configuration command"};
        else if constexpr (std::is_same_v<T, GetFileExclusions>)
          return {{"UwfCfgGetFileExclusions"}, true};
        else if constexpr (std::is_same_v<T, EditFileExclusion>) {
          if (item.action == ExclusionAction::Clear) return {{}, false, false, "Exclusion clearing is composed by the shared configuration command"};
          return {{exclusionExport(item.action, false)}};
        } else if constexpr (std::is_same_v<T, FindFileExclusion>)
          return {{}, false, false, "File exclusion lookup is derived from this backend's scoped list"};
        else if constexpr (std::is_same_v<T, GetRegistryExclusions>)
          return {{"UwfCfgGetRegExclusions"}, true};
        else if constexpr (std::is_same_v<T, EditRegistryExclusion>) {
          if (item.action == ExclusionAction::Clear) return {{}, false, false, "Exclusion clearing is composed by the shared configuration command"};
          return {{exclusionExport(item.action, true)}};
        } else if constexpr (std::is_same_v<T, FindRegistryExclusion>)
          return {{}, false, false, "Registry exclusion lookup is derived from this backend's scoped list"};
        else if constexpr (std::is_same_v<T, CommitFile>)
          return {{"UwfCfgCommitFile"}};
        else if constexpr (std::is_same_v<T, CommitFileDeletion>)
          return {{"UwfCfgCommitFileDeletion"}};
        else if constexpr (std::is_same_v<T, CommitRegistry>) {
          if (!item.valueName) return {{}, false, false, "The captured registry driver does not support whole-key commit"};
          return {{"UwfCfgCommitRegistry"}};
        } else if constexpr (std::is_same_v<T, CommitRegistryDeletion>) {
          if (item.valueName && item.valueName->empty())
            return {{}, false, false, "The captured registry driver cannot safely distinguish default-value deletion from whole-key deletion"};
          return {{"UwfCfgCommitRegistryDeletion"}};
        } else if constexpr (std::is_same_v<T, GetOverlayFiles>)
          return {{}, false, false, "Overlay file enumeration has no verified named native C API"};
        else if constexpr (std::is_same_v<T, GetSwapfileSize>)
          return {{"UwfCfgGetVolumeSwapfileSize"}};
        else if constexpr (std::is_same_v<T, CreateSwapfile>)
          return {{"UwfCfgSetVolumeSwapfileSize"}};
        else if constexpr (std::is_same_v<T, ResetSettings>)
          return {{"UwfCfgResetSettings"}};
        else if constexpr (std::is_same_v<T, UpdateWindows>)
          return {{}, false, false, "Windows servicing updates have no verified named native C API"};
        else if constexpr (std::is_same_v<T, Power>)
          return {{powerExport(item.action)}};
      },
      request);
}

NativeUwfBindings systemBindings() {
  auto library = std::make_shared<SystemLibrary>(L"uwfcfgmgmt.dll");
  NativeUwfBindings bindings;
  if (!library->loaded()) {
    bindings.unavailableCode = static_cast<std::int64_t>(library->error());
    bindings.unavailableReason = windowsErrorText(library->error()).toUtf8().toStdString();
    return bindings;
  }
  bindings.resolveExport = [library](const std::string_view symbol) {
    return reinterpret_cast<NativeUwfProcedure>(library->resolve(std::string(symbol).c_str()));
  };
  bindings.resolveImport = [library](const std::string_view symbol) { return reinterpret_cast<NativeUwfProcedure>(library->importedSymbol(symbol)); };
  return bindings;
}

struct NativeFailure {
  ErrorKind kind;
  std::string operation;
  HRESULT code;
  std::string detail;
};
ErrorKind errorKind(const HRESULT code, const WindowsUwfErrorContext context = WindowsUwfErrorContext::Operation) {
  return classifyWindowsUwfError(static_cast<std::uint32_t>(code), context).value_or(ErrorKind::Failed);
}

using ProducerDeallocate = native::Deallocate;
class StringArray final {
 public:
  explicit StringArray(const NativeUwfProcedure release) : m_release(reinterpret_cast<ProducerDeallocate>(release)) {}
  ~StringArray() {
    if (!m_array.items) return;
    const auto count = std::max<std::int32_t>(0, std::min(m_array.count, m_array.capacity));
    for (std::int32_t index = 0; index < count; ++index) SysFreeString(m_array.items[index]);
    m_release(m_array.items);
  }
  StringArray(const StringArray&) = delete;
  StringArray& operator=(const StringArray&) = delete;
  native::StringArray* output() { return &m_array; }
  std::vector<std::string> strings() const {
    if (m_array.count < 0 || m_array.capacity < m_array.count || (m_array.count != 0 && !m_array.items))
      throw NativeFailure{ErrorKind::Protocol, "ATL::CSimpleArray", E_UNEXPECTED, "The native string array violates the expected ABI"};
    std::vector<std::string> result;
    result.reserve(static_cast<std::size_t>(m_array.count));
    for (std::int32_t index = 0; index < m_array.count; ++index) {
      const auto value = m_array.items[index];
      result.push_back(wideToUtf8(std::wstring_view(value ? value : L"", value ? SysStringLen(value) : 0)));
    }
    return result;
  }

 private:
  native::StringArray m_array;
  ProducerDeallocate m_release;
};
class DriveLetterOutput final {
 public:
  explicit DriveLetterOutput(const NativeUwfProcedure release) : m_release(reinterpret_cast<ProducerDeallocate>(release)) {}
  ~DriveLetterOutput() {
    if (m_value) m_release(m_value);
  }
  DriveLetterOutput(const DriveLetterOutput&) = delete;
  DriveLetterOutput& operator=(const DriveLetterOutput&) = delete;
  wchar_t** output() { return &m_value; }
  std::string text() const {
    if (!m_value) throw NativeFailure{ErrorKind::Protocol, "UwfCfgGetDriveLetter", E_UNEXPECTED, "The native drive-letter getter returned no string"};
    auto result = wideToUtf8(m_value);
    if (!result.empty() && (result.size() != 2 || !((result[0] >= 'A' && result[0] <= 'Z') || (result[0] >= 'a' && result[0] <= 'z')) || result[1] != ':'))
      throw NativeFailure{ErrorKind::Protocol, "UwfCfgGetDriveLetter", E_UNEXPECTED, "The native drive-letter getter returned invalid drive metadata"};
    return result;
  }

 private:
  wchar_t* m_value = nullptr;
  ProducerDeallocate m_release;
};

class Calls final {
 public:
  Calls(const ExportMap& exports, const NativeUwfProcedure freeArray, const NativeUwfProcedure deleteArray, const Context& context)
      : exports(exports), freeArray(freeArray), deleteArray(deleteArray), context(context) {}
  template <class... Args>
  HRESULT invoke(const std::string_view symbol, const bool write, Args... args) {
    checkCanceled();
    const auto found = exports.find(symbol);
    if (found == exports.end() || !found->second)
      throw NativeFailure{ErrorKind::Unsupported, std::string(symbol), HRESULT_FROM_WIN32(ERROR_PROC_NOT_FOUND), "The native export is unavailable"};
    using Function = HRESULT(__cdecl*)(Args...);
    const auto function = reinterpret_cast<Function>(found->second);
    if (write) dispatchedWrite = true;
    const auto result = function(args...);
    if (FAILED(result))
      throw NativeFailure{errorKind(result, write ? WindowsUwfErrorContext::Operation : WindowsUwfErrorContext::Observation), std::string(symbol), result,
                          windowsErrorText(static_cast<std::uint32_t>(result)).toUtf8().toStdString()};
    return result;
  }
  void checkCanceled() const {
    if (context.stop.stop_requested()) throw NativeFailure{ErrorKind::Canceled, {}, HRESULT_FROM_WIN32(ERROR_CANCELLED), "The UWF operation was canceled"};
  }
  const ExportMap& exports;
  NativeUwfProcedure freeArray;
  NativeUwfProcedure deleteArray;
  const Context& context;
  bool dispatchedWrite = false;
};
bool isCurrent(const Session session) { return session == Session::Current; }
std::wstring volumeId(const std::string& value) { return utf8ToWide(normalizeVolumeId(value)); }

Value perform(Calls& calls, const GetVolumeProperty& request);
Value perform(Calls& calls, const GetBoolean& request) {
  const auto& api = propertyApi(request.property, booleanApis);
  bool value = false;
  if (api.sessionGetter)
    calls.invoke(api.get, false, isCurrent(request.session), &value);
  else
    calls.invoke(api.get, false, &value);
  return value;
}
Value perform(Calls& calls, const SetBoolean& request) {
  calls.invoke(propertyApi(request.property, booleanApis).set, true, request.value);
  return std::monostate{};
}
Value perform(Calls& calls, const GetNumber& request) {
  const auto& api = propertyApi(request.property, numberApis);
  ULONG value = 0;
  if (api.sessionGetter)
    calls.invoke(api.get, false, isCurrent(request.session), &value);
  else
    calls.invoke(api.get, false, &value);
  return static_cast<std::uint32_t>(value);
}
Value perform(Calls& calls, const SetNumber& request) {
  calls.invoke(propertyApi(request.property, numberApis).set, true, static_cast<ULONG>(request.value));
  return std::monostate{};
}
Value perform(Calls& calls, const GetOverlayUsage&) {
  // Match the CSP getters used by uwfmgr: omitted outputs retain zero, and
  // the entry point's HRESULT still determines success or failure.
  ULONG consumption = 0, available = 0;
  calls.invoke("UwfCfgGetOverlayInformation", false, &consumption, &available);
  return OverlayUsage{static_cast<std::uint32_t>(consumption), static_cast<std::uint32_t>(available)};
}
Value perform(Calls& calls, const GetVolumes& request) {
  StringArray array(calls.freeArray);
  const auto current = isCurrent(request.session);
  calls.invoke("UwfCfgGetVolumes", false, current, array.output());
  std::vector<Volume> volumes;
  for (const auto& id : array.strings()) {
    calls.checkCanceled();
    Volume volume;
    const auto identity = parseVolumeId(id);
    if (!identity) throw NativeFailure{ErrorKind::Protocol, "UwfCfgGetVolumes", 0, "The native inventory returned an invalid volume identity"};
    volume.id = *identity;
    const auto wideId = utf8ToWide(volume.id);
    // QuerySzValue may allocate before failing; establish DLL-matched ownership
    // before the call.
    DriveLetterOutput letter(calls.deleteArray);
    try {
      calls.invoke("UwfCfgGetDriveLetter", false, current, wideId.c_str(), letter.output());
      volume.driveLetter = letter.text();
    } catch (const NativeFailure& failure) {
      // GUID-only registration need not create the DriveLetter value. Other
      // fields below still have to confirm that this enumerated volume exists.
      if (failure.code != HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) throw;
    }
    volume.protectedVolume = std::get<bool>(perform(calls, GetVolumeProperty{volume.id, VolumeProperty::Protected, request.session}));
    volume.bindByDriveLetter = std::get<bool>(perform(calls, GetVolumeProperty{volume.id, VolumeProperty::BindByDriveLetter, request.session}));
    volumes.push_back(std::move(volume));
  }
  return volumes;
}
Value perform(Calls& calls, const AddVolume& request) {
  const auto id = volumeId(request.volume);
  // The native API derives drive-letter metadata from the volume GUID itself.
  calls.invoke("UwfCfgAddVolume", true, id.c_str());
  return std::monostate{};
}
Value perform(Calls& calls, const RemoveVolume& request) {
  const auto id = volumeId(request.volume);
  calls.invoke("UwfCfgRemoveVolume", true, id.c_str());
  return std::monostate{};
}
Value perform(Calls& calls, const SetVolume& request) {
  const auto id = volumeId(request.volume);
  calls.invoke(volumeSetter(request.property), true, id.c_str(), request.value);
  return std::monostate{};
}
Value perform(Calls& calls, const GetFileExclusions& request) {
  const auto id = volumeId(request.volume);
  StringArray array(calls.freeArray);
  calls.invoke("UwfCfgGetFileExclusions", false, isCurrent(request.session), id.c_str(), array.output());
  return array.strings();
}
Value perform(Calls& calls, const EditFileExclusion& request) {
  const auto id = volumeId(request.volume);
  const auto symbol = exclusionExport(request.action, false);
  const auto path = utf8ToWide(request.path);
  calls.invoke(symbol, true, id.c_str(), path.c_str());
  return std::monostate{};
}
Value perform(Calls&, const FindFileExclusion&) { throw std::logic_error("File exclusion lookup must use the backend's list derivation"); }
Value perform(Calls& calls, const GetRegistryExclusions& request) {
  StringArray array(calls.freeArray);
  calls.invoke("UwfCfgGetRegExclusions", false, isCurrent(request.session), array.output());
  return array.strings();
}
Value perform(Calls& calls, const EditRegistryExclusion& request) {
  const auto symbol = exclusionExport(request.action, true);
  const auto key = utf8ToWide(request.key);
  calls.invoke(symbol, true, key.c_str());
  return std::monostate{};
}
Value perform(Calls&, const FindRegistryExclusion&) { throw std::logic_error("Registry exclusion lookup must use the backend's list derivation"); }
template <class R>
Value commitFile(Calls& calls, const R& request, const std::string_view symbol) {
  const auto id = volumeId(request.volume), path = utf8ToWide(request.path);
  calls.invoke(symbol, true, id.c_str(), path.c_str());
  return std::monostate{};
}
Value perform(Calls& calls, const CommitFile& request) { return commitFile(calls, request, "UwfCfgCommitFile"); }
Value perform(Calls& calls, const CommitFileDeletion& request) { return commitFile(calls, request, "UwfCfgCommitFileDeletion"); }
template <class R>
Value commitRegistry(Calls& calls, const R& request, const std::string_view symbol) {
  const auto key = utf8ToWide(request.key);
  const auto value = request.valueName ? utf8ToWide(*request.valueName) : std::wstring{};
  // Both C exports require a non-null value pointer, including key deletion
  // (valueOperation=false). The boolean selects the key/value operation.
  calls.invoke(symbol, true, request.valueName.has_value(), key.c_str(), value.c_str());
  return std::monostate{};
}
Value perform(Calls& calls, const CommitRegistry& request) { return commitRegistry(calls, request, "UwfCfgCommitRegistry"); }
Value perform(Calls& calls, const CommitRegistryDeletion& request) { return commitRegistry(calls, request, "UwfCfgCommitRegistryDeletion"); }
Value perform(Calls&, const GetOverlayFiles&) { throw std::logic_error("Native overlay enumeration is unsupported"); }
Value perform(Calls& calls, const GetSwapfileSize& request) {
  const auto id = volumeId(request.volume);
  ULONG value = 0;
  calls.invoke("UwfCfgGetVolumeSwapfileSize", false, isCurrent(request.session), id.c_str(), &value);
  return static_cast<std::uint32_t>(value);
}
Value perform(Calls&, const CreateSwapfile&) { throw std::logic_error("Swapfile creation must use the shared configuration command"); }
Value perform(Calls&, const SetOverlayOption&) { throw std::logic_error("Overlay options must use the shared configuration command"); }
Value perform(Calls& calls, const GetVolumeProperty& request) {
  const auto id = volumeId(request.volume);
  bool value = false;
  const auto symbol = request.property == VolumeProperty::Protected ? "UwfCfgGetVolumeProtected" : "UwfCfgGetVolumeBindByDriveLetter";
  calls.invoke(symbol, false, isCurrent(request.session), id.c_str(), &value);
  return value;
}
Value perform(Calls& calls, const ResetSettings&) {
  calls.invoke("UwfCfgResetSettings", true);
  return std::monostate{};
}
Value perform(Calls&, const UpdateWindows&) { throw std::logic_error("Native Windows servicing updates are unsupported"); }
Value perform(Calls& calls, const Power& request) {
  calls.invoke(powerExport(request.action), true);
  return std::monostate{};
}
}  // namespace

NativeUwfBackend::NativeUwfBackend() : NativeUwfBackend(systemBindings()) {}
NativeUwfBackend::NativeUwfBackend(NativeUwfBindings bindings) : m_bindings(std::move(bindings)) {
  if (!m_bindings.resolveExport) return;
  const auto cache = [this](const std::string_view symbol) {
    if (!symbol.empty()) m_exports.emplace(symbol, m_bindings.resolveExport(symbol));
  };
  for (const auto& api : booleanApis) {
    cache(api.get);
    cache(api.set);
  }
  for (const auto& api : numberApis) {
    cache(api.get);
    cache(api.set);
  }
  for (const auto* symbol : otherExports) cache(symbol);
  if (m_bindings.resolveImport) {
    m_free = m_bindings.resolveImport(arrayFreeImport);
    m_deleteArray = m_bindings.resolveImport(arrayDeleteImport);
  }
}
std::string NativeUwfBackend::name() const { return std::string(backendName); }
Capability NativeUwfBackend::probeRaw(const Request& request) const {
  if (!m_bindings.resolveExport)
    return {Support::Unavailable, m_bindings.unavailableReason.empty() ? "The UWF native module is unavailable" : m_bindings.unavailableReason};
  if (sizeof(void*) != 8) return {Support::Unsupported, "The native ABI has only been verified for Windows x64"};
  try {
    const auto required = requirements(request);
    if (!required.unsupported.empty()) return {Support::Unsupported, required.unsupported};
    for (const auto symbol : required.exports) {
      const auto found = m_exports.find(symbol);
      if (found == m_exports.end() || !found->second) return {Support::Unsupported, "Missing native export: " + std::string(symbol)};
    }
    const auto* boolean = std::get_if<GetBoolean>(&request);
    if (boolean && boolean->property == BooleanProperty::PersistTscal && !m_bindings.supportsTscalObservation)
      return {Support::Unsupported, "The native TSCAL getter has a known unmatched critical-section leave"};
    if (required.stringArray && !m_free) return {Support::Unsupported, "The native string-array allocator cannot be matched to its deallocator"};
    if (required.driveLetter && !m_deleteArray) return {Support::Unsupported, "The native drive-letter allocator cannot be matched to its deallocator"};
    return {Support::Supported, {}};
  } catch (const std::invalid_argument& error) {
    return {Support::Unsupported, error.what()};
  }
}
Response NativeUwfBackend::executeRaw(const Request& request, const Context& context) const {
  const auto capability = probeRaw(request);
  if (capability.support != Support::Supported)
    return Response::fail(capability.support == Support::Unavailable ? ErrorKind::Unavailable : ErrorKind::Unsupported, capability.reason, Effect::None,
                          {{name(), operationName(request), m_bindings.unavailableCode, capability.reason}});
  Calls calls(m_exports, m_free, m_deleteArray, context);
  try {
    calls.checkCanceled();
    auto value = std::visit([&calls](const auto& item) { return perform(calls, item); }, request);
    const auto effect = calls.dispatchedWrite ? Effect::Confirmed : Effect::None;
    return {std::move(value), {}, effect, {}};
  } catch (const Error& error) {
    const auto& failure = error.failure();
    return Response::fail(failure.kind, failure.message, calls.dispatchedWrite ? Effect::Uncertain : failure.effect, failure.diagnostics);
  } catch (const NativeFailure& error) {
    return Response::fail(error.kind, error.detail, calls.dispatchedWrite ? Effect::Uncertain : Effect::None,
                          {{name(), error.operation.empty() ? operationName(request) : error.operation, error.code, error.detail}});
  } catch (const WindowsSystemError& error) {
    const auto code = HRESULT_FROM_WIN32(static_cast<DWORD>(error.code().value()));
    const auto kind = errorKind(code, WindowsUwfErrorContext::Observation);
    return Response::fail(kind, error.what(), calls.dispatchedWrite ? Effect::Uncertain : Effect::None,
                          {{name(), "convert native output text", code, error.what()}});
  } catch (const std::bad_alloc& error) {
    return Response::fail(ErrorKind::ResourceExhausted, error.what(), calls.dispatchedWrite ? Effect::Uncertain : Effect::None);
  } catch (const std::invalid_argument& error) {
    return Response::fail(ErrorKind::InvalidArgument, error.what(), calls.dispatchedWrite ? Effect::Uncertain : Effect::None);
  } catch (const std::exception& error) {
    return Response::fail(ErrorKind::Failed, error.what(), calls.dispatchedWrite ? Effect::Uncertain : Effect::None);
  }
}
Capability NativeUwfBackend::probe(const Request& request) const {
  if (const auto failure = requestInputFailure(request)) return {Support::Unsupported, failure->message};
  return probeRaw(request);
}
Response NativeUwfBackend::execute(const Request& request, const Context& context) const {
  if (context.stop.stop_requested()) return Response::fail(ErrorKind::Canceled, "UWF operation canceled before dispatch");
  if (const auto failure = requestInputFailure(request)) return Response::fail(failure->kind, failure->message);
  auto response =
      std::holds_alternative<CreateSwapfile>(request) ? writeSwapfile(std::get<CreateSwapfile>(request).volume, 1, context) : executeRaw(request, context);
  response = normalizeResponse(request, std::move(response));
  if (response.failure && response.failure->diagnostics.empty() && m_bindings.unavailableCode != 0)
    response.failure->diagnostics.push_back({name(), operationName(request), m_bindings.unavailableCode, m_bindings.unavailableReason});
  return response;
}
Response NativeUwfBackend::writeSwapfile(const std::string& volume, const std::uint32_t value, const Context& context) const {
  const auto capability = probeRaw(CreateSwapfile{});
  if (capability.support != Support::Supported)
    return Response::fail(capability.support == Support::Unavailable ? ErrorKind::Unavailable : ErrorKind::Unsupported, capability.reason);
  Calls calls(m_exports, m_free, m_deleteArray, context);
  try {
    const auto id = volumeId(volume);
    calls.invoke("UwfCfgSetVolumeSwapfileSize", true, id.c_str(), static_cast<ULONG>(value));
    return Response::success(std::monostate{}, Effect::Confirmed);
  } catch (const NativeFailure& error) {
    return Response::fail(error.kind, error.detail, calls.dispatchedWrite ? Effect::Uncertain : Effect::None,
                          {{name(), error.operation, error.code, error.detail}});
  } catch (const WindowsSystemError& error) {
    const auto code = HRESULT_FROM_WIN32(static_cast<DWORD>(error.code().value()));
    return Response::fail(errorKind(code, WindowsUwfErrorContext::Observation), error.what(), calls.dispatchedWrite ? Effect::Uncertain : Effect::None,
                          {{name(), "convert native swapfile volume identity", code, error.what()}});
  } catch (const std::bad_alloc& error) {
    return Response::fail(ErrorKind::ResourceExhausted, error.what(), calls.dispatchedWrite ? Effect::Uncertain : Effect::None);
  } catch (const std::exception& error) {
    return Response::fail(ErrorKind::Failed, error.what(), calls.dispatchedWrite ? Effect::Uncertain : Effect::None);
  }
}
}  // namespace uwf::windows::backends
