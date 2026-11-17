// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
// clang-format off
#include <Windows.h>
#include <OleAuto.h>
// clang-format on

#include <QTest>
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <new>
#include <optional>
#include <stop_token>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "infrastructure/uwf/routing/FallbackUwf.h"
#include "infrastructure/uwf/windows/RuntimeAdmissionFacts.h"
#include "infrastructure/uwf/windows/native/NativeUwfAbi.h"
#include "infrastructure/uwf/windows/native/NativeUwfBackend.h"
#include "infrastructure/windows/text/UnicodeConversion.h"
#include "support/uwf/CapturedUwfContractVectors.h"

namespace {
using namespace uwf::compat;
using namespace uwf::windows::backends;
constexpr auto guid = "11111111-2222-3333-4444-555555555555";
constexpr auto wrappedGuid = "\\\\?\\Volume{11111111-2222-3333-4444-555555555555}\\";
constexpr std::uint32_t unsignedValue = 0xfedcba98;

struct NativeState {
  HRESULT status = S_OK;
  std::optional<HRESULT> writeStatus;
  HRESULT arrayStatus = S_OK;
  std::optional<HRESULT> driveStatus;
  std::optional<HRESULT> protectedStatus;
  HRESULT volumeExistsStatus = S_OK;
  bool boolean = true;
  std::optional<bool> currentBoolean, nextBoolean;
  bool horm = false;
  bool mutate = true;
  bool dispatchedWrite = false;
  std::optional<HRESULT> observationStatus;
  int postWriteReads = 0;
  std::optional<bool> protectedVolume;
  std::optional<bool> bindByDriveLetter;
  ULONG overlayType = static_cast<ULONG>(OverlayType::Ram);
  ULONG number = unsignedValue;
  ULONG usageConsumption = 37, usageAvailable = 987;
  int usageOutputMask = 3;
  std::vector<std::wstring> strings;
  std::vector<bool> sessions;
  std::vector<std::string> readSymbols;
  std::vector<std::string> writeSymbols;
  std::vector<bool> writeBooleans;
  std::vector<ULONG> writeNumbers;
  std::vector<std::wstring> writeVolumes, writeTexts;
  bool literalTscalNext = false;
  ULONG literalCritical = 44, literalRuntimeReset = 0, literalSavedModeNext = 0;
  std::vector<std::wstring> volumes;
  std::vector<std::wstring> paths;
  std::vector<std::wstring> removedPaths;
  std::vector<std::optional<std::wstring>> registryValues;
  std::map<void*, std::vector<BSTR>> arrayBuffers;
  std::vector<void*> driveBuffers;
  int calls = 0;
  int removals = 0;
  int failRemovalAt = 0;
  int arrayReleases = 0;
  int driveReleases = 0;
  bool releaseMatched = true;
  bool registryAbiMatched = true;
  std::wstring driveLetter = L"C:";
  bool omitDriveOutput = false;
  bool swapAllocationFailure = false;
  int swapWriteAttempts = 0;
  int fileCommits = 0, noArgumentWrites = 0;
  std::uint32_t romDisposition = 0;
  std::optional<ULONG> overlayFlags;
};
thread_local NativeState* active = nullptr;

HRESULT readStatus() {
  if (active->dispatchedWrite) {
    ++active->postWriteReads;
    if (active->observationStatus) return *active->observationStatus;
  }
  return active->status;
}

HRESULT __cdecl getSessionBoolean(const bool current, bool* value) {
  ++active->calls;
  active->sessions.push_back(current);
  *value = (current ? active->currentBoolean : active->nextBoolean).value_or(active->boolean);
  return readStatus();
}
HRESULT __cdecl getGlobalBoolean(bool* value) {
  ++active->calls;
  *value = active->boolean;
  return readStatus();
}
HRESULT __cdecl getHorm(bool* value) {
  ++active->calls;
  *value = active->horm;
  return readStatus();
}
HRESULT __cdecl getSessionNumber(const bool current, ULONG* value) {
  ++active->calls;
  active->sessions.push_back(current);
  *value = active->number;
  return readStatus();
}
HRESULT __cdecl getOverlayFlags(const bool current, ULONG* value) {
  const auto result = getSessionNumber(current, value);
  *value = active->overlayFlags.value_or(active->number);
  return result;
}
HRESULT __cdecl getGlobalNumber(ULONG* value) {
  ++active->calls;
  *value = active->number;
  return readStatus();
}
HRESULT __cdecl getUsage(ULONG* consumption, ULONG* available) {
  ++active->calls;
  if (active->usageOutputMask & 1) *consumption = active->usageConsumption;
  if (active->usageOutputMask & 2) *available = active->usageAvailable;
  return active->status;
}
HRESULT __cdecl setBoolean(const bool value) {
  ++active->calls;
  active->dispatchedWrite = true;
  if (active->mutate) active->boolean = value;
  return active->writeStatus.value_or(active->status);
}
HRESULT __cdecl setHorm(const bool value) {
  ++active->calls;
  active->dispatchedWrite = true;
  if (active->mutate) active->horm = value;
  return active->writeStatus.value_or(active->status);
}
HRESULT __cdecl getOverlayType(const bool current, ULONG* value) {
  ++active->calls;
  active->sessions.push_back(current);
  *value = active->overlayType;
  return readStatus();
}
HRESULT __cdecl setOverlayType(const ULONG value) {
  ++active->calls;
  active->dispatchedWrite = true;
  if (active->mutate) active->overlayType = value;
  return active->writeStatus.value_or(active->status);
}
HRESULT __cdecl setNumber(const ULONG value) {
  ++active->calls;
  active->dispatchedWrite = true;
  if (active->mutate) active->number = value;
  return active->writeStatus.value_or(active->status);
}
HRESULT __cdecl setOverlayFlags(const ULONG value) {
  const auto result = setNumber(value);
  if (active->mutate) active->overlayFlags = value;
  return result;
}
HRESULT __cdecl noArguments() {
  ++active->calls;
  ++active->noArgumentWrites;
  return active->writeStatus.value_or(active->status);
}
HRESULT __cdecl setVolumeBoolean(const wchar_t* volume, const bool value) {
  active->volumes.emplace_back(volume);
  return setBoolean(value);
}
HRESULT __cdecl setSwapfile(const wchar_t* volume, const ULONG value) {
  ++active->swapWriteAttempts;
  if (active->swapAllocationFailure) throw std::bad_alloc{};
  active->volumes.emplace_back(volume);
  if (active->mutate) active->overlayType = value ? static_cast<ULONG>(OverlayType::Disk) : static_cast<ULONG>(OverlayType::Ram);
  return setNumber(value);
}
HRESULT __cdecl getSwapfile(const bool current, const wchar_t* volume, ULONG* value) {
  active->volumes.emplace_back(volume);
  return getSessionNumber(current, value);
}
HRESULT __cdecl doesVolumeExist(const bool current, const wchar_t* volume) {
  ++active->calls;
  active->sessions.push_back(current);
  active->volumes.emplace_back(volume);
  return active->volumeExistsStatus;
}
HRESULT __cdecl addVolume(const wchar_t* volume) {
  ++active->calls;
  active->volumes.emplace_back(volume);
  active->protectedVolume = false;
  active->bindByDriveLetter = false;
  active->volumeExistsStatus = S_OK;
  return active->status;
}
HRESULT __cdecl acknowledgeVolumeRemoval(const wchar_t* volume) {
  ++active->calls;
  active->volumes.emplace_back(volume);
  active->dispatchedWrite = true;
  return S_OK;
}
HRESULT __cdecl commitRegistry(const bool hasValue, const wchar_t* key, const wchar_t* value) {
  ++active->calls;
  if (!key || !value || (!hasValue && *value != L'\0')) {
    active->registryAbiMatched = false;
    return E_INVALIDARG;
  }
  active->paths.emplace_back(key);
  active->registryValues.push_back(hasValue ? std::optional<std::wstring>{value} : std::nullopt);
  return active->writeStatus.value_or(active->status);
}
HRESULT fillStrings(native::StringArray* output) {
  if (active->strings.empty()) return active->arrayStatus;
  output->items = static_cast<BSTR*>(HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, active->strings.size() * sizeof(BSTR)));
  if (!output->items) return E_OUTOFMEMORY;
  output->capacity = static_cast<std::int32_t>(active->strings.size());
  auto& allocated = active->arrayBuffers[output->items];
  for (const auto& string : active->strings) {
    const auto value = SysAllocStringLen(string.data(), static_cast<UINT>(string.size()));
    if (!value) return E_OUTOFMEMORY;
    output->items[output->count++] = value;
    allocated.push_back(value);
  }
  return active->arrayStatus;
}
HRESULT __cdecl getVolumes(const bool current, native::StringArray* output) {
  ++active->calls;
  active->sessions.push_back(current);
  return fillStrings(output);
}
HRESULT __cdecl getFileExclusions(const bool current, const wchar_t* volume, native::StringArray* output) {
  active->volumes.emplace_back(volume);
  return getVolumes(current, output);
}
HRESULT __cdecl getDriveLetter(const bool current, const wchar_t* volume, wchar_t** output) {
  ++active->calls;
  active->sessions.push_back(current);
  active->volumes.emplace_back(volume);
  if (active->omitDriveOutput) return active->driveStatus.value_or(active->status);
  const auto& letter = active->driveLetter;
  *output = static_cast<wchar_t*>(HeapAlloc(GetProcessHeap(), 0, (letter.size() + 1) * sizeof(wchar_t)));
  if (!*output) return E_OUTOFMEMORY;
  std::copy(letter.begin(), letter.end(), *output);
  (*output)[letter.size()] = L'\0';
  active->driveBuffers.push_back(*output);
  return active->driveStatus.value_or(active->status);
}

template <BooleanProperty Property>
HRESULT __cdecl literalSessionBoolean(const bool current, bool* value) {
  ++active->calls;
  active->sessions.push_back(current);
  if constexpr (Property == BooleanProperty::FilterEnabled) {
    active->readSymbols.emplace_back("UwfCfgGetFilterEnabled");
    *value = current;
  } else if constexpr (Property == BooleanProperty::ServicingEnabled) {
    active->readSymbols.emplace_back("UwfCfgGetServicingModeEnabled");
    *value = !current;
  } else if constexpr (Property == BooleanProperty::PersistDomainSecretKey) {
    active->readSymbols.emplace_back("UwfCfgGetPersistDomainSecretKey");
    *value = current;
  } else {
    static_assert(Property == BooleanProperty::PersistTscal);
    active->readSymbols.emplace_back("UwfCfgGetPersistTSCAL");
    *value = !current;
  }
  return S_OK;
}
template <BooleanProperty Property>
HRESULT __cdecl literalGlobalBoolean(bool* value) {
  ++active->calls;
  if constexpr (Property == BooleanProperty::ShutdownPending) {
    active->readSymbols.emplace_back("UwfCfgGetShutdownPending");
    *value = true;
  } else if constexpr (Property == BooleanProperty::HormEnabled) {
    active->readSymbols.emplace_back("UwfCfgGetHormEnabled");
    *value = false;
  } else {
    static_assert(Property == BooleanProperty::OverlayCommit);
    active->readSymbols.emplace_back("UwfCfgGetOverlayCommitState");
    *value = true;
  }
  return S_OK;
}
template <NumberProperty Property>
HRESULT __cdecl literalSessionNumber(const bool current, ULONG* value) {
  ++active->calls;
  active->sessions.push_back(current);
  if constexpr (Property == NumberProperty::OverlayType) {
    active->readSymbols.emplace_back("UwfCfgGetOverlayType");
    *value = current ? 0 : 1;
  } else if constexpr (Property == NumberProperty::MaximumOverlaySize) {
    active->readSymbols.emplace_back("UwfCfgGetOverlayMaximumSize");
    *value = current ? 2048 : 4096;
  } else if constexpr (Property == NumberProperty::OverlayFlags) {
    active->readSymbols.emplace_back("UwfCfgGetOverlayFlags");
    *value = current ? 0xa5 : 0x5a;
  } else {
    static_assert(Property == NumberProperty::ResetPersistentStateSavedMode);
    active->readSymbols.emplace_back("UwfCfgGetResetPersistentOverlaySavedMode");
    *value = current ? 77 : 88;
  }
  return S_OK;
}
template <NumberProperty Property>
HRESULT __cdecl literalGlobalNumber(ULONG* value) {
  ++active->calls;
  if constexpr (Property == NumberProperty::WarningThreshold) {
    active->readSymbols.emplace_back("UwfCfgGetWarningThreshold");
    *value = 33;
  } else if constexpr (Property == NumberProperty::CriticalThreshold) {
    active->readSymbols.emplace_back("UwfCfgGetCriticalThreshold");
    *value = 44;
  } else {
    static_assert(Property == NumberProperty::ResetPersistentState);
    active->readSymbols.emplace_back("UwfCfgGetResetPersistentOverlay");
    *value = 66;
  }
  return S_OK;
}
HRESULT __cdecl literalUsage(ULONG* consumption, ULONG* available) {
  ++active->calls;
  active->readSymbols.emplace_back("UwfCfgGetOverlayInformation");
  *consumption = 37;
  *available = 987;
  return S_OK;
}
HRESULT __cdecl literalInventory(const bool current, native::StringArray* output) {
  active->readSymbols.emplace_back("UwfCfgGetVolumes");
  active->strings = {L"11111111-2222-3333-4444-555555555555"};
  return getVolumes(current, output);
}
HRESULT __cdecl literalDriveLetter(const bool current, const wchar_t* volume, wchar_t** output) {
  active->readSymbols.emplace_back("UwfCfgGetDriveLetter");
  active->driveLetter = current ? L"c:" : L"d:";
  return getDriveLetter(current, volume, output);
}
template <VolumeProperty Property>
HRESULT __cdecl literalVolumeBoolean(const bool current, const wchar_t* volume, bool* value) {
  ++active->calls;
  active->sessions.push_back(current);
  active->volumes.emplace_back(volume);
  if constexpr (Property == VolumeProperty::Protected) {
    active->readSymbols.emplace_back("UwfCfgGetVolumeProtected");
    *value = current;
  } else {
    static_assert(Property == VolumeProperty::BindByDriveLetter);
    active->readSymbols.emplace_back("UwfCfgGetVolumeBindByDriveLetter");
    *value = !current;
  }
  return S_OK;
}
HRESULT __cdecl literalFileList(const bool current, const wchar_t* volume, native::StringArray* output) {
  active->readSymbols.emplace_back("UwfCfgGetFileExclusions");
  active->strings = current ? std::vector<std::wstring>{L"\\Data\\current"} : std::vector<std::wstring>{L"\\Data\\next"};
  return getFileExclusions(current, volume, output);
}
HRESULT __cdecl literalRegistryList(const bool current, native::StringArray* output) {
  active->readSymbols.emplace_back("UwfCfgGetRegExclusions");
  active->strings = current ? std::vector<std::wstring>{L"HKLM\\SOFTWARE\\Current"} : std::vector<std::wstring>{L"HKLM\\SOFTWARE\\Next"};
  return getVolumes(current, output);
}
HRESULT __cdecl literalSwapfile(const bool current, const wchar_t* volume, ULONG* value) {
  ++active->calls;
  active->sessions.push_back(current);
  active->volumes.emplace_back(volume);
  active->readSymbols.emplace_back("UwfCfgGetVolumeSwapfileSize");
  *value = current ? 111 : 222;
  return S_OK;
}
HRESULT __cdecl commitNativeFile(const wchar_t* volume, const wchar_t* path) {
  ++active->calls;
  ++active->fileCommits;
  active->volumes.emplace_back(volume);
  active->paths.emplace_back(path);
  active->dispatchedWrite = true;
  return active->writeStatus.value_or(active->status);
}
HRESULT __cdecl getVolumeBoolean(const bool current, const wchar_t* volume, bool* value) {
  active->volumes.emplace_back(volume);
  return getSessionBoolean(current, value);
}
HRESULT __cdecl getFailedInventoryProperty(const bool current, const wchar_t* volume, bool* value) {
  (void)getVolumeBoolean(current, volume, value);
  return active->protectedStatus.value_or(S_OK);
}
HRESULT __cdecl getVolumeProtected(const bool current, const wchar_t* volume, bool* value) {
  const auto result = getVolumeBoolean(current, volume, value);
  if (active->protectedVolume) *value = *active->protectedVolume;
  if (!active->protectedStatus && active->status == S_OK && active->volumeExistsStatus != S_OK) return active->volumeExistsStatus;
  return active->protectedStatus.value_or(result);
}
HRESULT __cdecl getVolumeBinding(const bool current, const wchar_t* volume, bool* value) {
  const auto result = getVolumeBoolean(current, volume, value);
  if (active->bindByDriveLetter) *value = *active->bindByDriveLetter;
  return result;
}
HRESULT removeExclusion(const wchar_t* path) {
  ++active->calls;
  active->paths.emplace_back(path);
  active->removedPaths.emplace_back(path);
  ++active->removals;
  if (active->removals == active->failRemovalAt) return E_ACCESSDENIED;
  std::erase(active->strings, std::wstring(path));
  return S_OK;
}
HRESULT __cdecl removeFileExclusion(const wchar_t* volume, const wchar_t* path) {
  active->volumes.emplace_back(volume);
  return removeExclusion(path);
}
HRESULT __cdecl removeRegistryExclusion(const wchar_t* key) { return removeExclusion(key); }
void recordLiteralWrite(const char* symbol) {
  ++active->calls;
  active->dispatchedWrite = true;
  active->writeSymbols.emplace_back(symbol);
}
HRESULT __cdecl mutableTscal(const bool current, bool* value) {
  ++active->calls;
  active->readSymbols.emplace_back("UwfCfgGetPersistTSCAL");
  active->sessions.push_back(current);
  *value = current ? false : active->literalTscalNext;
  return readStatus();
}
HRESULT __cdecl writeLiteralTscal(const bool value) {
  recordLiteralWrite("UwfCfgSetPersistTSCAL");
  active->writeBooleans.push_back(value);
  active->literalTscalNext = value;
  return S_OK;
}
HRESULT __cdecl mutableCritical(ULONG* value) {
  ++active->calls;
  active->readSymbols.emplace_back("UwfCfgGetCriticalThreshold");
  *value = active->literalCritical;
  return readStatus();
}
HRESULT __cdecl writeLiteralCritical(const ULONG value) {
  recordLiteralWrite("UwfCfgSetCriticalThreshold");
  active->writeNumbers.push_back(value);
  active->literalCritical = value;
  return S_OK;
}
HRESULT __cdecl mutableRuntimeReset(ULONG* value) {
  ++active->calls;
  active->readSymbols.emplace_back("UwfCfgGetResetPersistentOverlay");
  *value = active->literalRuntimeReset;
  return readStatus();
}
HRESULT __cdecl writeLiteralRuntimeReset(const ULONG value) {
  recordLiteralWrite("UwfCfgSetResetPersistentOverlay");
  active->writeNumbers.push_back(value);
  active->literalRuntimeReset = value;
  return S_OK;
}
HRESULT __cdecl mutableSavedMode(const bool current, ULONG* value) {
  ++active->calls;
  active->readSymbols.emplace_back("UwfCfgGetResetPersistentOverlaySavedMode");
  active->sessions.push_back(current);
  *value = current ? 77 : active->literalSavedModeNext;
  return readStatus();
}
HRESULT __cdecl writeLiteralSavedMode(const ULONG value) {
  recordLiteralWrite("UwfCfgSetResetPersistentOverlaySavedMode");
  active->writeNumbers.push_back(value);
  active->literalSavedModeNext = value;
  return S_OK;
}
HRESULT __cdecl mutableOverlayFlags(const bool current, ULONG* value) {
  active->readSymbols.emplace_back("UwfCfgGetOverlayFlags");
  return getOverlayFlags(current, value);
}
HRESULT __cdecl writeLiteralOverlayFlags(const ULONG value) {
  recordLiteralWrite("UwfCfgSetOverlayFlags");
  active->writeNumbers.push_back(value);
  active->overlayFlags = value;
  return S_OK;
}
HRESULT __cdecl writeLiteralProtected(const wchar_t* volume, const bool value) {
  recordLiteralWrite("UwfCfgSetVolumeProtected");
  active->writeVolumes.emplace_back(volume);
  active->writeBooleans.push_back(value);
  active->protectedVolume = value;
  return S_OK;
}
HRESULT __cdecl writeLiteralRemoveVolume(const wchar_t* volume) {
  recordLiteralWrite("UwfCfgRemoveVolume");
  active->writeVolumes.emplace_back(volume);
  active->volumeExistsStatus = HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
  return S_OK;
}
HRESULT __cdecl mutableFileExclusions(const bool current, const wchar_t* volume, native::StringArray* output) {
  active->readSymbols.emplace_back("UwfCfgGetFileExclusions");
  return getFileExclusions(current, volume, output);
}
HRESULT __cdecl mutableRegistryExclusions(const bool current, native::StringArray* output) {
  active->readSymbols.emplace_back("UwfCfgGetRegExclusions");
  return getVolumes(current, output);
}
HRESULT __cdecl writeLiteralFileAdd(const wchar_t* volume, const wchar_t* path) {
  recordLiteralWrite("UwfCfgAddFileExclusion");
  active->writeVolumes.emplace_back(volume);
  active->writeTexts.emplace_back(path);
  active->strings.emplace_back(path);
  return S_OK;
}
HRESULT __cdecl writeLiteralFileRemove(const wchar_t* volume, const wchar_t* path) {
  recordLiteralWrite("UwfCfgRemoveFileExclusion");
  active->writeVolumes.emplace_back(volume);
  active->writeTexts.emplace_back(path);
  std::erase(active->strings, std::wstring(path));
  return S_OK;
}
HRESULT __cdecl writeLiteralRegistryAdd(const wchar_t* key) {
  recordLiteralWrite("UwfCfgAddRegExclusion");
  active->writeTexts.emplace_back(key);
  active->strings.emplace_back(key);
  return S_OK;
}
HRESULT __cdecl writeLiteralRegistryRemove(const wchar_t* key) {
  recordLiteralWrite("UwfCfgRemoveRegExclusion");
  active->writeTexts.emplace_back(key);
  std::erase(active->strings, std::wstring(key));
  return S_OK;
}
void __cdecl producerFree(void* buffer) {
  ++active->arrayReleases;
  const auto found = active->arrayBuffers.find(buffer);
  active->releaseMatched &= found != active->arrayBuffers.end();
  if (found == active->arrayBuffers.end()) return;
  active->arrayBuffers.erase(found);
  active->releaseMatched &= HeapFree(GetProcessHeap(), 0, buffer) != FALSE;
}
void __cdecl producerDeleteArray(void* buffer) {
  ++active->driveReleases;
  const auto found = std::ranges::find(active->driveBuffers, buffer);
  active->releaseMatched &= found != active->driveBuffers.end();
  if (found == active->driveBuffers.end()) return;
  active->driveBuffers.erase(found);
  active->releaseMatched &= HeapFree(GetProcessHeap(), 0, buffer) != FALSE;
}

class Symbols final {
 public:
  Symbols() : m_prior(std::exchange(active, &state)) {}
  ~Symbols() {
    for (const auto& [buffer, strings] : state.arrayBuffers) {
      for (const auto value : strings) SysFreeString(value);
      HeapFree(GetProcessHeap(), 0, buffer);
    }
    for (auto* buffer : state.driveBuffers) HeapFree(GetProcessHeap(), 0, buffer);
    active = m_prior;
  }
  template <class Function>
  void add(const std::string& name, Function function) {
    exports[name] = reinterpret_cast<NativeUwfProcedure>(function);
  }
  void arrays() {
    add("UwfCfgGetHormEnabled", getHorm);
    add("UwfCfgGetVolumes", getVolumes);
    add("UwfCfgGetDriveLetter", getDriveLetter);
    add("UwfCfgGetVolumeProtected", getVolumeProtected);
    add("UwfCfgGetVolumeBindByDriveLetter", getVolumeBinding);
    add("UwfCfgDoesVolumeExist", doesVolumeExist);
    add("UwfCfgGetFileExclusions", getFileExclusions);
    add("UwfCfgRemoveFileExclusion", removeFileExclusion);
    add("UwfCfgGetRegExclusions", getVolumes);
    add("UwfCfgRemoveRegExclusion", removeRegistryExclusion);
    imports["free"] = reinterpret_cast<NativeUwfProcedure>(producerFree);
    imports["??_V@YAXPEAX@Z"] = reinterpret_cast<NativeUwfProcedure>(producerDeleteArray);
  }
  void maintenance() {
    add("UwfCfgGetFilterEnabled", getSessionBoolean);
    add("UwfCfgGetHormEnabled", getHorm);
    add("UwfCfgGetOverlayFlags", getOverlayFlags);
    add("UwfCfgGetVolumeProtected", getVolumeProtected);
    add("UwfCfgDoesVolumeExist", doesVolumeExist);
  }
  void completeExportSurface() {
    // Presence-only fixture for the capability census. The test requires zero
    // export calls; operation semantics are exercised by the direct ABI cases.
    arrays();
    maintenance();
    for (const auto* name : {"UwfCfgGetServicingModeEnabled", "UwfCfgGetPersistDomainSecretKey", "UwfCfgGetPersistTSCAL"}) add(name, getSessionBoolean);
    for (const auto* name : {"UwfCfgGetShutdownPending", "UwfCfgGetOverlayCommitState"}) add(name, getGlobalBoolean);
    for (const auto* name :
         {"UwfCfgSetFilterEnabled", "UwfCfgSetServicingModeEnabled", "UwfCfgSetPersistDomainSecretKey", "UwfCfgSetPersistTSCAL", "UwfCfgSetOverlayCommitState"})
      add(name, setBoolean);
    add("UwfCfgSetHormEnabled", setHorm);
    add("UwfCfgGetOverlayType", getOverlayType);
    add("UwfCfgGetOverlayMaximumSize", getSessionNumber);
    add("UwfCfgGetResetPersistentOverlaySavedMode", getSessionNumber);
    for (const auto* name : {"UwfCfgGetWarningThreshold", "UwfCfgGetCriticalThreshold", "UwfCfgGetResetPersistentOverlay"}) add(name, getGlobalNumber);
    add("UwfCfgSetOverlayType", setOverlayType);
    add("UwfCfgSetOverlayFlags", setOverlayFlags);
    for (const auto* name : {"UwfCfgSetOverlayMaximumSize", "UwfCfgSetWarningThreshold", "UwfCfgSetCriticalThreshold", "UwfCfgSetResetPersistentOverlay",
                             "UwfCfgSetResetPersistentOverlaySavedMode"})
      add(name, setNumber);
    add("UwfCfgGetOverlayInformation", getUsage);
    add("UwfCfgAddVolume", addVolume);
    add("UwfCfgRemoveVolume", acknowledgeVolumeRemoval);
    for (const auto* name : {"UwfCfgSetVolumeProtected", "UwfCfgSetVolumeBindByDriveLetter"}) add(name, setVolumeBoolean);
    add("UwfCfgAddFileExclusion", removeFileExclusion);
    add("UwfCfgAddRegExclusion", removeRegistryExclusion);
    for (const auto* name : {"UwfCfgCommitFile", "UwfCfgCommitFileDeletion"}) add(name, commitNativeFile);
    for (const auto* name : {"UwfCfgCommitRegistry", "UwfCfgCommitRegistryDeletion"}) add(name, commitRegistry);
    add("UwfCfgGetVolumeSwapfileSize", getSwapfile);
    add("UwfCfgSetVolumeSwapfileSize", setSwapfile);
    for (const auto* name : {"UwfCfgResetSettings", "UwfCfgShutdownSystem", "UwfCfgRestartSystem"}) add(name, noArguments);
  }
  void literalReadSurface() {
    arrays();
    add("UwfCfgGetFilterEnabled", literalSessionBoolean<BooleanProperty::FilterEnabled>);
    add("UwfCfgGetServicingModeEnabled", literalSessionBoolean<BooleanProperty::ServicingEnabled>);
    add("UwfCfgGetPersistDomainSecretKey", literalSessionBoolean<BooleanProperty::PersistDomainSecretKey>);
    add("UwfCfgGetPersistTSCAL", literalSessionBoolean<BooleanProperty::PersistTscal>);
    add("UwfCfgGetShutdownPending", literalGlobalBoolean<BooleanProperty::ShutdownPending>);
    add("UwfCfgGetHormEnabled", literalGlobalBoolean<BooleanProperty::HormEnabled>);
    add("UwfCfgGetOverlayCommitState", literalGlobalBoolean<BooleanProperty::OverlayCommit>);
    add("UwfCfgGetOverlayType", literalSessionNumber<NumberProperty::OverlayType>);
    add("UwfCfgGetOverlayMaximumSize", literalSessionNumber<NumberProperty::MaximumOverlaySize>);
    add("UwfCfgGetOverlayFlags", literalSessionNumber<NumberProperty::OverlayFlags>);
    add("UwfCfgGetResetPersistentOverlaySavedMode", literalSessionNumber<NumberProperty::ResetPersistentStateSavedMode>);
    add("UwfCfgGetWarningThreshold", literalGlobalNumber<NumberProperty::WarningThreshold>);
    add("UwfCfgGetCriticalThreshold", literalGlobalNumber<NumberProperty::CriticalThreshold>);
    add("UwfCfgGetResetPersistentOverlay", literalGlobalNumber<NumberProperty::ResetPersistentState>);
    add("UwfCfgGetOverlayInformation", literalUsage);
    add("UwfCfgGetVolumes", literalInventory);
    add("UwfCfgGetDriveLetter", literalDriveLetter);
    add("UwfCfgGetVolumeProtected", literalVolumeBoolean<VolumeProperty::Protected>);
    add("UwfCfgGetVolumeBindByDriveLetter", literalVolumeBoolean<VolumeProperty::BindByDriveLetter>);
    add("UwfCfgGetFileExclusions", literalFileList);
    add("UwfCfgGetRegExclusions", literalRegistryList);
    add("UwfCfgGetVolumeSwapfileSize", literalSwapfile);
  }
  void missingWriteWitnessSurface() {
    completeExportSurface();
    add("UwfCfgGetPersistTSCAL", mutableTscal);
    add("UwfCfgSetPersistTSCAL", writeLiteralTscal);
    add("UwfCfgGetWarningThreshold", literalGlobalNumber<NumberProperty::WarningThreshold>);
    add("UwfCfgGetCriticalThreshold", mutableCritical);
    add("UwfCfgSetCriticalThreshold", writeLiteralCritical);
    add("UwfCfgGetResetPersistentOverlay", mutableRuntimeReset);
    add("UwfCfgSetResetPersistentOverlay", writeLiteralRuntimeReset);
    add("UwfCfgGetResetPersistentOverlaySavedMode", mutableSavedMode);
    add("UwfCfgSetResetPersistentOverlaySavedMode", writeLiteralSavedMode);
    add("UwfCfgGetOverlayFlags", mutableOverlayFlags);
    add("UwfCfgSetOverlayFlags", writeLiteralOverlayFlags);
    add("UwfCfgSetVolumeProtected", writeLiteralProtected);
    add("UwfCfgRemoveVolume", writeLiteralRemoveVolume);
    add("UwfCfgGetFileExclusions", mutableFileExclusions);
    add("UwfCfgGetRegExclusions", mutableRegistryExclusions);
    add("UwfCfgAddFileExclusion", writeLiteralFileAdd);
    add("UwfCfgRemoveFileExclusion", writeLiteralFileRemove);
    add("UwfCfgAddRegExclusion", writeLiteralRegistryAdd);
    add("UwfCfgRemoveRegExclusion", writeLiteralRegistryRemove);
  }
  NativeUwfBindings bindings() {
    return {.resolveExport =
                [this](const std::string_view symbol) {
                  ++resolutions[std::string(symbol)];
                  const auto found = exports.find(std::string(symbol));
                  return found == exports.end() ? nullptr : found->second;
                },
            .resolveImport =
                [this](const std::string_view symbol) {
                  const auto found = imports.find(std::string(symbol));
                  return found == imports.end() ? nullptr : found->second;
                },
            .supportsTscalObservation = true,
            .unavailableReason = {},
            .unavailableCode = 0};
  }
  NativeState state;
  std::map<std::string, NativeUwfProcedure> exports;
  std::map<std::string, NativeUwfProcedure> imports;
  std::map<std::string, int> resolutions;

 private:
  NativeState* m_prior;
};

struct NativeCapabilityCase {
  const char* slot;
  Request request;
  Support support;
  bool requiresTscalObservation = false;
};
const auto& nativeCapabilityCases() {
  constexpr auto key = "HKEY_LOCAL_MACHINE\\SOFTWARE\\Vendor\\Product";
  static const std::array cases{
      NativeCapabilityCase{"GetBoolean.FilterEnabled", GetBoolean{BooleanProperty::FilterEnabled}, Support::Supported},
      NativeCapabilityCase{"GetBoolean.ShutdownPending", GetBoolean{BooleanProperty::ShutdownPending}, Support::Supported},
      NativeCapabilityCase{"GetBoolean.HormEnabled", GetBoolean{BooleanProperty::HormEnabled}, Support::Supported},
      NativeCapabilityCase{"GetBoolean.ServicingEnabled", GetBoolean{BooleanProperty::ServicingEnabled}, Support::Supported},
      NativeCapabilityCase{"GetBoolean.PersistDomainSecretKey", GetBoolean{BooleanProperty::PersistDomainSecretKey}, Support::Supported},
      NativeCapabilityCase{"GetBoolean.PersistTscal", GetBoolean{BooleanProperty::PersistTscal}, Support::Supported, true},
      NativeCapabilityCase{"GetBoolean.OverlayCommit", GetBoolean{BooleanProperty::OverlayCommit}, Support::Supported},
      NativeCapabilityCase{"GetBoolean.RegistryChangesAlwaysPersisted", GetBoolean{BooleanProperty::RegistryChangesAlwaysPersisted}, Support::Unsupported},
      NativeCapabilityCase{"SetBoolean.FilterEnabled", SetBoolean{BooleanProperty::FilterEnabled, true}, Support::Supported},
      NativeCapabilityCase{"SetBoolean.ShutdownPending", SetBoolean{BooleanProperty::ShutdownPending, true}, Support::Unsupported},
      NativeCapabilityCase{"SetBoolean.HormEnabled", SetBoolean{BooleanProperty::HormEnabled, true}, Support::Supported},
      NativeCapabilityCase{"SetBoolean.ServicingEnabled", SetBoolean{BooleanProperty::ServicingEnabled, true}, Support::Unsupported},
      NativeCapabilityCase{"SetBoolean.PersistDomainSecretKey", SetBoolean{BooleanProperty::PersistDomainSecretKey, true}, Support::Supported},
      NativeCapabilityCase{"SetBoolean.PersistTscal", SetBoolean{BooleanProperty::PersistTscal, true}, Support::Supported},
      NativeCapabilityCase{"SetBoolean.OverlayCommit", SetBoolean{BooleanProperty::OverlayCommit, true}, Support::Supported},
      NativeCapabilityCase{"SetBoolean.RegistryChangesAlwaysPersisted", SetBoolean{BooleanProperty::RegistryChangesAlwaysPersisted, true},
                           Support::Unsupported},
      NativeCapabilityCase{"GetNumber.OverlayType", GetNumber{NumberProperty::OverlayType}, Support::Supported},
      NativeCapabilityCase{"GetNumber.MaximumOverlaySize", GetNumber{NumberProperty::MaximumOverlaySize}, Support::Supported},
      NativeCapabilityCase{"GetNumber.WarningThreshold", GetNumber{NumberProperty::WarningThreshold}, Support::Supported},
      NativeCapabilityCase{"GetNumber.CriticalThreshold", GetNumber{NumberProperty::CriticalThreshold}, Support::Supported},
      NativeCapabilityCase{"GetNumber.OverlayFlags", GetNumber{NumberProperty::OverlayFlags}, Support::Supported},
      NativeCapabilityCase{"GetNumber.ResetPersistentState", GetNumber{NumberProperty::ResetPersistentState}, Support::Supported},
      NativeCapabilityCase{"GetNumber.ResetPersistentStateSavedMode", GetNumber{NumberProperty::ResetPersistentStateSavedMode}, Support::Supported},
      NativeCapabilityCase{"SetNumber.OverlayType", SetNumber{NumberProperty::OverlayType, 0}, Support::Supported},
      NativeCapabilityCase{"SetNumber.MaximumOverlaySize", SetNumber{NumberProperty::MaximumOverlaySize, 2048}, Support::Supported},
      NativeCapabilityCase{"SetNumber.WarningThreshold", SetNumber{NumberProperty::WarningThreshold, 0}, Support::Supported},
      NativeCapabilityCase{"SetNumber.CriticalThreshold", SetNumber{NumberProperty::CriticalThreshold, 0}, Support::Supported},
      NativeCapabilityCase{"SetNumber.OverlayFlags", SetNumber{NumberProperty::OverlayFlags, 0}, Support::Supported},
      NativeCapabilityCase{"SetNumber.ResetPersistentState", SetNumber{NumberProperty::ResetPersistentState, 1}, Support::Supported},
      NativeCapabilityCase{"SetNumber.ResetPersistentStateSavedMode", SetNumber{NumberProperty::ResetPersistentStateSavedMode, 1}, Support::Supported},
      NativeCapabilityCase{"SetOverlayOption.Passthrough", SetOverlayOption{OverlayOption::Passthrough, true}, Support::Unsupported},
      NativeCapabilityCase{"SetOverlayOption.Persistent", SetOverlayOption{OverlayOption::Persistent, true}, Support::Unsupported},
      NativeCapabilityCase{"SetOverlayOption.ReadOnlyMedia", SetOverlayOption{OverlayOption::ReadOnlyMedia, true}, Support::Unsupported},
      NativeCapabilityCase{"GetOverlayUsage", GetOverlayUsage{}, Support::Supported},
      NativeCapabilityCase{"GetVolumes", GetVolumes{}, Support::Supported},
      NativeCapabilityCase{"AddVolume", AddVolume{guid, "C:"}, Support::Supported},
      NativeCapabilityCase{"RemoveVolume", RemoveVolume{guid}, Support::Supported},
      NativeCapabilityCase{"SetVolume.Protected", SetVolume{guid, VolumeProperty::Protected, true}, Support::Supported},
      NativeCapabilityCase{"SetVolume.BindByDriveLetter", SetVolume{guid, VolumeProperty::BindByDriveLetter, true}, Support::Supported},
      NativeCapabilityCase{"GetVolumeProperty.Protected", GetVolumeProperty{guid, VolumeProperty::Protected}, Support::Supported},
      NativeCapabilityCase{"GetVolumeProperty.BindByDriveLetter", GetVolumeProperty{guid, VolumeProperty::BindByDriveLetter}, Support::Supported},
      NativeCapabilityCase{"GetFileExclusions", GetFileExclusions{guid}, Support::Supported},
      NativeCapabilityCase{"EditFileExclusion.Add", EditFileExclusion{guid, "\\Data", ExclusionAction::Add}, Support::Supported},
      NativeCapabilityCase{"EditFileExclusion.Remove", EditFileExclusion{guid, "\\Data", ExclusionAction::Remove}, Support::Supported},
      NativeCapabilityCase{"EditFileExclusion.Clear", EditFileExclusion{guid, {}, ExclusionAction::Clear}, Support::Unsupported},
      NativeCapabilityCase{"FindFileExclusion", FindFileExclusion{guid, "\\Data"}, Support::Unsupported},
      NativeCapabilityCase{"GetRegistryExclusions", GetRegistryExclusions{}, Support::Supported},
      NativeCapabilityCase{"EditRegistryExclusion.Add", EditRegistryExclusion{key, ExclusionAction::Add}, Support::Supported},
      NativeCapabilityCase{"EditRegistryExclusion.Remove", EditRegistryExclusion{key, ExclusionAction::Remove}, Support::Supported},
      NativeCapabilityCase{"EditRegistryExclusion.Clear", EditRegistryExclusion{{}, ExclusionAction::Clear}, Support::Unsupported},
      NativeCapabilityCase{"FindRegistryExclusion", FindRegistryExclusion{key}, Support::Unsupported},
      NativeCapabilityCase{"CommitFile", CommitFile{guid, "\\Data"}, Support::Supported},
      NativeCapabilityCase{"CommitFileDeletion", CommitFileDeletion{guid, "\\Data"}, Support::Supported},
      NativeCapabilityCase{"CommitRegistry", CommitRegistry{key, std::string{"Value"}}, Support::Supported},
      NativeCapabilityCase{"CommitRegistryDeletion", CommitRegistryDeletion{key, std::string{"Value"}}, Support::Supported},
      NativeCapabilityCase{"GetOverlayFiles", GetOverlayFiles{guid}, Support::Unsupported},
      NativeCapabilityCase{"GetSwapfileSize", GetSwapfileSize{guid}, Support::Supported},
      NativeCapabilityCase{"CreateSwapfile", CreateSwapfile{guid}, Support::Supported},
      NativeCapabilityCase{"ResetSettings", ResetSettings{}, Support::Supported},
      NativeCapabilityCase{"UpdateWindows", UpdateWindows{}, Support::Unsupported},
      NativeCapabilityCase{"Power.Shutdown", Power{PowerAction::Shutdown}, Support::Supported},
      NativeCapabilityCase{"Power.Restart", Power{PowerAction::Restart}, Support::Supported}};
  constexpr auto expected = std::variant_size_v<Request> + 2 * (static_cast<std::size_t>(BooleanProperty::Count) - 1) +
                            2 * (static_cast<std::size_t>(NumberProperty::Count) - 1) + (static_cast<std::size_t>(OverlayOption::Count) - 1) +
                            2 * (static_cast<std::size_t>(VolumeProperty::Count) - 1) + 2 * (static_cast<std::size_t>(ExclusionAction::Count) - 1) +
                            (static_cast<std::size_t>(PowerAction::Count) - 1);
  static_assert(cases.size() == expected);
  return cases;
}

struct NativeReadCase {
  const char* slot;
  Request request;
  Value currentValue;
  Value nextValue;
  std::vector<std::string> symbols;
  std::vector<bool> currentSessions;
  std::vector<bool> nextSessions;
  bool supported = true;
};
enum class NativeWriteFixture { Tscal, Critical, RuntimeReset, SavedMode, RemoveVolume, Protected, FileAdd, FileRemove, RegistryAdd, RegistryRemove };
struct NativeWriteWitness {
  const char* slot;
  Request request;
  NativeWriteFixture fixture;
  const char* symbol;
  std::vector<bool> booleans;
  std::vector<ULONG> numbers;
};
const auto& missingNativeWriteWitnesses() {
  static const std::array cases{
      NativeWriteWitness{
          "SetBoolean.PersistTscal", SetBoolean{BooleanProperty::PersistTscal, true}, NativeWriteFixture::Tscal, "UwfCfgSetPersistTSCAL", {true}, {}},
      NativeWriteWitness{"SetNumber.CriticalThreshold",
                         SetNumber{NumberProperty::CriticalThreshold, 113},
                         NativeWriteFixture::Critical,
                         "UwfCfgSetCriticalThreshold",
                         {},
                         {113}},
      NativeWriteWitness{"SetNumber.ResetPersistentState",
                         SetNumber{NumberProperty::ResetPersistentState, 1},
                         NativeWriteFixture::RuntimeReset,
                         "UwfCfgSetResetPersistentOverlay",
                         {},
                         {1}},
      NativeWriteWitness{"SetNumber.ResetPersistentStateSavedMode",
                         SetNumber{NumberProperty::ResetPersistentStateSavedMode, 1},
                         NativeWriteFixture::SavedMode,
                         "UwfCfgSetResetPersistentOverlaySavedMode",
                         {},
                         {1}},
      NativeWriteWitness{"RemoveVolume", RemoveVolume{wrappedGuid}, NativeWriteFixture::RemoveVolume, "UwfCfgRemoveVolume", {}, {}},
      NativeWriteWitness{"SetVolume.Protected",
                         SetVolume{wrappedGuid, VolumeProperty::Protected, true},
                         NativeWriteFixture::Protected,
                         "UwfCfgSetVolumeProtected",
                         {true},
                         {}},
      NativeWriteWitness{"EditFileExclusion.Add",
                         EditFileExclusion{wrappedGuid, "\\Data\\NativeLiteral", ExclusionAction::Add},
                         NativeWriteFixture::FileAdd,
                         "UwfCfgAddFileExclusion",
                         {},
                         {}},
      NativeWriteWitness{"EditFileExclusion.Remove",
                         EditFileExclusion{wrappedGuid, "\\Data\\NativeLiteral", ExclusionAction::Remove},
                         NativeWriteFixture::FileRemove,
                         "UwfCfgRemoveFileExclusion",
                         {},
                         {}},
      NativeWriteWitness{"EditRegistryExclusion.Add",
                         EditRegistryExclusion{"HKEY_LOCAL_MACHINE\\SOFTWARE\\NativeLiteral", ExclusionAction::Add},
                         NativeWriteFixture::RegistryAdd,
                         "UwfCfgAddRegExclusion",
                         {},
                         {}},
      NativeWriteWitness{"EditRegistryExclusion.Remove",
                         EditRegistryExclusion{"HKEY_LOCAL_MACHINE\\SOFTWARE\\NativeLiteral", ExclusionAction::Remove},
                         NativeWriteFixture::RegistryRemove,
                         "UwfCfgRemoveRegExclusion",
                         {},
                         {}}};
  static_assert(cases.size() == 10);
  return cases;
}
const auto& nativeReadCases() {
  static const std::array cases{
      NativeReadCase{"GetBoolean.FilterEnabled", GetBoolean{BooleanProperty::FilterEnabled}, true, false, {"UwfCfgGetFilterEnabled"}, {true}, {false}},
      NativeReadCase{"GetBoolean.ShutdownPending", GetBoolean{BooleanProperty::ShutdownPending}, true, true, {"UwfCfgGetShutdownPending"}, {}, {}},
      NativeReadCase{"GetBoolean.HormEnabled", GetBoolean{BooleanProperty::HormEnabled}, false, false, {"UwfCfgGetHormEnabled"}, {}, {}},
      NativeReadCase{
          "GetBoolean.ServicingEnabled", GetBoolean{BooleanProperty::ServicingEnabled}, false, true, {"UwfCfgGetServicingModeEnabled"}, {true}, {false}},
      NativeReadCase{"GetBoolean.PersistDomainSecretKey",
                     GetBoolean{BooleanProperty::PersistDomainSecretKey},
                     true,
                     false,
                     {"UwfCfgGetPersistDomainSecretKey"},
                     {true},
                     {false}},
      NativeReadCase{"GetBoolean.PersistTscal", GetBoolean{BooleanProperty::PersistTscal}, false, true, {"UwfCfgGetPersistTSCAL"}, {true}, {false}},
      NativeReadCase{"GetBoolean.OverlayCommit", GetBoolean{BooleanProperty::OverlayCommit}, true, true, {"UwfCfgGetOverlayCommitState"}, {}, {}},
      NativeReadCase{"GetBoolean.RegistryChangesAlwaysPersisted", GetBoolean{BooleanProperty::RegistryChangesAlwaysPersisted}, false, false, {}, {}, {}, false},
      NativeReadCase{
          "GetNumber.OverlayType", GetNumber{NumberProperty::OverlayType}, std::uint32_t{0}, std::uint32_t{1}, {"UwfCfgGetOverlayType"}, {true}, {false}},
      NativeReadCase{"GetNumber.MaximumOverlaySize",
                     GetNumber{NumberProperty::MaximumOverlaySize},
                     std::uint32_t{2048},
                     std::uint32_t{4096},
                     {"UwfCfgGetOverlayMaximumSize"},
                     {true},
                     {false}},
      NativeReadCase{"GetNumber.WarningThreshold",
                     GetNumber{NumberProperty::WarningThreshold},
                     std::uint32_t{33},
                     std::uint32_t{33},
                     {"UwfCfgGetWarningThreshold"},
                     {},
                     {}},
      NativeReadCase{"GetNumber.CriticalThreshold",
                     GetNumber{NumberProperty::CriticalThreshold},
                     std::uint32_t{44},
                     std::uint32_t{44},
                     {"UwfCfgGetCriticalThreshold"},
                     {},
                     {}},
      NativeReadCase{"GetNumber.OverlayFlags",
                     GetNumber{NumberProperty::OverlayFlags},
                     std::uint32_t{0xa5},
                     std::uint32_t{0x5a},
                     {"UwfCfgGetOverlayFlags"},
                     {true},
                     {false}},
      NativeReadCase{"GetNumber.ResetPersistentState",
                     GetNumber{NumberProperty::ResetPersistentState},
                     std::uint32_t{66},
                     std::uint32_t{66},
                     {"UwfCfgGetResetPersistentOverlay"},
                     {},
                     {}},
      NativeReadCase{"GetNumber.ResetPersistentStateSavedMode",
                     GetNumber{NumberProperty::ResetPersistentStateSavedMode},
                     std::uint32_t{77},
                     std::uint32_t{88},
                     {"UwfCfgGetResetPersistentOverlaySavedMode"},
                     {true},
                     {false}},
      NativeReadCase{"GetOverlayUsage", GetOverlayUsage{}, OverlayUsage{37, 987}, OverlayUsage{37, 987}, {"UwfCfgGetOverlayInformation"}, {}, {}},
      NativeReadCase{"GetVolumes",
                     GetVolumes{},
                     std::vector<Volume>{{guid, "C:", true, false, {}}},
                     std::vector<Volume>{{guid, "D:", false, true, {}}},
                     {"UwfCfgGetVolumes", "UwfCfgGetDriveLetter", "UwfCfgGetVolumeProtected", "UwfCfgGetVolumeBindByDriveLetter"},
                     {true, true, true, true},
                     {false, false, false, false}},
      NativeReadCase{
          "GetVolumeProperty.Protected", GetVolumeProperty{guid, VolumeProperty::Protected}, true, false, {"UwfCfgGetVolumeProtected"}, {true}, {false}},
      NativeReadCase{"GetVolumeProperty.BindByDriveLetter",
                     GetVolumeProperty{guid, VolumeProperty::BindByDriveLetter},
                     false,
                     true,
                     {"UwfCfgGetVolumeBindByDriveLetter"},
                     {true},
                     {false}},
      NativeReadCase{"GetFileExclusions",
                     GetFileExclusions{guid},
                     std::vector<std::string>{"\\Data\\current"},
                     std::vector<std::string>{"\\Data\\next"},
                     {"UwfCfgGetFileExclusions"},
                     {true},
                     {false}},
      NativeReadCase{"FindFileExclusion", FindFileExclusion{guid, "\\DATA\\CURRENT"}, true, false, {}, {}, {}, false},
      NativeReadCase{"GetRegistryExclusions",
                     GetRegistryExclusions{},
                     std::vector<std::string>{"HKLM\\SOFTWARE\\Current"},
                     std::vector<std::string>{"HKLM\\SOFTWARE\\Next"},
                     {"UwfCfgGetRegExclusions"},
                     {true},
                     {false}},
      NativeReadCase{"FindRegistryExclusion", FindRegistryExclusion{"hkLM\\software\\CURRENT"}, true, false, {}, {}, {}, false},
      NativeReadCase{"GetOverlayFiles", GetOverlayFiles{guid}, std::monostate{}, std::monostate{}, {}, {}, {}, false},
      NativeReadCase{"GetSwapfileSize", GetSwapfileSize{guid}, std::uint32_t{111}, std::uint32_t{222}, {"UwfCfgGetVolumeSwapfileSize"}, {true}, {false}}};
  static_assert(cases.size() == static_cast<std::size_t>(BooleanProperty::Count) + static_cast<std::size_t>(NumberProperty::Count) +
                                    static_cast<std::size_t>(VolumeProperty::Count) + 8);
  return cases;
}
}  // namespace

class NativeUwfBackendTests final : public QObject {
  Q_OBJECT
  enum class RegistryMaintenanceOperation { CommitValue, DeleteKey, DeleteValue };
  Q_ENUM(RegistryMaintenanceOperation)
 private slots:
  void primitiveWriteSlotsPreserveTheirLiteralNativeAbi_data() {
    QTest::addColumn<int>("caseIndex");
    for (std::size_t index = 0; index < missingNativeWriteWitnesses().size(); ++index)
      QTest::newRow(missingNativeWriteWitnesses()[index].slot) << static_cast<int>(index);
  }
  void primitiveWriteSlotsPreserveTheirLiteralNativeAbi() {
    QFETCH(int, caseIndex);
    const auto& entry = missingNativeWriteWitnesses()[static_cast<std::size_t>(caseIndex)];
    Symbols symbols;
    symbols.missingWriteWitnessSurface();
    symbols.state.boolean = false;
    symbols.state.currentBoolean = false;
    symbols.state.nextBoolean = false;
    symbols.state.number = 2048;
    symbols.state.overlayFlags = 0x81;
    symbols.state.protectedVolume = false;
    if (entry.fixture == NativeWriteFixture::RuntimeReset) {
      symbols.state.currentBoolean = true;
      symbols.state.overlayType = static_cast<ULONG>(OverlayType::Disk);
      symbols.state.overlayFlags = 3;  // Loaded Disk: Passthrough + Persistent.
    }
    if (entry.fixture == NativeWriteFixture::FileRemove) symbols.state.strings = {L"\\Data\\NativeLiteral"};
    if (entry.fixture == NativeWriteFixture::RegistryRemove) symbols.state.strings = {L"HKEY_LOCAL_MACHINE\\SOFTWARE\\NativeLiteral"};
    const NativeUwfBackend backend(symbols.bindings());
    const auto response = backend.execute(entry.request);
    QVERIFY(response.succeeded());
    QVERIFY(std::holds_alternative<std::monostate>(response.value));
    QCOMPARE(response.effect, Effect::Confirmed);
    QCOMPARE(symbols.state.writeSymbols, std::vector<std::string>{entry.symbol});
    QCOMPARE(symbols.state.writeBooleans, entry.booleans);
    QCOMPARE(symbols.state.writeNumbers, entry.numbers);
    QVERIFY(symbols.state.sessions.empty());
    QVERIFY(symbols.state.readSymbols.empty());
    const bool volumeArgument = entry.fixture == NativeWriteFixture::Protected || entry.fixture == NativeWriteFixture::RemoveVolume ||
                                entry.fixture == NativeWriteFixture::FileAdd || entry.fixture == NativeWriteFixture::FileRemove;
    QCOMPARE(symbols.state.writeVolumes, volumeArgument ? std::vector<std::wstring>{L"11111111-2222-3333-4444-555555555555"} : std::vector<std::wstring>{});
    const bool fileEntry = entry.fixture == NativeWriteFixture::FileAdd || entry.fixture == NativeWriteFixture::FileRemove;
    const bool registryEntry = entry.fixture == NativeWriteFixture::RegistryAdd || entry.fixture == NativeWriteFixture::RegistryRemove;
    const auto expectedText = fileEntry       ? std::vector<std::wstring>{L"\\Data\\NativeLiteral"}
                              : registryEntry ? std::vector<std::wstring>{L"HKEY_LOCAL_MACHINE\\SOFTWARE\\NativeLiteral"}
                                              : std::vector<std::wstring>{};
    QCOMPARE(symbols.state.writeTexts, expectedText);
    switch (entry.fixture) {
      case NativeWriteFixture::Tscal:
        QCOMPARE(symbols.state.literalTscalNext, true);
        break;
      case NativeWriteFixture::Critical:
        QCOMPARE(symbols.state.literalCritical, ULONG{113});
        break;
      case NativeWriteFixture::RuntimeReset:
        QCOMPARE(symbols.state.literalRuntimeReset, ULONG{1});
        break;
      case NativeWriteFixture::SavedMode:
        QCOMPARE(symbols.state.literalSavedModeNext, ULONG{1});
        break;
      case NativeWriteFixture::RemoveVolume:
        QCOMPARE(symbols.state.volumeExistsStatus, HRESULT_FROM_WIN32(ERROR_NOT_FOUND));
        break;
      case NativeWriteFixture::Protected:
        QCOMPARE(symbols.state.protectedVolume, std::optional<bool>{true});
        break;
      case NativeWriteFixture::FileAdd:
        QCOMPARE(symbols.state.strings, std::vector<std::wstring>{L"\\Data\\NativeLiteral"});
        break;
      case NativeWriteFixture::RegistryAdd:
        QCOMPARE(symbols.state.strings, std::vector<std::wstring>{L"HKEY_LOCAL_MACHINE\\SOFTWARE\\NativeLiteral"});
        break;
      case NativeWriteFixture::FileRemove:
      case NativeWriteFixture::RegistryRemove:
        QVERIFY(symbols.state.strings.empty());
        break;
    }
    QCOMPARE(symbols.state.calls, 1);
    QCOMPARE(symbols.state.postWriteReads, 0);
    QVERIFY(symbols.state.dispatchedWrite);
    QVERIFY(symbols.state.releaseMatched);
    QVERIFY(symbols.state.arrayBuffers.empty());
    QVERIFY(symbols.state.driveBuffers.empty());
  }
  void unsupportedWriteSlotsExecuteWithoutEnteringAnyNativeExport_data() {
    QTest::addColumn<int>("caseIndex");
    constexpr std::array slotNames{"SetBoolean.ShutdownPending", "SetBoolean.ServicingEnabled", "SetBoolean.RegistryChangesAlwaysPersisted", "UpdateWindows"};
    for (std::size_t index = 0; index < slotNames.size(); ++index) QTest::newRow(slotNames[index]) << static_cast<int>(index);
  }
  void unsupportedWriteSlotsExecuteWithoutEnteringAnyNativeExport() {
    QFETCH(int, caseIndex);
    const std::array requests{Request{SetBoolean{BooleanProperty::ShutdownPending, true}}, Request{SetBoolean{BooleanProperty::ServicingEnabled, true}},
                              Request{SetBoolean{BooleanProperty::RegistryChangesAlwaysPersisted, true}}, Request{UpdateWindows{}}};
    Symbols symbols;
    symbols.completeExportSurface();
    const NativeUwfBackend backend(symbols.bindings());
    const auto response = backend.execute(requests[static_cast<std::size_t>(caseIndex)]);
    QVERIFY(response.failure);
    QCOMPARE(response.failure->kind, ErrorKind::Unsupported);
    QCOMPARE(response.effect, Effect::None);
    QCOMPARE(symbols.state.calls, 0);
    QVERIFY(!symbols.state.dispatchedWrite);
  }
  void everyReadSlotPreservesLiteralValuesAndPhysicalSessionBinding_data() {
    QTest::addColumn<int>("caseIndex");
    QTest::addColumn<bool>("current");
    int rows = 0;
    for (std::size_t index = 0; index < nativeReadCases().size(); ++index) {
      const auto& entry = nativeReadCases()[index];
      const bool scoped = std::visit([](const auto& request) { return requires { request.session; }; }, entry.request);
      for (const bool current : {true, false}) {
        if (!scoped && !current) continue;
        const auto name = QStringLiteral("%1/%2").arg(QString::fromLatin1(entry.slot),
                                                      scoped ? current ? QStringLiteral("Current") : QStringLiteral("Next") : QStringLiteral("fixed-scope"));
        QTest::newRow(qUtf8Printable(name)) << static_cast<int>(index) << current;
        ++rows;
      }
    }
    QCOMPARE(rows, 48);
  }
  void everyReadSlotPreservesLiteralValuesAndPhysicalSessionBinding() {
    QFETCH(int, caseIndex);
    QFETCH(bool, current);
    const auto& entry = nativeReadCases()[static_cast<std::size_t>(caseIndex)];
    auto request = entry.request;
    std::visit(
        [&](auto& operation) {
          if constexpr (requires { operation.session; }) operation.session = current ? Session::Current : Session::Next;
        },
        request);
    Symbols symbols;
    symbols.literalReadSurface();
    const NativeUwfBackend backend(symbols.bindings());
    const auto response = backend.execute(request);
    QCOMPARE(response.succeeded(), entry.supported);
    QCOMPARE(response.effect, Effect::None);
    if (entry.supported) {
      QVERIFY(response.value == (current ? entry.currentValue : entry.nextValue));
      QVERIFY(!response.failure);
    } else {
      QVERIFY(response.failure);
      QCOMPARE(response.failure->kind, ErrorKind::Unsupported);
    }
    QCOMPARE(symbols.state.readSymbols, entry.symbols);
    QCOMPARE(symbols.state.sessions, current ? entry.currentSessions : entry.nextSessions);
    for (const auto& volume : symbols.state.volumes) QCOMPARE(volume, uwf::windows::utf8ToWide(guid));
    QVERIFY(symbols.state.releaseMatched);
    QVERIFY(symbols.state.arrayBuffers.empty());
    QVERIFY(symbols.state.driveBuffers.empty());
    QVERIFY(!symbols.state.dispatchedWrite);
  }
  void everyAbstractSlotHasAnExplicitNativeCapability_data() {
    QTest::addColumn<int>("caseIndex");
    QTest::addColumn<bool>("supportsTscalObservation");
    QTest::addColumn<bool>("current");
    int rows = 0;
    for (std::size_t index = 0; index < nativeCapabilityCases().size(); ++index) {
      const auto& entry = nativeCapabilityCases()[index];
      const bool scoped = std::visit([](const auto& request) { return requires { request.session; }; }, entry.request);
      for (const bool supportsTscalObservation : {false, true}) {
        for (const bool current : {true, false}) {
          if (!scoped && !current) continue;
          const auto name = QStringLiteral("%1/tscal-%2/%3")
                                .arg(QString::fromLatin1(entry.slot))
                                .arg(supportsTscalObservation ? QStringLiteral("supported") : QStringLiteral("guarded"))
                                .arg(scoped ? current ? QStringLiteral("Current") : QStringLiteral("Next") : QStringLiteral("fixed-scope"));
          QTest::newRow(qUtf8Printable(name)) << static_cast<int>(index) << supportsTscalObservation << current;
          ++rows;
        }
      }
    }
    QCOMPARE(rows, 170);
  }
  void everyAbstractSlotHasAnExplicitNativeCapability() {
    QFETCH(int, caseIndex);
    QFETCH(bool, supportsTscalObservation);
    QFETCH(bool, current);
    const auto& entry = nativeCapabilityCases()[static_cast<std::size_t>(caseIndex)];
    auto request = entry.request;
    std::visit(
        [&](auto& operation) {
          if constexpr (requires { operation.session; }) operation.session = current ? Session::Current : Session::Next;
        },
        request);
    Symbols symbols;
    symbols.completeExportSurface();
    auto bindings = symbols.bindings();
    bindings.supportsTscalObservation = supportsTscalObservation;
    const NativeUwfBackend backend(std::move(bindings));
    const auto capability = backend.probe(request);
    const auto expected = entry.requiresTscalObservation && !supportsTscalObservation ? Support::Unsupported : entry.support;
    QCOMPARE(capability.support, expected);
    if (expected == Support::Unsupported || expected == Support::Unknown) QVERIFY(!capability.reason.empty());
    QCOMPARE(symbols.state.calls, 0);
    QVERIFY(!symbols.state.dispatchedWrite);
  }
  void aStaticServicingFlagCannotCompleteTheServicingContract() {
    Symbols symbols;
    symbols.add("UwfCfgGetServicingModeEnabled", getSessionBoolean);
    symbols.add("UwfCfgSetServicingModeEnabled", setBoolean);
    const NativeUwfBackend backend(symbols.bindings());
    for (const bool enabled : {false, true}) {
      const Request request = SetBoolean{BooleanProperty::ServicingEnabled, enabled};
      QCOMPARE(backend.probe(request).support, Support::Unsupported);
      const auto response = backend.execute(request);
      QVERIFY(response.failure);
      QCOMPARE(response.failure->kind, ErrorKind::Unsupported);
      QCOMPARE(response.effect, Effect::None);
    }
    QCOMPARE(symbols.state.calls, 0);
    QVERIFY(!symbols.state.dispatchedWrite);
  }
  void composedRequestsRemainOutsideRawNativeEntryPoints() {
    Symbols symbols;
    symbols.completeExportSurface();
    const NativeUwfBackend backend(symbols.bindings());
    const std::array<Request, 8> requests{GetBoolean{BooleanProperty::RegistryChangesAlwaysPersisted},
                                          FindFileExclusion{guid, "\\Data"},
                                          FindRegistryExclusion{"HKEY_LOCAL_MACHINE\\Software\\Entry"},
                                          EditFileExclusion{guid, {}, ExclusionAction::Clear},
                                          EditRegistryExclusion{{}, ExclusionAction::Clear},
                                          SetOverlayOption{OverlayOption::Passthrough, true},
                                          SetOverlayOption{OverlayOption::Persistent, true},
                                          SetOverlayOption{OverlayOption::ReadOnlyMedia, true}};
    for (const auto& request : requests) {
      QCOMPARE(backend.probe(request).support, Support::Unsupported);
      const auto response = backend.execute(request);
      QVERIFY(response.failure);
      QCOMPARE(response.failure->kind, ErrorKind::Unsupported);
      QCOMPARE(response.effect, Effect::None);
    }
    QCOMPARE(symbols.state.calls, 0);
    QCOMPARE(symbols.state.arrayReleases, 0);
    QVERIFY(!symbols.state.dispatchedWrite);
  }
  void exportCapabilitiesAreIndependentAndNeverCallDuringProbe() {
    Symbols symbols;
    symbols.add("UwfCfgGetFilterEnabled", getSessionBoolean);
    NativeUwfBackend backend(symbols.bindings());
    QCOMPARE(backend.name(), std::string{"Native"});
    QCOMPARE(backend.probe(GetBoolean{BooleanProperty::FilterEnabled}).support, Support::Supported);
    QCOMPARE(backend.probe(SetBoolean{BooleanProperty::FilterEnabled, true}).support, Support::Unsupported);
    QCOMPARE(backend.probe(GetOverlayFiles{guid}).support, Support::Unsupported);
    QCOMPARE(backend.probe(UpdateWindows{}).support, Support::Unsupported);
    QCOMPARE(symbols.state.calls, 0);
    const auto resolved = symbols.resolutions;
    QVERIFY(call(backend, GetBoolean{BooleanProperty::FilterEnabled}));
    QVERIFY(call(backend, GetBoolean{BooleanProperty::FilterEnabled, Session::Next}));
    QCOMPARE(symbols.resolutions, resolved);
    QCOMPARE(symbols.resolutions.at("UwfCfgGetFilterEnabled"), 1);
    NativeUwfBackend missing(
        NativeUwfBindings{.resolveExport = {}, .resolveImport = {}, .unavailableReason = "Missing module", .unavailableCode = ERROR_MOD_NOT_FOUND});
    QCOMPARE(missing.name(), std::string{"Native"});
    const auto failure = missing.execute(GetOverlayUsage{});
    QVERIFY(failure.failure);
    QCOMPARE(failure.failure->kind, ErrorKind::Unavailable);
    QCOMPARE(failure.effect, Effect::None);
    QCOMPARE(failure.failure->diagnostics.front().code, std::int64_t{ERROR_MOD_NOT_FOUND});
    for (const auto& diagnostic : failure.failure->diagnostics) QCOMPARE(diagnostic.backend, std::string{"Native"});
  }

  void scalarVolumeQueriesAndSharedConfigurationDoNotNeedArrayAbi() {
    Symbols symbols;
    symbols.add("UwfCfgGetVolumeProtected", getVolumeBoolean);
    symbols.add("UwfCfgGetVolumeBindByDriveLetter", getVolumeBoolean);
    symbols.add("UwfCfgDoesVolumeExist", doesVolumeExist);
    symbols.add("UwfCfgGetFilterEnabled", getSessionBoolean);
    symbols.add("UwfCfgGetHormEnabled", getHorm);
    symbols.add("UwfCfgGetOverlayFlags", getOverlayFlags);
    symbols.add("UwfCfgSetOverlayFlags", setOverlayFlags);
    symbols.add("UwfCfgSetServicingModeEnabled", setBoolean);
    NativeUwfBackend backend(symbols.bindings());
    QVERIFY(call(backend, GetVolumeProperty{wrappedGuid, VolumeProperty::Protected, Session::Current}));
    QVERIFY(call(backend, GetVolumeProperty{wrappedGuid, VolumeProperty::BindByDriveLetter, Session::Next}));
    QCOMPARE(symbols.state.sessions, std::vector<bool>({true, false}));
    QCOMPARE(backend.probe(SetNumber{NumberProperty::OverlayFlags, 0}).support, Support::Supported);
    QCOMPARE(backend.probe(SetBoolean{BooleanProperty::ServicingEnabled, true}).support, Support::Unsupported);
    symbols.state.boolean = false;
    const auto previous = symbols.state.number;
    QCOMPARE(backend.probe(SetOverlayOption{OverlayOption::Passthrough, true}).support, Support::Unsupported);
    QCOMPARE(symbols.state.number, previous);
  }
  void volumePropertyFailuresPreserveTheGetterStatus_data() {
    QTest::addColumn<bool>("current");
    QTest::addColumn<int>("property");
    QTest::addColumn<qint32>("propertyStatus");
    QTest::addColumn<int>("kind");
    const std::array cases{
        std::pair{HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND), ErrorKind::NotFound}, std::pair{HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND), ErrorKind::NotFound},
        std::pair{HRESULT_FROM_WIN32(ERROR_NOT_FOUND), ErrorKind::NotFound},      std::pair{E_ACCESSDENIED, ErrorKind::AccessDenied},
        std::pair{HRESULT_FROM_WIN32(ERROR_INVALID_DATA), ErrorKind::Protocol},   std::pair{HRESULT_FROM_WIN32(ERROR_DATATYPE_MISMATCH), ErrorKind::Protocol}};
    for (const bool current : {false, true})
      for (const auto property : {VolumeProperty::Protected, VolumeProperty::BindByDriveLetter})
        for (const auto& [status, kind] : cases)
          QTest::newRow(qUtf8Printable(QStringLiteral("%1/%2/%3").arg(current).arg(int(property)).arg(quint32(status))))
              << current << int(property) << qint32(status) << int(kind);
  }
  void volumePropertyFailuresPreserveTheGetterStatus() {
    QFETCH(bool, current);
    QFETCH(int, property);
    QFETCH(qint32, propertyStatus);
    QFETCH(int, kind);
    Symbols symbols;
    symbols.state.status = propertyStatus;
    symbols.add("UwfCfgGetVolumeProtected", getVolumeBoolean);
    symbols.add("UwfCfgGetVolumeBindByDriveLetter", getVolumeBoolean);
    const NativeUwfBackend backend(symbols.bindings());
    const GetVolumeProperty request{wrappedGuid, static_cast<VolumeProperty>(property), current ? Session::Current : Session::Next};
    QCOMPARE(backend.probe(request).support, Support::Supported);
    const auto response = backend.execute(request);
    QVERIFY(response.failure);
    QCOMPARE(response.failure->kind, static_cast<ErrorKind>(kind));
    QCOMPARE(response.effect, Effect::None);
    QCOMPARE(response.failure->diagnostics.front().code, std::int64_t{propertyStatus});
    QCOMPARE(symbols.state.calls, 1);
    QCOMPARE(symbols.state.sessions, std::vector<bool>{current});
    QCOMPARE(symbols.state.volumes, std::vector<std::wstring>{uwf::windows::utf8ToWide(normalizeVolumeId(guid))});
    QVERIFY(!symbols.state.dispatchedWrite);
  }
  void volumeRemovalDoesNotReadAnUnrelatedPropertyForConfirmation() {
    Symbols symbols;
    symbols.state.observationStatus = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    symbols.add("UwfCfgRemoveVolume", acknowledgeVolumeRemoval);
    symbols.add("UwfCfgGetVolumeProtected", getVolumeBoolean);
    symbols.add("UwfCfgDoesVolumeExist", doesVolumeExist);
    const auto response = NativeUwfBackend(symbols.bindings()).execute(RemoveVolume{guid});
    QVERIFY(response.succeeded());
    QCOMPARE(response.effect, Effect::Confirmed);
    QCOMPARE(symbols.state.calls, 1);
    QCOMPARE(symbols.state.postWriteReads, 0);
    QVERIFY(symbols.state.sessions.empty());
  }
  void capturedFilterSessionVectorsUseRealNativeExports() {
    Symbols symbols;
    symbols.state.currentBoolean = false;
    symbols.state.nextBoolean = true;
    symbols.add("UwfCfgGetFilterEnabled", getSessionBoolean);
    const NativeUwfBackend backend(symbols.bindings());
    for (const auto& vector : uwf::test::captured::filterSessionReads()) {
      const auto response = backend.execute(vector.request);
      QVERIFY2(response.succeeded(), vector.id);
      QCOMPARE(response.effect, vector.expected.effect);
      QVERIFY(vector.expected.value);
      QCOMPARE(std::get<bool>(response.value), std::get<bool>(*vector.expected.value));
    }
    QCOMPARE(symbols.state.sessions, std::vector<bool>({true, false}));
  }
  void tscalGetterIsGuardedByDefaultWithoutBlockingSetter_data() {
    QTest::addColumn<int>("operation");
    for (int operation = 0; operation < 3; ++operation) QTest::newRow(qPrintable(QStringLiteral("operation-%1").arg(operation))) << operation;
  }
  void tscalGetterIsGuardedByDefaultWithoutBlockingSetter() {
    QFETCH(int, operation);
    Symbols symbols;
    symbols.add("UwfCfgGetPersistTSCAL", getSessionBoolean);
    symbols.add("UwfCfgSetPersistTSCAL", setBoolean);
    symbols.add("UwfCfgGetFilterEnabled", getSessionBoolean);
    NativeUwfBindings bindings;
    bindings.resolveExport = symbols.bindings().resolveExport;
    const NativeUwfBackend backend(std::move(bindings));
    const std::array<Request, 3> requests{GetBoolean{BooleanProperty::PersistTscal, Session::Current}, GetBoolean{BooleanProperty::PersistTscal, Session::Next},
                                          SetBoolean{BooleanProperty::PersistTscal, true}};
    const auto& request = requests[static_cast<std::size_t>(operation)];
    QCOMPARE(backend.probe(request).support, operation == 2 ? Support::Supported : Support::Unsupported);
    const auto response = backend.execute(request);
    QCOMPARE(response.succeeded(), operation == 2);
    if (response.failure) QCOMPARE(response.failure->kind, ErrorKind::Unsupported);
    QCOMPARE(response.effect, operation == 2 ? Effect::Confirmed : Effect::None);
    QCOMPARE(symbols.state.calls, operation == 2 ? 1 : 0);
    QCOMPARE(symbols.state.postWriteReads, 0);
    QCOMPARE(symbols.state.dispatchedWrite, operation == 2);
  }
  void tscalObservationFallsBackAsACompleteRequest_data() {
    QTest::addColumn<bool>("current");
    QTest::newRow("current") << true;
    QTest::newRow("next") << false;
  }
  void tscalObservationFallsBackAsACompleteRequest() {
    QFETCH(bool, current);
    Symbols guarded;
    guarded.add("UwfCfgGetPersistTSCAL", getSessionBoolean);
    guarded.add("UwfCfgGetFilterEnabled", getSessionBoolean);
    NativeUwfBindings guardedBindings;
    guardedBindings.resolveExport = guarded.bindings().resolveExport;
    auto first = std::make_shared<NativeUwfBackend>(std::move(guardedBindings));
    Symbols complete;
    complete.state.currentBoolean = false;
    complete.state.nextBoolean = true;
    complete.add("UwfCfgGetPersistTSCAL", getSessionBoolean);
    complete.add("UwfCfgGetFilterEnabled", getSessionBoolean);
    auto second = std::make_shared<NativeUwfBackend>(complete.bindings());
    const auto response = FallbackUwf({first, second}).execute(GetBoolean{BooleanProperty::PersistTscal, current ? Session::Current : Session::Next});
    QVERIFY(response.succeeded());
    QCOMPARE(response.attempts.size(), std::size_t{2});
    QCOMPARE(response.attempts.front().capability.support, Support::Unsupported);
    QCOMPARE(response.effect, Effect::None);
    QCOMPARE(guarded.state.calls, 0);
    QCOMPARE(complete.state.calls, 1);
    QCOMPARE(std::get<bool>(response.value), !current);
    QCOMPARE(complete.state.sessions, std::vector<bool>{current});
  }
  void usagePreservesCliZeroInitialization_data() {
    QTest::addColumn<int>("outputMask");
    QTest::addColumn<bool>("failed");
    for (int mask = 0; mask < 4; ++mask)
      for (const bool failed : {false, true})
        QTest::newRow(qPrintable(QStringLiteral("output-mask-%1/%2").arg(mask).arg(failed ? "failed" : "success"))) << mask << failed;
  }
  void usagePreservesCliZeroInitialization() {
    QFETCH(int, outputMask);
    QFETCH(bool, failed);
    Symbols symbols;
    symbols.state.usageOutputMask = outputMask;
    symbols.state.status = failed ? E_ACCESSDENIED : S_OK;
    symbols.add("UwfCfgGetOverlayInformation", getUsage);
    const NativeUwfBackend backend(symbols.bindings());
    QCOMPARE(backend.probe(GetOverlayUsage{}).support, Support::Supported);
    const auto response = backend.execute(GetOverlayUsage{});
    QCOMPARE(response.succeeded(), !failed);
    QCOMPARE(response.effect, Effect::None);
    QCOMPARE(symbols.state.calls, 1);
    QVERIFY(!symbols.state.dispatchedWrite);
    if (failed) {
      QVERIFY(response.failure);
      QCOMPARE(response.failure->kind, ErrorKind::AccessDenied);
      QVERIFY(std::holds_alternative<std::monostate>(response.value));
    } else {
      const OverlayUsage expected{outputMask & 1 ? symbols.state.usageConsumption : 0U, outputMask & 2 ? symbols.state.usageAvailable : 0U};
      QCOMPARE(std::get<OverlayUsage>(response.value), expected);
    }
  }
  void completeInjectedUsageCanLegitimatelyBeZero() {
    Symbols symbols;
    symbols.state.usageConsumption = 0;
    symbols.state.usageAvailable = 0;
    symbols.add("UwfCfgGetOverlayInformation", getUsage);
    symbols.add("UwfCfgGetFilterEnabled", getSessionBoolean);
    const auto response = NativeUwfBackend(symbols.bindings()).execute(GetOverlayUsage{});
    QVERIFY(response.succeeded());
    QCOMPARE(std::get<OverlayUsage>(response.value), (OverlayUsage{0, 0}));
    QCOMPARE(symbols.state.calls, 1);
  }
  void getterShapesPreserveSessionAndGlobalScopes() {
    Symbols symbols;
    const std::array scopedBooleans{std::pair{BooleanProperty::FilterEnabled, "UwfCfgGetFilterEnabled"},
                                    std::pair{BooleanProperty::ServicingEnabled, "UwfCfgGetServicingModeEnabled"},
                                    std::pair{BooleanProperty::PersistDomainSecretKey, "UwfCfgGetPersistDomainSecretKey"},
                                    std::pair{BooleanProperty::PersistTscal, "UwfCfgGetPersistTSCAL"}};
    const std::array globalBooleans{std::pair{BooleanProperty::ShutdownPending, "UwfCfgGetShutdownPending"},
                                    std::pair{BooleanProperty::HormEnabled, "UwfCfgGetHormEnabled"},
                                    std::pair{BooleanProperty::OverlayCommit, "UwfCfgGetOverlayCommitState"}};
    const std::array scopedNumbers{std::pair{NumberProperty::OverlayType, "UwfCfgGetOverlayType"},
                                   std::pair{NumberProperty::MaximumOverlaySize, "UwfCfgGetOverlayMaximumSize"},
                                   std::pair{NumberProperty::OverlayFlags, "UwfCfgGetOverlayFlags"},
                                   std::pair{NumberProperty::ResetPersistentStateSavedMode, "UwfCfgGetResetPersistentOverlaySavedMode"}};
    const std::array globalNumbers{std::pair{NumberProperty::WarningThreshold, "UwfCfgGetWarningThreshold"},
                                   std::pair{NumberProperty::CriticalThreshold, "UwfCfgGetCriticalThreshold"},
                                   std::pair{NumberProperty::ResetPersistentState, "UwfCfgGetResetPersistentOverlay"}};
    for (const auto& [property, symbol] : scopedBooleans) {
      static_cast<void>(property);
      symbols.add(symbol, getSessionBoolean);
    }
    for (const auto& [property, symbol] : globalBooleans) {
      static_cast<void>(property);
      symbols.add(symbol, getGlobalBoolean);
    }
    for (const auto& [property, symbol] : scopedNumbers) {
      static_cast<void>(property);
      symbols.add(symbol, getSessionNumber);
    }
    for (const auto& [property, symbol] : globalNumbers) {
      static_cast<void>(property);
      symbols.add(symbol, getGlobalNumber);
    }
    symbols.add("UwfCfgGetOverlayInformation", getUsage);
    NativeUwfBackend backend(symbols.bindings());
    const auto checkSession = [&symbols] {
      QCOMPARE(symbols.state.sessions, std::vector<bool>({true, false}));
      symbols.state.sessions.clear();
    };
    for (const auto& [property, symbol] : scopedBooleans) {
      static_cast<void>(symbol);
      for (const auto session : {Session::Current, Session::Next}) QVERIFY(call(backend, GetBoolean{property, session}));
      checkSession();
    }
    for (const auto& [property, symbol] : scopedNumbers) {
      static_cast<void>(symbol);
      for (const auto session : {Session::Current, Session::Next}) QCOMPARE(call(backend, GetNumber{property, session}), unsignedValue);
      checkSession();
    }
    symbols.state.boolean = false;
    for (const auto session : {Session::Current, Session::Next}) {
      for (const auto& [property, symbol] : globalBooleans) {
        static_cast<void>(symbol);
        QVERIFY(!call(backend, GetBoolean{property, session}));
      }
      for (const auto& [property, symbol] : globalNumbers) {
        static_cast<void>(symbol);
        QCOMPARE(call(backend, GetNumber{property, session}), unsignedValue);
      }
    }
    QVERIFY(symbols.state.sessions.empty());
    symbols.state.sessions.clear();
    symbols.state.boolean = true;
    const auto usage = call(backend, GetOverlayUsage{});
    QCOMPARE(usage.consumptionMb, std::uint32_t{37});
    QCOMPARE(usage.availableMb, std::uint32_t{987});
  }

  void nativeCallBoundariesUseTheSharedWindowsFailureTaxonomy_data() {
    QTest::addColumn<int>("boundary");
    QTest::addColumn<int>("failureIndex");
    constexpr std::array boundaries{"boolean-read", "number-read", "array-read", "opaque-write"};
    constexpr std::array failures{"invalid-data", "wrong-type", "busy", "disk-full", "unsupported", "rpc-unavailable", "invalid-environment", "abort"};
    for (std::size_t boundary = 0; boundary < boundaries.size(); ++boundary)
      for (std::size_t failure = 0; failure < failures.size(); ++failure)
        QTest::newRow(qUtf8Printable(QStringLiteral("%1/%2").arg(QString::fromLatin1(boundaries[boundary]), QString::fromLatin1(failures[failure]))))
            << static_cast<int>(boundary) << static_cast<int>(failure);
  }
  void nativeCallBoundariesUseTheSharedWindowsFailureTaxonomy() {
    QFETCH(int, boundary);
    QFETCH(int, failureIndex);
    struct FailureCase {
      HRESULT code;
      ErrorKind readKind;
      ErrorKind writeKind;
    };
    const std::array failures{FailureCase{HRESULT_FROM_WIN32(ERROR_INVALID_DATA), ErrorKind::Protocol, ErrorKind::Failed},
                              FailureCase{HRESULT_FROM_WIN32(ERROR_DATATYPE_MISMATCH), ErrorKind::Protocol, ErrorKind::Failed},
                              FailureCase{HRESULT_FROM_WIN32(ERROR_BUSY), ErrorKind::Conflict, ErrorKind::Conflict},
                              FailureCase{HRESULT_FROM_WIN32(ERROR_DISK_FULL), ErrorKind::ResourceExhausted, ErrorKind::ResourceExhausted},
                              FailureCase{HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED), ErrorKind::Unsupported, ErrorKind::Unsupported},
                              FailureCase{HRESULT_FROM_WIN32(RPC_S_SERVER_UNAVAILABLE), ErrorKind::Unavailable, ErrorKind::Unavailable},
                              FailureCase{HRESULT_FROM_WIN32(ERROR_INVALID_ENVIRONMENT), ErrorKind::Conflict, ErrorKind::Conflict},
                              FailureCase{E_ABORT, ErrorKind::Canceled, ErrorKind::Canceled}};
    const auto& failure = failures[static_cast<std::size_t>(failureIndex)];
    Symbols symbols;
    symbols.arrays();
    symbols.add("UwfCfgGetFilterEnabled", getSessionBoolean);
    symbols.add("UwfCfgGetWarningThreshold", getGlobalNumber);
    symbols.add("UwfCfgGetPersistDomainSecretKey", getSessionBoolean);
    symbols.add("UwfCfgSetPersistDomainSecretKey", setBoolean);
    symbols.state.strings = {L"HKLM\\SOFTWARE\\Observed"};
    const bool write = boundary == 3;
    if (write)
      symbols.state.writeStatus = failure.code;
    else if (boundary == 2)
      symbols.state.arrayStatus = failure.code;
    else
      symbols.state.status = failure.code;
    const std::array requests{Request{GetBoolean{BooleanProperty::FilterEnabled}}, Request{GetNumber{NumberProperty::WarningThreshold}},
                              Request{GetRegistryExclusions{}}, Request{SetBoolean{BooleanProperty::PersistDomainSecretKey, true}}};
    const NativeUwfBackend backend(symbols.bindings());
    QCOMPARE(backend.name(), std::string{"Native"});
    const auto response = backend.execute(requests[static_cast<std::size_t>(boundary)]);
    QVERIFY(response.failure);
    QCOMPARE(response.failure->kind, write ? failure.writeKind : failure.readKind);
    QCOMPARE(response.effect, write ? Effect::Uncertain : Effect::None);
    QVERIFY(!response.failure->diagnostics.empty());
    QCOMPARE(response.failure->diagnostics.back().code, static_cast<std::int64_t>(failure.code));
    for (const auto& diagnostic : response.failure->diagnostics) QCOMPARE(diagnostic.backend, std::string{"Native"});
    QCOMPARE(symbols.state.dispatchedWrite, write);
    QVERIFY(symbols.state.releaseMatched);
    QVERIFY(symbols.state.arrayBuffers.empty());
  }
  void swapfileAllocationFailureUsesTheSameTypedReceiptAsOtherNativeCalls() {
    Symbols symbols;
    symbols.completeExportSurface();
    symbols.state.boolean = false;
    symbols.state.protectedVolume = false;
    symbols.state.number = 2048;
    symbols.state.swapAllocationFailure = true;
    const NativeUwfBackend backend(symbols.bindings());
    const auto response = backend.execute(CreateSwapfile{guid});
    QVERIFY(response.failure);
    QCOMPARE(response.failure->kind, ErrorKind::ResourceExhausted);
    QCOMPARE(response.effect, Effect::Uncertain);
    QCOMPARE(symbols.state.swapWriteAttempts, 1);
    QCOMPARE(symbols.state.overlayType, static_cast<ULONG>(OverlayType::Ram));
    QCOMPARE(symbols.state.number, ULONG{2048});
    QCOMPARE(symbols.state.postWriteReads, 0);
  }

  void failuresKeepNativeStatusAndPossibleWriteEffects() {
    Symbols symbols;
    symbols.add("UwfCfgGetHormEnabled", getHorm);
    symbols.add("UwfCfgGetFilterEnabled", getSessionBoolean);
    symbols.add("UwfCfgGetOverlayCommitState", getGlobalBoolean);
    symbols.add("UwfCfgGetOverlayFlags", getOverlayFlags);
    symbols.add("UwfCfgGetVolumeProtected", getVolumeBoolean);
    symbols.add("UwfCfgDoesVolumeExist", doesVolumeExist);
    for (const auto* symbol : {"UwfCfgSetFilterEnabled", "UwfCfgSetOverlayCommitState"}) symbols.add(symbol, setBoolean);
    symbols.add("UwfCfgSetHormEnabled", setHorm);
    symbols.add("UwfCfgSetOverlayFlags", setOverlayFlags);
    symbols.add("UwfCfgSetVolumeSwapfileSize", setSwapfile);
    symbols.add("UwfCfgGetVolumeSwapfileSize", getSwapfile);
    symbols.add("UwfCfgGetOverlayType", getOverlayType);
    symbols.add("UwfCfgGetOverlayMaximumSize", getSessionNumber);
    symbols.add("UwfCfgResetSettings", noArguments);
    symbols.add("UwfCfgRestartSystem", noArguments);
    NativeUwfBackend backend(symbols.bindings());
    QCOMPARE(backend.name(), std::string{"Native"});
    const std::array writes{Request{SetBoolean{BooleanProperty::FilterEnabled, true}},
                            Request{SetBoolean{BooleanProperty::HormEnabled, true}},
                            Request{SetBoolean{BooleanProperty::OverlayCommit, true}},
                            Request{CreateSwapfile{wrappedGuid}},
                            Request{ResetSettings{}},
                            Request{Power{PowerAction::Restart}}};
    for (const auto status : {S_OK, E_ACCESSDENIED}) {
      symbols.state.writeStatus = status;
      for (const auto& request : writes) {
        const auto* boolean = std::get_if<SetBoolean>(&request);
        symbols.state.boolean = boolean && boolean->property == BooleanProperty::HormEnabled;
        symbols.state.horm = false;
        symbols.state.currentBoolean = boolean && (boolean->property == BooleanProperty::HormEnabled || boolean->property == BooleanProperty::OverlayCommit);
        symbols.state.overlayFlags = boolean && boolean->property == BooleanProperty::HormEnabled ? flags::readOnlyMedia : 0;
        symbols.state.overlayType = static_cast<ULONG>(OverlayType::Ram);
        symbols.state.number = 2048;
        const auto result = backend.execute(request);
        QCOMPARE(result.succeeded(), status == S_OK);
        QCOMPARE(result.effect, status == S_OK ? Effect::Confirmed : Effect::Uncertain);
        if (result.failure) {
          QCOMPARE(result.failure->kind, ErrorKind::AccessDenied);
          QCOMPARE(result.failure->diagnostics.front().code, static_cast<std::int64_t>(status));
          for (const auto& diagnostic : result.failure->diagnostics) QCOMPARE(diagnostic.backend, std::string{"Native"});
          QVERIFY(result.failure->diagnostics.front().operation.starts_with("UwfCfg"));
        } else
          QVERIFY(std::holds_alternative<std::monostate>(result.value));
      }
    }
    symbols.state.status = E_ACCESSDENIED;
    const auto read = backend.execute(GetBoolean{BooleanProperty::HormEnabled});
    QVERIFY(read.failure);
    QCOMPARE(read.effect, Effect::None);
    std::stop_source stop;
    stop.request_stop();
    const auto before = symbols.state.calls;
    const auto canceled = backend.execute(writes.front(), {stop.get_token()});
    QCOMPARE(canceled.failure->kind, ErrorKind::Canceled);
    QCOMPARE(canceled.effect, Effect::None);
    QCOMPARE(symbols.state.calls, before);
  }

  void configurationWriteReceiptsDoNotIssueStateQueries_data() {
    QTest::addColumn<int>("operation");
    QTest::addColumn<int>("fault");
    const std::array names{"filter", "HORM", "domain-persistence", "type", "size", "threshold", "binding", "swapfile"};
    const std::array faults{"accepted", "stale-state", "read-denied", "write-denied-with-matching-state"};
    for (std::size_t operation = 0; operation < names.size(); ++operation)
      for (std::size_t fault = 0; fault < faults.size(); ++fault)
        QTest::newRow(qPrintable(QStringLiteral("%1/%2").arg(QString::fromLatin1(names[operation]), QString::fromLatin1(faults[fault]))))
            << int(operation) << int(fault);
  }
  void configurationWriteReceiptsDoNotIssueStateQueries() {
    QFETCH(int, operation);
    QFETCH(int, fault);
    Symbols symbols;
    symbols.state.boolean = operation == 1;  // HORM enable requires the running filter.
    symbols.state.number = 2048;
    symbols.state.overlayFlags = operation == 1 ? flags::readOnlyMedia : 0;
    symbols.state.mutate = fault != 1;
    if (fault == 2) symbols.state.observationStatus = E_ACCESSDENIED;
    if (fault == 3) symbols.state.writeStatus = E_ACCESSDENIED;
    symbols.add("UwfCfgGetFilterEnabled", getSessionBoolean);
    symbols.add("UwfCfgSetFilterEnabled", setBoolean);
    symbols.add("UwfCfgGetHormEnabled", getHorm);
    symbols.add("UwfCfgSetHormEnabled", setHorm);
    symbols.add("UwfCfgGetPersistDomainSecretKey", getSessionBoolean);
    symbols.add("UwfCfgSetPersistDomainSecretKey", setBoolean);
    symbols.add("UwfCfgGetOverlayCommitState", getGlobalBoolean);
    symbols.add("UwfCfgSetOverlayCommitState", setBoolean);
    symbols.add("UwfCfgGetOverlayType", getOverlayType);
    symbols.add("UwfCfgSetOverlayType", setOverlayType);
    symbols.add("UwfCfgGetOverlayMaximumSize", getSessionNumber);
    symbols.add("UwfCfgSetOverlayMaximumSize", setNumber);
    symbols.add("UwfCfgGetWarningThreshold", getGlobalNumber);
    symbols.add("UwfCfgSetWarningThreshold", setNumber);
    symbols.add("UwfCfgGetCriticalThreshold", getGlobalNumber);
    symbols.add("UwfCfgGetOverlayFlags", getOverlayFlags);
    symbols.add("UwfCfgSetOverlayFlags", setOverlayFlags);
    symbols.add("UwfCfgGetVolumeProtected", getVolumeBoolean);
    symbols.add("UwfCfgGetVolumeBindByDriveLetter", getVolumeBoolean);
    symbols.add("UwfCfgDoesVolumeExist", doesVolumeExist);
    symbols.add("UwfCfgSetVolumeBindByDriveLetter", setVolumeBoolean);
    symbols.add("UwfCfgGetVolumeSwapfileSize", getSwapfile);
    symbols.add("UwfCfgSetVolumeSwapfileSize", setSwapfile);
    const std::array requests{Request{SetBoolean{BooleanProperty::FilterEnabled, true}},
                              Request{SetBoolean{BooleanProperty::HormEnabled, true}},
                              Request{SetBoolean{BooleanProperty::PersistDomainSecretKey, true}},
                              Request{SetNumber{NumberProperty::OverlayType, static_cast<std::uint32_t>(OverlayType::Disk)}},
                              Request{SetNumber{NumberProperty::MaximumOverlaySize, 4096}},
                              Request{SetNumber{NumberProperty::WarningThreshold, 128}},
                              Request{SetVolume{guid, VolumeProperty::BindByDriveLetter, true}},
                              Request{CreateSwapfile{guid}}};
    const NativeUwfBackend backend(symbols.bindings());
    const auto response = backend.execute(requests.at(static_cast<std::size_t>(operation)));
    QCOMPARE(response.succeeded(), fault != 3);
    QCOMPARE(response.effect, fault != 3 ? Effect::Confirmed : Effect::Uncertain);
    QVERIFY(symbols.state.dispatchedWrite);
    QCOMPARE(symbols.state.calls, 1);
    QCOMPARE(symbols.state.postWriteReads, 0);
    QVERIFY(symbols.state.sessions.empty());
    if (fault == 3) {
      QCOMPARE(response.failure->kind, ErrorKind::AccessDenied);
      QCOMPARE(response.failure->diagnostics.front().code, std::int64_t{E_ACCESSDENIED});
    }
  }

  void volumeAndRegistryArgumentsRetainIdentityAndValuePresence() {
    Symbols symbols;
    symbols.arrays();
    symbols.maintenance();
    symbols.add("UwfCfgSetVolumeProtected", setVolumeBoolean);
    symbols.add("UwfCfgSetVolumeBindByDriveLetter", setVolumeBoolean);
    symbols.add("UwfCfgGetVolumeProtected", getVolumeBoolean);
    symbols.add("UwfCfgGetVolumeBindByDriveLetter", getVolumeBoolean);
    symbols.add("UwfCfgDoesVolumeExist", doesVolumeExist);
    symbols.add("UwfCfgGetHormEnabled", getHorm);
    symbols.add("UwfCfgGetVolumeSwapfileSize", getSwapfile);
    symbols.add("UwfCfgCommitRegistry", commitRegistry);
    NativeUwfBackend backend(symbols.bindings());
    symbols.state.boolean = false;
    (void)call(backend, SetVolume{wrappedGuid, VolumeProperty::BindByDriveLetter, true});
    QCOMPARE(call(backend, GetSwapfileSize{std::string("Volume{") + guid + "}", Session::Next}), unsignedValue);
    symbols.state.strings = {L"\\资料\\résumé.txt"};
    QCOMPARE(call(backend, GetFileExclusions{wrappedGuid, Session::Current}), std::vector<std::string>{"\\资料\\résumé.txt"});
    for (const auto& volume : symbols.state.volumes) QCOMPARE(volume, uwf::windows::utf8ToWide(normalizeVolumeId(guid)));
    QVERIFY(symbols.state.sessions.back());
    symbols.state.status = S_OK;
    for (const auto& value : {std::optional<std::string>{""}, std::optional<std::string>{"默认值"}})
      (void)call(backend, CommitRegistry{"HKEY_LOCAL_MACHINE\\SOFTWARE\\测试", value});
    QCOMPARE(symbols.state.registryValues, std::vector<std::optional<std::wstring>>({L"", L"默认值"}));
    QVERIFY(symbols.state.registryAbiMatched);
  }

  void registryMaintenancePreservesTheRequestedTarget_data() {
    QTest::addColumn<QString>("key");
    QTest::addColumn<QString>("wireKey");
    QTest::addColumn<RegistryMaintenanceOperation>("operation");
    QTest::addColumn<bool>("supported");
    const std::array operations{std::pair{"commit-value", RegistryMaintenanceOperation::CommitValue},
                                std::pair{"delete-key", RegistryMaintenanceOperation::DeleteKey},
                                std::pair{"delete-value", RegistryMaintenanceOperation::DeleteValue}};
    struct Target {
      const char* input;
      const char* wire;
      bool supported;
    };
    const std::array targets{Target{"HKLM\\SYSTEM\\CurrentControlSetExtra", "", false},
                             Target{"hkey_local_machine\\system\\currentcontrolsetBackup", "", false},
                             Target{"hKlM\\SYSTEM\\CurrentControlSet ", "", false},
                             Target{"HKLM\\SYSTEM\\CurrentControlSet.", "", false},
                             Target{"HKEY_LOCAL_MACHINE\\system\\currentcontrolset", "HKEY_LOCAL_MACHINE\\system\\currentcontrolset", true},
                             Target{"HKEY_LOCAL_MACHINE\\SYSTEM\\ControlSet001", "HKEY_LOCAL_MACHINE\\SYSTEM\\ControlSet001", true},
                             Target{"HKEY_LOCAL_MACHINE\\SYSTEM\\ControlSet002", "HKEY_LOCAL_MACHINE\\SYSTEM\\ControlSet002", true}};
    for (const auto& [root, wireRoot, supported] : targets)
      for (const bool descendant : {false, true})
        for (const auto& [operationName, operation] : operations) {
          const auto key = QString::fromLatin1(root) + (descendant ? QStringLiteral("\\Child") : QString{});
          const auto wireKey = QString::fromLatin1(wireRoot) + (descendant ? QStringLiteral("\\Child") : QString{});
          QTest::newRow(qPrintable(key + QStringLiteral("/%1").arg(QString::fromLatin1(operationName)))) << key << wireKey << operation << supported;
        }
  }
  void registryMaintenancePreservesTheRequestedTarget() {
    QFETCH(QString, key);
    QFETCH(QString, wireKey);
    QFETCH(RegistryMaintenanceOperation, operation);
    QFETCH(bool, supported);
    Symbols symbols;
    symbols.maintenance();
    symbols.add("UwfCfgCommitRegistry", commitRegistry);
    symbols.add("UwfCfgCommitRegistryDeletion", commitRegistry);
    symbols.state.strings = {L"retained configuration"};
    NativeUwfBackend backend(symbols.bindings());
    // Native bindings cache exports at construction. Operation admission must
    // not add any lookup or invoke the cached function for an unsafe target.
    const auto resolutions = symbols.resolutions;
    const auto name = operation == RegistryMaintenanceOperation::DeleteKey ? std::nullopt : std::optional<std::string>{"Named"};
    const Request request = operation == RegistryMaintenanceOperation::CommitValue ? Request{CommitRegistry{key.toStdString(), name}}
                                                                                   : Request{CommitRegistryDeletion{key.toStdString(), name}};
    QCOMPARE(backend.probe(request).support, supported ? Support::Supported : Support::Unsupported);
    const auto result = backend.execute(request);
    QCOMPARE(result.succeeded(), supported);
    QCOMPARE(result.effect, supported ? Effect::Confirmed : Effect::None);
    QCOMPARE(symbols.resolutions, resolutions);
    QCOMPARE(symbols.state.strings, std::vector<std::wstring>{L"retained configuration"});
    QVERIFY(symbols.state.boolean);
    QVERIFY(!symbols.state.horm);
    QCOMPARE(symbols.state.overlayType, static_cast<ULONG>(OverlayType::Ram));
    QCOMPARE(symbols.state.number, static_cast<ULONG>(unsignedValue));
    if (supported) {
      QVERIFY(!symbols.resolutions.empty());
      QCOMPARE(symbols.state.calls, 1);
      // The raw ABI retains the exact supplied subkey and its spelling.
      QCOMPARE(symbols.state.paths, std::vector<std::wstring>{wireKey.toStdWString()});
      QCOMPARE(symbols.state.registryValues, std::vector<std::optional<std::wstring>>{
                                                 operation == RegistryMaintenanceOperation::DeleteKey ? std::nullopt : std::optional<std::wstring>{L"Named"}});
      QVERIFY(symbols.state.registryAbiMatched);
    } else {
      QVERIFY(result.failure);
      QCOMPARE(result.failure->kind, ErrorKind::Unsupported);
      QCOMPARE(result.failure->effect, Effect::None);
      QCOMPARE(symbols.state.calls, 0);
      QVERIFY(symbols.state.paths.empty());
      QVERIFY(symbols.state.registryValues.empty());
      QVERIFY(symbols.state.sessions.empty());
      QVERIFY(symbols.state.volumes.empty());
      QVERIFY(symbols.state.arrayBuffers.empty());
    }
  }
  void registryCommandsRespectOperationAndPointerContracts_data() {
    QTest::addColumn<bool>("deletion");
    QTest::addColumn<bool>("hasValue");
    QTest::addColumn<QString>("value");
    QTest::addColumn<bool>("supported");
    QTest::newRow("whole-key-commit-unsupported") << false << false << QString{} << false;
    QTest::newRow("default-value-commit") << false << true << QString{} << true;
    QTest::newRow("named-value-commit") << false << true << QStringLiteral("默认值") << true;
    QTest::newRow("whole-key-deletion-nonnull-empty-value") << true << false << QString{} << true;
    QTest::newRow("default-value-deletion-unsupported") << true << true << QString{} << false;
    QTest::newRow("named-value-deletion") << true << true << QStringLiteral("默认值") << true;
  }
  void registryCommandsRespectOperationAndPointerContracts() {
    QFETCH(bool, deletion);
    QFETCH(bool, hasValue);
    QFETCH(QString, value);
    QFETCH(bool, supported);
    Symbols symbols;
    symbols.maintenance();
    symbols.add("UwfCfgCommitRegistry", commitRegistry);
    symbols.add("UwfCfgCommitRegistryDeletion", commitRegistry);
    NativeUwfBackend backend(symbols.bindings());
    const auto name = hasValue ? std::optional<std::string>{value.toUtf8().toStdString()} : std::nullopt;
    const Request request = deletion ? Request{CommitRegistryDeletion{"HKEY_LOCAL_MACHINE\\Software\\Test", name}}
                                     : Request{CommitRegistry{"HKEY_LOCAL_MACHINE\\Software\\Test", name}};
    QCOMPARE(backend.probe(request).support, supported ? Support::Supported : Support::Unsupported);
    QCOMPARE(symbols.state.calls, 0);
    for (const auto status : {S_OK, E_ACCESSDENIED}) {
      symbols.state.writeStatus = status;
      const auto before = symbols.state.calls;
      const auto response = backend.execute(request);
      QCOMPARE(response.succeeded(), supported && status == S_OK);
      QCOMPARE(response.effect, !supported ? Effect::None : status == S_OK ? Effect::Confirmed : Effect::Uncertain);
      QCOMPARE(symbols.state.calls, before + (supported ? 1 : 0));
      QVERIFY(symbols.state.registryAbiMatched);
      if (supported)
        QCOMPARE(symbols.state.registryValues.back(), hasValue ? std::optional<std::wstring>{value.toStdWString()} : std::nullopt);
      else
        QCOMPARE(response.failure->kind, ErrorKind::Unsupported);
    }
  }

  void existenceStatusAndUnsafeDefaultDeletionAreNotGuessed() {
    Symbols symbols;
    symbols.maintenance();
    symbols.add("UwfCfgCommitRegistryDeletion", commitRegistry);
    NativeUwfBackend backend(symbols.bindings());
    symbols.state.protectedStatus = HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
    symbols.state.volumeExistsStatus = S_FALSE;
    const auto unknown = backend.execute(GetVolumeProperty{guid, VolumeProperty::Protected, Session::Current});
    QVERIFY(unknown.failure);
    QCOMPARE(unknown.failure->kind, ErrorKind::NotFound);
    const auto calls = symbols.state.calls;
    const Request deletion = CommitRegistryDeletion{"HKEY_LOCAL_MACHINE\\Software\\Test", std::string{}};
    QCOMPARE(backend.probe(deletion).support, Support::Unsupported);
    const auto rejected = backend.execute(deletion);
    QCOMPARE(rejected.failure->kind, ErrorKind::Unsupported);
    QCOMPARE(rejected.effect, Effect::None);
    QCOMPARE(symbols.state.calls, calls);
  }

  void malformedInventoryIdentityStopsBeforePropertyExports() {
    Symbols symbols;
    symbols.arrays();
    symbols.state.strings = {L"Volume{11111111-2222-3333-4444-555555555555}\\\\"};
    const auto response = NativeUwfBackend(symbols.bindings()).execute(GetVolumes{Session::Current});
    QVERIFY(response.failure);
    QCOMPARE(response.failure->kind, ErrorKind::Protocol);
    QCOMPARE(response.effect, Effect::None);
    QCOMPARE(symbols.state.calls, 1);
    QCOMPARE(symbols.state.arrayReleases, 1);
    QCOMPARE(symbols.state.driveReleases, 0);
    QVERIFY(symbols.state.releaseMatched);
    QVERIFY(symbols.state.arrayBuffers.empty());
  }
  void inventoryDistinguishesAbsentDriveMetadataFromInvalidSuccessfulOutput_data() {
    QTest::addColumn<int>("observationIndex");
    QTest::addColumn<bool>("current");
    constexpr std::array names{"uppercase",     "lowercase",     "allocated-empty", "missing-value", "success-without-output",
                               "invalid-drive", "access-denied", "invalid-utf16"};
    for (std::size_t index = 0; index < names.size(); ++index)
      for (const bool current : {true, false})
        QTest::newRow(
            qUtf8Printable(QStringLiteral("%1/%2").arg(QString::fromLatin1(names[index]), current ? QStringLiteral("Current") : QStringLiteral("Next"))))
            << static_cast<int>(index) << current;
  }
  void inventoryRequiredFieldsUseTheSameIdentityContractAsScalarQueries_data() { volumePropertyFailuresPreserveTheGetterStatus_data(); }
  void inventoryRequiredFieldsUseTheSameIdentityContractAsScalarQueries() {
    QFETCH(bool, current);
    QFETCH(int, property);
    QFETCH(qint32, propertyStatus);
    QFETCH(int, kind);
    Symbols symbols;
    symbols.arrays();
    symbols.state.strings = {L"11111111-2222-3333-4444-555555555555"};
    symbols.state.protectedStatus = propertyStatus;
    symbols.add("UwfCfgGetVolumeProtected", getVolumeBoolean);
    symbols.add("UwfCfgGetVolumeBindByDriveLetter", getVolumeBoolean);
    symbols.add(property == static_cast<int>(VolumeProperty::Protected) ? "UwfCfgGetVolumeProtected" : "UwfCfgGetVolumeBindByDriveLetter",
                getFailedInventoryProperty);
    const NativeUwfBackend backend(symbols.bindings());
    const auto response = backend.execute(GetVolumes{current ? Session::Current : Session::Next});
    QVERIFY(response.failure);
    QCOMPARE(response.failure->kind, static_cast<ErrorKind>(kind));
    QCOMPARE(response.effect, Effect::None);
    QVERIFY(!response.failure->diagnostics.empty());
    QCOMPARE(response.failure->diagnostics.front().code, std::int64_t{propertyStatus});
    QVERIFY(std::ranges::all_of(symbols.state.sessions, [current](const bool observed) { return observed == current; }));
    QCOMPARE(symbols.state.arrayReleases, 1);
    QCOMPARE(symbols.state.driveReleases, 1);
    QVERIFY(symbols.state.releaseMatched);
    QVERIFY(symbols.state.arrayBuffers.empty());
    QVERIFY(symbols.state.driveBuffers.empty());
    QVERIFY(!symbols.state.dispatchedWrite);
  }
  void inventoryDistinguishesAbsentDriveMetadataFromInvalidSuccessfulOutput() {
    QFETCH(int, observationIndex);
    QFETCH(bool, current);
    struct Observation {
      std::wstring output;
      HRESULT status;
      bool omitOutput;
      std::optional<ErrorKind> failure;
      const char* expectedDrive;
    };
    const std::array observations{Observation{L"C:", S_OK, false, {}, "C:"},
                                  Observation{L"c:", S_OK, false, {}, "C:"},
                                  Observation{L"", S_OK, false, {}, ""},
                                  Observation{L"", HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND), false, {}, ""},
                                  Observation{L"", S_OK, true, ErrorKind::Protocol, ""},
                                  Observation{L"C:\\bad", S_OK, false, ErrorKind::Protocol, ""},
                                  Observation{L"C:", E_ACCESSDENIED, false, ErrorKind::AccessDenied, ""},
                                  Observation{std::wstring(1, static_cast<wchar_t>(0xd800)), S_OK, false, ErrorKind::Protocol, ""}};
    const auto& observation = observations[static_cast<std::size_t>(observationIndex)];
    Symbols symbols;
    symbols.arrays();
    symbols.state.strings = {L"11111111-2222-3333-4444-555555555555"};
    symbols.state.driveLetter = observation.output;
    symbols.state.driveStatus = observation.status;
    symbols.state.omitDriveOutput = observation.omitOutput;
    symbols.state.protectedVolume = true;
    symbols.state.bindByDriveLetter = false;
    const NativeUwfBackend backend(symbols.bindings());
    const auto response = backend.execute(GetVolumes{current ? Session::Current : Session::Next});
    QCOMPARE(response.succeeded(), !observation.failure);
    QCOMPARE(response.effect, Effect::None);
    if (observation.failure) {
      QVERIFY(response.failure);
      QCOMPARE(response.failure->kind, *observation.failure);
      if (observation.status == E_ACCESSDENIED) QCOMPARE(response.failure->diagnostics.front().code, std::int64_t{E_ACCESSDENIED});
    } else {
      const auto& volumes = std::get<std::vector<Volume>>(response.value);
      QCOMPARE(volumes.size(), std::size_t{1});
      QCOMPARE(volumes.front().id, std::string(guid));
      QCOMPARE(volumes.front().driveLetter, std::string(observation.expectedDrive));
    }
    QCOMPARE(symbols.state.arrayReleases, 1);
    QCOMPARE(symbols.state.driveReleases, observation.omitOutput ? 0 : 1);
    QVERIFY(symbols.state.releaseMatched);
    QVERIFY(symbols.state.arrayBuffers.empty());
    QVERIFY(symbols.state.driveBuffers.empty());
    QVERIFY(!symbols.state.dispatchedWrite);
  }
  void inventoryDriveMetadataIsCanonicalOrAProtocolFailure() {
    for (const auto* letter : {L"C:", L"c:", L"", L"C:\\bad"}) {
      Symbols symbols;
      symbols.arrays();
      symbols.state.strings = {L"11111111-2222-3333-4444-555555555555"};
      symbols.state.driveLetter = letter;
      const auto response = NativeUwfBackend(symbols.bindings()).execute(GetVolumes{Session::Current});
      const bool valid = symbols.state.driveLetter.empty() || symbols.state.driveLetter.size() == 2;
      QCOMPARE(response.succeeded(), valid);
      if (valid)
        QCOMPARE(std::get<std::vector<Volume>>(response.value).front().driveLetter, symbols.state.driveLetter.empty() ? std::string{} : std::string("C:"));
      else {
        QCOMPARE(response.failure->kind, ErrorKind::Protocol);
        QCOMPARE(response.effect, Effect::None);
      }
      QVERIFY(symbols.state.releaseMatched);
      QVERIFY(symbols.state.driveBuffers.empty());
      QVERIFY(symbols.state.arrayBuffers.empty());
    }
  }
  void enumeratedVolumeMayHaveNoDriveLetterButOtherFailuresRemainErrors() {
    Symbols symbols;
    symbols.arrays();
    symbols.state.strings = {L"11111111-2222-3333-4444-555555555555"};
    NativeUwfBackend backend(symbols.bindings());
    symbols.state.driveStatus = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    const auto result = call(backend, GetVolumes{});
    QCOMPARE(result.size(), std::size_t{1});
    QVERIFY(result.front().driveLetter.empty());
    QVERIFY(result.front().protectedVolume);
    symbols.state.driveStatus = HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND);
    QVERIFY(backend.execute(GetVolumes{}).failure);
    symbols.state.driveStatus = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    symbols.state.status = E_ACCESSDENIED;
    const auto inaccessibleVolume = backend.execute(GetVolumes{});
    QCOMPARE(inaccessibleVolume.failure->kind, ErrorKind::AccessDenied);
    QVERIFY(symbols.state.releaseMatched);
    QVERIFY(symbols.state.driveBuffers.empty());
  }

  void volumeRegistrationDispatchesOnlyItsOwnExport() {
    Symbols symbols;
    symbols.add("UwfCfgAddVolume", addVolume);
    const NativeUwfBackend backend(symbols.bindings());
    QCOMPARE(backend.probe(AddVolume{guid, {}}).support, Support::Supported);
    for (const auto unrelatedStatus : {S_OK, HRESULT_FROM_WIN32(ERROR_NOT_FOUND), E_ACCESSDENIED}) {
      symbols.state.volumeExistsStatus = unrelatedStatus;
      const auto response = backend.execute(AddVolume{wrappedGuid, {}});
      QVERIFY(response.succeeded());
      QCOMPARE(response.effect, Effect::Confirmed);
    }
    QCOMPARE(symbols.state.calls, 3);
    QCOMPARE(symbols.state.postWriteReads, 0);
    QVERIFY(symbols.state.sessions.empty());
    QCOMPARE(symbols.state.volumes, std::vector<std::wstring>(3, uwf::windows::utf8ToWide(normalizeVolumeId(guid))));
  }
  void arrayCapabilitiesDoNotRequireModuleIdentity() {
    Symbols symbols;
    symbols.arrays();
    const auto injected = symbols.bindings();
    NativeUwfBindings bindings;
    bindings.resolveExport = injected.resolveExport;
    bindings.resolveImport = injected.resolveImport;
    const NativeUwfBackend backend(std::move(bindings));
    for (const Request& request : {Request{GetVolumes{}}, Request{GetFileExclusions{guid}}, Request{GetRegistryExclusions{}}})
      QCOMPARE(backend.probe(request).support, Support::Supported);
    QCOMPARE(symbols.state.calls, 0);
    symbols.state.strings = {L"\\one", L"\\two"};
    QCOMPARE(call(backend, GetFileExclusions{guid}), std::vector<std::string>({"\\one", "\\two"}));
    symbols.state.strings = {L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Vendor"};
    QCOMPARE(call(backend, GetRegistryExclusions{}), std::vector<std::string>({"HKEY_LOCAL_MACHINE\\SOFTWARE\\Vendor"}));
    symbols.state.strings = {L"11111111-2222-3333-4444-555555555555"};
    const auto volumes = call(backend, GetVolumes{});
    QCOMPARE(volumes.size(), std::size_t{1});
    QCOMPARE(volumes.front().id, normalizeVolumeId(guid));
    QCOMPARE(symbols.state.arrayReleases, 3);
    QCOMPARE(symbols.state.driveReleases, 1);
    QVERIFY(symbols.state.releaseMatched);
    QVERIFY(symbols.state.arrayBuffers.empty());
    QVERIFY(symbols.state.driveBuffers.empty());
  }
  void missingArrayDependenciesOnlyDisableRelatedOperations() {
    Symbols symbols;
    symbols.arrays();
    symbols.imports.erase("free");
    NativeUwfBackend noFree(symbols.bindings());
    QCOMPARE(noFree.probe(GetVolumes{}).support, Support::Unsupported);
    QCOMPARE(noFree.probe(GetFileExclusions{guid}).support, Support::Unsupported);
    QCOMPARE(noFree.probe(GetRegistryExclusions{}).support, Support::Unsupported);
    QCOMPARE(noFree.probe(GetVolumeProperty{guid, VolumeProperty::Protected}).support, Support::Supported);
    QVERIFY(noFree.execute(GetFileExclusions{guid}).failure);
    QCOMPARE(symbols.state.calls, 0);
    symbols.imports["free"] = reinterpret_cast<NativeUwfProcedure>(producerFree);
    symbols.imports.erase("??_V@YAXPEAX@Z");
    NativeUwfBackend noDeleteArray(symbols.bindings());
    QCOMPARE(noDeleteArray.probe(GetVolumes{}).support, Support::Unsupported);
    QCOMPARE(noDeleteArray.probe(GetFileExclusions{guid}).support, Support::Supported);
    QCOMPARE(noDeleteArray.probe(GetRegistryExclusions{}).support, Support::Supported);
    QCOMPARE(noDeleteArray.probe(GetVolumeProperty{guid, VolumeProperty::Protected}).support, Support::Supported);
    QVERIFY(noDeleteArray.execute(GetVolumes{}).failure);
    QCOMPARE(symbols.state.calls, 0);
    symbols.imports["??_V@YAXPEAX@Z"] = reinterpret_cast<NativeUwfProcedure>(producerDeleteArray);
    symbols.exports.erase("UwfCfgGetFileExclusions");
    NativeUwfBackend noFileExclusions(symbols.bindings());
    QCOMPARE(noFileExclusions.probe(GetFileExclusions{guid}).support, Support::Unsupported);
    QCOMPARE(noFileExclusions.probe(GetVolumes{}).support, Support::Supported);
    QCOMPARE(noFileExclusions.probe(GetRegistryExclusions{}).support, Support::Supported);
    QCOMPARE(noFileExclusions.probe(GetVolumeProperty{guid, VolumeProperty::Protected}).support, Support::Supported);
    QVERIFY(noFileExclusions.execute(GetFileExclusions{guid}).failure);
    QCOMPARE(symbols.state.calls, 0);
  }

  void arraysAndDriveLettersReleaseThroughTheirProducerBindings() {
    Symbols symbols;
    symbols.arrays();
    NativeUwfBackend backend(symbols.bindings());
    symbols.state.strings = {L"\\资料", L"\\second"};
    QCOMPARE(call(backend, GetFileExclusions{wrappedGuid, Session::Next}), std::vector<std::string>({"\\second", "\\资料"}));
    QCOMPARE(symbols.state.arrayReleases, 1);
    QVERIFY(symbols.state.arrayBuffers.empty());
    symbols.state.arrayStatus = E_ACCESSDENIED;
    const auto failedArray = backend.execute(GetFileExclusions{guid});
    QVERIFY(failedArray.failure);
    QCOMPARE(failedArray.effect, Effect::None);
    QCOMPARE(symbols.state.arrayReleases, 2);
    symbols.state.arrayStatus = S_OK;
    symbols.state.strings = {L"11111111-2222-3333-4444-555555555555"};
    const auto volumes = call(backend, GetVolumes{Session::Next});
    QCOMPARE(volumes.size(), std::size_t{1});
    QCOMPARE(volumes.front().id, normalizeVolumeId(guid));
    QCOMPARE(volumes.front().driveLetter, std::string("C:"));
    QVERIFY(volumes.front().protectedVolume);
    QVERIFY(volumes.front().bindByDriveLetter);
    QVERIFY(!volumes.front().commitPending);
    QCOMPARE(symbols.state.driveReleases, 1);
    QCOMPARE(symbols.state.arrayReleases, 3);
    symbols.state.status = E_ACCESSDENIED;
    QVERIFY(backend.execute(GetVolumes{}).failure);
    QCOMPARE(symbols.state.driveReleases, 2);
    QCOMPARE(symbols.state.arrayReleases, 4);
    QVERIFY(symbols.state.releaseMatched);
    QVERIFY(symbols.state.driveBuffers.empty());
    QVERIFY(symbols.state.arrayBuffers.empty());
  }

  void rawWritesNeverReadAdmissionOrConfirmationState_data() {
    QTest::addColumn<int>("operation");
    const std::array names{"FullCommit", "HORM", "register", "bind", "reset"};
    for (std::size_t operation = 0; operation < names.size(); ++operation) QTest::newRow(names[operation]) << static_cast<int>(operation);
  }
  void rawWritesNeverReadAdmissionOrConfirmationState() {
    QFETCH(int, operation);
    Symbols symbols;
    symbols.completeExportSurface();
    symbols.state.currentBoolean = false;
    symbols.state.romDisposition = 3;
    symbols.state.observationStatus = E_ACCESSDENIED;
    const NativeUwfBackend backend(symbols.bindings());
    const std::array<Request, 5> requests{SetBoolean{BooleanProperty::OverlayCommit, true}, SetBoolean{BooleanProperty::HormEnabled, true}, AddVolume{guid, {}},
                                          SetVolume{guid, VolumeProperty::BindByDriveLetter, true}, ResetSettings{}};
    const auto response = backend.execute(requests[static_cast<std::size_t>(operation)]);
    QVERIFY(response.succeeded());
    QCOMPARE(response.effect, Effect::Confirmed);
    QCOMPARE(symbols.state.calls, 1);
    QCOMPARE(symbols.state.postWriteReads, 0);
    QVERIFY(symbols.state.sessions.empty());
  }
  void hormSetterPreservesItsNativeFailureWithoutObservation_data() {
    QTest::addColumn<qint32>("status");
    QTest::newRow("success") << qint32{S_OK};
    QTest::newRow("denied") << qint32{E_ACCESSDENIED};
    QTest::newRow("invalid-state") << qint32{static_cast<HRESULT>(0x8007070du)};
  }
  void hormSetterPreservesItsNativeFailureWithoutObservation() {
    QFETCH(qint32, status);
    Symbols symbols;
    symbols.add("UwfCfgSetHormEnabled", setHorm);
    symbols.state.writeStatus = status;
    const NativeUwfBackend backend(symbols.bindings());
    const auto response = backend.execute(SetBoolean{BooleanProperty::HormEnabled, true});
    QCOMPARE(response.succeeded(), status == S_OK);
    QCOMPARE(response.effect, status == S_OK ? Effect::Confirmed : Effect::Uncertain);
    QCOMPARE(symbols.state.calls, 1);
    QCOMPARE(symbols.state.postWriteReads, 0);
    if (status != S_OK) {
      QCOMPARE(response.failure->kind, status == E_ACCESSDENIED ? ErrorKind::AccessDenied : ErrorKind::Conflict);
      QCOMPARE(response.failure->diagnostics.front().code, std::int64_t{status});
    }
  }
  void rawObservationsNeverSubstituteInactiveGlobalState() {
    Symbols symbols;
    symbols.state.currentBoolean = false;
    symbols.add("UwfCfgGetFilterEnabled", getSessionBoolean);
    const NativeUwfBackend backend(symbols.bindings());
    for (const Request& request : {Request{GetBoolean{BooleanProperty::OverlayCommit}}, Request{GetOverlayUsage{}}}) {
      QCOMPARE(backend.probe(request).support, Support::Unsupported);
      const auto response = backend.execute(request);
      QVERIFY(response.failure);
      QCOMPARE(response.failure->kind, ErrorKind::Unsupported);
      QCOMPARE(response.effect, Effect::None);
    }
    QCOMPARE(symbols.state.calls, 0);
    QVERIFY(symbols.state.sessions.empty());
  }
  void canceledRuntimeFactsDoNotOpenSystemDevices() {
    std::stop_source stop;
    stop.request_stop();
    const Context context{stop.get_token()};
    for (const auto& response : {uwf::windows::readRomDisposition(context), uwf::windows::readHormVolumeProtection(context),
                                 uwf::windows::readCapturedSettingsAvailable(context), uwf::windows::readShutdownPrivilegeAvailable(context)}) {
      QVERIFY(response.failure);
      QCOMPARE(response.failure->kind, ErrorKind::Canceled);
      QCOMPARE(response.effect, Effect::None);
    }
    const auto details = uwf::windows::readHormVolumeProtectionDetails(context);
    QVERIFY(std::holds_alternative<Failure>(details));
    QCOMPARE(std::get<Failure>(details).kind, ErrorKind::Canceled);
    QCOMPARE(std::get<Failure>(details).effect, Effect::None);
    const auto inspected = uwf::windows::inspectVolumeAdmission(guid, true, VolumeAdmissionPurpose::Registration, context);
    QVERIFY(std::holds_alternative<Failure>(inspected));
    QCOMPARE(std::get<Failure>(inspected).kind, ErrorKind::Canceled);
    const auto servicing = uwf::windows::readServicingAdmission(true, context);
    QVERIFY(std::holds_alternative<Failure>(servicing));
    QCOMPARE(std::get<Failure>(servicing).kind, ErrorKind::Canceled);
    QCOMPARE(std::get<Failure>(servicing).effect, Effect::None);
    const auto malformed = uwf::windows::inspectVolumeAdmission("Volume{incomplete}", false, VolumeAdmissionPurpose::Registration, {});
    QVERIFY(std::holds_alternative<Failure>(malformed));
    QCOMPARE(std::get<Failure>(malformed).kind, ErrorKind::InvalidArgument);
  }
  void nativePowerInvokesOnlyTheSelectedNativeEntryPoint() {
    Symbols symbols;
    symbols.add("UwfCfgShutdownSystem", noArguments);
    symbols.add("UwfCfgRestartSystem", noArguments);
    const NativeUwfBackend backend(symbols.bindings());
    for (const auto action : {PowerAction::Shutdown, PowerAction::Restart}) {
      const auto response = backend.execute(Power{action});
      QVERIFY(response.succeeded());
      QCOMPARE(response.effect, Effect::Confirmed);
    }
    QCOMPARE(symbols.state.calls, 2);
    QCOMPARE(symbols.state.noArgumentWrites, 2);
    QVERIFY(symbols.state.sessions.empty());
  }
  void fileMaintenancePreservesItsRequestWithoutQueryingDriveMetadata_data() {
    QTest::addColumn<bool>("deletion");
    QTest::addColumn<int>("relativeUnits");
    for (const bool deletion : {false, true})
      for (const int units : {1, 259, 260}) QTest::newRow(qUtf8Printable(QStringLiteral("%1/path-%2").arg(deletion).arg(units))) << deletion << units;
  }
  void fileMaintenancePreservesItsRequestWithoutQueryingDriveMetadata() {
    QFETCH(bool, deletion);
    QFETCH(int, relativeUnits);
    Symbols symbols;
    symbols.state.driveStatus = E_ACCESSDENIED;
    symbols.add("UwfCfgGetDriveLetter", getDriveLetter);
    symbols.add("UwfCfgCommitFile", commitNativeFile);
    symbols.add("UwfCfgCommitFileDeletion", commitNativeFile);
    const NativeUwfBackend backend(symbols.bindings());
    const auto path = "\\" + std::string(static_cast<std::size_t>(relativeUnits - 1), 'x');
    const Request request = deletion ? Request{CommitFileDeletion{wrappedGuid, path}} : Request{CommitFile{wrappedGuid, path}};
    const auto response = backend.execute(request);
    const bool fits = relativeUnits < 260;
    QCOMPARE(response.succeeded(), fits);
    QCOMPARE(response.effect, fits ? Effect::Confirmed : Effect::None);
    QCOMPARE(symbols.state.calls, fits ? 1 : 0);
    QCOMPARE(symbols.state.fileCommits, fits ? 1 : 0);
    QCOMPARE(symbols.state.driveReleases, 0);
    if (fits) {
      QCOMPARE(symbols.state.volumes, std::vector<std::wstring>{uwf::windows::utf8ToWide(normalizeVolumeId(guid))});
      QCOMPARE(symbols.state.paths, std::vector<std::wstring>{uwf::windows::utf8ToWide(path)});
    } else
      QCOMPARE(response.failure->kind, ErrorKind::InvalidArgument);
  }
};
QTEST_APPLESS_MAIN(NativeUwfBackendTests)
#include "NativeUwfBackendTests.moc"
