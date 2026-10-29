// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QDateTime>
#include <QString>
#include <QVector>
#include <stdexcept>
#include <stop_token>

namespace uwf::app {
enum class FileBrowserErrorCode { MissingDirectory, AccessDenied, AlreadyExists, CreateDirectoryFailed, RenameFailed, RenameUnsupported };
class FileBrowserError final : public std::runtime_error {
 public:
  explicit FileBrowserError(FileBrowserErrorCode code) : std::runtime_error("file browser operation failed"), m_code(code) {}
  [[nodiscard]] FileBrowserErrorCode code() const { return m_code; }

 private:
  FileBrowserErrorCode m_code;
};
struct FileBrowserEntry {
  QString path;
  QString name;
  bool directory = false;
  qint64 size = 0;
  QDateTime modified{};
  bool exists = true;
};
enum class FileBrowserPlaceKind { Custom, Home, Desktop, Downloads, Documents };
struct FileBrowserPlace {
  QString name;
  QString path;
  bool volume = false;
  bool system = false;
  FileBrowserPlaceKind kind = FileBrowserPlaceKind::Custom;
};
// Browsing and the sole write (explicitly creating a directory) are injected.
// Selecting a save target never creates, truncates or opens the target file.
class FileBrowser {
 public:
  virtual ~FileBrowser() = default;
  [[nodiscard]] virtual QVector<FileBrowserEntry> list(const QString& path, std::stop_token stop) const = 0;
  [[nodiscard]] virtual FileBrowserEntry inspect(const QString& path) const = 0;
  [[nodiscard]] virtual QVector<FileBrowserPlace> places() const = 0;
  virtual void createDirectory(const QString& path) const = 0;
  virtual void renameEntry(const QString&, const QString&) const { throw FileBrowserError(FileBrowserErrorCode::RenameUnsupported); }
};
}  // namespace uwf::app
