// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "presentation/viewmodels/overlay/OverlayFilesViewModel.h"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QLocale>
#include <QPointer>
#include <QTextStream>
#include <algorithm>
#include <stdexcept>
#include <utility>

#include "application/overlay/files/OverlayFileCatalog.h"
#include "foundation/logging/Log.h"
#include "foundation/paths/DriveLetter.h"
#include "foundation/text/ByteFormat.h"
#include "presentation/models/ItemRoles.h"
#include "presentation/viewmodels/common/PresentationText.h"
#include "presentation/viewmodels/common/TextFileExport.h"
namespace uwf::presentation {
using app::OverlayFileEntry;
namespace {
QString formatSize(qulonglong bytes) { return QString::fromStdString(formatBytes(bytes)); }
QString kindText(const app::OverlayFileKind kind) {
  switch (kind) {
    case app::OverlayFileKind::SystemMetadata:
      return PresentationText::tr("System metadata");
    case app::OverlayFileKind::Directory:
      return PresentationText::tr("Folder");
    case app::OverlayFileKind::File:
      return PresentationText::tr("File");
  }
  return PresentationText::tr("Unknown");
}

}  // namespace
OverlayFilesViewModel::OverlayFilesViewModel(const QString& driveLetter, const QString& volumeGuid, std::shared_ptr<app::OverlayFilesQueryManager> queries,
                                             app::TextFileWriter textFileWriter)
    : m_driveLetter(QString::fromStdString(drive::normalize(driveLetter.toStdString()))),
      m_volumeGuid(volumeGuid),
      m_queries(std::move(queries)),
      m_textFileWriter(std::move(textFileWriter)) {
  if (!m_queries) throw std::invalid_argument("overlay file query manager is required");
}
OverlayFilesViewModel::~OverlayFilesViewModel() = default;
void OverlayFilesViewModel::setCommitAvailability(const bool allowed, QString reason) {
  if (allowed) reason.clear();
  if (m_commitsAllowed == allowed && m_commitRestriction == reason) return;
  m_commitsAllowed = allowed;
  m_commitRestriction = std::move(reason);
  emit commitAvailabilityChanged();
}
bool OverlayFilesViewModel::canCommit(const QString& path) const {
  return m_commitsAllowed && m_state == ContentLoadState::Ready && commitTarget(path) && std::ranges::any_of(m_entries, [&](const auto& entry) {
           return entry.absolutePath == path && app::overlayFileKind(entry) != app::OverlayFileKind::SystemMetadata;
         });
}
std::optional<app::paths::VolumeFilePath> OverlayFilesViewModel::commitTarget(const QString& path) const {
  if (!app::paths::parseLocalFilePath(path)) return std::nullopt;
  const auto owner = app::paths::resolveVolumeFilePath(QStringLiteral("\\"), {}, m_volumeGuid.toStdString());
  if (!owner) return std::nullopt;
  const auto target = app::paths::resolveVolumeFilePath(path, {{m_driveLetter, owner->volume}});
  if (!target || target->volume != owner->volume) return std::nullopt;
  return target;
}
QString OverlayFilesViewModel::commitRestriction(const QString& path) const {
  const auto entry = std::ranges::find_if(m_entries, [&](const auto& item) { return item.absolutePath == path; });
  if (entry != m_entries.end() && app::overlayFileKind(*entry) == app::OverlayFileKind::SystemMetadata)
    return PresentationText::tr("System metadata is for diagnosis only and cannot be committed.");
  return m_commitsAllowed ? QString{} : m_commitRestriction.isEmpty() ? PresentationText::tr("UWF is currently unavailable") : m_commitRestriction;
}
void OverlayFilesViewModel::requestCommit(const QString& path) {
  if (canCommit(path))
    if (const auto target = commitTarget(path)) emit commitFileRequested(app::paths::volumeFilePath(*target));
}
void OverlayFilesViewModel::startLoading() {
  m_subscription.cancel();
  m_worker.cancel();
  const auto generation = m_loading.begin();
  m_state = ContentLoadState::Loading;
  m_failure.reset();
  QPointer<OverlayFilesViewModel> self(this);
  emit loadingStarted();
  if (!self || !m_loading.isCurrent(generation)) return;
  try {
    auto subscription = m_queries->subscribe(m_volumeGuid, this, [self, generation](app::OverlayFilesQueryManager::Result result) {
      if (self) self->onQueryFinished(generation, std::move(result));
    });
    if (!self || !m_loading.isCurrent(generation)) return;
    m_subscription = std::move(subscription);
  } catch (...) {
    onQueryFinished(generation, std::current_exception());
  }
}
void OverlayFilesViewModel::onQueryFinished(const LoadingGate::Ticket generation, app::OverlayFilesQueryManager::Result result) {
  if (!m_loading.isCurrent(generation)) return;
  const QPointer self(this);
  if (const auto* failure = std::get_if<std::exception_ptr>(&result)) {
    m_loading.publish(generation, [self, generation, failure = *failure] {
      if (self) self->onLoadFinished(generation, failure);
    });
    return;
  }
  auto entries = std::get<std::shared_ptr<const QVector<OverlayFileEntry>>>(std::move(result));
  m_worker.start(
      [driveLetter = m_driveLetter, entries = std::move(entries)](std::stop_token token) {
        if (token.stop_requested()) return QVector<OverlayFileEntry>{};
        return app::buildOverlayFileCatalog(driveLetter, *entries);
      },
      [this, self, generation](LoadResult result) mutable {
        m_loading.publish(generation, [self, generation, result = std::move(result)]() mutable {
          if (self) self->onLoadFinished(generation, std::move(result));
        });
      });
}
void OverlayFilesViewModel::cancelLoading() {
  m_subscription.cancel();
  m_worker.cancel();
  if (!loading()) return;
  m_loading.cancel();
  m_state = ContentLoadState::Canceled;
  emit loadingStopped();
}
void OverlayFilesViewModel::retranslate() {
  emit headerDataChanged(Qt::Horizontal, 0, 2);
  if (rowCount() > 0) emit dataChanged(index(0, 1), index(rowCount() - 1, 1), {Qt::DisplayRole, FullTextRole});
}
QString OverlayFilesViewModel::errorHint() const {
  if (!m_failure) return {};
  switch (m_failure->kind) {
    case app::OverlayFilesFailure::Kind::CompletionUnknown:
      return PresentationText::tr("The previous enumeration may still be running. Another read cannot start yet.");
    case app::OverlayFilesFailure::Kind::Unavailable:
      return PresentationText::tr("The UWF service is unavailable while enumerating overlay files.");
    case app::OverlayFilesFailure::Kind::ResourcesOrUnsupported:
      return PresentationText::tr(
          "Out of memory, or this operation is not supported. This feature requires an NTFS volume; close memory-intensive applications and try again.");
    default:
      return {};
  }
}
QString OverlayFilesViewModel::errorDetail() const {
  if (!m_failure) return {};
  const QString error = m_failure->kind == app::OverlayFilesFailure::Kind::Unexpected
                            ? PresentationText::tr("Unexpected error while enumerating overlay files.")
                            : m_failure->detail;
  return (errorHint().isEmpty() ? PresentationText::tr("Failed: %1") : PresentationText::tr("Raw error: %1")).arg(error);
}
QString OverlayFilesViewModel::errorText() const {
  const auto hint = errorHint();
  const auto detail = errorDetail();
  return hint.isEmpty() ? detail : QStringLiteral("%1\n\n%2").arg(hint, detail);
}
void OverlayFilesViewModel::onLoadFinished(const LoadingGate::Ticket generation, LoadResult result) {
  m_pendingLoad = PendingLoad{generation, std::move(result)};
  if (m_publishing) return;
  m_publishing = true;
  const QPointer self(this);
  while (m_pendingLoad) {
    auto next = std::move(*std::exchange(m_pendingLoad, {}));
    if (m_loading.isCurrent(next.generation)) publishLoad(next.generation, std::move(next.result));
    if (!self) return;
  }
  m_publishing = false;
}
void OverlayFilesViewModel::publishLoad(const LoadingGate::Ticket generation, LoadResult result) {
  QPointer<OverlayFilesViewModel> self(this);
  if (const auto* failure = std::get_if<std::exception_ptr>(&result)) {
    const auto classified = app::classifyOverlayFilesFailure(*failure);
    if (classified.kind == app::OverlayFilesFailure::Kind::Canceled) {
      m_state = ContentLoadState::Canceled;
      emit loadingStopped();
      return;
    }
    m_failure = classified;
    UWF_LOG_E("overlay-files") << "enumeration failed: error=" << errorDetail().toStdString();
    m_state = ContentLoadState::Failed;
    emit loadingStopped();
    if (!self || !m_loading.isCurrent(generation)) return;
    emit loadFailed(errorText());
    return;
  }
  auto entries = std::get<QVector<OverlayFileEntry>>(std::move(result));
  beginResetModel();
  if (!self) return;
  if (!m_loading.isCurrent(generation)) {
    endResetModel();
    return;
  }
  m_entries = std::move(entries);
  m_state = m_entries.isEmpty() ? ContentLoadState::Empty : ContentLoadState::Ready;
  m_loaded = true;
  endResetModel();
  if (!self || !m_loading.isCurrent(generation)) return;
  emit loadingStopped();
  if (!self || !m_loading.isCurrent(generation)) return;
  emit entriesLoaded();
}
int OverlayFilesViewModel::rowCount(const QModelIndex& parent) const { return parent.isValid() ? 0 : static_cast<int>(m_entries.size()); }
int OverlayFilesViewModel::columnCount(const QModelIndex& parent) const { return parent.isValid() ? 0 : 3; }
QVariant OverlayFilesViewModel::headerData(const int section, const Qt::Orientation orientation, const int role) const {
  if (orientation != Qt::Horizontal || section < 0 || section >= 3) return {};
  if (role == Qt::TextAlignmentRole) return static_cast<int>((section == 2 ? Qt::AlignRight : Qt::AlignLeft) | Qt::AlignVCenter);
  if (role != Qt::DisplayRole) return {};
  return section == 0 ? PresentationText::tr("Name and location") : section == 1 ? PresentationText::tr("Type") : PresentationText::tr("Size");
}
QVariant OverlayFilesViewModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid() || index.model() != this || index.column() < 0 || index.column() >= 3 || index.row() < 0 || index.row() >= rowCount()) return {};
  const auto& entry = m_entries[index.row()];
  const QString kind = kindText(app::overlayFileKind(entry));
  const auto path = QDir::fromNativeSeparators(entry.absolutePath);
  if (role == FullTextRole) return index.column() == 0 ? entry.absolutePath : index.column() == 1 ? kind : formatSize(entry.fileSize);
  if (role == Qt::DisplayRole) {
    if (index.column() == 0) return QFileInfo(path).fileName();
    if (index.column() == 1) return kind;
    if (entry.fileSize == 0) return QStringLiteral("—");
    const QLocale numbers(QLocale::English, QLocale::UnitedStates);
    return entry.fileSize >= 1048576 ? numbers.toString(static_cast<double>(entry.fileSize) / 1048576.0, 'f', 1) + QStringLiteral(" MB")
                                     : numbers.toString(qRound64(static_cast<double>(entry.fileSize) / 1024.0)) + QStringLiteral(" KB");
  }
  if (role == SecondaryTextRole && index.column() == 0) return QDir::toNativeSeparators(QFileInfo(path).path());
  if (role == Qt::TextAlignmentRole) return static_cast<int>((index.column() == 2 ? Qt::AlignRight : Qt::AlignLeft) | Qt::AlignVCenter);
  if (role == StatusRole && index.column() > 0) return static_cast<int>(ItemStatus::Muted);
  if (role == MonospaceRole) return index.column() == 2;
  if (role == Qt::ToolTipRole || role == PathRole) return entry.absolutePath;
  if (role == DirectoryRole) return entry.isDirectory;
  if (role == SystemMetadataRole) return entry.isSystemMetadata;
  if (role == IconResourceRole && index.column() == 0) return entry.isDirectory ? QStringLiteral(":/icons/folder.svg") : QStringLiteral(":/icons/file.svg");
  return {};
}
QString OverlayFilesViewModel::exportContent() const {
  const qulonglong total = app::overlayFileCatalogSize(m_entries);

  QString content;
  {
    QTextStream ts(&content);
    ts << "# UWF overlay files for " << m_driveLetter << '\n';
    ts << "# Generated " << QDateTime::currentDateTime().toString(Qt::ISODate) << '\n';
    ts << "# " << m_entries.size() << " entries, total " << formatSize(total) << '\n';
    ts << "# Note: list is approximate; files smaller than the disk cluster size (typically 4 KB) may be missing.\n";
    ts << "#\n";
    ts << "# type\tsize_bytes\tsize_human\tpath\n";
    for (const auto& e : std::as_const(m_entries)) {
      ts << app::overlayFileKindCode(app::overlayFileKind(e)) << '\t' << e.fileSize << '\t' << formatSize(e.fileSize) << '\t' << e.absolutePath << '\n';
    }
  }
  return content;
}
QString OverlayFilesViewModel::exportFile(const QString& path) const {
  if (!canExport()) return PresentationText::tr("Export failed");
  return exportTextFile(path, exportContent(), m_textFileWriter);
}
}  // namespace uwf::presentation

#include "moc_OverlayFilesViewModel.cpp"
