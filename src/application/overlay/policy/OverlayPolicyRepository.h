// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <memory>

#include "application/configuration/contracts/ConfigurationRepository.h"
#include "application/overlay/contracts/OverlayPolicyActions.h"

namespace uwf::app {
[[nodiscard]] std::shared_ptr<ConfigurationRepository> overlayPolicyRepository(std::shared_ptr<OverlayPolicyActions> policy);
}  // namespace uwf::app
