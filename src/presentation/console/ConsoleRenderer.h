// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <functional>
#include <string>
#include <string_view>

#include "application/console/ConsoleCommandExecutor.h"

namespace uwf::console {
struct ConsoleText {
  std::string output;
  std::string error;
};
// English presentation, independent of Windows handles, Qt and the UI locale.
// Report-wide field-name alignment, nested section indentation and read-status
// summaries belong here. Provider evidence remains in the original result;
// public text contains typed summaries, actual error codes and publicReason.
[[nodiscard]] ConsoleText renderConsoleResult(const app::ConsoleCommandResult& result);
// Publication cannot replay execution or discard its effect. Both streams are
// attempted independently, including when stdout is a closed pipe.
using ConsoleTextWriter = std::function<void(std::string_view)>;
[[nodiscard]] int publishConsoleResult(const app::ConsoleCommandResult& result, const ConsoleTextWriter& output, const ConsoleTextWriter& error);
}  // namespace uwf::console
