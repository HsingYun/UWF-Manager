// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <stop_token>

#include "application/registry/contracts/RegistryValueInfo.h"

namespace uwf::app {
// Read-only observations run with owned inputs. Cancellation is cooperative
// between platform calls; an in-flight operating-system call may finish first.
class RegistryBrowser {
 public:
  virtual ~RegistryBrowser() = default;
  [[nodiscard]] virtual std::vector<std::string> rootHiveLongNames(std::stop_token stop = {}) const = 0;
  [[nodiscard]] virtual std::vector<std::string> subkeyNames(std::string_view key, std::stop_token stop = {}) const = 0;
  [[nodiscard]] virtual bool hasSubkeys(std::string_view key, std::stop_token stop = {}) const = 0;
  [[nodiscard]] virtual std::vector<app::RegistryValueInfo> values(std::string_view key, std::stop_token stop = {}) const = 0;
};

}  // namespace uwf::app
