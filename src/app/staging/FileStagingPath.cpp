// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "FileStagingPath.h"

#include <filesystem>

#include "shared/DriveLetter.h"
#include "app/paths/PathPolicy.h"

namespace uwf::app {

bool isAbsoluteLocalFileStagingPath(const QString& path) {
  if (path.isEmpty() || !std::filesystem::path(path.toStdWString()).is_absolute()) return false;
  if (path.startsWith(QStringLiteral(R"(\\.\)")) || path.startsWith(QStringLiteral(R"(\\?\)"))) return false;
  return !drive::split(path.toStdString()).letter.empty();
}
std::pair<FileStagingKind, QString> fileStagingEntryKey(const FileStagingEntry& entry) {
  return {entry.kind, paths::identity(entry.path)};
}

}  // namespace uwf::app
