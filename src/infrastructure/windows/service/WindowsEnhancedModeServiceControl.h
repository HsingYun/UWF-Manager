// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <string_view>

#include "application/enhanced/contracts/EnhancedModeServiceControl.h"

namespace uwf::windows {

// The enhanced service requires exactly one --service argument. Interpret its
// SCM command line without opening or changing a service.
[[nodiscard]] bool serviceCommandMatchesExecutable(const wchar_t* commandLine, std::wstring_view executablePath);

class WindowsEnhancedModeServiceControl final : public app::EnhancedModeServiceControl {
 public:
  [[nodiscard]] app::EnhancedModeStatus query() const override;
  void installAndStart(const QString& description) override;
  void start() override;
  void disable() override;
  [[nodiscard]] std::vector<app::RegistryCommitTarget> planRegistryCommit() const override;
  [[nodiscard]] std::vector<app::RegistryCommitTarget> planRegistryDeletion() const override;
  void deleteRegistryRemnants() override;
  [[nodiscard]] Removal stopAndMarkForDeletion() override;
};

}  // namespace uwf::windows
