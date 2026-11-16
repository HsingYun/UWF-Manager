// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <variant>

#include "application/configuration/contracts/ConfigurationChanges.h"
#include "application/staging/contracts/FileStagingTypes.h"
#include "domain/uwf/model/UwfModel.h"

namespace uwf::app {
enum class ConfigurationOperationKind {
  Staging,
  Filter,
  OverlayType,
  OverlaySize,
  WarningThreshold,
  CriticalThreshold,
  VolumeProtection,
  VolumeBinding,
  AddFileExclusion,
  RemoveFileExclusion,
  AddRegistryExclusion,
  RemoveRegistryExclusion,
  DomainPersistence,
  LicensePersistence,
  OverlayPassthrough,
  OverlayPersistent,
  OverlayReadOnlyMedia,
  ResetPersistentState,
  ProtectAllVolumes,
  UnprotectAllVolumes,
  OverlayStorage,
  Horm,
  FullCommit
};
struct ConfigurationOperation {
  ConfigurationOperationKind kind;
  std::string target;
  bool operator==(const ConfigurationOperation&) const = default;
};
// Activation of the requested configuration, not the timing of every side
// effect (for example, preparing the overlay file while applying storage).
enum class ConfigurationActivationTiming { OnApply, WhenUwfStarts, AfterRestart, Mixed };
// The reviewed target is also the execution input. Storage never invents
// next-session exclusions independently of this plan.
struct ConfigurationPlan {
  core::PendingChanges changes;
  core::UwfSnapshot snapshot;
  // Include the unchanged saved list for dependency checks. nullopt means unobserved,
  // not empty; hasStagingChanges() separately determines whether to write.
  std::optional<FileStagingChanges> staging;
  // A saved list needs at least one of these next-session operations to
  // complete. Empty means the existing target already satisfies durability.
  std::vector<ConfigurationOperation> stagingCompletionAlternatives{};
  OverlayPolicyDelta overlayPolicy{};
  OverlayPolicySnapshot policySnapshot{};
  bool hasPolicyContext = false;
  OverlayMaintenanceChanges maintenance{};
  std::optional<OverlayMaintenanceSnapshot> maintenanceSnapshot{};
  [[nodiscard]] bool hasStagingChanges() const { return staging && staging->count() != 0; }
  // Shared planning/execution footprint for explicit overlay operations.
  [[nodiscard]] bool requiresOverlayFlagObservation() const;
  [[nodiscard]] std::vector<ConfigurationOperation> operations() const;
  [[nodiscard]] ConfigurationActivationTiming activationTiming() const;
  [[nodiscard]] ConfigurationActivationTiming activationTiming(ConfigurationOperationKind operation) const;
  [[nodiscard]] bool requiresPolicyContext() const {
    return maintenance.storageVolume || !overlayPolicy.empty() ||
           (hasPolicyContext && (changes.setOverlay.touchesTypeOrSize() || requiresOverlayFlagObservation()));
  }
  [[nodiscard]] std::size_t count() const { return ConfigurationChanges{changes, overlayPolicy, maintenance}.count() + (staging ? staging->count() : 0); }
};
struct ConfigurationOverlayTarget {
  core::OverlayConfig overlay;
  std::optional<OverlayFlags> flags;
  std::optional<std::int64_t> reset;
  // Explicit options and bits normalized by this transition are final targets;
  // unrelated provider-observed flags are not written back or overwritten.
  std::uint32_t flagMask = 0;
};
// Independent published maintenance observations can supply fields that a
// policy reader has not observed. Failed/unsupported policy fields stay so.
[[nodiscard]] OverlayPolicySnapshot fillOverlayPolicyObservations(OverlayPolicySnapshot policy, const OverlayMaintenanceSnapshot& maintenance);
[[nodiscard]] OverlayPolicySnapshot configurationOverlayBaseline(const ConfigurationPlan& plan);
// The same successful-transition model drives validation, preview and final
// target checks; explicit flag intent follows all type/storage side effects.
[[nodiscard]] ConfigurationOverlayTarget configurationOverlayTarget(const ConfigurationPlan& plan);
// Shared by editor availability and executable-plan admission. Review retains
// invalid additions; execution must not invent a volume registration for them.
[[nodiscard]] std::string fileExclusionTargetRequirement();
[[nodiscard]] bool nextVolumeRegistrationExists(const core::SessionSnapshot& next, std::string_view target);
void validateFileExclusionTargets(const core::PendingChanges& changes, const core::SessionSnapshot& next, bool bulkProtectionPrecedes = false);
void validateFileStagingTarget(const ConfigurationPlan& plan);
[[nodiscard]] ConfigurationPlan planConfiguration(core::PendingChanges changes, core::UwfSnapshot snapshot, std::optional<FileStagingChanges> staging = {});
[[nodiscard]] ConfigurationPlan planApplicationConfiguration(ConfigurationChanges changes, core::UwfSnapshot snapshot, OverlayPolicySnapshot policy,
                                                             std::optional<FileStagingChanges> staging = {},
                                                             std::optional<OverlayMaintenanceSnapshot> maintenanceSnapshot = {});
// Review preserves contradictory drafts so the user can inspect and correct
// them. Execution and executable command rendering always validate the plan.
[[nodiscard]] ConfigurationPlan reviewApplicationConfiguration(ConfigurationChanges changes, core::UwfSnapshot snapshot, OverlayPolicySnapshot policy,
                                                               std::optional<FileStagingChanges> staging = {},
                                                               std::optional<OverlayMaintenanceSnapshot> maintenanceSnapshot = {});

void validateOverlayMaintenancePlan(const ConfigurationPlan& plan);

struct ConfigurationSettingsPhase {
  core::PendingChanges settings{};
  std::optional<FileStagingChanges> staging{};
  std::vector<ConfigurationOperation> stagingCompletionAlternatives{};
  bool bulkProtectionPrecedes = false;
};
struct ConfigurationOverlayPolicyPhase {
  OverlayPolicyDelta changes;
};
struct ConfigurationBulkProtectionPhase {
  bool enabled;
};
struct ConfigurationMaintenancePhase {
  OverlayMaintenanceChanges changes;
};
using ConfigurationPhase =
    std::variant<ConfigurationSettingsPhase, ConfigurationOverlayPolicyPhase, ConfigurationBulkProtectionPhase, ConfigurationMaintenancePhase>;
// Ordered executable phases, including explicitly requested flag removals before
// changing overlay type. Throws before any execution if the target is invalid.
[[nodiscard]] std::vector<ConfigurationPhase> configurationPhases(const ConfigurationPlan& plan);
}  // namespace uwf::app
