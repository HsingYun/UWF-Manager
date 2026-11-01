// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include "application/commit/contracts/CommitPlan.h"
#include "application/commit/contracts/CommitTraversalSource.h"
namespace uwf::windows {
// The shared filesystem adapter for manual commits and automatic file staging.
class WindowsFileTraversalSource final : public app::CommitTraversalSource {
 public:
  [[nodiscard]] std::optional<app::CommitNode> inspect(const app::CommitNode& target) const override;
  [[nodiscard]] bool exists(const app::CommitNode& target) const override;
  [[nodiscard]] std::string driveLetterForPath(const QString& path) const override;
  [[nodiscard]] std::string driveLetterForPath(const QString& path, const app::FileCommitTarget& target) const override;
  [[nodiscard]] app::FileCommitTarget resolveFileTarget(const QString& path) const override;
  [[nodiscard]] QString enumerateChildren(const app::CommitNode& target, std::stop_token stop, const app::CommitNodeVisitor& discover) const override;
};
}  // namespace uwf::windows
