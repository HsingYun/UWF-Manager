// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "application/configuration/contracts/ConfigurationPlan.h"

#include <algorithm>
#include <stdexcept>
#include <utility>

#include "application/configuration/contracts/ConfiguredExclusionIdentity.h"
#include "application/localization/ApplicationText.h"
#include "application/paths/ExclusionCoverage.h"
#include "application/paths/PathPolicy.h"
#include "application/staging/editing/FileStagingConflictPolicy.h"
#include "application/staging/persistence/FileStagingRegistry.h"
#include "domain/uwf/contracts/Types.h"
#include "domain/uwf/policies/VolumeIdentityPolicy.h"
#include "foundation/paths/DriveLetter.h"
#include "foundation/paths/RegistryPath.h"
#include "foundation/text/StringUtil.h"

namespace uwf::app {
namespace {
QStringList fileExclusionPaths(const core::SessionSnapshot& session) {
  QStringList result;
  for (const auto& [identity, patterns] : session.fileExclusions) {
    const auto id = compat::parseVolumeId(identity);
    const auto volume = std::ranges::find_if(session.volumes, [&](const auto& item) {
      return equalAsciiInsensitive(item.volumeName, identity) || (id && compat::parseVolumeId(item.volumeName) == id);
    });
    // Provider strings are literal patterns owned by this configuration, even
    // when a pattern itself resembles a GUID address. Add exactly one explicit
    // shell; a GUID-only volume remains addressable without borrowing a letter.
    const auto owner = id ? id : volume == session.volumes.end() ? std::nullopt : compat::parseVolumeId(volume->volumeName);
    if (owner) {
      for (const auto& pattern : patterns) result.append(paths::volumeFilePath({*owner, QString::fromStdString(pattern)}));
      continue;
    }
    const auto letter = volume == session.volumes.end() ? drive::normalize(identity) : core::volumeConfigurationAddress(*volume);
    if (letter.empty()) continue;
    for (const auto& pattern : patterns) result.append(QString::fromStdString(letter + pattern));
  }
  return result;
}
FileStagingVolumeIdentities fileVolumeIdentities(const core::SessionSnapshot& session, FileStagingVolumeIdentities mounted) {
  FileStagingVolumeIdentities physical;
  for (const auto& volume : session.volumes) {
    const auto id = compat::parseVolumeId(volume.volumeName);
    const auto address = core::volumeConfigurationAddress(volume);
    if (!id || address.empty()) continue;
    const auto [position, inserted] = physical.emplace(QString::fromStdString(address), *id);
    if (!inserted && position->second != *id) throw std::runtime_error("Multiple UWF configurations apply to the selected volume");
  }
  for (const auto& [address, identity] : physical) mounted.insert_or_assign(address, identity);
  return mounted;
}
QString fileTargetPath(const core::SessionSnapshot& session, const std::string& target, const std::string& path,
                       const FileStagingVolumeIdentities& observed) {
  const auto* volume = core::findVolumeConfiguration(session, target);
  const auto owner = compat::parseVolumeId(volume ? volume->volumeName : target);
  const auto address = QString::fromStdString(path);
  if (const auto resolved = paths::resolveVolumeFilePath(address, fileVolumeIdentities(session, observed), owner.value_or(std::string{}))) {
    if (owner && resolved->volume != *owner) throw std::invalid_argument("The file exclusion address belongs to a different volume");
    return paths::volumeFilePath(*resolved);
  }
  const auto letter = volume ? core::volumeConfigurationAddress(*volume) : drive::normalize(target);
  return qualifyFileExclusionPath(address, QString::fromStdString(letter));
}
}  // namespace

void validateFileStagingTarget(const ConfigurationPlan& plan) {
  if (!plan.staging || plan.staging->after.isEmpty()) return;
  // Validate the relation whenever either side is edited. Unrelated operations
  // such as disabling HORM must remain available for an existing bad baseline.
  if (!plan.hasStagingChanges() && plan.changes.addFileExclusions.empty() && plan.changes.removeFileExclusions.empty()) return;
  auto next = fileExclusionPaths(plan.snapshot.next);
  for (const auto& [drive, patterns] : plan.changes.removeFileExclusions)
    for (const auto& pattern : patterns) {
      const auto removed = fileTargetPath(plan.snapshot.next, drive, pattern, plan.staging->volumeIdentities);
      next.removeIf([&](const auto& item) { return equalConfiguredExclusions(item, removed); });
    }
  for (const auto& [drive, patterns] : plan.changes.addFileExclusions)
    for (const auto& pattern : patterns)
      next.append(fileTargetPath(plan.snapshot.next, drive, pattern, plan.staging->volumeIdentities));
  // Current exclusions govern commits before restart even when reviewed changes remove their Next entries.
  const auto rejectConflict = [&](const QStringList& patterns, const core::SessionSnapshot& session) {
    if (const auto conflict = firstFileStagingExclusionConflict(plan.staging->after, patterns, fileVolumeIdentities(session, plan.staging->volumeIdentities)))
      throw std::invalid_argument(
          ApplicationText::tr("File staging conflicts with a file exclusion:\n%1\n%2").arg(conflict->entry.path, conflict->exclusion).toStdString());
  };
  rejectConflict(fileExclusionPaths(plan.snapshot.current), plan.snapshot.current);
  rejectConflict(next, plan.snapshot.next);
}

std::string fileExclusionTargetRequirement() {
  return ApplicationText::tr("Enable volume protection in this draft before adding file exclusions.").toStdString();
}

bool ConfigurationPlan::requiresOverlayFlagObservation() const {
  // A saved staging list is independent of Next overlay options. Automatic
  // execution evaluates the loaded mode when it actually runs.
  return maintenance.storageVolume || overlayPolicy.passthrough || overlayPolicy.persistent || overlayPolicy.readOnlyMedia || changes.setOverlay.type;
}
bool nextVolumeRegistrationExists(const core::SessionSnapshot& next, const std::string_view target) {
  return core::findVolumeConfiguration(next, target) != nullptr;
}
void validateFileExclusionTargets(const core::PendingChanges& changes, const core::SessionSnapshot& next, const bool bulkProtectionPrecedes) {
  const auto validTarget = [](const auto& target) { return !drive::normalize(target).empty() || compat::parseVolumeId(target).has_value(); };
  const auto identity = [&](const std::string& target) {
    if (const auto* volume = core::findVolumeConfiguration(next, target)) return compat::normalizeVolumeId(volume->volumeName);
    return compat::parseVolumeId(target).value_or(drive::normalize(target));
  };
  for (const auto& [target, value] : changes.volumeProtect) {
    (void)value;
    if (!validTarget(target)) throw std::invalid_argument("Volume protection requires a drive letter or a complete volume GUID");
  }
  for (const auto& [target, value] : changes.volumeBindings) {
    (void)value;
    if (!validTarget(target)) throw std::invalid_argument("Volume binding requires a drive letter or a complete volume GUID");
  }
  for (const auto& [target, paths] : changes.removeFileExclusions)
    if (!paths.empty() && !validTarget(target)) throw std::invalid_argument("File exclusion changes require a drive letter or a complete volume GUID");
  for (const auto& [target, paths] : changes.addFileExclusions) {
    if (paths.empty()) continue;
    if (!validTarget(target)) throw std::invalid_argument("File exclusion changes require a drive letter or a complete volume GUID");
    const bool registered = nextVolumeRegistrationExists(next, target);
    const auto selected = identity(target);
    const bool protecting = std::ranges::any_of(changes.volumeProtect, [&](const auto& entry) { return entry.second && identity(entry.first) == selected; });
    if (!hasPlannedVolumeConfiguration(registered || bulkProtectionPrecedes, protecting)) throw std::invalid_argument(fileExclusionTargetRequirement());
  }
}

std::vector<ConfigurationOperation> ConfigurationPlan::operations() const {
  using Kind = ConfigurationOperationKind;
  std::vector<ConfigurationOperation> result;
  const auto add = [&](Kind kind, std::string target = {}) { result.push_back({kind, std::move(target)}); };
  if (hasStagingChanges()) add(Kind::Staging);
  if (changes.setFilterEnabled) add(Kind::Filter);
  if (changes.setOverlay.type) add(Kind::OverlayType);
  if (changes.setOverlay.maximumSizeMb) add(Kind::OverlaySize);
  if (changes.setOverlay.warningThresholdMb) add(Kind::WarningThreshold);
  if (changes.setOverlay.criticalThresholdMb) add(Kind::CriticalThreshold);
  for (const auto& [drive, value] : changes.volumeProtect) {
    (void)value;
    add(Kind::VolumeProtection, drive);
  }
  for (const auto& [drive, value] : changes.volumeBindings) {
    (void)value;
    add(Kind::VolumeBinding, drive);
  }
  for (const auto& [drive, paths] : changes.addFileExclusions)
    for (const auto& path : paths) add(Kind::AddFileExclusion, drive + path);
  for (const auto& [drive, paths] : changes.removeFileExclusions)
    for (const auto& path : paths) add(Kind::RemoveFileExclusion, drive + path);
  for (const auto& path : changes.addRegistryExclusions) add(Kind::AddRegistryExclusion, path);
  for (const auto& path : changes.removeRegistryExclusions) add(Kind::RemoveRegistryExclusion, path);
  if (changes.setPersistDomainSecretKey) add(Kind::DomainPersistence);
  if (changes.setPersistTSCAL) add(Kind::LicensePersistence);
  if (overlayPolicy.passthrough) add(Kind::OverlayPassthrough);
  if (overlayPolicy.persistent) add(Kind::OverlayPersistent);
  if (overlayPolicy.readOnlyMedia) add(Kind::OverlayReadOnlyMedia);
  if (overlayPolicy.reset) add(Kind::ResetPersistentState);
  if (maintenance.allVolumesProtected) add(*maintenance.allVolumesProtected ? Kind::ProtectAllVolumes : Kind::UnprotectAllVolumes);
  if (maintenance.storageVolume) add(Kind::OverlayStorage, maintenance.storageVolume->toStdString());
  if (maintenance.hormEnabled) add(Kind::Horm);
  if (maintenance.fullCommit) add(Kind::FullCommit);
  return result;
}
ConfigurationActivationTiming ConfigurationPlan::activationTiming(const ConfigurationOperationKind operation) const {
  using Timing = ConfigurationActivationTiming;
  switch (operation) {
    case ConfigurationOperationKind::Staging:
    case ConfigurationOperationKind::Horm:
    case ConfigurationOperationKind::FullCommit:
      return Timing::OnApply;
    case ConfigurationOperationKind::WarningThreshold:
    case ConfigurationOperationKind::CriticalThreshold:
      // Both setters persist Dynamic values and update the live driver when
      // UWF is running. Otherwise the next UWF startup consumes those values.
      return snapshot.current.filter.enabled ? Timing::OnApply : Timing::WhenUwfStarts;
    default:
      return Timing::AfterRestart;
  }
}

ConfigurationActivationTiming ConfigurationPlan::activationTiming() const {
  using Timing = ConfigurationActivationTiming;
  std::optional<Timing> result;
  for (const auto& operation : operations()) {
    const auto timing = activationTiming(operation.kind);
    if (result && *result != timing) return Timing::Mixed;
    result = timing;
  }
  return result.value_or(Timing::OnApply);
}

ConfigurationPlan planConfiguration(core::PendingChanges pending, core::UwfSnapshot snapshot, std::optional<FileStagingChanges> staging) {
  std::vector<ConfigurationOperation> stagingCompletion;
  const bool stagingWasEdited = staging && staging->count() > 0;
  const std::string root(kFileStagingRegistryRoot);
  const auto isRoot = [](const auto& key) { return isFileStagingRegistryRoot(QString::fromStdString(key)); };
  // Ownership recognition is deliberately separate from driver coverage. A
  // malformed trailing separator still belongs to this metadata entry, but
  // is never used as proof that its descendants bypass protection.
  const auto isOwnedRoot = [&](const auto& key) {
    auto path = regkey::expandHive(key);
    while (!path.empty() && path.back() == '\\') path.pop_back();
    return isRoot(path);
  };
  const auto retainedExclusions = [&] {
    auto retained = snapshot.next.registryExclusions;
    std::erase_if(retained, [&](const auto& key) {
      return std::ranges::any_of(pending.removeRegistryExclusions, [&](const auto& removed) { return equalAsciiInsensitive(key, removed); });
    });
    return retained;
  };
  const auto coversRoot = [&](const auto& keys) { return registryExclusionCoverage(keys, QString::fromStdString(root)).writesBypass(); };
  auto target = retainedExclusions();
  target.insert(target.end(), pending.addRegistryExclusions.begin(), pending.addRegistryExclusions.end());
  const bool removalNeedsEmptyConfirmation =
      std::ranges::any_of(pending.removeRegistryExclusions, isOwnedRoot) || (coversRoot(snapshot.next.registryExclusions) && !coversRoot(target));
  if (!staging && removalNeedsEmptyConfirmation) staging = FileStagingChanges{{}, {}, true};
  // Existing state participates in admission without creating writes. Repair
  // durability only for a staging write or an edited coverage dependency.
  if (staging && (stagingWasEdited || removalNeedsEmptyConfirmation)) {
    const auto hasRoot = [&](const auto& keys) { return std::ranges::any_of(keys, isRoot); };
    if (!staging->after.isEmpty()) std::erase_if(pending.removeRegistryExclusions, isRoot);
    auto retained = retainedExclusions();
    target = retained;
    target.insert(target.end(), pending.addRegistryExclusions.begin(), pending.addRegistryExclusions.end());
    if (staging->after.isEmpty()) {
      if (removalNeedsEmptyConfirmation) staging->persistencePending = true;
      if (stagingWasEdited && !hasRoot(pending.addRegistryExclusions))
        for (const auto& key : snapshot.next.registryExclusions)
          if (isRoot(key) && std::ranges::none_of(pending.removeRegistryExclusions, [&](const auto& removed) { return equalAsciiInsensitive(key, removed); }))
            pending.removeRegistryExclusions.push_back(key);
      if (hasRoot(snapshot.next.registryExclusions))
        for (const auto& key : pending.removeRegistryExclusions)
          if (isRoot(key)) stagingCompletion.push_back({ConfigurationOperationKind::RemoveRegistryExclusion, key});
    } else if (!registryExclusionCoverage(target, QString::fromStdString(root)).writesBypass()) {
      pending.addRegistryExclusions.push_back(root);
    }
    if (!staging->after.isEmpty() && !registryExclusionCoverage(retained, QString::fromStdString(root)).writesBypass())
      for (const auto& key : pending.addRegistryExclusions)
        if (registryExclusionCoverage(std::vector<std::string>{key}, QString::fromStdString(root)).writesBypass())
          stagingCompletion.push_back({ConfigurationOperationKind::AddRegistryExclusion, key});
    if (!stagingCompletion.empty()) staging->persistencePending = true;
  }
  return {std::move(pending), std::move(snapshot), std::move(staging), std::move(stagingCompletion)};
}

ConfigurationPlan reviewApplicationConfiguration(ConfigurationChanges changes, core::UwfSnapshot snapshot, OverlayPolicySnapshot policy,
                                                 std::optional<FileStagingChanges> staging, std::optional<OverlayMaintenanceSnapshot> maintenanceSnapshot) {
  auto plan = planConfiguration(std::move(changes.settings), std::move(snapshot), std::move(staging));
  plan.overlayPolicy = std::move(changes.overlayPolicy);
  plan.policySnapshot = std::move(policy);
  plan.hasPolicyContext = plan.policySnapshot.transportAvailable || !plan.overlayPolicy.empty();
  plan.maintenance = std::move(changes.maintenance);
  plan.maintenanceSnapshot = std::move(maintenanceSnapshot);
  return plan;
}
ConfigurationPlan planApplicationConfiguration(ConfigurationChanges changes, core::UwfSnapshot snapshot, OverlayPolicySnapshot policy,
                                               std::optional<FileStagingChanges> staging, std::optional<OverlayMaintenanceSnapshot> maintenanceSnapshot) {
  auto plan = reviewApplicationConfiguration(std::move(changes), std::move(snapshot), std::move(policy), std::move(staging), std::move(maintenanceSnapshot));
  (void)configurationPhases(plan);
  return plan;
}
}  // namespace uwf::app
