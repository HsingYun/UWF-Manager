// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <optional>

#include "app/api/FileStagingTypes.h"
#include "core/UwfModel.h"

namespace uwf::app {
enum class ConfigurationOperationKind {
  Staging,
  Filter,
  OverlayType,
  OverlaySize,
  WarningThreshold,
  CriticalThreshold,
  VolumeRegistration,
  VolumeProtection,
  VolumeBinding,
  AddFileExclusion,
  RemoveFileExclusion,
  AddRegistryExclusion,
  RemoveRegistryExclusion,
  DomainPersistence,
  LicensePersistence
};
struct ConfigurationOperation {
  ConfigurationOperationKind kind;
  std::string target;
  bool operator==(const ConfigurationOperation&) const = default;
};
// The reviewed target is also the execution input. Storage never invents
// next-session exclusions independently of this plan.
struct ConfigurationPlan {
  core::PendingChanges changes;
  core::UwfSnapshot snapshot;
  std::optional<FileStagingChanges> staging;
  // A saved list needs at least one of these next-session operations to
  // complete. Empty means the existing target already satisfies durability.
  std::vector<ConfigurationOperation> stagingCompletionAlternatives{};
  [[nodiscard]] std::vector<ConfigurationOperation> operations() const;
  [[nodiscard]] std::size_t count() const { return changes.count() + (staging ? staging->count() : 0); }
};
[[nodiscard]] ConfigurationPlan planConfiguration(core::PendingChanges changes, core::UwfSnapshot snapshot, std::optional<FileStagingChanges> staging = {});
}  // namespace uwf::app
