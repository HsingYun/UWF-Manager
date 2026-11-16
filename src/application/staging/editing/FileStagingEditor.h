// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QPointer>
#include <cstdint>
#include <memory>

#include "application/commit/contracts/CommitPlan.h"
#include "application/commit/contracts/CommitTraversalSource.h"
#include "application/staging/contracts/FileStagingEditing.h"
#include "application/staging/contracts/FileStagingStore.h"
#include "application/staging/editing/FileStagingDraft.h"

namespace uwf::app {
class FileStagingConflictPolicy;

class FileStagingEditor final : public FileStagingEditing {
 public:
  FileStagingEditor(QString driveLetter, FileStagingDraft& draft, std::shared_ptr<const CommitTraversalSource> source,
                    FileStagingConflictPolicy* conflicts = nullptr, FileStagingScope scope = {});
  [[nodiscard]] const QString& driveLetter() const override { return m_driveLetter; }
  [[nodiscard]] const QList<FileStagingEntry>& entries() const override { return m_entries; }
  [[nodiscard]] QList<FileStagingEntry> baselineEntries() const override;
  [[nodiscard]] bool storageAvailable() const override { return m_storageAvailable; }
  [[nodiscard]] const QString& storageError() const override { return m_storageError; }
  void load() override;
  [[nodiscard]] bool busy() const override { return m_draft && m_draft->busy(); }
  [[nodiscard]] FileStagingPreparation prepareLoad() override;
  [[nodiscard]] FileStagingPreparation prepareAddEntries(const QList<FileStagingEntry>& entries) override;
  [[nodiscard]] FileStagingPreparation prepareRemovePaths(const QStringList& paths) override;
  [[nodiscard]] FileStagingPreparation prepareRestorePaths(const QStringList& paths) override;

 private:
  struct Candidate {
    QList<FileStagingEntry> entries;
    QList<FileStagingRejection> rejections;
    bool changed = false;
  };
  enum class Update { Load, Add, Remove, Restore };
  FileStagingPreparation prepareUpdate(Update update, QList<FileStagingEntry> entries = {}, QStringList paths = {});
  static std::optional<Candidate> prepareEntries(Update update, const QList<FileStagingEntry>& entries, const QList<FileStagingEntry>& baseline,
                                                 const QList<FileStagingEntry>& selected, const QStringList& paths, const QString& driveLetter,
                                                 const std::shared_ptr<const CommitTraversalSource>& source,
                                                 const std::function<std::optional<QString>(const FileStagingEntry&)>& conflict,
                                                 const std::function<bool()>& current);
  void setStorageUnavailable(const QString& error, std::uint64_t generation);
  void reportUpdateFailure(const QString& error);
  void reject(FileStagingRejection::Kind kind, QString detail = {});

  QString m_driveLetter;
  FileStagingScope m_scope;
  QPointer<FileStagingDraft> m_draft;
  std::shared_ptr<const CommitTraversalSource> m_source;
  QPointer<FileStagingConflictPolicy> m_conflicts;
  QList<FileStagingEntry> m_baselineEntries;
  QList<FileStagingEntry> m_entries;
  QString m_storageError;
  bool m_storageAvailable = true;
  std::uint64_t m_loadGeneration = 0;
  std::uint64_t m_editGeneration = 0;
};

}  // namespace uwf::app
