// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "application/configuration/exclusions/ExclusionEditor.h"

#include <QFileInfo>
#include <algorithm>
#include <exception>
#include <utility>

#include "application/configuration/contracts/ConfigurationPlan.h"
#include "application/configuration/exclusions/ExclusionRules.h"
#include "application/files/contracts/FileBrowser.h"
#include "application/localization/ApplicationText.h"
#include "application/paths/ExclusionCoverage.h"
#include "application/paths/PathPolicy.h"
#include "application/staging/persistence/FileStagingRegistry.h"
#include "domain/uwf/contracts/Types.h"
#include "foundation/paths/DriveLetter.h"

namespace uwf::app {
namespace {
void sortList(QStringList& list) {
  std::stable_sort(list.begin(), list.end(), [](const QString& a, const QString& b) { return a.compare(b, Qt::CaseInsensitive) < 0; });
}
bool hasAbsoluteFileObjectAddress(const QString& path) {
  const auto address = paths::splitVolumeFilePath(path);
  return address && address->relativePath.startsWith(u'\\') && !address->relativePath.startsWith(QStringLiteral("\\\\"));
}
std::optional<ExclusionEditor::FileEntryKind> observeFileEntryKind(const std::shared_ptr<const FileBrowser>& files, const QString& path) {
  if (!files || !hasAbsoluteFileObjectAddress(path)) return std::nullopt;
  try {
    const auto entry = files->inspect(path);
    if (entry.exists) return entry.directory ? ExclusionEditor::FileEntryKind::Directory : ExclusionEditor::FileEntryKind::File;
  } catch (const std::exception&) {
    // Unavailable metadata cannot authorize a commit. Display can still use the
    // spelling heuristic below without probing a different filesystem object.
  }
  return std::nullopt;
}

}  // namespace

ExclusionEditor::ExclusionEditor(Kind kind, app::FileStagingConflictPolicy* conflicts, std::shared_ptr<const FileBrowser> files, QObject* parent)
    : ExclusionEditor(kind, std::make_shared<app::ExclusionDraft>(), conflicts, std::move(files), parent) {}

ExclusionEditor::ExclusionEditor(Kind kind, std::shared_ptr<app::ExclusionDraft> state, app::FileStagingConflictPolicy* conflicts,
                                 std::shared_ptr<const FileBrowser> files, QObject* parent)
    : QObject(parent), m_state(std::move(state)), m_draft(*m_state), m_kind(kind), m_conflicts(conflicts), m_files(std::move(files)) {
  if (m_kind == Kind::File) m_draft.setFileIdentityContext({}, {});
  if (m_conflicts) {
    connect(m_conflicts, &FileStagingConflictPolicy::stagedEntriesChanged, this, &ExclusionEditor::editingAvailabilityChanged);
    if (m_kind == Kind::File)
      m_conflicts->setFileExclusionSource([weak = std::weak_ptr(m_state)] {
        const auto state = weak.lock();
        return state ? state->effectiveFileExclusions() : QStringList{};
      });
  }
}
bool ExclusionEditor::canCommitFile(const QString& relativePath) const {
  const auto addressed = absoluteFilePath(relativePath);
  return m_kind == Kind::File && m_commitEnabled && m_files && hasAbsoluteFileObjectAddress(addressed) &&
         fileCommitSkipExclusion(m_draft.current(), addressed, m_draft.fileVolumes()).isEmpty();
}
ExclusionEditor::FileEntryKindOperation ExclusionEditor::fileEntryKindOperation() const {
  if (!m_files) return {};
  return [files = m_files](const QString& path) { return observeFileEntryKind(files, path); };
}
ExclusionEditor::FileEntryKind ExclusionEditor::fileEntryKindHint(const QString& path) {
  // QFileInfo::suffix is string parsing only; actual metadata always uses the
  // injected browser, which preserves the selected Windows object name.
  const bool directory = path.endsWith('\\') || path.endsWith('/') || QFileInfo(path).suffix().isEmpty();
  return directory ? FileEntryKind::Directory : FileEntryKind::File;
}

QString ExclusionEditor::absoluteFilePath(const QString& path) const { return m_kind == Kind::File ? qualifyFileExclusionPath(path, m_driveLetter) : path; }

QStringList ExclusionEditor::effectiveFileExclusions() const { return m_kind == Kind::File ? m_draft.effectiveFileExclusions() : QStringList{}; }

ExclusionEditor::Update::Update(ExclusionEditor& editor) : EditorUpdate(editor), m_readOnly(editor.readOnly()), m_canAdd(editor.canAdd()) {}
bool ExclusionEditor::Update::publishSnapshot(const std::function<bool()>& continuePublishing) {
  if (!publish(continuePublishing, [&] { emit m_editor->baselineChanged(); }, [&] { emit m_editor->stateChanged(); }, [&] { emit m_editor->pendingChanged(); }))
    return false;
  return publishAvailability(continuePublishing);
}
bool ExclusionEditor::Update::publishAvailability(const std::function<bool()>& continuePublishing) {
  if (!current(continuePublishing)) return false;
  if (m_readOnly != m_editor->readOnly() || m_canAdd != m_editor->canAdd())
    return publish(continuePublishing, [&] { emit m_editor->editingAvailabilityChanged(); });
  return current(continuePublishing);
}

void ExclusionEditor::setDriveLetter(const QString& dl) {
  m_driveLetter = QString::fromStdString(drive::normalize(dl.toStdString()));
  if (m_kind == Kind::File) {
    paths::VolumeIdentities volumes;
    if (!m_driveLetter.isEmpty() && !m_fileVolumeId.empty()) volumes.emplace(m_driveLetter, m_fileVolumeId);
    m_draft.setFileIdentityContext(std::move(volumes), m_fileVolumeId, m_driveLetter);
  }
  emit stateChanged();
}
void ExclusionEditor::setFileVolumeIdentity(const std::string& volumeId) {
  if (m_kind != Kind::File) return;
  m_fileVolumeId = compat::parseVolumeId(volumeId).value_or(std::string{});
  paths::VolumeIdentities volumes;
  if (!m_driveLetter.isEmpty() && !m_fileVolumeId.empty()) volumes.emplace(m_driveLetter, m_fileVolumeId);
  m_draft.setFileIdentityContext(std::move(volumes), m_fileVolumeId, m_driveLetter);
  emit stateChanged();
}
void ExclusionEditor::setSystemDriveLetter(const QString& dl) { m_systemDrive = QString::fromStdString(drive::normalize(dl.toStdString())); }

void ExclusionEditor::setBaseline(const QStringList& current, const QStringList& next) {
  m_draft.setBaseline(current, next);
  const QPointer self(this);
  emit baselineChanged();
  if (self) publishChanges();
}
void ExclusionEditor::setProviderBaseline(const QStringList& current, const QStringList& next) {
  const auto addressed = [&](const QStringList& patterns) {
    QStringList result;
    result.reserve(patterns.size());
    for (const auto& pattern : patterns) {
      // FileExceptionsUserDefined is raw per-volume MULTI_SZ (native RVA
      // 0x19ef0). Never infer an address from a provider pattern's contents.
      result.append(!m_driveLetter.isEmpty() ? m_driveLetter + pattern : !m_fileVolumeId.empty() ? paths::volumeFilePath({m_fileVolumeId, pattern}) : pattern);
    }
    return result;
  };
  setBaseline(addressed(current), addressed(next));
}

void ExclusionEditor::setPersistBaseline(bool domainSecretKeyCurrent, bool domainSecretKeyNext, bool tscalCurrent, bool tscalNext) {
  m_draft.domainSecretKey.baseCurrent = domainSecretKeyCurrent;
  m_draft.domainSecretKey.baseNext = domainSecretKeyNext;
  m_draft.domainSecretKey.pendingNext.reset();
  m_draft.tscal.baseCurrent = tscalCurrent;
  m_draft.tscal.baseNext = tscalNext;
  m_draft.tscal.pendingNext.reset();
  emit stateChanged();
}

void ExclusionEditor::resetPending() {
  m_draft.reset();
  publishChanges();
}

QStringList ExclusionEditor::pendingAdded() const {
  auto l = m_draft.added().values();
  sortList(l);
  return l;
}

QStringList ExclusionEditor::pendingRemoved() const {
  auto l = m_draft.removed().values();
  sortList(l);
  return l;
}

void ExclusionEditor::refreshFileStagingConstraints() {
  const QPointer self(this);
  const bool required = fileStagingRegistryRootRequired();
  if (!self) return;
  bool restored = false;
  if (required) {
    const auto removed = m_draft.removed();
    for (const auto& key : removed)
      if (isFileStagingRegistryRoot(key)) restored |= m_draft.add(key);
  }
  if (restored) {
    publishChanges();
  }
  if (self) emit removalAvailabilityChanged();
}

std::optional<bool> ExclusionEditor::pendingPersistDomainSecretKey() const { return m_draft.domainSecretKey.pendingNext; }

std::optional<bool> ExclusionEditor::pendingPersistTSCAL() const { return m_draft.tscal.pendingNext; }

ExclusionEditor::ImportOutcome ExclusionEditor::importRemove(const QString& raw) {
  if (m_readOnly) return ImportOutcome::RejectedReadOnly;
  QString p = raw;
  if (p.isEmpty()) return ImportOutcome::NoOp;
  if (m_kind == Kind::File) p = absoluteFilePath(p);

  // Enforce the durability dependency here as well as in selection-based removal.
  const QPointer self(this);
  const bool required = m_kind == Kind::Registry && isFileStagingRegistryRoot(p) && fileStagingRegistryRootRequired();
  if (!self || m_readOnly) return ImportOutcome::RejectedReadOnly;
  if (required) return ImportOutcome::RejectedRequired;

  const bool changed = m_draft.remove(p);
  if (!changed) return ImportOutcome::NoOp;
  publishChanges();
  return ImportOutcome::Applied;
}

bool ExclusionEditor::fileStagingRegistryRootRequired() const {
  if (m_kind != Kind::Registry || !m_fileStagingDataProbe) return false;
  const auto probe = m_fileStagingDataProbe;
  try {
    return probe();
  } catch (...) {
    return true;
  }
}

void ExclusionEditor::setReadOnly(bool value) {
  if (m_readOnly == value) return;
  m_readOnly = value;
  emit editingAvailabilityChanged();
}

void ExclusionEditor::setFileAddTargetAvailable(const bool available) {
  if (m_fileAddTargetAvailable == available) return;
  m_fileAddTargetAvailable = available;
  emit editingAvailabilityChanged();
}
QString ExclusionEditor::additionDisabledReason() const {
  return m_kind == Kind::File && !m_fileAddTargetAvailable ? QString::fromStdString(fileExclusionTargetRequirement()) : QString{};
}

void ExclusionEditor::setCommitEnabled(bool enabled) { m_commitEnabled = enabled; }

void ExclusionEditor::setFileStagingDataProbe(std::function<bool()> probe) {
  m_fileStagingDataProbe = std::move(probe);
  refreshFileStagingConstraints();
}

bool ExclusionEditor::canRemoveSelection(const QStringList& paths) const {
  if (m_readOnly) return false;
  if (m_kind != Kind::Registry || !m_fileStagingDataProbe) return true;
  const bool selectedRoot = std::ranges::any_of(paths, [](const QString& path) { return isFileStagingRegistryRoot(path); });
  return !selectedRoot || !fileStagingRegistryRootRequired();
}

bool ExclusionEditor::removeSelection(const QVector<Selection>& selection) {
  const auto selected = selection;
  QStringList paths;
  for (const auto& item : selected) paths.append(item.path);
  const QPointer self(this);
  if (!canRemoveSelection(paths)) return false;
  if (!self || m_readOnly) return false;
  for (const auto& item : selected) {
    if (item.persistence) {
      auto& flag = *item.persistence == PersistenceItem::DomainSecretKey ? m_draft.domainSecretKey : m_draft.tscal;
      m_draft.togglePersistence(flag);
    } else {
      static_cast<void>(m_draft.remove(m_kind == Kind::File ? absoluteFilePath(item.path) : item.path));
    }
  }
  publishChanges();
  return true;
}

bool ExclusionEditor::canEnablePersistence(const PersistenceItem item) const {
  const auto& flag = item == PersistenceItem::DomainSecretKey ? m_draft.domainSecretKey : m_draft.tscal;
  return m_kind == Kind::Registry && canAdd() && !flag.effectiveNext();
}
void ExclusionEditor::enablePersistence(PersistenceItem item) {
  if (!canEnablePersistence(item)) return;
  auto& flag = item == PersistenceItem::DomainSecretKey ? m_draft.domainSecretKey : m_draft.tscal;
  m_draft.enablePersistence(flag);
  publishChanges();
}
bool ExclusionEditor::canRestoreSelection(const QVector<Selection>& selection) const { return restorationResult(selection).outcome == ImportOutcome::Applied; }
ExclusionEditor::EditResult ExclusionEditor::restorationResult(const QVector<Selection>& selection) const { return validateRestoration(selection).result; }
ExclusionEditor::ValidatedRestoration ExclusionEditor::validateRestoration(const QVector<Selection>& selection) const {
  if (m_readOnly) return {{}, {.outcome = ImportOutcome::RejectedReadOnly, .path = {}, .reason = ApplicationText::tr("Read-only"), .conflict = {}}};
  if (selection.isEmpty()) return {};
  ValidatedRestoration validated;
  for (const auto& item : selection) {
    if (item.persistence)
      validated.selection.append(item);
    else {
      const auto path = m_kind == Kind::File ? absoluteFilePath(item.path) : item.path;
      const auto result = validateAddition(item.path, m_draft.isRemoved(path));
      if (result.outcome != ImportOutcome::Applied) return {{}, result};
      validated.selection.append({result.path, std::nullopt});
    }
  }
  validated.result.outcome = ImportOutcome::Applied;
  return validated;
}
bool ExclusionEditor::restoreSelection(const QVector<Selection>& selection) {
  const auto validated = validateRestoration(selection);
  if (validated.result.outcome != ImportOutcome::Applied) return false;
  for (const auto& item : validated.selection) {
    if (item.persistence)
      m_draft.enablePersistence(*item.persistence == PersistenceItem::DomainSecretKey ? m_draft.domainSecretKey : m_draft.tscal);
    else
      static_cast<void>(m_draft.add(item.path));
  }
  publishChanges();
  return true;
}

ExclusionEditor::EditResult ExclusionEditor::validateAddition(const QString& raw, const bool restoringConfigured) const {
  EditResult result;
  result.path = m_kind == Kind::File ? absoluteFilePath(raw) : raw;
  auto& path = result.path;
  if (m_readOnly) {
    result.outcome = ImportOutcome::RejectedReadOnly;
    result.reason = ApplicationText::tr("Read-only");
    return result;
  }
  if (path.isEmpty()) return result;
  if (m_kind == Kind::File) {
    if (!restoringConfigured && !m_fileAddTargetAvailable) {
      result.outcome = ImportOutcome::RejectedMissingVolume;
      result.reason = additionDisabledReason();
      return result;
    }
    // Undo preserves an observed pattern even when new-entry syntax or volume
    // creation would reject it. Cross-feature invariants still apply to both.
    if (!restoringConfigured) {
      const auto address = m_draft.fileAddress(path);
      if (!address || (!address->relativePath.isEmpty() && !paths::windowsSeparators(address->relativePath).startsWith(u'\\'))) {
        result.outcome = ImportOutcome::RejectedNotOnVolume;
        return result;
      }
      result.reason = app::FileExclusionRules::rejectionReason(m_driveLetter + address->relativePath, m_driveLetter, m_systemDrive);
    }
    if (!result.reason.isEmpty()) {
      result.outcome = ImportOutcome::RejectedForbidden;
      return result;
    }
    if (m_conflicts) {
      if (const auto conflict = m_conflicts->conflictingStagedPath(absoluteFilePath(path))) result.conflict = *conflict;
    }
  } else if (!restoringConfigured) {
    result.reason = app::RegistryExclusionRules::rejectionReason(path);
  }
  if (!result.reason.isEmpty())
    result.outcome = ImportOutcome::RejectedForbidden;
  else if (!result.conflict.isEmpty())
    result.outcome = ImportOutcome::RejectedConflict;
  else
    result.outcome = ImportOutcome::Applied;
  return result;
}

ExclusionEditor::EditResult ExclusionEditor::addSelectedPath(const QString& selectedPath) {
  // A selected object may use Qt's '/' representation. This conversion belongs
  // only to the new object-entry boundary, never to an observed pattern.
  auto result = validateAddition(m_kind == Kind::File ? paths::windowsSeparators(selectedPath) : selectedPath);
  if (result.outcome != ImportOutcome::Applied) return result;
  result.outcome = m_draft.add(result.path) ? ImportOutcome::Applied : ImportOutcome::NoOp;
  publishChanges();
  return result;
}

ExclusionEditor::ImportOutcome ExclusionEditor::importAdd(const QString& raw) {
  const auto result = validateAddition(raw);
  if (result.outcome != ImportOutcome::Applied) return result.outcome;
  if (!m_draft.add(result.path)) return ImportOutcome::NoOp;
  publishChanges();
  return ImportOutcome::Applied;
}

ExclusionEditor::BatchUpdate::BatchUpdate(ExclusionEditor& editor) : m_editor(&editor) { ++editor.m_batchDepth; }
ExclusionEditor::BatchUpdate::BatchUpdate(BatchUpdate&& other) noexcept : m_editor(std::exchange(other.m_editor, {})) {}
ExclusionEditor::BatchUpdate::~BatchUpdate() {
  if (m_editor) m_editor->finishBatch();
}
void ExclusionEditor::publishChanges() {
  if (m_batchDepth > 0) {
    m_batchChanged = true;
    return;
  }
  const QPointer self(this);
  emit stateChanged();
  if (self) emit pendingChanged();
}
void ExclusionEditor::finishBatch() {
  if (--m_batchDepth != 0 || !std::exchange(m_batchChanged, false)) return;
  const QPointer self(this);
  emit stateChanged();
  if (self) emit pendingChanged();
}

}  // namespace uwf::app

#include "moc_ExclusionEditor.cpp"
