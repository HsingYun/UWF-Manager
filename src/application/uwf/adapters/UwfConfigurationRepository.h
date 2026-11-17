// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <memory>

#include "application/configuration/contracts/ConfigurationRepository.h"
#include "application/staging/editing/FileStagingDraft.h"
#include "domain/uwf/contracts/Backend.h"

namespace uwf::app {
[[nodiscard]] std::shared_ptr<ConfigurationRepository> uwfConfigurationRepository(std::shared_ptr<compat::Uwf> system,
                                                                                  std::shared_ptr<FileStagingDraft> staging = {});
}
