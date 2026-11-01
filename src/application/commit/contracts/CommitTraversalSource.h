// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include "application/commit/contracts/CommitPlan.h"

namespace uwf::app {
using CommitNodeVisitor = std::function<void(const CommitNode&)>;
// Empty means complete enumeration. A nonempty diagnostic reports an incomplete
// enumeration while preserving every child already delivered to the visitor.
using CommitEnumerationError = QString;

class CommitTraversalSource {
 public:
  virtual ~CommitTraversalSource() = default;
  // Missing targets are an ordinary observation. Access/metadata errors throw.
  // A scan-owned fileTarget selects the exact object without resolving its
  // display path again; execution revalidates that binding at its boundary.
  [[nodiscard]] virtual std::optional<CommitNode> inspect(const CommitNode& target) const = 0;
  // Only an authoritative not-found observation returns false. Access and
  // enumeration failures remain errors, including during deletion verification.
  [[nodiscard]] virtual bool exists(const CommitNode& target) const = 0;
  // Display drive (for example C:); an empty result is not a provider identity.
  [[nodiscard]] virtual std::string driveLetterForPath(const QString& path) const = 0;
  // Reuse this observation's bound volume when the source can map it directly.
  [[nodiscard]] virtual std::string driveLetterForPath(const QString& path, const FileCommitTarget&) const { return driveLetterForPath(path); }
  // Resolve the actual mounted volume, not its display drive letter. Failures
  // remain errors. A scan reuses identities emitted by its enumeration; execution
  // resolves the selected path again before a later operation.
  [[nodiscard]] virtual FileCommitTarget resolveFileTarget(const QString& path) const = 0;
  // Children are published immediately. A returned error describes an incomplete
  // enumeration and never invalidates already discovered siblings.
  [[nodiscard]] virtual CommitEnumerationError enumerateChildren(const CommitNode& target, std::stop_token stop, const CommitNodeVisitor& discover) const = 0;
};
// Every observed or selected name must survive the application's UTF-8 ports
// exactly. Native UTF-16 names outside that representation are rejected before
// observations or writes can address a different object.
void validateCommitNodeText(const CommitNode& node);
// Validate a scan-owned binding without resolving a path or observing the OS.
void validateFileCommitTarget(const FileCommitTarget& target);
[[nodiscard]] CommitNode bindFileCommitTarget(const CommitTraversalSource& source, CommitNode target);
}  // namespace uwf::app
