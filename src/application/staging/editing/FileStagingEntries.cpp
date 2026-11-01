// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "application/staging/editing/FileStagingEntries.h"

#include <algorithm>
#include <map>
#include <set>
#include <utility>

#include "application/commit/contracts/CommitTraversalSource.h"
#include "application/paths/PathPolicy.h"
#include "application/staging/editing/FileStagingPath.h"

namespace uwf::app {
QList<FileStagingEntry> normalizeFileStagingEntries(const QList<FileStagingEntry>& entries, const CommitTraversalSource* source) {
  std::map<ObjectIdentity, FileStagingEntry> objects;
  for (const auto& entry : entries) {
    const bool validSelection = (entry.kind == FileStagingKind::File || entry.kind == FileStagingKind::Directory) && isAbsoluteLocalFileStagingPath(entry.path);
    FileStagingEntry normalized{entry.kind, validSelection ? paths::normalize(entry.path) : entry.path, validSelection ? entry.fileTarget : std::nullopt};
    // A GUID address supplies its own volume identity even while that volume is
    // offline. A drive-addressed path needs a current mount observation.
    const auto explicitAddress = validSelection ? paths::resolveVolumeFilePath(normalized.path) : std::nullopt;
    const auto explicitTarget = explicitAddress ? std::optional{FileCommitTarget{explicitAddress->volume, explicitAddress->relativePath}} : std::nullopt;
    if (!normalized.fileTarget && explicitTarget) normalized.fileTarget = explicitTarget;
    if (source) {
      normalized.fileTarget = explicitTarget;
      if (validSelection) {
        try {
          normalized.fileTarget = bindFileCommitTarget(*source, {CommitTargetKind::File, normalized.path, {}, {}}).fileTarget;
        } catch (const std::exception&) {
          // Offline and malformed stored names remain visible and removable.
        }
      }
    }
    const auto [found, inserted] = objects.try_emplace(ObjectIdentity(fileStagingObjectPath(normalized)), normalized);
    if (!inserted &&
        (found->second.kind != normalized.kind || (!found->second.fileTarget && normalized.fileTarget) ||
         (found->second.fileTarget.has_value() == normalized.fileTarget.has_value() &&
          normalized.path.compare(found->second.path, Qt::CaseSensitive) < 0)))
      found->second = std::move(normalized);
  }
  std::set<ObjectIdentity> directories;
  for (const auto& [identity, entry] : objects)
    // Keep invalid stored selections visible for rejection/removal. In
    // particular, a forbidden volume-root entry must not swallow valid files.
    if (entry.kind == FileStagingKind::Directory && isAbsoluteLocalFileStagingPath(entry.path) && !entry.path.contains(QChar::Null) &&
        !paths::isDriveRoot(entry.path) && (!entry.fileTarget || entry.fileTarget->relativePath != QStringLiteral("\\")))
      directories.insert(identity);

  QList<FileStagingEntry> result;
  result.reserve(static_cast<qsizetype>(objects.size()));
  for (auto& [identity, entry] : objects) {
    (void)identity;
    bool covered = false;
    const auto objectPath = fileStagingObjectPath(entry);
    for (qsizetype separator = objectPath.indexOf(u'\\'); separator >= 0; separator = objectPath.indexOf(u'\\', separator + 1)) {
      // Preserve the separator in a drive root; it is part of its identity.
      const auto length = separator == 2 && objectPath[1] == u':' ? separator + 1 : separator;
      if (length < objectPath.size() && directories.contains(ObjectIdentity(objectPath.left(length)))) {
        covered = true;
        break;
      }
    }
    if (!covered) result.append(std::move(entry));
  }
  std::ranges::sort(result, [](const auto& left, const auto& right) { return ObjectIdentity(left.path) < ObjectIdentity(right.path); });
  return result;
}
}  // namespace uwf::app
