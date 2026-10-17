/*
 * Copyright (c) 2026 HsingYun (iakext@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "FileStagingWidget.h"

#include <QAbstractItemView>
#include <QAbstractSlider>
#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QColor>
#include <QDir>
#include <QEvent>
#include <QHBoxLayout>
#include <QItemSelectionModel>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QPushButton>
#include <QScrollBar>
#include <QSizePolicy>
#include <QVBoxLayout>
#include <algorithm>
#include <exception>
#include <filesystem>
#include <stdexcept>
#include <utility>

#include "../app/FileStagingConflictPolicy.h"
#include "../util/DriveLetter.h"
#include "../util/Log.h"
#include "../util/PathMatch.h"
#include "../util/WindowsFileSystem.h"
#include "Dialogs.h"
#include "I18n.h"
#include "RoundedCornerOverlay.h"
#include "ThemeManager.h"
#include "UiUtil.h"

namespace uwf::ui {

namespace {

QString normalizePath(const QString& path) {
  if (path.isEmpty()) return {};
  return QDir::toNativeSeparators(QDir::cleanPath(QDir::fromNativeSeparators(path)));
}

QString identityOf(const QString& path) { return normalizePath(path).toCaseFolded(); }

void sortEntries(QList<app::FileStagingEntry>& entries) {
  std::ranges::sort(entries, [](const app::FileStagingEntry& lhs, const app::FileStagingEntry& rhs) {
    return QString::compare(lhs.path, rhs.path, Qt::CaseInsensitive) < 0;
  });
}

bool isDriveRoot(const QString& path) {
  const drive::PathSplit split = drive::split(path.toStdString());
  return !split.letter.empty() && stripTrailingSep(split.rest).empty();
}

bool isSameOrDescendant(const QString& rawPath, const QString& rawParent) {
  const QString path = identityOf(rawPath);
  QString parent = identityOf(rawParent);
  while (parent.endsWith(u'\\')) parent.chop(1);
  if (path == parent) return true;
  return path.size() > parent.size() && path.startsWith(parent) && path[parent.size()] == u'\\';
}

bool isCoveredByParentDirectory(const QList<app::FileStagingEntry>& entries, const QString& path) {
  const QString identity = identityOf(path);
  return std::ranges::any_of(entries, [&](const app::FileStagingEntry& entry) {
    return entry.kind == app::FileStagingKind::Directory && identityOf(entry.path) != identity && isSameOrDescendant(path, entry.path);
  });
}

QList<app::FileStagingEntry> entriesForDrive(const QList<app::FileStagingEntry>& entries, const QString& driveLetter) {
  QList<app::FileStagingEntry> filtered;
  for (const auto& entry : entries) {
    QString entryDrive;
    try {
      entryDrive = QString::fromStdString(drive::fromPath(entry.path.toStdString()));
    } catch (const std::exception&) {
      // 已断开或已失效的卷 GUID 没有对应磁盘页；保留在全局存储中，待卷重新
      // 出现后即可重新归入其页面，不能因为单条路径暂时无法解析而禁用所有页。
      continue;
    }
    if (entryDrive.compare(driveLetter, Qt::CaseInsensitive) == 0) filtered.append(entry);
  }
  return filtered;
}

}  // namespace

FileStagingWidget::FileStagingWidget(QString driveLetter, app::FileStagingStore& store, QWidget* parent)
    : FileStagingWidget(std::move(driveLetter), store, dialogs::systemFileDialogs(), nullptr, parent) {}

FileStagingWidget::FileStagingWidget(QString driveLetter, app::FileStagingStore& store, dialogs::FileDialogProvider& fileDialogs, QWidget* parent)
    : FileStagingWidget(std::move(driveLetter), store, fileDialogs, nullptr, parent) {}

FileStagingWidget::FileStagingWidget(QString driveLetter, app::FileStagingStore& store, dialogs::FileDialogProvider& fileDialogs,
                                     app::FileStagingConflictPolicy& conflicts, QWidget* parent)
    : FileStagingWidget(std::move(driveLetter), store, fileDialogs, &conflicts, parent) {}

FileStagingWidget::FileStagingWidget(QString driveLetter, app::FileStagingStore& store, dialogs::FileDialogProvider& fileDialogs,
                                     app::FileStagingConflictPolicy* conflicts, QWidget* parent)
    : QWidget(parent),
      m_driveLetter(QString::fromStdString(drive::normalize(driveLetter.toStdString()))),
      m_store(store),
      m_fileDialogs(fileDialogs),
      m_conflicts(conflicts) {
  if (m_driveLetter.isEmpty()) throw std::invalid_argument("file staging requires a valid drive letter");

  setObjectName(QStringLiteral("fileStagingWidget"));
  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(8);

  auto* header = new QHBoxLayout();
  m_filter = new QLineEdit(this);
  m_filter->setObjectName(QStringLiteral("filterInput"));
  m_filter->setPlaceholderText(I18n::tr("Search staged paths…"));
  m_filter->setClearButtonEnabled(true);
  header->addWidget(m_filter, 1);

  auto& theme = ThemeManager::instance();
  const QColor buttonIconColor(0xE8EAED);
  m_addButton = new QPushButton(theme.iconWithColor(":/icons/add.svg", buttonIconColor), I18n::tr("Add"), this);
  m_addButton->setObjectName(QStringLiteral("primaryBtn"));
  m_addButton->setToolTip(I18n::tr("Add files or a folder to the automatic commit list."));
  auto* addMenu = new QMenu(m_addButton);
  m_addFileAction = addMenu->addAction(theme.icon(":/icons/file.svg"), I18n::tr("File…"));
  m_addFileAction->setObjectName(QStringLiteral("stageFilesAction"));
  m_addFileAction->setToolTip(I18n::tr("Select one or more files to commit automatically before safe shutdown or restart."));
  m_addDirectoryAction = addMenu->addAction(theme.icon(":/icons/folder.svg"), I18n::tr("Folder…"));
  m_addDirectoryAction->setObjectName(QStringLiteral("stageDirectoryAction"));
  m_addDirectoryAction->setToolTip(I18n::tr("Select a folder whose files will be committed recursively before safe shutdown or restart."));
  addMenu->setToolTipsVisible(true);
  m_addButton->setMenu(addMenu);
  header->addWidget(m_addButton);

  m_removeButton = new QPushButton(theme.iconWithColor(":/icons/remove.svg", buttonIconColor), I18n::tr("Remove selected"), this);
  m_removeButton->setObjectName(QStringLiteral("dangerBtn"));
  m_removeButton->setToolTip(I18n::tr("Remove the selected paths from automatic commit immediately."));
  header->addWidget(m_removeButton);
  layout->addLayout(header);

  m_summary = new QLabel(this);
  m_summary->setObjectName(QStringLiteral("diffSummary"));
  m_summary->setWordWrap(true);
  m_summary->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  layout->addWidget(m_summary);

  m_list = new QListWidget(this);
  // 复用文件排除列表的 QSS 选择器，保证行高、圆角、hover 与选中态完全一致。
  m_list->setObjectName(QStringLiteral("exclusionList"));
  m_list->setSelectionMode(QAbstractItemView::ExtendedSelection);
  m_list->setAlternatingRowColors(false);
  m_list->setUniformItemSizes(true);
  m_list->setIconSize({16, 16});
  m_list->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Expanding);
  m_list->setMinimumHeight(80);
  m_list->setMinimumWidth(0);
  layout->addWidget(m_list, 1);

  m_cornerOverlay = new RoundedCornerOverlay(
      m_list, 0, 6, [] { return ThemeManager::instance().color(Sem::Bg); }, [] { return ThemeManager::instance().color(Sem::Border); }, 1);
  m_cornerOverlay->syncToParent();
  m_list->installEventFilter(this);
  connect(m_list->verticalScrollBar(), &QAbstractSlider::valueChanged, m_cornerOverlay, QOverload<>::of(&QWidget::update));

  connect(m_addFileAction, &QAction::triggered, this, &FileStagingWidget::addFiles);
  connect(m_addDirectoryAction, &QAction::triggered, this, &FileStagingWidget::addDirectory);
  connect(m_removeButton, &QPushButton::clicked, this, &FileStagingWidget::removeSelected);
  connect(m_filter, &QLineEdit::textChanged, this, &FileStagingWidget::filterChanged);
  connect(m_list, &QListWidget::itemDoubleClicked, this, &FileStagingWidget::copySelectedPath);
  connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [this](Theme) { refreshThemedIcons(); });

  m_list->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(m_list, &QListWidget::customContextMenuRequested, this, [this](const QPoint& position) {
    auto* item = m_list->itemAt(position);
    if (!item) return;
    m_list->setCurrentItem(item, QItemSelectionModel::ClearAndSelect);
    const QString path = item->data(Qt::UserRole).toString();
    QMenu menu(this);
    auto* open = menu.addAction(ThemeManager::instance().icon(":/icons/folder.svg"), I18n::tr("Open containing folder"));
    connect(open, &QAction::triggered, this, [path] { revealInExplorer(path); });
    auto* copy = menu.addAction(I18n::tr("Copy file path"));
    connect(copy, &QAction::triggered, this, [this, path] {
      QApplication::clipboard()->setText(path);
      emit copiedToClipboard(I18n::tr("Copied to clipboard: ") + path);
    });
    menu.exec(m_list->viewport()->mapToGlobal(position));
  });

  load();
}

void FileStagingWidget::load() {
  try {
    const auto storedEntries = m_store.load();
    if (m_conflicts) m_conflicts->setStagedEntries(storedEntries);
    m_entries = entriesForDrive(storedEntries, m_driveLetter);
    sortEntries(m_entries);
    m_storageAvailable = true;
    rebuild();
  } catch (const std::exception& error) {
    setStorageUnavailable(QString::fromUtf8(error.what()));
  }
}

void FileStagingWidget::setReadOnly(const bool readOnly) {
  if (m_readOnly == readOnly) return;
  m_readOnly = readOnly;
  updateModificationControls();
}

bool FileStagingWidget::eventFilter(QObject* watched, QEvent* event) {
  if (watched == m_list && event->type() == QEvent::Resize && m_cornerOverlay) m_cornerOverlay->syncToParent();
  return QWidget::eventFilter(watched, event);
}

void FileStagingWidget::addFiles() {
  if (!m_storageAvailable || m_readOnly) return;
  addEntries(m_fileDialogs.openFiles(this, {I18n::tr("Select files for automatic commit"), dialogs::dialogBasePath(m_driveLetter), {}}),
             app::FileStagingKind::File);
}

void FileStagingWidget::addDirectory() {
  if (!m_storageAvailable || m_readOnly) return;
  const QString path =
      m_fileDialogs.selectDirectory(this, {I18n::tr("Select a folder for automatic recursive commit"), dialogs::dialogBasePath(m_driveLetter), {}});
  if (!path.isEmpty()) addEntries({path}, app::FileStagingKind::Directory);
}

void FileStagingWidget::addEntries(const QStringList& paths, const app::FileStagingKind kind) {
  QList<app::FileStagingEntry> candidate;
  try {
    candidate = m_store.load();
  } catch (const std::exception& error) {
    reportUpdateFailure(QString::fromUtf8(error.what()));
    return;
  }

  bool changed = false;
  for (const auto& rawPath : paths) {
    const QString path = normalizePath(rawPath);
    if (path.isEmpty()) continue;
    if (!app::isAbsoluteLocalFileStagingPath(path)) {
      dialogs::warning(this, I18n::tr("Cannot stage this path"),
                       I18n::tr("Only absolute paths on local volumes can be added to automatic file staging:\n%1").arg(path));
      continue;
    }
    const std::filesystem::path nativePath(path.toStdWString());
    winfs::EntryType entryType;
    try {
      entryType = winfs::inspect(nativePath);
    } catch (const std::exception& error) {
      dialogs::warning(this, I18n::tr("Cannot stage this path"),
                       I18n::tr("The selected path could not be inspected:\n%1").arg(QString::fromUtf8(error.what())));
      continue;
    }
    if (entryType == winfs::EntryType::ReparsePoint) {
      dialogs::warning(this, I18n::tr("Cannot stage this path"), I18n::tr("Reparse points cannot be added to automatic file staging:\n%1").arg(path));
      continue;
    }
    const bool expectedType = kind == app::FileStagingKind::File ? entryType == winfs::EntryType::File : entryType == winfs::EntryType::Directory;
    if (!expectedType) {
      dialogs::warning(this, I18n::tr("Cannot stage this path"), I18n::tr("The selected path does not exist or is not the expected file type:\n%1").arg(path));
      continue;
    }
    try {
      const QString targetDrive = QString::fromStdString(drive::fromPath(path.toStdString()));
      if (targetDrive.isEmpty()) {
        dialogs::warning(this, I18n::tr("Cannot stage this path"), I18n::tr("The path has no drive letter and cannot be committed by UWF:\n%1").arg(path));
        continue;
      }
      if (targetDrive.compare(m_driveLetter, Qt::CaseInsensitive) != 0) {
        dialogs::warning(this, I18n::tr("Cannot stage this path"),
                         I18n::tr("The selected path is on volume %1. Add it from that volume's File staging tab.").arg(targetDrive));
        continue;
      }
    } catch (const std::exception& error) {
      dialogs::warning(this, I18n::tr("Cannot stage this path"), I18n::tr("Failed to resolve the target volume: %1").arg(QString::fromUtf8(error.what())));
      continue;
    }
    if (kind == app::FileStagingKind::Directory && isDriveRoot(path)) {
      dialogs::warning(this, I18n::tr("Cannot stage this path"),
                       I18n::tr("A volume root cannot be staged because it would recursively commit the entire volume."));
      continue;
    }
    if (m_conflicts) {
      if (const auto conflict = m_conflicts->conflictingExclusion(path)) {
        dialogs::warning(this, I18n::tr("Cannot stage this path"),
                         I18n::tr("This path overlaps a UWF file exclusion. Remove the exclusion before adding the staged path:\n%1").arg(*conflict));
        continue;
      }
    }
    if (isCoveredByParentDirectory(candidate, path)) continue;

    const QString identity = identityOf(path);
    const auto duplicate = std::ranges::find_if(candidate, [&](const app::FileStagingEntry& entry) { return identityOf(entry.path) == identity; });
    if (duplicate != candidate.end()) {
      if (duplicate->kind == kind) continue;
      candidate.erase(duplicate);
      changed = true;
    }

    if (kind == app::FileStagingKind::Directory) {
      const qsizetype removed = candidate.removeIf([&](const app::FileStagingEntry& entry) { return isSameOrDescendant(entry.path, path); });
      changed = changed || removed > 0;
    }
    candidate.append({kind, path});
    changed = true;
  }
  if (changed) persist(candidate);
}

void FileStagingWidget::removeSelected() {
  if (!m_storageAvailable || m_readOnly) return;
  const auto selected = m_list->selectedItems();
  if (selected.isEmpty()) return;

  QList<app::FileStagingEntry> candidate;
  try {
    candidate = m_store.load();
  } catch (const std::exception& error) {
    reportUpdateFailure(QString::fromUtf8(error.what()));
    return;
  }

  qsizetype removed = 0;
  for (const auto* item : selected) {
    const QString identity = identityOf(item->data(Qt::UserRole).toString());
    removed += candidate.removeIf([&](const app::FileStagingEntry& entry) { return identityOf(entry.path) == identity; });
  }
  if (removed > 0) {
    persist(candidate);
  } else {
    m_entries = entriesForDrive(candidate, m_driveLetter);
    sortEntries(m_entries);
    rebuild();
  }
}

void FileStagingWidget::persist(const QList<app::FileStagingEntry>& entries) {
  try {
    QList<app::FileStagingEntry> ordered = entries;
    sortEntries(ordered);
    m_store.replace(ordered);
    // 生产存储的清空操作还包含 UWF CommitRegistryDeletion。若持久化边界报告
    // 部分失败，必须回读权威状态，不能继续展示候选列表冒充已落盘结果。
    const auto storedEntries = m_store.load();
    if (m_conflicts) m_conflicts->setStagedEntries(storedEntries);
    m_entries = entriesForDrive(storedEntries, m_driveLetter);
    sortEntries(m_entries);
    rebuild();
  } catch (const std::exception& error) {
    try {
      const auto storedEntries = m_store.load();
      if (m_conflicts) m_conflicts->setStagedEntries(storedEntries);
      m_entries = entriesForDrive(storedEntries, m_driveLetter);
      sortEntries(m_entries);
      rebuild();
    } catch (...) {
      setStorageUnavailable(QString::fromUtf8(error.what()));
      return;
    }
    reportUpdateFailure(QString::fromUtf8(error.what()));
  }
}

void FileStagingWidget::filterChanged(const QString& text) {
  const QString needle = text.trimmed().toCaseFolded();
  for (int index = 0; index < m_list->count(); ++index) {
    auto* item = m_list->item(index);
    item->setHidden(!needle.isEmpty() && !item->text().toCaseFolded().contains(needle));
  }
}

void FileStagingWidget::copySelectedPath(QListWidgetItem* item) {
  if (!item) return;
  const QString path = item->data(Qt::UserRole).toString();
  QApplication::clipboard()->setText(path);
  emit copiedToClipboard(I18n::tr("Copied to clipboard: ") + path);
}

void FileStagingWidget::rebuild() {
  m_list->clear();
  int files = 0;
  int directories = 0;
  auto& theme = ThemeManager::instance();
  for (const auto& entry : std::as_const(m_entries)) {
    const bool directory = entry.kind == app::FileStagingKind::Directory;
    if (directory)
      ++directories;
    else
      ++files;
    auto* item = new QListWidgetItem(theme.icon(directory ? ":/icons/folder.svg" : ":/icons/file.svg"), entry.path);
    item->setData(Qt::UserRole, entry.path);
    item->setToolTip((directory ? I18n::tr("Folder: %1") : I18n::tr("File: %1")).arg(entry.path) + '\n' +
                     I18n::tr("Committed automatically before safe shutdown or restart."));
    m_list->addItem(item);
  }
  m_summary->setText(I18n::tr("%1 staged entries · %2 files · %3 folders").arg(m_entries.size()).arg(files).arg(directories));
  filterChanged(m_filter->text());
}

void FileStagingWidget::refreshThemedIcons() {
  const QColor buttonIconColor(0xE8EAED);
  auto& theme = ThemeManager::instance();
  m_addButton->setIcon(theme.iconWithColor(":/icons/add.svg", buttonIconColor));
  m_removeButton->setIcon(theme.iconWithColor(":/icons/remove.svg", buttonIconColor));
  m_addFileAction->setIcon(theme.icon(":/icons/file.svg"));
  m_addDirectoryAction->setIcon(theme.icon(":/icons/folder.svg"));
  rebuild();
}

void FileStagingWidget::updateModificationControls() {
  const bool writable = m_storageAvailable && !m_readOnly;
  m_addButton->setEnabled(writable);
  m_removeButton->setEnabled(writable);
  m_addFileAction->setEnabled(writable);
  m_addDirectoryAction->setEnabled(writable);
}

void FileStagingWidget::setStorageUnavailable(const QString& error) {
  m_storageAvailable = false;
  updateModificationControls();
  m_filter->setEnabled(false);
  m_list->setEnabled(false);
  m_summary->setText(I18n::tr("File staging is unavailable: %1").arg(error));
  UWF_LOG_E("staging") << "file staging registry read failed: error=" << error.toStdString();
}

void FileStagingWidget::reportUpdateFailure(const QString& error) {
  UWF_LOG_E("staging") << "file staging update failed: error=" << error.toStdString();
  dialogs::warning(this, I18n::tr("File staging could not be saved"),
                   I18n::tr("The file staging state was reloaded after the update could not be completed:\n%1").arg(error));
}

}  // namespace uwf::ui
