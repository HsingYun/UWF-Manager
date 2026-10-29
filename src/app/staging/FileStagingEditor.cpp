// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "FileStagingEditor.h"

#include <QDir>
#include <algorithm>
#include <exception>
#include <stdexcept>
#include <utility>

#include "app/paths/PathPolicy.h"
#include "app/staging/FileStagingConflictPolicy.h"
#include "app/staging/FileStagingPath.h"
#include "shared/DriveLetter.h"
#include "shared/Log.h"

namespace uwf::app {

namespace {

void sortEntries(QList<FileStagingEntry>& entries) {
  std::ranges::sort(entries,
                    [](const FileStagingEntry& lhs, const FileStagingEntry& rhs) { return QString::compare(lhs.path, rhs.path, Qt::CaseInsensitive) < 0; });
}

bool isCoveredByParentDirectory(const QList<FileStagingEntry>& entries, const QString& path) {
  const QString identity = paths::identity(path);
  return std::ranges::any_of(entries, [&](const FileStagingEntry& entry) {
    return entry.kind == FileStagingKind::Directory && paths::identity(entry.path) != identity && paths::contains(entry.path, path);
  });
}

QList<FileStagingEntry> entriesForDrive(const QList<FileStagingEntry>& entries, const QString& driveLetter, const CommitTraversalSource& source) {
  QList<FileStagingEntry> filtered;
  for (const auto& entry : entries) {
    QString entryDrive;
    try {
      entryDrive = QString::fromStdString(source.volumeForPath(entry.path));
    } catch (const std::exception&) {
      // 已断开或已失效的卷 GUID 没有对应磁盘页；保留在全局存储中，待卷重新
      // 出现后即可重新归入其页面，不能因为单条路径暂时无法解析而禁用所有页。
      continue;
    }
    if (entryDrive.compare(driveLetter, Qt::CaseInsensitive) == 0) filtered.append(entry);
  }
  return filtered;
}

}  // namespace

FileStagingEditor::FileStagingEditor(QString driveLetter, FileStagingDraft& draft, std::shared_ptr<const CommitTraversalSource> source,
                                     FileStagingConflictPolicy* conflicts)
    : m_driveLetter(QString::fromStdString(drive::normalize(driveLetter.toStdString()))),

      m_draft(draft),
      m_source(std::move(source)),
      m_conflicts(conflicts) {
  if (m_driveLetter.isEmpty()) throw std::invalid_argument("file staging requires a valid drive letter");
  if (!m_source) throw std::invalid_argument("file staging traversal source is required");
  connect(&m_draft, &FileStagingDraft::changed, this, &FileStagingEditor::load, Qt::QueuedConnection);
}

QList<FileStagingEntry> FileStagingEditor::baselineEntries() const { return entriesForDrive(m_draft.baseline(), m_driveLetter, *m_source); }

void FileStagingEditor::restorePaths(const QStringList& paths) {
  auto candidate = m_draft.load();
  for (const auto baseline = baselineEntries(); const auto& entry : baseline) {
    if (!paths.contains(entry.path, Qt::CaseInsensitive)) continue;
    if (m_conflicts) {
      if (const auto conflict = m_conflicts->conflictingExclusion(entry.path)) {
        reject(FileStagingRejection::Kind::ExclusionConflict, *conflict);
        return;
      }
    }
    if (!std::ranges::any_of(candidate, [&](const auto& other) { return paths::identity(other.path) == paths::identity(entry.path); })) candidate.append(entry);
  }
  updateDraft(candidate);
}

void FileStagingEditor::load() {
  try {
    const auto storedEntries = m_draft.load();
    if (m_conflicts) m_conflicts->setStagedEntries(storedEntries);
    m_entries = entriesForDrive(storedEntries, m_driveLetter, *m_source);
    sortEntries(m_entries);
    const bool recovered = !m_storageAvailable;
    m_storageAvailable = true;
    m_storageError.clear();
    if (recovered) emit storageAvailabilityChanged();
    emit entriesChanged();
  } catch (const std::exception& error) {
    setStorageUnavailable(QString::fromUtf8(error.what()));
  }
}

void FileStagingEditor::addEntries(const QStringList& paths, const FileStagingKind kind) {
  QList<FileStagingEntry> candidate;
  try {
    candidate = m_draft.load();
  } catch (const std::exception& error) {
    reportUpdateFailure(QString::fromUtf8(error.what()));
    return;
  }

  bool changed = false;
  for (const auto& rawPath : paths) {
    const QString path = paths::normalize(rawPath);
    if (path.isEmpty()) continue;
    if (!isAbsoluteLocalFileStagingPath(path)) {
      reject(FileStagingRejection::Kind::NonLocalPath, path);
      continue;
    }
    std::optional<CommitNode> node;
    try {
      node = m_source->inspect({CommitTargetKind::File, path, {}, {}});
    } catch (const std::exception& error) {
      reject(FileStagingRejection::Kind::InspectionFailed, QString::fromUtf8(error.what()));
      continue;
    }
    if (node && !node->skippedReason.isEmpty()) {
      reject(FileStagingRejection::Kind::ReparsePoint, path);
      continue;
    }
    const bool expectedType = node && node->kind == (kind == FileStagingKind::File ? CommitTargetKind::File : CommitTargetKind::Folder);
    if (!expectedType) {
      reject(FileStagingRejection::Kind::WrongType, path);
      continue;
    }
    try {
      const QString targetDrive = QString::fromStdString(m_source->volumeForPath(path));
      if (targetDrive.isEmpty()) {
        reject(FileStagingRejection::Kind::MissingDrive, path);
        continue;
      }
      if (targetDrive.compare(m_driveLetter, Qt::CaseInsensitive) != 0) {
        reject(FileStagingRejection::Kind::WrongDrive, targetDrive);
        continue;
      }
    } catch (const std::exception& error) {
      reject(FileStagingRejection::Kind::ResolutionFailed, QString::fromUtf8(error.what()));
      continue;
    }
    if (kind == FileStagingKind::Directory && paths::isDriveRoot(path)) {
      reject(FileStagingRejection::Kind::DriveRoot);
      continue;
    }
    if (m_conflicts) {
      if (const auto conflict = m_conflicts->conflictingExclusion(path)) {
        reject(FileStagingRejection::Kind::ExclusionConflict, *conflict);
        continue;
      }
    }
    if (isCoveredByParentDirectory(candidate, path)) continue;

    const QString identity = paths::identity(path);
    const auto duplicate = std::ranges::find_if(candidate, [&](const FileStagingEntry& entry) { return paths::identity(entry.path) == identity; });
    if (duplicate != candidate.end()) {
      if (duplicate->kind == kind) continue;
      candidate.erase(duplicate);
    }

    if (kind == FileStagingKind::Directory) {
      candidate.removeIf([&](const FileStagingEntry& entry) { return paths::contains(path, entry.path); });
    }
    candidate.append({kind, path});
    changed = true;
  }
  if (changed) updateDraft(candidate);
}

void FileStagingEditor::removePaths(const QStringList& paths) {
  if (paths.isEmpty()) return;

  QList<FileStagingEntry> candidate;
  try {
    candidate = m_draft.load();
  } catch (const std::exception& error) {
    reportUpdateFailure(QString::fromUtf8(error.what()));
    return;
  }

  qsizetype removed = 0;
  for (const auto& path : paths) {
    const QString identity = paths::identity(path);
    removed += candidate.removeIf([&](const FileStagingEntry& entry) { return paths::identity(entry.path) == identity; });
  }
  if (removed > 0) {
    updateDraft(candidate);
  } else {
    m_entries = entriesForDrive(candidate, m_driveLetter, *m_source);
    sortEntries(m_entries);
    emit entriesChanged();
  }
}

void FileStagingEditor::updateDraft(const QList<FileStagingEntry>& entries) {
  try {
    QList<FileStagingEntry> ordered = entries;
    sortEntries(ordered);
    m_draft.replace(ordered);
    // Editing only replaces the shared draft. Persistent writes belong to the
    // accepted application plan, including any registry-exclusion cleanup.
    const auto storedEntries = m_draft.load();
    if (m_conflicts) m_conflicts->setStagedEntries(storedEntries);
    m_entries = entriesForDrive(storedEntries, m_driveLetter, *m_source);
    sortEntries(m_entries);
    emit entriesChanged();
  } catch (const std::exception& error) {
    try {
      const auto storedEntries = m_draft.load();
      if (m_conflicts) m_conflicts->setStagedEntries(storedEntries);
      m_entries = entriesForDrive(storedEntries, m_driveLetter, *m_source);
      sortEntries(m_entries);
      emit entriesChanged();
    } catch (...) {
      setStorageUnavailable(QString::fromUtf8(error.what()));
      return;
    }
    reportUpdateFailure(QString::fromUtf8(error.what()));
  }
}

void FileStagingEditor::setStorageUnavailable(const QString& error) {
  m_storageAvailable = false;
  m_storageError = error;
  emit storageAvailabilityChanged();
  emit storageUnavailable();
  UWF_LOG_E("staging") << "file staging registry read failed: error=" << error.toStdString();
}

void FileStagingEditor::reportUpdateFailure(const QString& error) {
  UWF_LOG_E("staging") << "file staging update failed: error=" << error.toStdString();
  emit updateFailed(error);
}

void FileStagingEditor::reject(const FileStagingRejection::Kind kind, QString detail) { emit pathRejected({kind, std::move(detail)}); }

}  // namespace uwf::app
