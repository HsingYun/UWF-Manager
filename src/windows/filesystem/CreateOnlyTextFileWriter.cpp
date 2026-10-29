// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "CreateOnlyTextFileWriter.h"

#include <windows.h>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QScopeGuard>
#include <QTemporaryFile>
#include <filesystem>

#include "app/ports/WindowsFileSystem.h"
#include "windows/system/WindowsErrorText.h"

namespace uwf::windows {
app::TextFileCreationResult createUtf8TextFile(const QString& path, const QString& text) {
  using Outcome = app::TextFileCreationResult::Outcome;
  const std::filesystem::path target(path.toStdWString());
  if (!target.is_absolute() || path.contains(QChar::Null)) return {Outcome::Failed, QStringLiteral("Target path must be absolute and valid.")};
  const auto nativeTarget = winfs::extendedPath(target).wstring();
  if (GetFileAttributesW(nativeTarget.c_str()) != INVALID_FILE_ATTRIBUTES) return {Outcome::AlreadyExists, {}};
  const DWORD inspectionError = GetLastError();
  if (inspectionError != ERROR_FILE_NOT_FOUND && inspectionError != ERROR_PATH_NOT_FOUND) return {Outcome::Failed, windowsErrorText(inspectionError)};

  QString stagedPath;
  auto removeStaged = qScopeGuard([&] {
    if (!stagedPath.isEmpty()) QFile::remove(stagedPath);
  });
  {
    QTemporaryFile staged(QDir(QFileInfo(path).absolutePath()).filePath(QStringLiteral(".uwf-create-XXXXXX.tmp")));
    if (!staged.open()) return {Outcome::Failed, staged.errorString()};
    const auto encoded = text.toUtf8();
    if (staged.write(encoded) != encoded.size() || !staged.flush()) return {Outcome::Failed, staged.errorString()};
    stagedPath = staged.fileName();
    staged.setAutoRemove(false);
    // close() retains QTemporaryFile's native handle; destruction releases it before the Windows rename.
  }
  const auto nativeStaged = winfs::extendedPath(std::filesystem::path(stagedPath.toStdWString())).wstring();
  // Same-directory publication has no copy fallback and never replaces a concurrent creator's file.
  if (!MoveFileExW(nativeStaged.c_str(), nativeTarget.c_str(), MOVEFILE_WRITE_THROUGH)) {
    const DWORD publishError = GetLastError();
    if (publishError == ERROR_FILE_EXISTS || publishError == ERROR_ALREADY_EXISTS || GetFileAttributesW(nativeTarget.c_str()) != INVALID_FILE_ATTRIBUTES)
      return {Outcome::AlreadyExists, {}};
    return {Outcome::Failed, windowsErrorText(publishError)};
  }
  removeStaged.dismiss();
  return {};
}
}  // namespace uwf::windows
