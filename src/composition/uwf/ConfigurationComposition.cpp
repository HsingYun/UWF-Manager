// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "composition/uwf/ConfigurationComposition.h"

#include <utility>
#include <vector>

#include "application/configuration/execution/CompositeConfigurationRepository.h"
#include "application/overlay/policy/OverlayPolicyRepository.h"
#include "application/uwf/adapters/UwfConfigurationRepository.h"
#include "application/uwf/adapters/UwfOverlayOperations.h"

namespace uwf::composition {
std::shared_ptr<app::ConfigurationRepository> uwfConfigurationRepository(std::shared_ptr<compat::Uwf> system, std::shared_ptr<app::FileStagingDraft> staging,
                                                                         std::shared_ptr<app::OverlayPolicyActions> policy) {
  if (!policy) policy = app::uwfOverlayPolicy(system);
  std::vector<std::shared_ptr<app::ConfigurationRepository>> repositories;
  repositories.push_back(app::uwfConfigurationRepository(std::move(system), std::move(staging)));
  repositories.push_back(app::overlayPolicyRepository(std::move(policy)));
  return std::make_shared<app::CompositeConfigurationRepository>(std::move(repositories));
}
}  // namespace uwf::composition
