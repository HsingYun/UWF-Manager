// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include "application/configuration/contracts/ConfigurationActions.h"
#include "application/overlay/contracts/OverlayPolicy.h"

namespace uwf::app {
enum class ConfigurationArea { Settings, OverlayPolicy };
// Domain storage boundary. A prepared operation owns its worker dependencies.
// Implementations route semantic changes and retain authoritative command
// receipts. Unedited fields are observations, not implicit write intent.
class ConfigurationRepository {
 public:
  using WriteOperation = std::function<ConfigurationApplyResult(const OverlayPolicySnapshot&)>;
  virtual ~ConfigurationRepository() = default;
  [[nodiscard]] virtual bool supports(ConfigurationArea area) const = 0;
  // Published observations only. Execution admission must not refresh a provider.
  [[nodiscard]] virtual OverlayPolicySnapshot snapshot() const { return {}; }
  [[nodiscard]] virtual WriteOperation prepareWrite(const ConfigurationPhase& phase, const ConfigurationPlan& plan) = 0;
  virtual void completeWrite(const ConfigurationApplyResult&) {}
};
}  // namespace uwf::app
