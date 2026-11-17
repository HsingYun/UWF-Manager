// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "presentation/viewmodels/configuration/ExclusionListViewModel.h"

#include <QDir>
#include <QHash>
#include <QPointer>
#include <algorithm>
#include <utility>

#include "application/staging/persistence/FileStagingRegistry.h"
#include "presentation/models/ItemRoles.h"
#include "presentation/viewmodels/common/PresentationText.h"

namespace uwf::presentation {
ExclusionListViewModel::ExclusionListViewModel(Kind kind, app::FileStagingConflictPolicy* conflicts, QObject* parent)
    : ExclusionListViewModel(std::make_shared<app::ExclusionEditor>(kind, conflicts), parent) {}
ExclusionListViewModel::ExclusionListViewModel(Kind kind, std::shared_ptr<app::ExclusionDraft> state, app::FileStagingConflictPolicy* conflicts,
                                               QObject* parent)
    : ExclusionListViewModel(std::make_shared<app::ExclusionEditor>(kind, std::move(state), conflicts), parent) {}
ExclusionListViewModel::ExclusionListViewModel(std::shared_ptr<app::ExclusionEditor> editor, QObject* parent)
    : QAbstractListModel(parent), m_editor(std::move(editor)) {
  m_metadataTimer.setSingleShot(true);
  connect(&m_metadataTimer, &QTimer::timeout, this, &ExclusionListViewModel::readFileMetadata);
  connect(m_editor.get(), &app::ExclusionEditor::baselineChanged, this, &ExclusionListViewModel::invalidateFileMetadata);
  connect(m_editor.get(), &app::ExclusionEditor::stateChanged, this, &ExclusionListViewModel::rebuildRows);
  connect(m_editor.get(), &app::ExclusionEditor::editingAvailabilityChanged, this, &ExclusionListViewModel::editingAvailabilityChanged);
  connect(m_editor.get(), &app::ExclusionEditor::removalAvailabilityChanged, this, &ExclusionListViewModel::removalAvailabilityChanged);
  rebuildRows();
}

QString ExclusionListViewModel::additionHint() const {
  if (readOnly()) return PresentationText::tr("Read-only");
  if (const auto reason = additionDisabledReason(); !reason.isEmpty()) return reason;
  return kind() == Kind::File ? PresentationText::tr("Add a file or folder to the exclusion list.")
                              : PresentationText::tr("Add a registry key to the exclusion list, or enable a persistence switch.");
}

ExclusionListViewModel::FileCommitState ExclusionListViewModel::fileCommitState(const QString& relativePath) const {
  FileCommitState state;
  if (kind() != Kind::File) return state;
  const auto path = absoluteFilePath(relativePath);
  const auto found = m_fileKinds.constFind(path);
  const auto observed = found == m_fileKinds.cend() ? std::nullopt : found.value();
  state.kind = observed.value_or(app::ExclusionEditor::fileEntryKindHint(path));
  const bool admitted = m_editor->canCommitFile(relativePath);
  state.visible = !m_commitRestriction.isEmpty() || (admitted && (found == m_fileKinds.cend() || observed.has_value()));
  state.enabled = admitted && observed.has_value();
  return state;
}
QString ExclusionListViewModel::restorationRestriction(const QVector<Selection>& selection) const {
  const auto result = m_editor->restorationResult(selection);
  if (result.outcome == ImportOutcome::Applied) return {};
  if (result.outcome == ImportOutcome::RejectedConflict)
    return PresentationText::tr("This path overlaps a file staging entry. Remove the staged path before adding the exclusion:\n%1").arg(result.conflict);
  return result.reason.isEmpty() ? PresentationText::tr("Selected entries cannot currently be restored.") : result.reason;
}

int ExclusionListViewModel::rowCount(const QModelIndex& parent) const { return parent.isValid() ? 0 : static_cast<int>(m_rows.size()); }

QVariant ExclusionListViewModel::data(const QModelIndex& index, const int role) const {
  if (!index.isValid() || index.model() != this || index.column() != 0 || index.row() < 0 || index.row() >= m_rows.size()) return {};
  const auto& row = m_rows.at(index.row());
  switch (role) {
    case Qt::DisplayRole:
    case PathRole:
      return row.persistence ? (*row.persistence == PersistenceItem::DomainSecretKey ? PresentationText::tr("Domain Secret Key persistence")
                                                                                     : PresentationText::tr("Terminal Services license persistence"))
                             : row.path;
    case Qt::ToolTipRole: {
      auto translated = row;
      translateRow(translated);
      return translated.tooltip;
    }
    case IconResourceRole:
      return row.icon;
    case StatusRole:
      return static_cast<int>(row.status);
    case StatusTextRole:
      return kind() == Kind::Registry && app::isFileStagingRegistryRoot(row.path)      ? PresentationText::tr("Required configuration")
             : row.status == ItemStatus::Removed                                       ? PresentationText::tr("Pending removal")
             : row.status == ItemStatus::Added                                         ? PresentationText::tr("Added")
             : !row.persistence && !m_currentSessionExclusionsEffective && row.current ? PresentationText::tr("Inactive this session")
                                                                                       : PresentationText::tr("Configured");
    case BadgeRole:
      return static_cast<int>(row.badge);
    case RequiredRole:
      return kind() == Kind::Registry && app::isFileStagingRegistryRoot(row.path);
    case PersistenceRole:
      return row.persistence ? QVariant(static_cast<int>(*row.persistence)) : QVariant();
    default:
      return {};
  }
}

void ExclusionListViewModel::setCurrentSessionExclusionsEffective(const bool effective) {
  if (m_currentSessionExclusionsEffective == effective) return;
  m_currentSessionExclusionsEffective = effective;
  if (!m_rows.isEmpty()) emit dataChanged(index(0), index(rowCount() - 1), {Qt::ToolTipRole, StatusTextRole});
}

void ExclusionListViewModel::setCommitAvailability(const bool enabled, QString reason) {
  if (enabled) reason.clear();
  if (m_editor->commitEnabled() == enabled && m_commitRestriction == reason) return;
  m_commitRestriction = std::move(reason);
  m_editor->setCommitEnabled(enabled);
  emit commitAvailabilityChanged();
}

void ExclusionListViewModel::retranslate() {
  if (!m_rows.isEmpty()) emit dataChanged(index(0), index(rowCount() - 1), {Qt::DisplayRole, Qt::ToolTipRole, PathRole, StatusTextRole});
}

void ExclusionListViewModel::rebuildRows() {
  m_rebuildPending = true;
  if (m_rebuildingRows) return;
  m_rebuildingRows = true;
  const QPointer self(this);
  const auto editor = m_editor;
  while (m_rebuildPending) {
    m_rebuildPending = false;
    const auto& state = draft();
    QHash<QString, qsizetype> positions;
    QList<Row> all;
    const auto collect = [&](const auto& paths, bool Row::* membership) {
      for (const auto& path : paths) {
        const auto key = state.identityKey(path);
        auto position = positions.find(key);
        if (position == positions.end()) {
          position = positions.insert(key, all.size());
          Row row;
          row.path = path;
          all.append(std::move(row));
        }
        all[*position].*membership = true;
      }
    };
    // Keep the first spelling while collecting all session and draft membership.
    collect(state.added(), &Row::userAdded);
    collect(state.removed(), &Row::userRemoved);
    collect(state.current(), &Row::current);
    collect(state.next(), &Row::next);
    QList<Row> pending, rest;
    for (const auto& row : std::as_const(all)) {
      const bool modified = row.userAdded || row.userRemoved || row.current != row.next;
      (modified ? pending : rest).append(row);
    }
    const auto sort = [](QList<Row>& rows) {
      std::ranges::sort(rows, [](const Row& a, const Row& b) { return a.path.compare(b.path, Qt::CaseInsensitive) < 0; });
    };
    sort(pending);
    sort(rest);
    const QString domainName = PresentationText::tr("Domain Secret Key persistence");
    const QString tscalName = PresentationText::tr("Terminal Services license persistence");
    QList<Row> pseudoPending, pseudoRest;
    if (kind() == Kind::Registry) {
      for (const auto* flag : {&state.domainSecretKey, &state.tscal}) {
        if (!flag->visible()) continue;
        const bool modified = flag->pendingNext.has_value() || flag->baseCurrent != flag->baseNext;
        Row row;
        row.path = flag == &state.domainSecretKey ? domainName : tscalName;
        (modified ? pseudoPending : pseudoRest).append(std::move(row));
      }
    }
    const auto display = pseudoPending + pending + pseudoRest + rest;
    QList<Row> rows;
    rows.reserve(display.size());
    for (auto row : display) {
      const app::PersistenceFlagDraft* flag = nullptr;
      if (kind() == Kind::Registry) {
        if (row.path == domainName) {
          flag = &state.domainSecretKey;
          row.persistence = PersistenceItem::DomainSecretKey;
        } else if (row.path == tscalName) {
          flag = &state.tscal;
          row.persistence = PersistenceItem::Tscal;
        }
      }
      const bool inCurrent = flag ? flag->baseCurrent : row.current;
      const bool inNext = flag ? flag->baseNext : row.next;
      const bool userAdded = flag ? flag->pendingNext == true : row.userAdded;
      const bool userRemoved = flag ? flag->pendingNext == false : row.userRemoved;
      row.current = inCurrent;
      row.next = inNext;
      row.userAdded = userAdded;
      row.userRemoved = userRemoved;
      if (userAdded || userRemoved) {
        row.badge = userAdded ? ItemBadge::Add : ItemBadge::Remove;
      } else if (!inCurrent && inNext) {
        row.badge = ItemBadge::Add;
      } else if (inCurrent && !inNext) {
        row.badge = ItemBadge::Remove;
      }
      row.status = row.badge == ItemBadge::Add ? ItemStatus::Added : row.badge == ItemBadge::Remove ? ItemStatus::Removed : ItemStatus::Normal;
      const auto path = absoluteFilePath(row.path);
      const bool directory = kind() == Kind::File &&
                             m_fileKinds.value(path).value_or(app::ExclusionEditor::fileEntryKindHint(path)) == app::ExclusionEditor::FileEntryKind::Directory;
      row.icon = kind() == Kind::File             ? (directory ? ":/icons/folder.svg" : ":/icons/file.svg")
                 : flag == &state.domainSecretKey ? ":/icons/key.svg"
                 : flag == &state.tscal           ? ":/icons/license.svg"
                                                  : ":/icons/registry.svg";
      rows.append(std::move(row));
    }
    const auto rank = [this](const Row& row) {
      const bool stagingRoot = kind() == Kind::Registry && app::isFileStagingRegistryRoot(row.path);
      const int status = row.status == ItemStatus::Removed ? 0 : row.status == ItemStatus::Added ? 1 : 2;
      return std::pair{stagingRoot, status};
    };
    std::stable_sort(rows.begin(), rows.end(), [&](const Row& a, const Row& b) { return rank(a) < rank(b); });
    beginResetModel();
    if (!self) return;
    m_rows = std::move(rows);
    endResetModel();
    if (!self) return;
    queueFileMetadata();
    if (!self) return;
    emit stateChanged();
    if (!self) return;
  }
  m_rebuildingRows = false;
}

void ExclusionListViewModel::invalidateFileMetadata() {
  ++m_metadataGeneration;
  m_metadata.cancel();
  m_metadataTimer.stop();
  m_fileKinds.clear();
  m_pendingMetadata.clear();
  m_inflightMetadata.clear();
  emit commitAvailabilityChanged();
}
void ExclusionListViewModel::queueFileMetadata() {
  if (kind() != Kind::File || !m_editor->fileEntryKindOperation()) return;
  QSet<QString> active;
  for (const auto& row : std::as_const(m_rows)) active.insert(absoluteFilePath(row.path));
  // Keep observations for this baseline when a draft removal is reversed.
  // A new baseline invalidates both display metadata and menu admission.
  m_pendingMetadata.intersect(active);
  for (const auto& path : std::as_const(active))
    if (!m_fileKinds.contains(path) && !m_inflightMetadata.contains(path)) m_pendingMetadata.insert(path);
  // One event-turn collects all synchronous additions before starting I/O.
  if (!m_pendingMetadata.isEmpty() && !m_metadata.isRunning()) m_metadataTimer.start(0);
}
void ExclusionListViewModel::readFileMetadata() {
  if (m_metadata.isRunning() || m_pendingMetadata.isEmpty()) return;
  auto read = m_editor->fileEntryKindOperation();
  if (!read) return;
  const auto paths = std::exchange(m_pendingMetadata, {});
  m_inflightMetadata = paths;
  const auto generation = m_metadataGeneration;
  m_metadata.start(
      [paths, read = std::move(read)](std::stop_token stop) {
        FileKinds kinds;
        for (const auto& path : paths) {
          if (stop.stop_requested()) break;
          kinds.insert(path, read(path));
        }
        return kinds;
      },
      [this, generation, paths](AsyncResult<FileKinds> result) {
        if (generation != m_metadataGeneration) return;
        m_inflightMetadata.clear();
        const auto* observed = std::get_if<FileKinds>(&result);
        int firstChanged = -1, lastChanged = -1;
        for (int i = 0; i < m_rows.size(); ++i) {
          auto& row = m_rows[i];
          const auto path = absoluteFilePath(row.path);
          if (!paths.contains(path)) continue;
          const auto hint = app::ExclusionEditor::fileEntryKindHint(path);
          const auto observation = observed ? observed->value(path) : std::nullopt;
          m_fileKinds.insert(path, observation);
          const auto kind = observation.value_or(hint);
          const auto icon = kind == app::ExclusionEditor::FileEntryKind::Directory ? QStringLiteral(":/icons/folder.svg") : QStringLiteral(":/icons/file.svg");
          if (row.icon == icon) continue;
          row.icon = icon;
          if (firstChanged < 0) firstChanged = i;
          lastChanged = i;
        }
        const QPointer self(this);
        if (firstChanged >= 0) emit dataChanged(index(firstChanged), index(lastChanged), {IconResourceRole, Qt::ToolTipRole});
        if (!self || generation != m_metadataGeneration) return;
        emit commitAvailabilityChanged();
        if (self && generation == m_metadataGeneration) queueFileMetadata();
      });
}

void ExclusionListViewModel::translateRow(Row& row) const {
  if (row.persistence) {
    row.path = *row.persistence == PersistenceItem::DomainSecretKey ? PresentationText::tr("Domain Secret Key persistence")
                                                                    : PresentationText::tr("Terminal Services license persistence");
    row.tooltip = PresentationText::tr("Excluded by default; change it anytime.") + '\n';
  } else {
    const QString full = kind() == Kind::File ? absoluteFilePath(row.path) : row.path;
    if (kind() == Kind::File)
      row.tooltip = (row.icon == QStringLiteral(":/icons/folder.svg") ? PresentationText::tr("Folder: %1") : PresentationText::tr("File: %1")).arg(full) + '\n';
    else if (app::isFileStagingRegistryRoot(full))
      row.tooltip = PresentationText::tr("File staging relies on this registry exclusion to preserve its list across restarts.") + '\n';
    else
      row.tooltip = PresentationText::tr("Registry: %1").arg(full) + '\n';
  }
  const auto excluded = PresentationText::tr("Excluded");
  const auto notExcluded = PresentationText::tr("Not excluded");
  const auto currentText = row.current && !row.persistence && !m_currentSessionExclusionsEffective ? PresentationText::tr("Configured, inactive this session")
                           : row.current                                                           ? excluded
                                                                                                   : notExcluded;
  row.tooltip += '\n' + PresentationText::tr("Current session: %1\nNext session: %2").arg(currentText, row.next ? excluded : notExcluded);
  if (!row.persistence && !m_currentSessionExclusionsEffective && row.current)
    row.tooltip += '\n' + PresentationText::tr("This exclusion is configured but does not bypass protection in the current session.");
  if (row.userAdded || row.userRemoved)
    row.tooltip += '\n' + PresentationText::tr("Pending change: %1").arg(row.userAdded ? PresentationText::tr("Add") : PresentationText::tr("Remove"));
}
}  // namespace uwf::presentation

#include "moc_ExclusionListViewModel.cpp"
