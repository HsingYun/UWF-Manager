// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QList>
#include <QString>
#include <cstddef>
#include <exception>
#include <functional>
#include <optional>

namespace uwf::app {

enum class FileStagingKind {
  File,
  Directory,
};

struct FileStagingEntry {
  FileStagingKind kind;
  QString path;

  bool operator==(const FileStagingEntry&) const = default;
};

struct FileStagingChanges {
  QList<FileStagingEntry> before;
  QList<FileStagingEntry> after;
  bool persistencePending = false;
  [[nodiscard]] QList<FileStagingEntry> added() const;
  [[nodiscard]] QList<FileStagingEntry> removed() const;
  [[nodiscard]] std::size_t count() const;
};

struct FileStagingWriteResult {
  std::optional<QList<FileStagingEntry>> observed;
  std::exception_ptr failure;
  bool attempted = false;
};
using FileStagingWriteOperation = std::function<FileStagingWriteResult()>;

enum class FileStagingCommitFailureKind {
  InvalidPath,
  ReparsePoint,
  PathInspectionFailed,
  FileTypeChanged,
  DirectoryTypeChanged,
  DirectoryEnumerationFailed,
  MissingDriveLetter,
  VolumeRoot,
  ProviderFailure,
  UnknownFailure,
};

struct FileStagingCommitFailure {
  QString path;
  FileStagingCommitFailureKind kind;
  QString detail;
};

struct FileStagingCommitResult {
  std::size_t discoveredFiles = 0;
  std::size_t committedFiles = 0;
  std::size_t skippedFiles = 0;
  std::size_t skippedEntries = 0;
  QList<FileStagingCommitFailure> failures;

  [[nodiscard]] bool succeeded() const { return failures.isEmpty(); }
};

}  // namespace uwf::app
