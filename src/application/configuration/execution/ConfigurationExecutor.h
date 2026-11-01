// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <memory>

#include "application/configuration/contracts/ConfigurationRepository.h"

namespace uwf::app {
class ConfigurationExecutor final : public ConfigurationActions {
 public:
  explicit ConfigurationExecutor(std::shared_ptr<ConfigurationRepository> repository, std::optional<FileStagingChanges> stagingChanges = {},
                                 std::shared_ptr<OverlayMaintenanceActions> maintenance = {});
  [[nodiscard]] ApplyOperation prepareApply(const core::PendingChanges& changes, const core::UwfSnapshot& snapshot) override;
  [[nodiscard]] ApplyOperation prepareApply(const ConfigurationPlan& plan) override;
  void completeApply(const ConfigurationApplyResult& result) override;

 private:
  std::shared_ptr<ConfigurationRepository> m_repository;
  std::optional<FileStagingChanges> m_stagingChanges;
  std::shared_ptr<OverlayMaintenanceActions> m_maintenance;
};
}  // namespace uwf::app
