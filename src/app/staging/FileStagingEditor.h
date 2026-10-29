// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <memory>

#include "FileStagingDraft.h"
#include "app/api/CommitPreparation.h"
#include "app/api/FileStagingEditing.h"
#include "app/ports/FileStagingStore.h"

namespace uwf::app {
class FileStagingConflictPolicy;

class FileStagingEditor final : public FileStagingEditing {
 public:
  FileStagingEditor(QString driveLetter, FileStagingDraft& draft, std::shared_ptr<const CommitTraversalSource> source,
                    FileStagingConflictPolicy* conflicts = nullptr);
  [[nodiscard]] const QString& driveLetter() const override { return m_driveLetter; }
  [[nodiscard]] const QList<FileStagingEntry>& entries() const override { return m_entries; }
  [[nodiscard]] QList<FileStagingEntry> baselineEntries() const override;
  [[nodiscard]] bool storageAvailable() const override { return m_storageAvailable; }
  [[nodiscard]] const QString& storageError() const override { return m_storageError; }
  void load() override;
  void addEntries(const QStringList& paths, FileStagingKind kind) override;
  void removePaths(const QStringList& paths) override;
  void restorePaths(const QStringList& paths) override;

 private:
  void updateDraft(const QList<FileStagingEntry>& entries);
  void setStorageUnavailable(const QString& error);
  void reportUpdateFailure(const QString& error);
  void reject(FileStagingRejection::Kind kind, QString detail = {});

  QString m_driveLetter;
  FileStagingDraft& m_draft;
  std::shared_ptr<const CommitTraversalSource> m_source;
  FileStagingConflictPolicy* m_conflicts;
  QList<FileStagingEntry> m_entries;
  QString m_storageError;
  bool m_storageAvailable = true;
};

}  // namespace uwf::app
