// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include "app/api/CommitPreparation.h"
namespace uwf::windows {
// The shared filesystem adapter for manual commits and automatic file staging.
class WindowsFileTraversalSource final : public app::CommitTraversalSource {
 public:
  [[nodiscard]] std::optional<app::CommitNode> inspect(const app::CommitNode& target) const override;
  [[nodiscard]] bool exists(const app::CommitNode& target) const override;
  [[nodiscard]] std::string volumeForPath(const QString& path) const override;
  [[nodiscard]] std::vector<std::string> mountedLetters(const std::string& volume) const override;
  [[nodiscard]] QString read(const app::CommitNode& target, std::stop_token stop, const app::CommitNodeVisitor& discover) const override;
};
}  // namespace uwf::windows
