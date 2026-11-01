// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <optional>

#include "application/enhanced/contracts/EnhancedModeServiceControl.h"
#include "application/enhanced/contracts/EnhancedModeStatus.h"
#include "application/uwf/contracts/UwfCapability.h"
#include "application/uwf/contracts/UwfMaintenanceRepository.h"

namespace uwf::app {
// Synchronous platform workflow. Workers own this value; QObject notifications
// and agent connection state remain in EnhancedModeManager on its owner thread.
class EnhancedModeOperations final {
 public:
  EnhancedModeOperations(std::shared_ptr<EnhancedModeServiceControl> control, std::shared_ptr<UwfMaintenanceRepository> repository, UwfCapability capability);
  [[nodiscard]] EnhancedModeStatus status() const;
  [[nodiscard]] EnhancedModeChangeResult enable(const QString& description);
  [[nodiscard]] EnhancedModeChangeResult start();
  [[nodiscard]] EnhancedModeChangeResult disable();
  [[nodiscard]] EnhancedModeChangeResult remove();

 private:
  [[nodiscard]] bool registryWriteRequiresCommit() const;
  void persistConfiguration(bool requiresCommit, EnhancedModeChangeResult& result) const;
  [[nodiscard]] std::optional<std::vector<RegistryCommitTarget>> prepareRemovalPersistence() const;
  void persistRemoval(const std::vector<RegistryCommitTarget>& deletionPlan, EnhancedModeChangeResult& result) const;
  std::shared_ptr<EnhancedModeServiceControl> m_serviceControl;
  std::shared_ptr<UwfMaintenanceRepository> m_repository;
  UwfCapability m_capability;
};
}  // namespace uwf::app
