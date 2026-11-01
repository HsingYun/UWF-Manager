// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include "application/console/ConsoleCommandExecutor.h"

namespace uwf::windows {
class WindowsConsoleEnvironment final : public app::ConsoleEnvironment {
 public:
  [[nodiscard]] app::ConsoleVolumeTarget resolveVolume(const std::string& operand) const override;
  [[nodiscard]] app::ConsoleFileTarget resolveFile(const std::string& operand) const override;
};
}  // namespace uwf::windows
