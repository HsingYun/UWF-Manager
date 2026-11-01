// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "application/uwf/UwfStateUpdates.h"

#include <algorithm>
#include <optional>
#include <type_traits>
#include <utility>

#include "domain/uwf/policies/OverlayWritePlan.h"
#include "foundation/text/StringUtil.h"

namespace uwf::app {
namespace {
using namespace compat;
SessionStateFacts& session(UwfStateFacts& state, const Session value) { return value == Session::Current ? state.current : state.next; }
void boolean(UwfStateFacts& state, const BooleanProperty property, const Session scope, const bool value) {
  auto& fields = session(state, scope);
  switch (property) {
    case BooleanProperty::FilterEnabled:
      fields.filterEnabled = Fact<bool>::known(value);
      break;
    case BooleanProperty::HormEnabled:
      state.runtime.horm = Fact<bool>::known(value);
      break;
    case BooleanProperty::ShutdownPending:
      state.runtime.shutdownPending = Fact<bool>::known(value);
      break;
    case BooleanProperty::ServicingEnabled:
      fields.servicingEnabled = Fact<bool>::known(value);
      break;
    case BooleanProperty::PersistDomainSecretKey:
      fields.persistDomainSecretKey = Fact<bool>::known(value);
      break;
    case BooleanProperty::PersistTscal:
      fields.persistTscal = Fact<bool>::known(value);
      break;
    case BooleanProperty::OverlayCommit:
      state.runtime.fullCommit = Fact<bool>::known(value);
      break;
    case BooleanProperty::RegistryChangesAlwaysPersisted:
    case BooleanProperty::Count:
      break;
  }
}
void number(UwfStateFacts& state, const NumberProperty property, const Session scope, const std::uint32_t value) {
  auto& fields = session(state, scope);
  switch (property) {
    case NumberProperty::OverlayType:
      fields.overlayType = Fact<std::uint32_t>::known(value);
      break;
    case NumberProperty::MaximumOverlaySize:
      fields.maximumOverlaySize = Fact<std::uint32_t>::known(value);
      break;
    case NumberProperty::WarningThreshold:
      state.runtime.warningThreshold = Fact<std::uint32_t>::known(value);
      break;
    case NumberProperty::CriticalThreshold:
      state.runtime.criticalThreshold = Fact<std::uint32_t>::known(value);
      break;
    case NumberProperty::OverlayFlags:
      fields.overlayFlags = BitFacts::known(value);
      break;
    case NumberProperty::ResetPersistentState:
      state.runtime.resetState = Fact<std::uint32_t>::known(value);
      break;
    case NumberProperty::ResetPersistentStateSavedMode:
      fields.savedMode = Fact<std::uint32_t>::known(value);
      break;
    case NumberProperty::Count:
      break;
  }
}
bool edit(Fact<std::vector<std::string>>& list, const std::string& name, const ExclusionAction action) {
  if (action == ExclusionAction::Clear) {
    list = Fact<std::vector<std::string>>::known({});
    return true;
  }
  if (!list.value) return false;
  auto& entries = *list.value;
  const auto found = std::ranges::find_if(entries, [&](const auto& entry) { return equalAsciiInsensitive(entry, name); });
  if (action == ExclusionAction::Add && found == entries.end()) entries.push_back(name);
  if (action == ExclusionAction::Remove && found != entries.end()) entries.erase(found);
  return true;
}
void setBits(BitFacts& bits, const std::uint32_t mask, const bool enabled) {
  bits.knownMask |= mask;
  bits.valueBits = enabled ? bits.valueBits | mask : bits.valueBits & ~mask;
}
bool reset(UwfStateFacts& state) {
  if (state.runtime.controlDevicePresent.value == true) {
    // The global IOCTL can swallow a filter/header failure and acknowledge
    // success without changing its observable reset cache, even in Persistent
    // mode. Only the getter publishes that value; S_OK is not its value receipt.
    return false;
  } else if (state.runtime.controlDevicePresent.value == false) {
    if (state.next.overlayType.value == static_cast<std::uint32_t>(OverlayType::Disk)) {
      setBits(state.next.overlayFlags, flags::passthrough, true);
      return true;
    }
    return state.next.overlayType.value == static_cast<std::uint32_t>(OverlayType::Ram);
  }
  return false;
}
bool filter(UwfStateFacts& state, const bool enabled) {
  bool complete = state.current.filterEnabled.isKnown();
  if (state.current.filterEnabled.value == true) {
    state.runtime.controlDevicePresent = Fact<bool>::known(true);
    const auto rom = state.current.overlayFlags.hasAny(flags::readOnlyMedia);
    if (rom == true) setBits(state.runtime.romDisposition, disposition::filterChange, true);
    if (!rom) complete = false;
  }
  const bool resetComplete = reset(state);
  state.next.filterEnabled = Fact<bool>::known(enabled);
  return complete && resetComplete;
}
bool horm(UwfStateFacts& state, const bool enabled) {
  const auto previous = state.runtime.horm.value;
  state.runtime.controlDevicePresent = Fact<bool>::known(true);
  if (previous && *previous != enabled) {
    if (enabled) {
      state.runtime.romDisposition = BitFacts::known(disposition::horm);
      state.runtime.fullCommit = Fact<bool>::known(false);
    } else {
      setBits(state.runtime.romDisposition, disposition::filterChange, true);
    }
  }
  state.runtime.horm = Fact<bool>::known(enabled);
  return previous.has_value();
}
bool overlayType(UwfStateFacts& state, const std::uint32_t type) {
  const auto previous = state.next.overlayType.value;
  if (previous) {
    const auto before = core::overlayTypeFromValue(*previous);
    const auto after = core::overlayTypeFromValue(type);
    auto& bits = state.next.overlayFlags;
    bits.knownMask |= core::overlayTypeFlagMask(before, after);
    bits.valueBits = core::overlayFlagsAfterTypeChange(before, after, bits.valueBits);
  }
  state.next.overlayType = Fact<std::uint32_t>::known(type);
  return previous.has_value();
}
void swapfile(UwfStateFacts& state, const std::string& volume) {
  const auto previousType = state.next.overlayType.value;
  const auto selectedId = normalizeVolumeId(volume);
  for (auto& [id, observed] : state.next.volumes)
    if (observed.present.value != false) observed.swapfileMarker = Fact<std::uint32_t>::known(normalizeVolumeId(id) == selectedId ? 1u : 0u);
  state.next.volumes[selectedId].swapfileMarker = Fact<std::uint32_t>::known(1);
  state.next.overlayType = Fact<std::uint32_t>::known(static_cast<std::uint32_t>(OverlayType::Disk));
  const auto before = previousType ? core::overlayTypeFromValue(*previousType) : core::OverlayType::Unknown;
  if (before != core::OverlayType::Unknown) {
    auto& bits = state.next.overlayFlags;
    bits.knownMask |= core::overlayStorageFlagMask(before);
    bits.valueBits = core::overlayFlagsAfterStorage(before, bits.valueBits);
  } else {
    // Without the prior type, only Passthrough is a guaranteed flag receipt.
    // The required refresh determines the remaining storage effects.
    setBits(state.next.overlayFlags, flags::passthrough, true);
  }
}
}  // namespace

UwfStateUpdate recordUwfState(compat::UwfStateFacts& state, const compat::Request& request, const compat::Response& response) {
  using namespace compat;
  if (!response.succeeded()) return UwfStateUpdate::Complete;
  bool complete = true;
  std::visit(
      [&](const auto& operation) {
        using R = std::decay_t<decltype(operation)>;
        if constexpr (std::is_same_v<R, GetBoolean>)
          boolean(state, operation.property, operation.session, std::get<bool>(response.value));
        else if constexpr (std::is_same_v<R, GetNumber>)
          number(state, operation.property, operation.session, std::get<std::uint32_t>(response.value));
        else if constexpr (std::is_same_v<R, SetBoolean>) {
          if (operation.property == BooleanProperty::FilterEnabled)
            complete = filter(state, operation.value);
          else if (operation.property == BooleanProperty::HormEnabled)
            complete = horm(state, operation.value);
          else if (operation.property == BooleanProperty::OverlayCommit) {
            if (state.current.overlayFlags.hasAny(flags::readOnlyMedia) == true) {
              state.runtime.fullCommit = Fact<bool>::known(true);
              setBits(state.runtime.romDisposition, disposition::fullCommit, true);
            }
            complete = false;
          } else {
            boolean(state, operation.property, Session::Next, operation.value);
            if (operation.property == BooleanProperty::ServicingEnabled) complete = false;
          }
          // Servicing's restoration and runtime receipt are read by the explicit
          // refresh after this successful entry point, not guessed from its flag.
        } else if constexpr (std::is_same_v<R, SetNumber>) {
          if (operation.property == NumberProperty::OverlayType)
            complete = overlayType(state, operation.value);
          else if (operation.property == NumberProperty::ResetPersistentState) {
            const bool saved = operation.value == static_cast<std::uint32_t>(ResetMode::Saved);
            complete = reset(state);
            if (saved) complete = false;
          } else
            number(state, operation.property, Session::Next, operation.value);
        } else if constexpr (std::is_same_v<R, SetOverlayOption>) {
          const auto mask = operation.option == OverlayOption::Passthrough  ? flags::passthrough
                            : operation.option == OverlayOption::Persistent ? flags::persistent
                                                                            : flags::readOnlyMedia;
          setBits(state.next.overlayFlags, mask, operation.value);
        } else if constexpr (std::is_same_v<R, GetVolumes>) {
          auto& fields = session(state, operation.session);
          for (auto& [id, observed] : fields.volumes) {
            (void)id;
            observed.present = Fact<bool>::known(false);
          }
          for (const auto& observed : std::get<std::vector<Volume>>(response.value)) {
            auto& volume = fields.volumes[normalizeVolumeId(observed.id)];
            volume.present = Fact<bool>::known(true);
            volume.protectedVolume = Fact<bool>::known(observed.protectedVolume);
            volume.bindByDriveLetter = Fact<bool>::known(observed.bindByDriveLetter);
            volume.driveLetter = Fact<std::string>::known(observed.driveLetter);
          }
          fields.volumesObserved = true;
        } else if constexpr (std::is_same_v<R, GetVolumeProperty>) {
          auto& volume = session(state, operation.session).volumes[normalizeVolumeId(operation.volume)];
          volume.present = Fact<bool>::known(true);
          (operation.property == VolumeProperty::Protected ? volume.protectedVolume : volume.bindByDriveLetter) =
              Fact<bool>::known(std::get<bool>(response.value));
        } else if constexpr (std::is_same_v<R, AddVolume>) {
          auto& volume = state.next.volumes[normalizeVolumeId(operation.volume)];
          volume.present = Fact<bool>::known(true);
          volume.protectedVolume = volume.bindByDriveLetter = Fact<bool>::known(false);
          volume.fileExclusions = Fact<std::vector<std::string>>::known({});
          volume.swapfileMarker = Fact<std::uint32_t>::known(0);
          volume.driveLetter = Fact<std::string>::known(operation.driveLetter);
          complete = false;
        } else if constexpr (std::is_same_v<R, RemoveVolume>) {
          auto& volume = state.next.volumes[normalizeVolumeId(operation.volume)];
          auto physical = std::move(volume.physicalDriveLetter);
          volume = {};
          volume.physicalDriveLetter = std::move(physical);
          volume.present = Fact<bool>::known(false);
        } else if constexpr (std::is_same_v<R, SetVolume>) {
          auto& volume = state.next.volumes[normalizeVolumeId(operation.volume)];
          (operation.property == VolumeProperty::Protected ? volume.protectedVolume : volume.bindByDriveLetter) = Fact<bool>::known(operation.value);
        } else if constexpr (std::is_same_v<R, GetFileExclusions>) {
          session(state, operation.session).volumes[normalizeVolumeId(operation.volume)].fileExclusions =
              Fact<std::vector<std::string>>::known(std::get<std::vector<std::string>>(response.value));
        } else if constexpr (std::is_same_v<R, GetRegistryExclusions>) {
          session(state, operation.session).registryExclusions = Fact<std::vector<std::string>>::known(std::get<std::vector<std::string>>(response.value));
        } else if constexpr (std::is_same_v<R, EditFileExclusion>)
          complete = edit(state.next.volumes[normalizeVolumeId(operation.volume)].fileExclusions, operation.path, operation.action);
        else if constexpr (std::is_same_v<R, EditRegistryExclusion>)
          complete = edit(state.next.registryExclusions, operation.key, operation.action);
        else if constexpr (std::is_same_v<R, GetSwapfileSize>)
          session(state, operation.session).volumes[normalizeVolumeId(operation.volume)].swapfileMarker =
              Fact<std::uint32_t>::known(std::get<std::uint32_t>(response.value));
        else if constexpr (std::is_same_v<R, CreateSwapfile>) {
          swapfile(state, operation.volume);
          complete = false;
        } else if constexpr (std::is_same_v<R, Power>) {
          if (state.current.filterEnabled.value == true) state.runtime.shutdownPending = Fact<bool>::known(true);
          complete = state.current.filterEnabled.isKnown();
        }
        // ResetSettings has no portable captured-default payload. Its successful
        // native effects are published by the explicit refresh, without clearing
        // the current snapshot before any replacement observation exists.
        else if constexpr (std::is_same_v<R, ResetSettings>)
          complete = false;
      },
      request);
  return complete ? UwfStateUpdate::Complete : UwfStateUpdate::RefreshRequired;
}

compat::Response refreshUwfState(const compat::Uwf& system, const compat::Context& context) {
  using namespace compat;
  Response result = Response::success();
  auto currentFilter = system.state().current.filterEnabled.value;
  const auto read = [&](const Request& request) {
    Response response;
    try {
      response = normalizeResponse(request, system.execute(request, context));
    } catch (const Error& error) {
      response = Response::fail(error.failure().kind, error.what(), error.failure().effect, error.failure().diagnostics);
      response.attempts = error.attempts();
    } catch (const std::exception& error) {
      response = Response::fail(ErrorKind::Failed, error.what());
    } catch (...) {
      response = Response::fail(ErrorKind::Failed, "The UWF refresh raised an unknown exception");
    }
    result.attempts.insert(result.attempts.end(), response.attempts.begin(), response.attempts.end());
    result.warnings.insert(result.warnings.end(), response.warnings.begin(), response.warnings.end());
    result.effect = combineEffects(result.effect, response.effect);
    if (response.failure) {
      if (!result.failure)
        result.failure = response.failure;
      else {
        result.failure->diagnostics.insert(result.failure->diagnostics.end(), response.failure->diagnostics.begin(), response.failure->diagnostics.end());
        result.failure->diagnostics.push_back({"UwfStateRefresh", operationName(request), 0, response.failure->message});
      }
      result.failure->effect = combineEffects(result.failure->effect, response.failure->effect);
    }
    return response;
  };
  for (const auto scope : {Session::Current, Session::Next}) {
    for (const auto property :
         {BooleanProperty::FilterEnabled, BooleanProperty::ServicingEnabled, BooleanProperty::PersistDomainSecretKey, BooleanProperty::PersistTscal}) {
      if (context.stop.stop_requested()) break;
      const auto response = read(GetBoolean{property, scope});
      if (scope == Session::Current && property == BooleanProperty::FilterEnabled && response.succeeded()) currentFilter = std::get<bool>(response.value);
    }
    for (const auto property :
         {NumberProperty::OverlayType, NumberProperty::MaximumOverlaySize, NumberProperty::OverlayFlags, NumberProperty::ResetPersistentStateSavedMode}) {
      if (context.stop.stop_requested()) break;
      (void)read(GetNumber{property, scope});
    }
    if (context.stop.stop_requested()) break;
    const auto inventory = read(GetVolumes{scope});
    if (inventory.succeeded()) {
      for (const auto& volume : std::get<std::vector<Volume>>(inventory.value)) {
        if (context.stop.stop_requested()) break;
        (void)read(GetFileExclusions{volume.id, scope});
        if (context.stop.stop_requested()) break;
        (void)read(GetSwapfileSize{volume.id, scope});
      }
    }
    if (context.stop.stop_requested()) break;
    (void)read(GetRegistryExclusions{scope});
  }
  for (const auto property : {BooleanProperty::HormEnabled, BooleanProperty::OverlayCommit, BooleanProperty::ShutdownPending}) {
    if (context.stop.stop_requested()) break;
    if (property == BooleanProperty::OverlayCommit && currentFilter == false) {
      // An inactive Current filter has no effective FullCommit session. The
      // native disposition getter still requires a live control device, so this
      // derived observation must not depend on that inapplicable entry point.
      auto state = system.state();
      (void)recordUwfState(state, GetBoolean{property}, Response::success(false));
      system.updateState(std::move(state));
      continue;
    }
    (void)read(GetBoolean{property});
  }
  for (const auto property : {NumberProperty::ResetPersistentState, NumberProperty::WarningThreshold, NumberProperty::CriticalThreshold}) {
    if (context.stop.stop_requested()) break;
    (void)read(GetNumber{property});
  }
  if (context.stop.stop_requested() && !result.failure) result.failure = Failure{ErrorKind::Canceled, Effect::None, "The UWF refresh was canceled", {}};
  return result;
}
}  // namespace uwf::app
