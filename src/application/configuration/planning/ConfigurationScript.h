// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include "application/configuration/commands/UwfmgrCommand.h"
#include "application/configuration/contracts/ConfigurationPlan.h"

namespace uwf::app {
enum class ConfigurationScriptScope { PendingChanges, NextConfiguration };
enum class ConfigurationScriptOmissionKind {
  VolumeBinding,
  DomainPersistence,
  LicensePersistence,
  VolumeWithoutDrive,
  FileStaging,
  UnrepresentableArgument,
  DependentCommand,
  InvalidObservedOverlayStorage,
  IncompleteVolumeScope,
  CurrentSessionAction
};
struct ConfigurationScriptOmission {
  ConfigurationScriptOmissionKind kind;
  std::string target;
  bool operator==(const ConfigurationScriptOmission&) const = default;
};
// Export only CLI-expressible settings and retain the omissions. Arguments with
// ASCII controls/DEL, and their dependent commands, are omitted from that chain.
struct ConfigurationScript {
  std::vector<UwfmgrCommand> commands;
  std::vector<ConfigurationScriptOmission> omissions;
  bool exportSupported = true;
  ConfigurationScriptScope scope = ConfigurationScriptScope::PendingChanges;
};

// Use execution phase ordering. Target scripts also include baseline settings;
// actions that require the active session (HORM, FullCommit and persistent reset)
// are exported only from Pending changes. Bulk unprotection expands the confirmed
// Next scope, including offline GUID targets. File exclusions remain drive-addressed.
[[nodiscard]] ConfigurationScript configurationChangeScript(const ConfigurationPlan& plan);
[[nodiscard]] ConfigurationScript configurationTargetScript(const ConfigurationPlan& plan);
// Shared by command previews, clipboard text, and exported script comments.
[[nodiscard]] std::string configurationScriptScope(const ConfigurationScript& script);
[[nodiscard]] std::string renderConfigurationScript(const ConfigurationScript& script);

}  // namespace uwf::app
