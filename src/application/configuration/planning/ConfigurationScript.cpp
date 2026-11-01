// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "application/configuration/planning/ConfigurationScript.h"

#include <algorithm>
#include <iterator>
#include <map>
#include <set>
#include <type_traits>
#include <utility>

#include "application/localization/ApplicationText.h"
#include "application/paths/ExclusionCoverage.h"
#include "application/paths/PathPolicy.h"
#include "domain/uwf/contracts/Types.h"
#include "domain/uwf/policies/OverlayWritePlan.h"
#include "domain/uwf/policies/UwfConstraints.h"
#include "domain/uwf/policies/VolumeIdentityPolicy.h"
#include "foundation/paths/DriveLetter.h"
#include "foundation/text/StringUtil.h"

namespace uwf::app {
namespace {
UwfmgrCommand makeCommand(const UwfmgrKind kind, std::vector<std::string> arguments = {}) {
  UwfmgrCommand command;
  command.kind = kind;
  command.args = std::move(arguments);
  return command;
}

std::vector<UwfmgrCommand> renderOverlayChanges(const core::OverlayConfig& baseline, const core::OverlayConfigDelta& changes) {
  std::vector<UwfmgrCommand> commands;
  for (const auto& step : core::planOverlayWrites(baseline, changes)) {
    switch (step.field) {
      case core::OverlayField::Type:
        commands.push_back(makeCommand(UwfmgrKind::OverlaySetType, {*changes.type == core::OverlayType::RAM ? "RAM" : "Disk"}));
        break;
      case core::OverlayField::MaximumSize:
        commands.push_back(makeCommand(UwfmgrKind::OverlaySetSize, {std::to_string(*changes.maximumSizeMb)}));
        break;
      case core::OverlayField::WarningThreshold:
        commands.push_back(makeCommand(UwfmgrKind::OverlaySetWarningThreshold, {std::to_string(*changes.warningThresholdMb)}));
        break;
      case core::OverlayField::CriticalThreshold:
        commands.push_back(makeCommand(UwfmgrKind::OverlaySetCriticalThreshold, {std::to_string(*changes.criticalThresholdMb)}));
        break;
    }
    commands.back().requiresPrevious = step.requiresPrevious;
  }
  return commands;
}

core::OverlayConfigDelta writableObservedOverlay(const core::OverlayConfig& overlay) {
  core::OverlayConfigDelta result;
  result.warningThresholdMb = overlay.warningThresholdMb;
  result.criticalThresholdMb = overlay.criticalThresholdMb;
  if (core::validOverlayTarget(overlay.type, overlay.maximumSizeMb)) {
    result.type = overlay.type;
    result.maximumSizeMb = overlay.maximumSizeMb;
  }
  return result;
}

std::pair<UwfmgrKind, std::string> commandTarget(const UwfmgrCommand& command) {
  auto kind = command.kind;
  if (kind == UwfmgrKind::FilterDisable) kind = UwfmgrKind::FilterEnable;
  if (kind == UwfmgrKind::VolumeUnprotect) kind = UwfmgrKind::VolumeProtect;
  if (kind == UwfmgrKind::FileRemoveExclusion) kind = UwfmgrKind::FileAddExclusion;
  if (kind == UwfmgrKind::RegistryRemoveExclusion) kind = UwfmgrKind::RegistryAddExclusion;

  std::string argument = command.args.empty() ? std::string{} : command.args.front();
  switch (kind) {
    case UwfmgrKind::VolumeProtect:
      if (const auto identity = compat::parseVolumeId(argument))
        argument = *identity;
      else
        argument = drive::normalize(argument);
      break;
    case UwfmgrKind::FileAddExclusion:
    case UwfmgrKind::RegistryAddExclusion:
      // These targets identify original management-list entries. Canonical
      // object names would merge distinct hive spellings/trailing separators.
      argument = toLowerAscii(argument);
      break;
    default:
      argument.clear();
      break;
  }
  return {kind, std::move(argument)};
}

std::string fileVolumeAddress(const core::SessionSnapshot& session, const std::string& target) {
  const auto* volume = core::findVolumeConfiguration(session, target);
  const auto address = volume ? core::volumeConfigurationAddress(*volume) : drive::normalize(target);
  if (!address.empty()) return address;
  if (const auto identity = compat::parseVolumeId(volume ? volume->volumeName : target)) {
    auto root = compat::volumePath(*identity);
    root.pop_back();
    return root;
  }
  return {};
}
std::optional<std::string> fileCommandPath(const core::SessionSnapshot& session, const std::string& target, const std::string& path,
                                           const bool providerPattern = false) {
  const auto address = fileVolumeAddress(session, target);
  if (address.empty()) return {};
  // A snapshot contains provider patterns, not addresses. Even an apparent
  // GUID path inside one is passed as the literal suffix of this owning volume.
  auto suffix = QString::fromStdString(path);
  auto argument = address + path;
  if (!providerPattern) {
    if (const auto supplied = paths::splitVolumeFilePath(suffix)) {
      suffix = supplied->relativePath;
      if (const auto id = compat::parseVolumeId(supplied->volume)) {
        const auto* volume = core::findVolumeConfiguration(session, target);
        if (compat::parseVolumeId(volume ? volume->volumeName : target) != id) return {};
        argument = address + suffix.toStdString();
      } else {
        if (supplied->volume != drive::normalize(address)) return {};
        argument = path;
      }
    }
  }
  // Native uwfmgr passes a 0x105-WCHAR path buffer to ParsePathSpec. Its
  // complete literal suffix must fit with NUL; an empty suffix would address
  // the CSP collection rather than a file-exclusion node.
  constexpr qsizetype maximumNativeFileSuffixUnits = 260;
  if (suffix.isEmpty() || suffix.size() > maximumNativeFileSuffixUnits) return {};
  return argument;
}
bool bulkScopeExportable(const ConfigurationPlan& plan) {
  if (plan.maintenance.allVolumesProtected != false) return true;
  // Drive topology resolution may omit stale registrations from the ordinary
  // settings snapshot. Only the independent, complete Next scope can certify
  // the identities needed by an Unprotect all export (including an empty scope).
  return plan.maintenanceSnapshot && plan.maintenanceSnapshot->unprotectScopeConfirmed &&
         std::ranges::all_of(plan.maintenanceSnapshot->unprotectTargetVolumes,
                             [](const auto& volume) { return compat::parseVolumeId(volume.toStdString()).has_value(); });
}
void appendBulkCommands(const ConfigurationPlan& plan, const bool enabled, std::vector<UwfmgrCommand>& commands) {
  if (!bulkScopeExportable(plan)) return;
  if (enabled) {
    commands.push_back(makeCommand(UwfmgrKind::VolumeProtectAll));
  } else {
    // There is no native "unprotect all" command. Preserve every exact Next
    // identity, including offline/letterless rows, with valid individual CLI
    // syntax. Never substitute the current UI's mounted drive inventory.
    std::set<std::string> emitted;
    for (const auto& volume : plan.maintenanceSnapshot->unprotectTargetVolumes) {
      const auto identity = *compat::parseVolumeId(volume.toStdString());
      if (emitted.insert(identity).second) commands.push_back(makeCommand(UwfmgrKind::VolumeUnprotect, {compat::volumePath(identity)}));
    }
  }
}
bool isCurrentSessionAction(const UwfmgrKind kind) {
  switch (kind) {
    case UwfmgrKind::FilterEnableHorm:
    case UwfmgrKind::FilterDisableHorm:
    case UwfmgrKind::OverlayCommit:
    case UwfmgrKind::OverlayResetPersistentState:
      return true;
    default:
      return false;
  }
}
void appendMaintenanceCommands(const OverlayMaintenanceChanges& changes, std::vector<UwfmgrCommand>& commands) {
  if (changes.storageVolume) {
    const auto identity = compat::parseVolumeId(changes.storageVolume->toStdString());
    commands.push_back(makeCommand(UwfmgrKind::VolumeCreateSwapfile, {identity ? compat::volumePath(*identity) : changes.storageVolume->toStdString()}));
  }
  if (changes.hormEnabled) commands.push_back(makeCommand(*changes.hormEnabled ? UwfmgrKind::FilterEnableHorm : UwfmgrKind::FilterDisableHorm));
  if (changes.fullCommit) commands.push_back(makeCommand(UwfmgrKind::OverlayCommit));
}
std::vector<UwfmgrCommand> renderPendingChanges(const core::PendingChanges& c, const core::SessionSnapshot& baseline) {
  std::vector<UwfmgrCommand> out;

  if (c.setFilterEnabled) {
    out.push_back(makeCommand(*c.setFilterEnabled ? UwfmgrKind::FilterEnable : UwfmgrKind::FilterDisable));
  }

  const auto overlay = renderOverlayChanges(baseline.overlay, c.setOverlay);
  out.insert(out.end(), overlay.begin(), overlay.end());

  for (const auto& [dl, want] : c.volumeProtect) {
    const auto* configured = core::findVolumeConfiguration(baseline, dl);
    const auto identity = compat::parseVolumeId(configured ? configured->volumeName : dl);
    out.push_back(makeCommand(want ? UwfmgrKind::VolumeProtect : UwfmgrKind::VolumeUnprotect, {identity ? compat::volumePath(*identity) : dl}));
  }
  // Volume binding has no CLI representation.

  for (const auto& [target, paths] : c.addFileExclusions)
    for (const auto& path : paths)
      if (const auto argument = fileCommandPath(baseline, target, path)) out.push_back(makeCommand(UwfmgrKind::FileAddExclusion, {*argument}));
  for (const auto& [target, paths] : c.removeFileExclusions)
    for (const auto& path : paths)
      if (const auto argument = fileCommandPath(baseline, target, path)) out.push_back(makeCommand(UwfmgrKind::FileRemoveExclusion, {*argument}));
  for (const auto& k : c.addRegistryExclusions) out.push_back(makeCommand(UwfmgrKind::RegistryAddExclusion, {k}));
  for (const auto& k : c.removeRegistryExclusions) out.push_back(makeCommand(UwfmgrKind::RegistryRemoveExclusion, {k}));

  return out;
}

std::vector<UwfmgrCommand> renderSession(const core::SessionSnapshot& s) {
  std::vector<UwfmgrCommand> out;

  out.push_back(makeCommand(s.filter.enabled ? UwfmgrKind::FilterEnable : UwfmgrKind::FilterDisable));
  const auto allOverlay = writableObservedOverlay(s.overlay);
  const auto overlay = renderOverlayChanges(s.overlay, allOverlay);
  out.insert(out.end(), overlay.begin(), overlay.end());

  for (const auto& volume : s.volumes) {
    const auto id = compat::parseVolumeId(volume.volumeName);
    const auto address = core::volumeConfigurationAddress(volume);
    if (id || !address.empty())
      out.push_back(makeCommand(volume.isProtected ? UwfmgrKind::VolumeProtect : UwfmgrKind::VolumeUnprotect, {id ? compat::volumePath(*id) : address}));
  }

  // Only emit a path when uwfmgr will resolve it to this configuration's GUID.
  for (const auto& [identity, paths] : s.fileExclusions)
    for (const auto& path : paths)
      if (const auto argument = fileCommandPath(s, identity, path, true)) out.push_back(makeCommand(UwfmgrKind::FileAddExclusion, {*argument}));
  for (const auto& k : s.registryExclusions) out.push_back(makeCommand(UwfmgrKind::RegistryAddExclusion, {k}));

  return out;
}

std::vector<UwfmgrCommand> renderPlannedSession(const core::SessionSnapshot& baseline, const core::PendingChanges& changes) {
  // Render the writable target, not an abnormal old observation as an
  // intermediate executable command. A repaired target can still be exported.
  auto target = baseline;
  target.overlay.type = changes.setOverlay.type.value_or(baseline.overlay.type);
  target.overlay.maximumSizeMb = changes.setOverlay.maximumSizeMb.value_or(baseline.overlay.maximumSizeMb);
  auto commands = renderSession(target);
  std::map<std::pair<UwfmgrKind, std::string>, std::size_t> targets;
  for (std::size_t index = 0; index < commands.size(); ++index) targets.emplace(commandTarget(commands[index]), index);

  for (auto& command : renderPendingChanges(changes, baseline)) {
    const auto [position, inserted] = targets.emplace(commandTarget(command), commands.size());
    if (inserted)
      commands.push_back(std::move(command));
    else
      commands[position->second] = std::move(command);
  }
  // Replacing targets preserves old positions; rebuild overlay dependency order from the baseline.
  auto targetOverlay = target.overlay;
  targetOverlay.warningThresholdMb = changes.setOverlay.warningThresholdMb.value_or(baseline.overlay.warningThresholdMb);
  targetOverlay.criticalThresholdMb = changes.setOverlay.criticalThresholdMb.value_or(baseline.overlay.criticalThresholdMb);
  const auto allOverlay = writableObservedOverlay(targetOverlay);
  const auto overlay = renderOverlayChanges(baseline.overlay, allOverlay);
  std::copy(overlay.begin(), overlay.end(), commands.begin() + 1);
  return commands;
}

std::vector<UwfmgrCommand> renderPolicyChanges(const OverlayPolicyDelta& delta) {
  std::vector<UwfmgrCommand> commands;
  const auto add = [&](UwfmgrKind kind, std::optional<bool> value) {
    if (value) commands.push_back(makeCommand(kind, {*value ? "on" : "off"}));
  };
  add(UwfmgrKind::OverlaySetPassthrough, delta.passthrough);
  add(UwfmgrKind::OverlaySetPersistent, delta.persistent);
  add(UwfmgrKind::OverlaySetReadOnlyMedia, delta.readOnlyMedia);
  if (delta.reset) commands.push_back(makeCommand(UwfmgrKind::OverlayResetPersistentState, {*delta.reset == PersistentReset::Clear ? "on" : "off"}));
  return commands;
}
void chainConfigurationDependencies(const ConfigurationPlan& plan, std::vector<UwfmgrCommand>& commands) {
  // A script cannot recover structured errors. Stop the configuration command
  // chain after the first failing command.
  const bool createsFileTarget = std::ranges::any_of(plan.changes.addFileExclusions, [&](const auto& target) {
    return !target.second.empty() && !nextVolumeRegistrationExists(plan.snapshot.next, target.first);
  });
  if (plan.hasPolicyContext || !plan.overlayPolicy.empty() || createsFileTarget || !plan.maintenance.empty())
    for (std::size_t index = 0; index < commands.size(); ++index) commands[index].requiresPrevious = index != 0;
}
std::vector<UwfmgrCommand> renderConfigurationPlan(const ConfigurationPlan& plan) {
  std::vector<UwfmgrCommand> commands;
  for (const auto& phase : configurationPhases(plan)) {
    std::visit(
        [&](const auto& payload) {
          using Payload = std::decay_t<decltype(payload)>;
          if constexpr (std::is_same_v<Payload, ConfigurationBulkProtectionPhase>) {
            appendBulkCommands(plan, payload.enabled, commands);
          } else if constexpr (std::is_same_v<Payload, ConfigurationSettingsPhase>) {
            const auto settings = renderPendingChanges(payload.settings, plan.snapshot.next);
            commands.insert(commands.end(), settings.begin(), settings.end());
          } else if constexpr (std::is_same_v<Payload, ConfigurationOverlayPolicyPhase>) {
            const auto policy = renderPolicyChanges(payload.changes);
            commands.insert(commands.end(), policy.begin(), policy.end());
          } else if constexpr (std::is_same_v<Payload, ConfigurationMaintenancePhase>) {
            appendMaintenanceCommands(payload.changes, commands);
          }
        },
        phase);
  }
  chainConfigurationDependencies(plan, commands);
  return commands;
}

std::vector<UwfmgrCommand> renderPlannedConfiguration(const ConfigurationPlan& plan) {
  const auto phases = configurationPhases(plan);
  core::PendingChanges ordinarySettings;
  bool hasSettingsPhase = false;
  for (const auto& phase : phases) {
    if (std::holds_alternative<ConfigurationBulkProtectionPhase>(phase)) break;
    if (const auto* settings = std::get_if<ConfigurationSettingsPhase>(&phase)) {
      if (!hasSettingsPhase) ordinarySettings = settings->settings;
      hasSettingsPhase = true;
    }
  }
  std::vector<UwfmgrCommand> commands;
  const auto fullSettings = renderPlannedSession(plan.snapshot.next, ordinarySettings);
  bool settingsRendered = false;
  const auto appendSettings = [&] {
    if (settingsRendered) return;
    commands.insert(commands.end(), fullSettings.begin(), fullSettings.end());
    settingsRendered = true;
  };
  bool flagsRendered = false;
  const auto appendObservedFlags = [&] {
    if (flagsRendered) return;
    flagsRendered = true;
    // A type mutation may normalize unedited flags. Export only explicit intent in that case.
    const bool changesType =
        plan.maintenance.storageVolume.has_value() || (plan.changes.setOverlay.type && *plan.changes.setOverlay.type != plan.snapshot.next.overlay.type);
    if (changesType || plan.policySnapshot.next.flags.status != PolicyObservationStatus::Available || !plan.policySnapshot.next.flags.value) return;
    const auto flags = *plan.policySnapshot.next.flags.value;
    const auto append = [&](UwfmgrKind kind, bool value) {
      if (std::ranges::none_of(commands, [kind](const auto& command) { return command.kind == kind; }))
        commands.push_back(makeCommand(kind, {value ? "on" : "off"}));
    };
    append(UwfmgrKind::OverlaySetPassthrough, plan.overlayPolicy.passthrough.value_or(flags.passthrough()));
    append(UwfmgrKind::OverlaySetPersistent, plan.overlayPolicy.persistent.value_or(flags.persistent()));
    append(UwfmgrKind::OverlaySetReadOnlyMedia, plan.overlayPolicy.readOnlyMedia.value_or(flags.readOnlyMedia()));
  };
  bool bulkRendered = false;
  bool storageRendered = false;
  for (const auto& phase : phases) {
    std::visit(
        [&](const auto& payload) {
          using Payload = std::decay_t<decltype(payload)>;
          if constexpr (std::is_same_v<Payload, ConfigurationSettingsPhase>) {
            if (bulkRendered || storageRendered) {
              // Exceptions are explicit intent even when they restore the observed baseline.
              const auto exceptions = renderPendingChanges(payload.settings, plan.snapshot.next);
              commands.insert(commands.end(), exceptions.begin(), exceptions.end());
            } else {
              appendSettings();
            }
          } else if constexpr (std::is_same_v<Payload, ConfigurationOverlayPolicyPhase>) {
            if (!hasSettingsPhase) appendSettings();
            const auto policyCommands = renderPolicyChanges(payload.changes);
            commands.insert(commands.end(), policyCommands.begin(), policyCommands.end());
          } else if constexpr (std::is_same_v<Payload, ConfigurationBulkProtectionPhase>) {
            appendSettings();
            appendObservedFlags();
            appendBulkCommands(plan, payload.enabled, commands);
            bulkRendered = true;
          } else if constexpr (std::is_same_v<Payload, ConfigurationMaintenancePhase>) {
            if (payload.changes.storageVolume) appendSettings();
            appendMaintenanceCommands(payload.changes, commands);
            storageRendered = storageRendered || payload.changes.storageVolume.has_value();
          }
        },
        phase);
  }
  appendSettings();
  appendObservedFlags();
  return commands;
}
std::vector<ConfigurationScriptOmission> changeOmissions(const ConfigurationPlan& plan) {
  using Kind = ConfigurationScriptOmissionKind;
  std::vector<ConfigurationScriptOmission> result;
  for (const auto& [drive, binding] : plan.changes.volumeBindings) {
    (void)binding;
    result.push_back({Kind::VolumeBinding, drive});
  }
  for (const auto* edits : {&plan.changes.addFileExclusions, &plan.changes.removeFileExclusions})
    for (const auto& [target, paths] : *edits)
      if (std::ranges::any_of(paths, [&](const auto& path) { return !fileCommandPath(plan.snapshot.next, target, path); }))
        if (const ConfigurationScriptOmission omission{Kind::VolumeWithoutDrive, target}; std::ranges::find(result, omission) == result.end())
          result.push_back(omission);
  if (plan.changes.setPersistDomainSecretKey) result.push_back({Kind::DomainPersistence, {}});
  if (plan.changes.setPersistTSCAL) result.push_back({Kind::LicensePersistence, {}});
  if (plan.hasStagingChanges()) result.push_back({Kind::FileStaging, {}});
  if (!bulkScopeExportable(plan)) result.push_back({Kind::IncompleteVolumeScope, {}});
  return result;
}
ConfigurationScript expressibleScript(std::vector<UwfmgrCommand> commands, std::vector<ConfigurationScriptOmission> omissions) {
  ConfigurationScript result{{}, std::move(omissions)};
  bool predecessorOmitted = false;
  for (auto& command : commands) {
    const bool unsupported = !isUwfmgrCommandRepresentable(command);
    if (unsupported || (command.requiresPrevious && predecessorOmitted)) {
      result.omissions.push_back({unsupported ? ConfigurationScriptOmissionKind::UnrepresentableArgument : ConfigurationScriptOmissionKind::DependentCommand,
                                  command.args.empty() ? std::string{} : command.args.front()});
      predecessorOmitted = true;
    } else {
      result.commands.push_back(std::move(command));
      predecessorOmitted = false;
    }
  }
  return result;
}
}  // namespace

ConfigurationScript configurationChangeScript(const ConfigurationPlan& plan) {
  auto result = expressibleScript(renderConfigurationPlan(plan), changeOmissions(plan));
  result.exportSupported = bulkScopeExportable(plan);
  return result;
}

ConfigurationScript configurationTargetScript(const ConfigurationPlan& plan) {
  auto omissions = changeOmissions(plan);
  const auto append = [&](ConfigurationScriptOmission omission) {
    if (std::ranges::find(omissions, omission) == omissions.end()) omissions.push_back(std::move(omission));
  };
  using Kind = ConfigurationScriptOmissionKind;
  const auto& overlay = plan.snapshot.next.overlay;
  if (!core::validOverlayTarget(plan.changes.setOverlay.type.value_or(overlay.type), plan.changes.setOverlay.maximumSizeMb.value_or(overlay.maximumSizeMb)))
    append({Kind::InvalidObservedOverlayStorage, {}});
  for (const auto& volume : plan.snapshot.next.volumes) {
    const auto address = core::volumeConfigurationAddress(volume);
    const auto& identity = address.empty() ? volume.volumeName : address;
    append({Kind::VolumeBinding, identity});
    if (address.empty() && !compat::parseVolumeId(volume.volumeName) && plan.maintenance.allVolumesProtected != false)
      append({Kind::VolumeWithoutDrive, volume.volumeName});
  }
  for (const auto& [volume, paths] : plan.snapshot.next.fileExclusions)
    if (std::ranges::any_of(paths, [&](const auto& path) { return !fileCommandPath(plan.snapshot.next, volume, path, true); }))
      append({Kind::VolumeWithoutDrive, volume});
  append({Kind::DomainPersistence, {}});
  append({Kind::LicensePersistence, {}});
  auto commands = renderPlannedConfiguration(plan);
  // A reproducible next-session snapshot is applied with UWF disabled. Actions
  // admitted against the active session belong only to the Pending changes
  // product. Project the command scope before building the dependency chain,
  // including a leading reset cancellation that preceded configuration writes.
  if (std::erase_if(commands, [](const auto& command) { return isCurrentSessionAction(command.kind); }) != 0) append({Kind::CurrentSessionAction, {}});
  chainConfigurationDependencies(plan, commands);
  auto result = expressibleScript(std::move(commands), std::move(omissions));
  result.scope = ConfigurationScriptScope::NextConfiguration;
  result.exportSupported = bulkScopeExportable(plan);
  return result;
}

std::string configurationScriptScope(const ConfigurationScript& script) {
  std::string text;
  if (script.scope == ConfigurationScriptScope::NextConfiguration)
    text = ApplicationText::tr(
               "This configuration snapshot includes settings that require UWF disabled in the current session. Disable UWF and restart before running it.")
               .toStdString();
  if (script.omissions.empty()) return text;
  if (!text.empty()) text += "\n\n";
  text += ApplicationText::tr("This script contains only settings supported by uwfmgr commands. Not included:").toStdString();
  std::set<ConfigurationScriptOmissionKind> described;
  for (const auto& omission : script.omissions) {
    if (!described.insert(omission.kind).second) continue;
    QString description;
    switch (omission.kind) {
      case ConfigurationScriptOmissionKind::VolumeBinding:
        description = ApplicationText::tr("Volume binding");
        break;
      case ConfigurationScriptOmissionKind::DomainPersistence:
        description = ApplicationText::tr("Domain Secret Key persistence");
        break;
      case ConfigurationScriptOmissionKind::LicensePersistence:
        description = ApplicationText::tr("Terminal Services license persistence");
        break;
      case ConfigurationScriptOmissionKind::VolumeWithoutDrive:
        description = ApplicationText::tr("Settings without a reliable drive-letter address");
        break;
      case ConfigurationScriptOmissionKind::FileStaging:
        description = ApplicationText::tr("File staging");
        break;
      case ConfigurationScriptOmissionKind::UnrepresentableArgument:
        description = ApplicationText::tr("Settings with control characters in paths or names");
        break;
      case ConfigurationScriptOmissionKind::DependentCommand:
        description = ApplicationText::tr("Commands that depend on omitted settings");
        break;
      case ConfigurationScriptOmissionKind::InvalidObservedOverlayStorage:
        description = ApplicationText::tr("Observed overlay storage outside the writable size/type range");
        break;
      case ConfigurationScriptOmissionKind::IncompleteVolumeScope:
        description = ApplicationText::tr("Bulk unprotection requires a confirmed complete next-session volume scope.");
        break;
      case ConfigurationScriptOmissionKind::CurrentSessionAction:
        description = ApplicationText::tr("Current-session actions (HORM, FullCommit, and persistent overlay reset; export them from Pending changes)");
        break;
    }
    // Scope comments describe controlled setting categories, never arbitrary
    // target names or paths that could become CMD syntax when copied.
    text += "\n- " + description.toStdString();
  }
  return text;
}

std::string renderConfigurationScript(const ConfigurationScript& script) {
  return renderUwfmgrCommandScript(script.commands, configurationScriptScope(script));
}
}  // namespace uwf::app
