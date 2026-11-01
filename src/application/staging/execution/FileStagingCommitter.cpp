// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "application/staging/execution/FileStagingCommitter.h"

#include <QDir>
#include <QElapsedTimer>
#include <QScopeGuard>
#include <algorithm>
#include <exception>
#include <set>
#include <stdexcept>
#include <system_error>
#include <utility>
#include <vector>

#include "application/commit/CommitPlanner.h"
#include "application/commit/contracts/CommitTraversalSource.h"
#include "application/paths/ExclusionCoverage.h"
#include "application/paths/PathPolicy.h"
#include "application/staging/editing/FileStagingPath.h"
#include "domain/uwf/contracts/Types.h"
#include "foundation/logging/Log.h"

namespace uwf::app {

namespace {

AutomaticCommitDisposition automaticCommitDisposition(const UwfMaintenanceRepository& repository, const std::string& volume) {
  const auto disposition = repository.automaticFileCommitDisposition(volume);
  if (disposition == AutomaticCommitDisposition::Unavailable)
    throw OperationError(std::make_error_code(std::errc::state_not_recoverable), "The automatic file commit state is unavailable");
  return disposition;
}

const char* failureKindName(const FileStagingCommitFailureKind kind) noexcept {
  switch (kind) {
    case FileStagingCommitFailureKind::InvalidPath:
      return "invalid-path";
    case FileStagingCommitFailureKind::ReparsePoint:
      return "reparse-point";
    case FileStagingCommitFailureKind::PathInspectionFailed:
      return "path-inspection-failed";
    case FileStagingCommitFailureKind::FileTypeChanged:
      return "file-type-changed";
    case FileStagingCommitFailureKind::DirectoryTypeChanged:
      return "directory-type-changed";
    case FileStagingCommitFailureKind::DirectoryEnumerationFailed:
      return "directory-enumeration-failed";
    case FileStagingCommitFailureKind::VolumeResolutionFailed:
      return "volume-resolution-failed";
    case FileStagingCommitFailureKind::VolumeRoot:
      return "volume-root";
    case FileStagingCommitFailureKind::ProviderFailure:
      return "provider-failure";
    case FileStagingCommitFailureKind::UnknownFailure:
      return "unknown-failure";
  }
  return "unknown-failure";
}

void recordFailure(QList<FileStagingCommitFailure>& failures, const QString& path, const FileStagingCommitFailureKind kind, const QString& detail = {},
                   std::optional<OperationFailure> operation = {}) {
  failures.append({path, kind, detail, operation});
  // Log each failure before summaries truncate it; logging must not abort the batch.
  try {
    UWF_LOG_E("staging") << "file staging item failed: path=" << path.toStdString() << " kind=" << failureKindName(kind) << " error=" << detail.toStdString();
  } catch (...) {
  }
}

using VolumeExclusions = std::map<std::string, std::vector<std::string>>;
ExclusionCoverage coverageOf(const VolumeExclusions& exclusions, const CommitNode& node) {
  if (!node.fileTarget) return {};
  const auto found = exclusions.find(node.fileTarget->volumeId);
  return found == exclusions.end() ? ExclusionCoverage{} : fileExclusionCoverage(found->second, node.fileTarget->relativePath);
}
class StagingTraversalSource final : public CommitTraversalSource {
 public:
  StagingTraversalSource(const CommitTraversalSource& files, const VolumeExclusions& exclusions) : m_exclusions(exclusions), m_files(files) {}
  bool exists(const CommitNode& target) const override { return m_files.exists(target); }
  std::string driveLetterForPath(const QString& path) const override { return m_files.driveLetterForPath(path); }
  FileCommitTarget resolveFileTarget(const QString& path) const override { return m_files.resolveFileTarget(path); }
  std::optional<CommitNode> inspect(const CommitNode& target) const override {
    const auto selected = target.fileTarget ? target : bindFileCommitTarget(m_files, target);
    validateFileCommitTarget(*selected.fileTarget);
    const auto coverage = coverageOf(m_exclusions, selected);
    if (!coverage.preventsCommit()) {
      auto observed = m_files.inspect(selected);
      if (observed) observed->fileTarget = selected.fileTarget;
      return observed;
    }
    if (!coverage.writesBypass()) throw std::runtime_error(coverage.unreliableCommitReason().toStdString());
    auto skipped = selected;
    skipped.skippedReason = coverage.directWriteExclusion;
    return skipped;
  }
  QString enumerateChildren(const CommitNode& target, std::stop_token stop, const CommitNodeVisitor& discover) const override {
    return m_files.enumerateChildren(target, stop, discover);
  }

 private:
  const VolumeExclusions& m_exclusions;
  const CommitTraversalSource& m_files;
};

}  // namespace

FileStagingCommitter::FileStagingCommitter(std::shared_ptr<UwfMaintenanceRepository> repository, std::shared_ptr<const CommitTraversalSource> source)
    : m_repository(repository), m_source(std::move(source)) {
  if (!m_repository || !m_source) throw std::invalid_argument("file staging requires a repository and traversal source");
}

FileStagingScanPlan FileStagingCommitter::prepareScan(const QList<app::FileStagingEntry>& entries) const {
  FileStagingScanPlan plan;
  plan.m_source = m_source;
  QList<app::FileStagingEntry> candidates;
  for (const auto& entry : entries) {
    if (!app::isAbsoluteLocalFileStagingPath(entry.path)) {
      recordFailure(plan.m_failures, entry.path, FileStagingCommitFailureKind::InvalidPath);
      continue;
    }
    candidates.append(entry);
  }
  if (candidates.isEmpty()) return plan;
  bool inventoryRead = false;
  for (const auto& entry : candidates) {
    CommitNode selected{entry.kind == FileStagingKind::Directory ? CommitTargetKind::Folder : CommitTargetKind::File, entry.path, {}, {}, entry.fileTarget};
    try {
      selected = bindFileCommitTarget(*m_source, std::move(selected));
    } catch (const std::exception& error) {
      recordFailure(plan.m_failures, entry.path, FileStagingCommitFailureKind::VolumeResolutionFailed, QString::fromUtf8(error.what()),
                    operationFailure(error, WriteEffect::None));
      continue;
    }
    const auto& id = selected.fileTarget->volumeId;
    if (automaticFileCommitPaused(automaticCommitDisposition(*m_repository, id))) {
      plan.m_pausedTargets.append(std::move(selected));
      continue;
    }
    plan.m_selectedTargets.append(selected);
    if (m_repository->fileCommitRequirement(id) == CommitRequirement::AlreadyPersistent) {
      plan.m_persistentTargets.append(selected);
      ++plan.m_skippedEntries;
      continue;
    }
    if (!inventoryRead) {
      plan.m_volumes = m_repository->prepareVolumeTargets();
      inventoryRead = true;
    }
    const auto found = std::ranges::find_if(plan.m_volumes, [&](const auto& target) { return compat::normalizeVolumeId(target->volume().volumeName) == id; });
    const auto volume = found == plan.m_volumes.end() ? nullptr : *found;
    if (!volume || !volume->volume().isProtected) throw std::runtime_error("The UWF inventory disagrees with the selected volume protection");
    if (!plan.m_fileExclusions.contains(id)) plan.m_fileExclusions.emplace(id, volume->exclusions());
    const auto coverage = coverageOf(plan.m_fileExclusions, selected);
    if (coverage.writesBypass()) {
      ++plan.m_skippedEntries;
      continue;
    }
    if (coverage.commitSkipped()) {
      recordFailure(plan.m_failures, entry.path, FileStagingCommitFailureKind::ProviderFailure, coverage.unreliableCommitReason());
      continue;
    }
    plan.m_entries.append(std::move(selected));
  }
  return plan;
}

FileStagingScan FileStagingCommitter::scan(FileStagingScanPlan plan, const std::stop_token stopToken) {
  FileStagingScan scan;
  scan.m_source = std::move(plan.m_source);
  scan.m_pausedTargets = std::move(plan.m_pausedTargets);
  scan.m_persistentTargets = std::move(plan.m_persistentTargets);
  scan.m_volumes = std::move(plan.m_volumes);
  scan.m_fileExclusions = std::move(plan.m_fileExclusions);
  scan.m_selectedTargets = std::move(plan.m_selectedTargets);
  scan.m_skippedEntries = plan.m_skippedEntries;
  scan.m_failures = std::move(plan.m_failures);
  std::set<ObjectIdentity> seenTargets;
  const StagingTraversalSource traversalSource(*scan.m_source, scan.m_fileExclusions);
  QElapsedTimer elapsed;
  elapsed.start();
  std::size_t visited = 0;
  constexpr qint64 kProgressLogIntervalMs = 10'000;
  constexpr std::size_t kProgressLogCheckStride = 1024;
  qint64 nextProgressMs = kProgressLogIntervalMs;
  UWF_LOG_I("staging") << "file staging scan started: entries=" << plan.m_entries.size() << " exclusions=" << scan.m_fileExclusions.size();
  const auto logCompletion = [&] {
    UWF_LOG_I("staging") << "file staging scan ended: visited=" << visited << " files=" << scan.m_files.size() << " failures=" << scan.m_failures.size()
                         << " canceled=" << stopToken.stop_requested() << " elapsedMs=" << elapsed.elapsed();
  };

  const auto addFile = [&](CommitNode node, const bool excluded) {
    const QString path = paths::normalize(node.path);
    node.path = path;
    const auto identity = ObjectIdentity(fileStagingObjectPath({FileStagingKind::File, path, node.fileTarget}));
    if (seenTargets.contains(identity)) return;
    seenTargets.insert(identity);
    if (excluded) {
      scan.m_selectedTargets.append(node);
      ++scan.m_skippedFiles;
      return;
    }
    scan.m_files.append(std::move(node));
  };

  for (const auto& entry : std::as_const(plan.m_entries)) {
    if (stopToken.stop_requested()) {
      logCompletion();
      return scan;
    }

    const QString path = paths::normalize(entry.path);
    QElapsedTimer entryElapsed;
    const auto filesBefore = seenTargets.size();
    if (entry.kind == CommitTargetKind::Folder) {
      entryElapsed.start();
      UWF_LOG_I("staging") << "scanning staged directory: path=" << path.toStdString();
    }
    const auto logEntryCompletion = qScopeGuard([&] {
      if (entry.kind == CommitTargetKind::Folder) {
        UWF_LOG_I("staging") << "staged directory scanned: path=" << path.toStdString() << " discoveredFiles=" << seenTargets.size() - filesBefore
                             << " elapsedMs=" << entryElapsed.elapsed();
      }
    });
    std::optional<CommitNode> node;
    try {
      const auto bound = bindFileCommitTarget(*scan.m_source, entry);
      node = scan.m_source->inspect(bound);
      if (node) node->fileTarget = bound.fileTarget;
    } catch (const std::exception& error) {
      recordFailure(scan.m_failures, path, FileStagingCommitFailureKind::PathInspectionFailed, QString::fromUtf8(error.what()),
                    operationFailure(error, WriteEffect::None));
      continue;
    }
    if (!node) {
      ++scan.m_skippedEntries;
      continue;
    }
    if (!node->skippedReason.isEmpty()) {
      recordFailure(scan.m_failures, path, FileStagingCommitFailureKind::ReparsePoint);
      continue;
    }

    if (entry.kind == CommitTargetKind::File) {
      if (node->kind != CommitTargetKind::File) {
        recordFailure(scan.m_failures, path, FileStagingCommitFailureKind::FileTypeChanged);
        continue;
      }
      const auto coverage = coverageOf(scan.m_fileExclusions, *node);
      if (coverage.commitSkipped() && !coverage.writesBypass()) {
        recordFailure(scan.m_failures, path, FileStagingCommitFailureKind::ProviderFailure, coverage.unreliableCommitReason());
        continue;
      }
      addFile(*node, coverage.writesBypass());
      continue;
    }

    if (node->kind != CommitTargetKind::Folder) {
      recordFailure(scan.m_failures, path, FileStagingCommitFailureKind::DirectoryTypeChanged);
      continue;
    }
    if (node->fileTarget->relativePath == QStringLiteral("\\")) {
      recordFailure(scan.m_failures, path, FileStagingCommitFailureKind::VolumeRoot);
      continue;
    }

    const auto prepared = prepareCommit({CommitOperation::Persist, *node}, traversalSource, {}, stopToken, [&](const CommitTraversalProgress& progress) {
      ++visited;
      if (visited % kProgressLogCheckStride == 0 && elapsed.elapsed() >= nextProgressMs) {
        UWF_LOG_I("staging") << "file staging scan progress: visited=" << visited << " files=" << seenTargets.size() << " elapsedMs=" << elapsed.elapsed()
                             << " path=" << progress.path.toStdString();
        nextProgressMs = elapsed.elapsed() + kProgressLogIntervalMs;
      }
    });
    for (const auto& record : prepared.records) {
      if (record.status == TraversalStatus::Succeeded && record.node.kind == CommitTargetKind::File)
        addFile(record.node, false);
      else if (record.status == TraversalStatus::Skipped && coverageOf(scan.m_fileExclusions, record.node).writesBypass()) {
        if (record.node.kind == CommitTargetKind::File)
          addFile(record.node, true);
        else
          scan.m_selectedTargets.append(record.node);
      } else if (record.status == TraversalStatus::Failed) {
        // Disappearance is an idempotent skip; access/enumeration failures remain explicit.
        try {
          if (!scan.m_source->exists(record.node)) continue;
        } catch (...) {
        }
        recordFailure(scan.m_failures, record.node.path, FileStagingCommitFailureKind::DirectoryEnumerationFailed, record.error);
      }
    }
    if (stopToken.stop_requested()) {
      logCompletion();
      return scan;
    }
  }

  logCompletion();
  return scan;
}

FileStagingCommitPlan FileStagingCommitter::prepare(FileStagingScan scan) const {
  FileStagingCommitPlan plan;
  plan.m_source = std::move(scan.m_source);
  plan.m_pausedTargets = std::move(scan.m_pausedTargets);
  plan.m_persistentTargets = std::move(scan.m_persistentTargets);
  // Discovery counts include skipped files; the execution list contains pending files only.
  plan.m_discoveredFiles = static_cast<std::size_t>(scan.m_files.size()) + scan.m_skippedFiles;
  plan.m_skippedFiles = scan.m_skippedFiles;
  plan.m_skippedEntries = scan.m_skippedEntries;
  plan.m_failures = std::move(scan.m_failures);
  plan.m_files = std::move(scan.m_files);
  plan.m_selectedTargets = std::move(scan.m_selectedTargets);
  plan.m_volumes = std::move(scan.m_volumes);
  return plan;
}

FileStagingCommitOperation::FileStagingCommitOperation(const std::shared_ptr<UwfMaintenanceRepository>& repository, FileStagingCommitPlan plan)
    : m_repository(repository), m_source(std::move(plan.m_source)), m_volumes(std::move(plan.m_volumes)) {
  m_result.discoveredFiles = plan.m_discoveredFiles;
  m_result.skippedFiles = plan.m_skippedFiles;
  m_result.skippedEntries = plan.m_skippedEntries;
  m_result.pausedEntries = static_cast<std::size_t>(plan.m_pausedTargets.size());
  m_result.automaticCommit = plan.m_automaticCommit;
  m_result.failures = std::move(plan.m_failures);
  const auto includeScope = [&](const CommitNode& selected) {
    if (selected.fileTarget) m_proposal.scopes.emplace_back(selected.identity(), plan.m_dispositions.at(selected.fileTarget->volumeId));
  };
  for (const auto& selected : std::as_const(plan.m_selectedTargets)) includeScope(selected);
  for (const auto& selected : std::as_const(plan.m_pausedTargets)) includeScope(selected);
  for (const auto& selected : std::as_const(plan.m_selectedTargets)) {
    if (selected.fileTarget && automaticFileCommitPaused(plan.m_dispositions.at(selected.fileTarget->volumeId))) ++m_result.pausedEntries;
  }
  for (auto& file : plan.m_files) {
    if (!file.fileTarget) throw std::logic_error("A prepared staging file has no bound volume");
    const auto disposition = plan.m_dispositions.at(file.fileTarget->volumeId);
    if (automaticFileCommitPaused(disposition))
      ++m_result.pausedFiles;
    else if (disposition == AutomaticCommitDisposition::AlreadyPersistent)
      ++m_result.skippedFiles;
    else
      m_files.append(std::move(file));
  }
  for (const auto& file : std::as_const(m_files)) m_proposal.files.push_back(file.identity());
  std::ranges::sort(m_proposal.scopes);
  m_proposal.scopes.erase(std::unique(m_proposal.scopes.begin(), m_proposal.scopes.end()), m_proposal.scopes.end());
  std::ranges::sort(m_proposal.files);
}

QString FileStagingCommitOperation::advance() {
  if (finished()) throw std::logic_error("cannot advance a completed file staging operation");

  const auto selected = m_files.at(m_nextFile);
  const QString path = selected.path;
  ++m_nextFile;

  std::optional<CommitNode> node;
  try {
    const auto bound = bindFileCommitTarget(*m_source, selected);
    const auto disposition = automaticCommitDisposition(*m_repository, bound.fileTarget->volumeId);
    m_result.automaticCommit.include(disposition);
    if (automaticFileCommitPaused(disposition)) {
      ++m_result.pausedFiles;
      return path;
    }
    node = m_source->inspect(bound);
    if (node) node->fileTarget = bound.fileTarget;
  } catch (const std::exception& error) {
    recordFailure(m_result.failures, path, FileStagingCommitFailureKind::PathInspectionFailed, QString::fromUtf8(error.what()),
                  operationFailure(error, WriteEffect::None));
    return path;
  }
  if (!node) {
    ++m_result.skippedFiles;
    return path;
  }
  if (!node->skippedReason.isEmpty()) {
    recordFailure(m_result.failures, path, FileStagingCommitFailureKind::ReparsePoint);
    return path;
  }
  if (node->kind != CommitTargetKind::File) {
    recordFailure(m_result.failures, path, FileStagingCommitFailureKind::FileTypeChanged);
    return path;
  }

  const auto& id = node->fileTarget->volumeId;
  const auto found = std::ranges::find_if(m_volumes, [&](const auto& target) { return compat::normalizeVolumeId(target->volume().volumeName) == id; });
  const auto volume = found == m_volumes.end() ? nullptr : *found;
  if (!volume || !volume->volume().isProtected) {
    recordFailure(m_result.failures, path, FileStagingCommitFailureKind::ProviderFailure, QStringLiteral("The prepared UWF volume is unavailable"));
    return path;
  }

  bool dispatched = false;
  try {
    const auto disposition = automaticCommitDisposition(*m_repository, id);
    m_result.automaticCommit.include(disposition);
    if (automaticFileCommitPaused(disposition)) {
      ++m_result.pausedFiles;
      return path;
    }
    if (m_repository->fileCommitRequirement(id) == CommitRequirement::AlreadyPersistent) {
      ++m_result.skippedFiles;
      return path;
    }
    dispatched = true;
    m_result.effect = combineWriteEffects(m_result.effect, volume->commitFile(node->fileTarget->relativePath.toStdString()));
    ++m_result.committedFiles;
  } catch (const std::exception& error) {
    recordFailure(m_result.failures, path, FileStagingCommitFailureKind::ProviderFailure, QString::fromUtf8(error.what()),
                  operationFailure(error, dispatched ? WriteEffect::Uncertain : WriteEffect::None));
  } catch (...) {
    recordFailure(m_result.failures, path, FileStagingCommitFailureKind::UnknownFailure, {},
                  OperationFailure{OperationFailureKind::Failed, dispatched ? WriteEffect::Uncertain : WriteEffect::None, {}});
  }

  return path;
}

FileStagingCommitOperation FileStagingCommitter::beginCommit(FileStagingCommitPlan plan) const {
  return revalidate(plan);
}
FileStagingCommitOperation FileStagingCommitter::revalidate(FileStagingCommitPlan& plan, const std::stop_token stop) const {
  // A paused automatic target is not a completed commit. If the pause no longer
  // applies after preparation, discover its entries before allowing shutdown.
  QList<FileStagingEntry> resumed;
  const auto persistent = std::exchange(plan.m_persistentTargets, {});
  for (const auto& selected : persistent) {
    auto bound = bindFileCommitTarget(*m_source, selected);
    const auto disposition = automaticCommitDisposition(*m_repository, bound.fileTarget->volumeId);
    if (disposition == AutomaticCommitDisposition::AlreadyPersistent) {
      plan.m_persistentTargets.append(std::move(bound));
      continue;
    }
    --plan.m_skippedEntries;
    plan.m_selectedTargets.removeIf([&](const auto& target) { return target.identity() == selected.identity(); });
    if (automaticFileCommitPaused(disposition))
      plan.m_pausedTargets.append(std::move(bound));
    else
      resumed.append({bound.kind == CommitTargetKind::Folder ? FileStagingKind::Directory : FileStagingKind::File, bound.path, bound.fileTarget});
  }
  const auto paused = std::exchange(plan.m_pausedTargets, {});
  for (const auto& selected : paused) {
    auto bound = bindFileCommitTarget(*m_source, selected);
    if (automaticFileCommitPaused(automaticCommitDisposition(*m_repository, bound.fileTarget->volumeId)))
      plan.m_pausedTargets.append(std::move(bound));
    else
      resumed.append({bound.kind == CommitTargetKind::Folder ? FileStagingKind::Directory : FileStagingKind::File, bound.path, bound.fileTarget});
  }
  if (!resumed.isEmpty()) {
    auto additional = prepare(scan(prepareScan(resumed), stop));
    plan.m_discoveredFiles += additional.m_discoveredFiles;
    plan.m_skippedFiles += additional.m_skippedFiles;
    plan.m_skippedEntries += additional.m_skippedEntries;
    plan.m_failures.append(additional.m_failures);
    plan.m_files.append(additional.m_files);
    plan.m_pausedTargets.append(additional.m_pausedTargets);
    plan.m_persistentTargets.append(additional.m_persistentTargets);
    plan.m_selectedTargets.append(additional.m_selectedTargets);
    for (auto& target : additional.m_volumes)
      if (std::ranges::none_of(plan.m_volumes, [&](const auto& existing) { return existing->volume().volumeName == target->volume().volumeName; }))
        plan.m_volumes.push_back(std::move(target));
  }
  // Revalidate durability after scanning, including plans whose exclusion-based skips leave no writes.
  std::set<std::string> volumes;
  plan.m_automaticCommit = {};
  plan.m_dispositions.clear();
  for (const auto& selected : std::as_const(plan.m_selectedTargets)) {
    const auto bound = bindFileCommitTarget(*m_source, selected);
    volumes.insert(bound.fileTarget->volumeId);
  }
  for (const auto& selected : std::as_const(plan.m_pausedTargets)) {
    const auto bound = bindFileCommitTarget(*m_source, selected);
    volumes.insert(bound.fileTarget->volumeId);
  }
  for (const auto& file : std::as_const(plan.m_files)) {
    if (!file.fileTarget) throw std::logic_error("A prepared staging file has no bound volume");
    volumes.insert(file.fileTarget->volumeId);
  }
  for (const auto& volume : volumes) {
    const auto disposition = automaticCommitDisposition(*m_repository, volume);
    plan.m_automaticCommit.include(disposition);
    plan.m_dispositions.emplace(volume, disposition);
    if (!automaticFileCommitPaused(disposition)) (void)m_repository->fileCommitRequirement(volume);
  }
  return FileStagingCommitOperation(m_repository, plan);
}

}  // namespace uwf::app
