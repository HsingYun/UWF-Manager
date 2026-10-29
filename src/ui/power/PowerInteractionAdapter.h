// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include "app/api/PowerInteraction.h"
class QWidget;
namespace uwf::ui {
[[nodiscard]] app::PowerInteraction powerInteraction(QWidget* dialogParent);
}  // namespace uwf::ui
