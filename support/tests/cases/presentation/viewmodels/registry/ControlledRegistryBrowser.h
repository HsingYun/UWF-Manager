// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <stdexcept>
#include <system_error>

#include "application/registry/contracts/RegistryBrowser.h"

namespace uwf::test::viewmodels {
class ControlledRegistryBrowser final : public app::RegistryBrowser {
 public:
  std::string deniedChildren;
  std::vector<std::string> childNames{"SOFTWARE", "PERSISTENT_INTERNAL", "Restricted"};
  std::vector<app::RegistryValueInfo> valueEntries{{"Named", 1u, std::vector<uint8_t>{}}};
  mutable std::string lastValueKey;
  bool denyValues = false;
  std::vector<std::string> rootHiveLongNames(std::stop_token = {}) const override { return {"HKEY_LOCAL_MACHINE"}; }
  std::vector<std::string> subkeyNames(std::string_view key, std::stop_token = {}) const override {
    if (key == deniedChildren) throw std::runtime_error("拒绝访问");
    return childNames;
  }
  bool hasSubkeys(std::string_view key, std::stop_token = {}) const override {
    if (key.ends_with("Restricted")) throw std::system_error(std::make_error_code(std::errc::permission_denied));
    return true;
  }
  std::vector<uwf::app::RegistryValueInfo> values(std::string_view key, std::stop_token = {}) const override {
    lastValueKey = key;
    if (denyValues) throw std::runtime_error("拒绝访问");
    return valueEntries;
  }
};

}  // namespace uwf::test::viewmodels
