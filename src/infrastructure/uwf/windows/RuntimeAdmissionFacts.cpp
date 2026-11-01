// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "infrastructure/uwf/windows/RuntimeAdmissionFacts.h"

#include <Windows.h>

#include <array>
#include <cstddef>
#include <cstring>
#include <exception>
#include <memory>
#include <new>
#include <type_traits>
#include <vector>

#include "infrastructure/uwf/errors/WindowsUwfError.h"
#include "infrastructure/uwf/windows/PhysicalVolumeObservation.h"
#include "infrastructure/windows/errors/WindowsErrorText.h"
#include "infrastructure/windows/resources/UniqueHandle.h"
#include "infrastructure/windows/text/UnicodeConversion.h"

namespace uwf::windows {
namespace {
using namespace compat;
constexpr DWORD romDispositionIoctl = 0x2249c4;
constexpr DWORD volumeStateIoctl = 0x22496c;
constexpr std::string_view factSource = "WindowsRuntimeAdmission";
namespace volumeState {
// Captured UWFVOL_VOLUME_DATA_OUTPUT states used by admission checks.
constexpr DWORD protectedVolume = 2;
constexpr DWORD ineligibleForProtection = 3;
}  // namespace volumeState
// Captured UWFVOL_VOLUME_DATA_OUTPUT, queried by IsPersistentVolume and
// get_AllFixedMountedVolumesProtected. The unused DWORDs remain opaque.
struct UwfVolumeAdmissionState {
  DWORD state;
  std::array<DWORD, 2> opaque;
  BYTE mounted;  // Driver output of UWFVOL_FILTER::IsMounted.
  std::array<BYTE, 3> reserved;
};
static_assert(sizeof(UwfVolumeAdmissionState) == 16 && offsetof(UwfVolumeAdmissionState, mounted) == 12);

void canceled(const Context& context) {
  if (context.stop.stop_requested()) throw Error({ErrorKind::Canceled, Effect::None, "The UWF admission observation was canceled", {}});
}
ErrorKind errorKind(const DWORD error) { return classifyWindowsUwfError(error, WindowsUwfErrorContext::Observation).value_or(ErrorKind::Failed); }
[[noreturn]] void failWindows(const DWORD error, const char* operation) {
  const auto detail = windowsErrorText(error).toUtf8().toStdString();
  throw Error({errorKind(error), Effect::None, detail, {{std::string(factSource), operation, static_cast<std::int64_t>(HRESULT_FROM_WIN32(error)), detail}}});
}
[[noreturn]] void protocol(const char* operation, const char* detail) {
  throw Error({ErrorKind::Protocol, Effect::None, detail, {{std::string(factSource), operation, 0, detail}}});
}
UniqueHandle openVolume(const std::wstring& path, const Context& context) {
  canceled(context);
  // Match CDevice::Initialize's synchronous access/sharing contract; these
  // observation IOCTLs require the handle but do not mutate state.
  UniqueHandle result(
      CreateFileW(path.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr));
  if (!result.valid()) failWindows(GetLastError(), "open admission device");
  return result;
}
DWORD readDevice(HANDLE device, const DWORD code, void* input, const DWORD inputBytes, void* output, const DWORD outputBytes, const DWORD minimumBytes,
                 const char* operation, const Context& context) {
  canceled(context);
  DWORD returned = 0;
  if (!DeviceIoControl(device, code, input, inputBytes, output, outputBytes, &returned, nullptr)) failWindows(GetLastError(), operation);
  canceled(context);
  if (returned < minimumBytes || returned > outputBytes) protocol(operation, "The admission device returned an incomplete or invalid observation");
  return returned;
}
std::wstring devicePath(const std::string& volume) {
  const auto identity = parseVolumeId(volume);
  if (!identity) throw Error({ErrorKind::InvalidArgument, Effect::None, "The UWF volume identity must be a complete GUID", {}});
  auto result = utf8ToWide(volumePath(*identity));
  result.pop_back();  // A volume device, rather than the root directory.
  return result;
}
UwfVolumeAdmissionState readVolumeState(HANDLE device, const Context& context) {
  UwfVolumeAdmissionState state{};
  readDevice(device, volumeStateIoctl, nullptr, 0, &state, sizeof(state), sizeof(state), "read UWF volume admission state", context);
  return state;
}
VolumeAdmission inspect(const std::string& volume, const bool currentFilter, const VolumeAdmissionPurpose purpose, const Context& context) {
  const auto facts = readPhysicalVolumeFacts(volume, purpose != VolumeAdmissionPurpose::Binding, context.stop);
  if (purpose == VolumeAdmissionPurpose::Binding) return {facts.fixedMedia.value_or(false), false};
  switch (core::physicalVolumeAdmission(facts)) {
    case core::PhysicalVolumeAdmission::NotFixedMedia:
      return {false, false};
    case core::PhysicalVolumeAdmission::UnsupportedBus:
      return {true, false};
    case core::PhysicalVolumeAdmission::Unknown:
      throw Error({ErrorKind::Unavailable, Effect::None, "The physical volume admission could not be determined", {}});
    case core::PhysicalVolumeAdmission::Eligible:
      break;
  }
  if (currentFilter) {
    auto device = openVolume(devicePath(volume), context);
    if (readVolumeState(device.get(), context).state == volumeState::ineligibleForProtection) return {true, false};
  }
  return {true, true};
}
bool hormVolumeProtected(const std::string& identity, const Context& context) {
  const auto path = devicePath(identity);
  if (!inspect(identity, true, VolumeAdmissionPurpose::Registration, context).protectionEligible) return true;
  auto device = openVolume(path, context);
  const auto state = readVolumeState(device.get(), context);
  // The captured mounted-volume check accepts unmounted volumes.
  return state.mounted == 0 || state.state == volumeState::protectedVolume;
}
template <class Function>
Response observe(Function&& function) {
  try {
    return Response::success(function());
  } catch (const Error& error) {
    const auto& failure = error.failure();
    return Response::fail(failure.kind, failure.message, Effect::None, failure.diagnostics);
  } catch (const std::bad_alloc& error) {
    return Response::fail(ErrorKind::ResourceExhausted, error.what());
  } catch (const std::exception& error) {
    return Response::fail(ErrorKind::Failed, error.what());
  }
}
// The shared walk preserves the boolean query's first-blocker exit. Only the
// detail observation collects all blocked identities and continues enumeration.
HormVolumeProtectionDetailsResult observeHormProtection(const Context& context, const bool collectBlockers) {
  try {
    canceled(context);
    HormVolumeProtectionDetails result;
    std::array<wchar_t, MAX_PATH + 50> name{};
    const auto handle = FindFirstVolumeW(name.data(), static_cast<DWORD>(name.size()));
    if (handle == INVALID_HANDLE_VALUE) failWindows(GetLastError(), "enumerate HORM admission volumes");
    const std::unique_ptr<void, decltype(&FindVolumeClose)> enumeration(handle, &FindVolumeClose);
    do {
      const auto identity = parseVolumeId(wideToUtf8(name.data()));
      if (!identity) protocol("enumerate HORM admission volumes", "Windows returned an invalid volume identity");
      if (!hormVolumeProtected(*identity, context)) {
        result.allProtected = false;
        if (!collectBlockers) return result;
        result.unprotectedVolumes.push_back(*identity);
      }
      canceled(context);
    } while (FindNextVolumeW(handle, name.data(), static_cast<DWORD>(name.size())));
    const auto error = GetLastError();
    if (error != ERROR_NO_MORE_FILES) failWindows(error, "enumerate HORM admission volumes");
    return result;
  } catch (const Error& error) {
    return error.failure();
  } catch (const std::bad_alloc& error) {
    return Failure{ErrorKind::ResourceExhausted, Effect::None, error.what(), {}};
  } catch (const std::exception& error) {
    return Failure{ErrorKind::Failed, Effect::None, error.what(), {}};
  }
}
std::optional<DWORD> servicingDword(const wchar_t* path, const wchar_t* name, const compat::Context& context) {
  canceled(context);
  HKEY key = nullptr;
  const auto opened = RegOpenKeyExW(HKEY_LOCAL_MACHINE, path, 0, KEY_QUERY_VALUE, &key);
  if (opened == ERROR_FILE_NOT_FOUND || opened == ERROR_PATH_NOT_FOUND) return std::nullopt;
  if (opened != ERROR_SUCCESS) failWindows(static_cast<DWORD>(opened), "read servicing admission state");
  const std::unique_ptr<std::remove_pointer_t<HKEY>, decltype(&RegCloseKey)> settings(key, &RegCloseKey);
  DWORD value = 0, type = 0, bytes = sizeof(value);
  const auto status = RegQueryValueExW(key, name, nullptr, &type, reinterpret_cast<BYTE*>(&value), &bytes);
  canceled(context);
  if (status == ERROR_FILE_NOT_FOUND || status == ERROR_PATH_NOT_FOUND) return std::nullopt;
  if (status == ERROR_MORE_DATA || (status == ERROR_SUCCESS && (type != REG_DWORD || bytes != sizeof(value))))
    protocol("read servicing admission state", "The servicing value is not a four-byte REG_DWORD");
  if (status != ERROR_SUCCESS) failWindows(static_cast<DWORD>(status), "read servicing admission state");
  return value;
}
}  // namespace

compat::Response readRomDisposition(const compat::Context& context) {
  return observe([&] {
    auto device = openVolume(L"\\\\.\\UwfvolControl", context);
    DWORD disposition = 0;
    readDevice(device.get(), romDispositionIoctl, nullptr, 0, &disposition, sizeof(disposition), sizeof(disposition), "read ROM disposition", context);
    return static_cast<std::uint32_t>(disposition);
  });
}
compat::VolumeAdmissionResult inspectVolumeAdmission(const std::string& volume, const bool currentFilter, const compat::VolumeAdmissionPurpose purpose,
                                                     const compat::Context& context) {
  try {
    return inspect(volume, currentFilter, purpose, context);
  } catch (const compat::Error& error) {
    return error.failure();
  } catch (const std::bad_alloc& error) {
    return compat::Failure{compat::ErrorKind::ResourceExhausted, compat::Effect::None, error.what(), {}};
  } catch (const std::exception& error) {
    return compat::Failure{compat::ErrorKind::Failed, compat::Effect::None, error.what(), {}};
  }
}
compat::Response readHormVolumeProtection(const compat::Context& context) {
  const auto observed = observeHormProtection(context, false);
  if (const auto* failure = std::get_if<compat::Failure>(&observed))
    return compat::Response::fail(failure->kind, failure->message, compat::Effect::None, failure->diagnostics);
  return compat::Response::success(std::get<HormVolumeProtectionDetails>(observed).allProtected);
}
HormVolumeProtectionDetailsResult readHormVolumeProtectionDetails(const compat::Context& context) { return observeHormProtection(context, true); }
compat::Response readCapturedSettingsAvailable(const compat::Context& context) {
  return observe([&] {
    canceled(context);
    HKEY key = nullptr;
    const auto opened = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services\\UWFVOL\\Parameters.Default", 0, KEY_READ, &key);
    if (opened == ERROR_FILE_NOT_FOUND || opened == ERROR_PATH_NOT_FOUND) return false;
    if (opened != ERROR_SUCCESS) failWindows(static_cast<DWORD>(opened), "read captured UWF settings");
    const std::unique_ptr<std::remove_pointer_t<HKEY>, decltype(&RegCloseKey)> settings(key, &RegCloseKey);
    DWORD value = 0, type = 0, bytes = sizeof(value);
    const auto status = RegQueryValueExW(key, L"CapturedUwfSettings", nullptr, &type, reinterpret_cast<BYTE*>(&value), &bytes);
    canceled(context);
    if (status == ERROR_FILE_NOT_FOUND || status == ERROR_PATH_NOT_FOUND || status == ERROR_MORE_DATA) return false;
    if (status != ERROR_SUCCESS) failWindows(static_cast<DWORD>(status), "read captured UWF settings");
    return type == REG_DWORD && bytes == sizeof(value) && value == 1;
  });
}
compat::Response readShutdownPrivilegeAvailable(const compat::Context& context) {
  return observe([&] {
    canceled(context);
    LUID shutdown{};
    if (!LookupPrivilegeValueW(nullptr, SE_SHUTDOWN_NAME, &shutdown)) failWindows(GetLastError(), "resolve shutdown privilege");
    HANDLE rawToken = nullptr;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &rawToken)) failWindows(GetLastError(), "open process token for shutdown admission");
    UniqueHandle token(rawToken);
    DWORD required = 0;
    if (GetTokenInformation(token.get(), TokenPrivileges, nullptr, 0, &required))
      protocol("size process shutdown privileges", "The token privilege sizing request unexpectedly returned no buffer");
    const auto sizeError = GetLastError();
    if (sizeError != ERROR_INSUFFICIENT_BUFFER) failWindows(sizeError, "size process shutdown privileges");
    constexpr DWORD headerBytes = offsetof(TOKEN_PRIVILEGES, Privileges);
    if (required < headerBytes) protocol("read process shutdown privileges", "The token privilege list has an invalid size");
    std::vector<std::byte> buffer(required);
    DWORD returned = 0;
    if (!GetTokenInformation(token.get(), TokenPrivileges, buffer.data(), required, &returned)) failWindows(GetLastError(), "read process shutdown privileges");
    canceled(context);
    if (returned < headerBytes || returned > buffer.size()) protocol("read process shutdown privileges", "The token privilege list is incomplete");
    DWORD count = 0;
    std::memcpy(&count, buffer.data(), sizeof(count));
    if (count > (returned - headerBytes) / sizeof(LUID_AND_ATTRIBUTES))
      protocol("read process shutdown privileges", "The token privilege count exceeds the returned list");
    for (DWORD index = 0; index < count; ++index) {
      LUID_AND_ATTRIBUTES privilege{};
      std::memcpy(&privilege, buffer.data() + headerBytes + index * sizeof(privilege), sizeof(privilege));
      if (privilege.Luid.LowPart == shutdown.LowPart && privilege.Luid.HighPart == shutdown.HighPart)
        // Native power enables the privilege itself; possession suffices here.
        // This client-token observation does not describe a WMI host's token.
        return (privilege.Attributes & SE_PRIVILEGE_REMOVED) == 0;
    }
    return false;
  });
}
compat::ServicingAdmissionResult readServicingAdmission(const bool enabled, const compat::Context& context) {
  try {
    constexpr auto statePath = L"SYSTEM\\CurrentControlSet\\Services\\UwfServicingSvc\\Servicing";
    constexpr auto runtimePath = L"SYSTEM\\CurrentControlSet\\Services\\UwfServicingSvc\\Servicing\\Runtime";
    // Preserve the captured getter's zero default without creating its missing key.
    compat::ServicingAdmissionFacts facts{servicingDword(statePath, L"Status", context).value_or(compat::servicingStatus::notStarted), std::nullopt};
    // The transition validates Information before branching on Status, even for a no-op.
    (void)servicingDword(statePath, L"Information", context);
    if (!enabled && (facts.status == compat::servicingStatus::started || facts.status == compat::servicingStatus::failed)) {
      if (const auto saved = servicingDword(runtimePath, L"FilterState", context)) facts.savedFilter = *saved == 1;
    }
    return facts;
  } catch (const compat::Error& error) {
    return error.failure();
  } catch (const std::bad_alloc& error) {
    return compat::Failure{compat::ErrorKind::ResourceExhausted, compat::Effect::None, error.what(), {}};
  } catch (const std::exception& error) {
    return compat::Failure{compat::ErrorKind::Failed, compat::Effect::None, error.what(), {}};
  }
}
}  // namespace uwf::windows
