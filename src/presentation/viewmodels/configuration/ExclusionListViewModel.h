// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QAbstractListModel>
#include <QHash>
#include <QSet>
#include <QTimer>
#include <cstdint>
#include <memory>

#include "application/configuration/exclusions/ExclusionEditor.h"
#include "presentation/models/ItemRoles.h"
#include "presentation/viewmodels/common/AsyncTask.h"

namespace uwf::presentation {
class ExclusionListViewModel final : public QAbstractListModel {
  Q_OBJECT
 public:
  using Kind = app::ExclusionEditor::Kind;
  using PersistenceItem = app::ExclusionEditor::PersistenceItem;
  using ImportOutcome = app::ExclusionEditor::ImportOutcome;
  using EditResult = app::ExclusionEditor::EditResult;
  using Selection = app::ExclusionEditor::Selection;
  explicit ExclusionListViewModel(Kind kind, app::FileStagingConflictPolicy* conflicts = nullptr, QObject* parent = nullptr);
  ExclusionListViewModel(Kind kind, std::shared_ptr<app::ExclusionDraft> state, app::FileStagingConflictPolicy* conflicts = nullptr, QObject* parent = nullptr);
  explicit ExclusionListViewModel(std::shared_ptr<app::ExclusionEditor> editor, QObject* parent = nullptr);
  int rowCount(const QModelIndex& parent = {}) const override;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  void retranslate();
  [[nodiscard]] const std::shared_ptr<app::ExclusionEditor>& editor() const { return m_editor; }
  [[nodiscard]] Kind kind() const { return m_editor->kind(); }
  [[nodiscard]] const app::ExclusionDraft& draft() const { return m_editor->draft(); }
  [[nodiscard]] const QString& driveLetter() const { return m_editor->driveLetter(); }
  [[nodiscard]] bool readOnly() const { return m_editor->readOnly(); }
  [[nodiscard]] bool canAdd() const { return m_editor->canAdd(); }
  [[nodiscard]] bool canBrowseFiles() const { return kind() == Kind::File && !driveLetter().isEmpty(); }
  [[nodiscard]] QString additionHint() const;
  [[nodiscard]] QString additionDisabledReason() const { return m_editor->additionDisabledReason(); }
  [[nodiscard]] bool commitEnabled() const { return m_editor->commitEnabled(); }
  [[nodiscard]] bool currentSessionExclusionsEffective() const { return m_currentSessionExclusionsEffective; }
  [[nodiscard]] const QString& commitRestriction() const { return m_commitRestriction; }
  void setCurrentSessionExclusionsEffective(bool effective);
  void setCommitAvailability(bool enabled, QString reason = {});
  struct FileCommitState {
    app::ExclusionEditor::FileEntryKind kind = app::ExclusionEditor::FileEntryKind::File;
    bool visible = false;
    bool enabled = false;
  };
  [[nodiscard]] FileCommitState fileCommitState(const QString& relativePath) const;
  [[nodiscard]] bool canEnablePersistence(PersistenceItem item) const { return m_editor->canEnablePersistence(item); }
  [[nodiscard]] QStringList pendingAdded() const { return m_editor->pendingAdded(); }
  [[nodiscard]] QStringList pendingRemoved() const { return m_editor->pendingRemoved(); }
  [[nodiscard]] std::optional<bool> pendingPersistDomainSecretKey() const { return m_editor->pendingPersistDomainSecretKey(); }
  [[nodiscard]] std::optional<bool> pendingPersistTSCAL() const { return m_editor->pendingPersistTSCAL(); }
  [[nodiscard]] QString absoluteFilePath(const QString& path) const { return m_editor->absoluteFilePath(path); }
  void setDriveLetter(const QString& letter) {
    const auto editor = m_editor;
    editor->setDriveLetter(letter);
  }
  void setBaseline(const QStringList& current, const QStringList& next) {
    const auto editor = m_editor;
    editor->setBaseline(current, next);
  }
  void setPersistBaseline(bool dc, bool dn, bool tc, bool tn) {
    const auto editor = m_editor;
    editor->setPersistBaseline(dc, dn, tc, tn);
  }
  void resetPending() {
    const auto editor = m_editor;
    editor->resetPending();
  }
  void setReadOnly(bool value) {
    const auto editor = m_editor;
    editor->setReadOnly(value);
  }
  void setFileStagingDataProbe(std::function<bool()> probe) {
    const auto editor = m_editor;
    editor->setFileStagingDataProbe(std::move(probe));
  }
  [[nodiscard]] bool canRemoveSelection(const QStringList& paths) const { return m_editor->canRemoveSelection(paths); }
  bool removeSelection(const QVector<Selection>& selection) {
    const auto editor = m_editor;
    return editor->removeSelection(selection);
  }
  [[nodiscard]] bool canRestoreSelection(const QVector<Selection>& selection) const { return m_editor->canRestoreSelection(selection); }
  [[nodiscard]] QString restorationRestriction(const QVector<Selection>& selection) const;
  bool restoreSelection(const QVector<Selection>& selection) {
    const auto editor = m_editor;
    return editor->restoreSelection(selection);
  }
  void enablePersistence(PersistenceItem item) {
    const auto editor = m_editor;
    editor->enablePersistence(item);
  }
  [[nodiscard]] EditResult addSelectedPath(const QString& selectedPath) {
    const auto editor = m_editor;
    return editor->addSelectedPath(selectedPath);
  }
  [[nodiscard]] app::ExclusionEditor::BatchUpdate beginBatch() { return m_editor->beginBatch(); }
  ImportOutcome importAdd(const QString& raw) {
    const auto editor = m_editor;
    return editor->importAdd(raw);
  }
  ImportOutcome importRemove(const QString& raw) {
    const auto editor = m_editor;
    return editor->importRemove(raw);
  }
 signals:
  void stateChanged();
  void editingAvailabilityChanged();
  void removalAvailabilityChanged();
  void commitAvailabilityChanged();

 private:
  void rebuildRows();
  void invalidateFileMetadata();
  void queueFileMetadata();
  void readFileMetadata();
  struct Row {
    QString path;
    QString tooltip;
    QString icon;
    ItemStatus status = ItemStatus::Normal;
    ItemBadge badge = ItemBadge::None;
    std::optional<PersistenceItem> persistence;
    bool current = false;
    bool next = false;
    bool userAdded = false;
    bool userRemoved = false;
  };
  void translateRow(Row& row) const;
  std::shared_ptr<app::ExclusionEditor> m_editor;
  bool m_currentSessionExclusionsEffective = true;
  QString m_commitRestriction;
  QList<Row> m_rows;
  bool m_rebuildingRows = false;
  bool m_rebuildPending = false;
  using FileKinds = QHash<QString, std::optional<app::ExclusionEditor::FileEntryKind>>;
  FileKinds m_fileKinds;
  QSet<QString> m_pendingMetadata;
  QSet<QString> m_inflightMetadata;
  QTimer m_metadataTimer{this};
  AsyncTask m_metadata{this};
  std::uint64_t m_metadataGeneration = 0;
};
}  // namespace uwf::presentation
