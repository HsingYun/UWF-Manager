// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "application/operations/contracts/OperationError.h"
#include "application/power/contracts/PowerTypes.h"
#include "application/staging/contracts/AutomaticCommitDisposition.h"
#include "domain/uwf/model/UwfModel.h"

namespace uwf::app {

// Admission failures throw. Both values are successful observations of whether
// persistence needs an explicit UWF commit in this session.
enum class CommitRequirement { AlreadyPersistent, RequiresUwfCommit };

// Known active HORM blocks configuration edits. Prechecks use published state;
// preparing or executing an operation never refreshes it implicitly. Individual
// commit admission separately depends on target protection and loaded ROM.
[[nodiscard]] constexpr bool hormAllowsChanges(const std::optional<bool> hormEnabled) { return hormEnabled != true; }

// A prepared target keeps its stable UWF identity for the entire batch. Each
// commit validates admission from global state immediately before dispatch.
class UwfVolumeTarget {
 public:
  virtual ~UwfVolumeTarget() = default;
  [[nodiscard]] virtual const core::VolumeRecord& volume() const = 0;
  [[nodiscard]] virtual std::vector<std::string> exclusions() const = 0;
  virtual WriteEffect commitFile(const std::string& path) const = 0;
  virtual WriteEffect commitFileDeletion(const std::string& path) const = 0;
};

// A Current registry maintenance target. It cannot edit Next configuration.
class UwfRegistryMaintenanceTarget {
 public:
  virtual ~UwfRegistryMaintenanceTarget() = default;
  [[nodiscard]] virtual std::vector<std::string> exclusions() const = 0;
  // Use the published SYSTEM\Select\Current for driver CurrentControlSet translation.
  // Unavailable observations throw; do not infer a set number from a key name.
  [[nodiscard]] virtual std::uint32_t currentControlSet() const = 0;
  virtual WriteEffect commit(const std::string& key, const std::string& valueName) const = 0;
  // nullopt selects a key. A present empty name is unsupported; it must never
  // be collapsed into whole-key deletion.
  virtual WriteEffect commitDeletion(const std::string& key, const std::optional<std::string>& valueName) const = 0;
};

// Next registry exclusion configuration is a distinct capability: it cannot
// submit Current value commits or deletions.
class UwfRegistryExclusionConfiguration {
 public:
  virtual ~UwfRegistryExclusionConfiguration() = default;
  [[nodiscard]] virtual std::vector<std::string> exclusions() const = 0;
  virtual void addExclusion(const std::string& key) const = 0;
};

// Semantic maintenance port. Reads throw on failure; prepared operations own
// their dependencies. Backend selection remains inside the compatibility service.
class UwfMaintenanceRepository {
 public:
  virtual ~UwfMaintenanceRepository() = default;
  // Admission for existing visible content. AlreadyPersistent applies only when
  // Current UWF is off or the target is unprotected. Active protected targets
  // reject HORM and loaded ROM; Next options and FullCommit do not prove that
  // historical data reached storage. Missing required facts throw.
  [[nodiscard]] virtual CommitRequirement fileCommitRequirement(const std::string& volumeId) const = 0;
  [[nodiscard]] virtual CommitRequirement registryCommitRequirement() const = 0;
  // Automatic handling distinguishes persistent targets, paused modes and those
  // requiring a durability check. It does not relax explicit commit admission.
  [[nodiscard]] virtual AutomaticCommitDisposition automaticFileCommitDisposition(const std::string& volumeId) const = 0;
  // Publish one fresh read-only observation before a final automatic decision.
  // In-memory repositories may already expose live facts; production overrides
  // this boundary to refresh Current, Next and runtime together or fail.
  virtual void refreshAutomaticCommitState() const {}
  // Admit a new host registry mutation before it occurs. Current UWF off,
  // unprotected system storage or FullCommit make that new write direct.
  // This is not evidence that existing overlay values or deletions are durable;
  // historical reconciliation must use registryCommitRequirement instead.
  [[nodiscard]] virtual CommitRequirement registryWriteRequirement() const = 0;
  [[nodiscard]] virtual std::vector<std::shared_ptr<UwfVolumeTarget>> prepareVolumeTargets() const = 0;
  [[nodiscard]] virtual std::shared_ptr<UwfRegistryMaintenanceTarget> prepareRegistryMaintenance() const = 0;
  [[nodiscard]] virtual std::shared_ptr<UwfRegistryExclusionConfiguration> prepareRegistryExclusions() const = 0;
  // Validate the requested power capability before preshutdown coordination.
  [[nodiscard]] virtual std::function<void()> preparePowerAction(PowerAction action) const = 0;
};

}  // namespace uwf::app
