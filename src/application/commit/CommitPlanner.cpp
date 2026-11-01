// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "application/commit/CommitPlanner.h"

#include <QStringView>
#include <algorithm>
#include <exception>
#include <map>
#include <set>
#include <utility>

#include "application/commit/contracts/CommitTraversalSource.h"
#include "application/localization/ApplicationText.h"
#include "application/operations/contracts/OperationError.h"
#include "application/paths/PathPolicy.h"
#include "domain/uwf/contracts/Types.h"
#include "foundation/text/Utf8.h"

namespace uwf::app {
bool FileCommitTarget::operator==(const FileCommitTarget& other) const {
  const auto volume = compat::parseVolumeId(volumeId);
  const auto otherVolume = compat::parseVolumeId(other.volumeId);
  if (!volume || !otherVolume) return volumeId == other.volumeId && relativePath == other.relativePath;
  return volume == otherVolume && equalObjectNames(relativePath, other.relativePath);
}
void validateCommitNodeText(const CommitNode& node) {
  const auto valid = [](const QString& text) { return !text.contains(QChar::Null) && isValidUtf16(QStringView(text)); };
  if (!valid(node.path) || !valid(node.valueName) || (node.fileTarget && !valid(node.fileTarget->relativePath)))
    throw OperationError(std::make_error_code(std::errc::invalid_argument),
                         ApplicationText::tr("A parameter was rejected by the system (invalid path or argument).").toStdString());
}
void validateFileCommitTarget(const FileCommitTarget& target) {
  if (!compat::parseVolumeId(target.volumeId) || target.relativePath.isEmpty() || !target.relativePath.startsWith(u'\\') ||
      target.relativePath.startsWith(QStringLiteral("\\\\")) || target.relativePath.contains(QChar::Null) || !isValidUtf16(QStringView(target.relativePath)))
    throw OperationError(std::make_error_code(std::errc::protocol_error), "The file source returned an invalid volume-relative identity");
}
CommitNode bindFileCommitTarget(const CommitTraversalSource& source, CommitNode target) {
  validateCommitNodeText(target);
  if (target.kind == CommitTargetKind::RegistryKey || target.kind == CommitTargetKind::RegistryValue) return target;
  auto identity = source.resolveFileTarget(target.path);
  validateFileCommitTarget(identity);
  identity.volumeId = compat::normalizeVolumeId(identity.volumeId);
  if (target.fileTarget && *target.fileTarget != identity)
    throw OperationError(std::make_error_code(std::errc::device_or_resource_busy), "The file path now resolves to a different volume or object");
  target.fileTarget = std::move(identity);
  return target;
}
namespace {
bool container(const CommitNode& node) { return node.kind == CommitTargetKind::Folder || node.kind == CommitTargetKind::RegistryKey; }
QString objectPath(const CommitNode& node) {
  if ((node.kind == CommitTargetKind::File || node.kind == CommitTargetKind::Folder) && node.fileTarget)
    return paths::volumeFilePath({node.fileTarget->volumeId, node.fileTarget->relativePath});
  return node.path;
}
QString scopePath(const CommitNode& node) {
  auto path = objectPath(node);
  if (node.kind == CommitTargetKind::File || node.kind == CommitTargetKind::Folder)
    while (path.size() > 3 && path.endsWith(u'\\')) path.chop(1);
  return path;
}
bool compactRecords(QList<CommitTraversalRecord>& records) {
  std::map<CommitNodeIdentity, qsizetype> indexes;
  QList<CommitTraversalRecord> unique;
  const auto priority = [](const TraversalStatus status) { return status == TraversalStatus::Failed ? 2 : status == TraversalStatus::Skipped ? 1 : 0; };
  for (const auto& record : std::as_const(records)) {
    auto captured = record;
    if (!captured.error.isEmpty() && !captured.errorHistory.contains(captured.error)) captured.errorHistory.append(captured.error);
    const auto [found, inserted] = indexes.try_emplace(record.node.identity(), unique.size());
    if (inserted) {
      unique.append(std::move(captured));
      continue;
    }
    auto& retained = unique[found->second];
    const auto history = retained.errorHistory;
    const auto attempts = std::max(retained.attempts, record.attempts);
    if (priority(captured.status) > priority(retained.status)) retained = captured;
    for (const auto& error : history)
      if (!retained.errorHistory.contains(error)) retained.errorHistory.append(error);
    for (const auto& error : std::as_const(captured.errorHistory))
      if (!retained.errorHistory.contains(error)) retained.errorHistory.append(error);
    retained.attempts = attempts;
  }
  const bool changed = unique.size() != records.size();
  records = std::move(unique);
  return changed;
}
}  // namespace
CommitNodeIdentity CommitNode::identity() const {
  return {kind == CommitTargetKind::Folder ? CommitTargetKind::File : kind, ObjectIdentity(objectPath(*this)), ObjectIdentity(valueName)};
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
  std::set<CommitNodeIdentity> emitted;
  std::set<CommitNodeIdentity> unavailable;
  std::set<QString, ObjectNameLess> blocked;
  const auto blockAncestors = [&](QString path) {
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
  };
  for (const auto& record : records)
    if (record.status != TraversalStatus::Succeeded) {
      unavailable.insert(record.node.identity());
      blockAncestors(scopePath(record.node));
      // Rejected/unread bindings need not describe the selected object. Keep
      // the display ancestry blocked as well so malformed observations cannot
      // make a containing deletion appear complete.
      auto display = record.node;
      display.fileTarget.reset();
      blockAncestors(scopePath(display));
    }
  if (!unavailable.empty() && container(request.target)) {
    // Every record came from this selected traversal. Even an unresolved legacy
    // alias is an incomplete branch of its root, regardless of its spelling.
    blockAncestors(scopePath(request.target));
    auto display = request.target;
    display.fileTarget.reset();
    blockAncestors(scopePath(display));
  }
  std::set<QString, ObjectNameLess> deletedKeys;
  if (request.operation == CommitOperation::DeleteAndPersist)
    for (const auto& record : records)
      if (record.status == TraversalStatus::Succeeded && record.node.kind == CommitTargetKind::RegistryKey && !blocked.contains(record.node.path))
        deletedKeys.insert(record.node.path);
  for (const auto& record : records) {
    if (record.status != TraversalStatus::Succeeded) continue;
    const auto& node = record.node;
    if (emitted.contains(node.identity()) || unavailable.contains(node.identity())) continue;
    if (request.operation == CommitOperation::Persist) {
      if (!container(node)) {
        emitted.insert(node.identity());
        result.append(node);
      }
    } else {
      // Deleting a registry key includes its values. Never enqueue a parent
      // containing an unread/skipped branch, even when partial work is accepted.
      if (request.target.kind == CommitTargetKind::RegistryKey && node.kind == CommitTargetKind::RegistryValue &&
          (deletedKeys.contains(node.path) || node.valueName.isEmpty()))
        continue;
      if (container(node)) {
        auto display = node;
        display.fileTarget.reset();
        if (blocked.contains(scopePath(node)) || blocked.contains(scopePath(display))) continue;
      }
      emitted.insert(node.identity());
      result.append(node);
    }
  }
  if (request.operation == CommitOperation::DeleteAndPersist)
    std::stable_sort(result.begin(), result.end(),
                     [](const CommitNode& a, const CommitNode& b) { return scopePath(a).count(u'\\') > scopePath(b).count(u'\\'); });
  return result;
}
CommitPlan prepareCommit(const CommitRequest& request, const CommitTraversalSource& source, CommitPlan plan, const std::stop_token stop,
                         const CommitPreparationProgress& publish) {
  const auto previousTarget = plan.request.target;
  compactRecords(plan.records);
  std::map<CommitNodeIdentity, CommitTraversalRecord> history;
  if (request.preparationIntent == CommitPreparationIntent::TraverseScope) {
    for (const auto& record : std::as_const(plan.records)) history.emplace(record.node.identity(), record);
    plan.records.clear();
  }
  plan.request = request;
  if (request.preparationIntent == CommitPreparationIntent::TraverseScope) {
    plan.request.target.skippedReason.clear();
    if (!plan.request.target.fileTarget && previousTarget.fileTarget) plan.request.target.fileTarget = previousTarget.fileTarget;
  }
  plan.canceled = false;
  std::map<CommitNodeIdentity, qsizetype> indexes;
  enum class Observation { Required, Available, Retry };
  struct PendingNode {
    CommitNode node;
    Observation observation = Observation::Required;
  };
  QList<PendingNode> pending;
  for (qsizetype i = 0; i < plan.records.size(); ++i) {
    indexes.emplace(plan.records[i].node.identity(), i);
    if (plan.records[i].status == TraversalStatus::Failed) {
      const auto& node = plan.records[i].node;
      const bool selected = request.target.fileTarget && node.identity() == request.target.identity() && node.fileTarget == request.target.fileTarget;
      pending.append(PendingNode{node, selected ? Observation::Required : Observation::Retry});
    }
  }
  if (plan.records.isEmpty()) pending.append(PendingNode{plan.request.target});
  std::set<CommitNodeIdentity> visited;
  CommitTraversalProgress counts = plan.progress();
  const auto indexOf = [&](const CommitNode& node) -> std::optional<qsizetype> {
    const auto found = indexes.find(node.identity());
    return found == indexes.cend() ? std::nullopt : std::optional<qsizetype>{found->second};
  };
  const auto finished = [&](const CommitNode& node) {
    const auto index = indexOf(node);
    return index && plan.records[*index].status != TraversalStatus::Failed;
  };
  const auto seedRecord = [&](const CommitNode& node, const std::optional<qsizetype> previous) {
    CommitTraversalRecord record;
    if (previous)
      record = plan.records[*previous];
    else if (const auto old = history.find(node.identity()); old != history.end())
      record = old->second;
    record.node = node;
    record.error.clear();
    ++record.attempts;
    return record;
  };
  const auto restoreObservedHistory = [&](CommitTraversalRecord& record, std::optional<qsizetype>& previous) {
    if (previous) return;
    previous = indexOf(record.node);
    if (previous) {
      record.errorHistory = plan.records[*previous].errorHistory;
      record.attempts = plan.records[*previous].attempts + 1;
    } else if (const auto old = history.find(record.node.identity()); old != history.end()) {
      record.errorHistory = old->second.errorHistory;
      record.attempts = old->second.attempts + 1;
    }
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
      indexes.erase(plan.records[*previous].node.identity());
      plan.records[*previous] = record;
      indexes.insert_or_assign(record.node.identity(), *previous);
    } else {
      indexes.insert_or_assign(record.node.identity(), plan.records.size());
      plan.records.append(record);
    }
    updateCounts(record, 1);
    counts.path = record.node.display();
    counts.kind = plan.request.target.kind;
    if (publish) publish(counts);
  };
  const auto reuseAlias = [&](const qsizetype existing, const qsizetype previous) {
    auto record = plan.records[existing];
    const auto& old = plan.records[previous];
    record.attempts = std::max(record.attempts, old.attempts);
    for (const auto& error : old.errorHistory)
      if (!record.errorHistory.contains(error)) record.errorHistory.append(error);
    if (!old.error.isEmpty() && !record.errorHistory.contains(old.error)) record.errorHistory.append(old.error);
    remember(std::move(record), previous);
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
    auto previous = indexOf(candidate);
    auto record = seedRecord(candidate, previous);
    try {
      validateCommitNodeText(candidate);
      const auto observed = source.inspect(candidate);
      if (!observed) throw std::runtime_error(ApplicationText::tr("The target no longer exists.").toStdString());
      validateCommitNodeText(*observed);
      record.node = *observed;
      // Inspection can attach a volume identity to an unbound display path.
      // Check it before publishing or queueing a second alias of that object.
      if (record.node.identity() != candidate.identity() && (visited.contains(record.node.identity()) || finished(record.node))) {
        visited.insert(candidate.identity());
        if (previous) {
          const auto existing = indexOf(record.node);
          if (existing) reuseAlias(*existing, *previous);
        }
        return;
      }
      restoreObservedHistory(record, previous);
      if (container(record.node) && record.node.skippedReason.isEmpty()) {
        pending.append(PendingNode{record.node, Observation::Available});
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
    const auto pendingNode = pending.takeLast();
    const auto& candidate = pendingNode.node;
    if (visited.contains(candidate.identity()) || finished(candidate)) continue;
    visited.insert(candidate.identity());
    auto previous = indexOf(candidate);
    auto record = seedRecord(candidate, previous);
    try {
      validateCommitNodeText(candidate);
      const auto selected = pendingNode.observation == Observation::Retry && candidate.fileTarget ? bindFileCommitTarget(source, candidate) : candidate;
      const auto observed = pendingNode.observation == Observation::Available ? std::optional{selected} : source.inspect(selected);
      if (!observed) throw std::runtime_error(ApplicationText::tr("The target no longer exists.").toStdString());
      validateCommitNodeText(*observed);
      record.node = *observed;
      if (record.node.identity() != candidate.identity() && (visited.contains(record.node.identity()) || finished(record.node))) {
        if (previous) {
          const auto existing = indexOf(record.node);
          if (existing) reuseAlias(*existing, *previous);
        }
        continue;
      }
      restoreObservedHistory(record, previous);
      visited.insert(record.node.identity());
      if (equalObjectNames(record.node.path, request.target.path) && equalObjectNames(record.node.valueName, request.target.valueName) &&
          (record.node.kind == request.target.kind || ((record.node.kind == CommitTargetKind::File || record.node.kind == CommitTargetKind::Folder) &&
                                                       (request.target.kind == CommitTargetKind::File || request.target.kind == CommitTargetKind::Folder))))
        plan.request.target = record.node;
      if (!record.node.skippedReason.isEmpty()) {
        record.status = TraversalStatus::Skipped;
        record.error = record.node.skippedReason;
      } else {
        record.error = source.enumerateChildren(record.node, stop, discover);
        record.status = record.error.isEmpty() ? TraversalStatus::Succeeded : TraversalStatus::Failed;
      }
    } catch (...) {
      observeFailure(record);
    }
    if (stop.stop_requested()) break;
    remember(std::move(record), previous);
  }
  plan.canceled = stop.stop_requested();
  if (compactRecords(plan.records) && publish) {
    auto finalProgress = plan.progress();
    finalProgress.kind = plan.request.target.kind;
    finalProgress.path = counts.path;
    publish(finalProgress);
  }
  return plan;
}
}  // namespace uwf::app
