// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <memory>

#include "application/state/contracts/ApplicationStateSource.h"
#include "application/volumes/contracts/VolumeRepository.h"
#include "domain/uwf/contracts/Backend.h"

namespace uwf::app {
[[nodiscard]] std::unique_ptr<ApplicationStateSource> uwfApplicationStateSource(std::shared_ptr<compat::Uwf> system,
                                                                                std::shared_ptr<const VolumeRepository> volumes, bool elevated);
}
