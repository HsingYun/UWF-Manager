// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "application/configuration/execution/CompositeConfigurationRepository.h"

#include <algorithm>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace uwf::app {
CompositeConfigurationRepository::CompositeConfigurationRepository(std::vector<std::shared_ptr<ConfigurationRepository>> repositories)
    : m_repositories(std::move(repositories)) {
  if (std::ranges::any_of(m_repositories, [](const auto& repository) { return !repository; }))
    throw std::invalid_argument("Configuration repositories must not be null");
}
std::shared_ptr<ConfigurationRepository> CompositeConfigurationRepository::owner(const ConfigurationArea area) const {
  const auto found = std::ranges::find_if(m_repositories, [area](const auto& repository) { return repository->supports(area); });
  return found == m_repositories.end() ? nullptr : *found;
}
bool CompositeConfigurationRepository::supports(const ConfigurationArea area) const { return bool(owner(area)); }
OverlayPolicySnapshot CompositeConfigurationRepository::snapshot() const {
  if (const auto repository = owner(ConfigurationArea::OverlayPolicy)) return repository->snapshot();
  if (const auto repository = owner(ConfigurationArea::Settings)) return repository->snapshot();
  return {};
}
ConfigurationRepository::WriteOperation CompositeConfigurationRepository::prepareWrite(const ConfigurationPhase& phase, const ConfigurationPlan& plan) {
  const auto area = std::visit(
      [](const auto& payload) -> ConfigurationArea {
        using Payload = std::decay_t<decltype(payload)>;
        if constexpr (std::is_same_v<Payload, ConfigurationSettingsPhase>)
          return ConfigurationArea::Settings;
        else if constexpr (std::is_same_v<Payload, ConfigurationOverlayPolicyPhase>)
          return ConfigurationArea::OverlayPolicy;
        else {
          static_assert(std::is_same_v<Payload, ConfigurationBulkProtectionPhase> || std::is_same_v<Payload, ConfigurationMaintenancePhase>);
          throw std::invalid_argument("Maintenance phases are executed by configuration coordination");
        }
      },
      phase);
  const auto repository = owner(area);
  if (!repository) throw OperationError(std::make_error_code(std::errc::not_supported), "No repository supports the requested configuration area");
  return repository->prepareWrite(phase, plan);
}
void CompositeConfigurationRepository::completeWrite(const ConfigurationApplyResult& result) {
  std::vector<std::shared_ptr<ConfigurationRepository>> completed;
  for (const auto area : {ConfigurationArea::Settings, ConfigurationArea::OverlayPolicy}) {
    const auto repository = owner(area);
    if (repository && std::ranges::find(completed, repository) == completed.end()) {
      repository->completeWrite(result);
      completed.push_back(repository);
    }
  }
}
}  // namespace uwf::app
