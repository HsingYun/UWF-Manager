// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <cstdint>
#include <memory>
#include <optional>

#include "application/enhanced/EnhancedModeOperations.h"
#include "application/enhanced/contracts/EnhancedModeActions.h"
#include "application/enhanced/contracts/EnhancedModeServiceControl.h"
#include "application/uwf/contracts/UwfCapability.h"
#include "application/uwf/contracts/UwfMaintenanceRepository.h"

namespace uwf::app {

class EnhancedModeManager final : public EnhancedModeActions {
  Q_OBJECT
 public:
  EnhancedModeManager(std::shared_ptr<EnhancedModeServiceControl> serviceControl, std::shared_ptr<UwfMaintenanceRepository> repository,
                      UwfCapability capability, QObject* parent = nullptr);

  [[nodiscard]] EnhancedModeStatus status() const override;
  void setAgentState(EnhancedModeAgentState state) override;

 private:
  [[nodiscard]] ChangeWork prepareChange(EnhancedModeAction action, const QString& serviceDescription) override;
  void acceptChangeResult(EnhancedModeChangeResult& result) override;
  std::shared_ptr<EnhancedModeOperations> m_operations;
  mutable std::optional<EnhancedModeStatus> m_observed;
  mutable std::uint64_t m_observationGeneration = 0;
  EnhancedModeAgentState m_agentState = EnhancedModeAgentState::Unobserved;
};

}  // namespace uwf::app
