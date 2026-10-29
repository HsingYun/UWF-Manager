// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include "app/api/RegistryValueInfo.h"
namespace uwf::app {
class RegistryBrowser {
 public:
  virtual ~RegistryBrowser() = default;
  [[nodiscard]] virtual std::vector<std::string> rootHiveLongNames() const = 0;
  [[nodiscard]] virtual std::vector<std::string> subkeyNames(std::string_view key) const = 0;
  [[nodiscard]] virtual bool hasSubkeys(std::string_view key) const = 0;
  [[nodiscard]] virtual std::vector<app::RegistryValueInfo> values(std::string_view key) const = 0;
};

}  // namespace uwf::app
