// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "WindowsRegistryBrowser.h"

#include "app/ports/RegistryKey.h"
namespace uwf::windows {
namespace {
class WindowsRegistryBrowser final : public app::RegistryBrowser {
 public:
  [[nodiscard]] std::vector<std::string> rootHiveLongNames() const override { return regkey::rootHiveLongNames(); }
  [[nodiscard]] std::vector<std::string> subkeyNames(const std::string_view key) const override { return regkey::subkeyNames(key); }
  [[nodiscard]] bool hasSubkeys(const std::string_view key) const override { return regkey::hasSubkeys(key); }
  [[nodiscard]] std::vector<app::RegistryValueInfo> values(const std::string_view key) const override { return regkey::values(key); }
};

}  // namespace
const app::RegistryBrowser& systemRegistryBrowser() {
  static const WindowsRegistryBrowser browser;
  return browser;
}

}  // namespace uwf::windows
