// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QStringList>
#include <functional>
#include <optional>
#include <stop_token>
#include <string>
#include <tuple>

#include "application/commit/contracts/CommitResult.h"
#include "application/identity/contracts/ObjectIdentity.h"

namespace uwf::app {
using CommitNodeIdentity = std::tuple<CommitTargetKind, ObjectIdentity, ObjectIdentity>;
// Stable execution identity. The display path may be rebound to another volume;
// filesystem observations and UWF dispatch use this volume-relative target.
struct FileCommitTarget {
  std::string volumeId;
  QString relativePath;
  bool operator==(const FileCommitTarget& other) const;
};
struct CommitNode {
  CommitTargetKind kind = CommitTargetKind::File;
  QString path;
  QString valueName;
  QString skippedReason;
  std::optional<FileCommitTarget> fileTarget{};
  [[nodiscard]] CommitNodeIdentity identity() const;
  [[nodiscard]] QString display() const;
};
enum class CommitPreparationIntent { RetryFailed, TraverseScope };
struct CommitRequest {
  CommitOperation operation = CommitOperation::Persist;
  CommitNode target;
  // An empty previous plan always starts discovery. For an existing plan,
  // retry failed observations or explicitly traverse the selected scope again.
  CommitPreparationIntent preparationIntent = CommitPreparationIntent::RetryFailed;
};
enum class TraversalStatus { Succeeded, Failed, Skipped };
struct CommitTraversalRecord {
  CommitNode node;
  TraversalStatus status = TraversalStatus::Succeeded;
  QString error;
  QStringList errorHistory;
  int attempts = 0;
};
struct CommitTraversalProgress {
  qsizetype items = 0;
  qsizetype containers = 0;
  qsizetype failed = 0;
  QString path;
  std::optional<CommitTargetKind> kind;
  bool operator==(const CommitTraversalProgress&) const = default;
};
struct CommitPlan {
  CommitRequest request;
  QList<CommitTraversalRecord> records;
  bool canceled = false;
  [[nodiscard]] CommitTraversalProgress progress() const;
  [[nodiscard]] QList<CommitNode> targets() const;
};
using CommitPreparationProgress = std::function<void(const CommitTraversalProgress&)>;
class CommitBackend {
 public:
  virtual ~CommitBackend() = default;
  [[nodiscard]] virtual CommitPlan prepare(const CommitRequest& request, CommitPlan previous, std::stop_token stop,
                                           const CommitPreparationProgress& progress) = 0;
  [[nodiscard]] virtual CommitTargetResult execute(const CommitNode& target, CommitOperation operation) = 0;
};
}  // namespace uwf::app
