// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include "app/ports/EnhancedModeServiceControl.h"

namespace uwf::windows {

class WindowsEnhancedModeServiceControl final : public app::EnhancedModeServiceControl {
 public:
  [[nodiscard]] app::EnhancedModeStatus query() const override;
  void installAndStart(const QString& description) override;
  void start() override;
  [[nodiscard]] std::vector<app::RegistryCommitTarget> planRegistryCommit() const override;
  [[nodiscard]] std::vector<app::RegistryCommitTarget> planRegistryDeletion() const override;
  void deleteRegistryRemnants() override;
  [[nodiscard]] Removal stopAndMarkForDeletion() override;
};

}  // namespace uwf::windows
