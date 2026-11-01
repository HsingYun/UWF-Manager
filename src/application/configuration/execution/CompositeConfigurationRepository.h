// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <memory>

#include "application/configuration/contracts/ConfigurationRepository.h"

namespace uwf::app {
// The first repository declaring an area owns reads and writes of that area.
// Routing is stable: read/write failures never trigger a silent backend fallback.
class CompositeConfigurationRepository final : public ConfigurationRepository {
 public:
  explicit CompositeConfigurationRepository(std::vector<std::shared_ptr<ConfigurationRepository>> repositories);
  [[nodiscard]] bool supports(ConfigurationArea area) const override;
  [[nodiscard]] OverlayPolicySnapshot snapshot() const override;
  [[nodiscard]] WriteOperation prepareWrite(const ConfigurationPhase& phase, const ConfigurationPlan& plan) override;
  void completeWrite(const ConfigurationApplyResult& result) override;

 private:
  [[nodiscard]] std::shared_ptr<ConfigurationRepository> owner(ConfigurationArea area) const;
  std::vector<std::shared_ptr<ConfigurationRepository>> m_repositories;
};
}  // namespace uwf::app
