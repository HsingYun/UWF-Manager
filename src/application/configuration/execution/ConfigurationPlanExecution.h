// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <exception>
#include <functional>
#include <string_view>

#include "application/configuration/contracts/ConfigurationRepository.h"

namespace uwf::app {
struct PreparedConfigurationPhase {
  ConfigurationPhase phase{};
  ConfigurationRepository::WriteOperation write{};
};
struct ConfigurationObservations {
  OverlayPolicySnapshot policy;
  std::optional<OverlayMaintenanceSnapshot> maintenance;
};
// Log only exceptions converted into a result at this boundary. Failed results
// returned by a repository are already logged by their operation owner.
void logConfigurationException(std::string_view context, std::exception_ptr error, const ConfigurationApplyResult& result) noexcept;
// Admission checks published memory against the reviewed snapshot. Successful
// receipts supply later baselines; execution never refreshes the repository.
[[nodiscard]] ConfigurationApplyResult executeConfigurationPlan(const ConfigurationPlan& plan, const std::vector<PreparedConfigurationPhase>& phases,
                                                                const OverlayPolicySnapshot& published,
                                                                const std::function<ConfigurationObservations()>& publishedAfterExecution = {});
}  // namespace uwf::app
