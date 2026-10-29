// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QAbstractListModel>
#include <memory>

#include "app/configuration/ExclusionEditor.h"
#include "presentation/common/ItemRoles.h"

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
  [[nodiscard]] bool commitEnabled() const { return m_editor->commitEnabled(); }
  [[nodiscard]] std::optional<app::ExclusionEditor::FileEntryKind> fileCommitKind(const QString& relativePath) const {
    return m_editor->fileCommitKind(relativePath);
  }
  [[nodiscard]] QStringList pendingAdded() const { return m_editor->pendingAdded(); }
  [[nodiscard]] QStringList pendingRemoved() const { return m_editor->pendingRemoved(); }
  [[nodiscard]] std::optional<bool> pendingPersistDomainSecretKey() const { return m_editor->pendingPersistDomainSecretKey(); }
  [[nodiscard]] std::optional<bool> pendingPersistTSCAL() const { return m_editor->pendingPersistTSCAL(); }
  [[nodiscard]] QString absoluteFilePath(const QString& path) const { return m_editor->absoluteFilePath(path); }
  void setDriveLetter(const QString& letter) { m_editor->setDriveLetter(letter); }
  void setBaseline(const QStringList& current, const QStringList& next) { m_editor->setBaseline(current, next); }
  void setPersistBaseline(bool dc, bool dn, bool tc, bool tn) { m_editor->setPersistBaseline(dc, dn, tc, tn); }
  void resetPending() { m_editor->resetPending(); }
  void setReadOnly(bool value) { m_editor->setReadOnly(value); }
  void setCommitEnabled(bool value) { m_editor->setCommitEnabled(value); }
  void setFileStagingDataProbe(std::function<bool()> probe) { m_editor->setFileStagingDataProbe(std::move(probe)); }
  [[nodiscard]] bool canRemoveSelection(const QStringList& paths) const { return m_editor->canRemoveSelection(paths); }
  bool removeSelection(const QVector<Selection>& selection) { return m_editor->removeSelection(selection); }
  bool restoreSelection(const QVector<Selection>& selection) { return m_editor->restoreSelection(selection); }
  void enablePersistence(PersistenceItem item) { m_editor->enablePersistence(item); }
  [[nodiscard]] EditResult addEntry(const QString& raw) { return m_editor->addEntry(raw); }
  ImportOutcome importAdd(const QString& raw) { return m_editor->importAdd(raw); }
  ImportOutcome importRemove(const QString& raw) { return m_editor->importRemove(raw); }
 signals:
  void stateChanged();
  void pendingChanged();
  void readOnlyChanged();
  void removalAvailabilityChanged();

 private:
  void rebuildRows();
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
  QList<Row> m_rows;
};
}  // namespace uwf::presentation
