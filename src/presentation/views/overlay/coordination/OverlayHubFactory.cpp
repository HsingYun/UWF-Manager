// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "presentation/views/overlay/coordination/OverlayHubFactory.h"

#include <memory>

#include "presentation/views/overlay/hud/OverlayFloatingWidget.h"
#include "presentation/views/overlay/coordination/OverlayHub.h"
#include "presentation/views/overlay/hud/OverlayTaskbarWidget.h"

namespace uwf::ui {

std::unique_ptr<OverlayHub> createDefaultOverlayHub() {
  auto hub = std::make_unique<OverlayHub>();
  hub->registerView(std::make_unique<OverlayTaskbarWidget>());
  hub->registerView(std::make_unique<OverlayFloatingWidget>());
  return hub;
}

}  // namespace uwf::ui
