// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "application/staging/editing/FileStagingPath.h"

#include "application/paths/PathPolicy.h"

namespace uwf::app {

bool isAbsoluteLocalFileStagingPath(const QString& path) {
  return paths::parseLocalFilePath(path).has_value();
}
FileStagingEntryKey fileStagingEntryKey(const FileStagingEntry& entry) {
  return {entry.kind, ObjectIdentity(isAbsoluteLocalFileStagingPath(entry.path) ? paths::normalize(entry.path) : entry.path)};
}
QString fileStagingObjectPath(const FileStagingEntry& entry) {
  return entry.fileTarget ? paths::volumeFilePath({entry.fileTarget->volumeId, entry.fileTarget->relativePath})
                          : (isAbsoluteLocalFileStagingPath(entry.path) ? paths::normalize(entry.path) : entry.path);
}

}  // namespace uwf::app
