// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "FileStagingConflictPolicy.h"

#include <algorithm>
#include <utility>

#include "app/paths/PathPolicy.h"

namespace uwf::app {

namespace {

QString normalizedPath(const QString& path) { return paths::normalize(path.trimmed()); }

void normalizeUnique(QStringList& paths) {
  for (auto& path : paths) path = normalizedPath(path);
  paths.removeIf([](const QString& path) { return path.isEmpty(); });
  std::ranges::sort(paths, [](const QString& left, const QString& right) { return QString::compare(left, right, Qt::CaseInsensitive) < 0; });
  for (qsizetype index = paths.size() - 1; index > 0; --index) {
    if (QString::compare(paths.at(index), paths.at(index - 1), Qt::CaseInsensitive) == 0) paths.removeAt(index);
  }
}

std::optional<QString> firstConflict(const QString& candidate, const QStringList& existing) {
  const auto normalized = normalizedPath(candidate);
  const auto match = std::ranges::find_if(existing, [&](const QString& path) { return paths::overlap(normalized, path); });
  return match == existing.end() ? std::nullopt : std::optional{*match};
}

}  // namespace

void FileStagingConflictPolicy::setFileExclusions(QStringList exclusions) {
  normalizeUnique(exclusions);
  m_fileExclusions = std::move(exclusions);
}

void FileStagingConflictPolicy::setStagedEntries(const QList<FileStagingEntry>& entries) {
  QStringList paths;
  paths.reserve(entries.size());
  for (const auto& entry : entries) paths.append(entry.path);
  normalizeUnique(paths);
  m_stagedPaths = std::move(paths);
}

std::optional<QString> FileStagingConflictPolicy::conflictingExclusion(const QString& stagedPath) const { return firstConflict(stagedPath, m_fileExclusions); }

std::optional<QString> FileStagingConflictPolicy::conflictingStagedPath(const QString& exclusionPath) const {
  return firstConflict(exclusionPath, m_stagedPaths);
}

}  // namespace uwf::app
