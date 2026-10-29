// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include "app/ports/FileStagingStorage.h"
namespace uwf::windows {
class RegistryFileStagingStore final : public app::FileStagingStorage {
 public:
  [[nodiscard]] QList<app::FileStagingEntry> load() const override;
  void replace(const QList<app::FileStagingEntry>& entries) override;
  [[nodiscard]] bool exists() const override;
  [[nodiscard]] std::vector<app::RegistryCommitTarget> deletionPlan() const override;
};
}  // namespace uwf::windows
