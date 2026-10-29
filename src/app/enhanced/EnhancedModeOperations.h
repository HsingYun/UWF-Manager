// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <optional>

#include "app/api/EnhancedModeStatus.h"
#include "app/api/UwfCapability.h"
#include "app/ports/EnhancedModeServiceControl.h"
#include "app/ports/WmiOperations.h"

namespace uwf::app {
// Synchronous platform workflow. Workers own this value; QObject notifications
// and agent connection state remain in EnhancedModeManager on its owner thread.
class EnhancedModeOperations final {
 public:
  EnhancedModeOperations(std::shared_ptr<EnhancedModeServiceControl> control, wmi::WmiOperations& session, UwfCapability capability);
  [[nodiscard]] EnhancedModeStatus status() const;
  [[nodiscard]] EnhancedModeChangeResult enable(const QString& description);
  [[nodiscard]] EnhancedModeChangeResult start();
  [[nodiscard]] EnhancedModeChangeResult disable();

 private:
  [[nodiscard]] QString persistInstallation() const;
  [[nodiscard]] std::optional<std::vector<RegistryCommitTarget>> prepareRemovalPersistence() const;
  [[nodiscard]] QString persistRemoval(const std::vector<RegistryCommitTarget>& deletionPlan) const;
  std::shared_ptr<EnhancedModeServiceControl> m_serviceControl;
  std::function<wmi::WmiOperations&()> m_session;
  UwfCapability m_capability;
};
}  // namespace uwf::app
