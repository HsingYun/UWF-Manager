// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "composition/uwf/SystemUwf.h"

#include <optional>

#include "application/uwf/UwfRequestPrecheck.h"
#include "application/uwf/UwfStateUpdates.h"
#include "application/uwf/adapters/UwfReadState.h"
#include "composition/uwf/MaintenanceComposition.h"
#include "infrastructure/uwf/routing/FallbackUwf.h"
#include "infrastructure/uwf/windows/csp/CspUwfBackend.h"
#include "infrastructure/uwf/windows/execution/MtaUwf.h"
#include "infrastructure/uwf/windows/native/NativeUwfBackend.h"
#include "infrastructure/uwf/windows/wmi/WmiUwfBackend.h"
#include "infrastructure/windows/execution/MtaExecutor.h"
#include "infrastructure/windows/management/wmi/WmiRuntime.h"
#include "infrastructure/windows/storage/AllVolumeProtectionScope.h"
#include "infrastructure/windows/storage/WindowsVolumeRepository.h"

namespace uwf::composition {
namespace {
compat::Response refreshSystemState(const compat::Uwf& system, const windows::WindowsConfigurationEnvironment& environment, const bool hostInventory,
                                    const compat::Context& context) {
  using namespace compat;
  return synchronizedUwf(system, [&] {
    auto result = app::refreshUwfState(system, context);
    if (context.stop.stop_requested()) return result;
    auto state = system.state();
    const auto note = [&](Failure failure) {
      if (failure.diagnostics.empty()) failure.diagnostics.push_back({"StateRefresh", "ObserveHostState", 0, failure.message});
      if (!result.failure)
        result.failure = std::move(failure);
      else {
        result.failure->diagnostics.insert(result.failure->diagnostics.end(), failure.diagnostics.begin(), failure.diagnostics.end());
      }
    };
    const auto warn = [&](const char* name, Failure failure) {
      if (failure.kind == ErrorKind::Canceled || failure.effect != Effect::None)
        note(std::move(failure));
      else
        result.warnings.push_back(std::string(name) + ": " + failure.message);
    };
    const auto observe = [&](const char* name, const auto& read, const auto& failed) {
      if (context.stop.stop_requested()) return;
      try {
        read();
      } catch (const Error& error) {
        failed(error.failure());
      } catch (const std::exception& error) {
        failed(Failure{ErrorKind::Failed, Effect::None, std::string(name) + ": " + error.what(), {}});
      }
    };
    const auto boolean = [&](const auto& read, Fact<bool>& field, const char* name) {
      if (!read) return;
      field = {};
      observe(
          name,
          [&] {
            const auto observed = read(context);
            if (observed.failure)
              warn(name, *observed.failure);
            else if (const auto* value = std::get_if<bool>(&observed.value))
              field = Fact<bool>::known(*value);
            else
              warn(name, {ErrorKind::Protocol, Effect::None, "The host observation returned an invalid value", {}});
          },
          [&](Failure failure) { warn(name, std::move(failure)); });
    };
    // Core state failures keep the global write barrier. Capability-specific
    // observations invalidate only their own facts; consumers already handle
    // unknown prerequisites without reusing a previous successful observation.
    if (environment.isAdministrator) observe("Administrator", [&] { state.administrator = Fact<bool>::known(environment.isAdministrator()); }, note);
    if (environment.hiberFileExists) {
      state.runtime.hiberFile = {};
      observe(
          "Hibernation file", [&] { state.runtime.hiberFile = Fact<bool>::known(environment.hiberFileExists()); },
          [&](Failure failure) { warn("Hibernation file", std::move(failure)); });
    }
    if (environment.systemVolumeId)
      observe(
          "System volume",
          [&] {
            const auto id = parseVolumeId(environment.systemVolumeId());
            if (id)
              state.systemVolumeId = Fact<std::string>::known(*id);
            else
              note({ErrorKind::Protocol, Effect::None, "The system volume observation returned an invalid identity", {}});
          },
          note);
    const bool inactive = state.current.filterEnabled.value == false;
    if (inactive) {
      // A disabled boot does not create UwfvolControl. Its absence is an
      // ordinary configuration state, not a failed runtime observation.
      state.runtime.controlDevicePresent = Fact<bool>::known(false);
      state.runtime.romDisposition = {};
    } else if (environment.romDisposition)
      observe(
          "ROM disposition",
          [&] {
            const auto observed = environment.romDisposition(context);
            if (observed.failure) {
              if (observed.failure->kind == ErrorKind::NotFound && observed.failure->effect == Effect::None)
                state.runtime.controlDevicePresent = Fact<bool>::known(false);
              note(*observed.failure);
            } else if (const auto* value = std::get_if<std::uint32_t>(&observed.value)) {
              state.runtime.romDisposition = BitFacts::known(*value);
              state.runtime.controlDevicePresent = Fact<bool>::known(true);
            } else {
              note({ErrorKind::Protocol, Effect::None, "The ROM disposition observation returned an invalid value", {}});
            }
          },
          note);
    if (!inactive) boolean(environment.hormVolumeProtection, state.runtime.hormVolumesProtected, "HORM volume protection");
    boolean(environment.capturedSettingsAvailable, state.capturedSettingsAvailable, "Captured settings");
    boolean(environment.shutdownPrivilegeAvailable, state.runtime.shutdownPrivilegeAvailable, "Shutdown privilege");
    if (environment.servicingAdmission) {
      state.runtime.servicingStatus = {};
      state.runtime.savedServicingFilter = {};
      observe(
          "Servicing state",
          [&] {
            // Request the complete saved restoration state, without changing the mode.
            const auto observed = environment.servicingAdmission(false, context);
            if (const auto* value = std::get_if<ServicingAdmissionFacts>(&observed)) {
              state.runtime.servicingStatus = Fact<std::uint32_t>::known(value->status);
              state.runtime.savedServicingFilter = Fact<std::optional<bool>>::known(value->savedFilter);
            } else {
              warn("Servicing state", std::get<Failure>(observed));
            }
          },
          [&](Failure failure) { warn("Servicing state", std::move(failure)); });
    }
    if (hostInventory) {
      state.runtime.currentControlSet = {};
      observe(
          "Current control set", [&] { state.runtime.currentControlSet = Fact<std::uint32_t>::known(systemCurrentControlSet()); },
          [&](Failure failure) { warn("Current control set", std::move(failure)); });
    }
    if (environment.volumeProtectionInventory && state.current.filterEnabled.value) {
      // A complete refresh replaces discovery facts even when the previous
      // observation was known. Failed discovery cannot retain a usable old scope.
      state.runtime.protectScope = {};
      for (auto& [id, target] : state.targets) {
        (void)id;
        target.volumeEligibility = {};
      }
      observe(
          "Physical volume protection inventory",
          [&] {
            auto inventory = environment.volumeProtectionInventory(*state.current.filterEnabled.value, context);
            state.runtime.protectScope = Fact<std::vector<std::string>>::known(std::move(inventory.scope));
            for (const auto& [id, admission] : inventory.admissions)
              state.targets[normalizeVolumeId(id)].volumeEligibility = Fact<VolumeAdmission>::known(admission);
          },
          [&](Failure failure) { warn("Physical volume protection inventory", std::move(failure)); });
    }
    system.updateState(std::move(state));
    if (context.stop.stop_requested() && !result.failure) result = Response::fail(ErrorKind::Canceled, "UWF state refresh canceled");
    return result;
  });
}
void initializeSystemState(const compat::Uwf& system, const app::VolumeRepository& volumes) {
  std::optional<std::vector<core::VolumeInfo>> physical;
  try {
    physical = volumes.read();
  } catch (const std::exception&) {
    // Desktop refresh reports inventory failures. Console operations can still
    // read UWF configuration and report unavailable admission for a target.
  }
  if (physical)
    (void)app::refreshUwfStateWithPhysicalInventory(system, *physical);
  else
    (void)system.refreshState();
}
}  // namespace
std::shared_ptr<compat::Uwf> uwfWithStateRefresh(std::shared_ptr<compat::Uwf> system, const windows::WindowsConfigurationEnvironment& environment,
                                                 const bool hostInventory, std::shared_ptr<const app::VolumeRepository> volumes) {
#if defined(UWF_ENABLE_FAILURE_SOLVER)
  constexpr bool enabled = true;
#else
  constexpr bool enabled = false;
#endif
  auto operations = std::make_shared<app::PrecheckedUwf>(std::move(system), app::UwfFactsSource{}, enabled,
                                                         [environment, hostInventory](const compat::Uwf& shared, const compat::Context& context) {
                                                           return refreshSystemState(shared, environment, hostInventory, context);
                                                         });
  // Injected transports retain their existing lightweight construction contract.
  // The production singleton initializes every field through refreshState below.
  if (!hostInventory && environment.isAdministrator) {
    auto state = operations->state();
    state.administrator = compat::Fact<bool>::known(environment.isAdministrator());
    operations->updateState(std::move(state));
  }
  if (volumes) initializeSystemState(*operations, *volumes);
  return operations;
}
std::shared_ptr<compat::Uwf> systemUwf() {
  static const auto system = std::make_shared<windows::backends::MtaUwf>(
      [] {
        const auto environment = windows::windowsConfigurationEnvironment();
        return uwfWithStateRefresh(std::make_shared<compat::FallbackUwf>(std::vector<std::shared_ptr<compat::Backend>>{
                                       std::make_shared<windows::backends::WmiUwfBackend>(&windows::embeddedWmiSession),
                                       std::make_shared<windows::backends::CspUwfBackend>(), windows::backends::createNativeUwfBackend()}),
                                   environment, true, windows::systemVolumeRepository());
      },
      windows::systemMtaExecutor());
  return system;
}
std::shared_ptr<compat::Uwf> wmiUwf(windows::wmi::WmiOperations& session, windows::WindowsConfigurationEnvironment environment) {
  return std::make_shared<windows::backends::MtaUwf>(
      [accessor = session.threadAccessor(), environment = std::move(environment)]() mutable {
        return uwfWithStateRefresh(std::make_shared<windows::backends::WmiUwfBackend>(std::move(accessor)), environment);
      },
      windows::systemMtaExecutor());
}
}  // namespace uwf::composition
