// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "application/staging/editing/FileStagingEditor.h"

#include <QPointer>
#include <QScopeGuard>
#include <algorithm>
#include <exception>
#include <map>
#include <set>
#include <stdexcept>
#include <utility>
#include <variant>

#include "application/commit/contracts/CommitTraversalSource.h"
#include "application/localization/ApplicationText.h"
#include "application/paths/PathPolicy.h"
#include "application/staging/editing/FileStagingConflictPolicy.h"
#include "application/staging/editing/FileStagingEntries.h"
#include "application/staging/editing/FileStagingPath.h"
#include "foundation/logging/Log.h"
#include "foundation/paths/DriveLetter.h"

namespace uwf::app {

namespace {

void sortEntries(QList<FileStagingEntry>& entries) {
  std::ranges::sort(entries,
                    [](const FileStagingEntry& lhs, const FileStagingEntry& rhs) { return QString::compare(lhs.path, rhs.path, Qt::CaseInsensitive) < 0; });
}

QList<FileStagingEntry> entriesForDrive(const QList<FileStagingEntry>& entries, const QString& driveLetter, const FileStagingDraft::DriveMap& drives,
                                        const FileStagingScope& scope) {
  QList<FileStagingEntry> filtered;
  for (const auto& entry : entries) {
    const auto found = drives.find(entry.path);
    auto owner = found != drives.end() ? found->second : QString{};
    if (!scope.fallbackDrive.isEmpty() && !scope.managedDrives.contains(owner, Qt::CaseInsensitive)) owner = scope.fallbackDrive;
    if (owner.compare(driveLetter, Qt::CaseInsensitive) == 0) filtered.append(entry);
  }
  return filtered;
}

// One editing round observes each display path once. Failures are observations
// too; another phase in this round must not silently replace one with success.
class EditingObservations final : public CommitTraversalSource {
 public:
  explicit EditingObservations(std::shared_ptr<const CommitTraversalSource> source) : m_source(std::move(source)) {}
  FileCommitTarget resolveFileTarget(const QString& path) const override {
    return observe(m_targets, path, [&] { return m_source->resolveFileTarget(path); });
  }
  std::string driveLetterForPath(const QString& path) const override {
    return observe(m_drives, path, [&] { return m_source->driveLetterForPath(path, resolveFileTarget(path)); });
  }
  std::optional<CommitNode> inspect(const CommitNode& target) const override {
    auto node = m_source->inspect(target);
    if (node && node->fileTarget) m_targets.try_emplace(target.path, *node->fileTarget);
    return node;
  }
  bool exists(const CommitNode& target) const override { return m_source->exists(target); }
  CommitEnumerationError enumerateChildren(const CommitNode& target, const std::stop_token stop, const CommitNodeVisitor& visit) const override {
    return m_source->enumerateChildren(target, stop, visit);
  }

 private:
  template <typename Value, typename Read>
  static Value observe(std::map<QString, std::variant<Value, std::exception_ptr>, ObjectNameLess>& values, const QString& path, Read read) {
    auto found = values.find(path);
    if (found == values.end()) {
      try {
        found = values.emplace(path, read()).first;
      } catch (...) {
        found = values.emplace(path, std::current_exception()).first;
      }
    }
    if (const auto* error = std::get_if<std::exception_ptr>(&found->second)) std::rethrow_exception(*error);
    return std::get<Value>(found->second);
  }
  std::shared_ptr<const CommitTraversalSource> m_source;
  mutable std::map<QString, std::variant<FileCommitTarget, std::exception_ptr>, ObjectNameLess> m_targets;
  mutable std::map<QString, std::variant<std::string, std::exception_ptr>, ObjectNameLess> m_drives;
};

}  // namespace

FileStagingEditor::FileStagingEditor(QString driveLetter, FileStagingDraft& draft, std::shared_ptr<const CommitTraversalSource> source,
                                     FileStagingConflictPolicy* conflicts, FileStagingScope scope)
    : m_driveLetter(QString::fromStdString(drive::normalize(driveLetter.toStdString()))),
      m_scope(std::move(scope)),
      m_draft(&draft),
      m_source(std::move(source)),
      m_conflicts(conflicts) {
  if (m_driveLetter.isEmpty()) throw std::invalid_argument("file staging requires a valid drive letter");
  if (!m_source) throw std::invalid_argument("file staging traversal source is required");
  connect(&draft, &FileStagingDraft::changed, this, &FileStagingEditor::load);
  connect(&draft, &FileStagingDraft::storageAvailabilityChanged, this, [this] {
    if (m_draft && !m_draft->storageAvailable()) setStorageUnavailable(m_draft->storageError(), ++m_loadGeneration);
  });
  connect(&draft, &FileStagingDraft::busyChanged, this, &FileStagingEditing::busyChanged);
  connect(&draft, &QObject::destroyed, this, &FileStagingEditor::load);
  m_storageAvailable = draft.storageAvailable();
  m_storageError = draft.storageError();
}

QList<FileStagingEntry> FileStagingEditor::baselineEntries() const { return m_baselineEntries; }

void FileStagingEditor::load() {
  const auto generation = ++m_loadGeneration;
  const QPointer self(this);
  const auto current = [self, generation] { return self && self->m_loadGeneration == generation; };
  const auto driveLetter = m_driveLetter;
  const auto draft = m_draft;
  const auto revision = draft ? draft->revision() : 0;
  try {
    if (!draft) {
      setStorageUnavailable(ApplicationText::tr("The file staging list could not be read:\n%1").arg(ApplicationText::tr("Unavailable")), generation);
      return;
    }
    const auto snapshot = draft->readSnapshot();
    if (!current()) return;
    auto visibleBaseline = entriesForDrive(snapshot.baseline, driveLetter, snapshot.baselineDrives, m_scope);
    auto visibleEntries = entriesForDrive(snapshot.entries, driveLetter, snapshot.drives, m_scope);
    sortEntries(visibleBaseline);
    sortEntries(visibleEntries);
    const bool recovered = !m_storageAvailable;
    // Publish only after committing the full projection; reentrant loads or
    // destruction must prevent this operation from overwriting newer state.
    m_baselineEntries = std::move(visibleBaseline);
    m_entries = std::move(visibleEntries);
    m_storageAvailable = true;
    m_storageError.clear();
    if (m_conflicts) m_conflicts->setStagedEntries(snapshot.entries);
    if (!current()) return;
    if (recovered) emit storageAvailabilityChanged();
    if (!current()) return;
    emit entriesChanged();
  } catch (const std::exception& error) {
    if (current() && draft) {
      if (!draft->acceptReadFailure(revision, QString::fromUtf8(error.what()))) return;
      if (current() && m_storageAvailable) setStorageUnavailable(draft->storageError(), generation);
    }
  }
}

FileStagingPreparation FileStagingEditor::prepareLoad() { return prepareUpdate(Update::Load); }
FileStagingPreparation FileStagingEditor::prepareAddEntries(const QList<FileStagingEntry>& entries) {
  return entries.isEmpty() ? FileStagingPreparation{} : prepareUpdate(Update::Add, entries);
}
FileStagingPreparation FileStagingEditor::prepareRemovePaths(const QStringList& paths) {
  return paths.isEmpty() ? FileStagingPreparation{} : prepareUpdate(Update::Remove, {}, paths);
}
FileStagingPreparation FileStagingEditor::prepareRestorePaths(const QStringList& paths) {
  return paths.isEmpty() ? FileStagingPreparation{} : prepareUpdate(Update::Restore, {}, paths);
}
FileStagingPreparation FileStagingEditor::prepareUpdate(const Update update, QList<FileStagingEntry> entries, QStringList paths) {
  const auto draft = m_draft;
  if (!draft || draft->busy()) return {};
  std::optional<FileStagingDraft::ReadSnapshot> cached;
  try {
    if (draft->storageAvailable()) cached = draft->cachedSnapshot();
  } catch (const std::exception& error) {
    reportUpdateFailure(QString::fromUtf8(error.what()));
    return {};
  }
  if (update == Update::Load && cached) return FileStagingEditing::prepareLoad();
  struct Prepared {
    std::optional<FileStagingDraft::ReadResult> initial;
    std::optional<Candidate> candidate;
    std::optional<FileStagingDraft::Projection> projection;
  };
  const auto prepared = std::make_shared<Prepared>();
  const auto source = std::make_shared<EditingObservations>(m_source);
  const auto read = draft->prepareRead(source, true);
  const auto project = draft->prepareProjection(source);
  const auto conflict = m_conflicts ? m_conflicts->exclusionChecker() : std::function<std::optional<QString>(const FileStagingEntry&)>{};
  const auto generation = ++m_editGeneration;
  const auto revision = draft->revision();
  const bool reading = !cached.has_value();
  const QPointer self(this);
  auto work = [prepared, cached = std::move(cached), read, project, source, update, entries = std::move(entries), paths = std::move(paths),
               drive = m_driveLetter, conflict](const std::stop_token stop) {
    auto snapshot = cached;
    if (!snapshot) {
      prepared->initial = read(stop);
      if (!prepared->initial) return;
      const auto& baseline = prepared->initial->projection;
      const auto& target = prepared->initial->target ? *prepared->initial->target : baseline;
      snapshot = FileStagingDraft::ReadSnapshot{target.entries, baseline.entries, target.drives, baseline.drives};
    }
    if (update == Update::Load || stop.stop_requested()) return;
    prepared->candidate =
        prepareEntries(update, snapshot->entries, snapshot->baseline, entries, paths, drive, source, conflict, [stop] { return !stop.stop_requested(); });
    if (prepared->candidate && prepared->candidate->changed) prepared->projection = project(prepared->candidate->entries, stop);
  };
  auto cancel = [self, draft, generation] {
    if (self && self->m_editGeneration != generation) return;
    if (self) ++self->m_editGeneration;
    if (draft) draft->setBusy(false);
  };
  auto complete = [self, draft, prepared, generation, revision, reading](const std::exception_ptr failure) mutable {
    if (!draft || (self && self->m_editGeneration != generation)) return;
    const auto finished = qScopeGuard([draft] {
      if (draft) draft->setBusy(false);
    });
    if (draft->revision() != revision) return;
    if (failure) {
      try {
        std::rethrow_exception(failure);
      } catch (const std::exception& error) {
        if (reading && !prepared->initial)
          draft->acceptReadFailure(revision, QString::fromUtf8(error.what()));
        else if (self)
          self->reportUpdateFailure(QString::fromUtf8(error.what()));
      } catch (...) {
        if (reading && !prepared->initial)
          draft->acceptReadFailure(revision, ApplicationText::tr("Unavailable"));
        else if (self)
          self->reportUpdateFailure(ApplicationText::tr("Unavailable"));
      }
      return;
    }
    auto acceptedRevision = revision;
    if (prepared->initial) {
      if (!draft->acceptRead(revision, std::move(*prepared->initial))) return;
      ++acceptedRevision;
    }
    if (!self || !draft || self->m_editGeneration != generation || draft->revision() != acceptedRevision) return;
    if (prepared->candidate && prepared->candidate->changed && prepared->projection) {
      // Exclusions may have changed while the host observations were in flight.
      if (self->m_conflicts) {
        const auto previous = draft->cachedSnapshot();
        const auto additions = FileStagingChanges{previous->entries, prepared->projection->entries}.added();
        for (const auto& entry : additions) {
          if (const auto conflict = self->m_conflicts->conflictingExclusion(entry)) {
            self->reject(FileStagingRejection::Kind::ExclusionConflict, *conflict);
            return;
          }
        }
      }
      if (!draft->acceptProjection(acceptedRevision, std::move(*prepared->projection))) return;
    }
    if (!self || self->m_editGeneration != generation) return;
    if (prepared->candidate) {
      for (const auto& rejection : std::as_const(prepared->candidate->rejections)) {
        if (!self || self->m_editGeneration != generation) return;
        emit self->pathRejected(rejection);
      }
    }
  };
  draft->setBusy(true);
  if (!self || !draft || self->m_editGeneration != generation) {
    cancel();
    return {};
  }
  return {std::move(work), std::move(complete), std::move(cancel)};
}

std::optional<FileStagingEditor::Candidate> FileStagingEditor::prepareEntries(
    const Update update, const QList<FileStagingEntry>& entries, const QList<FileStagingEntry>& baseline, const QList<FileStagingEntry>& selectedEntries,
    const QStringList& selectedPaths, const QString& driveLetter, const std::shared_ptr<const CommitTraversalSource>& source,
    const std::function<std::optional<QString>(const FileStagingEntry&)>& conflict, const std::function<bool()>& current) {
  Candidate result{entries, {}, false};
  auto& candidate = result.entries;
  const auto reject = [&](const FileStagingRejection::Kind kind, QString detail = {}) { result.rejections.append({kind, std::move(detail)}); };
  const auto belongsToDrive = [&](const QString& path) {
    try {
      const auto targetDrive = QString::fromStdString(source->driveLetterForPath(path));
      if (!current()) return false;
      if (targetDrive.isEmpty()) {
        reject(FileStagingRejection::Kind::MissingDrive, path);
        return false;
      }
      if (targetDrive.compare(driveLetter, Qt::CaseInsensitive) != 0) {
        reject(FileStagingRejection::Kind::WrongDrive, targetDrive);
        return false;
      }
      return true;
    } catch (const std::exception& error) {
      if (current()) reject(FileStagingRejection::Kind::ResolutionFailed, QString::fromUtf8(error.what()));
      return false;
    }
  };
  if (update == Update::Restore) {
    const auto& selected = selectedPaths;
    QList<FileStagingEntry> selections;
    for (const auto& path : selected) selections.append({FileStagingKind::File, path});
    const auto aliases = normalizeFileStagingEntries(selections, source.get());
    if (!current()) return std::nullopt;
    std::set<ObjectIdentity> identities;
    for (const auto& entry : aliases) identities.emplace(fileStagingObjectPath(entry));
    for (const auto& entry : baseline) {
      if (!identities.contains(ObjectIdentity(fileStagingObjectPath(entry))) &&
          !std::ranges::any_of(selected, [&](const QString& path) { return equalObjectNames(path, entry.path); }))
        continue;
      auto restored = entry;
      try {
        restored.fileTarget = bindFileCommitTarget(*source, {CommitTargetKind::File, entry.path, {}, {}, entry.fileTarget}).fileTarget;
      } catch (const std::exception& error) {
        if (current()) reject(FileStagingRejection::Kind::ResolutionFailed, QString::fromUtf8(error.what()));
        return result;
      }
      if (!current()) return std::nullopt;
      if (!belongsToDrive(restored.path)) return current() ? std::optional{result} : std::nullopt;
      if (conflict) {
        if (const auto found = conflict(restored)) {
          reject(FileStagingRejection::Kind::ExclusionConflict, *found);
          return result;
        }
      }
      // Restoring a selection chooses its saved address and kind. Automatic
      // alias projection must not replace it with the current representative.
      const auto identity = ObjectIdentity(fileStagingObjectPath(restored));
      candidate.removeIf([&](const FileStagingEntry& existing) { return ObjectIdentity(fileStagingObjectPath(existing)) == identity; });
      candidate.append(std::move(restored));
    }
    result.changed = true;
  } else if (update == Update::Add) {
    const auto& selected = selectedEntries;

    bool accepted = false;
    for (const auto& entry : selected) {
      const auto kind = entry.kind;
      QString path = entry.path;
      if (path.isEmpty()) continue;
      if (kind != FileStagingKind::File && kind != FileStagingKind::Directory) {
        reject(FileStagingRejection::Kind::WrongType, path);
        continue;
      }
      if (!isAbsoluteLocalFileStagingPath(path)) {
        reject(FileStagingRejection::Kind::NonLocalPath, path);
        continue;
      }
      path = paths::normalize(path);
      std::optional<CommitNode> node;
      try {
        node = source->inspect({CommitTargetKind::File, path, {}, {}});
        if (!current()) return std::nullopt;
      } catch (const std::exception& error) {
        if (!current()) return std::nullopt;
        reject(FileStagingRejection::Kind::InspectionFailed, QString::fromUtf8(error.what()));
        continue;
      }
      if (node && !node->skippedReason.isEmpty()) {
        reject(FileStagingRejection::Kind::ReparsePoint, path);
        continue;
      }
      const bool expectedType = node && node->kind == (kind == FileStagingKind::File ? CommitTargetKind::File : CommitTargetKind::Folder);
      if (!expectedType) {
        reject(FileStagingRejection::Kind::WrongType, path);
        continue;
      }
      try {
        node = bindFileCommitTarget(*source, *node);
      } catch (const std::exception& error) {
        if (!current()) return std::nullopt;
        reject(FileStagingRejection::Kind::ResolutionFailed, QString::fromUtf8(error.what()));
        continue;
      }
      if (!current()) return std::nullopt;
      if (!belongsToDrive(path)) {
        if (!current()) return std::nullopt;
        continue;
      }
      if (kind == FileStagingKind::Directory && node->fileTarget->relativePath == QStringLiteral("\\")) {
        reject(FileStagingRejection::Kind::DriveRoot);
        continue;
      }
      if (conflict) {
        if (const auto found = conflict(FileStagingEntry{kind, path, node->fileTarget})) {
          reject(FileStagingRejection::Kind::ExclusionConflict, *found);
          continue;
        }
      }
      candidate.append({kind, path, node->fileTarget});
      accepted = true;
    }
    result.changed = accepted;
  } else if (update == Update::Remove) {
    const auto& selected = selectedPaths;

    qsizetype removed = 0;
    for (const auto& path : selected) {
      const auto existing = std::ranges::find_if(entries, [&](const auto& entry) { return equalObjectNames(entry.path, path); });
      auto objectPath = existing == entries.end() ? path : fileStagingObjectPath(*existing);
      if (existing == entries.end()) {
        const auto aliases = normalizeFileStagingEntries({{FileStagingKind::File, path}}, source.get());
        if (!current()) return std::nullopt;
        objectPath = fileStagingObjectPath(aliases.front());
      }
      const auto identity = ObjectIdentity(std::move(objectPath));
      const auto displayIdentity = fileStagingEntryKey({FileStagingKind::File, path}).second;
      removed += candidate.removeIf([&](const FileStagingEntry& entry) {
        return ObjectIdentity(fileStagingObjectPath(entry)) == identity || fileStagingEntryKey(entry).second == displayIdentity;
      });
    }
    result.changed = removed > 0;
  }
  return result;
}
void FileStagingEditor::setStorageUnavailable(const QString& error, const std::uint64_t generation) {
  m_storageAvailable = false;
  m_storageError = error;
  const QPointer self(this);
  emit storageAvailabilityChanged();
  if (!self || self->m_loadGeneration != generation) return;
  emit storageUnavailable();
}

void FileStagingEditor::reportUpdateFailure(const QString& error) {
  UWF_LOG_E("staging") << "file staging update failed: error=" << error.toStdString();
  emit updateFailed(error);
}

void FileStagingEditor::reject(const FileStagingRejection::Kind kind, QString detail) { emit pathRejected({kind, std::move(detail)}); }

}  // namespace uwf::app
