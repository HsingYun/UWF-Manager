// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "ExclusionListViewModel.h"

#include <QDir>
#include <algorithm>

#include "app/staging/FileStagingRegistry.h"
#include "presentation/common/ItemRoles.h"
#include "presentation/common/PresentationText.h"

namespace uwf::presentation {
ExclusionListViewModel::ExclusionListViewModel(Kind kind, app::FileStagingConflictPolicy* conflicts, QObject* parent)
    : ExclusionListViewModel(std::make_shared<app::ExclusionEditor>(kind, conflicts), parent) {}
ExclusionListViewModel::ExclusionListViewModel(Kind kind, std::shared_ptr<app::ExclusionDraft> state, app::FileStagingConflictPolicy* conflicts,
                                               QObject* parent)
    : ExclusionListViewModel(std::make_shared<app::ExclusionEditor>(kind, std::move(state), conflicts), parent) {}
ExclusionListViewModel::ExclusionListViewModel(std::shared_ptr<app::ExclusionEditor> editor, QObject* parent)
    : QAbstractListModel(parent), m_editor(std::move(editor)) {
  connect(m_editor.get(), &app::ExclusionEditor::stateChanged, this, [this] {
    rebuildRows();
    emit stateChanged();
  });
  connect(m_editor.get(), &app::ExclusionEditor::pendingChanged, this, &ExclusionListViewModel::pendingChanged);
  connect(m_editor.get(), &app::ExclusionEditor::readOnlyChanged, this, &ExclusionListViewModel::readOnlyChanged);
  connect(m_editor.get(), &app::ExclusionEditor::removalAvailabilityChanged, this, &ExclusionListViewModel::removalAvailabilityChanged);
  rebuildRows();
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

void ExclusionListViewModel::retranslate() {
  if (!m_rows.isEmpty()) emit dataChanged(index(0), index(rowCount() - 1), {Qt::DisplayRole, Qt::ToolTipRole, PathRole});
}

void ExclusionListViewModel::rebuildRows() {
  const auto& state = draft();
  QSet<QString> seen;
  QStringList all;
  const auto pushUnique = [&](const QString& path) {
    const auto key = path.toLower();
    if (seen.contains(key)) return;
    seen.insert(key);
    all.append(path);
  };
  for (const auto& path : state.added) pushUnique(path);
  for (const auto& path : state.removed) pushUnique(path);
  for (const auto& path : state.current) pushUnique(path);
  for (const auto& path : state.next) pushUnique(path);
  QStringList pending, rest;
  for (const auto& path : std::as_const(all)) {
    const bool modified = state.added.contains(path) || state.removed.contains(path) ||
                          state.current.contains(path, Qt::CaseInsensitive) != state.next.contains(path, Qt::CaseInsensitive);
    (modified ? pending : rest).append(path);
  }
  const auto sort = [](QStringList& paths) {
    std::ranges::sort(paths, [](const QString& a, const QString& b) { return a.compare(b, Qt::CaseInsensitive) < 0; });
    paths.removeDuplicates();
  };
  sort(pending);
  sort(rest);
  const QString domainName = PresentationText::tr("Domain Secret Key persistence");
  const QString tscalName = PresentationText::tr("Terminal Services license persistence");
  QStringList pseudoPending, pseudoRest;
  if (kind() == Kind::Registry) {
    for (const auto* flag : {&state.domainSecretKey, &state.tscal}) {
      if (!flag->visible()) continue;
      const bool modified = flag->pendingNext.has_value() || flag->baseCurrent != flag->baseNext;
      (modified ? pseudoPending : pseudoRest).append(flag == &state.domainSecretKey ? domainName : tscalName);
    }
  }
  const auto display = pseudoPending + pending + pseudoRest + rest;
  QList<Row> rows;
  rows.reserve(display.size());
  for (const auto& entry : display) {
    Row row;
    row.path = entry;
    const app::PersistenceFlagDraft* flag = nullptr;
    if (kind() == Kind::Registry) {
      if (entry == domainName) {
        flag = &state.domainSecretKey;
        row.persistence = PersistenceItem::DomainSecretKey;
      } else if (entry == tscalName) {
        flag = &state.tscal;
        row.persistence = PersistenceItem::Tscal;
      }
    }
    const bool inCurrent = flag ? flag->baseCurrent : state.current.contains(entry, Qt::CaseInsensitive);
    const bool inNext = flag ? flag->baseNext : state.next.contains(entry, Qt::CaseInsensitive);
    const bool userAdded = flag ? flag->pendingNext == true : state.added.contains(entry);
    const bool userRemoved = flag ? flag->pendingNext == false : state.removed.contains(entry);
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
    const bool directory = kind() == Kind::File && m_editor->fileEntryKind(entry) == app::ExclusionEditor::FileEntryKind::Directory;
    row.icon = kind() == Kind::File             ? (directory ? ":/icons/folder.svg" : ":/icons/file.svg")
               : flag == &state.domainSecretKey ? ":/icons/key.svg"
               : flag == &state.tscal           ? ":/icons/license.svg"
                                                : ":/icons/registry.svg";
    rows.append(std::move(row));
  }
  const auto rank = [](const Row& row) { return row.status == ItemStatus::Removed ? 0 : row.status == ItemStatus::Added ? 1 : 2; };
  std::stable_sort(rows.begin(), rows.end(), [&](const Row& a, const Row& b) { return rank(a) < rank(b); });
  beginResetModel();
  m_rows = std::move(rows);
  endResetModel();
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
  row.tooltip +=
      '\n' + PresentationText::tr("Current session: %1\nNext session: %2").arg(row.current ? excluded : notExcluded, row.next ? excluded : notExcluded);
  if (row.userAdded || row.userRemoved)
    row.tooltip += '\n' + PresentationText::tr("Pending change: %1").arg(row.userAdded ? PresentationText::tr("Add") : PresentationText::tr("Remove"));
}
}  // namespace uwf::presentation

#include "moc_ExclusionListViewModel.cpp"
