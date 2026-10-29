// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QObject>
#include <QStringList>

#include "app/api/FileStagingTypes.h"

namespace uwf::app {

struct FileStagingRejection {
  enum class Kind { NonLocalPath, InspectionFailed, ReparsePoint, WrongType, MissingDrive, WrongDrive, ResolutionFailed, DriveRoot, ExclusionConflict };
  Kind kind;
  QString detail;
};

class FileStagingEditing : public QObject {
  Q_OBJECT
 public:
  using QObject::QObject;
  [[nodiscard]] virtual const QString& driveLetter() const = 0;
  [[nodiscard]] virtual const QList<FileStagingEntry>& entries() const = 0;
  [[nodiscard]] virtual QList<FileStagingEntry> baselineEntries() const = 0;
  [[nodiscard]] virtual bool storageAvailable() const = 0;
  [[nodiscard]] virtual const QString& storageError() const = 0;
  virtual void load() = 0;
  virtual void addEntries(const QStringList& paths, FileStagingKind kind) = 0;
  virtual void removePaths(const QStringList& paths) = 0;
  virtual void restorePaths(const QStringList& paths) = 0;

 signals:
  void entriesChanged();
  void storageUnavailable();
  void storageAvailabilityChanged();
  void updateFailed(const QString& error);
  void pathRejected(uwf::app::FileStagingRejection rejection);
};

}  // namespace uwf::app
