// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "infrastructure/windows/filesystem/browsing/LocalFileBrowser.h"

#include <windows.h>

#include <QDir>
#include <QStandardPaths>
#include <QStorageInfo>
#include <array>
#include <filesystem>
#include <limits>
#include <stdexcept>
#include <utility>

#include "application/paths/PathPolicy.h"
#include "infrastructure/windows/errors/WindowsSystemError.h"
#include "infrastructure/windows/filesystem/WindowsFileSystem.h"

namespace uwf::windows {
namespace {
app::FileBrowserEntry entry(const std::filesystem::path& path, const std::optional<winfs::EntryMetadata>& observed) {
  const auto display = app::paths::fileSystemPath(QString::fromStdWString(path.native()));
  const auto name = QString::fromStdWString(path.filename().native());
  if (!observed) return {display, name, false, 0, {}, false};
  if (observed->size > static_cast<std::uint64_t>(std::numeric_limits<qint64>::max())) throw std::overflow_error("file size exceeds the browser value range");
  return {display, name, observed->directory, static_cast<qint64>(observed->size), QDateTime::fromMSecsSinceEpoch(observed->modifiedSinceUnixEpoch.count()),
          true};
}
std::filesystem::path absolutePath(const QString& path) {
  const std::filesystem::path native(path.toStdWString());
  return native.empty() || native.is_absolute() ? native : std::filesystem::current_path() / native;
}
class LocalFileBrowser final : public app::FileBrowser {
 public:
  QVector<app::FileBrowserEntry> list(const QString& path, const std::stop_token stop) const override {
    QVector<app::FileBrowserEntry> result;
    try {
      winfs::enumerate(absolutePath(path), stop, [&](const auto& child, const auto& observed) { result.append(entry(child, observed)); });
    } catch (const WindowsSystemError& error) {
      if (error.code().value() == ERROR_FILE_NOT_FOUND || error.code().value() == ERROR_PATH_NOT_FOUND || error.code().value() == ERROR_DIRECTORY)
        throw app::FileBrowserError(app::FileBrowserErrorCode::MissingDirectory);
      if (error.code().value() == ERROR_ACCESS_DENIED) throw app::FileBrowserError(app::FileBrowserErrorCode::AccessDenied);
      throw;
    }
    return stop.stop_requested() ? QVector<app::FileBrowserEntry>{} : result;
  }
  app::FileBrowserEntry inspect(const QString& path) const override {
    const auto native = absolutePath(path);
    return entry(native, winfs::metadata(native));
  }
  QVector<app::FileBrowserPlace> places() const override {
    QVector<app::FileBrowserPlace> result;
    result.append({QStringLiteral("Home"), QDir::homePath(), false, false, app::FileBrowserPlaceKind::Home});
    const QString localData = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    if (!localData.isEmpty()) result.append({QStringLiteral("AppData"), app::paths::parentDirectory(localData)});
    const std::array locations{std::pair{QStandardPaths::DesktopLocation, app::FileBrowserPlaceKind::Desktop},
                               std::pair{QStandardPaths::DownloadLocation, app::FileBrowserPlaceKind::Downloads},
                               std::pair{QStandardPaths::DocumentsLocation, app::FileBrowserPlaceKind::Documents}};
    for (const auto& [location, kind] : locations) {
      const auto path = QStandardPaths::writableLocation(location);
      if (!path.isEmpty()) result.append({QStandardPaths::displayName(location), path, false, false, kind});
    }
    const auto systemDrive = qEnvironmentVariable("SystemDrive");
    for (auto storages = QStorageInfo::mountedVolumes(); const auto& storage : std::as_const(storages)) {
      if (!storage.isValid() || !storage.isReady()) continue;
      const QString root = QDir::toNativeSeparators(storage.rootPath());
      const QString name = storage.name().isEmpty() ? root : storage.name() + QStringLiteral(" (") + root + u')';
      result.append({name, storage.rootPath(), true, root.startsWith(systemDrive, Qt::CaseInsensitive)});
    }
    return result;
  }
  void createDirectory(const QString& path) const override {
    try {
      winfs::createDirectory(std::filesystem::path(path.toStdWString()));
    } catch (const WindowsSystemError& error) {
      throw app::FileBrowserError(error.code().value() == ERROR_ALREADY_EXISTS || error.code().value() == ERROR_FILE_EXISTS
                                      ? app::FileBrowserErrorCode::AlreadyExists
                                      : app::FileBrowserErrorCode::CreateDirectoryFailed);
    }
  }
  void renameEntry(const QString& path, const QString& destination) const override {
    try {
      winfs::rename(std::filesystem::path(path.toStdWString()), std::filesystem::path(destination.toStdWString()));
    } catch (const WindowsSystemError& error) {
      throw app::FileBrowserError(error.code().value() == ERROR_ALREADY_EXISTS || error.code().value() == ERROR_FILE_EXISTS
                                      ? app::FileBrowserErrorCode::AlreadyExists
                                      : app::FileBrowserErrorCode::RenameFailed);
    }
  }
};
}  // namespace
std::shared_ptr<const app::FileBrowser> systemFileBrowser() {
  static const auto browser = std::make_shared<LocalFileBrowser>();
  return browser;
}
}  // namespace uwf::windows
