// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <utility>

#include "application/configuration/contracts/ConfigurationPlan.h"
#include "application/configuration/overlay/OverlayPolicyConstraints.h"
#include "application/overlay/HormEligibility.h"
#include "application/overlay/contracts/MaintenanceValidation.h"
#include "domain/uwf/policies/ConfigurationModePolicy.h"
#include "domain/uwf/policies/OverlayWritePlan.h"

namespace uwf::app {
namespace {
[[noreturn]] void reject(OverlayProperty node, const char* message) {
  throw OverlayPolicyError({.origin = OverlayPolicyFailureOrigin::Policy,
                            .kind = OperationFailureKind::InvalidArgument,
                            .node = node,
                            .operation = "PlanConfiguration",
                            .message = message});
}
template <typename T>
T observed(const PolicyObservation<T>& field, OverlayProperty node) {
  if (field.status == PolicyObservationStatus::Available && field.value) return *field.value;
  if (field.failure) throw OverlayPolicyError(*field.failure);
  reject(node, "Required overlay baseline has not been observed");
}
}  // namespace

OverlayPolicySnapshot fillOverlayPolicyObservations(OverlayPolicySnapshot policy, const OverlayMaintenanceSnapshot& maintenance) {
  const auto fill = [](auto& field, const auto& value) {
    if (field.status == PolicyObservationStatus::Unobserved && value) field = {PolicyObservationStatus::Available, value};
  };
  fill(policy.current.filterEnabled, maintenance.filterEnabled);
  fill(policy.current.overlayType, maintenance.currentOverlayType);
  fill(policy.next.overlayType, maintenance.nextOverlayType);
  fill(policy.current.flags, maintenance.currentFlags);
  fill(policy.next.flags, maintenance.nextFlags);
  return policy;
}
OverlayPolicySnapshot configurationOverlayBaseline(const ConfigurationPlan& plan) {
  return plan.maintenance.storageVolume && plan.maintenanceSnapshot ? fillOverlayPolicyObservations(plan.policySnapshot, *plan.maintenanceSnapshot)
                                                                    : plan.policySnapshot;
}
ConfigurationOverlayTarget configurationOverlayTarget(const ConfigurationPlan& plan) {
  const auto baseline = configurationOverlayBaseline(plan);
  auto overlay = plan.snapshot.next.overlay;
  if (plan.maintenance.storageVolume && plan.maintenanceSnapshot && plan.maintenanceSnapshot->maximumOverlaySize)
    overlay.maximumSizeMb = *plan.maintenanceSnapshot->maximumOverlaySize;
  auto type = baseline.next.overlayType.value ? core::overlayTypeFromValue(*baseline.next.overlayType.value) : overlay.type;
  auto flags = baseline.next.flags.status == PolicyObservationStatus::Available ? baseline.next.flags.value : std::nullopt;
  std::uint32_t flagMask = 0;
  if (plan.maintenance.storageVolume) {
    flagMask |= core::overlayStorageFlagMask(type);
    if (flags) flags->raw = core::overlayFlagsAfterStorage(type, flags->raw);
    type = core::OverlayType::Disk;
  }
  if (plan.changes.setOverlay.type) {
    flagMask |= core::overlayTypeFlagMask(type, *plan.changes.setOverlay.type);
    if (flags) flags->raw = core::overlayFlagsAfterTypeChange(type, *plan.changes.setOverlay.type, flags->raw);
    type = *plan.changes.setOverlay.type;
  }
  if (flags) flags->raw = plan.overlayPolicy.applyFlags(flags->raw);
  if (plan.overlayPolicy.passthrough) flagMask |= OverlayFlags::Passthrough;
  if (plan.overlayPolicy.persistent) flagMask |= OverlayFlags::Persistent;
  if (plan.overlayPolicy.readOnlyMedia) flagMask |= OverlayFlags::ReadOnlyMedia;
  overlay.type = type;
  if (plan.changes.setOverlay.maximumSizeMb) overlay.maximumSizeMb = *plan.changes.setOverlay.maximumSizeMb;
  if (plan.changes.setOverlay.warningThresholdMb) overlay.warningThresholdMb = *plan.changes.setOverlay.warningThresholdMb;
  if (plan.changes.setOverlay.criticalThresholdMb) overlay.criticalThresholdMb = *plan.changes.setOverlay.criticalThresholdMb;
  const auto reset = plan.overlayPolicy.reset ? std::optional{static_cast<std::int64_t>(*plan.overlayPolicy.reset)} : baseline.resetPersistentState.value;
  return {overlay, flags, reset, flagMask};
}

std::vector<ConfigurationPhase> configurationPhases(const ConfigurationPlan& plan) {
  // Validate the whole batch before staging preparation or any filter-enabling phase.
  validateOverlayMaintenancePlan(plan);
  auto mode = core::configurationModeState(plan.snapshot);
  if (plan.policySnapshot.current.flags.value) mode.runtime.overlayFlags = plan.policySnapshot.current.flags.value->raw;
  if (plan.maintenanceSnapshot) {
    const auto& state = *plan.maintenanceSnapshot;
    if (state.filterEnabled) mode.filterEnabled = state.filterEnabled;
    if (state.systemVolumeProtected) mode.systemVolumeProtected = state.systemVolumeProtected;
    if (state.hormEnabled) mode.runtime.hormEnabled = state.hormEnabled;
    if (state.currentFlags) mode.runtime.overlayFlags = state.currentFlags->raw;
    if (state.romDisposition) mode.runtime.romDisposition = state.romDisposition;
    if (state.overlayCommit) mode.runtime.fullCommit = state.overlayCommit;
  }
  for (const auto& operation : plan.operations()) {
    using Kind = ConfigurationOperationKind;
    std::optional<core::ConfigurationWrite> write;
    switch (operation.kind) {
      case Kind::VolumeProtection:
      case Kind::VolumeBinding:
      case Kind::ProtectAllVolumes:
      case Kind::UnprotectAllVolumes:
      case Kind::AddFileExclusion:
      case Kind::RemoveFileExclusion:
      case Kind::AddRegistryExclusion:
      case Kind::RemoveRegistryExclusion:
      case Kind::DomainPersistence:
      case Kind::LicensePersistence:
        write = core::ConfigurationWrite::Ordinary;
        break;
      case Kind::WarningThreshold:
      case Kind::CriticalThreshold:
        write = core::ConfigurationWrite::Threshold;
        break;
      default:
        break;
    }
    if (!write) continue;
    const auto restriction = core::configurationWriteRestriction(mode, *write);
    if (restriction == core::ConfigurationWriteRestriction::RomVolatile || restriction == core::ConfigurationWriteRestriction::RomCommitUnavailable)
      throw OperationError(std::make_error_code(std::errc::permission_denied), core::configurationWriteReason(restriction));
  }
  auto overlayBaseline = plan.snapshot.next.overlay;
  if (plan.maintenance.storageVolume && plan.maintenanceSnapshot && plan.maintenanceSnapshot->maximumOverlaySize)
    overlayBaseline.maximumSizeMb = *plan.maintenanceSnapshot->maximumOverlaySize;
  // Storage selects a host through its own native operation. Only explicit
  // type/size edits are governed by the CLI's type/size command limits.
  core::validateOverlayTarget(overlayBaseline, plan.changes.setOverlay);
  validateFileExclusionTargets(plan.changes, plan.snapshot.next, plan.maintenance.allVolumesProtected == true);
  validateFileStagingTarget(plan);
  const auto& delta = plan.overlayPolicy;
  const auto baseline = configurationOverlayBaseline(plan);
  const auto target = configurationOverlayTarget(plan);
  if (delta.reset == PersistentReset::UseSavedMode)
    reject(OverlayProperty::ResetPersistentState, "Reset selector 255 is read-only in application configuration");
  std::vector<ConfigurationPhase> phases;
  auto settings = plan.changes;
  core::PendingChanges volumes;
  if (plan.maintenance.allVolumesProtected) {
    // Bulk intent establishes registration/protection first. Explicit per-volume
    // targets are exceptions applied afterwards, even when equal to the baseline.
    volumes.volumeProtect = std::exchange(settings.volumeProtect, {});
    volumes.volumeBindings = std::exchange(settings.volumeBindings, {});
    volumes.addFileExclusions = std::exchange(settings.addFileExclusions, {});
    volumes.removeFileExclusions = std::exchange(settings.removeFileExclusions, {});
  }
  const auto appendBulkAndVolumes = [&] {
    if (plan.maintenance.allVolumesProtected) phases.emplace_back(ConfigurationBulkProtectionPhase{*plan.maintenance.allVolumesProtected});
    if (!volumes.empty())
      phases.emplace_back(ConfigurationSettingsPhase{.settings = volumes, .bulkProtectionPrecedes = plan.maintenance.allVolumesProtected == true});
  };
  const auto appendSettings = [&] {
    if (!settings.empty() || plan.hasStagingChanges())
      phases.emplace_back(ConfigurationSettingsPhase{.settings = settings,
                                                     .staging = plan.hasStagingChanges() ? plan.staging : std::nullopt,
                                                     .stagingCompletionAlternatives = plan.stagingCompletionAlternatives});
  };
  const auto appendRemainingMaintenance = [&] {
    if (plan.maintenance.hormEnabled || plan.maintenance.fullCommit)
      phases.emplace_back(ConfigurationMaintenancePhase{{.hormEnabled = plan.maintenance.hormEnabled, .fullCommit = plan.maintenance.fullCommit}});
  };
  const bool storageChange = plan.changes.setOverlay.touchesTypeOrSize() || plan.maintenance.storageVolume.has_value();
  const bool mixed = plan.hasPolicyContext || !delta.empty() || plan.maintenance.storageVolume.has_value();
  // Without policy context, retain the standard-settings execution path.
  if (!mixed) {
    appendSettings();
    appendBulkAndVolumes();
    appendRemainingMaintenance();
    return phases;
  }
  validateOverlayPolicyPlan(plan).requireValid();
  if (storageChange && plan.snapshot.current.filter.enabled)
    reject(OverlayProperty::CurrentFilterEnabled,
           "Overlay storage changes require current UWF disabled; a next-session disable does not satisfy this condition");
  OverlayPolicyDelta before;
  OverlayPolicyDelta after = delta;
  if (delta.reset) {
    const auto value = static_cast<int>(*delta.reset);
    if (value != 0 && value != 1) reject(OverlayProperty::ResetPersistentState, "Invalid reset selector");
    (void)observed(baseline.resetPersistentState, OverlayProperty::ResetPersistentState);
  }
  if (plan.requiresOverlayFlagObservation()) {
    const auto initial = observed(baseline.next.flags, OverlayProperty::NextOverlayFlags).raw;
    const auto initialType = observed(baseline.next.overlayType, OverlayProperty::NextOverlayType);
    const auto targetType = static_cast<std::int64_t>(target.overlay.type);
    const auto overlayType = core::overlayTypeFromValue(targetType);
    if (overlayType == core::OverlayType::Unknown) reject(OverlayProperty::NextOverlayType, "Unknown target overlay type");
    if (targetType != initialType || plan.maintenance.storageVolume) {
      if (overlayType == core::OverlayType::Disk && (initial & OverlayFlags::ReadOnlyMedia) != 0) {
        before.readOnlyMedia = false;
        after.readOnlyMedia.reset();
      }
    }
  }
  if (delta.reset == PersistentReset::Preserve && baseline.resetMode() == PersistentReset::Clear) {
    phases.emplace_back(ConfigurationOverlayPolicyPhase{{.reset = PersistentReset::Preserve}});
    after.reset.reset();
  }
  if (!before.empty()) phases.emplace_back(ConfigurationOverlayPolicyPhase{before});
  // Storage is a compound RAM -> host markers -> Disk transition. Prepare its
  // capacity first, retain a separate explicit type receipt, then apply flags.
  const auto explicitType = plan.maintenance.storageVolume ? std::exchange(settings.setOverlay.type, {}) : std::nullopt;
  appendSettings();
  if (plan.maintenance.storageVolume) {
    appendBulkAndVolumes();
    phases.emplace_back(ConfigurationMaintenancePhase{{.storageVolume = plan.maintenance.storageVolume}});
    if (explicitType) {
      core::PendingChanges typeConfirmation;
      typeConfirmation.setOverlay.type = explicitType;
      phases.emplace_back(ConfigurationSettingsPhase{.settings = typeConfirmation});
    }
  }
  if (!after.empty()) phases.emplace_back(ConfigurationOverlayPolicyPhase{after});
  if (!plan.maintenance.storageVolume) appendBulkAndVolumes();
  appendRemainingMaintenance();
  return phases;
}

void validateOverlayMaintenancePlan(const ConfigurationPlan& plan) {
  const auto& changes = plan.maintenance;
  const auto invalid = [](const MaintenanceValidationIssue issue) { throw MaintenanceValidationError(issue); };
  if (plan.maintenanceSnapshot && plan.maintenanceSnapshot->hormEnabled == true) {
    const bool hormOnly = changes.hormEnabled.has_value() && !changes.storageVolume && !changes.fullCommit && !changes.allVolumesProtected &&
                          plan.changes.empty() && plan.overlayPolicy.empty() && !plan.hasStagingChanges();
    if (!hormOnly) invalid(MaintenancePlanIssue::HormActive);
  }
  if (changes.empty()) return;
  if (changes.allVolumesProtected == false && changes.hormEnabled == true) invalid(MaintenancePlanIssue::UnprotectAndHormConflict);
  if (changes.fullCommit && changes.hormEnabled == true) invalid(MaintenancePlanIssue::FullCommitAndHormConflict);
  if (changes.storageVolume && changes.hormEnabled == true) invalid(MaintenancePlanIssue::StorageAndHormConflict);
  if (changes.storageVolume && changes.fullCommit) invalid(MaintenancePlanIssue::StorageAndFullCommitConflict);
  if (changes.storageVolume && changes.storageVolume->isEmpty()) invalid(MaintenancePlanIssue::StorageVolumeRequired);
  if (changes.storageVolume && plan.changes.setOverlay.type == core::OverlayType::RAM) invalid(MaintenancePlanIssue::StorageAndRamConflict);
  if (!plan.maintenanceSnapshot) return;
  const auto& state = *plan.maintenanceSnapshot;
  if (changes.allVolumesProtected && !(*changes.allVolumesProtected ? state.protectAllCapability : state.unprotectAllCapability).canAttempt())
    invalid(MaintenancePlanIssue::BatchProtectionUnsupported);
  if (changes.storageVolume) {
    if (!state.swapCapability.canAttempt()) invalid(MaintenancePlanIssue::StorageUnsupported);
    if (state.filterEnabled == true) invalid(MaintenancePlanIssue::StorageRequiresFilterDisabled);
  }
  if (changes.fullCommit) {
    if (!state.fullCommitCapability.canAttempt()) invalid(MaintenancePlanIssue::FullCommitUnsupported);
    if (state.filterEnabled == false || (state.currentFlags && !state.currentFlags->readOnlyMedia()))
      invalid(MaintenancePlanIssue::FullCommitRequiresReadOnlyMedia);
    if (plan.changes.setFilterEnabled && *plan.changes.setFilterEnabled != plan.snapshot.next.filter.enabled)
      invalid(MaintenancePlanIssue::FullCommitFilterChange);
  }
  if (changes.hormEnabled) {
    const auto eligibility = evaluateHormEligibility(state, *changes.hormEnabled);
    if (!eligibility.allowed()) invalid(eligibility.issue);
  }
}
}  // namespace uwf::app
