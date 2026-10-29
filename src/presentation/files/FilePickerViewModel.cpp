// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "FilePickerViewModel.h"

#include <QDir>
#include <QFileInfo>
#include <QLocale>
#include <QPointer>
#include <QRegularExpression>
#include <algorithm>
#include <stdexcept>
#include <utility>

#include "presentation/common/ItemRoles.h"
#include "presentation/common/PresentationText.h"
#include "presentation/common/PresentationTiming.h"
#include "shared/ByteFormat.h"

namespace uwf::presentation {
namespace {
QString fileBrowserErrorText(const std::exception& error) {
  const auto* browserError = dynamic_cast<const app::FileBrowserError*>(&error);
  if (!browserError) return QString::fromUtf8(error.what());
  switch (browserError->code()) {
    case app::FileBrowserErrorCode::MissingDirectory:
      return PresentationText::tr("The folder does not exist.");
    case app::FileBrowserErrorCode::AccessDenied:
      return PresentationText::tr("Access denied.");
    case app::FileBrowserErrorCode::AlreadyExists:
      return PresentationText::tr("An item with this name already exists.");
    case app::FileBrowserErrorCode::CreateDirectoryFailed:
      return PresentationText::tr("The folder could not be created. Check the name and permissions.");
    case app::FileBrowserErrorCode::RenameFailed:
      return PresentationText::tr("The item could not be renamed. Check the name and permissions.");
    case app::FileBrowserErrorCode::RenameUnsupported:
      return PresentationText::tr("Renaming is not supported by this browser.");
  }
  return QString::fromUtf8(error.what());
}
struct Listing {
  QVector<app::FileBrowserEntry> entries;
  QString error;
};
QString placeName(const app::FileBrowserPlace& place) {
  switch (place.kind) {
    case app::FileBrowserPlaceKind::Home:
      return PresentationText::tr("Home");
    case app::FileBrowserPlaceKind::Desktop:
      return PresentationText::tr("Desktop");
    case app::FileBrowserPlaceKind::Downloads:
      return PresentationText::tr("Downloads");
    case app::FileBrowserPlaceKind::Documents:
      return PresentationText::tr("Documents");
    case app::FileBrowserPlaceKind::Custom:
      return place.name;
  }
  return place.name;
}
class PlacesModel final : public QAbstractListModel {
 public:
  int rowCount(const QModelIndex& parent = {}) const override { return parent.isValid() ? 0 : static_cast<int>(m_rows.size()); }
  QVariant data(const QModelIndex& index, const int role = Qt::DisplayRole) const override {
    if (!index.isValid() || index.column() != 0 || index.row() < 0 || index.row() >= rowCount()) return {};
    const auto& row = m_rows[index.row()];
    if (role == Qt::DisplayRole || role == Qt::AccessibleTextRole)
      return row.heading ? (row.place.volume ? PresentationText::tr("This PC") : PresentationText::tr("Quick access")) : placeName(row.place);
    if (role == PathRole || role == Qt::ToolTipRole) return row.place.path;
    if (role == SeparatorRole) return row.heading;
    if (role == SystemMetadataRole) return row.place.system;
    if (role == IconResourceRole) return row.place.volume ? QStringLiteral(":/icons/disk.svg") : QStringLiteral(":/icons/folder.svg");
    if (role == Qt::CheckStateRole)
      return !row.heading && QDir::cleanPath(row.place.path).compare(m_path, Qt::CaseInsensitive) == 0 ? Qt::Checked : Qt::Unchecked;
    return {};
  }
  Qt::ItemFlags flags(const QModelIndex& index) const override {
    if (!index.isValid() || index.row() < 0 || index.row() >= rowCount()) return Qt::NoItemFlags;
    return m_rows[index.row()].heading ? Qt::NoItemFlags : Qt::ItemIsEnabled | Qt::ItemIsSelectable;
  }
  void reset(const QVector<app::FileBrowserPlace>& places) {
    beginResetModel();
    m_rows.clear();
    bool quick = false, volumes = false;
    for (auto place : places) {
      if (!place.volume && !quick) {
        m_rows.append({{{}, {}, false, false}, true});
        quick = true;
      }
      if (place.volume && !volumes) {
        m_rows.append({{{}, {}, true, false}, true});
        volumes = true;
      }
      m_rows.append({std::move(place), false});
    }
    endResetModel();
  }
  void retranslate() {
    if (!m_rows.isEmpty()) emit dataChanged(index(0), index(rowCount() - 1), {Qt::DisplayRole, Qt::AccessibleTextRole});
  }
  void setPath(const QString& path) {
    const auto current = QDir::cleanPath(path);
    if (m_path == current) return;
    m_path = current;
    if (!m_rows.isEmpty()) emit dataChanged(index(0), index(rowCount() - 1), {Qt::CheckStateRole});
  }

 private:
  struct Row {
    app::FileBrowserPlace place;
    bool heading;
  };
  QVector<Row> m_rows;
  QString m_path;
};
bool validLeafName(const QString& name) {
  static const QRegularExpression invalid(QStringLiteral("[\\\\/<>:\"|?*\\x{0000}-\\x{001f}]"));
  static const QRegularExpression reserved(QStringLiteral("^(CON|PRN|AUX|NUL|COM[1-9]|LPT[1-9])(?:\\.|$)"), QRegularExpression::CaseInsensitiveOption);
  return !name.isEmpty() && name != QLatin1String(".") && name != QLatin1String("..") && !name.endsWith(u'.') && !name.endsWith(u' ') &&
         !name.contains(invalid) && !reserved.match(name).hasMatch();
}
}  // namespace
FilePickerViewModel::FilePickerViewModel(const Mode mode, std::shared_ptr<const app::FileBrowser> browser, QObject* parent)
    : QAbstractTableModel(parent), m_mode(mode), m_browser(std::move(browser)) {
  if (!m_browser) throw std::invalid_argument("file browser is required");
  m_places = m_browser->places();
  auto places = std::make_unique<PlacesModel>();
  places->reset(m_places);
  m_placesModel = std::move(places);
  connect(this, &FilePickerViewModel::stateChanged, this, [this] { static_cast<PlacesModel*>(m_placesModel.get())->setPath(m_path); });
  setFilters({});
}
FilePickerViewModel::~FilePickerViewModel() { cancel(); }
int FilePickerViewModel::rowCount(const QModelIndex& parent) const { return parent.isValid() ? 0 : static_cast<int>(m_visible.size()); }
int FilePickerViewModel::columnCount(const QModelIndex& parent) const { return parent.isValid() ? 0 : 4; }
QVariant FilePickerViewModel::headerData(const int section, const Qt::Orientation orientation, const int role) const {
  if (orientation != Qt::Horizontal || section < 0 || section >= 4) return {};
  if (role == Qt::TextAlignmentRole) return static_cast<int>((section == 3 ? Qt::AlignRight : Qt::AlignLeft) | Qt::AlignVCenter);
  if (role != Qt::DisplayRole) return {};
  return section == 0   ? PresentationText::tr("Name")
         : section == 1 ? PresentationText::tr("Date modified")
         : section == 2 ? PresentationText::tr("Type")
                        : PresentationText::tr("Size");
}
QVariant FilePickerViewModel::data(const QModelIndex& index, const int role) const {
  if (!index.isValid() || index.row() < 0 || index.row() >= rowCount() || index.column() < 0 || index.column() >= 4) return {};
  const auto& item = m_visible[index.row()];
  if (role == PathRole || role == FullTextRole || role == Qt::ToolTipRole) return item.path;
  if (role == DirectoryRole) return item.directory;
  if (role == IconResourceRole && index.column() == 0) return item.directory ? QStringLiteral(":/icons/folder.svg") : QStringLiteral(":/icons/file.svg");
  if (role == MonospaceRole) return index.column() == 3;
  if (role == StatusRole && index.column() > 0) return static_cast<int>(ItemStatus::Muted);
  if (role == Qt::TextAlignmentRole) return static_cast<int>((index.column() == 3 ? Qt::AlignRight : Qt::AlignLeft) | Qt::AlignVCenter);
  if (role == Qt::CheckStateRole && multiple() && index.column() == 0) return m_selected.contains(item.path) ? Qt::Checked : Qt::Unchecked;
  if (role != Qt::DisplayRole) return {};
  switch (index.column()) {
    case 0:
      return item.name;
    case 1:
      return item.modified.toString(QStringLiteral("yyyy/MM/dd HH:mm"));
    case 2:
      return item.directory ? PresentationText::tr("Folder") : PresentationText::tr("File");
    default:
      return item.directory ? QStringLiteral("—") : QString::fromStdString(formatBytes(static_cast<std::uint64_t>(std::max(qint64{0}, item.size))));
  }
}
Qt::ItemFlags FilePickerViewModel::flags(const QModelIndex& index) const {
  return index.isValid() ? Qt::ItemIsEnabled | Qt::ItemIsSelectable : Qt::NoItemFlags;
}
QString FilePickerViewModel::resolve(const QString& value) const {
  QString path = QDir::fromNativeSeparators(value.trimmed());
  if (path.startsWith(u'"') && path.endsWith(u'"')) path = path.mid(1, path.size() - 2);
  if (path == QLatin1String("~")) path = QDir::homePath();
  if (path.startsWith(QLatin1String("~/"))) path = QDir::home().filePath(path.mid(2));
  return QDir::cleanPath(QDir::isAbsolutePath(path) ? path : QDir(m_path.isEmpty() ? QDir::homePath() : m_path).absoluteFilePath(path));
}
bool FilePickerViewModel::isPathAllowed(const QString& value) const {
  if (m_allowedRoots.isEmpty()) return true;
  const QString target = QDir::cleanPath(QDir::fromNativeSeparators(value));
  for (const auto& root : m_allowedRoots) {
    const auto prefix = root.endsWith(u'/') ? root : root + u'/';
    if (target.compare(root, Qt::CaseInsensitive) == 0 || target.startsWith(prefix, Qt::CaseInsensitive)) return true;
  }
  return false;
}
void FilePickerViewModel::setAllowedRoots(const QStringList& roots) {
  m_allowedRoots.clear();
  for (const auto& value : roots) {
    if (value.trimmed().isEmpty()) continue;
    QString root = QDir::fromNativeSeparators(value.trimmed());
    if (root.size() == 2 && root.at(1) == u':') root += u'/';
    m_allowedRoots.append(QDir::cleanPath(root));
  }
  m_places.clear();
  for (const auto places = m_browser->places(); const auto& place : places)
    if (isPathAllowed(place.path)) m_places.append(place);
  static_cast<PlacesModel*>(m_placesModel.get())->reset(m_places);
  m_history.clear();
  m_historyIndex = -1;
  m_selected.clear();
  m_result.clear();
  m_resultItems.clear();
  if (!m_path.isEmpty() && !isPathAllowed(m_path))
    navigate(m_allowedRoots.constFirst());
  else {
    refilter();
    emit stateChanged();
  }
}
void FilePickerViewModel::start(const QString& initialPath) {
  QString path = initialPath.isEmpty() ? (m_allowedRoots.isEmpty() ? QDir::homePath() : m_allowedRoots.constFirst()) : resolve(initialPath);
  if (!isPathAllowed(path)) path = m_allowedRoots.constFirst();
  const auto initial = m_browser->inspect(path);
  if (!initial.directory) {
    if (m_mode == Mode::SaveFile) m_filename = QFileInfo(path).fileName();
    path = QFileInfo(path).absolutePath();
  }
  navigate(path);
}
void FilePickerViewModel::navigate(const QString& path) {
  const QString target = resolve(path);
  if (!isPathAllowed(target)) {
    fail(PresentationText::tr("Choose a location within the allowed drive or folder."));
    return;
  }
  if (target != m_path) {
    while (m_history.size() > m_historyIndex + 1) m_history.removeLast();
    m_history.append(target);
    m_historyIndex = m_history.size() - 1;
    m_path = target;
  }
  load();
}
void FilePickerViewModel::goBack() {
  if (!canGoBack()) return;
  m_path = m_history[--m_historyIndex];
  load();
}
void FilePickerViewModel::goForward() {
  if (!canGoForward()) return;
  m_path = m_history[++m_historyIndex];
  load();
}
bool FilePickerViewModel::canGoUp() const {
  const auto parent = QDir::cleanPath(QFileInfo(m_path).absolutePath());
  return !m_path.isEmpty() && QDir::cleanPath(m_path) != parent && isPathAllowed(parent);
}
void FilePickerViewModel::goUp() {
  if (canGoUp()) navigate(QFileInfo(m_path).absolutePath());
}
void FilePickerViewModel::refresh() { load(); }
void FilePickerViewModel::retranslate() {
  static_cast<PlacesModel*>(m_placesModel.get())->retranslate();
  emit headerDataChanged(Qt::Horizontal, 0, 3);
  if (rowCount() > 0) emit dataChanged(index(0, 0), index(rowCount() - 1, 3), {Qt::DisplayRole});
}
void FilePickerViewModel::cancel() {
  m_worker.cancel();
  m_loading.cancel();
  m_result.clear();
  m_resultItems.clear();
  m_state = ContentLoadState::Canceled;
}
void FilePickerViewModel::load() {
  m_selectAfterLoad.clear();
  m_result.clear();
  m_resultItems.clear();
  m_worker.cancel();
  const auto ticket = m_loading.begin(timing::kProgressRevealDelay, [this] { emit stateChanged(); });
  m_state = ContentLoadState::Loading;
  m_error.clear();
  m_query.clear();
  m_selected.clear();
  if (m_mode != Mode::SaveFile) m_filename.clear();
  QPointer<FilePickerViewModel> self(this);
  m_worker.start(
      [browser = m_browser, path = m_path](std::stop_token token) {
        Listing result;
        if (token.stop_requested()) return result;
        try {
          result.entries = browser->list(path, token);
        } catch (const std::exception& error) {
          result.error = fileBrowserErrorText(error);
        } catch (...) {
          result.error = PresentationText::tr("Unable to read this folder.");
        }
        return result;
      },
      [this, self, ticket](AsyncResult<Listing> outcome) {
        Listing result;
        if (const auto* failure = std::get_if<std::exception_ptr>(&outcome)) {
          try {
            std::rethrow_exception(*failure);
          } catch (const std::exception& error) {
            result.error = fileBrowserErrorText(error);
          } catch (...) {
            result.error = PresentationText::tr("Unable to read this folder.");
          }
        } else
          result = std::get<Listing>(std::move(outcome));
        m_loading.publish(ticket, [self, result = std::move(result)] {
          if (!self) return;
          self->m_entries = result.entries;
          self->m_error = result.error;
          self->m_state = !result.error.isEmpty() ? ContentLoadState::Failed : result.entries.isEmpty() ? ContentLoadState::Empty : ContentLoadState::Ready;
          self->refilter();
          if (self) emit self->stateChanged();
        });
      });
  emit stateChanged();
}
void FilePickerViewModel::setQuery(const QString& query) {
  if (query != m_query) {
    m_query = query;
    refilter();
    emit stateChanged();
  }
}
void FilePickerViewModel::setFilters(const QString& filters) {
  m_filters = filters.split(QStringLiteral(";;"), Qt::SkipEmptyParts);
  if (m_filters.isEmpty()) m_filters.append(PresentationText::tr("All files (*)"));
  setFilterIndex(0);
}
void FilePickerViewModel::setFilterIndex(const int index) {
  if (index < 0 || index >= m_filters.size()) return;
  m_filter = index;
  const auto& filter = m_filters[index];
  const auto start = filter.lastIndexOf(u'(');
  const auto end = filter.lastIndexOf(u')');
  const auto patterns = start >= 0 && end > start ? filter.mid(start + 1, end - start - 1) : QStringLiteral("*");
  static const QRegularExpression separators(QStringLiteral("[ ;]+"));
  m_patterns = patterns.split(separators, Qt::SkipEmptyParts);
  refilter();
  emit stateChanged();
}
void FilePickerViewModel::refilter() {
  beginResetModel();
  m_visible.clear();
  for (const auto& item : std::as_const(m_entries)) {
    if (!isPathAllowed(item.path)) continue;
    if (m_mode == Mode::Directory && !item.directory) continue;
    if (!item.name.contains(m_query, Qt::CaseInsensitive)) continue;
    if (!item.directory && !m_patterns.contains(QStringLiteral("*")) && !m_patterns.contains(QStringLiteral("*.*")) && !QDir::match(m_patterns, item.name))
      continue;
    m_visible.append(item);
  }
  std::ranges::stable_sort(m_visible, [this](const auto& a, const auto& b) {
    if (a.directory != b.directory) return a.directory;
    const int comparison = m_sortColumn == 1   ? (a.modified < b.modified   ? -1
                                                  : a.modified > b.modified ? 1
                                                                            : 0)
                           : m_sortColumn == 3 ? (a.size < b.size   ? -1
                                                  : a.size > b.size ? 1
                                                                    : 0)
                                               : QString::compare(a.name, b.name, Qt::CaseInsensitive);
    return m_sortOrder == Qt::AscendingOrder ? comparison < 0 : comparison > 0;
  });
  const bool hadSelection = !m_selected.isEmpty();
  m_selected.removeIf([this](const QString& path) { return !std::ranges::any_of(m_visible, [&](const auto& item) { return item.path == path; }); });
  if (hadSelection && m_selected.isEmpty() && m_mode != Mode::SaveFile) m_filename.clear();
  if (!m_selectAfterLoad.isEmpty()) {
    const auto found = std::ranges::find_if(m_visible, [this](const auto& item) { return item.path == m_selectAfterLoad; });
    if (found != m_visible.end()) {
      m_selected = {found->path};
      m_filename = found->name;
    }
    m_selectAfterLoad.clear();
  }
  endResetModel();
}
void FilePickerViewModel::sort(const int column, const Qt::SortOrder order) {
  m_sortColumn = column;
  m_sortOrder = order;
  refilter();
}
bool FilePickerViewModel::isDirectory(const int row) const { return row >= 0 && row < rowCount() && m_visible[row].directory; }
QString FilePickerViewModel::entryPath(const int row) const { return row >= 0 && row < rowCount() ? m_visible[row].path : QString{}; }
void FilePickerViewModel::setFilename(const QString& name) {
  m_filename = name;
  m_selected.clear();
  m_result.clear();
  m_resultItems.clear();
  emit selectionChanged();
}
void FilePickerViewModel::selectRows(const QList<int>& rows) {
  m_selected.clear();
  QStringList names;
  for (const int row : rows) {
    if (row < 0 || row >= rowCount()) continue;
    if (!multiple() && !m_selected.isEmpty()) break;
    m_selected.append(m_visible[row].path);
    names.append(m_visible[row].name);
  }
  if (m_mode != Mode::SaveFile || !names.isEmpty()) m_filename = names.join(QStringLiteral("; "));
  m_result.clear();
  m_resultItems.clear();
  emit selectionChanged();
  if (rowCount() > 0) emit dataChanged(index(0, 0), index(rowCount() - 1, 0), {Qt::CheckStateRole});
}
bool FilePickerViewModel::canAccept() const {
  return (m_state == ContentLoadState::Ready || m_state == ContentLoadState::Empty) && !m_path.isEmpty() &&
         (m_mode == Mode::Directory || !m_filename.trimmed().isEmpty());
}
bool FilePickerViewModel::fail(const QString& message) {
  m_result.clear();
  m_resultItems.clear();
  emit validationFailed(message);
  return false;
}
FilePickerViewModel::Acceptance FilePickerViewModel::prepareAcceptance() {
  m_result.clear();
  m_resultItems.clear();
  if (!canAccept()) return Acceptance::Invalid;
  QStringList candidates = m_selected;
  if (candidates.isEmpty()) candidates.append(m_mode == Mode::Directory ? m_path : resolve(m_filename));
  try {
    if (m_mode == Mode::SaveFile) {
      QString target = resolve(m_filename);
      if (!isPathAllowed(target)) {
        fail(PresentationText::tr("Choose a location within the allowed drive or folder."));
        return Acceptance::Invalid;
      }
      auto info = m_browser->inspect(target);
      if (info.exists && info.directory) {
        navigate(target);
        return Acceptance::Navigate;
      }
      if (QFileInfo(target).suffix().isEmpty()) {
        for (const auto& pattern : std::as_const(m_patterns)) {
          if (pattern.startsWith(QLatin1String("*.")) && !pattern.mid(2).contains(u'*') && !pattern.contains(u'?')) {
            target += pattern.mid(1);
            break;
          }
        }
      }
      info = m_browser->inspect(target);
      const auto parent = m_browser->inspect(QFileInfo(target).absolutePath());
      if (!parent.exists || !parent.directory) {
        fail(PresentationText::tr("The destination folder does not exist."));
        return Acceptance::Invalid;
      }
      if (info.directory || !validLeafName(QFileInfo(target).fileName())) {
        fail(PresentationText::tr("Enter a valid file name."));
        return Acceptance::Invalid;
      }
      m_result.append(QDir::toNativeSeparators(target));
      info.path = m_result.constLast();
      m_resultItems.append(info);
      return info.exists ? Acceptance::Replace : Acceptance::Ready;
    }
    for (const auto& target : candidates) {
      if (!isPathAllowed(target)) {
        fail(PresentationText::tr("Choose a location within the allowed drive or folder."));
        return Acceptance::Invalid;
      }
      const auto info = m_browser->inspect(target);
      if (!info.exists) {
        fail(PresentationText::tr("The selected item no longer exists: %1").arg(QDir::toNativeSeparators(target)));
        return Acceptance::Invalid;
      }
      if (info.directory && m_mode != Mode::Directory && m_mode != Mode::Mixed) {
        if (candidates.size() == 1) {
          navigate(target);
          return Acceptance::Navigate;
        }
        fail(PresentationText::tr("Select files only, or open a folder to browse its contents."));
        return Acceptance::Invalid;
      }
      if (!info.directory && m_mode == Mode::Directory) {
        fail(PresentationText::tr("Select a folder."));
        return Acceptance::Invalid;
      }
      m_result.append(QDir::toNativeSeparators(target));
      auto selected = info;
      selected.path = m_result.constLast();
      m_resultItems.append(selected);
    }
    return Acceptance::Ready;
  } catch (const std::exception& error) {
    fail(fileBrowserErrorText(error));
    return Acceptance::Invalid;
  }
}
void FilePickerViewModel::createDirectory(const QString& name) {
  const QString value = name.trimmed();
  if (!validLeafName(value)) {
    fail(PresentationText::tr("Enter a valid folder name."));
    return;
  }
  try {
    const auto path = QDir(m_path).filePath(value);
    m_browser->createDirectory(path);
    refresh();
    m_selectAfterLoad = path;
  } catch (const std::exception& error) {
    fail(fileBrowserErrorText(error));
  }
}
void FilePickerViewModel::renameEntry(const int row, const QString& name) {
  const auto source = entryPath(row);
  const auto leaf = name.trimmed();
  if (source.isEmpty() || !validLeafName(leaf)) {
    fail(PresentationText::tr("Enter a valid file or folder name."));
    return;
  }
  const auto target = QDir(QFileInfo(source).absolutePath()).filePath(leaf);
  if (!isPathAllowed(source) || !isPathAllowed(target)) {
    fail(PresentationText::tr("Choose a location within the allowed drive or folder."));
    return;
  }
  if (source == target) return;
  try {
    m_browser->renameEntry(source, target);
    refresh();
    m_selectAfterLoad = target;
  } catch (const std::exception& error) {
    fail(fileBrowserErrorText(error));
  }
}
}  // namespace uwf::presentation
#include "moc_FilePickerViewModel.cpp"
