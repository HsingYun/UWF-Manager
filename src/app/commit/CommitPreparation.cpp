// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "CommitPreparation.h"

#include <QHash>
#include <QSet>
#include <algorithm>
#include <exception>

#include "app/common/ApplicationText.h"

namespace uwf::app {
namespace {
bool container(const CommitNode& node) { return node.kind == CommitTargetKind::Folder || node.kind == CommitTargetKind::RegistryKey; }
}  // namespace
QString CommitNode::identity() const {
  const auto key = path.toCaseFolded();
  return QString::number(static_cast<int>(kind == CommitTargetKind::Folder ? CommitTargetKind::File : kind)) + u':' + QString::number(key.size()) + u':' + key +
         valueName.toCaseFolded();
}
QString CommitNode::display() const {
  return kind == CommitTargetKind::RegistryValue ? path + QStringLiteral(" : ") + (valueName.isEmpty() ? ApplicationText::tr("(Default)") : valueName) : path;
}
CommitTraversalProgress CommitPlan::progress() const {
  CommitTraversalProgress result;
  result.kind = request.target.kind;
  for (const auto& record : records) {
    if (record.status == TraversalStatus::Failed)
      ++result.failed;
    else if (record.status == TraversalStatus::Succeeded) {
      if (container(record.node))
        ++result.containers;
      else
        ++result.items;
    }
  }
  return result;
}
QList<CommitNode> CommitPlan::targets() const {
  QList<CommitNode> result;
  if (canceled) return result;
  QSet<QString> blocked;
  for (const auto& record : records)
    if (record.status != TraversalStatus::Succeeded) {
      auto path = record.node.path.toCaseFolded();
      while (!path.isEmpty()) {
        blocked.insert(path);
        const auto slash = path.lastIndexOf(u'\\');
        if (slash < 0) break;
        if (slash == 2 && path.size() >= 3 && path[1] == u':') {
          blocked.insert(path.left(3));
          break;
        }
        path.truncate(slash);
      }
    }
  QSet<QString> deletedKeys;
  if (request.operation == CommitOperation::DeleteAndPersist)
    for (const auto& record : records)
      if (record.status == TraversalStatus::Succeeded && record.node.kind == CommitTargetKind::RegistryKey &&
          !blocked.contains(record.node.path.toCaseFolded()))
        deletedKeys.insert(record.node.path.toCaseFolded());
  for (const auto& record : records) {
    if (record.status != TraversalStatus::Succeeded) continue;
    const auto& node = record.node;
    if (request.operation == CommitOperation::Persist) {
      if (!container(node)) result.append(node);
    } else {
      // Deleting a registry key includes its values. Never enqueue a parent
      // containing an unread/skipped branch, even when partial work is accepted.
      if (request.target.kind == CommitTargetKind::RegistryKey && node.kind == CommitTargetKind::RegistryValue &&
          (deletedKeys.contains(node.path.toCaseFolded()) || node.valueName.isEmpty()))
        continue;
      if (container(node) && blocked.contains(node.path.toCaseFolded())) continue;
      result.append(node);
    }
  }
  if (request.operation == CommitOperation::DeleteAndPersist)
    std::stable_sort(result.begin(), result.end(), [](const CommitNode& a, const CommitNode& b) { return a.path.count(u'\\') > b.path.count(u'\\'); });
  return result;
}
CommitPlan prepareCommit(const CommitRequest& request, const CommitTraversalSource& source, CommitPlan plan, const std::stop_token stop,
                         const CommitPreparationProgress& publish) {
  plan.request = request;
  plan.canceled = false;
  QHash<QString, qsizetype> indexes;
  QList<CommitNode> pending;
  for (qsizetype i = 0; i < plan.records.size(); ++i) {
    indexes.insert(plan.records[i].node.identity(), i);
    if (plan.records[i].status == TraversalStatus::Failed) pending.append(plan.records[i].node);
  }
  if (plan.records.isEmpty()) pending.append(request.target);
  QSet<QString> visited;
  CommitTraversalProgress counts = plan.progress();
  const auto indexOf = [&](const CommitNode& node) -> std::optional<qsizetype> {
    const auto found = indexes.constFind(node.identity());
    return found == indexes.cend() ? std::nullopt : std::optional<qsizetype>{*found};
  };
  const auto finished = [&](const CommitNode& node) {
    const auto index = indexOf(node);
    return index && plan.records[*index].status != TraversalStatus::Failed;
  };
  const auto updateCounts = [&](const CommitTraversalRecord& value, const int sign) {
    if (value.status == TraversalStatus::Failed)
      counts.failed += sign;
    else if (value.status == TraversalStatus::Succeeded) {
      if (container(value.node))
        counts.containers += sign;
      else
        counts.items += sign;
    }
  };
  const auto remember = [&](CommitTraversalRecord record, const std::optional<qsizetype> previous) {
    if (!record.error.isEmpty()) record.errorHistory.append(record.error);
    if (previous) {
      updateCounts(plan.records[*previous], -1);
      indexes.remove(plan.records[*previous].node.identity());
      plan.records[*previous] = record;
      indexes.insert(record.node.identity(), *previous);
    } else {
      indexes.insert(record.node.identity(), plan.records.size());
      plan.records.append(record);
    }
    updateCounts(record, 1);
    counts.path = record.node.display();
    counts.kind = plan.request.target.kind;
    if (publish) publish(counts);
  };
  const auto observeFailure = [](CommitTraversalRecord& record) {
    record.status = TraversalStatus::Failed;
    try {
      throw;
    } catch (const std::exception& error) {
      record.error = QString::fromUtf8(error.what());
    } catch (...) {
      record.error = ApplicationText::tr("The operation failed with an unknown error.");
    }
  };
  const auto discover = [&](const CommitNode& candidate) {
    if (stop.stop_requested() || visited.contains(candidate.identity()) || finished(candidate)) return;
    const auto previous = indexOf(candidate);
    CommitTraversalRecord record = previous ? plan.records[*previous] : CommitTraversalRecord{};
    record.node = candidate;
    record.error.clear();
    ++record.attempts;
    try {
      const auto observed = source.inspect(candidate);
      if (!observed) throw std::runtime_error(ApplicationText::tr("The target no longer exists.").toStdString());
      record.node = *observed;
      if (container(record.node) && record.node.skippedReason.isEmpty()) {
        pending.append(record.node);
        return;
      }
      record.status = record.node.skippedReason.isEmpty() ? TraversalStatus::Succeeded : TraversalStatus::Skipped;
      record.error = record.node.skippedReason;
    } catch (...) {
      observeFailure(record);
    }
    if (stop.stop_requested()) return;
    visited.insert(candidate.identity());
    visited.insert(record.node.identity());
    remember(std::move(record), previous);
  };
  while (!pending.isEmpty() && !stop.stop_requested()) {
    const auto candidate = pending.takeLast();
    if (visited.contains(candidate.identity()) || finished(candidate)) continue;
    visited.insert(candidate.identity());
    const auto previous = indexOf(candidate);
    CommitTraversalRecord record = previous ? plan.records[*previous] : CommitTraversalRecord{};
    record.node = candidate;
    record.error.clear();
    ++record.attempts;
    try {
      const auto observed = source.inspect(candidate);
      if (!observed) throw std::runtime_error(ApplicationText::tr("The target no longer exists.").toStdString());
      record.node = *observed;
      visited.insert(record.node.identity());
      if (record.node.path.compare(request.target.path, Qt::CaseInsensitive) == 0 &&
          record.node.valueName.compare(request.target.valueName, Qt::CaseInsensitive) == 0 &&
          (record.node.kind == request.target.kind || ((record.node.kind == CommitTargetKind::File || record.node.kind == CommitTargetKind::Folder) &&
                                                       (request.target.kind == CommitTargetKind::File || request.target.kind == CommitTargetKind::Folder))))
        plan.request.target = record.node;
      if (!record.node.skippedReason.isEmpty()) {
        record.status = TraversalStatus::Skipped;
        record.error = record.node.skippedReason;
      } else {
        record.error = source.read(record.node, stop, discover);
        record.status = record.error.isEmpty() ? TraversalStatus::Succeeded : TraversalStatus::Failed;
      }
    } catch (...) {
      observeFailure(record);
    }
    if (stop.stop_requested()) break;
    remember(std::move(record), previous);
  }
  plan.canceled = stop.stop_requested();
  return plan;
}
}  // namespace uwf::app
