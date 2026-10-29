// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "OverlayHubFactory.h"

#include <memory>

#include "OverlayFloatingWidget.h"
#include "OverlayHub.h"
#include "OverlayTaskbarWidget.h"

namespace uwf::ui {

std::unique_ptr<OverlayHub> createDefaultOverlayHub() {
  auto hub = std::make_unique<OverlayHub>();
  hub->registerView(std::make_unique<OverlayTaskbarWidget>());
  hub->registerView(std::make_unique<OverlayFloatingWidget>());
  return hub;
}

}  // namespace uwf::ui
