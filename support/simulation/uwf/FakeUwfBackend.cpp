// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "FakeUwfBackend.h"

#include <algorithm>
#include <array>
#include <functional>
#include <limits>
#include <mutex>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

#include "application/uwf/UwfOperations.h"
#include "domain/uwf/policies/ExclusionCoverage.h"
#include "domain/uwf/policies/FileExclusionPolicy.h"
#include "domain/uwf/policies/RegistryCommitPolicy.h"
#include "domain/uwf/policies/RegistryExclusionPolicy.h"
#include "foundation/paths/RegistryPath.h"
#include "foundation/text/ObjectNameComparison.h"
#include "foundation/text/StringUtil.h"
#include "foundation/text/Utf8.h"
#include "infrastructure/uwf/errors/WindowsUwfError.h"

namespace uwf::simulation {
class FakeUwfBackend::System final {
 public:
  explicit System(State initial) : state(std::move(initial)) {}
  std::mutex mutex;
  State state;
  compat::Fact<std::vector<std::string>> protectScope;
};
namespace {
using namespace compat;
using Fake = FakeUwfBackend;
using State = Fake::State;
using Configuration = Fake::Configuration;
using Checkpoint = std::function<void(Fake::FaultTiming, Effect)>;
constexpr std::uint64_t bytesPerMb = 1024 * 1024;
constexpr auto ram = static_cast<std::uint32_t>(OverlayType::Ram);
constexpr auto disk = static_cast<std::uint32_t>(OverlayType::Disk);
constexpr auto preserve = static_cast<std::uint32_t>(ResetMode::Preserve);
constexpr auto clear = static_cast<std::uint32_t>(ResetMode::Clear);
constexpr auto commitReset = static_cast<std::uint32_t>(ResetMode::Commit);
constexpr auto saved = static_cast<std::uint32_t>(ResetMode::Saved);
constexpr std::uint32_t romFilterChange = 1, romHormPreparation = 2, romFullCommit = 4;

enum class FileMaintenanceReservation { None, AllVolumes, SystemVolume };

// Exact relative names from the managed file-maintenance contract. Whether a
// SystemVolume reservation applies requires a separately observed identity.
FileMaintenanceReservation fileMaintenanceReservation(const std::string_view path) {
  if (equalAsciiInsensitive(path, "\\pagefile.sys")) return FileMaintenanceReservation::AllVolumes;
  if (equalAsciiInsensitive(path, "\\hiberfil.sys") || equalAsciiInsensitive(path, "\\UWFSWAP.SYS")) return FileMaintenanceReservation::SystemVolume;
  return FileMaintenanceReservation::None;
}

std::u16string objectName(const std::string_view value) {
  if (value.size() > static_cast<std::size_t>(std::numeric_limits<int>::max())) throw std::length_error("The simulated object name is too long");
  auto result = utf8ToUtf16(value);
  if (!result) throw std::invalid_argument("The simulated object name is not valid UTF-8");
  return std::move(*result);
}
int compareObjectNames(const std::u16string_view left, const std::u16string_view right) { return uwf::compareObjectNames(left, right); }
int compareObjectIdentity(const std::string_view left, const std::string_view right) { return compareObjectNames(objectName(left), objectName(right)); }
bool sameObjectIdentity(const std::string_view left, const std::string_view right) { return compareObjectIdentity(left, right) == 0; }
std::string pathSeparators(std::string path) {
  std::replace(path.begin(), path.end(), '/', '\\');
  while (path.size() > 1 && path.back() == '\\') path.pop_back();
  return path;
}
std::string filePath(std::string path) {
  path = pathSeparators(std::move(path));
  if (path.size() >= 2 && path[1] == ':') path.erase(0, 2);
  if (path.empty() || path.front() != '\\') path.insert(path.begin(), '\\');
  return path;
}
std::string registryKey(const std::string& key) { return regkey::expandHive(key); }
std::string registryObjectKey(const std::string& key, const std::uint32_t currentControlSet) {
  const auto translated = core::registryDriverObjectName(objectName(key), currentControlSet);
  const auto result = utf16ToUtf8(translated);
  if (!result) throw std::invalid_argument("The simulated registry object name is not valid UTF-16");
  return *result;
}
bool objectWithin(const std::string_view candidate, const std::string_view root) {
  const auto candidateName = objectName(candidate);
  const auto rootName = objectName(root);
  if (candidateName.size() < rootName.size() || compareObjectNames(std::u16string_view(candidateName).substr(0, rootName.size()), rootName) != 0) return false;
  return candidateName.size() == rootName.size() || (rootName.size() == 1 && rootName.front() == '\\') || candidateName[rootName.size()] == '\\';
}
template <class Evaluate>
core::ExclusionEffects exclusionEffects(const std::string_view pattern, const std::string_view target, Evaluate evaluate) {
  const auto stored = objectName(pattern);
  const auto actual = objectName(target);
  return evaluate(std::u16string_view(stored), std::u16string_view(actual));
}
bool driverExcluded(const std::vector<std::string>& exclusions, const std::string& path) {
  return std::ranges::any_of(exclusions,
                             [&path](const auto& exclusion) { return exclusionEffects(exclusion, path, core::fileExclusionEffects).driverBypassesWrites; });
}
bool configuredExcluded(const std::vector<std::string>& exclusions, const std::string& path) {
  return std::ranges::any_of(exclusions,
                             [&path](const auto& exclusion) { return exclusionEffects(exclusion, path, core::fileExclusionEffects).managementSkipsCommit; });
}
bool configuredExclusionExists(const std::vector<std::string>& exclusions, const std::string& path) {
  return std::ranges::any_of(exclusions, [&path](const auto& exclusion) { return equalAsciiInsensitive(path, exclusion); });
}
struct ModelFailure {
  ErrorKind kind;
  std::string message;
  Effect effect = Effect::None;
  std::int64_t code = 0;
  // Classify captured Windows statuses at the public boundary; injected faults retain their declared semantic kind.
  bool capturedStatus = false;
  // IOCTL stages expose NTSTATUS in the captured seam. The public DLL can
  // bridge that status through GetLastError before returning an HRESULT.
  std::optional<std::int64_t> publicStatus = std::nullopt;
};
ModelFailure receiptFailure(ModelFailure failure, const bool captured, const bool writeInvoked, const bool observation) {
  if (!captured && failure.capturedStatus) {
    if (failure.publicStatus) failure.code = *failure.publicStatus;
    const auto context = observation ? windows::WindowsUwfErrorContext::Observation : windows::WindowsUwfErrorContext::Operation;
    failure.kind = windows::classifyWindowsUwfError(static_cast<std::uint32_t>(failure.code), context).value_or(ErrorKind::Failed);
  }
  if (!captured && writeInvoked) failure.effect = Effect::Uncertain;
  return failure;
}
void require(const bool condition, const ErrorKind kind, const char* message) {
  if (!condition) throw ModelFailure{kind, message};
}
Configuration& config(State& state, const Session session) { return session == Session::Current ? state.current : state.next; }
const Fake::VolumeConfiguration& volume(const Configuration& configuration, const std::string& id) {
  const auto found = configuration.volumes.find(id);
  require(found != configuration.volumes.end(), ErrorKind::NotFound, "The UWF volume is not configured in this session");
  return found->second;
}
Fake::VolumeConfiguration& volume(Configuration& configuration, const std::string& id) {
  const auto found = configuration.volumes.find(id);
  require(found != configuration.volumes.end(), ErrorKind::NotFound, "The UWF volume is not configured in this session");
  return found->second;
}
bool persistentDisk(const Configuration& configuration) { return configuration.overlayType == disk && (configuration.overlayFlags & flags::persistent) != 0; }
bool fullCommit(const State& state) { return (state.runtime.romDisposition & romFullCommit) != 0; }
bool controlPresent(const State& state) {
  require(state.runtime.controlDeviceObservationAvailable, ErrorKind::Unavailable, "The UWF control-device presence observation is unavailable");
  return state.runtime.controlDevicePresent.value_or(state.current.filterEnabled);
}
bool readOnlyMedia(const State& state) {
  if (state.runtime.hormEnabled) return true;
  return (state.current.overlayFlags & flags::readOnlyMedia) != 0 &&
         (fullCommit(state) || (state.runtime.romDisposition & (romFilterChange | romHormPreparation)) == 0);
}
bool registryExcluded(const State& state, const std::string& key, const bool management) {
  const auto& configuration = state.current;
  const auto covered = [&](const std::string_view exclusion) {
    const auto effects = exclusionEffects(exclusion, key, [&](const auto pattern, const auto target) {
      return core::registryExclusionEffects(pattern, target, state.runtime.currentControlSet);
    });
    return management ? effects.managementSkipsCommit : effects.driverBypassesWrites;
  };
  return std::ranges::any_of(configuration.registryExclusions, covered) ||
         (configuration.persistDomainSecretKey && covered("HKEY_LOCAL_MACHINE\\SECURITY\\Policy\\Secrets\\$MACHINE.ACC")) ||
         (configuration.persistTscal && covered("HKEY_LOCAL_MACHINE\\Software\\Microsoft\\MSLicensing"));
}
bool protectedFile(const State& state, const Fake::FileAddress& address) {
  const auto found = state.current.volumes.find(address.volume);
  return state.current.filterEnabled && !fullCommit(state) && found != state.current.volumes.end() && found->second.volume.protectedVolume &&
         (readOnlyMedia(state) || !driverExcluded(found->second.fileExclusions, address.path));
}
bool knownSystemVolumeProtected(const State& state) {
  require(state.runtime.systemVolumeId.has_value(), ErrorKind::Unavailable, "The simulated system volume identity is unavailable");
  const auto found = state.current.volumes.find(*state.runtime.systemVolumeId);
  return found != state.current.volumes.end() && found->second.volume.protectedVolume;
}
bool protectedRegistry(const State& state, const Fake::RegistryAddress& address) {
  return state.current.filterEnabled && knownSystemVolumeProtected(state) && !fullCommit(state) &&
         (readOnlyMedia(state) || !registryExcluded(state, address.key, false));
}
Fake::FileAddress fileAddress(std::string id, std::string path) { return {normalizeVolumeId(std::move(id)), filePath(std::move(path))}; }
Fake::RegistryAddress registryAddress(std::string key, std::string name, const std::uint32_t currentControlSet) {
  return {registryObjectKey(key, currentControlSet), std::move(name)};
}

template <class Map, class Key>
std::optional<std::string> lookup(const Map& entries, const Key& address) {
  const auto found = entries.find(address);
  if (found == entries.end()) return {};
  return found->second;
}
template <class Map, class Overlay, class Key>
std::optional<std::string> visible(const Map& durable, const Overlay& overlay, const Key& address) {
  const auto changed = overlay.find(address);
  return changed != overlay.end() ? changed->second : lookup(durable, address);
}
template <class Durable, class Overlay, class Predicate>
void commitChanges(Durable& durable, Overlay& overlay, Predicate matches) {
  for (auto entry = overlay.begin(); entry != overlay.end();) {
    if (!matches(entry->first)) {
      ++entry;
      continue;
    }
    if (entry->second)
      durable.insert_or_assign(entry->first, *entry->second);
    else
      durable.erase(entry->first);
    entry = overlay.erase(entry);
  }
}
template <class Durable, class Overlay, class Key>
void commitEntry(Durable& durable, Overlay& overlay, const Key& address) {
  const auto entry = overlay.find(address);
  if (entry == overlay.end()) return;
  if (entry->second)
    durable.insert_or_assign(entry->first, *entry->second);
  else
    durable.erase(entry->first);
  overlay.erase(entry);
}
template <class Set, class Map, class Key>
bool visibleObject(const Set& durable, const Map& overlay, const Key& target) {
  const auto changed = overlay.find(target);
  return changed == overlay.end() ? durable.contains(target) : changed->second;
}
template <class Set, class Map>
void commitObjects(Set& durable, Map& overlay) {
  for (const auto& [target, exists] : overlay) {
    if (exists)
      durable.insert(target);
    else
      durable.erase(target);
  }
  overlay.clear();
}
bool isVisibleDirectory(const State& state, const Fake::FileAddress& target) {
  return visibleObject(state.durable.directories, state.overlay.directories, target);
}
bool visibleRegistryKey(const State& state, const std::string& key) { return visibleObject(state.durable.registryKeys, state.overlay.registryKeys, key); }
void refreshCommitPending(State& state) {
  for (auto& [id, configured] : state.current.volumes) {
    const auto first = state.overlay.files.lower_bound(Fake::FileAddress{id, {}});
    configured.volume.commitPending = first != state.overlay.files.end() && first->first.volume == id;
  }
}
void applyFileWrite(State& state, const Fake::FileAddress& address, std::optional<std::string> contents) {
  if (protectedFile(state, address))
    state.overlay.files.insert_or_assign(address, std::move(contents));
  else {
    state.partialPersistentFiles.erase(address);
    if (contents)
      state.durable.files.insert_or_assign(address, std::move(*contents));
    else
      state.durable.files.erase(address);
    state.overlay.files.erase(address);
  }
  state.runtime.usageOverride.reset();
  refreshCommitPending(state);
}
void commitFilterOverlay(State& state, const std::string& id, const Fake::FullCommitFilterOutcome& outcome) {
  // uwfvol!CommitOverlay frees the overlay and enables forwarding even after copy failure.
  // Completed-copy evidence is per filter; failed cluster contents remain indeterminate.
  for (auto entry = state.overlay.files.begin(); entry != state.overlay.files.end();) {
    if (entry->first.volume != id) {
      ++entry;
      continue;
    }
    const auto address = entry->first;
    const auto witnessed = [&](const auto& paths) {
      return std::ranges::any_of(paths, [&](const auto& path) { return sameObjectIdentity(path, address.path); });
    };
    if (!outcome.copyFails || witnessed(outcome.completedFilePaths)) {
      if (entry->second)
        state.durable.files.insert_or_assign(address, *entry->second);
      else
        state.durable.files.erase(address);
      state.partialPersistentFiles.erase(address);
    } else if (witnessed(outcome.indeterminateFilePaths)) {
      state.durable.files.erase(address);
      state.partialPersistentFiles.insert_or_assign(address, State::PartialFile{});
    }
    entry = state.overlay.files.erase(entry);
  }
  for (auto entry = state.overlay.directories.begin(); entry != state.overlay.directories.end();) {
    if (entry->first.volume != id) {
      ++entry;
      continue;
    }
    if (!outcome.copyFails) {
      if (entry->second)
        state.durable.directories.insert(entry->first);
      else
        state.durable.directories.erase(entry->first);
    }
    entry = state.overlay.directories.erase(entry);
  }
  if (state.runtime.systemVolumeId && id == *state.runtime.systemVolumeId) {
    if (!outcome.copyFails || outcome.registryCopyCompleted) {
      commitChanges(state.durable.registry, state.overlay.registry, [](const auto&) { return true; });
      commitObjects(state.durable.registryKeys, state.overlay.registryKeys);
    } else {
      state.overlay.registry.clear();
      state.overlay.registryKeys.clear();
    }
  }
}
void applyRegistryWrite(State& state, const Fake::RegistryAddress& address, std::optional<std::string> contents) {
  if (contents) {
    if (protectedRegistry(state, address))
      state.overlay.registryKeys.insert_or_assign(address.key, true);
    else
      state.durable.registryKeys.insert(address.key);
  }
  if (protectedRegistry(state, address))
    state.overlay.registry.insert_or_assign(address, std::move(contents));
  else {
    if (contents)
      state.durable.registry.insert_or_assign(address, std::move(*contents));
    else
      state.durable.registry.erase(address);
    state.overlay.registry.erase(address);
  }
  state.runtime.usageOverride.reset();
}
void changeOverlayType(State& state, const std::uint32_t type) {
  require(type == ram || type == disk, ErrorKind::InvalidArgument, "The overlay type must be RAM or Disk");
  if (state.next.overlayType == type) return;
  require(!state.current.filterEnabled, ErrorKind::AccessDenied, "Changing the overlay type requires UWF to be disabled in the current session");
  if (type == disk)
    state.next.overlayFlags |= flags::passthrough;
  else
    state.next.overlayFlags &= ~(flags::passthrough | flags::persistent);
  state.next.overlayType = type;
}
void changePersistentReset(State& state, const std::uint32_t desired) {
  if (controlPresent(state)) {
    require(desired == preserve || desired == clear, ErrorKind::InvalidArgument, "The control API accepts reset states 0 and 1");
    const bool activeFilters =
        state.current.filterEnabled && std::ranges::any_of(state.current.volumes, [](const auto& entry) { return entry.second.volume.protectedVolume; });
    // The global control driver logs a failing filter request and returns
    // success without updating its cached getter value. Model that distinction.
    if (activeFilters && (state.current.overlayFlags & flags::persistent) == 0) return;
    if (activeFilters) {
      state.runtime.persistentRollbackState = desired;
      if (state.runtime.persistentResetHeaderWriteFails) return;
    }
    state.runtime.resetPersistentState = desired;
  } else if (state.next.overlayType == disk) {
    state.next.overlayFlags |= flags::passthrough;
  } else if (state.next.overlayType != ram) {
    throw ModelFailure{ErrorKind::Protocol, "The Next overlay type is not recognized by the no-device reset fallback", Effect::None,
                       static_cast<std::int32_t>(0x8007000du), true};
  }
}
void changeFilterEnabled(State& state, const bool enabled) {
  if (state.current.filterEnabled) {
    if (!controlPresent(state))
      throw ModelFailure{ErrorKind::Unavailable, "The UWF volume control device is unavailable", Effect::None, static_cast<std::int32_t>(0x8000ffffu), true};
    if ((state.current.overlayFlags & flags::readOnlyMedia) != 0) {
      const auto disposition = state.runtime.romDisposition;
      if (disposition != 0 && (disposition & (romFilterChange | romFullCommit)) == 0)
        throw ModelFailure{ErrorKind::Conflict,
                           "The current ROM disposition does not permit a filter change",
                           Effect::None,
                           0xc0000184LL,
                           true,
                           static_cast<std::int32_t>(0x80070016u)};
      state.runtime.romDisposition |= romFilterChange;
    }
  }
  // UwfCfgSetFilterEnabled reads and validates Next OverlayType after the
  // Current disposition request, before reset selection and the Next flag.
  // A prior disposition transition is not rolled back by invalid metadata.
  if (state.next.overlayType != ram && state.next.overlayType != disk)
    throw ModelFailure{ErrorKind::Protocol, "The Next overlay type is not recognized", state.current.filterEnabled ? Effect::Uncertain : Effect::None,
                       static_cast<std::int32_t>(0x8007000du), true};
  changePersistentReset(state, state.current.filterEnabled != enabled || state.current.servicingEnabled != state.next.servicingEnabled ? clear : preserve);
  state.next.filterEnabled = enabled;
}
void leaveServicing(State& state) {
  if (!state.runtime.servicingSavedFilterEnabled) {
    state.runtime.servicingStatus = Fake::ServicingStatus::Failed;
    return;
  }
  try {
    // The native exit transition enables a previously enabled filter. A saved
    // zero does not issue a filter-disable command.
    if (*state.runtime.servicingSavedFilterEnabled) changeFilterEnabled(state, true);
    state.runtime.servicingStatus = Fake::ServicingStatus::Completed;
  } catch (const ModelFailure&) {
    // The captured function returns SetStatusEx's receipt even after restoration failure; retain the Failed runtime status.
    state.runtime.servicingStatus = Fake::ServicingStatus::Failed;
  }
}
void changeServicingEnabled(State& state, const bool enabled) {
  changePersistentReset(state, state.current.filterEnabled == state.next.filterEnabled && state.current.servicingEnabled == enabled ? preserve : clear);
  state.next.servicingEnabled = enabled;
  // SetServicingModeState writes the static flag before starting the runtime
  // transition. Any later failure must retain that earlier write's effect.
  try {
    switch (state.runtime.servicingStatus) {
      case Fake::ServicingStatus::NotStarted:
        if (!enabled) return;  // The outer wrapper normalizes 0x8000000D.
        break;
      case Fake::ServicingStatus::Started:
        if (enabled) return;
        [[fallthrough]];
      case Fake::ServicingStatus::Failed:
        if (!enabled) {
          leaveServicing(state);
          return;
        }
        break;
      case Fake::ServicingStatus::Completed:
      default:
        constexpr std::int64_t invalidData = static_cast<std::int32_t>(0x8007000du);
        throw ModelFailure{ErrorKind::Failed, "The captured servicing transition rejects this runtime status", Effect::Uncertain, invalidData, true};
    }
    state.runtime.servicingSavedFilterEnabled = state.next.filterEnabled;
    try {
      changeFilterEnabled(state, false);
    } catch (const ModelFailure&) {
      leaveServicing(state);
      state.runtime.servicingStatus = Fake::ServicingStatus::Failed;
      throw;
    }
    state.runtime.servicingStatus = Fake::ServicingStatus::Started;
  } catch (ModelFailure& failure) {
    failure.effect = Effect::Uncertain;
    throw;
  }
}
void changeHormEnabled(State& state, const bool enabled) {
  // Captured CUwfManagement::set_HORMEnabled (18a20) owns these private
  // observations. Public getters need not expose its Current type/flags.
  if (!controlPresent(state))
    throw ModelFailure{ErrorKind::Unavailable, "The UWF volume control device is unavailable", Effect::None, static_cast<std::int32_t>(0x8007070du), true};
  if (state.runtime.hormEnabled == enabled) return;
  if (enabled) {
    const auto requireState = [](const bool condition, const ErrorKind stageKind, const char* message) {
      if (!condition) throw ModelFailure{stageKind, message, Effect::None, static_cast<std::int32_t>(0x8007070du), true};
    };
    requireState(state.runtime.hiberfileExists, ErrorKind::NotFound, "HORM requires a hibernation file");
    requireState(state.current.overlayFlags == flags::readOnlyMedia, ErrorKind::Conflict, "HORM requires exactly the loaded Read-Only Media overlay flags");
    requireState(state.current.filterEnabled, ErrorKind::Conflict, "HORM requires an active UWF filter");
    requireState(state.current.overlayType == ram, ErrorKind::Conflict, "HORM requires a loaded RAM overlay");
    require(state.runtime.hormFixedVolumeSafety != Fake::SafetyObservation::Unknown, ErrorKind::Unavailable,
            "The fixed/mounted volume HORM safety observation is unavailable");
    requireState(state.runtime.hormFixedVolumeSafety == Fake::SafetyObservation::Satisfied, ErrorKind::Conflict,
                 "The observed fixed/mounted volume HORM safety check failed");
    if (state.runtime.romDisposition != 0)
      throw ModelFailure{ErrorKind::Conflict,
                         "HORM preparation requires an unchanged ROM disposition",
                         Effect::None,
                         0xc0000184LL,
                         true,
                         static_cast<std::int32_t>(0x80070016u)};
    state.runtime.romDisposition = romHormPreparation;
    // Mounted-filter/token safety follows disposition preparation. Failure
    // retains that acknowledged effect even though the HORM setting rolls back.
    if (state.runtime.hormFinalSafety != Fake::SafetyObservation::Satisfied)
      throw ModelFailure{state.runtime.hormFinalSafety == Fake::SafetyObservation::Unknown ? ErrorKind::Unavailable : ErrorKind::Conflict,
                         "The final HORM mounted-filter/token safety check did not succeed", Effect::Uncertain};
  } else {
    // SetRomModeDispositionFlagUnsafe(1,false) ORs mask 1 without clearing mask 2.
    state.runtime.romDisposition |= romFilterChange;
  }
  state.runtime.hormEnabled = enabled;
}
std::size_t utf16Length(const std::string_view text) {
  std::size_t count = 0;
  for (const char character : text) {
    const auto byte = static_cast<unsigned char>(character);
    if ((byte & 0xc0) == 0x80) continue;
    count += (byte & 0xf8) == 0xf0 ? 2 : 1;
  }
  return count;
}
void validateRegistryExclusion(const std::string& key) {
  const auto name = objectName(key);
  const auto error = core::registryExclusionError(std::u16string_view(name));
  require(error == core::RegistryExclusionError::None, ErrorKind::InvalidArgument, "The registry exclusion target is invalid");
}
void validateFileExclusion(const State& state, const std::string& id, const std::string& original) {
  const auto name = objectName(original);
  const auto error = core::fileExclusionError(std::u16string_view(name));
  require(error == core::FileExclusionError::None, ErrorKind::InvalidArgument, "The file exclusion path has invalid characters or length");
  const auto address = fileAddress(id, original);
  require(address.path != "\\" || !isVisibleDirectory(state, address), ErrorKind::InvalidArgument, "An existing volume root directory cannot be excluded");
  require(state.runtime.pagingFilesValidation != Fake::SafetyObservation::Unknown, ErrorKind::Unavailable,
          "The captured PagingFiles exclusion-validation context is unknown");
  require(state.runtime.pagingFilesValidation == Fake::SafetyObservation::Satisfied, ErrorKind::InvalidArgument,
          "The captured global PagingFiles context rejects file exclusions");
}
void validateFileCommitName(const State& state, const Fake::FileAddress& address, const bool directory = false) {
  // Format the management path before no-op checks, using only the observed Current DriveLetter.
  constexpr std::size_t managementNameBufferUnits = 260;  // Includes the NUL.
  const auto& prefix = volume(state.current, address.volume).volume.driveLetter;
  if (!prefix.empty())
    require(utf16Length(prefix) + utf16Length(address.path) < managementNameBufferUnits, ErrorKind::InvalidArgument,
            "The combined volume prefix and file name exceed the captured management buffer");
  if (directory) return;
  const bool system = state.runtime.systemVolumeId && address.volume == *state.runtime.systemVolumeId;
  require(!equalAsciiInsensitive(address.path, "\\pagefile.sys") &&
              (!system || (!equalAsciiInsensitive(address.path, "\\hiberfil.sys") && !equalAsciiInsensitive(address.path, "\\UWFSWAP.SYS"))),
          ErrorKind::InvalidArgument, "The file is reserved by UWF and cannot be committed");
}
void validateRegistryCommitTarget(const std::string& key, const std::optional<std::string>& value) {
  require(core::isRegistryCommitTargetAllowed(key, value.has_value()), ErrorKind::InvalidArgument,
          value ? "Registry value commits require HKLM" : "The whole registry key is reserved by UWF");
}
constexpr std::size_t registryCommitKeyLimit = 1023;
constexpr std::size_t registryCommitValueLimit = 255;
constexpr std::size_t fileCommitPathLimit = 1023;
void validateRegistryCommitBuffer(const std::string& key, const std::optional<std::string>& value) {
  require(utf16Length(key) <= registryCommitKeyLimit && (!value || utf16Length(*value) <= registryCommitValueLimit), ErrorKind::InvalidArgument,
          "The registry target exceeds the captured commit request buffer");
}
void changeExclusion(std::vector<std::string>& exclusions, std::string path, const ExclusionAction action) {
  const auto found = std::ranges::find_if(exclusions, [&path](const auto& existing) { return equalAsciiInsensitive(existing, path); });
  switch (action) {
    case ExclusionAction::Add:
      if (found == exclusions.end()) exclusions.push_back(std::move(path));
      return;
    case ExclusionAction::Remove:
      require(found != exclusions.end(), ErrorKind::NotFound, "The exclusion is not configured");
      exclusions.erase(found);
      return;
    case ExclusionAction::Clear:
      exclusions.clear();
      return;
    case ExclusionAction::Count:
      break;
  }
  throw ModelFailure{ErrorKind::InvalidArgument, "Unknown exclusion action"};
}
OverlayUsage overlayUsage(const State& state) {
  if (state.runtime.usageOverride) return *state.runtime.usageOverride;
  const bool active =
      state.current.filterEnabled && std::ranges::any_of(state.current.volumes, [](const auto& item) { return item.second.volume.protectedVolume; });
  if (!active) return {};
  std::uint64_t bytes = 0;
  for (const auto& [address, data] : state.overlay.files) {
    static_cast<void>(address);
    if (data) bytes += data->size();
  }
  for (const auto& [address, data] : state.overlay.registry) {
    static_cast<void>(address);
    if (data) bytes += data->size();
  }
  const auto used = static_cast<std::uint32_t>(std::min<std::uint64_t>(bytes / bytesPerMb, std::numeric_limits<std::uint32_t>::max()));
  return {used, used < state.current.maximumOverlaySize ? state.current.maximumOverlaySize - used : 0};
}

Value perform(State& state, const GetBoolean& request) {
  const auto& session = config(state, request.session);
  switch (request.property) {
    case BooleanProperty::FilterEnabled:
      return session.filterEnabled;
    case BooleanProperty::ShutdownPending:
      // GetSystemShutdownState initializes false and accepts a known absent
      // control device. A stale injected cache is not a device observation.
      return controlPresent(state) && state.runtime.shutdownPending;
    case BooleanProperty::HormEnabled:
      return state.runtime.hormEnabled;
    case BooleanProperty::ServicingEnabled:
      return session.servicingEnabled;
    case BooleanProperty::PersistDomainSecretKey:
      return session.persistDomainSecretKey;
    case BooleanProperty::PersistTscal:
      return session.persistTscal;
    case BooleanProperty::OverlayCommit:
      if (!controlPresent(state))
        throw ModelFailure{ErrorKind::Unavailable, "The UWF volume control device is unavailable", Effect::None, static_cast<std::int32_t>(0x8000ffffu), true};
      return fullCommit(state);
    case BooleanProperty::RegistryChangesAlwaysPersisted:
      throw ModelFailure{ErrorKind::Unsupported, "RegistryChangesAlwaysPersisted is a shared read-only observation"};
    case BooleanProperty::Count:
      break;
  }
  throw ModelFailure{ErrorKind::InvalidArgument, "Unknown boolean property"};
}
Value perform(State& state, const SetBoolean& request) {
  switch (request.property) {
    case BooleanProperty::FilterEnabled:
      changeFilterEnabled(state, request.value);
      break;
    case BooleanProperty::ShutdownPending:
      throw ModelFailure{ErrorKind::Unsupported, "ShutdownPending is read-only"};
    case BooleanProperty::RegistryChangesAlwaysPersisted:
      throw ModelFailure{ErrorKind::Unsupported, "RegistryChangesAlwaysPersisted is read-only"};
    case BooleanProperty::ServicingEnabled:
      changeServicingEnabled(state, request.value);
      break;
    case BooleanProperty::PersistDomainSecretKey:
      state.next.persistDomainSecretKey = request.value;
      break;
    case BooleanProperty::PersistTscal:
      state.next.persistTscal = request.value;
      break;
    case BooleanProperty::HormEnabled:
      changeHormEnabled(state, request.value);
      break;
    case BooleanProperty::OverlayCommit:
      require(request.value, ErrorKind::InvalidArgument, "OverlayCommit accepts only an enabled request");
      if (!controlPresent(state))
        throw ModelFailure{ErrorKind::Unavailable, "The UWF volume control device is unavailable", Effect::None, static_cast<std::int32_t>(0x8000ffffu), true};
      if ((state.current.overlayFlags & flags::readOnlyMedia) == 0) break;
      require(state.runtime.romDisposition == 0 || fullCommit(state), ErrorKind::Conflict, "The current ROM disposition does not permit FullCommit");
      if (fullCommit(state)) break;
      state.runtime.romDisposition = romFullCommit;
      for (const auto& [id, configured] : state.current.volumes) {
        if (!configured.volume.protectedVolume) continue;
        const auto observed = state.runtime.fullCommitFilters.find(id);
        Fake::FullCommitFilterOutcome outcome;
        outcome.copyFails = state.runtime.fullCommitCopyFails;
        if (observed != state.runtime.fullCommitFilters.end()) outcome = observed->second;
        commitFilterOverlay(state, id, outcome);
      }
      state.runtime.usageOverride.reset();
      refreshCommitPending(state);
      break;
    default:
      throw ModelFailure{ErrorKind::InvalidArgument, "Unknown boolean property"};
  }
  return std::monostate{};
}
Value perform(State& state, const GetNumber& request) {
  const auto& session = config(state, request.session);
  switch (request.property) {
    case NumberProperty::OverlayType:
      return session.overlayType;
    case NumberProperty::MaximumOverlaySize:
      return session.maximumOverlaySize;
    case NumberProperty::WarningThreshold:
      return state.current.warningThreshold;
    case NumberProperty::CriticalThreshold:
      return state.current.criticalThreshold;
    case NumberProperty::OverlayFlags:
      return session.overlayFlags;
    case NumberProperty::ResetPersistentState:
      return controlPresent(state) ? state.runtime.resetPersistentState : preserve;
    case NumberProperty::ResetPersistentStateSavedMode:
      return session.resetPersistentStateSavedMode;
    case NumberProperty::Count:
      break;
  }
  throw ModelFailure{ErrorKind::InvalidArgument, "Unknown numeric property"};
}
Value perform(State& state, const SetNumber& request) {
  switch (request.property) {
    case NumberProperty::OverlayType:
      changeOverlayType(state, request.value);
      break;
    case NumberProperty::MaximumOverlaySize:
      if (request.value == state.next.maximumOverlaySize) break;
      require(!state.current.filterEnabled, ErrorKind::AccessDenied, "Changing the overlay maximum size requires UWF to be disabled in the current session");
      state.next.maximumOverlaySize = request.value;
      break;
    case NumberProperty::WarningThreshold:
      state.current.warningThreshold = state.next.warningThreshold = request.value;
      break;
    case NumberProperty::CriticalThreshold:
      state.current.criticalThreshold = state.next.criticalThreshold = request.value;
      break;
    case NumberProperty::OverlayFlags:
      require(!state.current.filterEnabled, ErrorKind::AccessDenied, "Changing OverlayFlags requires UWF to be disabled in the current session");
      state.next.overlayFlags = request.value;
      break;
    case NumberProperty::ResetPersistentState: {
      require(request.value == preserve || request.value == clear || request.value == commitReset || request.value == saved, ErrorKind::InvalidArgument,
              "Unknown persistent overlay reset mode");
      const auto desired = request.value == saved ? state.current.resetPersistentStateSavedMode : request.value;
      changePersistentReset(state, desired);
      break;
    }
    case NumberProperty::ResetPersistentStateSavedMode:
      require(request.value == 1 || request.value == 2, ErrorKind::InvalidArgument, "The captured SavedMode wrapper accepts only 1 and 2");
      if (request.value != state.next.resetPersistentStateSavedMode) {
        require(request.value <= 1, ErrorKind::InvalidArgument, "The captured SavedMode static writer accepts only 0 and 1");
        state.next.resetPersistentStateSavedMode = request.value;
      }
      break;
    default:
      throw ModelFailure{ErrorKind::InvalidArgument, "Unknown numeric property"};
  }
  return std::monostate{};
}
Value perform(State& state, const GetOverlayUsage&) { return overlayUsage(state); }
Value perform(State& state, const GetVolumes& request) {
  std::vector<Volume> result;
  for (const auto& [id, configured] : config(state, request.session).volumes) {
    auto copy = configured.volume;
    copy.id = id;
    result.push_back(std::move(copy));
  }
  return result;
}
Value perform(State& state, const AddVolume& request) {
  const auto id = normalizeVolumeId(request.volume);
  require(!state.next.volumes.contains(id), ErrorKind::Conflict, "The UWF volume is already registered");
  auto [found, inserted] = state.next.volumes.try_emplace(id);
  if (inserted) {
    found->second.volume.id = id;
    found->second.volume.driveLetter = request.driveLetter;
  }
  return std::monostate{};
}
Value perform(State& state, const RemoveVolume& request) {
  require(state.next.volumes.erase(normalizeVolumeId(request.volume)) != 0, ErrorKind::NotFound, "The UWF volume is not configured for the next session");
  return std::monostate{};
}
Value perform(State& state, const SetVolume& request) {
  auto& target = volume(state.next, normalizeVolumeId(request.volume)).volume;
  switch (request.property) {
    case VolumeProperty::Protected:
      target.protectedVolume = request.value;
      break;
    case VolumeProperty::BindByDriveLetter:
      target.bindByDriveLetter = request.value;
      break;
    default:
      throw ModelFailure{ErrorKind::InvalidArgument, "Unknown volume property"};
  }
  return std::monostate{};
}
Value perform(State& state, const GetVolumeProperty& request) {
  const auto& target = volume(config(state, request.session), normalizeVolumeId(request.volume)).volume;
  switch (request.property) {
    case VolumeProperty::Protected:
      return target.protectedVolume;
    case VolumeProperty::BindByDriveLetter:
      return target.bindByDriveLetter;
    case VolumeProperty::Count:
      break;
  }
  throw ModelFailure{ErrorKind::InvalidArgument, "Unknown volume property"};
}
Value perform(State&, const SetOverlayOption&) { throw ModelFailure{ErrorKind::Unsupported, "Overlay options require the shared configuration command"}; }
Value perform(State&, const CreateSwapfile&) { throw ModelFailure{ErrorKind::Unsupported, "Swapfile creation requires the shared configuration command"}; }
Value perform(State& state, const GetFileExclusions& request) {
  return volume(config(state, request.session), normalizeVolumeId(request.volume)).fileExclusions;
}
Value perform(State& state, const EditFileExclusion& request, Effect& effect) {
  const auto id = normalizeVolumeId(request.volume);
  auto& exclusions = volume(state.next, id).fileExclusions;
  if (request.action == ExclusionAction::Add) validateFileExclusion(state, id, request.path);
  const bool submitsPrimitive = request.action != ExclusionAction::Clear || !exclusions.empty();
  changeExclusion(exclusions, request.path, request.action);
  effect = submitsPrimitive ? Effect::Confirmed : Effect::None;
  return std::monostate{};
}
Value perform(State& state, const FindFileExclusion& request) {
  return configuredExclusionExists(volume(config(state, request.session), normalizeVolumeId(request.volume)).fileExclusions, request.path);
}
Value perform(State& state, const GetRegistryExclusions& request) { return config(state, request.session).registryExclusions; }
Value perform(State& state, const EditRegistryExclusion& request, Effect& effect) {
  if (request.action == ExclusionAction::Add) validateRegistryExclusion(request.key);
  const bool submitsPrimitive = request.action != ExclusionAction::Clear || !state.next.registryExclusions.empty();
  changeExclusion(state.next.registryExclusions, request.key, request.action);
  effect = submitsPrimitive ? Effect::Confirmed : Effect::None;
  return std::monostate{};
}
Value perform(State& state, const FindRegistryExclusion& request) {
  return configuredExclusionExists(config(state, request.session).registryExclusions, request.key);
}
void requireActiveVolume(const State& state, const std::string& id) {
  require(state.current.filterEnabled && volume(state.current, id).volume.protectedVolume, ErrorKind::Conflict,
          "File commits require an active protected volume");
}
void requirePersistentCommitView(State& state, const bool registry, const std::string& id = {}) {
  if (!registry) {
    const auto found = state.runtime.fileCommitViews.find(id);
    const auto ready = found == state.runtime.fileCommitViews.end() ? state.runtime.defaultFileCommitView : found->second;
    if (ready != Fake::PersistentView::Ready)
      throw ModelFailure{ErrorKind::Unavailable,
                         ready == Fake::PersistentView::Unknown ? "Persistent file-view readiness is unknown" : "The persistent file view is not ready",
                         Effect::None,
                         ready == Fake::PersistentView::NotReady ? 0xc00000a3LL : 0,
                         ready == Fake::PersistentView::NotReady,
                         ready == Fake::PersistentView::NotReady ? std::optional<std::int64_t>{static_cast<std::int32_t>(0x80070015u)} : std::nullopt};
    return;
  }
  require(state.runtime.registryCommitHive != Fake::RegistryHive::Unknown, ErrorKind::Unavailable, "Persistent hive readiness is unknown");
  if (state.runtime.registryCommitHive == Fake::RegistryHive::Ready) return;
  // ROM rejects creation of a backing hive; an already-cached hive needs no creation check.
  if (readOnlyMedia(state))
    throw ModelFailure{ErrorKind::Failed,
                       "Read-Only Media mode rejects creating the uncached persistent hive",
                       Effect::None,
                       0xc0000001LL,
                       true,
                       static_cast<std::int32_t>(0x8007001fu)};
  state.runtime.registryCommitHive = Fake::RegistryHive::Ready;
}
Value perform(State& state, const CommitFile& request, const Checkpoint& checkpoint) {
  const auto address = fileAddress(request.volume, request.path);
  // The native wrapper resolves Current registration before it can decide
  // whether the existing file needs an overlay commit.
  const auto& currentVolume = volume(state.current, address.volume);
  validateFileCommitName(state, address);
  require(state.overlay.files.contains(address) || !state.partialPersistentFiles.contains(address), ErrorKind::Unavailable,
          "The finite model has no exact visible source bytes after a partial persistent copy");
  require(!isVisibleDirectory(state, address), ErrorKind::InvalidArgument, "A file commit does not accept a directory");
  require(visible(state.durable.files, state.overlay.files, address).has_value(), ErrorKind::NotFound, "The file does not exist");
  if (configuredExcluded(currentVolume.fileExclusions, address.path)) return std::monostate{};
  if (!state.current.filterEnabled || !currentVolume.volume.protectedVolume) return std::monostate{};
  require(utf16Length(address.path) <= fileCommitPathLimit, ErrorKind::InvalidArgument, "The file path exceeds the captured commit request buffer");
  requirePersistentCommitView(state, false, address.volume);
  if (!protectedFile(state, address)) return std::monostate{};
  const auto source = *visible(state.durable.files, state.overlay.files, address);
  auto& stages = state.fileCommitStages[address];
  stages.clear();
  const auto reached = [&](const Fake::FaultTiming stage) {
    stages.push_back(stage);
    checkpoint(stage, Effect::Uncertain);
  };
  // uwfs!UwfsCopyDataStreamByHandle sets EOF before 64 KiB writes; an unwritten tail remains indeterminate.
  state.durable.files.erase(address);
  state.partialPersistentFiles.insert_or_assign(address, State::PartialFile{.length = source.size(), .completedPrefix = {}});
  if (source.empty()) {
    state.durable.files.insert_or_assign(address, source);
    state.partialPersistentFiles.erase(address);
  }
  reached(Fake::FaultTiming::AfterDestinationEof);
  constexpr std::size_t copyChunk = 0x10000;
  for (std::size_t copied = 0; copied < source.size();) {
    copied += std::min(copyChunk, source.size() - copied);
    state.partialPersistentFiles.at(address).completedPrefix = source.substr(0, copied);
    if (copied == source.size()) {
      state.durable.files.insert_or_assign(address, source);
      state.partialPersistentFiles.erase(address);
    }
    reached(Fake::FaultTiming::AfterFileDataChunk);
  }
  state.durable.files.insert_or_assign(address, source);
  state.partialPersistentFiles.erase(address);
  reached(Fake::FaultTiming::AfterFileData);
  // Recovered metadata order: EA -> alternate streams -> timestamps -> security.
  // Checkpoints record completed helpers without modeling security descriptors.
  for (const auto stage :
       {Fake::FaultTiming::AfterFileEa, Fake::FaultTiming::AfterFileAds, Fake::FaultTiming::AfterFileTime, Fake::FaultTiming::AfterFileSecurity})
    reached(stage);
  state.overlay.files.erase(address);
  state.runtime.usageOverride.reset();
  refreshCommitPending(state);
  return std::monostate{};
}
Value perform(State& state, const CommitFileDeletion& request, const Checkpoint& checkpoint) {
  const auto address = fileAddress(request.volume, request.path);
  const auto currentVolume = state.current.volumes.find(address.volume);
  require(currentVolume != state.current.volumes.end() && !configuredExcluded(currentVolume->second.fileExclusions, address.path) &&
              state.current.filterEnabled && currentVolume->second.volume.protectedVolume,
          ErrorKind::Conflict, "A deletion commit requires a target in the active overlay that is not excluded");
  require(utf16Length(address.path) <= fileCommitPathLimit, ErrorKind::InvalidArgument, "The file path exceeds the captured deletion request buffer");
  const bool directory = isVisibleDirectory(state, address);
  const bool sourceIsPersistent = !protectedFile(state, address);
  validateFileCommitName(state, address, directory);
  requirePersistentCommitView(state, false, address.volume);
  require(directory || state.overlay.files.contains(address) || !state.partialPersistentFiles.contains(address), ErrorKind::Unavailable,
          "The finite model cannot observe the deletion source after a partial persistent copy");
  require(directory || visible(state.durable.files, state.overlay.files, address).has_value(), ErrorKind::NotFound, "The deletion target is not visible");
  if (directory) {
    const auto child = [&address](const auto& target) {
      return target.volume == address.volume && !sameObjectIdentity(target.path, address.path) && objectWithin(target.path, address.path);
    };
    const auto visibleFileChild = [&](const auto& item) {
      return child(item.first) && visible(state.durable.files, state.overlay.files, item.first).has_value();
    };
    const auto visibleDirectoryChild = [&](const auto& target) { return child(target) && isVisibleDirectory(state, target); };
    require(!std::ranges::any_of(state.durable.files, visibleFileChild) && !std::ranges::any_of(state.overlay.files, visibleFileChild) &&
                !std::ranges::any_of(state.durable.directories, visibleDirectoryChild) &&
                !std::ranges::any_of(state.overlay.directories, [&](const auto& item) { return visibleDirectoryChild(item.first); }),
            ErrorKind::Conflict, "Directory deletion is nonrecursive and requires its children to be deleted first");
    if (sourceIsPersistent)
      require(!std::ranges::any_of(state.durable.files, [&](const auto& item) { return child(item.first); }) &&
                  !std::ranges::any_of(state.durable.directories, child),
              ErrorKind::Conflict, "The source persistent directory still has children");
  }
  auto& progress = state.fileDeletions[address];
  progress = {};
  const auto savedFile = state.overlay.files.find(address);
  std::optional<std::optional<std::string>> savedFileOverlay;
  if (savedFile != state.overlay.files.end()) savedFileOverlay.emplace(savedFile->second);
  const auto savedDirectory = state.overlay.directories.find(address);
  const auto savedDirectoryOverlay = savedDirectory == state.overlay.directories.end() ? std::optional<bool>{} : savedDirectory->second;
  try {
    checkpoint(Fake::FaultTiming::AfterFileAccessAdjusted, Effect::Uncertain);
    if (directory)
      state.overlay.directories.insert_or_assign(address, false);
    else
      state.overlay.files.insert_or_assign(address, std::nullopt);
    progress.visibleDeleted = true;
    // A driver-only exclusion can make the source handle the base object.
    // Its first deletion already mutates the base; no second delete follows.
    if (sourceIsPersistent) {
      if (directory)
        state.durable.directories.erase(address);
      else
        state.durable.files.erase(address);
      state.partialPersistentFiles.erase(address);
      progress.persistentDeleted = true;
    }
    checkpoint(Fake::FaultTiming::AfterVisibleDeletion, Effect::Uncertain);
    if (!sourceIsPersistent) checkpoint(Fake::FaultTiming::BeforePersistentDeletion, Effect::Uncertain);
    if (directory) {
      const auto child = [&address](const auto& target) {
        return target.volume == address.volume && !sameObjectIdentity(target.path, address.path) && objectWithin(target.path, address.path);
      };
      if (std::ranges::any_of(state.durable.files, [&](const auto& item) { return child(item.first); }) ||
          std::ranges::any_of(state.durable.directories, child))
        throw ModelFailure{ErrorKind::Conflict, "The persistent directory still has children", Effect::Uncertain};
      state.durable.directories.erase(address);
      state.overlay.directories.erase(address);
    } else {
      state.durable.files.erase(address);
      state.overlay.files.erase(address);
      state.partialPersistentFiles.erase(address);
    }
    progress.persistentDeleted = true;
    if (!sourceIsPersistent) checkpoint(Fake::FaultTiming::AfterPersistentDeletion, Effect::Uncertain);
  } catch (const ModelFailure&) {
    if (progress.persistentDeleted) throw;
    // Failure after the visible delete attempts cancellation. Both cleanup
    // calls are best effort and must never replace the original code/kind.
    if (progress.visibleDeleted && !progress.persistentDeleted) {
      progress.compensationAttempted = true;
      try {
        checkpoint(Fake::FaultTiming::DuringDeleteCompensation, Effect::Uncertain);
        if (directory) {
          if (savedDirectoryOverlay)
            state.overlay.directories.insert_or_assign(address, *savedDirectoryOverlay);
          else
            state.overlay.directories.erase(address);
        } else if (savedFileOverlay) {
          state.overlay.files.insert_or_assign(address, *savedFileOverlay);
        } else {
          state.overlay.files.erase(address);
        }
        progress.compensationSucceeded = true;
      } catch (const ModelFailure&) {
      }
    }
    progress.permissionsRestoreAttempted = true;
    try {
      checkpoint(Fake::FaultTiming::DuringDeletePermissionsRestore, Effect::Uncertain);
      progress.permissionsRestoreSucceeded = true;
    } catch (const ModelFailure&) {
    }
    throw;
  }
  // Captured success goes directly to cleanup; it bypasses permission restore.
  state.runtime.usageOverride.reset();
  refreshCommitPending(state);
  return std::monostate{};
}
template <class R>
auto registryScope(const State& state, const R& request) {
  const auto key = registryObjectKey(request.key, state.runtime.currentControlSet);
  return [key, name = request.valueName](const Fake::RegistryAddress& address) {
    return sameObjectIdentity(address.key, key) && (!name || sameObjectIdentity(address.valueName, *name));
  };
}
Value perform(State& state, const CommitRegistry& request) {
  require(request.valueName.has_value(), ErrorKind::Unsupported, "The captured registry driver does not support whole-key commit");
  validateRegistryCommitTarget(registryKey(request.key), request.valueName);
  const auto address = registryAddress(request.key, *request.valueName, state.runtime.currentControlSet);
  require(visible(state.durable.registry, state.overlay.registry, address).has_value(), ErrorKind::NotFound, "The registry target does not exist");
  if (!state.current.filterEnabled || !knownSystemVolumeProtected(state) || registryExcluded(state, registryKey(request.key), true)) return std::monostate{};
  validateRegistryCommitBuffer(request.key, request.valueName);
  if (fullCommit(state)) return std::monostate{};
  requirePersistentCommitView(state, true);
  commitEntry(state.durable.registry, state.overlay.registry, address);
  state.durable.registryKeys.insert(address.key);
  state.runtime.usageOverride.reset();
  return std::monostate{};
}
Value perform(State& state, const CommitRegistryDeletion& request, const std::function<void()>& afterVisible) {
  require(!request.valueName || !request.valueName->empty(), ErrorKind::Unsupported,
          "The captured registry driver cannot safely distinguish default-value deletion from whole-key deletion");
  validateRegistryCommitTarget(registryKey(request.key), request.valueName);
  require(state.current.filterEnabled && knownSystemVolumeProtected(state) && !registryExcluded(state, registryKey(request.key), true), ErrorKind::Conflict,
          "A deletion commit requires a target in the active overlay");
  validateRegistryCommitBuffer(request.key, request.valueName);
  // Protected deletion loads the persistent hive and synchronizes permissions
  // before deleting Current. ROM fails that prerequisite without deleting either.
  if (!fullCommit(state)) requirePersistentCommitView(state, true);
  const auto exactKey = registryObjectKey(request.key, state.runtime.currentControlSet);
  require(
      request.valueName
          ? visible(state.durable.registry, state.overlay.registry, registryAddress(exactKey, *request.valueName, state.runtime.currentControlSet)).has_value()
          : visibleRegistryKey(state, exactKey),
      ErrorKind::NotFound, "The registry deletion target is not visible");
  const auto matches = registryScope(state, request);
  const auto hasChild = [&exactKey](const auto& address) { return !sameObjectIdentity(address.key, exactKey) && objectWithin(address.key, exactKey); };
  if (!request.valueName) {
    const auto visibleChild = [&](const auto& entry) {
      return hasChild(entry.first) && visible(state.durable.registry, state.overlay.registry, entry.first).has_value();
    };
    const auto visibleKeyChild = [&](const auto& childKey) {
      return !sameObjectIdentity(childKey, exactKey) && objectWithin(childKey, exactKey) && visibleRegistryKey(state, childKey);
    };
    require(!std::ranges::any_of(state.durable.registry, visibleChild) && !std::ranges::any_of(state.overlay.registry, visibleChild) &&
                !std::ranges::any_of(state.durable.registryKeys, visibleKeyChild) &&
                !std::ranges::any_of(state.overlay.registryKeys, [&](const auto& item) { return visibleKeyChild(item.first); }),
            ErrorKind::Conflict, "Registry key deletion is nonrecursive and requires its subkeys to be deleted first");
  }
  // The captured driver removes the visible target before deleting the base
  // hive target. A failure between these stages must remain uncertain.
  for (const auto& [address, contents] : state.durable.registry) {
    static_cast<void>(contents);
    if (matches(address)) state.overlay.registry.insert_or_assign(address, std::nullopt);
  }
  for (auto& [address, contents] : state.overlay.registry)
    if (matches(address)) contents.reset();
  if (!request.valueName) state.overlay.registryKeys.insert_or_assign(exactKey, false);
  state.runtime.usageOverride.reset();
  afterVisible();
  if (!request.valueName && (std::ranges::any_of(state.durable.registry, [&hasChild](const auto& entry) { return hasChild(entry.first); }) ||
                             std::ranges::any_of(state.durable.registryKeys, [&](const auto& childKey) {
                               return !sameObjectIdentity(childKey, exactKey) && objectWithin(childKey, exactKey);
                             })))
    throw ModelFailure{ErrorKind::Conflict, "The persistent registry key still has subkeys", Effect::Uncertain};
  const auto entryMatches = [&matches](const auto& item) { return matches(item.first); };
  std::erase_if(state.durable.registry, entryMatches);
  std::erase_if(state.overlay.registry, entryMatches);
  if (!request.valueName) {
    state.durable.registryKeys.erase(exactKey);
    state.overlay.registryKeys.erase(exactKey);
  }
  return std::monostate{};
}
Value perform(State& state, const GetOverlayFiles& request) {
  const auto id = normalizeVolumeId(request.volume);
  requireActiveVolume(state, id);
  const auto explicitReport = state.runtime.diagnosticCandidateReports.find(id);
  if (explicitReport != state.runtime.diagnosticCandidateReports.end()) return explicitReport->second;
  require(state.runtime.diagnosticClusterSizeBytes.has_value(), ErrorKind::Unavailable, "The diagnostic cluster-size observation is unknown");
  const auto cluster = static_cast<std::uint64_t>(*state.runtime.diagnosticClusterSizeBytes);
  require(cluster != 0, ErrorKind::Protocol, "The diagnostic cluster-size observation is zero");
  std::vector<OverlayFile> result;
  for (const auto& [address, contents] : state.overlay.files) {
    if (address.volume != id) continue;
    const auto bytes = contents ? static_cast<std::uint64_t>(contents->size()) : 0;
    const auto records = bytes / cluster + (bytes % cluster != 0 ? 1 : 0);
    require(records <= std::numeric_limits<std::uint64_t>::max() / cluster, ErrorKind::ResourceExhausted, "The diagnostic extent-byte count overflows");
    result.push_back({address.path, records * cluster});
  }
  return result;
}
Value perform(State& state, const GetSwapfileSize& request) { return volume(config(state, request.session), normalizeVolumeId(request.volume)).swapfileSize; }
Value changeSwapfile(State& state, const std::string& target, const std::uint32_t value) {
  const auto id = normalizeVolumeId(target);
  const auto previous = volume(state.next, id).swapfileSize;
  if (value == 0 && previous == 0) return std::monostate{};
  const auto originalType = state.next.overlayType;
  const auto originalFlags = state.next.overlayFlags;
  changeOverlayType(state, ram);
  bool changed = originalType != state.next.overlayType || originalFlags != state.next.overlayFlags;
  for (auto& [volumeId, configured] : state.next.volumes) {
    const auto desired = volumeId == id ? value : 0;
    changed = changed || configured.swapfileSize != desired;
    configured.swapfileSize = desired;
  }
  if (value != 0) {
    try {
      changeOverlayType(state, disk);
    } catch (ModelFailure& failure) {
      if (changed) failure.effect = Effect::Uncertain;
      throw;
    }
  }
  return std::monostate{};
}
Value perform(State& state, const ResetSettings&) {
  require(state.capturedSettings.has_value(), ErrorKind::Unsupported, "No captured UWF configuration is available to restore");
  state.next = *state.capturedSettings;
  state.current.warningThreshold = state.next.warningThreshold;
  state.current.criticalThreshold = state.next.criticalThreshold;
  return std::monostate{};
}
Value perform(State& state, const UpdateWindows&) {
  ++state.windowsUpdates;
  return state.windowsUpdateOutcome;
}
Value perform(State& state, const Power& request, const Checkpoint& checkpoint) {
  require(request.action == PowerAction::Restart || request.action == PowerAction::Shutdown, ErrorKind::InvalidArgument, "Unknown power action");
  if (state.current.filterEnabled) {
    require(controlPresent(state), ErrorKind::Unavailable, "The UWF volume control device is unavailable");
    state.runtime.shutdownPending = true;
    checkpoint(Fake::FaultTiming::AfterUwfNotification, Effect::Uncertain);
  }
  checkpoint(Fake::FaultTiming::BeforeOsPowerRequest, state.current.filterEnabled ? Effect::Uncertain : Effect::None);
  state.powerRequests.push_back(request.action);
  return std::monostate{};
}

void normalizeConfiguration(Configuration& configuration) {
  std::map<std::string, Fake::VolumeConfiguration> volumes;
  for (auto& [id, configured] : configuration.volumes) {
    const auto normalized = normalizeVolumeId(configured.volume.id.empty() ? id : configured.volume.id);
    configured.volume.id = normalized;
    volumes.insert_or_assign(normalized, std::move(configured));
  }
  configuration.volumes = std::move(volumes);
}
template <class Data>
void normalizeData(Data& data, const std::uint32_t currentControlSet) {
  decltype(data.files) files;
  for (auto& [address, value] : data.files) files.emplace(fileAddress(address.volume, address.path), std::move(value));
  data.files = std::move(files);
  decltype(data.registry) registry;
  for (auto& [address, value] : data.registry) registry.emplace(registryAddress(address.key, address.valueName, currentControlSet), std::move(value));
  data.registry = std::move(registry);
  decltype(data.directories) directories;
  decltype(data.registryKeys) keys;
  if constexpr (std::is_same_v<Data, Fake::Data>) {
    for (const auto& address : data.directories) directories.insert(fileAddress(address.volume, address.path));
    for (const auto& key : data.registryKeys) keys.insert(registryObjectKey(key, currentControlSet));
    for (const auto& [address, value] : data.registry) {
      static_cast<void>(value);
      keys.insert(address.key);
    }
  } else {
    for (const auto& [address, exists] : data.directories) directories.insert_or_assign(fileAddress(address.volume, address.path), exists);
    for (const auto& [key, exists] : data.registryKeys) keys.insert_or_assign(registryObjectKey(key, currentControlSet), exists);
    for (const auto& [address, value] : data.registry)
      if (value) keys.try_emplace(address.key, true);
  }
  data.directories = std::move(directories);
  data.registryKeys = std::move(keys);
}
State normalized(State state) {
  if (state.runtime.systemVolumeId) state.runtime.systemVolumeId = normalizeVolumeId(*state.runtime.systemVolumeId);
  if (state.runtime.windowsDirectory) state.runtime.windowsDirectory = filePath(*state.runtime.windowsDirectory);
  normalizeConfiguration(state.current);
  normalizeConfiguration(state.next);
  if (state.capturedSettings) normalizeConfiguration(*state.capturedSettings);
  normalizeData(state.durable, state.runtime.currentControlSet);
  normalizeData(state.overlay, state.runtime.currentControlSet);
  decltype(state.runtime.fileCommitViews) views;
  for (const auto& [id, view] : state.runtime.fileCommitViews) views.insert_or_assign(normalizeVolumeId(id), view);
  state.runtime.fileCommitViews = std::move(views);
  decltype(state.runtime.physicalVolumes) physicalVolumes;
  for (const auto& [id, admission] : state.runtime.physicalVolumes) physicalVolumes.insert_or_assign(normalizeVolumeId(id), admission);
  state.runtime.physicalVolumes = std::move(physicalVolumes);
  decltype(state.runtime.fileMaintenancePrefixes) prefixes;
  for (auto& [id, prefix] : state.runtime.fileMaintenancePrefixes) prefixes.insert_or_assign(normalizeVolumeId(id), std::move(prefix));
  state.runtime.fileMaintenancePrefixes = std::move(prefixes);
  decltype(state.runtime.diagnosticCandidateReports) reports;
  for (auto& [id, report] : state.runtime.diagnosticCandidateReports) reports.insert_or_assign(normalizeVolumeId(id), std::move(report));
  state.runtime.diagnosticCandidateReports = std::move(reports);
  decltype(state.runtime.fullCommitFilters) filters;
  for (auto& [id, outcome] : state.runtime.fullCommitFilters) {
    const auto normalizePaths = [](auto& paths) {
      std::set<std::string> normalizedPaths;
      for (const auto& path : paths) normalizedPaths.insert(filePath(path));
      paths = std::move(normalizedPaths);
    };
    normalizePaths(outcome.completedFilePaths);
    normalizePaths(outcome.indeterminateFilePaths);
    filters.insert_or_assign(normalizeVolumeId(id), std::move(outcome));
  }
  state.runtime.fullCommitFilters = std::move(filters);
  const auto normalizeFileKeys = [](auto& entries) {
    std::decay_t<decltype(entries)> normalizedEntries;
    for (auto& [address, value] : entries) normalizedEntries.insert_or_assign(fileAddress(address.volume, address.path), std::move(value));
    entries = std::move(normalizedEntries);
  };
  normalizeFileKeys(state.partialPersistentFiles);
  normalizeFileKeys(state.fileCommitStages);
  normalizeFileKeys(state.fileDeletions);
  return state;
}
}  // namespace

bool FakeUwfBackend::FileAddressLess::operator()(const FileAddress& left, const FileAddress& right) const {
  if (left.volume != right.volume) return left.volume < right.volume;
  return compareObjectIdentity(left.path, right.path) < 0;
}
bool FakeUwfBackend::RegistryKeyLess::operator()(const std::string& left, const std::string& right) const { return compareObjectIdentity(left, right) < 0; }
bool FakeUwfBackend::RegistryAddressLess::operator()(const RegistryAddress& left, const RegistryAddress& right) const {
  const auto keyOrder = compareObjectIdentity(left.key, right.key);
  return keyOrder != 0 ? keyOrder < 0 : compareObjectIdentity(left.valueName, right.valueName) < 0;
}
FakeUwfBackend::FakeUwfBackend(std::string name) : FakeUwfBackend(State{}, std::move(name), Profile{}) {}
FakeUwfBackend::FakeUwfBackend(State state, std::string name) : FakeUwfBackend(std::move(state), std::move(name), Profile{}) {}
FakeUwfBackend::FakeUwfBackend(State state, std::string name, const Profile profile)
    : FakeUwfBackend(createSystem(std::move(state)), std::move(name), profile) {}
FakeUwfBackend::FakeUwfBackend(SystemHandle system, std::string name) : FakeUwfBackend(std::move(system), std::move(name), Profile{}) {}
FakeUwfBackend::FakeUwfBackend(SystemHandle system, std::string name, const Profile profile)
    : m_name(std::move(name)), m_profile(profile), m_system(std::move(system)) {
  if (!m_system) throw std::invalid_argument("A simulated UWF system is required");
}
FakeUwfBackend::SystemHandle FakeUwfBackend::createSystem() { return createSystem(State{}); }
FakeUwfBackend::SystemHandle FakeUwfBackend::createSystem(State state) { return std::make_shared<System>(normalized(std::move(state))); }
std::string FakeUwfBackend::name() const { return m_name; }
Capability FakeUwfBackend::capability(const Request& request) const {
  if (const auto* commit = std::get_if<CommitRegistry>(&request); commit && !commit->valueName)
    return {Support::Unsupported, "The captured registry driver does not support whole-key commit"};
  if (const auto* deletion = std::get_if<CommitRegistryDeletion>(&request); deletion && deletion->valueName && deletion->valueName->empty())
    return {Support::Unsupported, "The captured registry driver cannot safely distinguish default-value deletion from whole-key deletion"};
  const auto exact = std::ranges::find_if(m_capabilities, [&request](const auto& item) { return item.first == request; });
  if (exact != m_capabilities.end()) return exact->second;
  const auto operation = m_operationCapabilities.find(operationName(request));
  if (operation != m_operationCapabilities.end()) return operation->second;
  if (const auto* setter = std::get_if<SetBoolean>(&request); setter && setter->property == BooleanProperty::ShutdownPending)
    return {Support::Unsupported, "ShutdownPending is read-only"};
  if (m_profile.savedMode == SavedModeProfile::Unsupported) {
    const auto* get = std::get_if<GetNumber>(&request);
    const auto* set = std::get_if<SetNumber>(&request);
    if ((get && get->property == NumberProperty::ResetPersistentStateSavedMode) ||
        (set &&
         (set->property == NumberProperty::ResetPersistentStateSavedMode || (set->property == NumberProperty::ResetPersistentState && set->value == saved))))
      return {Support::Unsupported, "SavedMode is unavailable in this FakeUWF profile"};
  }
  return {Support::Supported, {}};
}
Capability FakeUwfBackend::probeRaw(const Request& request) const {
  const std::lock_guard lock(m_system->mutex);
  return capability(request);
}
std::optional<FakeUwfBackend::Fault> FakeUwfBackend::takeFault(const Request& request, const FaultTiming timing) const {
  const auto found = std::ranges::find_if(m_faults, [&request, timing](const auto& fault) {
    return fault.timing == timing && (!fault.request || *fault.request == request) && (fault.operation.empty() || fault.operation == operationName(request));
  });
  if (found == m_faults.end()) return {};
  auto result = std::move(*found);
  m_faults.erase(found);
  return result;
}
Response FakeUwfBackend::executeRaw(const Request& request, const Context& context, const ReceiptSurface surface) const {
  const std::lock_guard lock(m_system->mutex);
  m_primitiveRequests.push_back(request);
  const auto fail = [this, &request](const ErrorKind kind, std::string message, const Effect effect = Effect::None, const std::int64_t code = 0) {
    return Response::fail(kind, message, effect, {{m_name, operationName(request), code, message}});
  };
  Response response;
  const auto support = capability(request);
  if (context.stop.stop_requested())
    response = fail(ErrorKind::Canceled, "The UWF operation was canceled");
  else if (support.support != Support::Supported && support.support != Support::Unknown)
    response = fail(support.support == Support::Unavailable ? ErrorKind::Unavailable : ErrorKind::Unsupported, support.reason);
  else if (auto beforeFault = takeFault(request, FaultTiming::Before))
    response = fail(beforeFault->kind, beforeFault->message, Effect::None, beforeFault->code);
  else {
    bool writeInvoked = false;
    try {
      auto effect = isWrite(request) ? Effect::Confirmed : Effect::None;
      response.value = std::visit(
          [this, &request, &effect, &writeInvoked](const auto& typed) -> Value {
            const Checkpoint checkpoint = [this, &request](const FaultTiming timing, const Effect reachedEffect) {
              m_stages.push_back({request, timing});
              if (auto fault = takeFault(request, timing)) throw ModelFailure{fault->kind, fault->message, reachedEffect, fault->code};
            };
            writeInvoked = isWrite(request);
            if constexpr (std::is_same_v<std::decay_t<decltype(typed)>, CommitRegistryDeletion>) {
              return perform(m_system->state, typed, [&checkpoint] { checkpoint(FaultTiming::AfterVisibleDeletion, Effect::Uncertain); });
            } else if constexpr (std::is_same_v<std::decay_t<decltype(typed)>, CommitFile> ||
                                 std::is_same_v<std::decay_t<decltype(typed)>, CommitFileDeletion> || std::is_same_v<std::decay_t<decltype(typed)>, Power>) {
              return perform(m_system->state, typed, checkpoint);
            } else if constexpr (std::is_same_v<std::decay_t<decltype(typed)>, EditFileExclusion> ||
                                 std::is_same_v<std::decay_t<decltype(typed)>, EditRegistryExclusion>) {
              return perform(m_system->state, typed, effect);
            } else {
              return perform(m_system->state, typed);
            }
          },
          request);
      response.effect = effect;
      if (auto afterEffectFault = takeFault(request, FaultTiming::AfterEffect))
        response = fail(afterEffectFault->kind, afterEffectFault->message, effect == Effect::None ? Effect::None : Effect::Uncertain, afterEffectFault->code);
    } catch (const ModelFailure& failure) {
      const auto receipt = receiptFailure(failure, surface == ReceiptSurface::Captured, writeInvoked, !isWrite(request));
      response = fail(receipt.kind, receipt.message, receipt.effect, receipt.code);
    } catch (const std::bad_alloc& failure) {
      response = fail(ErrorKind::ResourceExhausted, failure.what(), writeInvoked ? Effect::Uncertain : Effect::None);
    } catch (const std::invalid_argument& failure) {
      response = fail(ErrorKind::InvalidArgument, failure.what(), surface == ReceiptSurface::Portable && writeInvoked ? Effect::Uncertain : Effect::None);
    }
  }
  return response;
}
// The facade below exercises the production shared command layer. Its
// downstream adapter dispatches only modeled native entry points, so composed
// sends never recurse through the public Fake facade.
class FakeUwfBackend::RawUwf final : public Backend {
 public:
  explicit RawUwf(const FakeUwfBackend& owner) : m_owner(owner) {}
  explicit RawUwf(std::shared_ptr<const FakeUwfBackend> owner) : m_owner(*owner), m_lifetime(std::move(owner)) {}
  std::string name() const override { return m_owner.name(); }
  UwfStateSnapshot stateSnapshot() const override { return m_owner.stateSnapshot(); }
  void updateState(UwfStateFacts facts) const override { m_owner.updateState(std::move(facts)); }
  Capability probe(const Request& request) const override { return m_owner.probeRaw(request); }
  Response execute(const Request& request, const Context& context) const override {
    if (m_lifetime) return m_owner.executeImpl(request, context, false);
    if (const auto* swap = std::get_if<CreateSwapfile>(&request)) return m_owner.writeSwapfile(swap->volume, 1, context);
    return m_owner.executeRaw(request, context);
  }

 private:
  const FakeUwfBackend& m_owner;
  std::shared_ptr<const FakeUwfBackend> m_lifetime;
};
std::shared_ptr<Backend> FakeUwfBackend::rawBackend() const {
  auto owner = weak_from_this().lock();
  if (!owner) throw std::logic_error("A raw backend view requires a shared FakeUwfBackend");
  return std::make_shared<RawUwf>(std::move(owner));
}
UwfStateFacts FakeUwfBackend::requestFacts(const Request& request) const {
  auto facts = state();
  // The fixture's default represents a published physical observation for an
  // otherwise unlisted target. Expanding it reads modeled memory only.
  std::visit(
      [&](const auto& operation) {
        if constexpr (requires { operation.volume; }) {
          const auto observed = snapshot();
          const auto id = normalizeVolumeId(operation.volume);
          const auto explicitVolume = observed.runtime.physicalVolumes.find(id);
          const auto eligibility =
              explicitVolume != observed.runtime.physicalVolumes.end() ? std::optional{explicitVolume->second} : observed.runtime.defaultVolumeEligibility;
          if (eligibility) facts.targets[id].volumeEligibility = Fact<VolumeAdmission>::known(*eligibility);
          using Operation = std::decay_t<decltype(operation)>;
          if constexpr (std::is_same_v<Operation, CommitFile> || std::is_same_v<Operation, CommitFileDeletion>) {
            facts.requestTarget = request;
            auto& target = facts.targets[id];
            const auto readiness = observed.runtime.fileCommitViews.find(id);
            const auto view = readiness == observed.runtime.fileCommitViews.end() ? observed.runtime.defaultFileCommitView : readiness->second;
            if (view != PersistentView::Unknown) target.persistentFileViewReady = Fact<bool>::known(view == PersistentView::Ready);
            const auto address = fileAddress(id, operation.path);
            FileMaintenanceFacts file;
            const auto currentVolume = facts.current.volumes.find(id);
            const auto prefix = currentVolume != facts.current.volumes.end() && currentVolume->second.driveLetter.value
                                    ? *currentVolume->second.driveLetter.value
                                    : std::string{};
            file.fullPathUtf16Units = utf16Length(prefix) + utf16Length(address.path);
            if (isVisibleDirectory(observed, address)) {
              const auto child = [&](const auto& candidate) {
                return candidate.volume == address.volume && !sameObjectIdentity(candidate.path, address.path) && objectWithin(candidate.path, address.path);
              };
              const auto hasFile = [&](const auto& entry) {
                return child(entry.first) && visible(observed.durable.files, observed.overlay.files, entry.first).has_value();
              };
              const bool nonempty = std::ranges::any_of(observed.durable.files, hasFile) || std::ranges::any_of(observed.overlay.files, hasFile) ||
                                    std::ranges::any_of(observed.durable.directories,
                                                        [&](const auto& candidate) { return child(candidate) && isVisibleDirectory(observed, candidate); }) ||
                                    std::ranges::any_of(observed.overlay.directories,
                                                        [&](const auto& entry) { return child(entry.first) && isVisibleDirectory(observed, entry.first); });
              file.kind = nonempty ? FileMaintenanceKind::NonemptyDirectory : FileMaintenanceKind::EmptyDirectory;
            } else if (visible(observed.durable.files, observed.overlay.files, address)) {
              file.kind = FileMaintenanceKind::File;
            } else if (observed.partialPersistentFiles.contains(address)) {
              target.fileTarget = {};
              return;
            } else {
              file.kind = FileMaintenanceKind::Missing;
            }
            if (file.kind != FileMaintenanceKind::EmptyDirectory && file.kind != FileMaintenanceKind::NonemptyDirectory) {
              const auto reservation = fileMaintenanceReservation(address.path);
              const bool system = observed.runtime.systemVolumeId && address.volume == *observed.runtime.systemVolumeId;
              file.reserved = reservation == FileMaintenanceReservation::AllVolumes || (system && reservation == FileMaintenanceReservation::SystemVolume);
            }
            target.fileTarget = Fact<FileMaintenanceFacts>::known(file);
          }
        }
      },
      request);
  return facts;
}
Capability FakeUwfBackend::probe(const Request& request) const {
  const auto facts = requestFacts(request);
  const auto command = app::probeUwfOperation(RawUwf(*this), request, {{}, &facts});
  if (command.support == Support::Unsupported || command.support == Support::Unavailable) return command;
  {
    const std::lock_guard lock(m_system->mutex);
    const bool overridden = std::ranges::any_of(m_capabilities, [&](const auto& item) { return item.first == request; }) ||
                            m_operationCapabilities.contains(operationName(request));
    if (overridden) return capability(request);
  }
  return command;
}
Response FakeUwfBackend::execute(const Request& request, const Context& context) const { return executeImpl(request, context, true); }
Response FakeUwfBackend::executeImpl(const Request& request, const Context& context, const bool compose) const {
  Response response;
  if (context.stop.stop_requested()) {
    response = Response::fail(ErrorKind::Canceled, "The UWF operation was canceled");
    const std::lock_guard lock(m_system->mutex);
    m_trace.push_back({request, response});
    return response;
  }
  if (const auto failure = requestInputFailure(request)) {
    response = Response::fail(failure->kind, failure->message, failure->effect, failure->diagnostics);
    const std::lock_guard lock(m_system->mutex);
    m_trace.push_back({request, response});
    return response;
  }
  const auto support = compose ? probe(request) : probeRaw(request);
  std::optional<Fault> before;
  {
    const std::lock_guard lock(m_system->mutex);
    if (!context.stop.stop_requested() && (support.support == Support::Supported || support.support == Support::Unknown))
      before = takeFault(request, FaultTiming::Before);
  }
  if (context.stop.stop_requested())
    response = Response::fail(ErrorKind::Canceled, "The UWF operation was canceled");
  else if (support.support == Support::Unsupported || support.support == Support::Unavailable)
    response = Response::fail(support.support == Support::Unsupported ? ErrorKind::Unsupported : ErrorKind::Unavailable, support.reason);
  else if (before)
    response = Response::fail(before->kind, before->message, Effect::None, {{m_name, operationName(request), before->code, before->message}});
  else {
    if (compose) {
      const auto facts = context.facts ? *context.facts : requestFacts(request);
      response = normalizeResponse(request, app::executeUwfOperation(RawUwf(*this), request, {context.stop, &facts}));
    } else {
      response = normalizeResponse(request, RawUwf(*this).execute(request, context));
    }
  }
  const std::lock_guard lock(m_system->mutex);
  if (response.succeeded()) {
    if (auto fault = takeFault(request, FaultTiming::AfterEffect))
      response = Response::fail(fault->kind, fault->message, response.effect == Effect::None ? Effect::None : Effect::Uncertain,
                                {{m_name, operationName(request), fault->code, fault->message}});
  }
  m_trace.push_back({request, response});
  return response;
}
Response FakeUwfBackend::executeCapturedPrimitiveForTest(const Request& request, const Context& context) const {
  // The captured swapfile export is a compound primitive, carried separately
  // from ordinary Request dispatch by RawUwf::execute.
  auto response = std::holds_alternative<CreateSwapfile>(request)
                      ? writeSwapfile(std::get<CreateSwapfile>(request).volume, 1, context, ReceiptSurface::Captured)
                      : executeRaw(request, context, ReceiptSurface::Captured);
  const std::lock_guard lock(m_system->mutex);
  m_trace.push_back({request, response, true});
  return response;
}
Response FakeUwfBackend::writeSwapfile(const std::string& target, const std::uint32_t value, const Context& context, const ReceiptSurface surface) const {
  const std::lock_guard lock(m_system->mutex);
  const Request request = CreateSwapfile{target};
  m_primitiveRequests.push_back(request);
  const auto fail = [this, &request](const ErrorKind kind, const std::string& message, const Effect effect = Effect::None, const std::int64_t code = 0) {
    return Response::fail(kind, message, effect, {{m_name, operationName(request), code, message}});
  };
  Response response;
  if (context.stop.stop_requested())
    response = fail(ErrorKind::Canceled, "The UWF operation was canceled");
  else if (auto beforeFault = takeFault(request, FaultTiming::Before))
    response = fail(beforeFault->kind, beforeFault->message, Effect::None, beforeFault->code);
  else {
    try {
      response = Response::success(changeSwapfile(m_system->state, target, value), Effect::Confirmed);
      if (auto fault = takeFault(request, FaultTiming::AfterEffect)) response = fail(fault->kind, fault->message, Effect::Uncertain, fault->code);
    } catch (const ModelFailure& failure) {
      const auto receipt = receiptFailure(failure, surface == ReceiptSurface::Captured, true, false);
      response = fail(receipt.kind, receipt.message, receipt.effect, receipt.code);
    } catch (const std::bad_alloc& failure) {
      response = fail(ErrorKind::ResourceExhausted, failure.what(), Effect::Uncertain);
    } catch (const std::invalid_argument& failure) {
      response = fail(ErrorKind::InvalidArgument, failure.what(), surface == ReceiptSurface::Portable ? Effect::Uncertain : Effect::None);
    }
  }
  return response;
}
void FakeUwfBackend::seed(State state) {
  auto candidate = normalized(std::move(state));
  const std::lock_guard lock(m_system->mutex);
  m_system->state = std::move(candidate);
}
FakeUwfBackend::State FakeUwfBackend::snapshot() const {
  const std::lock_guard lock(m_system->mutex);
  return m_system->state;
}
UwfStateSnapshot FakeUwfBackend::stateSnapshot() const {
  const auto observed = snapshot();
  UwfStateFacts facts;
  {
    const std::lock_guard lock(m_system->mutex);
    facts.runtime.protectScope = m_system->protectScope;
  }
  const auto copy = [&](SessionStateFacts& target, const Configuration& configuration) {
    target.filterEnabled = Fact<bool>::known(configuration.filterEnabled);
    target.servicingEnabled = Fact<bool>::known(configuration.servicingEnabled);
    target.persistDomainSecretKey = Fact<bool>::known(configuration.persistDomainSecretKey);
    target.persistTscal = Fact<bool>::known(configuration.persistTscal);
    target.overlayType = Fact<std::uint32_t>::known(configuration.overlayType);
    target.maximumOverlaySize = Fact<std::uint32_t>::known(configuration.maximumOverlaySize);
    target.overlayFlags = BitFacts::known(configuration.overlayFlags);
    target.savedMode = Fact<std::uint32_t>::known(configuration.resetPersistentStateSavedMode);
    target.registryExclusions = Fact<std::vector<std::string>>::known(configuration.registryExclusions);
    target.volumesObserved = true;
    for (const auto& [id, entry] : configuration.volumes) {
      auto& volume = target.volumes[normalizeVolumeId(id)];
      volume.present = Fact<bool>::known(true);
      volume.protectedVolume = Fact<bool>::known(entry.volume.protectedVolume);
      volume.bindByDriveLetter = Fact<bool>::known(entry.volume.bindByDriveLetter);
      volume.driveLetter = Fact<std::string>::known(entry.volume.driveLetter);
      volume.swapfileMarker = Fact<std::uint32_t>::known(entry.swapfileSize);
      volume.fileExclusions = Fact<std::vector<std::string>>::known(entry.fileExclusions);
    }
  };
  copy(facts.current, observed.current);
  copy(facts.next, observed.next);
  facts.administrator = Fact<bool>::known(observed.runtime.administrator);
  facts.runtime.horm = Fact<bool>::known(observed.runtime.hormEnabled);
  facts.runtime.shutdownPending = Fact<bool>::known(observed.runtime.shutdownPending);
  facts.runtime.hiberFile = Fact<bool>::known(observed.runtime.hiberfileExists);
  facts.runtime.currentControlSet = Fact<std::uint32_t>::known(observed.runtime.currentControlSet);
  facts.runtime.fullCommit = Fact<bool>::known((observed.runtime.romDisposition & romFullCommit) != 0);
  if (observed.runtime.shutdownPrivilegeAvailable) facts.runtime.shutdownPrivilegeAvailable = Fact<bool>::known(*observed.runtime.shutdownPrivilegeAvailable);
  if (observed.runtime.controlDeviceObservationAvailable) {
    facts.runtime.controlDevicePresent = Fact<bool>::known(controlPresent(observed));
    if (controlPresent(observed)) facts.runtime.romDisposition = BitFacts::known(observed.runtime.romDisposition);
  }
  facts.runtime.resetState = Fact<std::uint32_t>::known(observed.runtime.resetPersistentState);
  facts.runtime.warningThreshold = Fact<std::uint32_t>::known(observed.current.warningThreshold);
  facts.runtime.criticalThreshold = Fact<std::uint32_t>::known(observed.current.criticalThreshold);
  facts.capturedSettingsAvailable = Fact<bool>::known(observed.capturedSettings.has_value());
  if (observed.runtime.systemVolumeId) facts.systemVolumeId = Fact<std::string>::known(normalizeVolumeId(*observed.runtime.systemVolumeId));
  if (observed.runtime.servicingObservationAvailable) {
    facts.runtime.servicingStatus = Fact<std::uint32_t>::known(static_cast<std::uint32_t>(observed.runtime.servicingStatus));
    facts.runtime.savedServicingFilter = Fact<std::optional<bool>>::known(observed.runtime.servicingSavedFilterEnabled);
  }
  if (observed.runtime.hormFixedVolumeSafety != SafetyObservation::Unknown)
    facts.runtime.hormVolumesProtected = Fact<bool>::known(observed.runtime.hormFixedVolumeSafety == SafetyObservation::Satisfied);
  for (const auto& [id, observedVolume] : observed.runtime.physicalVolumes)
    facts.targets[normalizeVolumeId(id)].volumeEligibility = Fact<VolumeAdmission>::known(observedVolume);
  const auto physicalIdentity = [&](const std::string& target) {
    const auto id = normalizeVolumeId(target);
    for (auto* session : {&facts.current, &facts.next})
      if (!session->volumes.contains(id)) session->volumes[id].present = Fact<bool>::known(false);
    return id;
  };
  for (const auto& [id, eligibility] : observed.runtime.physicalVolumes) {
    (void)eligibility;
    (void)physicalIdentity(id);
  }
  for (const auto& [target, prefix] : observed.runtime.fileMaintenancePrefixes) {
    const auto id = physicalIdentity(target);
    facts.current.volumes[id].physicalDriveLetter = Fact<std::string>::known(prefix);
    facts.next.volumes[id].physicalDriveLetter = Fact<std::string>::known(prefix);
  }
  return std::make_shared<const UwfStateFacts>(std::move(facts));
}
void FakeUwfBackend::updateState(UwfStateFacts facts) const {
  // Configuration/runtime values remain projections of the modeled native
  // state. Only externally observed complete scopes need publication storage.
  const std::lock_guard lock(m_system->mutex);
  m_system->protectScope = std::move(facts.runtime.protectScope);
}
void FakeUwfBackend::simulateRestart() {
  const std::lock_guard lock(m_system->mutex);
  const auto reset = m_system->state.runtime.persistentRollbackState;
  const bool keep = m_system->state.current.filterEnabled && m_system->state.next.filterEnabled && persistentDisk(m_system->state.current) &&
                    persistentDisk(m_system->state.next) && reset != clear;
  if (!keep) m_system->state.overlay = {};
  m_system->state.current = m_system->state.next;
  // Successful fresh-header initialization consumes Clear. Preserve an
  // independently seeded header state 2 instead of normalizing all boots to 0.
  m_system->state.runtime.persistentRollbackState = reset == clear ? preserve : reset;
  // Restart models successful control-device initialization independently of the persistent header state.
  m_system->state.runtime.resetPersistentState = preserve;
  m_system->state.runtime.shutdownPending = false;
  m_system->state.runtime.romDisposition = 0;
  m_system->state.runtime.usageOverride.reset();
  m_system->state.runtime.diagnosticCandidateReports.clear();
  if (!m_system->state.current.filterEnabled || m_system->state.current.overlayType != ram || m_system->state.current.overlayFlags != flags::readOnlyMedia)
    m_system->state.runtime.hormEnabled = false;
  ++m_system->state.restarts;
  refreshCommitPending(m_system->state);
}
void FakeUwfBackend::seedFile(std::string id, std::string path, std::string contents, const Storage storage) {
  auto address = fileAddress(std::move(id), std::move(path));
  const std::lock_guard lock(m_system->mutex);
  if (storage == Storage::Durable) {
    m_system->state.partialPersistentFiles.erase(address);
    m_system->state.durable.files.insert_or_assign(std::move(address), std::move(contents));
  } else
    m_system->state.overlay.files.insert_or_assign(std::move(address), std::move(contents));
  m_system->state.runtime.usageOverride.reset();
  refreshCommitPending(m_system->state);
}
void FakeUwfBackend::seedDirectory(std::string id, std::string path, const Storage storage) {
  const auto address = fileAddress(std::move(id), std::move(path));
  const std::lock_guard lock(m_system->mutex);
  if (storage == Storage::Durable)
    m_system->state.durable.directories.insert(address);
  else
    m_system->state.overlay.directories.insert_or_assign(address, true);
}
void FakeUwfBackend::seedRegistryKey(std::string key, const Storage storage) {
  const std::lock_guard lock(m_system->mutex);
  key = registryObjectKey(key, m_system->state.runtime.currentControlSet);
  if (storage == Storage::Durable)
    m_system->state.durable.registryKeys.insert(key);
  else
    m_system->state.overlay.registryKeys.insert_or_assign(key, true);
}
void FakeUwfBackend::seedRegistryValue(std::string key, std::string name, std::string contents, const Storage storage) {
  const std::lock_guard lock(m_system->mutex);
  auto address = registryAddress(std::move(key), std::move(name), m_system->state.runtime.currentControlSet);
  if (storage == Storage::Durable)
    m_system->state.durable.registryKeys.insert(address.key);
  else
    m_system->state.overlay.registryKeys.insert_or_assign(address.key, true);
  if (storage == Storage::Durable)
    m_system->state.durable.registry.insert_or_assign(std::move(address), std::move(contents));
  else
    m_system->state.overlay.registry.insert_or_assign(std::move(address), std::move(contents));
  m_system->state.runtime.usageOverride.reset();
}
void FakeUwfBackend::writeFile(std::string id, std::string path, std::string contents) {
  const auto address = fileAddress(std::move(id), std::move(path));
  const std::lock_guard lock(m_system->mutex);
  applyFileWrite(m_system->state, address, std::move(contents));
}
void FakeUwfBackend::deleteFile(std::string id, std::string path) {
  const auto address = fileAddress(std::move(id), std::move(path));
  const std::lock_guard lock(m_system->mutex);
  applyFileWrite(m_system->state, address, {});
}
void FakeUwfBackend::createRegistryKey(std::string key) {
  const std::lock_guard lock(m_system->mutex);
  key = registryObjectKey(key, m_system->state.runtime.currentControlSet);
  if (uwf::simulation::visibleRegistryKey(m_system->state, key)) return;
  if (protectedRegistry(m_system->state, {key, {}}))
    m_system->state.overlay.registryKeys.insert_or_assign(key, true);
  else {
    m_system->state.durable.registryKeys.insert(key);
    m_system->state.overlay.registryKeys.erase(key);
  }
}
void FakeUwfBackend::writeRegistryValue(std::string key, std::string name, std::string contents) {
  const std::lock_guard lock(m_system->mutex);
  const auto address = registryAddress(std::move(key), std::move(name), m_system->state.runtime.currentControlSet);
  applyRegistryWrite(m_system->state, address, std::move(contents));
}
void FakeUwfBackend::deleteRegistryValue(std::string key, std::string name) {
  const std::lock_guard lock(m_system->mutex);
  const auto address = registryAddress(std::move(key), std::move(name), m_system->state.runtime.currentControlSet);
  applyRegistryWrite(m_system->state, address, {});
}
void FakeUwfBackend::deleteRegistryKey(std::string key) {
  const std::lock_guard lock(m_system->mutex);
  key = registryObjectKey(key, m_system->state.runtime.currentControlSet);
  const auto child = [&](const auto& candidate) {
    return !sameObjectIdentity(candidate, key) && objectWithin(candidate, key) && uwf::simulation::visibleRegistryKey(m_system->state, candidate);
  };
  if (std::ranges::any_of(m_system->state.durable.registryKeys, child) ||
      std::ranges::any_of(m_system->state.overlay.registryKeys, [&](const auto& item) { return child(item.first); }))
    throw std::invalid_argument("Registry key deletion is nonrecursive");
  std::set<RegistryAddress, RegistryAddressLess> addresses;
  for (const auto& [address, contents] : m_system->state.durable.registry) {
    static_cast<void>(contents);
    if (sameObjectIdentity(address.key, key)) addresses.insert(address);
  }
  for (const auto& [address, contents] : m_system->state.overlay.registry) {
    static_cast<void>(contents);
    if (sameObjectIdentity(address.key, key)) addresses.insert(address);
  }
  const bool redirected = protectedRegistry(m_system->state, registryAddress(key, {}, m_system->state.runtime.currentControlSet));
  for (const auto& address : addresses) applyRegistryWrite(m_system->state, address, {});
  if (redirected)
    m_system->state.overlay.registryKeys.insert_or_assign(key, false);
  else {
    m_system->state.durable.registryKeys.erase(key);
    m_system->state.overlay.registryKeys.erase(key);
  }
}
std::optional<std::string> FakeUwfBackend::readFile(std::string id, std::string path, const Storage storage) const {
  const auto address = fileAddress(std::move(id), std::move(path));
  const std::lock_guard lock(m_system->mutex);
  if (storage == Storage::Durable && m_system->state.partialPersistentFiles.contains(address))
    throw std::logic_error("Persistent file bytes are indeterminate; inspect partialPersistentFiles instead of inventing an exact value");
  return storage == Storage::Durable ? lookup(m_system->state.durable.files, address) : lookup(m_system->state.overlay.files, address);
}
std::optional<std::string> FakeUwfBackend::readRegistryValue(std::string key, std::string name, const Storage storage) const {
  const std::lock_guard lock(m_system->mutex);
  const auto address = registryAddress(std::move(key), std::move(name), m_system->state.runtime.currentControlSet);
  return storage == Storage::Durable ? lookup(m_system->state.durable.registry, address) : lookup(m_system->state.overlay.registry, address);
}
std::optional<std::string> FakeUwfBackend::visibleFile(std::string id, std::string path) const {
  const auto address = fileAddress(std::move(id), std::move(path));
  const std::lock_guard lock(m_system->mutex);
  if (!m_system->state.overlay.files.contains(address) && m_system->state.partialPersistentFiles.contains(address))
    throw std::logic_error("The visible persistent file bytes are indeterminate");
  return visible(m_system->state.durable.files, m_system->state.overlay.files, address);
}
bool FakeUwfBackend::visibleDirectory(std::string id, std::string path) const {
  const std::lock_guard lock(m_system->mutex);
  return isVisibleDirectory(m_system->state, fileAddress(std::move(id), std::move(path)));
}
bool FakeUwfBackend::visibleRegistryKey(std::string key) const {
  const std::lock_guard lock(m_system->mutex);
  return uwf::simulation::visibleRegistryKey(m_system->state, registryObjectKey(key, m_system->state.runtime.currentControlSet));
}
std::optional<std::string> FakeUwfBackend::visibleRegistryValue(std::string key, std::string name) const {
  const std::lock_guard lock(m_system->mutex);
  const auto address = registryAddress(std::move(key), std::move(name), m_system->state.runtime.currentControlSet);
  return visible(m_system->state.durable.registry, m_system->state.overlay.registry, address);
}
void FakeUwfBackend::setCapability(Request request, Capability support) {
  const std::lock_guard lock(m_system->mutex);
  const auto found = std::ranges::find_if(m_capabilities, [&request](const auto& item) { return item.first == request; });
  if (found == m_capabilities.end())
    m_capabilities.emplace_back(std::move(request), std::move(support));
  else
    found->second = std::move(support);
}
void FakeUwfBackend::setOperationCapability(std::string operation, Capability support) {
  const std::lock_guard lock(m_system->mutex);
  m_operationCapabilities.insert_or_assign(std::move(operation), std::move(support));
}
void FakeUwfBackend::clearCapabilityOverrides() {
  const std::lock_guard lock(m_system->mutex);
  m_capabilities.clear();
  m_operationCapabilities.clear();
}
void FakeUwfBackend::inject(Fault fault) {
  const std::lock_guard lock(m_system->mutex);
  m_faults.push_back(std::move(fault));
}
void FakeUwfBackend::clearFaults() {
  const std::lock_guard lock(m_system->mutex);
  m_faults.clear();
}
std::vector<FakeUwfBackend::TraceEntry> FakeUwfBackend::trace() const {
  const std::lock_guard lock(m_system->mutex);
  return m_trace;
}
void FakeUwfBackend::clearTrace() {
  const std::lock_guard lock(m_system->mutex);
  m_trace.clear();
}
std::vector<Request> FakeUwfBackend::primitiveRequests() const {
  const std::lock_guard lock(m_system->mutex);
  return m_primitiveRequests;
}
void FakeUwfBackend::clearPrimitiveRequests() {
  const std::lock_guard lock(m_system->mutex);
  m_primitiveRequests.clear();
}
std::vector<FakeUwfBackend::StageEntry> FakeUwfBackend::stages() const {
  const std::lock_guard lock(m_system->mutex);
  return m_stages;
}
void FakeUwfBackend::clearStages() {
  const std::lock_guard lock(m_system->mutex);
  m_stages.clear();
}
}  // namespace uwf::simulation
