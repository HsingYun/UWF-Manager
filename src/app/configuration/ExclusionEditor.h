// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QObject>
#include <QVector>
#include <functional>
#include <memory>

#include "app/configuration/ExclusionDraft.h"
#include "app/staging/FileStagingConflictPolicy.h"

namespace uwf::app {

class ExclusionEditor final : public QObject {
  Q_OBJECT
 public:
  enum class Kind { File, Registry };
  enum class PersistenceItem { DomainSecretKey, Tscal };
  using ImportOutcome = app::ExclusionImportOutcome;
  struct EditResult {
    ImportOutcome outcome = ImportOutcome::NoOp;
    QString path;
    QString reason;
    QString conflict;
  };
  struct Selection {
    QString path;
    std::optional<PersistenceItem> persistence;
  };

  explicit ExclusionEditor(Kind kind, app::FileStagingConflictPolicy* conflicts = nullptr, QObject* parent = nullptr);
  ExclusionEditor(Kind kind, std::shared_ptr<app::ExclusionDraft> state, app::FileStagingConflictPolicy* conflicts = nullptr, QObject* parent = nullptr);
  [[nodiscard]] Kind kind() const { return m_kind; }
  [[nodiscard]] const app::ExclusionDraft& draft() const { return m_draft; }
  [[nodiscard]] const QString& driveLetter() const { return m_driveLetter; }
  [[nodiscard]] bool readOnly() const { return m_readOnly; }
  enum class FileEntryKind { File, Directory };
  [[nodiscard]] bool commitEnabled() const { return m_commitEnabled; }
  [[nodiscard]] std::optional<FileEntryKind> fileCommitKind(const QString& relativePath) const;
  [[nodiscard]] FileEntryKind fileEntryKind(const QString& relativePath) const;
  [[nodiscard]] QString absoluteFilePath(const QString& path) const;
  [[nodiscard]] QStringList effectiveFileExclusions() const;
  [[nodiscard]] QStringList pendingAdded() const;
  [[nodiscard]] QStringList pendingRemoved() const;
  [[nodiscard]] std::optional<bool> pendingPersistDomainSecretKey() const;
  [[nodiscard]] std::optional<bool> pendingPersistTSCAL() const;

  void setDriveLetter(const QString& driveLetter);
  void setSystemDriveLetter(const QString& driveLetter);
  void setBaseline(const QStringList& current, const QStringList& next);
  void setPersistBaseline(bool domainCurrent, bool domainNext, bool tscalCurrent, bool tscalNext);
  void resetPending();
  void refreshFileStagingConstraints();
  void setReadOnly(bool readOnly);
  void setCommitEnabled(bool enabled);
  void setFileStagingDataProbe(std::function<bool()> probe);
  [[nodiscard]] bool canRemoveSelection(const QStringList& paths) const;
  bool removeSelection(const QVector<Selection>& selection);
  bool restoreSelection(const QVector<Selection>& selection);
  void enablePersistence(PersistenceItem item);
  [[nodiscard]] EditResult addEntry(const QString& raw);
  ImportOutcome importAdd(const QString& raw);
  ImportOutcome importRemove(const QString& raw);

 signals:
  void stateChanged();
  void pendingChanged();
  void readOnlyChanged();
  void removalAvailabilityChanged();

 private:
  [[nodiscard]] EditResult validateAddition(const QString& raw) const;
  [[nodiscard]] bool fileStagingRegistryRootRequired() const;
  void normalizeFileExclusions();
  void publishFileExclusions() const;

  std::shared_ptr<app::ExclusionDraft> m_state;
  app::ExclusionDraft& m_draft;
  Kind m_kind;
  QString m_driveLetter;
  QString m_systemDrive;
  app::FileStagingConflictPolicy* m_conflicts;
  std::function<bool()> m_fileStagingDataProbe;
  bool m_readOnly = false;
  bool m_commitEnabled = false;
};

}  // namespace uwf::app
