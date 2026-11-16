// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "application/staging/editing/FileStagingDraft.h"

#include <QPointer>
#include <algorithm>
#include <exception>
#include <set>
#include <stdexcept>
#include <utility>

#include "application/commit/contracts/CommitTraversalSource.h"
#include "application/localization/ApplicationText.h"
#include "application/operations/contracts/OperationError.h"
#include "application/staging/editing/FileStagingEntries.h"
#include "application/staging/editing/FileStagingPath.h"
#include "foundation/logging/Log.h"
#include "foundation/paths/DriveLetter.h"

namespace uwf::app {
namespace {
[[noreturn]] void draftChanged() {
  throw std::runtime_error(ApplicationText::tr("The file staging draft changed after this plan was opened. Review a new plan.").toStdString());
}
using EntryKeys = std::multiset<FileStagingEntryKey>;
EntryKeys entryKeys(const QList<FileStagingEntry>& entries) {
  EntryKeys keys;
  for (const auto& entry : entries) keys.insert(fileStagingEntryKey(entry));
  return keys;
}
bool sameEntries(const QList<FileStagingEntry>& first, const QList<FileStagingEntry>& second) {
  return first.size() == second.size() && entryKeys(first) == entryKeys(second);
}
// A selection retains its persisted address. Physical aliases are deduplicated
// during projection, but explicitly replacing that address is still an edit.
// Completing a binding observation at the same address is not a user edit;
// two known bindings must continue to identify the same object.
class SelectionIndex final {
 public:
  void insert(const FileStagingEntry& entry) {
    const auto display = fileStagingEntryKey(entry);
    m_displays.insert(display);
    if (entry.fileTarget)
      m_objects.emplace(display, ObjectIdentity(fileStagingObjectPath(entry)));
    else
      m_unboundDisplays.insert(display);
  }
  [[nodiscard]] bool contains(const FileStagingEntry& entry) const {
    const auto display = fileStagingEntryKey(entry);
    if (!entry.fileTarget) return m_displays.contains(display);
    return m_objects.contains({display, ObjectIdentity(fileStagingObjectPath(entry))}) || m_unboundDisplays.contains(display);
  }

 private:
  std::set<FileStagingEntryKey> m_displays;
  std::set<FileStagingEntryKey> m_unboundDisplays;
  std::set<std::pair<FileStagingEntryKey, ObjectIdentity>> m_objects;
};
QList<FileStagingEntry> difference(const QList<FileStagingEntry>& first, const QList<FileStagingEntry>& second) {
  QList<FileStagingEntry> result;
  SelectionIndex selections;
  for (const auto& entry : second) selections.insert(entry);
  for (const auto& entry : first)
    if (!selections.contains(entry)) result.append(entry);
  return result;
}
}  // namespace

QList<FileStagingEntry> FileStagingChanges::added() const { return difference(after, before); }
QList<FileStagingEntry> FileStagingChanges::removed() const { return difference(before, after); }
std::size_t FileStagingChanges::count() const {
  const auto edits = static_cast<std::size_t>(added().size() + removed().size());
  return edits == 0 && persistencePending ? 1 : edits;
}

FileStagingDraft::FileStagingDraft(std::shared_ptr<FileStagingStore> store, std::shared_ptr<const CommitTraversalSource> source)
    : m_store(std::move(store)), m_source(std::move(source)) {}
namespace {
std::optional<FileStagingDraft::Projection> projectStagingEntries(const QList<FileStagingEntry>& entries,
                                                                  const std::shared_ptr<const CommitTraversalSource>& source,
                                                                  const std::function<bool()>& current) {
  using Projection = FileStagingDraft::Projection;
  Projection result{normalizeFileStagingEntries(entries, source.get()), {}, {}};
  if (!current()) return std::nullopt;
  std::set<QString, ObjectNameLess> roots;
  for (const auto& entry : std::as_const(result.entries)) {
    const auto root = QString::fromStdString(drive::split(entry.path.toStdString()).letter);
    auto displayDrive = root;
    QString actualDrive;
    if (source && (entry.kind == FileStagingKind::File || entry.kind == FileStagingKind::Directory) && isAbsoluteLocalFileStagingPath(entry.path)) {
      try {
        const auto actual =
            QString::fromStdString(entry.fileTarget ? source->driveLetterForPath(entry.path, *entry.fileTarget) : source->driveLetterForPath(entry.path));
        if (!actual.isEmpty()) displayDrive = actualDrive = actual;
      } catch (const std::exception&) {
        // Unresolved stored names remain removable on their display-drive page.
      }
      if (!current()) return std::nullopt;
    }
    result.drives.insert_or_assign(entry.path, displayDrive);
    if (!entry.fileTarget) continue;
    if (!root.isEmpty()) roots.insert(root);
    if (!actualDrive.isEmpty()) result.volumes.insert_or_assign(actualDrive, entry.fileTarget->volumeId);
  }
  if (!source) return result;
  for (const auto& root : roots) {
    try {
      const auto target = bindFileCommitTarget(*source, {CommitTargetKind::Folder, root + u'\\', {}, {}});
      result.volumes.insert_or_assign(root, target.fileTarget->volumeId);
    } catch (const std::exception&) {
      // Losing a mount never assigns the object to a guessed management root.
    }
    if (!current()) return std::nullopt;
  }
  return result;
}
}  // namespace
std::optional<FileStagingDraft::Projection> FileStagingDraft::projectEntries(const QList<FileStagingEntry>& entries) const {
  const QPointer self(this);
  const auto revision = m_revision;
  const auto source = m_source;
  const auto input = entries;
  return projectStagingEntries(input, source, [self, revision] { return self && self->m_revision == revision; });
}
std::optional<FileStagingDraft::ReadSnapshot> FileStagingDraft::cachedSnapshot() const {
  if (!storageAvailable()) throw std::runtime_error(m_storageError.toStdString());
  if (!m_loaded) return std::nullopt;
  if (!m_baselineKnown) draftChanged();
  return ReadSnapshot{m_entries, m_baseline, m_drives, m_baselineDrives};
}
FileStagingDraft::ReadOperation FileStagingDraft::prepareRead(std::shared_ptr<const CommitTraversalSource> source, const bool retainChanges) const {
  if (!source) source = m_source;
  const auto target = retainChanges && m_loaded ? std::optional{m_entries} : std::nullopt;
  return [store = m_store, source = std::move(source), target, retainChanges](const std::stop_token stop) -> std::optional<ReadResult> {
    if (stop.stop_requested()) return std::nullopt;
    auto stored = store->load();
    auto projection = projectStagingEntries(stored, source, [stop] { return !stop.stop_requested(); });
    if (!projection) return std::nullopt;
    std::optional<Projection> projectedTarget;
    if (target) {
      projectedTarget = projectStagingEntries(*target, source, [stop] { return !stop.stop_requested(); });
      if (!projectedTarget) return std::nullopt;
    }
    return ReadResult{std::move(stored), std::move(*projection), std::move(projectedTarget), retainChanges};
  };
}
FileStagingDraft::ProjectionOperation FileStagingDraft::prepareProjection(std::shared_ptr<const CommitTraversalSource> source) const {
  return [source = std::move(source)](const QList<FileStagingEntry>& entries, const std::stop_token stop) {
    return projectStagingEntries(entries, source, [stop] { return !stop.stop_requested(); });
  };
}
bool FileStagingDraft::acceptRead(const std::uint64_t revision, ReadResult result) {
  if (m_revision != revision) return false;
  const bool recovered = !storageAvailable();
  m_storageError.clear();
  m_storedBaseline = std::move(result.stored);
  m_baseline = result.projection.entries;
  m_baselineDrives = result.projection.drives;
  m_baselineVolumeIdentities = result.projection.volumes;
  auto target = result.target ? std::move(*result.target) : std::move(result.projection);
  m_entries = std::move(target.entries);
  m_volumeIdentities = std::move(target.volumes);
  m_drives = std::move(target.drives);
  m_loaded = true;
  m_baselineKnown = true;
  if (!result.retainChanges) m_persistencePending = false;
  ++m_revision;
  const QPointer self(this);
  emit changed();
  if (self && recovered) emit storageAvailabilityChanged();
  return true;
}
bool FileStagingDraft::acceptReadFailure(const std::uint64_t revision, QString error) {
  if (m_revision != revision) return false;
  if (error.isEmpty()) error = ApplicationText::tr("Unavailable");
  if (m_storageError == error) return true;
  m_storageError = std::move(error);
  UWF_LOG_E("staging") << "file staging registry read failed: error=" << m_storageError.toStdString();
  emit storageAvailabilityChanged();
  return true;
}
bool FileStagingDraft::acceptProjection(const std::uint64_t revision, Projection projection) {
  if (m_revision != revision || !m_loaded || !m_baselineKnown) return false;
  replaceCached(std::move(projection));
  return true;
}
void FileStagingDraft::setBusy(const bool busy) {
  if (m_busy == busy) return;
  m_busy = busy;
  emit busyChanged();
}
bool FileStagingDraft::ensureLoaded() { return m_loaded || readSynchronously(false); }
bool FileStagingDraft::readSynchronously(const bool retainChanges) {
  const QPointer self(this);
  const auto revision = m_revision;
  const auto store = m_store;
  QList<FileStagingEntry> stored;
  std::exception_ptr failure;
  try {
    stored = store->load();
  } catch (...) {
    failure = std::current_exception();
  }
  if (!self) draftChanged();
  if (self->m_revision != revision) return false;
  if (failure) {
    try {
      std::rethrow_exception(failure);
    } catch (const std::exception& error) {
      self->acceptReadFailure(revision, QString::fromUtf8(error.what()));
    } catch (...) {
      self->acceptReadFailure(revision, ApplicationText::tr("Unavailable"));
    }
    std::rethrow_exception(failure);
  }
  auto baseline = self->projectEntries(stored);
  if (!baseline) return false;
  std::optional<Projection> target;
  if (retainChanges && self->m_loaded) {
    target = self->projectEntries(self->m_entries);
    if (!target) return false;
  }
  return self->acceptRead(revision, {std::move(stored), std::move(*baseline), std::move(target), retainChanges});
}
QList<FileStagingEntry> FileStagingDraft::load() {
  const QPointer self(this);
  const auto loaded = ensureLoaded();
  if (!self || (!loaded && !self->m_loaded)) draftChanged();
  if (!storageAvailable()) throw std::runtime_error(m_storageError.toStdString());
  if (!m_baselineKnown)
    throw std::runtime_error(ApplicationText::tr("Some changes may already have been written. Refresh the system state before retrying.").toStdString());
  return m_entries;
}
FileStagingDraft::ReadSnapshot FileStagingDraft::readSnapshot() {
  const auto entries = load();
  return {entries, m_baseline, m_drives, m_baselineDrives};
}
QList<FileStagingEntry> FileStagingDraft::baseline() const { return m_baseline; }
void FileStagingDraft::replace(const QList<FileStagingEntry>& entries) {
  auto normalized = projectEntries(entries);
  if (!normalized) return;
  if (!ensureLoaded()) return;
  static_cast<void>(load());
  replaceCached(std::move(*normalized));
}
void FileStagingDraft::replaceCached(Projection projection) {
  if (m_drives == projection.drives && m_volumeIdentities == projection.volumes &&
      std::ranges::equal(m_entries, projection.entries,
                         [](const auto& left, const auto& right) { return left == right && left.fileTarget == right.fileTarget; }))
    return;
  m_entries = std::move(projection.entries);
  m_volumeIdentities = std::move(projection.volumes);
  m_drives = std::move(projection.drives);
  ++m_revision;
  emit changed();
}
std::optional<FileStagingChanges> FileStagingDraft::changes() const {
  const auto edits = editSnapshot();
  return edits && edits->count() != 0 ? edits : std::nullopt;
}
std::optional<FileStagingChanges> FileStagingDraft::editSnapshot() const {
  if (!m_loaded) return std::nullopt;
  return FileStagingChanges{m_baseline, m_entries, m_persistencePending, m_storedBaseline, m_volumeIdentities};
}
void FileStagingDraft::reload() { (void)readSynchronously(false); }
void FileStagingDraft::reconcile() { (void)readSynchronously(true); }
void FileStagingDraft::discardChanges() {
  if (!m_baselineKnown)
    throw std::runtime_error(ApplicationText::tr("Some changes may already have been written. Refresh the system state before retrying.").toStdString());
  if (!m_loaded || !storageAvailable())
    throw std::runtime_error((m_storageError.isEmpty() ? ApplicationText::tr("Unavailable") : m_storageError).toStdString());
  m_entries = m_baseline;
  m_drives = m_baselineDrives;
  m_volumeIdentities = m_baselineVolumeIdentities;
  m_persistencePending = false;
  ++m_revision;
  emit changed();
}
FileStagingWriteOperation FileStagingDraft::prepareWrite(const FileStagingChanges& edits) const {
  if (!m_loaded || !m_baselineKnown) draftChanged();
  if (!storageAvailable()) throw std::runtime_error(m_storageError.toStdString());
  return [store = m_store, source = m_source, revision = m_revision, target = m_entries, edits = edits]() mutable {
    FileStagingWriteResult result;
    result.revision = revision;
    bool replacementCompleted = false;
    try {
      edits.after = normalizeFileStagingEntries(edits.after);
      if (!sameEntries(target, edits.after)) draftChanged();
      if (source)
        for (const auto& entry : std::as_const(edits.after))
          if (entry.fileTarget) (void)bindFileCommitTarget(*source, {CommitTargetKind::File, entry.path, {}, {}, entry.fileTarget});
      if (edits.storedBefore && !sameEntries(normalizeFileStagingEntries(*edits.storedBefore, source.get()), edits.before))
        throw std::runtime_error(
            ApplicationText::tr("The saved file staging list changed after this plan was opened. Refresh and review the changes.").toStdString());
      const auto current = store->load();
      if (!sameEntries(current, edits.storedBefore.value_or(edits.before)) && !sameEntries(current, edits.after))
        throw std::runtime_error(
            ApplicationText::tr("The saved file staging list changed after this plan was opened. Refresh and review the changes.").toStdString());
      result.attempted = true;
      store->replace(edits.after);
      replacementCompleted = true;
      result.observed = store->load();
      if (!sameEntries(*result.observed, edits.after))
        throw std::runtime_error(ApplicationText::tr("The saved file staging list does not match the accepted plan.").toStdString());
    } catch (...) {
      result.failure = std::current_exception();
      if (result.attempted) {
        try {
          result.observed = store->load();
        } catch (...) {
          result.observed.reset();
        }
        try {
          std::rethrow_exception(result.failure);
        } catch (const OperationError& error) {
          // The store owns failures from replace itself. Once it returned,
          // even a read-only confirmation failure follows a completed write.
          if (replacementCompleted) result.failure = std::make_exception_ptr(OperationError(error, WriteEffect::Uncertain));
        } catch (const std::exception& error) {
          result.failure =
              std::make_exception_ptr(OperationError(std::make_error_code(std::errc::state_not_recoverable), error.what(), WriteEffect::Uncertain));
        } catch (...) {
          result.failure =
              std::make_exception_ptr(OperationError(std::make_error_code(std::errc::state_not_recoverable),
                                                     ApplicationText::tr("File staging could not be saved").toStdString(), WriteEffect::Uncertain));
        }
      }
    }
    if (result.observed) {
      try {
        result.projection = projectStagingEntries(*result.observed, source, [] { return true; });
      } catch (const std::exception& error) {
        if (!result.failure)
          result.failure =
              std::make_exception_ptr(OperationError(std::make_error_code(std::errc::state_not_recoverable), error.what(), WriteEffect::Uncertain));
      } catch (...) {
        if (!result.failure)
          result.failure =
              std::make_exception_ptr(OperationError(std::make_error_code(std::errc::state_not_recoverable),
                                                     ApplicationText::tr("File staging could not be saved").toStdString(), WriteEffect::Uncertain));
      }
    }
    return result;
  };
}
void FileStagingDraft::acceptWrite(const FileStagingWriteResult& result) {
  if (!result.attempted || result.revision != m_revision) return;
  if (result.observed && result.projection) {
    m_storedBaseline = *result.observed;
    m_baseline = result.projection->entries;
    m_baselineDrives = result.projection->drives;
    m_baselineVolumeIdentities = result.projection->volumes;
  }
  m_baselineKnown = result.observed.has_value() && result.projection.has_value();
  m_persistencePending = result.failure != nullptr;
  ++m_revision;
  emit changed();
}
void FileStagingDraft::apply(const FileStagingChanges& edits) {
  const QPointer self(this);
  const auto operation = prepareWrite(edits);
  const auto result = operation();
  if (self) self->acceptWrite(result);
  if (result.failure) std::rethrow_exception(result.failure);
}
}  // namespace uwf::app
#include "moc_FileStagingDraft.cpp"
