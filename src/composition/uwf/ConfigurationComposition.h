// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <memory>

#include "application/configuration/contracts/ConfigurationRepository.h"
#include "application/overlay/contracts/OverlayPolicyActions.h"
#include "application/staging/editing/FileStagingDraft.h"
#include "domain/uwf/contracts/Backend.h"

namespace uwf::composition {
// Assemble application repositories over the supplied UWF compatibility service.
[[nodiscard]] std::shared_ptr<app::ConfigurationRepository> uwfConfigurationRepository(std::shared_ptr<compat::Uwf> system,
                                                                                       std::shared_ptr<app::FileStagingDraft> staging = {},
                                                                                       std::shared_ptr<app::OverlayPolicyActions> policy = {});
}  // namespace uwf::composition
