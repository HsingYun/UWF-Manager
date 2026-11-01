// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "presentation/viewmodels/staging/FileStagingViewModel.h"

#include <QPointer>
#include <algorithm>
#include <set>
#include <utility>

#include "application/staging/editing/FileStagingPath.h"
#include "domain/uwf/policies/VolumeIdentityPolicy.h"
#include "presentation/models/ItemRoles.h"
#include "presentation/viewmodels/common/AutomaticCommitText.h"
#include "presentation/viewmodels/common/PresentationText.h"
#include "presentation/viewmodels/common/RuntimeWriteText.h"

namespace uwf::presentation {

FileStagingViewModel::FileStagingViewModel(std::shared_ptr<app::FileStagingEditing> editing) : m_editing(std::move(editing)) {
  rebuildEntries();
  connect(m_editing.get(), &app::FileStagingEditing::entriesChanged, this, &FileStagingViewModel::rebuildEntries);
  connect(m_editing.get(), &app::FileStagingEditing::storageUnavailable, this, &FileStagingViewModel::storageUnavailable);
  connect(m_editing.get(), &app::FileStagingEditing::storageAvailabilityChanged, this, [this] {
    const QPointer self(this);
    emit storageAvailabilityChanged();
    if (self) emit modificationAvailabilityChanged();
  });
  connect(m_editing.get(), &app::FileStagingEditing::pathRejected, this, &FileStagingViewModel::reportRejection);
  connect(m_editing.get(), &app::FileStagingEditing::busyChanged, this, &FileStagingViewModel::modificationAvailabilityChanged);
  connect(m_editing.get(), &app::FileStagingEditing::updateFailed, this, [this](const QString& error) {
    emit warningRaised(PresentationText::tr("File staging could not be saved"),
                       PresentationText::tr("The file staging state was reloaded after the update could not be completed:\n%1").arg(error));
  });
}

FileStagingViewModel::~FileStagingViewModel() {
  disconnect(m_editing.get(), nullptr, this, nullptr);
  m_preparation.cancel();
  if (m_cancelPreparation) m_cancelPreparation();
}

void FileStagingViewModel::runPreparation(const std::function<app::FileStagingPreparation()>& create) {
  const QPointer self(this);
  auto preparation = create();
  if (!self) {
    if (preparation.cancel) preparation.cancel();
    return;
  }
  if (!preparation.complete) return;
  if (!preparation.prepare) {
    preparation.complete({});
    return;
  }
  m_cancelPreparation = std::move(preparation.cancel);
  m_preparation.start(
      [prepare = std::move(preparation.prepare)](const std::stop_token stop) {
        prepare(stop);
        return std::monostate{};
      },
      [this, complete = std::move(preparation.complete)](AsyncResult<std::monostate> result) {
        m_cancelPreparation = {};
        const auto* failure = std::get_if<std::exception_ptr>(&result);
        complete(failure ? *failure : std::exception_ptr{});
      });
}

void FileStagingViewModel::rebuildEntries() {
  m_rebuildPending = true;
  if (m_rebuildingEntries) return;
  m_rebuildingEntries = true;
  const auto editing = m_editing;
  const QPointer self(this);
  while (m_rebuildPending) {
    m_rebuildPending = false;
    auto entries = editing->entries();
    if (!self) return;
    const auto baseline = editing->baselineEntries();
    if (!self) return;
    const app::FileStagingChanges changes{baseline, entries};
    const auto additions = changes.added();
    std::set<app::FileStagingEntryKey> added;
    for (const auto& entry : additions) added.insert(app::fileStagingEntryKey(entry));
    QList<Row> rows;
    for (auto removed = changes.removed(); const auto& entry : std::as_const(removed)) rows.append({entry, ItemStatus::Removed});
    for (const auto& entry : std::as_const(entries))
      rows.append({entry, added.contains(app::fileStagingEntryKey(entry)) ? ItemStatus::Added : ItemStatus::Normal});
    beginResetModel();
    if (!self) return;
    const auto previousAutomaticCommit = automaticCommitSummary();
    m_entries = std::move(entries);
    m_rows = std::move(rows);
    endResetModel();
    if (!self) return;
    if (previousAutomaticCommit != automaticCommitSummary()) emit automaticCommitStateChanged();
    if (!self) return;
    emit entriesChanged();
    if (!self) return;
  }
  m_rebuildingEntries = false;
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
  if (role == StatusTextRole)
    return row.status == ItemStatus::Removed ? PresentationText::tr("Pending removal")
           : row.status == ItemStatus::Added ? PresentationText::tr("Added")
                                             : automaticCommitPresentation(entry).status;
  if (role == BadgeRole)
    return static_cast<int>(row.status == ItemStatus::Removed ? ItemBadge::Remove : row.status == ItemStatus::Added ? ItemBadge::Add : ItemBadge::None);
  if (role == IconResourceRole) return directory ? QStringLiteral(":/icons/folder.svg") : QStringLiteral(":/icons/file.svg");
  if (role == Qt::ToolTipRole)
    return (directory ? PresentationText::tr("Folder: %1") : PresentationText::tr("File: %1")).arg(entry.path) + '\n' +
           automaticCommitPresentation(entry).description;
  return {};
}
void FileStagingViewModel::load() {
  const auto editing = m_editing;
  runPreparation([editing] { return editing->prepareLoad(); });
}
void FileStagingViewModel::retranslate() {
  if (rowCount() > 0) emit dataChanged(index(0), index(rowCount() - 1), {Qt::ToolTipRole, StatusTextRole});
}
QString FileStagingViewModel::summaryText() const {
  const auto directories = std::ranges::count_if(m_entries, [](const auto& entry) { return entry.kind == app::FileStagingKind::Directory; });
  const auto summary =
      PresentationText::tr("%1 staged entries · %2 files · %3 folders").arg(m_entries.size()).arg(m_entries.size() - directories).arg(directories);
  return summary + QStringLiteral(" · ") + (busy() ? PresentationText::tr("Please wait for the operation to finish.") : automaticCommitPresentation().status);
}

QString FileStagingViewModel::editingRestrictionText() const { return runtimeWriteRestrictionText(editingRestriction()); }

AutomaticCommitPresentation FileStagingViewModel::automaticCommitPresentation() const {
  return uwf::presentation::automaticCommitPresentation(automaticCommitSummary());
}

app::AutomaticCommitDisposition FileStagingViewModel::automaticCommitDisposition(const app::FileStagingEntry& entry) const {
  if (!entry.fileTarget) return app::AutomaticCommitDisposition::Unavailable;
  const auto& volumes = *m_availability.automaticCommitVolumes;
  const auto found = volumes.dispositions.find(core::volumeIdentity(entry.fileTarget->volumeId));
  return found != volumes.dispositions.end() ? found->second : volumes.unregistered;
}

app::AutomaticCommitSummary FileStagingViewModel::automaticCommitSummary() const {
  if (!m_availability.automaticCommitVolumes || m_entries.isEmpty()) return m_availability.automaticCommit;
  app::AutomaticCommitSummary summary;
  for (const auto& entry : m_entries) summary.include(automaticCommitDisposition(entry));
  return summary;
}

AutomaticCommitPresentation FileStagingViewModel::automaticCommitPresentation(const app::FileStagingEntry& entry) const {
  if (!m_availability.automaticCommitVolumes) return uwf::presentation::automaticCommitPresentation(m_availability.automaticCommit);
  app::AutomaticCommitSummary summary;
  summary.include(automaticCommitDisposition(entry));
  return uwf::presentation::automaticCommitPresentation(summary);
}

void FileStagingViewModel::setAvailability(const Availability availability) {
  if (m_availability == availability) return;
  const auto previous = std::exchange(m_availability, availability);
  const bool restrictionChanged = previous.editingRestriction != availability.editingRestriction;
  const bool presentationChanged =
      previous.automaticCommit != availability.automaticCommit || previous.automaticCommitVolumes != availability.automaticCommitVolumes;
  const QPointer self(this);
  if (presentationChanged && rowCount() > 0) emit dataChanged(index(0), index(rowCount() - 1), {Qt::ToolTipRole, StatusTextRole});
  if (!self) return;
  if (presentationChanged) emit automaticCommitStateChanged();
  if (!self) return;
  if (restrictionChanged || previous.readOnly != availability.readOnly || previous.additionsAllowed != availability.additionsAllowed)
    emit modificationAvailabilityChanged();
  if (self && previous.enhancedModeAvailable != availability.enhancedModeAvailable) emit enhancedModeAvailabilityChanged();
}

void FileStagingViewModel::addEntries(const QList<app::FileStagingEntry>& entries) {
  const auto editing = m_editing;
  if (canAddEntries()) runPreparation([editing, entries] { return editing->prepareAddEntries(entries); });
}

void FileStagingViewModel::removePaths(const QStringList& paths) {
  const auto editing = m_editing;
  if (writable() && !paths.isEmpty()) runPreparation([editing, paths] { return editing->prepareRemovePaths(paths); });
}
void FileStagingViewModel::restorePaths(const QStringList& paths) {
  const auto editing = m_editing;
  if (canAddEntries() && !paths.isEmpty()) runPreparation([editing, paths] { return editing->prepareRestorePaths(paths); });
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
      body = PresentationText::tr("The path has no drive letter and cannot be added to file staging:\n%1").arg(detail);
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
