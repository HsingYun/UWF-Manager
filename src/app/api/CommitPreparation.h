// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QList>
#include <QStringList>
#include <functional>
#include <memory>
#include <optional>
#include <stop_token>
#include <string>
#include <vector>

#include "CommitInteraction.h"

namespace uwf::app {
struct CommitNode {
  CommitTargetKind kind = CommitTargetKind::File;
  QString path;
  QString valueName;
  QString skippedReason;
  [[nodiscard]] QString identity() const;
  [[nodiscard]] QString display() const;
};
struct CommitRequest {
  CommitOperation operation = CommitOperation::Persist;
  CommitNode target;
};
enum class TraversalStatus { Succeeded, Failed, Skipped };
struct CommitTraversalRecord {
  CommitNode node;
  TraversalStatus status = TraversalStatus::Succeeded;
  QString error;
  QStringList errorHistory;
  int attempts = 0;
};
using CommitNodeVisitor = std::function<void(const CommitNode&)>;
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
class CommitTraversalSource {
 public:
  virtual ~CommitTraversalSource() = default;
  // Missing targets are an ordinary observation. Access/metadata errors throw.
  [[nodiscard]] virtual std::optional<CommitNode> inspect(const CommitNode& target) const = 0;
  // Only an authoritative not-found observation returns false. Access and
  // enumeration failures remain errors, including during deletion verification.
  [[nodiscard]] virtual bool exists(const CommitNode& target) const = 0;
  [[nodiscard]] virtual std::string volumeForPath(const QString& path) const = 0;
  [[nodiscard]] virtual std::vector<std::string> mountedLetters(const std::string& volume) const = 0;
  // Children are published immediately. A returned error describes an incomplete
  // enumeration and never invalidates already discovered siblings.
  [[nodiscard]] virtual QString read(const CommitNode& target, std::stop_token stop, const CommitNodeVisitor& discover) const = 0;
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
