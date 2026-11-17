// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <memory>

#include "application/overlay/contracts/OverlayFileLoader.h"
#include "application/overlay/files/OverlayFilesQueryManager.h"
#include "composition/desktop/DesktopPlatformPorts.h"
#include "domain/uwf/contracts/Backend.h"
#include "presentation/viewmodels/common/ViewModelFactory.h"

namespace uwf::composition {
std::shared_ptr<presentation::ViewModelFactory> createViewModelFactory(std::shared_ptr<compat::Uwf> system, DesktopPlatformPorts ports,
                                                                       app::OverlayFileLoader overlayFiles = {},
                                                                       std::shared_ptr<app::OverlayFilesQueryManager> overlayFileQueries = {});
}  // namespace uwf::composition
