// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "OverlayFilesViewModel.h"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QLocale>
#include <QPointer>
#include <QTextStream>
#include <stdexcept>
#include <utility>

#include "app/overlay/OverlayFileCatalog.h"
#include "app/overlay/OverlayFilesFailure.h"
#include "presentation/common/ItemRoles.h"
#include "presentation/common/PresentationText.h"
#include "presentation/common/TextFileExport.h"
#include "shared/ByteFormat.h"
#include "shared/DriveLetter.h"
#include "shared/Log.h"
namespace uwf::presentation {
using app::OverlayFileEntry;
using app::OverlayFileLoader;
namespace {
QString formatSize(qulonglong bytes) { return QString::fromStdString(formatBytes(bytes)); }

}  // namespace
OverlayFilesViewModel::OverlayFilesViewModel(const QString& driveLetter, app::OverlayFileLoader loader, app::TextFileWriter textFileWriter)
    : m_driveLetter(QString::fromStdString(drive::normalize(driveLetter.toStdString()))),
      m_loader(std::move(loader)),
      m_textFileWriter(std::move(textFileWriter)) {
  if (!m_loader) throw std::invalid_argument("overlay file loader must be callable");
}
OverlayFilesViewModel::~OverlayFilesViewModel() = default;
void OverlayFilesViewModel::startLoading() {
  m_worker.cancel();
  const auto generation = m_loading.begin();
  m_state = ContentLoadState::Loading;
  m_error.clear();
  m_errorHint.clear();
  m_errorDetail.clear();
  QPointer<OverlayFilesViewModel> self(this);
  emit loadingStarted();
  if (!self || !m_loading.isCurrent(generation)) return;
  m_worker.start([driveLetter = m_driveLetter, loader = m_loader](std::stop_token token) { return loader(driveLetter, token); },
                 [this, self, generation](LoadResult result) mutable {
                   m_loading.publish(generation, [self, generation, result = std::move(result)]() mutable {
                     if (self) self->onLoadFinished(generation, std::move(result));
                   });
                 });
}
void OverlayFilesViewModel::cancelLoading() {
  if (!loading()) return;
  m_loading.cancel();
  m_worker.cancel();
  m_state = ContentLoadState::Canceled;
  emit loadingStopped();
}
void OverlayFilesViewModel::retranslate() {
  emit headerDataChanged(Qt::Horizontal, 0, 2);
  if (rowCount() > 0) emit dataChanged(index(0, 1), index(rowCount() - 1, 1), {Qt::DisplayRole, FullTextRole});
}
void OverlayFilesViewModel::onLoadFinished(const LoadingGate::Ticket generation, LoadResult result) {
  QPointer<OverlayFilesViewModel> self(this);
  if (const auto* failure = std::get_if<std::exception_ptr>(&result)) {
    const auto classified = app::classifyOverlayFilesFailure(*failure);
    if (classified.kind == app::OverlayFilesFailure::Kind::Canceled) {
      m_state = ContentLoadState::Canceled;
      emit loadingStopped();
      return;
    }
    const QString error = classified.kind == app::OverlayFilesFailure::Kind::Unexpected
                              ? PresentationText::tr("Unexpected error while enumerating overlay files.")
                              : classified.detail;
    UWF_LOG_E("overlay-files") << "enumeration failed: error=" << error.toStdString();
    if (classified.kind == app::OverlayFilesFailure::Kind::ProviderCrashed) {
      m_errorHint = PresentationText::tr(
          "The WMI provider crashed while enumerating overlay files. This is a known instability of UWF_Overlay.GetOverlayFiles when the overlay "
          "is large or under I/O pressure.");
    } else if (classified.kind == app::OverlayFilesFailure::Kind::ResourcesOrUnsupported) {
      m_errorHint = PresentationText::tr(
          "Out of memory or operation not supported by the provider. Overlay file enumeration only works on NTFS volumes and requires headroom; "
          "try again with a smaller overlay or after closing memory-heavy applications.");
    }
    m_errorDetail = (m_errorHint.isEmpty() ? PresentationText::tr("Failed: %1") : PresentationText::tr("Raw error: %1")).arg(error);
    // Retain the plain-text diagnostic for signal consumers; the UI renders its semantic parts separately.
    m_error = m_errorHint.isEmpty() ? m_errorDetail : QStringLiteral("%1\n\n%2").arg(m_errorHint, m_errorDetail);
    m_state = ContentLoadState::Failed;
    emit loadingStopped();
    if (!self || !m_loading.isCurrent(generation)) return;
    emit loadFailed(m_error);
    return;
  }
  auto entries = std::get<QVector<OverlayFileEntry>>(std::move(result));
  entries = app::buildOverlayFileCatalog(m_driveLetter, std::move(entries));
  m_state = entries.isEmpty() ? ContentLoadState::Empty : ContentLoadState::Ready;
  emit loadingStopped();
  if (!self || !m_loading.isCurrent(generation)) return;

  beginResetModel();
  if (!self) return;
  if (!m_loading.isCurrent(generation)) {
    endResetModel();
    return;
  }
  m_entries = std::move(entries);
  endResetModel();
  if (!self || !m_loading.isCurrent(generation)) return;
  m_loaded = true;
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
  const QString kind = entry.isSystemMetadata ? PresentationText::tr("System metadata")
                       : entry.isDirectory    ? PresentationText::tr("Folder")
                                              : PresentationText::tr("File");
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
      ts << (e.isDirectory ? "dir" : "file") << '\t' << e.fileSize << '\t' << formatSize(e.fileSize) << '\t' << e.absolutePath << '\n';
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
