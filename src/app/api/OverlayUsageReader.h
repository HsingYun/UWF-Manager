// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include "OverlayUsage.h"
namespace uwf::app {
class OverlayUsageReader {
 public:
  virtual ~OverlayUsageReader() = default;
  [[nodiscard]] virtual OverlayUsageState read() const = 0;
};
}  // namespace uwf::app
