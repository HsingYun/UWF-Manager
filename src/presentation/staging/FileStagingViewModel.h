// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <QAbstractListModel>
#include <memory>

#include "app/api/FileStagingEditing.h"
#include "presentation/common/ItemRoles.h"

namespace uwf::presentation {

class FileStagingViewModel final : public QAbstractListModel {
  Q_OBJECT
 public:
  [[nodiscard]] int rowCount(const QModelIndex& parent = {}) const override;
  [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  explicit FileStagingViewModel(std::shared_ptr<app::FileStagingEditing> editing);
  [[nodiscard]] const QString& driveLetter() const { return m_editing->driveLetter(); }
  [[nodiscard]] const QList<app::FileStagingEntry>& entries() const { return m_entries; }
  [[nodiscard]] bool storageAvailable() const { return m_editing->storageAvailable(); }
  [[nodiscard]] const QString& storageError() const { return m_editing->storageError(); }
  [[nodiscard]] bool readOnly() const { return m_readOnly; }
  [[nodiscard]] bool enhancedModeAvailable() const { return m_enhancedModeAvailable; }
  [[nodiscard]] bool writable() const { return storageAvailable() && !m_readOnly; }
  void load();
  void retranslate();
  [[nodiscard]] QString summaryText() const;
  void setReadOnly(bool value);
  void setEnhancedModeAvailable(bool value);
  void addEntries(const QStringList& paths, app::FileStagingKind kind);
  void removePaths(const QStringList& paths);
  void restorePaths(const QStringList& paths);

 signals:
  void entriesChanged();
  void storageUnavailable();
  void storageAvailabilityChanged();
  void modificationAvailabilityChanged();
  void enhancedModeAvailabilityChanged();
  void warningRaised(const QString& title, const QString& body);

 private:
  void reportRejection(const app::FileStagingRejection& rejection);
  void rebuildEntries();

  std::shared_ptr<app::FileStagingEditing> m_editing;
  QList<app::FileStagingEntry> m_entries;
  struct Row {
    app::FileStagingEntry entry;
    ItemStatus status;
  };
  QList<Row> m_rows;
  bool m_readOnly = false;
  bool m_enhancedModeAvailable = false;
};

}  // namespace uwf::presentation
