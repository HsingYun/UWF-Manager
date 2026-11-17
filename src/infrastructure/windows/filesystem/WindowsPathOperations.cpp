// SPDX-License-Identifier: GPL-3.0-or-later
#include "application/paths/contracts/WindowsPathOperations.h"

#include <QDir>
#include <QFileInfo>
#include <filesystem>

namespace uwf::app::paths::platform {
// Keep the production Windows primitives identical to their original callers.
QString nativeSeparators(const QString& path) { return QDir::toNativeSeparators(path); }
QString portableSeparators(const QString& path) { return QDir::fromNativeSeparators(path); }
QString clean(const QString& path) { return QDir::cleanPath(path); }
bool fullyQualified(const QString& path) { return std::filesystem::path(path.toStdWString()).is_absolute(); }
bool absolute(const QString& path) { return QDir::isAbsolutePath(path); }
QString fileName(const QString& path) { return QFileInfo(path).fileName(); }
QString parent(const QString& path) { return QFileInfo(path).absolutePath(); }
QString join(const QString& directory, const QString& path) { return QDir(directory).filePath(path); }
QString resolve(const QString& directory, const QString& path) { return QDir(directory).absoluteFilePath(path); }
}  // namespace uwf::app::paths::platform
