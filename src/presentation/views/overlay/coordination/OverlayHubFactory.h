// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <memory>

namespace uwf::ui {

class OverlayHub;

// Registers the application HUD endpoints without exposing concrete views to the controller.
[[nodiscard]] std::unique_ptr<OverlayHub> createDefaultOverlayHub();

}  // namespace uwf::ui
