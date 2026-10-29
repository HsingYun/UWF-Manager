// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QObject>
#include <memory>
#include <optional>

#include "app/ports/FileStagingStore.h"

namespace uwf::app {
// Every volume edits the same in-memory list. Only an accepted application plan
// may replace the persistent list and its associated UWF exclusion.
class FileStagingDraft final : public QObject {
  Q_OBJECT
 public:
  explicit FileStagingDraft(std::shared_ptr<FileStagingStore> store);
  [[nodiscard]] QList<FileStagingEntry> load() const;
  void replace(const QList<FileStagingEntry>& entries);
  [[nodiscard]] QList<FileStagingEntry> baseline() const;
  [[nodiscard]] std::optional<FileStagingChanges> changes() const;
  void reload();
  // Update observed storage while retaining the user's target and unfinished
  // persistence work. reload(), in contrast, explicitly discards that intent.
  void reconcile();
  [[nodiscard]] bool reconciliationRequired() const { return !m_baselineKnown; }
  void apply(const FileStagingChanges& changes);
  [[nodiscard]] FileStagingWriteOperation prepareWrite(const FileStagingChanges& changes) const;
  void acceptWrite(const FileStagingWriteResult& result);

 signals:
  void changed();

 private:
  void ensureLoaded() const;
  std::shared_ptr<FileStagingStore> m_store;
  mutable bool m_loaded = false;
  bool m_baselineKnown = true;
  bool m_persistencePending = false;
  mutable QList<FileStagingEntry> m_baseline;
  mutable QList<FileStagingEntry> m_entries;
};
}  // namespace uwf::app
