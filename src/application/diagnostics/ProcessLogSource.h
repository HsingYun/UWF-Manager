// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include "application/diagnostics/contracts/LogSource.h"

namespace uwf::app {
class ProcessLogSource final : public LogSource {
 public:
  [[nodiscard]] std::vector<std::string> recentLines() const override;
  void clear() override;
};
}  // namespace uwf::app
