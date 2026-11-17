// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QVector>
#include <functional>
#include <memory>
#include <optional>

#include "application/configuration/contracts/ExclusionImportOutcome.h"
#include "application/configuration/editing/EditorUpdate.h"
#include "application/configuration/exclusions/ExclusionDraft.h"
#include "application/staging/editing/FileStagingConflictPolicy.h"

namespace uwf::app {
class FileBrowser;

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

  explicit ExclusionEditor(Kind kind, app::FileStagingConflictPolicy* conflicts = nullptr, std::shared_ptr<const FileBrowser> files = {},
                           QObject* parent = nullptr);
  ExclusionEditor(Kind kind, std::shared_ptr<app::ExclusionDraft> state, app::FileStagingConflictPolicy* conflicts = nullptr,
                  std::shared_ptr<const FileBrowser> files = {}, QObject* parent = nullptr);
  // Snapshot publication belongs to the editor, including its admission changes.
  class Update : public EditorUpdate<ExclusionEditor> {
   public:
    explicit Update(ExclusionEditor& editor);
    Update(Update&& other) noexcept = default;
    Update(const Update&) = delete;
    Update& operator=(const Update&) = delete;
    [[nodiscard]] bool publishSnapshot(const std::function<bool()>& continuePublishing = {});
    [[nodiscard]] bool publishAvailability(const std::function<bool()>& continuePublishing = {});

   private:
    bool m_readOnly;
    bool m_canAdd;
  };
  [[nodiscard]] Update beginUpdate() { return Update(*this); }
  [[nodiscard]] Kind kind() const { return m_kind; }
  [[nodiscard]] const app::ExclusionDraft& draft() const { return m_draft; }
  [[nodiscard]] const QString& driveLetter() const { return m_driveLetter; }
  [[nodiscard]] bool readOnly() const { return m_readOnly; }
  [[nodiscard]] bool canAdd() const { return !m_readOnly && (m_kind != Kind::File || m_fileAddTargetAvailable); }
  [[nodiscard]] QString additionDisabledReason() const;
  enum class FileEntryKind { File, Directory };
  using FileEntryKindOperation = std::function<std::optional<FileEntryKind>(const QString&)>;
  // Owned metadata reads run off-thread. Missing/failed observations remain
  // unknown; spelling hints are only for presentation. Execution revalidates targets.
  [[nodiscard]] FileEntryKindOperation fileEntryKindOperation() const;
  [[nodiscard]] static FileEntryKind fileEntryKindHint(const QString& path);
  class BatchUpdate {
   public:
    explicit BatchUpdate(ExclusionEditor& editor);
    BatchUpdate(BatchUpdate&& other) noexcept;
    BatchUpdate(const BatchUpdate&) = delete;
    BatchUpdate& operator=(const BatchUpdate&) = delete;
    ~BatchUpdate();

   private:
    QPointer<ExclusionEditor> m_editor;
  };
  [[nodiscard]] BatchUpdate beginBatch() { return BatchUpdate(*this); }
  [[nodiscard]] bool commitEnabled() const { return m_commitEnabled; }
  [[nodiscard]] bool canCommitFile(const QString& relativePath) const;
  [[nodiscard]] QString absoluteFilePath(const QString& path) const;
  [[nodiscard]] QStringList effectiveFileExclusions() const;
  [[nodiscard]] QStringList pendingAdded() const;
  [[nodiscard]] QStringList pendingRemoved() const;
  [[nodiscard]] std::optional<bool> pendingPersistDomainSecretKey() const;
  [[nodiscard]] std::optional<bool> pendingPersistTSCAL() const;

  void setDriveLetter(const QString& driveLetter);
  void setFileVolumeIdentity(const std::string& volumeId);
  void setSystemDriveLetter(const QString& driveLetter);
  void setBaseline(const QStringList& current, const QStringList& next);
  // Provider values are per-volume literal patterns, even when their text
  // resembles an absolute address. Add exactly one explicit address shell.
  void setProviderBaseline(const QStringList& current, const QStringList& next);
  void setPersistBaseline(bool domainCurrent, bool domainNext, bool tscalCurrent, bool tscalNext);
  void resetPending();
  void refreshFileStagingConstraints();
  void setReadOnly(bool readOnly);
  // A volume owner supplies its final draft registration eligibility. Standalone
  // pattern editors have no volume context; executable plans validate it too.
  void setFileAddTargetAvailable(bool available);
  void setCommitEnabled(bool enabled);
  void setFileStagingDataProbe(std::function<bool()> probe);
  [[nodiscard]] bool canRemoveSelection(const QStringList& paths) const;
  bool removeSelection(const QVector<Selection>& selection);
  [[nodiscard]] bool canRestoreSelection(const QVector<Selection>& selection) const;
  [[nodiscard]] EditResult restorationResult(const QVector<Selection>& selection) const;
  bool restoreSelection(const QVector<Selection>& selection);
  [[nodiscard]] bool canEnablePersistence(PersistenceItem item) const;
  void enablePersistence(PersistenceItem item);
  // Picker selections identify existing objects. File separators may use Qt's
  // representation; preserve object-name characters and the registry hive name.
  // Observed/imported configuration patterns do not use this conversion.
  [[nodiscard]] EditResult addSelectedPath(const QString& selectedPath);
  ImportOutcome importAdd(const QString& raw);
  ImportOutcome importRemove(const QString& raw);

 signals:
  void baselineChanged();
  void stateChanged();
  void pendingChanged();
  void editingAvailabilityChanged();
  void removalAvailabilityChanged();

 private:
  [[nodiscard]] EditResult validateAddition(const QString& raw, bool restoringConfigured = false) const;
  struct ValidatedRestoration {
    QVector<Selection> selection;
    EditResult result;
  };
  [[nodiscard]] ValidatedRestoration validateRestoration(const QVector<Selection>& selection) const;
  [[nodiscard]] bool fileStagingRegistryRootRequired() const;
  void publishChanges();
  void finishBatch();

  std::shared_ptr<app::ExclusionDraft> m_state;
  app::ExclusionDraft& m_draft;
  Kind m_kind;
  QString m_driveLetter;
  std::string m_fileVolumeId;
  QString m_systemDrive;
  QPointer<app::FileStagingConflictPolicy> m_conflicts;
  std::shared_ptr<const FileBrowser> m_files;
  std::function<bool()> m_fileStagingDataProbe;
  bool m_readOnly = false;
  bool m_fileAddTargetAvailable = true;
  bool m_commitEnabled = false;
  int m_batchDepth = 0;
  bool m_batchChanged = false;
};

}  // namespace uwf::app
