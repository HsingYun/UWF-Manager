// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "FileStagingViewModel.h"

#include <algorithm>
#include <QSet>
#include "app/staging/FileStagingPath.h"

#include "presentation/common/ItemRoles.h"
#include "presentation/common/PresentationText.h"

namespace uwf::presentation {

FileStagingViewModel::FileStagingViewModel(std::shared_ptr<app::FileStagingEditing> editing) : m_editing(std::move(editing)) {
  rebuildEntries();
  connect(m_editing.get(), &app::FileStagingEditing::entriesChanged, this, [this] {
    beginResetModel();
    rebuildEntries();
    endResetModel();
    emit entriesChanged();
  });
  connect(m_editing.get(), &app::FileStagingEditing::storageUnavailable, this, &FileStagingViewModel::storageUnavailable);
  connect(m_editing.get(), &app::FileStagingEditing::storageAvailabilityChanged, this, [this] {
    emit storageAvailabilityChanged();
    emit modificationAvailabilityChanged();
  });
  connect(m_editing.get(), &app::FileStagingEditing::pathRejected, this, &FileStagingViewModel::reportRejection);
  connect(m_editing.get(), &app::FileStagingEditing::updateFailed, this, [this](const QString& error) {
    emit warningRaised(PresentationText::tr("File staging could not be saved"),
                       PresentationText::tr("The file staging state was reloaded after the update could not be completed:\n%1").arg(error));
  });
}

void FileStagingViewModel::rebuildEntries() {
  m_entries = m_editing->entries();
  const auto baseline = m_editing->baselineEntries();
  QSet<std::pair<app::FileStagingKind, QString>> original;
  original.reserve(baseline.size());
  for (const auto& entry : baseline) original.insert(app::fileStagingEntryKey(entry));
  m_rows.clear();
  for (const auto removed = app::FileStagingChanges{baseline, m_entries}.removed(); const auto& entry : removed)
    m_rows.append({entry, ItemStatus::Removed});
  for (const auto& entry : std::as_const(m_entries))
    m_rows.append({entry, original.contains(app::fileStagingEntryKey(entry)) ? ItemStatus::Normal : ItemStatus::Added});
}
int FileStagingViewModel::rowCount(const QModelIndex& parent) const { return parent.isValid() ? 0 : static_cast<int>(m_rows.size()); }
QVariant FileStagingViewModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid() || index.model() != this || index.column() != 0 || index.row() < 0 || index.row() >= rowCount()) return {};
  const auto& row = m_rows[index.row()];
  const auto& entry = row.entry;
  const bool directory = entry.kind == app::FileStagingKind::Directory;
  if (role == Qt::DisplayRole || role == PathRole) return entry.path;
  if (role == DirectoryRole) return directory;
  if (role == StatusRole) return static_cast<int>(row.status);
  if (role == BadgeRole)
    return static_cast<int>(row.status == ItemStatus::Removed ? ItemBadge::Remove : row.status == ItemStatus::Added ? ItemBadge::Add : ItemBadge::None);
  if (role == IconResourceRole) return directory ? QStringLiteral(":/icons/folder.svg") : QStringLiteral(":/icons/file.svg");
  if (role == Qt::ToolTipRole)
    return (directory ? PresentationText::tr("Folder: %1") : PresentationText::tr("File: %1")).arg(entry.path) + '\n' +
           PresentationText::tr("Committed automatically before safe shutdown or restart.");
  return {};
}
void FileStagingViewModel::load() { m_editing->load(); }
void FileStagingViewModel::retranslate() {
  if (rowCount() > 0) emit dataChanged(index(0), index(rowCount() - 1), {Qt::ToolTipRole});
}
QString FileStagingViewModel::summaryText() const {
  const auto directories = std::ranges::count_if(m_entries, [](const auto& entry) { return entry.kind == app::FileStagingKind::Directory; });
  return PresentationText::tr("%1 staged entries · %2 files · %3 folders").arg(m_entries.size()).arg(m_entries.size() - directories).arg(directories);
}

void FileStagingViewModel::setReadOnly(const bool value) {
  if (m_readOnly == value) return;
  m_readOnly = value;
  emit modificationAvailabilityChanged();
}

void FileStagingViewModel::setEnhancedModeAvailable(const bool value) {
  if (m_enhancedModeAvailable == value) return;
  m_enhancedModeAvailable = value;
  emit enhancedModeAvailabilityChanged();
}

void FileStagingViewModel::addEntries(const QStringList& paths, const app::FileStagingKind kind) {
  if (writable()) m_editing->addEntries(paths, kind);
}

void FileStagingViewModel::removePaths(const QStringList& paths) {
  if (writable() && !paths.isEmpty()) m_editing->removePaths(paths);
}
void FileStagingViewModel::restorePaths(const QStringList& paths) {
  if (writable() && !paths.isEmpty()) m_editing->restorePaths(paths);
}

void FileStagingViewModel::reportRejection(const app::FileStagingRejection& rejection) {
  using Kind = app::FileStagingRejection::Kind;
  const auto& detail = rejection.detail;
  QString body;
  switch (rejection.kind) {
    case Kind::NonLocalPath:
      body = PresentationText::tr("Only absolute paths on local volumes can be added to automatic file staging:\n%1").arg(detail);
      break;
    case Kind::InspectionFailed:
      body = PresentationText::tr("The selected path could not be inspected:\n%1").arg(detail);
      break;
    case Kind::ReparsePoint:
      body = PresentationText::tr("Reparse points cannot be added to automatic file staging:\n%1").arg(detail);
      break;
    case Kind::WrongType:
      body = PresentationText::tr("The selected path does not exist or is not the expected file type:\n%1").arg(detail);
      break;
    case Kind::MissingDrive:
      body = PresentationText::tr("The path has no drive letter and cannot be committed by UWF:\n%1").arg(detail);
      break;
    case Kind::WrongDrive:
      body = PresentationText::tr("The selected path is on volume %1. Add it from that volume's File staging tab.").arg(detail);
      break;
    case Kind::ResolutionFailed:
      body = PresentationText::tr("Failed to resolve the target volume: %1").arg(detail);
      break;
    case Kind::DriveRoot:
      body = PresentationText::tr("A volume root cannot be staged because it would recursively commit the entire volume.");
      break;
    case Kind::ExclusionConflict:
      body = PresentationText::tr("This path overlaps a UWF file exclusion. Remove the exclusion before adding the staged path:\n%1").arg(detail);
      break;
  }
  emit warningRaised(PresentationText::tr("Cannot stage this path"), body);
}

}  // namespace uwf::presentation

#include "moc_FileStagingViewModel.cpp"
