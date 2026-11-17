// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <stdexcept>
#include <utility>

#include "application/commit/contracts/CommitTraversalSource.h"

namespace uwf::test::application {
inline constexpr auto fixtureVolumeId = "11111111-2222-3333-4444-555555555555";
inline constexpr auto otherFixtureVolumeId = "66666666-7777-8888-9999-aaaaaaaaaaaa";

// Explicit memory-source resolution. Real filesystem fixtures use the actual
// Windows resolver and seed the returned volume identity instead.
[[nodiscard]] inline app::FileCommitTarget memoryFileTarget(const QString& path, std::string volume = fixtureVolumeId) {
  auto relative = path;
  relative.replace(u'/', u'\\');
  if (relative.size() < 3 || relative[1] != u':' || relative[2] != u'\\') throw std::invalid_argument("Memory file source requires an absolute drive path");
  return {std::move(volume), relative.mid(2)};
}

// Stored-entry projection permits drive resolution only; unexpected inspection or traversal fails.
class StoredPathSource final : public app::CommitTraversalSource {
 public:
  std::string driveLetterForPath(const QString& path) const override {
    (void)memoryFileTarget(path);
    return path.left(2).toUpper().toStdString();
  }
  std::optional<app::CommitNode> inspect(const app::CommitNode&) const override { throw std::logic_error("Unexpected stored-entry metadata read"); }
  bool exists(const app::CommitNode&) const override { throw std::logic_error("Unexpected stored-entry existence read"); }
  app::FileCommitTarget resolveFileTarget(const QString&) const override { throw std::logic_error("Unexpected stored-entry target binding"); }
  QString enumerateChildren(const app::CommitNode&, std::stop_token, const app::CommitNodeVisitor&) const override {
    throw std::logic_error("Unexpected stored-entry traversal");
  }
};
}  // namespace uwf::test::application
