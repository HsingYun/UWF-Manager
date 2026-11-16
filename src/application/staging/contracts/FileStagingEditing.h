// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QObject>
#include <QStringList>
#include <exception>
#include <functional>
#include <stop_token>

#include "application/staging/contracts/FileStagingTypes.h"

namespace uwf::app {

// A page owns its mounted drive. One existing page also manages saved targets
// without a staging page; this does not change their actual volume bindings.
struct FileStagingScope {
  QStringList managedDrives;
  QString fallbackDrive;
};

struct FileStagingRejection {
  enum class Kind { NonLocalPath, InspectionFailed, ReparsePoint, WrongType, MissingDrive, WrongDrive, ResolutionFailed, DriveRoot, ExclusionConflict };
  Kind kind;
  QString detail;
};

// Prepare owns only background inputs. Complete and cancel run on the editing
// thread; the caller delivers exactly one of them, including on destruction.
struct FileStagingPreparation {
  std::function<void(std::stop_token)> prepare;
  std::function<void(std::exception_ptr)> complete;
  std::function<void()> cancel;
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
  [[nodiscard]] virtual bool busy() const { return false; }
  [[nodiscard]] virtual FileStagingPreparation prepareLoad() {
    return {{}, [this](std::exception_ptr) { load(); }, {}};
  }
  // A selection is one batch even when it contains both files and directories.
  [[nodiscard]] virtual FileStagingPreparation prepareAddEntries(const QList<FileStagingEntry>& entries) = 0;
  [[nodiscard]] virtual FileStagingPreparation prepareRemovePaths(const QStringList& paths) = 0;
  [[nodiscard]] virtual FileStagingPreparation prepareRestorePaths(const QStringList& paths) = 0;

 signals:
  void entriesChanged();
  void storageUnavailable();
  void storageAvailabilityChanged();
  void updateFailed(const QString& error);
  void pathRejected(uwf::app::FileStagingRejection rejection);
  void busyChanged();
};

}  // namespace uwf::app
