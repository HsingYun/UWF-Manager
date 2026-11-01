// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "presentation/viewmodels/files/FilePickerViewModel.h"

#include <QDir>
#include <QFileInfo>
#include <QHash>
#include <QLocale>
#include <QPointer>
#include <QRegularExpression>
#include <algorithm>
#include <optional>
#include <stdexcept>
#include <utility>

#include "application/paths/PathPolicy.h"
#include "foundation/text/ByteFormat.h"
#include "presentation/models/ItemRoles.h"
#include "presentation/viewmodels/common/PresentationText.h"
#include "presentation/viewmodels/common/PresentationTiming.h"

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
  std::optional<QVector<app::FileBrowserPlace>> places;
  QString path;
  QString filename;
  QString selectedFile;
  std::exception_ptr failure;
};
struct FilteredListing {
  QVector<app::FileBrowserEntry> visible;
  QStringList selected;
  QStringList selectedNames;
  std::optional<QString> filename;
};
bool withinAllowedRoots(const QString& value, const QStringList& roots) {
  if (roots.isEmpty()) return true;
  const auto target = app::paths::fileSystemPath(value);
  return std::ranges::any_of(roots, [&](const QString& root) { return app::paths::contains(root, target); });
}
FilteredListing filterListing(const QVector<app::FileBrowserEntry>& entries, const FilePickerViewModel::Mode mode, const QStringList& roots,
                              const QString& query, const QStringList& patterns, const int sortColumn, const Qt::SortOrder order, const QStringList& selected,
                              const QString& selectAfterLoad, const std::stop_token stop) {
  FilteredListing result;
  const auto canceled = [&] {
    if (stop.stop_requested()) throw std::runtime_error("File query canceled");
  };
  for (const auto& item : entries) {
    canceled();
    if (!withinAllowedRoots(item.path, roots)) continue;
    if (mode == FilePickerViewModel::Mode::Directory && !item.directory) continue;
    if (!item.name.contains(query, Qt::CaseInsensitive)) continue;
    if (!item.directory && !patterns.contains(QStringLiteral("*")) && !patterns.contains(QStringLiteral("*.*")) && !QDir::match(patterns, item.name)) continue;
    result.visible.append(item);
  }
  std::ranges::stable_sort(result.visible, [&](const auto& a, const auto& b) {
    canceled();
    if (a.directory != b.directory) return a.directory;
    const int comparison = sortColumn == 1   ? (a.modified < b.modified   ? -1
                                                : a.modified > b.modified ? 1
                                                                          : 0)
                           : sortColumn == 3 ? (a.size < b.size   ? -1
                                                : a.size > b.size ? 1
                                                                  : 0)
                                             : QString::compare(a.name, b.name, Qt::CaseInsensitive);
    return order == Qt::AscendingOrder ? comparison < 0 : comparison > 0;
  });
  QHash<QString, QString> visibleNames;
  visibleNames.reserve(result.visible.size());
  for (const auto& item : std::as_const(result.visible)) {
    canceled();
    visibleNames.insert(item.path, item.name);
  }
  for (const auto& path : selected) {
    canceled();
    if (mode == FilePickerViewModel::Mode::SaveFile || visibleNames.contains(path)) {
      result.selected.append(path);
      result.selectedNames.append(visibleNames.value(path));
    }
  }
  if (!selectAfterLoad.isEmpty() && visibleNames.contains(selectAfterLoad)) {
    result.selected = {selectAfterLoad};
    result.selectedNames = {visibleNames.value(selectAfterLoad)};
    if (mode == FilePickerViewModel::Mode::SaveFile) result.filename = visibleNames.value(selectAfterLoad);
  }
  return result;
}
QString failureText(const std::exception_ptr failure) {
  try {
    std::rethrow_exception(failure);
  } catch (const std::exception& error) {
    return fileBrowserErrorText(error);
  } catch (...) {
    return PresentationText::tr("Unable to read this folder.");
  }
}
struct Selection {
  FilePickerViewModel::Acceptance status = FilePickerViewModel::Acceptance::Invalid;
  QStringList paths;
  QVector<app::FileBrowserEntry> items;
  QString navigation;
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
      return !row.heading && app::equalObjectNames(app::paths::fileSystemPath(row.place.path), m_path) ? Qt::Checked : Qt::Unchecked;
    return {};
  }
  Qt::ItemFlags flags(const QModelIndex& index) const override {
    if (!index.isValid() || index.row() < 0 || index.row() >= rowCount()) return Qt::NoItemFlags;
    return m_rows[index.row()].heading ? Qt::NoItemFlags : Qt::ItemIsEnabled | Qt::ItemIsSelectable;
  }
  void reset(const QVector<app::FileBrowserPlace>& places) {
    QVector<Row> rows;
    bool quick = false, volumes = false;
    for (auto place : places) {
      if (!place.volume && !quick) {
        rows.append({{{}, {}, false, false}, true});
        quick = true;
      }
      if (place.volume && !volumes) {
        rows.append({{{}, {}, true, false}, true});
        volumes = true;
      }
      rows.append({std::move(place), false});
    }
    m_pendingRows = std::move(rows);
    if (m_publishing) return;
    m_publishing = true;
    const QPointer self(this);
    while (m_pendingRows) {
      auto next = std::move(*std::exchange(m_pendingRows, {}));
      beginResetModel();
      if (!self) return;
      m_rows = std::move(next);
      endResetModel();
      if (!self) return;
    }
    m_publishing = false;
  }
  void retranslate() {
    if (!m_rows.isEmpty()) emit dataChanged(index(0), index(rowCount() - 1), {Qt::DisplayRole, Qt::AccessibleTextRole});
  }
  void setPath(const QString& path) {
    const auto current = app::paths::fileSystemPath(path);
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
  std::optional<QVector<Row>> m_pendingRows;
  bool m_publishing = false;
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
  auto places = std::make_unique<PlacesModel>();
  m_placesModel = std::move(places);
  connect(this, &FilePickerViewModel::stateChanged, this, [this] { static_cast<PlacesModel*>(m_placesModel.get())->setPath(m_path); });
  setFilters({});
}
FilePickerViewModel::~FilePickerViewModel() = default;
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
  QString path = value;
  if (path.startsWith(u'"') && path.endsWith(u'"')) path = path.mid(1, path.size() - 2);
  if (path == QLatin1String("~")) path = QDir::homePath();
  if (path.startsWith(QLatin1String("~/")) || path.startsWith(QLatin1String("~\\"))) path = app::paths::joinFilePath(QDir::homePath(), path.mid(2));
  return app::paths::absoluteFilePath(path, m_path.isEmpty() ? QDir::homePath() : m_path);
}
bool FilePickerViewModel::isPathAllowed(const QString& value) const { return withinAllowedRoots(value, m_allowedRoots); }
void FilePickerViewModel::setAllowedRoots(const QStringList& roots) {
  if (m_loading.isActive()) return;
  m_allowedRoots.clear();
  for (const auto& value : roots) {
    if (value.isEmpty()) continue;
    QString root = value;
    if (root.size() == 2 && root.at(1) == u':') root += u'/';
    m_allowedRoots.append(app::paths::fileSystemPath(root));
  }
  const QPointer self(this);
  updatePlaces();
  if (!self) return;
  m_history.clear();
  m_historyIndex = -1;
  m_selected.clear();
  m_selectedNames.clear();
  ++m_selectionGeneration;
  m_result.clear();
  m_resultItems.clear();
  if (!m_path.isEmpty() && !isPathAllowed(m_path))
    navigate(m_allowedRoots.constFirst());
  else {
    refilter();
    emit stateChanged();
  }
}
void FilePickerViewModel::updatePlaces() {
  m_places.clear();
  for (const auto& place : std::as_const(m_allPlaces))
    if (isPathAllowed(place.path)) m_places.append(place);
  static_cast<PlacesModel*>(m_placesModel.get())->reset(m_places);
}
void FilePickerViewModel::start(const QString& initialPath) {
  if (m_loading.isActive()) return;
  QString path = initialPath.isEmpty() ? (m_allowedRoots.isEmpty() ? QDir::homePath() : m_allowedRoots.constFirst()) : resolve(initialPath);
  if (!isPathAllowed(path)) path = m_allowedRoots.constFirst();
  m_path = path;
  m_history.clear();
  m_historyIndex = -1;
  load(true);
}
void FilePickerViewModel::navigate(const QString& path) {
  if (m_loading.isActive()) return;
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
  const auto parent = app::paths::parentDirectory(m_path);
  return !busy() && !m_path.isEmpty() && app::paths::fileSystemPath(m_path) != parent && isPathAllowed(parent);
}
void FilePickerViewModel::goUp() {
  if (canGoUp()) navigate(app::paths::parentDirectory(m_path));
}
void FilePickerViewModel::refresh() {
  if (!m_loading.isActive()) load();
}
void FilePickerViewModel::retranslate() {
  static_cast<PlacesModel*>(m_placesModel.get())->retranslate();
  emit headerDataChanged(Qt::Horizontal, 0, 3);
  if (rowCount() > 0) emit dataChanged(index(0, 0), index(rowCount() - 1, 3), {Qt::DisplayRole});
}
void FilePickerViewModel::cancel() {
  m_refilterPending = false;
  m_readPublishing = false;
  m_filterWorker.cancel();
  m_filtering.cancel();
  m_worker.cancel();
  m_loading.cancel();
  m_result.clear();
  m_resultItems.clear();
  m_state = ContentLoadState::Canceled;
  emit stateChanged();
}
void FilePickerViewModel::load(const bool initial) {
  m_refilterPending = false;
  m_readPublishing = false;
  m_filterWorker.cancel();
  m_filtering.cancel();
  m_selectAfterLoad.clear();
  m_result.clear();
  m_resultItems.clear();
  m_worker.cancel();
  const auto ticket = m_loading.begin(timing::kProgressRevealDelay, [this] { emit stateChanged(); });
  m_state = ContentLoadState::Loading;
  m_error.clear();
  m_query.clear();
  m_selected.clear();
  m_selectedNames.clear();
  ++m_selectionGeneration;
  if (m_mode != Mode::SaveFile) m_enteredName.clear();
  QPointer<FilePickerViewModel> self(this);
  m_worker.start(
      [browser = m_browser, path = m_path, roots = m_allowedRoots, initial, saving = m_mode == Mode::SaveFile,
       readPlaces = !m_placesLoaded](std::stop_token token) {
        Listing result;
        if (token.stop_requested()) return result;
        if (readPlaces) result.places = browser->places();
        result.path = path;
        if (token.stop_requested()) return result;
        try {
          if (initial) {
            const auto info = browser->inspect(path);
            if (!info.directory) {
              if (saving) {
                result.filename = app::paths::fileName(path);
                if (info.exists) result.selectedFile = path;
              }
              const auto directory = app::paths::parentDirectory(path);
              if (!withinAllowedRoots(directory, roots))
                throw std::runtime_error(PresentationText::tr("Choose a location within the allowed drive or folder.").toStdString());
              result.path = directory;
            }
          }
          if (!token.stop_requested()) result.entries = browser->list(result.path, token);
        } catch (...) {
          result.failure = std::current_exception();
        }
        return result;
      },
      [this, self, ticket, initial](AsyncResult<Listing> outcome) {
        m_loading.publish(ticket, [self, ticket, outcome = std::move(outcome), initial]() mutable {
          if (!self) return;
          self->m_readPublishing = true;
          bool placesChanged = false;
          if (const auto* failure = std::get_if<std::exception_ptr>(&outcome)) {
            self->m_entries.clear();
            self->m_error = failureText(*failure);
            self->m_listingState = ContentLoadState::Failed;
          } else {
            auto result = std::get<Listing>(std::move(outcome));
            if (result.places) {
              self->m_allPlaces = std::move(*result.places);
              self->m_placesLoaded = true;
              placesChanged = true;
            }
            self->m_path = std::move(result.path);
            if (initial) {
              self->m_history = {self->m_path};
              self->m_historyIndex = 0;
              self->m_enteredName = std::move(result.filename);
              if (!result.selectedFile.isEmpty()) self->m_selected = {std::move(result.selectedFile)};
            }
            self->m_entries = std::move(result.entries);
            if (result.failure) self->m_error = failureText(result.failure);
            self->m_listingState = result.failure ? ContentLoadState::Failed : self->m_entries.isEmpty() ? ContentLoadState::Empty : ContentLoadState::Ready;
          }
          if (placesChanged) {
            self->updatePlaces();
            if (!self || !self->m_loading.isCurrent(ticket)) return;
          }
          self->m_readPublishing = false;
          self->refilter();
        });
      });
  emit stateChanged();
}
void FilePickerViewModel::setQuery(const QString& query) {
  if (query != m_query) {
    m_query = query;
    const QPointer self(this);
    if (!m_loading.isActive()) refilter();
    if (self) emit stateChanged();
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
  const QPointer self(this);
  if (!m_loading.isActive()) refilter();
  if (self) emit stateChanged();
}
void FilePickerViewModel::finishFilterPublication() {
  m_filterPublishing = false;
  if (m_refilterPending) {
    m_refilterPending = false;
    if (!m_loading.isActive()) refilter();
  }
}
void FilePickerViewModel::refilter() {
  m_filterWorker.cancel();
  if (m_filterPublishing) {
    m_filtering.cancel();
    m_refilterPending = true;
    return;
  }
  m_refilterPending = false;
  const auto ticket = m_filtering.begin(timing::kProgressRevealDelay, [this] { emit stateChanged(); });
  const auto selectionGeneration = m_selectionGeneration;
  const auto apply = [this, ticket, selectionGeneration](AsyncResult<FilteredListing> outcome) {
    m_filtering.publish(ticket, [this, ticket, selectionGeneration, outcome = std::move(outcome)]() mutable {
      if (!m_filtering.isCurrent(ticket)) return;
      if (selectionGeneration != m_selectionGeneration) {
        refilter();
        return;
      }
      FilteredListing result;
      std::optional<QString> error;
      if (const auto* failure = std::get_if<std::exception_ptr>(&outcome))
        error = failureText(*failure);
      else
        result = std::get<FilteredListing>(std::move(outcome));
      const auto previousSelection = m_selected;
      const QPointer self(this);
      m_filterPublishing = true;
      beginResetModel();
      if (!self) return;
      if (!m_filtering.isCurrent(ticket) || selectionGeneration != m_selectionGeneration) {
        if (m_filtering.isCurrent(ticket)) m_refilterPending = true;
        endResetModel();
        if (self) finishFilterPublication();
        return;
      }
      if (error) {
        m_error = std::move(*error);
        m_listingState = ContentLoadState::Failed;
      }
      m_visible = std::move(result.visible);
      m_selected = std::move(result.selected);
      m_selectedNames = std::move(result.selectedNames);
      if (result.filename) m_enteredName = std::move(*result.filename);
      m_selectAfterLoad.clear();
      m_state = m_listingState;
      endResetModel();
      if (!self) return;
      finishFilterPublication();
      if (!self || !m_filtering.isCurrent(ticket)) return;
      if (selectionGeneration == m_selectionGeneration && previousSelection != m_selected) {
        m_result.clear();
        m_resultItems.clear();
        emit selectionChanged();
      }
      if (self && m_filtering.isCurrent(ticket)) emit stateChanged();
    });
  };
  // Empty listings allow synchronous filter setup before the initial read.
  if (m_entries.isEmpty()) {
    FilteredListing empty;
    if (m_mode == Mode::SaveFile) empty.selected = m_selected;
    apply(AsyncResult<FilteredListing>{std::in_place_index<0>, std::move(empty)});
    return;
  }
  m_filterWorker.start(
      [entries = m_entries, mode = m_mode, roots = m_allowedRoots, query = m_query, patterns = m_patterns, column = m_sortColumn, order = m_sortOrder,
       selected = m_selected, selectAfterLoad = m_selectAfterLoad](std::stop_token stop) {
        return filterListing(entries, mode, roots, query, patterns, column, order, selected, selectAfterLoad, stop);
      },
      apply);
  emit stateChanged();
}

void FilePickerViewModel::sort(const int column, const Qt::SortOrder order) {
  m_sortColumn = column;
  m_sortOrder = order;
  if (!m_loading.isActive()) refilter();
}
bool FilePickerViewModel::isDirectory(const int row) const { return row >= 0 && row < rowCount() && m_visible[row].directory; }
QString FilePickerViewModel::entryPath(const int row) const { return row >= 0 && row < rowCount() ? m_visible[row].path : QString{}; }
void FilePickerViewModel::setFilename(const QString& name) {
  if (m_loading.isActive()) return;
  m_enteredName = name;
  m_selectAfterLoad.clear();
  m_selected.clear();
  m_selectedNames.clear();
  ++m_selectionGeneration;
  m_result.clear();
  m_resultItems.clear();
  emit selectionChanged();
}
QString FilePickerViewModel::filename() const {
  if (m_mode == Mode::SaveFile || m_selected.isEmpty()) return m_enteredName;
  return m_selectedNames.join(QStringLiteral("; "));
}
void FilePickerViewModel::selectRows(const QList<int>& rows) {
  if (m_loading.isActive()) return;
  m_selectAfterLoad.clear();
  m_selected.clear();
  m_selectedNames.clear();
  ++m_selectionGeneration;
  QStringList names;
  for (const int row : rows) {
    if (row < 0 || row >= rowCount()) continue;
    if (!multiple() && !m_selected.isEmpty()) break;
    m_selected.append(m_visible[row].path);
    names.append(m_visible[row].name);
  }
  m_selectedNames = names;
  if (m_mode == Mode::SaveFile) {
    if (!names.isEmpty()) m_enteredName = names.constFirst();
  } else {
    m_enteredName.clear();
  }
  m_result.clear();
  m_resultItems.clear();
  const QPointer self(this);
  emit selectionChanged();
  if (self && rowCount() > 0) emit dataChanged(index(0, 0), index(rowCount() - 1, 0), {Qt::CheckStateRole});
}
bool FilePickerViewModel::canAccept() const {
  return !busy() && (m_state == ContentLoadState::Ready || m_state == ContentLoadState::Empty) && !m_path.isEmpty() &&
         (m_mode == Mode::Directory || !filename().isEmpty());
}
bool FilePickerViewModel::fail(const QString& message) {
  m_result.clear();
  m_resultItems.clear();
  emit validationFailed(message);
  return false;
}
void FilePickerViewModel::requestAcceptance() {
  if (!canAccept()) return;
  m_result.clear();
  m_resultItems.clear();
  QStringList candidates = m_selected;
  const bool selectedSaveTarget = m_mode == Mode::SaveFile && !m_selected.isEmpty();
  const QString target = selectedSaveTarget ? m_selected.constFirst() : resolve(filename());
  if (candidates.isEmpty()) candidates.append(m_mode == Mode::Directory ? m_path : target);
  if (m_mode == Mode::SaveFile) candidates = {target};
  for (const auto& candidate : std::as_const(candidates))
    if (!isPathAllowed(candidate)) {
      fail(PresentationText::tr("Choose a location within the allowed drive or folder."));
      emit acceptanceFinished(Acceptance::Invalid);
      return;
    }
  const auto ticket = m_loading.begin(timing::kProgressRevealDelay, [this] { emit stateChanged(); });
  m_worker.start(
      [browser = m_browser, mode = m_mode, candidates, target = target, patterns = m_patterns, selectedSaveTarget](std::stop_token token) mutable {
        Selection result;
        const auto invalid = [&](QString error) {
          result.error = std::move(error);
          return result;
        };
        if (mode == Mode::SaveFile) {
          auto info = browser->inspect(target);
          if (token.stop_requested()) return result;
          if (info.exists && info.directory) {
            result.status = Acceptance::Navigate;
            result.navigation = target;
            return result;
          }
          if (selectedSaveTarget && !info.exists)
            return invalid(PresentationText::tr("The selected item no longer exists: %1").arg(QDir::toNativeSeparators(target)));
          if (!selectedSaveTarget && !validLeafName(app::paths::fileName(target))) return invalid(PresentationText::tr("Enter a valid file name."));
          if (!selectedSaveTarget && QFileInfo(app::paths::fileName(target)).suffix().isEmpty()) {
            for (const auto& pattern : patterns) {
              if (pattern.startsWith(QLatin1String("*.")) && !pattern.mid(2).contains(u'*') && !pattern.contains(u'?')) {
                target += pattern.mid(1);
                break;
              }
            }
          }
          if (!selectedSaveTarget) info = browser->inspect(target);
          if (token.stop_requested()) return result;
          const auto parent = browser->inspect(app::paths::parentDirectory(target));
          if (!parent.exists || !parent.directory) return invalid(PresentationText::tr("The destination folder does not exist."));
          if (info.directory || (!selectedSaveTarget && !validLeafName(app::paths::fileName(target))))
            return invalid(PresentationText::tr("Enter a valid file name."));
          result.paths.append(QDir::toNativeSeparators(target));
          info.path = result.paths.constLast();
          result.items.append(info);
          result.status = info.exists ? Acceptance::Replace : Acceptance::Ready;
          return result;
        }
        for (const auto& candidate : std::as_const(candidates)) {
          if (token.stop_requested()) return result;
          auto info = browser->inspect(candidate);
          if (!info.exists) return invalid(PresentationText::tr("The selected item no longer exists: %1").arg(QDir::toNativeSeparators(candidate)));
          if (info.directory && mode != Mode::Directory && mode != Mode::Mixed) {
            if (candidates.size() == 1) {
              result.status = Acceptance::Navigate;
              result.navigation = candidate;
              return result;
            }
            return invalid(PresentationText::tr("Select files only, or open a folder to browse its contents."));
          }
          if (!info.directory && mode == Mode::Directory) return invalid(PresentationText::tr("Select a folder."));
          result.paths.append(QDir::toNativeSeparators(candidate));
          info.path = result.paths.constLast();
          result.items.append(std::move(info));
        }
        result.status = Acceptance::Ready;
        return result;
      },
      [this, ticket](AsyncResult<Selection> outcome) {
        m_loading.publish(ticket, [this, outcome = std::move(outcome)]() mutable {
          Selection result;
          if (const auto* failure = std::get_if<std::exception_ptr>(&outcome))
            result.error = failureText(*failure);
          else
            result = std::get<Selection>(std::move(outcome));
          const QPointer<FilePickerViewModel> self(this);
          m_result = std::move(result.paths);
          m_resultItems = std::move(result.items);
          if (!result.error.isEmpty()) fail(result.error);
          if (!self) return;
          if (result.status == Acceptance::Navigate) navigate(result.navigation);
          if (!self) return;
          emit stateChanged();
          if (self) emit acceptanceFinished(result.status);
        });
      });
  emit stateChanged();
}
void FilePickerViewModel::createDirectory(const QString& name) {
  if (busy() || (m_state != ContentLoadState::Ready && m_state != ContentLoadState::Empty)) return;
  if (!validLeafName(name)) {
    fail(PresentationText::tr("Enter a valid folder name."));
    return;
  }
  writeEntry(app::paths::joinFilePath(m_path, name));
}
void FilePickerViewModel::renameEntry(const int row, const QString& name) {
  if (busy() || (m_state != ContentLoadState::Ready && m_state != ContentLoadState::Empty)) return;
  const auto source = entryPath(row);
  if (source.isEmpty()) {
    fail(PresentationText::tr("Enter a valid file or folder name."));
    return;
  }
  // Existing object names can exceed the policy for newly created names. An
  // unchanged picker prefill is still the same object, including literal spaces.
  if (name == m_visible[row].name) {
    m_error.clear();
    emit stateChanged();
    return;
  }
  if (!validLeafName(name)) {
    fail(PresentationText::tr("Enter a valid file or folder name."));
    return;
  }
  const auto target = app::paths::joinFilePath(app::paths::parentDirectory(source), name);
  if (source != target) writeEntry(target, source);
}
void FilePickerViewModel::writeEntry(const QString& target, const QString& source) {
  if (!isPathAllowed(target) || (!source.isEmpty() && !isPathAllowed(source))) {
    fail(PresentationText::tr("Choose a location within the allowed drive or folder."));
    return;
  }
  m_result.clear();
  m_resultItems.clear();
  const auto ticket = m_loading.begin(timing::kProgressRevealDelay, [this] { emit stateChanged(); });
  m_worker.start(
      [browser = m_browser, target, source](std::stop_token token) {
        if (token.stop_requested()) return false;
        if (source.isEmpty())
          browser->createDirectory(target);
        else
          browser->renameEntry(source, target);
        return true;
      },
      [this, ticket, target](AsyncResult<bool> outcome) {
        m_loading.publish(ticket, [this, target, outcome = std::move(outcome)] {
          if (const auto* failure = std::get_if<std::exception_ptr>(&outcome)) {
            emit stateChanged();
            fail(failureText(*failure));
            return;
          }
          load();
          m_selectAfterLoad = target;
        });
      });
  emit stateChanged();
}
}  // namespace uwf::presentation
#include "moc_FilePickerViewModel.cpp"
