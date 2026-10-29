// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <string>
#include <vector>
namespace uwf::app {
class LogSource {
 public:
  virtual ~LogSource() = default;
  virtual std::vector<std::string> recentLines() const = 0;
  virtual void clear() = 0;
};
}  // namespace uwf::app
