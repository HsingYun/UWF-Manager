// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "infrastructure/windows/registry/WindowsRegistryBrowser.h"

#include "infrastructure/windows/registry/RegistryKey.h"

namespace uwf::windows {
namespace {
class WindowsRegistryBrowser final : public app::RegistryBrowser {
 public:
  [[nodiscard]] std::vector<std::string> rootHiveLongNames(const std::stop_token stop = {}) const override {
    return stop.stop_requested() ? std::vector<std::string>{} : regkey::rootHiveLongNames();
  }
  [[nodiscard]] std::vector<std::string> subkeyNames(const std::string_view key, const std::stop_token stop = {}) const override {
    return stop.stop_requested() ? std::vector<std::string>{} : regkey::subkeyNames(key, stop);
  }
  [[nodiscard]] bool hasSubkeys(const std::string_view key, const std::stop_token stop = {}) const override {
    return !stop.stop_requested() && regkey::hasSubkeys(key);
  }
  [[nodiscard]] std::vector<app::RegistryValueInfo> values(const std::string_view key, const std::stop_token stop = {}) const override {
    return regkey::values(key, stop);
  }
};

}  // namespace
const app::RegistryBrowser& systemRegistryBrowser() {
  static const WindowsRegistryBrowser browser;
  return browser;
}

}  // namespace uwf::windows
