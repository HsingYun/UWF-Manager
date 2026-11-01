// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "application/staging/editing/FileStagingConflictPolicy.h"

#include <QSet>
#include <algorithm>
#include <utility>

#include "application/configuration/contracts/ConfiguredExclusionIdentity.h"
#include "application/paths/ExclusionCoverage.h"
#include "application/paths/PathPolicy.h"
#include "domain/uwf/contracts/Types.h"
#include "foundation/paths/DriveLetter.h"

namespace uwf::app {

namespace {

bool conflicts(const QString& pattern, const QString& path, const FileStagingKind kind) {
  return kind == FileStagingKind::Directory ? fileExclusionIntersectsDirectory(pattern, path)
                                            : fileExclusionCoverage(QStringList{pattern}, path).preventsCommit();
}

}  // namespace

std::optional<FileStagingExclusionConflict> firstFileStagingExclusionConflict(const QList<FileStagingEntry>& entries, const QStringList& exclusions,
                                                                              const FileStagingVolumeIdentities& volumes) {
  for (const auto& entry : entries) {
    const auto path = paths::normalize(entry.path);
    const auto object = entry.fileTarget
                            ? std::optional{paths::VolumeFilePath{compat::normalizeVolumeId(entry.fileTarget->volumeId), entry.fileTarget->relativePath}}
                            : paths::resolveVolumeFilePath(path, volumes);
    const auto match = std::ranges::find_if(exclusions, [&](const auto& pattern) {
      // A pattern's explicit root chooses its management volume. Resolve only
      // the selected object; never follow mount points inside stored patterns.
      const auto address = paths::resolveVolumeFilePath(pattern, volumes);
      if (object && address) return object->volume == address->volume && conflicts(address->relativePath, object->relativePath, entry.kind);
      // Standalone callers without mount facts retain literal display-path
      // checks; these cannot establish equality between different addresses.
      if (volumes.empty() && !address) return conflicts(pattern, path, entry.kind);
      if (object || address) return false;
      return conflicts(pattern, path, entry.kind);
    });
    if (match != exclusions.end()) return FileStagingExclusionConflict{{entry.kind, path}, *match};
  }
  return std::nullopt;
}

void FileStagingConflictPolicy::setVolumeIdentity(const QString& driveLetter, const std::string& volumeId) {
  m_volumes.clear();
  if (const auto identity = compat::parseVolumeId(volumeId); identity && !driveLetter.isEmpty()) m_volumes.emplace(driveLetter, *identity);
}

void FileStagingConflictPolicy::setFileExclusions(QStringList exclusions) {
  QStringList unique;
  QSet<QString> seen;
  for (const auto& exclusion : exclusions) {
    if (exclusion.isEmpty()) continue;
    const auto key = configuredExclusionKey(exclusion);
    if (seen.contains(key)) continue;
    seen.insert(key);
    unique.append(exclusion);
  }
  setFileExclusionSource([exclusions = std::move(unique)] { return exclusions; });
}

void FileStagingConflictPolicy::setFileExclusionSource(std::function<QStringList()> source) { m_fileExclusionSource = std::move(source); }

void FileStagingConflictPolicy::setStagedEntries(const QList<FileStagingEntry>& entries) {
  if (std::ranges::equal(m_stagedEntries, entries, [](const auto& left, const auto& right) { return left == right && left.fileTarget == right.fileTarget; }))
    return;
  m_stagedEntries = entries;
  emit stagedEntriesChanged();
}

std::optional<QString> FileStagingConflictPolicy::conflictingExclusion(const QString& stagedPath, const FileStagingKind kind) const {
  return conflictingExclusion(FileStagingEntry{kind, stagedPath});
}
std::optional<QString> FileStagingConflictPolicy::conflictingExclusion(const FileStagingEntry& entry) const {
  const auto conflict = firstFileStagingExclusionConflict({entry}, fileExclusions(), m_volumes);
  return conflict ? std::optional{conflict->exclusion} : std::nullopt;
}

std::optional<QString> FileStagingConflictPolicy::conflictingStagedPath(const QString& exclusionPath) const {
  const auto conflict = firstFileStagingExclusionConflict(m_stagedEntries, {exclusionPath}, m_volumes);
  return conflict ? std::optional{conflict->entry.path} : std::nullopt;
}

}  // namespace uwf::app

#include "moc_FileStagingConflictPolicy.cpp"
