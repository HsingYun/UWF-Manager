// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "ExclusionEditor.h"

#include <QFileInfo>
#include <algorithm>
#include <utility>

#include "app/configuration/ExclusionRules.h"
#include "app/paths/PathPolicy.h"
#include "app/ports/FileStagingStore.h"
#include "app/staging/FileStagingRegistry.h"
#include "shared/DriveLetter.h"
#include "shared/RegistryPath.h"

namespace uwf::app {
namespace {
QString normRegKey(const QString& key) { return QString::fromStdString(regkey::normalize(key.toStdString())); }

void sortList(QStringList& list) {
  std::stable_sort(list.begin(), list.end(), [](const QString& a, const QString& b) { return a.compare(b, Qt::CaseInsensitive) < 0; });
  list.erase(std::unique(list.begin(), list.end(), [](const QString& a, const QString& b) { return a.compare(b, Qt::CaseInsensitive) == 0; }), list.end());
}

}  // namespace

ExclusionEditor::ExclusionEditor(Kind kind, app::FileStagingConflictPolicy* conflicts, QObject* parent)
    : ExclusionEditor(kind, std::make_shared<app::ExclusionDraft>(), conflicts, parent) {}

ExclusionEditor::ExclusionEditor(Kind kind, std::shared_ptr<app::ExclusionDraft> state, app::FileStagingConflictPolicy* conflicts, QObject* parent)
    : QObject(parent), m_state(std::move(state)), m_draft(*m_state), m_kind(kind), m_conflicts(conflicts) {}

std::optional<ExclusionEditor::FileEntryKind> ExclusionEditor::fileCommitKind(const QString& relativePath) const {
  if (!m_commitEnabled || m_draft.current.contains(absoluteFilePath(relativePath), Qt::CaseInsensitive)) return std::nullopt;
  const QFileInfo info(absoluteFilePath(relativePath));
  if (!info.exists()) return std::nullopt;
  return info.isDir() ? FileEntryKind::Directory : FileEntryKind::File;
}
ExclusionEditor::FileEntryKind ExclusionEditor::fileEntryKind(const QString& relativePath) const {
  const QFileInfo info(absoluteFilePath(relativePath));
  const bool directory = info.exists() ? info.isDir() : relativePath.endsWith('\\') || relativePath.endsWith('/') || info.suffix().isEmpty();
  return directory ? FileEntryKind::Directory : FileEntryKind::File;
}

QString ExclusionEditor::absoluteFilePath(const QString& path) const {
  if (m_kind != Kind::File || path.isEmpty()) return path;
  return paths::normalize(path, m_driveLetter);
}

void ExclusionEditor::normalizeFileExclusions() {
  if (m_kind != Kind::File) return;
  for (auto* list : {&m_draft.current, &m_draft.next}) {
    for (auto& path : *list) path = absoluteFilePath(path);
    sortList(*list);
  }
  for (auto* set : {&m_draft.added, &m_draft.removed}) {
    QStringList normalized;
    normalized.reserve(set->size());
    for (const auto& path : std::as_const(*set)) normalized.append(absoluteFilePath(path));
    sortList(normalized);
    *set = QSet<QString>(normalized.begin(), normalized.end());
  }
}

QStringList ExclusionEditor::effectiveFileExclusions() const {
  if (m_kind != Kind::File) return {};
  QStringList result;
  const auto appendUnique = [&](const QString& path) {
    const QString absolute = absoluteFilePath(path);
    if (!result.contains(absolute, Qt::CaseInsensitive)) result.append(absolute);
  };
  // 当前会话排除项仍会影响本次关机提交，即使用户已把它标为下次会话移除；
  // 因此约束集合是 current 与“应用待改动后的 next”的并集。
  for (const auto& path : std::as_const(m_draft.current)) appendUnique(path);
  for (const auto& path : std::as_const(m_draft.next)) {
    if (!containsExclusion(m_draft.removed, path)) appendUnique(path);
  }
  for (const auto& path : std::as_const(m_draft.added)) appendUnique(path);
  return result;
}

void ExclusionEditor::publishFileExclusions() const {
  if (m_conflicts && m_kind == Kind::File) m_conflicts->setFileExclusions(effectiveFileExclusions());
}

void ExclusionEditor::setDriveLetter(const QString& dl) {
  m_driveLetter = QString::fromStdString(drive::normalize(dl.toStdString()));
  normalizeFileExclusions();
  publishFileExclusions();
  emit stateChanged();
}

void ExclusionEditor::setBaseline(const QStringList& current, const QStringList& next) {
  m_draft.current = current;
  m_draft.next = next;
  m_draft.added.clear();
  m_draft.removed.clear();
  // 注册表基线统一为完整 hive 名，避免简写与完整名称重复。
  if (m_kind == Kind::Registry) {
    for (QString& k : m_draft.current) k = normRegKey(k);
    for (QString& k : m_draft.next) k = normRegKey(k);
    sortList(m_draft.current);
    sortList(m_draft.next);
  } else
    normalizeFileExclusions();
  emit stateChanged();
  publishFileExclusions();
  emit pendingChanged();
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
  emit stateChanged();
  publishFileExclusions();
  emit pendingChanged();
}

QStringList ExclusionEditor::pendingAdded() const {
  auto l = m_draft.added.values();
  sortList(l);
  return l;
}

QStringList ExclusionEditor::pendingRemoved() const {
  auto l = m_draft.removed.values();
  sortList(l);
  return l;
}

void ExclusionEditor::refreshFileStagingConstraints() {
  const QString root = QString::fromLatin1(app::kFileStagingRegistryRoot.data(), static_cast<qsizetype>(app::kFileStagingRegistryRoot.size()));
  if (fileStagingRegistryRootRequired() && containsExclusion(m_draft.removed, root)) {
    removeExclusion(m_draft.removed, root);
    emit stateChanged();
    emit pendingChanged();
  }
  emit removalAvailabilityChanged();
}

std::optional<bool> ExclusionEditor::pendingPersistDomainSecretKey() const { return m_draft.domainSecretKey.pendingNext; }

std::optional<bool> ExclusionEditor::pendingPersistTSCAL() const { return m_draft.tscal.pendingNext; }

ExclusionEditor::ImportOutcome ExclusionEditor::importRemove(const QString& raw) {
  QString p = raw.trimmed();
  if (p.isEmpty()) return ImportOutcome::NoOp;
  if (m_kind == Kind::File)
    p = absoluteFilePath(p);
  else
    p = normRegKey(p);

  // 导入命令不经过列表选区和“移除所选”按钮，因此必须在
  // 状态写入边界单独执行同一持久性约束。
  if (m_kind == Kind::Registry && isFileStagingRegistryRoot(p) && fileStagingRegistryRootRequired()) return ImportOutcome::RejectedRequired;

  const bool changed = m_draft.remove(p);
  if (!changed) return ImportOutcome::NoOp;
  emit stateChanged();
  publishFileExclusions();
  emit pendingChanged();
  return ImportOutcome::Applied;
}

bool ExclusionEditor::fileStagingRegistryRootRequired() const {
  if (m_kind != Kind::Registry || !m_fileStagingDataProbe) return false;
  try {
    return m_fileStagingDataProbe();
  } catch (...) {
    return true;
  }
}

void ExclusionEditor::setReadOnly(bool value) {
  m_readOnly = value;
  emit readOnlyChanged();
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
  QStringList paths;
  for (const auto& item : selection) paths.append(item.path);
  if (!canRemoveSelection(paths)) return false;
  for (const auto& item : selection) {
    if (item.persistence) {
      auto& flag = *item.persistence == PersistenceItem::DomainSecretKey ? m_draft.domainSecretKey : m_draft.tscal;
      m_draft.togglePersistence(flag);
    } else {
      static_cast<void>(m_draft.remove(m_kind == Kind::File ? absoluteFilePath(item.path) : item.path));
    }
  }
  emit stateChanged();
  publishFileExclusions();
  emit pendingChanged();
  return true;
}

void ExclusionEditor::enablePersistence(PersistenceItem item) {
  if (m_readOnly) return;
  auto& flag = item == PersistenceItem::DomainSecretKey ? m_draft.domainSecretKey : m_draft.tscal;
  m_draft.enablePersistence(flag);
  emit stateChanged();
  emit pendingChanged();
}
bool ExclusionEditor::restoreSelection(const QVector<Selection>& selection) {
  if (m_readOnly || selection.isEmpty()) return false;
  QVector<Selection> validated;
  for (const auto& item : selection) {
    if (item.persistence) validated.append(item);
    else {
      const auto result = validateAddition(item.path);
      if (result.outcome != ImportOutcome::Applied) return false;
      validated.append({result.path,std::nullopt});
    }
  }
  for (const auto& item : std::as_const(validated)) {
    if (item.persistence) m_draft.enablePersistence(*item.persistence == PersistenceItem::DomainSecretKey ? m_draft.domainSecretKey : m_draft.tscal);
    else static_cast<void>(m_draft.add(item.path));
  }
  emit stateChanged();
  publishFileExclusions();
  emit pendingChanged();
  return true;
}

ExclusionEditor::EditResult ExclusionEditor::validateAddition(const QString& raw) const {
  EditResult result;
  result.path = raw.trimmed();
  auto& path = result.path;
  if (path.isEmpty()) return result;
  if (m_kind == Kind::File) {
    path = absoluteFilePath(path);
    if (!m_driveLetter.isEmpty() && !path.startsWith(m_driveLetter + "\\", Qt::CaseInsensitive) && path.compare(m_driveLetter, Qt::CaseInsensitive) != 0) {
      result.outcome = ImportOutcome::RejectedNotOnVolume;
      return result;
    }
    result.reason = app::FileExclusionRules::rejectionReason(path, m_driveLetter);
    if (!result.reason.isEmpty()) {
      result.outcome = ImportOutcome::RejectedForbidden;
      return result;
    }
    if (m_conflicts) {
      if (const auto conflict = m_conflicts->conflictingStagedPath(absoluteFilePath(path))) result.conflict = *conflict;
    }
  } else {
    path = normRegKey(path);
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

ExclusionEditor::EditResult ExclusionEditor::addEntry(const QString& raw) {
  auto result = validateAddition(raw);
  if (result.outcome != ImportOutcome::Applied) return result;
  result.outcome = m_draft.add(result.path) ? ImportOutcome::Applied : ImportOutcome::NoOp;
  emit stateChanged();
  publishFileExclusions();
  emit pendingChanged();
  return result;
}

ExclusionEditor::ImportOutcome ExclusionEditor::importAdd(const QString& raw) {
  const auto result = validateAddition(raw);
  if (result.outcome != ImportOutcome::Applied) return result.outcome;
  if (!m_draft.add(result.path)) return ImportOutcome::NoOp;
  emit stateChanged();
  publishFileExclusions();
  emit pendingChanged();
  return ImportOutcome::Applied;
}

}  // namespace uwf::app

#include "moc_ExclusionEditor.cpp"
