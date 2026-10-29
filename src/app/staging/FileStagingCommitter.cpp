// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "FileStagingCommitter.h"

#include <QDir>
#include <QElapsedTimer>
#include <QScopeGuard>
#include <QSet>
#include <algorithm>
#include <exception>
#include <stdexcept>
#include <utility>
#include <vector>

#include "app/commit/CommitPreparation.h"
#include "app/paths/PathExclusionIndex.h"
#include "app/paths/PathPolicy.h"
#include "app/staging/FileStagingPath.h"
#include "app/wmi/WmiError.h"
#include "app/wmi/WmiException.h"
#include "shared/Log.h"

namespace uwf::app {

namespace {

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
    case FileStagingCommitFailureKind::MissingDriveLetter:
      return "missing-drive-letter";
    case FileStagingCommitFailureKind::VolumeRoot:
      return "volume-root";
    case FileStagingCommitFailureKind::ProviderFailure:
      return "provider-failure";
    case FileStagingCommitFailureKind::UnknownFailure:
      return "unknown-failure";
  }
  return "unknown-failure";
}

void recordFailure(QList<FileStagingCommitFailure>& failures, const QString& path, const FileStagingCommitFailureKind kind, const QString& detail = {}) {
  failures.append({path, kind, detail});
  // Log where the failure is produced, before any UI limit or batch reuse.
  // Logging must not turn a recorded item failure into an aborted batch.
  try {
    UWF_LOG_E("staging") << "file staging item failed: path=" << path.toStdString() << " kind=" << failureKindName(kind) << " error=" << detail.toStdString();
  } catch (...) {
  }
}

class StagingTraversalSource final : public CommitTraversalSource {
 public:
  StagingTraversalSource(const CommitTraversalSource& files, const paths::PathExclusionIndex& exclusions) : m_exclusions(exclusions), m_files(files) {}
  bool exists(const CommitNode& target) const override { return m_files.exists(target); }
  std::string volumeForPath(const QString& path) const override { return m_files.volumeForPath(path); }
  std::vector<std::string> mountedLetters(const std::string& volume) const override { return m_files.mountedLetters(volume); }
  std::optional<CommitNode> inspect(const CommitNode& target) const override {
    if (!m_exclusions.contains(target.path)) return m_files.inspect(target);
    auto skipped = target;
    skipped.skippedReason = m_exclusions.covering(target.path);
    return skipped;
  }
  QString read(const CommitNode& target, std::stop_token stop, const CommitNodeVisitor& discover) const override {
    return m_files.read(target, stop, discover);
  }

 private:
  const paths::PathExclusionIndex& m_exclusions;
  const CommitTraversalSource& m_files;
};

QString absoluteExclusionPath(const provider::VolumeRow& volume, const provider::ExcludedFile& exclusion) {
  const QString path = paths::normalize(QString::fromStdString(exclusion.fileName));
  if (path.startsWith('\\')) return QString::fromStdString(volume.driveLetter) + path;
  return path;
}

}  // namespace

FileStagingCommitter::FileStagingCommitter(wmi::WmiOperations& session, std::shared_ptr<const CommitTraversalSource> source)
    : m_session(session.threadAccessor()),
      m_source(std::move(source)),
      m_volume(session, [source = m_source](const std::string& volume) { return source->mountedLetters(volume); }) {
  if (!m_source) throw std::invalid_argument("file staging traversal source is required");
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

  // These snapshots govern the whole batch. Let its owner handle an unavailable
  // provider instead of inventing an item failure without a target.
  QElapsedTimer elapsed;
  elapsed.start();
  UWF_LOG_I("staging") << "reading file staging volume snapshot";
  plan.m_volumes = m_volume.readAll();
  UWF_LOG_I("staging") << "file staging volume snapshot read: rows=" << plan.m_volumes.size() << " elapsedMs=" << elapsed.elapsed();
  for (const auto& volume : plan.m_volumes) {
    if (!volume.currentSession || !volume.isProtected) continue;
    UWF_LOG_I("staging") << "reading file staging exclusions: drive=" << volume.driveLetter;
    elapsed.start();
    const auto exclusions = m_volume.getExclusions(volume);
    UWF_LOG_I("staging") << "file staging exclusions read: drive=" << volume.driveLetter << " count=" << exclusions.size()
                         << " elapsedMs=" << elapsed.elapsed();
    for (const auto& exclusion : exclusions) {
      if (!exclusion.fileName.empty()) plan.m_fileExclusions.append(absoluteExclusionPath(volume, exclusion));
    }
  }

  const paths::PathExclusionIndex exclusions(plan.m_fileExclusions);
  for (const auto& entry : candidates) {
    std::string driveLetter;
    try {
      driveLetter = m_source->volumeForPath(entry.path);
    } catch (const std::exception& error) {
      recordFailure(plan.m_failures, entry.path, FileStagingCommitFailureKind::MissingDriveLetter, QString::fromUtf8(error.what()));
      continue;
    }
    if (driveLetter.empty()) {
      recordFailure(plan.m_failures, entry.path, FileStagingCommitFailureKind::MissingDriveLetter);
      continue;
    }

    const auto* volume =
        provider::findBySession(plan.m_volumes, provider::Session::Current, [&](const provider::VolumeRow& row) { return row.driveLetter == driveLetter; });
    if (!volume || !volume->isProtected) {
      ++plan.m_skippedEntries;
      continue;
    }
    if (exclusions.contains(entry.path)) {
      ++plan.m_skippedEntries;
      continue;
    }
    plan.m_entries.append(entry);
  }
  return plan;
}

FileStagingScan FileStagingCommitter::scan(FileStagingScanPlan plan, const std::stop_token stopToken, std::atomic_size_t* const discoveredProgress) {
  FileStagingScan scan;
  scan.m_source = std::move(plan.m_source);
  scan.m_volumes = std::move(plan.m_volumes);
  scan.m_fileExclusions = std::move(plan.m_fileExclusions);
  scan.m_skippedEntries = plan.m_skippedEntries;
  scan.m_failures = std::move(plan.m_failures);
  const paths::PathExclusionIndex exclusions(scan.m_fileExclusions);
  QSet<QString> seenTargets;
  QSet<QString> announcedTargets;
  const StagingTraversalSource traversalSource(*scan.m_source, exclusions);
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

  const auto announceFile = [&](const QString& path) {
    const auto identity = paths::identity(path);
    if (announcedTargets.contains(identity)) return;
    announcedTargets.insert(identity);
    if (discoveredProgress) discoveredProgress->fetch_add(1, std::memory_order_relaxed);
  };
  const auto addFile = [&](const QString& rawPath, const bool excluded) {
    const QString path = paths::normalize(rawPath);
    const QString identity = path.toCaseFolded();
    if (seenTargets.contains(identity)) return;
    seenTargets.insert(identity);
    announceFile(path);
    if (excluded) {
      ++scan.m_skippedFiles;
      return;
    }
    scan.m_files.append(path);
  };

  for (const auto& entry : std::as_const(plan.m_entries)) {
    if (stopToken.stop_requested()) {
      logCompletion();
      return scan;
    }

    const QString path = paths::normalize(entry.path);
    QElapsedTimer entryElapsed;
    const auto filesBefore = seenTargets.size();
    if (entry.kind == app::FileStagingKind::Directory) {
      entryElapsed.start();
      UWF_LOG_I("staging") << "scanning staged directory: path=" << path.toStdString();
    }
    const auto logEntryCompletion = qScopeGuard([&] {
      if (entry.kind == app::FileStagingKind::Directory) {
        UWF_LOG_I("staging") << "staged directory scanned: path=" << path.toStdString() << " discoveredFiles=" << seenTargets.size() - filesBefore
                             << " elapsedMs=" << entryElapsed.elapsed();
      }
    });
    std::optional<CommitNode> node;
    try {
      node = scan.m_source->inspect({CommitTargetKind::File, path, {}, {}});
    } catch (const std::exception& error) {
      recordFailure(scan.m_failures, path, FileStagingCommitFailureKind::PathInspectionFailed, QString::fromUtf8(error.what()));
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

    if (entry.kind == app::FileStagingKind::File) {
      if (node->kind != CommitTargetKind::File) {
        recordFailure(scan.m_failures, path, FileStagingCommitFailureKind::FileTypeChanged);
        continue;
      }
      addFile(path, exclusions.contains(path));
      continue;
    }

    if (node->kind != CommitTargetKind::Folder) {
      recordFailure(scan.m_failures, path, FileStagingCommitFailureKind::DirectoryTypeChanged);
      continue;
    }
    try {
      if (scan.m_source->volumeForPath(path).empty()) {
        recordFailure(scan.m_failures, path, FileStagingCommitFailureKind::MissingDriveLetter);
        continue;
      }
    } catch (const std::exception& error) {
      recordFailure(scan.m_failures, path, FileStagingCommitFailureKind::MissingDriveLetter, QString::fromUtf8(error.what()));
      continue;
    }
    if (paths::isDriveRoot(path)) {
      recordFailure(scan.m_failures, path, FileStagingCommitFailureKind::VolumeRoot);
      continue;
    }

    qsizetype previousItems = 0;
    const auto prepared = prepareCommit({CommitOperation::Persist, {CommitTargetKind::Folder, path, {}, {}}}, traversalSource, {}, stopToken,
                                        [&](const CommitTraversalProgress& progress) {
                                          ++visited;
                                          if (progress.items > previousItems) announceFile(progress.path);
                                          previousItems = progress.items;
                                          if (visited % kProgressLogCheckStride == 0 && elapsed.elapsed() >= nextProgressMs) {
                                            UWF_LOG_I("staging") << "file staging scan progress: visited=" << visited << " files=" << announcedTargets.size()
                                                                 << " elapsedMs=" << elapsed.elapsed() << " path=" << progress.path.toStdString();
                                            nextProgressMs = elapsed.elapsed() + kProgressLogIntervalMs;
                                          }
                                        });
    for (const auto& record : prepared.records) {
      if (record.status == TraversalStatus::Succeeded && record.node.kind == CommitTargetKind::File)
        addFile(record.node.path, false);
      else if (record.status == TraversalStatus::Skipped && exclusions.contains(record.node.path)) {
        if (record.node.kind == CommitTargetKind::File) addFile(record.node.path, true);
      } else if (record.status == TraversalStatus::Failed) {
        // Disappearance during a staging scan is an ordinary idempotent skip.
        // Access and enumeration failures remain explicit failures for the
        // power workflow's separate decision about continuing shutdown.
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
  // 发现总数包含扫描期间已跳过的文件；实际提交清单仍只包含待处理文件。
  plan.m_discoveredFiles = static_cast<std::size_t>(scan.m_files.size()) + scan.m_skippedFiles;
  plan.m_skippedFiles = scan.m_skippedFiles;
  plan.m_skippedEntries = scan.m_skippedEntries;
  plan.m_failures = std::move(scan.m_failures);
  plan.m_files = std::move(scan.m_files);
  plan.m_volumes = std::move(scan.m_volumes);
  return plan;
}

FileStagingCommitOperation::FileStagingCommitOperation(const std::function<wmi::WmiOperations&()>& session, FileStagingCommitPlan plan)
    : m_session(session), m_source(std::move(plan.m_source)), m_files(std::move(plan.m_files)), m_volumes(std::move(plan.m_volumes)) {
  m_result.discoveredFiles = plan.m_discoveredFiles;
  m_result.skippedFiles = plan.m_skippedFiles;
  m_result.skippedEntries = plan.m_skippedEntries;
  m_result.failures = std::move(plan.m_failures);
}

QString FileStagingCommitOperation::advance() {
  if (finished()) throw std::logic_error("cannot advance a completed file staging operation");

  const QString path = m_files.at(m_nextFile);
  ++m_nextFile;

  std::optional<CommitNode> node;
  try {
    node = m_source->inspect({CommitTargetKind::File, path, {}, {}});
  } catch (const std::exception& error) {
    recordFailure(m_result.failures, path, FileStagingCommitFailureKind::PathInspectionFailed, QString::fromUtf8(error.what()));
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

  std::string driveLetter;
  try {
    driveLetter = m_source->volumeForPath(path);
  } catch (const std::exception& error) {
    recordFailure(m_result.failures, path, FileStagingCommitFailureKind::MissingDriveLetter, QString::fromUtf8(error.what()));
    return path;
  }
  const auto* volume =
      provider::findBySession(m_volumes, provider::Session::Current, [&](const provider::VolumeRow& row) { return row.driveLetter == driveLetter; });
  if (!volume || !volume->isProtected) {
    // 计划和执行之间卷状态理论上不会在同一 Windows 会话中变化；仍在边界处
    // 重新核验，避免对不再受保护的卷调用 provider。
    ++m_result.skippedFiles;
    return path;
  }

  // Resolving this worker's transport is a batch prerequisite. Keep it outside
  // the target's provider invocation boundary, after all idempotent skips.
  auto& transport = m_session();
  try {
    provider::UwfVolume(transport, [this](const std::string& name) { return m_source->mountedLetters(name); }).commitFile(*volume, path.toStdString());
    ++m_result.committedFiles;
  } catch (const wmi::WmiException& error) {
    // CommitFile 对“不在覆盖层中”的现存文件返回 NotFound。暂存列表会跨多次
    // 关机保留，因此这是正常幂等结果，不能让之后每次关机都被永久阻止。
    if (error.hasCode(wmi::WmiErrorCode::NotFound)) {
      ++m_result.skippedFiles;
    } else {
      recordFailure(m_result.failures, path, FileStagingCommitFailureKind::ProviderFailure, QString::fromUtf8(error.what()));
    }
  } catch (const std::exception& error) {
    recordFailure(m_result.failures, path, FileStagingCommitFailureKind::ProviderFailure, QString::fromUtf8(error.what()));
  } catch (...) {
    recordFailure(m_result.failures, path, FileStagingCommitFailureKind::UnknownFailure);
  }

  return path;
}

FileStagingCommitOperation FileStagingCommitter::beginCommit(FileStagingCommitPlan plan) const {
  return FileStagingCommitOperation(m_session, std::move(plan));
}

}  // namespace uwf::app
