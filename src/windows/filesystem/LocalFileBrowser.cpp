// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "LocalFileBrowser.h"

#include <QDir>
#include <QDirListing>
#include <QFileInfo>
#include <QStandardPaths>
#include <QStorageInfo>
#include <array>
#include <stdexcept>
#include <utility>

namespace uwf::windows {
namespace {
app::FileBrowserEntry entry(const QFileInfo& info) {
  return {info.absoluteFilePath(), info.fileName(), info.isDir(), info.size(), info.lastModified(), info.exists()};
}
class LocalFileBrowser final : public app::FileBrowser {
 public:
  QVector<app::FileBrowserEntry> list(const QString& path, const std::stop_token stop) const override {
    const QFileInfo directory(path);
    if (!directory.exists() || !directory.isDir()) throw app::FileBrowserError(app::FileBrowserErrorCode::MissingDirectory);
    if (!directory.isReadable()) throw app::FileBrowserError(app::FileBrowserErrorCode::AccessDenied);
    QVector<app::FileBrowserEntry> result;
    for (const auto& item : QDirListing(path, QDirListing::IteratorFlag::IncludeHidden)) {
      if (stop.stop_requested()) return {};
      result.append(entry(item.fileInfo()));
    }
    return result;
  }
  app::FileBrowserEntry inspect(const QString& path) const override { return entry(QFileInfo(path)); }
  QVector<app::FileBrowserPlace> places() const override {
    QVector<app::FileBrowserPlace> result;
    result.append({QStringLiteral("Home"), QDir::homePath(), false, false, app::FileBrowserPlaceKind::Home});
    const QString localData = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    if (!localData.isEmpty()) result.append({QStringLiteral("AppData"), QDir::cleanPath(QDir(localData).filePath(QStringLiteral("..")))});
    const std::array locations{std::pair{QStandardPaths::DesktopLocation, app::FileBrowserPlaceKind::Desktop},
                               std::pair{QStandardPaths::DownloadLocation, app::FileBrowserPlaceKind::Downloads},
                               std::pair{QStandardPaths::DocumentsLocation, app::FileBrowserPlaceKind::Documents}};
    for (const auto& [location, kind] : locations) {
      const auto path = QStandardPaths::writableLocation(location);
      if (!path.isEmpty()) result.append({QStandardPaths::displayName(location), path, false, false, kind});
    }
    const auto systemDrive = qEnvironmentVariable("SystemDrive");
    const auto storages = QStorageInfo::mountedVolumes();
    for (const auto& storage : storages) {
      if (!storage.isValid() || !storage.isReady()) continue;
      const QString root = QDir::toNativeSeparators(storage.rootPath());
      const QString name = storage.name().isEmpty() ? root : storage.name() + QStringLiteral(" (") + root + u')';
      result.append({name, storage.rootPath(), true, root.startsWith(systemDrive, Qt::CaseInsensitive)});
    }
    return result;
  }
  void createDirectory(const QString& path) const override {
    const QFileInfo info(path);
    if (info.exists()) throw app::FileBrowserError(app::FileBrowserErrorCode::AlreadyExists);
    if (!QDir(info.absolutePath()).mkdir(info.fileName())) throw app::FileBrowserError(app::FileBrowserErrorCode::CreateDirectoryFailed);
  }
  void renameEntry(const QString& path, const QString& destination) const override {
    if (!QDir().rename(path, destination)) {
      throw app::FileBrowserError(QFileInfo::exists(destination) ? app::FileBrowserErrorCode::AlreadyExists : app::FileBrowserErrorCode::RenameFailed);
    }
  }
};
}  // namespace
std::shared_ptr<const app::FileBrowser> systemFileBrowser() {
  static const auto browser = std::make_shared<LocalFileBrowser>();
  return browser;
}
}  // namespace uwf::windows
