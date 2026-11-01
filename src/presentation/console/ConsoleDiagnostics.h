// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <string_view>

#include "application/console/ConsoleCommandExecutor.h"

namespace uwf::console {

// Developer evidence is published independently of user-visible result text.
// Logging never changes the command's receipt or its effect.
void logConsoleResult(const app::ConsoleCommandResult& result) noexcept;
void logConsoleOutputFailure(std::string_view operation, std::string_view detail) noexcept;

}  // namespace uwf::console
