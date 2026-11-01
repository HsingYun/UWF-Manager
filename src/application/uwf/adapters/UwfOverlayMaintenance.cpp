// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "application/uwf/adapters/UwfOverlayMaintenance.h"
#include "domain/uwf/policies/ConfigurationModePolicy.h"

#include <algorithm>
#include <functional>
#include <iterator>
#include <limits>
#include <map>
#include <memory>
#include <mutex>
#include <stdexcept>

#include "application/uwf/adapters/UwfCall.h"
#include "application/uwf/adapters/UwfReadState.h"
#include "domain/uwf/contracts/UwfStateFacts.h"
#include "domain/uwf/policies/AllVolumeProtectionProjection.h"
#include "domain/uwf/policies/UwfConstraints.h"
#include "foundation/concurrency/AtomicSnapshot.h"

namespace uwf::app {
namespace {
using namespace compat;

void canceled(const std::stop_token stop) {
  if (stop.stop_requested()) throw MaintenanceValidationError(MaintenancePlanIssue::Canceled, std::errc::operation_canceled);
}
[[noreturn]] void reject(const MaintenanceValidationIssue issue, const std::errc code = std::errc::device_or_resource_busy) {
  throw MaintenanceValidationError(issue, code);
}
OverlayMaintenanceCapability capability(const Uwf& system, const Request& request) {
  try {
    const auto value = system.probe(request);
    const auto support = value.support == Support::Supported     ? OverlayMaintenanceSupport::Supported
                         : value.support == Support::Unsupported ? OverlayMaintenanceSupport::Unsupported
                         : value.support == Support::Unavailable ? OverlayMaintenanceSupport::Unavailable
                                                                 : OverlayMaintenanceSupport::Unknown;
    return {support, QString::fromStdString(value.reason)};
  } catch (const std::exception& error) {
    return {OverlayMaintenanceSupport::Unavailable, QString::fromUtf8(error.what())};
  }
}
QString observationError(const std::exception& error) {
  if (const auto* failure = dynamic_cast<const compat::Error*>(&error)) return QString::fromStdString(uwfDiagnostics(*failure));
  return QString::fromUtf8(error.what());
}

class UwfOverlayMaintenance final : public OverlayMaintenanceActions {
 public:
  UwfOverlayMaintenance(std::shared_ptr<Uwf> system, std::shared_ptr<const VolumeRepository> volumes, OverlayMaintenanceReadOnlyPorts observations,
                        std::shared_ptr<AllVolumeProtectionActions> allVolumeProtection)
      : m_system(std::move(system)),
        m_volumes(std::move(volumes)),
        m_observations(std::move(observations)),
        m_allVolumeProtection(std::move(allVolumeProtection)) {
    if (!m_system) throw std::invalid_argument("UWF maintenance requires a compatibility service");
  }
  OverlayMaintenanceSnapshot observe(const std::stop_token stop) override {
    // Explicit reads refresh the shared publisher, including mount identity.
    // snapshot() remains a pure projection while this worker collects details.
    const std::lock_guard lock(m_observationMutex);
    canceled(stop);
    const auto physical = m_volumes ? std::optional{m_volumes->read()} : std::nullopt;
    canceled(stop);
    const auto refreshed = physical ? refreshUwfStateWithPhysicalInventory(*m_system, *physical, {stop}) : m_system->refreshState({stop});
    canceled(stop);
    if (refreshed.failure || m_system->state().refreshFailure) return *snapshot();
    OverlayMaintenanceSnapshot result;
    const auto published = m_system->state();
    const auto readHostFact = [&](const auto& observation, QString& detail, auto assign) {
      if (!observation) {
        detail = QStringLiteral("No read-only host observation port was supplied.");
        return;
      }
      try {
        canceled(stop);
        auto value = observation(stop);
        canceled(stop);
        assign(std::move(value));
      } catch (const std::exception& error) {
        detail = observationError(error);
      }
    };
    readHostFact(m_observations.hibernationCapabilities, result.hibernationError, [&](const OverlayHibernationCapabilities capabilities) {
      result.hibernationCapabilities = capabilities;
      result.hibernationAvailable = capabilities.available();
    });
    // This independent detail query names blocked volumes; configuration and
    // runtime admission values themselves belong to the global publisher.
    if (published.current.filterEnabled.value != false)
      readHostFact(m_observations.hormVolumeProtection, result.allVolumesProtectedError, [&](OverlayHormVolumeProtection value) {
        if (value.allProtected && !value.blockedVolumes.empty()) throw std::runtime_error("The HORM volume observation is inconsistent");
        for (auto& blocked : value.blockedVolumes) {
          const auto id = parseVolumeId(blocked.volumeId.toStdString());
          if (!id) throw std::runtime_error("A blocked-volume observation did not contain a complete volume GUID");
          blocked.volumeId = QString::fromStdString(*id);
        }
        result.blockedVolumes = std::move(value.blockedVolumes);
        result.allVolumesProtectedError = std::move(value.detailError);
      });
    if (physical) {
      for (const auto& volume : *physical) {
        const auto id = parseVolumeId(volume.volumeName);
        if (!id) continue;
        result.volumes.push_back({QString::fromStdString(*id),
                                  QString::fromStdString(volume.driveLetter),
                                  QString::fromStdString(volume.label),
                                  volume.freeBytes,
                                  core::supportsVolumeProtection(volume.support),
                                  {},
                                  {}});
      }
      result.volumesReadable = true;
      try {
        canceled(stop);
        result.systemDrive = QString::fromStdString(m_volumes->systemDrive());
      } catch (const std::exception& error) {
        result.systemDriveError = QString::fromUtf8(error.what());
      }
    }
    result.fullCommitCapability = capability(*m_system, SetBoolean{BooleanProperty::OverlayCommit, true});
    result.hormCapability = capability(*m_system, SetBoolean{BooleanProperty::HormEnabled, published.runtime.horm.value != true});
    const std::string target = result.volumes.empty() ? "00000000-0000-0000-0000-000000000000" : result.volumes.front().id.toStdString();
    result.swapCapability = capability(*m_system, CreateSwapfile{target});
    if (!m_allVolumeProtection) {
      result.protectAllCapability = {OverlayMaintenanceSupport::Unsupported, QStringLiteral("Protect all is unavailable")};
      result.unprotectAllCapability = {OverlayMaintenanceSupport::Unsupported, QStringLiteral("Unprotect all is unavailable")};
    } else {
      // The port's explicit observation refreshes configuration. Its snapshot
      // gives us the independent capability result without another Get round.
      try {
        if (const auto observation = m_allVolumeProtection->snapshot()) {
          result.protectAllCapability = {observation->protectSupported ? OverlayMaintenanceSupport::Supported : OverlayMaintenanceSupport::Unsupported,
                                         observation->protectError};
          result.unprotectAllCapability = {observation->unprotectSupported ? OverlayMaintenanceSupport::Supported : OverlayMaintenanceSupport::Unsupported,
                                           observation->unprotectError};
          result.protectAllError = observation->protectError;
          result.unprotectAllError = observation->unprotectError;
        }
      } catch (const std::exception& error) {
        result.protectAllError = observationError(error);
        result.protectAllCapability = {OverlayMaintenanceSupport::Unavailable, result.protectAllError};
        result.unprotectAllError = result.protectAllError;
        result.unprotectAllCapability = {OverlayMaintenanceSupport::Unavailable, result.unprotectAllError};
      }
    }
    canceled(stop);
    m_snapshot.store(std::make_shared<const OverlayMaintenanceSnapshot>(std::move(result)));
    return *snapshot();
  }
  std::optional<OverlayMaintenanceSnapshot> snapshot() const override { return projectState(); }

 private:
  std::optional<OverlayMaintenanceSnapshot> projectState() const {
    const auto host = m_snapshot.load();
    auto result = host ? *host : OverlayMaintenanceSnapshot{};
    const auto state = m_system->state();
    result.filterEnabled = state.current.filterEnabled.value;
    result.systemVolumeProtected = compat::configurationModeState(state).systemVolumeProtected;
    result.overlayCommit = state.runtime.fullCommit.value;
    result.hormEnabled = state.runtime.horm.value;
    const auto number = [](const Fact<std::uint32_t>& value) -> std::optional<std::int64_t> {
      return value.value ? std::optional<std::int64_t>{*value.value} : std::nullopt;
    };
    const auto flags = [](const BitFacts& value) -> std::optional<OverlayFlags> {
      return value.knownMask == std::numeric_limits<std::uint32_t>::max() ? std::optional{OverlayFlags{value.valueBits}} : std::nullopt;
    };
    result.currentOverlayType = number(state.current.overlayType);
    result.nextOverlayType = number(state.next.overlayType);
    result.maximumOverlaySize = state.next.maximumOverlaySize.value;
    result.currentFlags = flags(state.current.overlayFlags);
    result.nextFlags = flags(state.next.overlayFlags);
    result.romDisposition = state.runtime.romDisposition.knownMask == std::numeric_limits<std::uint32_t>::max()
                                ? std::optional{state.runtime.romDisposition.valueBits}
                                : std::nullopt;
    result.allVolumesProtected = state.runtime.hormVolumesProtected.value;
    if (result.hibernationCapabilities && state.runtime.hiberFile.value) {
      result.hibernationCapabilities->hiberFilePresent = *state.runtime.hiberFile.value;
      result.hibernationAvailable = result.hibernationCapabilities->available();
    } else {
      result.hibernationAvailable.reset();
    }
    result.systemVolumeId = state.systemVolumeId.value ? std::optional{QString::fromStdString(*state.systemVolumeId.value)} : std::nullopt;
    const auto merge = [&](const SessionStateFacts& session, const bool next) {
      for (const auto& [id, observed] : session.volumes) {
        auto found = std::ranges::find_if(result.volumes, [&](const auto& volume) { return parseVolumeId(volume.id.toStdString()) == parseVolumeId(id); });
        if (found == result.volumes.end()) {
          result.volumes.push_back({QString::fromStdString(id), {}, {}, 0, !m_volumes, {}, {}});
          found = std::prev(result.volumes.end());
        }
        if (observed.physicalDriveLetter.value)
          found->driveLetter = QString::fromStdString(*observed.physicalDriveLetter.value);
        else if (found->driveLetter.isEmpty() && observed.driveLetter.value)
          found->driveLetter = QString::fromStdString(*observed.driveLetter.value);
        if (const auto target = state.targets.find(id); target != state.targets.end() && target->second.volumeEligibility.value)
          found->eligible = target->second.volumeEligibility.value->protectionEligible;
        (next ? found->nextMarker : found->currentMarker) = observed.swapfileMarker.value;
      }
    };
    merge(state.current, false);
    merge(state.next, true);
    if (!m_volumes) result.volumesReadable = state.next.volumesObserved;
    std::ranges::sort(result.volumes, {}, &OverlaySwapVolume::id);
    const auto storage = [&](const SessionStateFacts& session, const std::optional<std::int64_t> type) {
      OverlayStorageLocation location;
      if (type == 0) {
        location.origin = OverlayStorageOrigin::NotUsed;
        return location;
      }
      if (type != 1) {
        location.issue = OverlayStorageIssue::OverlayTypeUnknown;
        return location;
      }
      if (!session.volumesObserved) {
        location.issue = OverlayStorageIssue::ObservationUnavailable;
        location.diagnostics = QStringLiteral("The global configured-volume inventory is unavailable.");
        return location;
      }
      std::optional<std::string> selected;
      for (const auto& [id, observed] : session.volumes) {
        if (observed.present.value == false) continue;
        if (!observed.present.value || !observed.swapfileMarker.value) {
          location.issue = OverlayStorageIssue::MarkerUnavailable;
          location.diagnostics = QStringLiteral("The global storage marker is unavailable.");
          return location;
        }
        if (*observed.swapfileMarker.value == 0) continue;
        if (selected) {
          location.issue = OverlayStorageIssue::MultipleHosts;
          return location;
        }
        selected = id;
      }
      if (selected) {
        // Publish a location only after every registration has a readable
        // marker and the complete inventory identifies exactly one host.
        location.origin = OverlayStorageOrigin::ConfiguredMarker;
        location.volumeId = QString::fromStdString(*selected);
        const auto volume = std::ranges::find_if(result.volumes, [&](const auto& value) { return value.id == *location.volumeId; });
        if (volume != result.volumes.end()) location.driveLetter = volume->driveLetter;
      } else {
        if (!result.systemVolumeId && result.systemDrive.isEmpty()) {
          location.issue = OverlayStorageIssue::SystemDriveUnavailable;
          return location;
        }
        location.origin = OverlayStorageOrigin::SystemDriveDefault;
        location.volumeId = result.systemVolumeId;
        location.driveLetter = result.systemDrive;
      }
      return location;
    };
    result.currentStorage = storage(state.current, result.currentOverlayType);
    result.nextStorage = storage(state.next, result.nextOverlayType);
    const auto protectedState = projectAllVolumeProtection(state, true);
    const auto unprotectedState = projectAllVolumeProtection(state, false);
    result.protectScopeConfirmed = protectedState.scopeConfirmed;
    result.unprotectScopeConfirmed = unprotectedState.scopeConfirmed;
    result.allVolumesNextProtected = protectedState.value;
    result.allVolumesNextUnprotected = unprotectedState.value;
    result.protectTargetVolumes.clear();
    result.unprotectTargetVolumes.clear();
    for (const auto& volume : protectedState.volumes) result.protectTargetVolumes.push_back(QString::fromStdString(volume.volume));
    for (const auto& volume : unprotectedState.volumes) result.unprotectTargetVolumes.push_back(QString::fromStdString(volume.volume));
    if (state.refreshFailure) {
      const auto detail = QString::fromStdString(uwfDiagnostics(compat::Error(*state.refreshFailure)));
      if (!result.readError.contains(detail)) {
        if (!result.readError.isEmpty()) result.readError += QLatin1Char('\n');
        result.readError += detail;
      }
    } else if (!result.filterEnabled && !result.hormEnabled && result.readError.isEmpty()) {
      result.readError = QStringLiteral("The global UWF maintenance state is unavailable.");
    }
    return result;
  }

 public:
  OverlayMaintenanceResult setAllVolumesProtected(const bool enabled, const std::stop_token stop) override {
    return compat::synchronizedUwf(*m_system, [&] { return applyAllVolumesProtected(enabled, stop); });
  }

 private:
  OverlayMaintenanceResult applyAllVolumesProtected(const bool enabled, const std::stop_token stop) {
    OverlayMaintenanceResult result;
    if (!m_allVolumeProtection) {
      result.validationIssue = MaintenancePlanIssue::BatchProtectionUnsupported;
      result.failure = OperationFailure{OperationFailureKind::Unsupported, WriteEffect::None, {}};
      result.snapshot = *snapshot();
      return result;
    }
    try {
      canceled(stop);
      const auto receipt = m_allVolumeProtection->setProtected(enabled, stop);
      result.error = receipt.error;
      result.failure = receipt.failure;
      result.effect = receipt.effect;
      result.completionConfirmed = receipt.completionConfirmed;
      result.allVolumeItems = receipt.items;
    } catch (const MaintenanceValidationError& error) {
      result.validationIssue = error.issue();
      result.failure = operationFailure(error);
      result.effect = result.failure->effect;
    } catch (const std::exception& error) {
      result.error = QString::fromUtf8(error.what());
      result.failure = operationFailure(error);
      result.effect = result.failure->effect;
    }
    result.snapshot = *snapshot();
    return result;
  }

 public:
  OverlayMaintenanceResult fullCommit(const std::stop_token stop) override {
    return perform(stop, [&] {
      if (m_system->state().runtime.fullCommit.value == true) return WriteEffect::None;
      return callUwfWrite(*m_system, SetBoolean{BooleanProperty::OverlayCommit, true}, {stop});
    });
  }

  OverlayMaintenanceResult createSwap(const QString& volume, const std::stop_token stop) override {
    const auto id = parseVolumeId(volume.toStdString());
    return perform(stop, [&] {
      if (!id) reject(MaintenancePlanIssue::StorageVolumeRequired, std::errc::invalid_argument);
      const auto state = m_system->state();
      QString drive;
      const auto next = state.next.volumes.find(*id);
      const auto current = state.current.volumes.find(*id);
      const auto mount = [](const VolumeStateFacts& target) -> const Fact<std::string>& {
        return target.physicalDriveLetter.isKnown() ? target.physicalDriveLetter : target.driveLetter;
      };
      if (next != state.next.volumes.end() && mount(next->second).value)
        drive = QString::fromStdString(*mount(next->second).value);
      else if (current != state.current.volumes.end() && mount(current->second).value)
        drive = QString::fromStdString(*mount(current->second).value);
      return callUwfWrite(*m_system, CreateSwapfile{*id, drive.toStdString(), false}, {stop});
    });
  }

  OverlayMaintenanceResult setHorm(const bool enabled, const std::stop_token stop) override {
    return perform(stop, [&] {
      const auto current = m_system->state().runtime.horm.value;
      if (current == enabled) return WriteEffect::None;
      return callUwfWrite(*m_system, SetBoolean{BooleanProperty::HormEnabled, enabled}, {stop});
    });
  }

 private:
  template <class Write>
  OverlayMaintenanceResult perform(const std::stop_token stop, Write write) {
    return compat::synchronizedUwf(*m_system, [&] { return performState(stop, write); });
  }
  template <class Write>
  OverlayMaintenanceResult performState(const std::stop_token stop, Write write) {
    OverlayMaintenanceResult result;
    try {
      canceled(stop);
      result.effect = write();
    } catch (const MaintenanceValidationError& error) {
      result.validationIssue = error.issue();
      result.failure = operationFailure(error);
      result.effect = result.failure->effect;
    } catch (const std::exception& error) {
      result.error = QString::fromUtf8(error.what());
      result.failure = operationFailure(error);
      result.effect = result.failure->effect;
    } catch (...) {
      result.validationIssue = MaintenancePlanIssue::UnknownFailure;
      result.failure = OperationFailure{OperationFailureKind::Failed, WriteEffect::Uncertain, {}};
      result.effect = WriteEffect::Uncertain;
    }
    result.snapshot = *snapshot();
    // Preserve the entry point receipt; the memory projection is display data.
    if (!result.failure) result.completionConfirmed = true;
    return result;
  }
  std::shared_ptr<Uwf> m_system;
  std::shared_ptr<const VolumeRepository> m_volumes;
  OverlayMaintenanceReadOnlyPorts m_observations;
  std::shared_ptr<AllVolumeProtectionActions> m_allVolumeProtection;
  mutable std::mutex m_observationMutex;
  foundation::AtomicSnapshot<OverlayMaintenanceSnapshot> m_snapshot;
};
}  // namespace
std::shared_ptr<OverlayMaintenanceActions> uwfOverlayMaintenance(std::shared_ptr<compat::Uwf> system, std::shared_ptr<const VolumeRepository> volumes,
                                                                 OverlayMaintenanceReadOnlyPorts observations,
                                                                 std::shared_ptr<AllVolumeProtectionActions> allVolumeProtection) {
  return std::make_shared<UwfOverlayMaintenance>(std::move(system), std::move(volumes), std::move(observations), std::move(allVolumeProtection));
}
}  // namespace uwf::app
