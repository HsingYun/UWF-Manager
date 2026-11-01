// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <cstdint>

namespace uwf::app {

enum class ApplicationCommandKind : std::uint16_t {
  Activate = 1,
  EnsureRunning = 2,
};

}  // namespace uwf::app
