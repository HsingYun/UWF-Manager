// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QObject>
#include <cstdint>
#include <memory>
#include <optional>
#include <stop_token>

#include "application/staging/contracts/FileStagingStore.h"

namespace uwf::app {
class CommitTraversalSource;
// All volume editors share one draft. Persistence uses a prepared accepted write.
// Keep the raw stored baseline separate from the normalized object set for
// conflict detection; normalization alone does not schedule a repair write.
class FileStagingDraft final : public QObject {
  Q_OBJECT
 public:
  using DriveMap = std::map<QString, QString, ObjectNameLess>;
  struct ReadSnapshot {
    QList<FileStagingEntry> entries, baseline;
    DriveMap drives, baselineDrives;
  };
  // Host resolution belongs to this shared projection; volume pages filter memory.
  [[nodiscard]] ReadSnapshot readSnapshot();
  struct EditSnapshot {
    QList<FileStagingEntry> entries;
    std::uint64_t revision;
  };
  using Projection = FileStagingProjection;
  struct ReadResult {
    QList<FileStagingEntry> stored;
    Projection projection;
    std::optional<Projection> target{};
    bool retainChanges = false;
  };
  using ReadOperation = std::function<std::optional<ReadResult>(std::stop_token)>;
  using ProjectionOperation = std::function<std::optional<Projection>(const QList<FileStagingEntry>&, std::stop_token)>;
  explicit FileStagingDraft(std::shared_ptr<FileStagingStore> store, std::shared_ptr<const CommitTraversalSource> source = {});
  [[nodiscard]] std::optional<ReadSnapshot> cachedSnapshot() const;
  [[nodiscard]] ReadOperation prepareRead(std::shared_ptr<const CommitTraversalSource> source = {}, bool retainChanges = false) const;
  [[nodiscard]] ProjectionOperation prepareProjection(std::shared_ptr<const CommitTraversalSource> source) const;
  bool acceptRead(std::uint64_t revision, ReadResult result);
  bool acceptReadFailure(std::uint64_t revision, QString error);
  [[nodiscard]] bool storageAvailable() const { return m_storageError.isEmpty(); }
  [[nodiscard]] const QString& storageError() const { return m_storageError; }
  bool acceptProjection(std::uint64_t revision, Projection projection);
  [[nodiscard]] bool busy() const { return m_busy; }
  void setBusy(bool busy);
  [[nodiscard]] QList<FileStagingEntry> load();
  [[nodiscard]] std::optional<EditSnapshot> readEdit();
  void replace(const QList<FileStagingEntry>& entries);
  // An edit may call fallible host ports before committing its candidate. A
  // newer shared target or observation invalidates that candidate, even when
  // the final object set happens to equal the earlier one.
  [[nodiscard]] std::uint64_t revision() const { return m_revision; }
  [[nodiscard]] bool replaceIfCurrent(std::uint64_t revision, const QList<FileStagingEntry>& entries);
  [[nodiscard]] QList<FileStagingEntry> baseline() const;
  [[nodiscard]] std::optional<FileStagingChanges> changes() const;
  // Complete cached editing state, including an unchanged target. No storage
  // reads; nullopt until a successful load. During reconciliation this retains
  // user intent, not proof of persisted state. Review requires cachedSnapshot().
  [[nodiscard]] std::optional<FileStagingChanges> editSnapshot() const;
  void reload();
  // Update observed storage while retaining the user's target and unfinished
  // persistence work. reload(), in contrast, explicitly discards that intent.
  void reconcile();
  void discardChanges();
  [[nodiscard]] bool reconciliationRequired() const { return !m_baselineKnown; }
  // Prepared I/O outlives this facade. Deliver its receipt only to a live owner;
  // failures still propagate to the synchronous caller.
  void apply(const FileStagingChanges& changes);
  [[nodiscard]] FileStagingWriteOperation prepareWrite(const FileStagingChanges& changes) const;
  void acceptWrite(const FileStagingWriteResult& result);

 signals:
  void changed();
  void busyChanged();
  void storageAvailabilityChanged();

 private:
  bool m_busy = false;
  bool ensureLoaded();
  [[nodiscard]] std::optional<Projection> projectEntries(const QList<FileStagingEntry>& entries) const;
  bool readSynchronously(bool retainChanges);
  void replaceCached(Projection projection);
  std::shared_ptr<FileStagingStore> m_store;
  std::shared_ptr<const CommitTraversalSource> m_source;
  bool m_loaded = false;
  bool m_baselineKnown = true;
  bool m_persistencePending = false;
  QString m_storageError;
  QList<FileStagingEntry> m_storedBaseline;
  QList<FileStagingEntry> m_baseline;
  QList<FileStagingEntry> m_entries;
  FileStagingVolumeIdentities m_volumeIdentities;
  FileStagingVolumeIdentities m_baselineVolumeIdentities;
  DriveMap m_drives, m_baselineDrives;
  std::uint64_t m_revision = 0;
};
}  // namespace uwf::app
