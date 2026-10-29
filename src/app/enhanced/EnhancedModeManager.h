// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <memory>
#include <optional>

#include "EnhancedModeOperations.h"
#include "app/api/EnhancedModeActions.h"
#include "app/api/UwfCapability.h"
#include "app/ports/EnhancedModeServiceControl.h"
#include "app/ports/WmiOperations.h"

namespace uwf::app {

class EnhancedModeManager final : public EnhancedModeActions {
  Q_OBJECT
 public:
  EnhancedModeManager(std::shared_ptr<EnhancedModeServiceControl> serviceControl, wmi::WmiOperations& session, UwfCapability capability,
                      QObject* parent = nullptr);

  [[nodiscard]] EnhancedModeStatus status() const override;
  [[nodiscard]] EnhancedModeChangeResult enable(const QString& serviceDescription) override;
  [[nodiscard]] EnhancedModeChangeResult start() override;
  [[nodiscard]] EnhancedModeChangeResult disable() override;
  void setAgentState(EnhancedModeAgentState state) override;
  [[nodiscard]] ChangeWork prepareChange(EnhancedModeAction action, const QString& serviceDescription) override;

 private:
  void acceptChangeResult(EnhancedModeChangeResult& result) override;
  std::shared_ptr<EnhancedModeOperations> m_operations;
  mutable std::optional<EnhancedModeStatus> m_observed;
  EnhancedModeAgentState m_agentState = EnhancedModeAgentState::Unobserved;
};

}  // namespace uwf::app
