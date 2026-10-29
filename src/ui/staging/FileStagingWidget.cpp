// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "FileStagingWidget.h"

#include <QAbstractItemView>
#include <QAbstractSlider>
#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QColor>
#include <QEvent>
#include <QFrame>
#include <QHBoxLayout>
#include <QItemSelectionModel>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QMenu>
#include <QPushButton>
#include <QScrollBar>
#include <QSizePolicy>
#include <QVBoxLayout>
#include <utility>

#include "presentation/common/ItemRoles.h"
#include "ui/appearance/I18n.h"
#include "ui/appearance/ThemeManager.h"
#include "ui/dialogs/Dialogs.h"
#include "ui/widgets/CommandMenu.h"
#include "ui/widgets/EntryListView.h"
#include "ui/widgets/PaginationBar.h"
#include "ui/widgets/RoundedCornerOverlay.h"
#include "ui/widgets/SearchField.h"
#include "ui/widgets/TextLabel.h"
#include "windows/system/WindowsShell.h"

namespace uwf::ui {

FileStagingWidget::FileStagingWidget(std::shared_ptr<presentation::FileStagingViewModel> viewModel, dialogs::FileDialogProvider& fileDialogs, QWidget* parent)
    : QWidget(parent), m_viewModel(std::move(viewModel)), m_fileDialogs(fileDialogs) {
  setObjectName(QStringLiteral("fileStagingWidget"));
  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 14, 0, 0);
  layout->setSpacing(14);

  m_commands = new QWidget(this);
  auto* header = new QHBoxLayout(m_commands);
  header->setContentsMargins(0, 0, 0, 0);
  header->setSpacing(3);
  m_filter = new SearchField(this);
  m_filter->setObjectName(QStringLiteral("filterInput"));
  I18n::bind(m_filter, [target = m_filter] { target->setPlaceholderText(I18n::tr("Search paths")); });
  m_filter->setClearButtonEnabled(true);
  header->addWidget(m_filter, 1);
  header->addSpacing(9);

  auto& theme = ThemeManager::instance();
  const QColor buttonIconColor(0xE8EAED);
  m_addButton = new QPushButton(theme.iconWithColor(":/icons/add.svg", buttonIconColor), QString{}, this);
  I18n::bind(m_addButton, [target = m_addButton] { target->setText(I18n::tr("Add")); });
  m_addButton->setObjectName(QStringLiteral("primaryBtn"));
  I18n::bind(m_addButton, [target = m_addButton] { target->setToolTip(I18n::tr("Add files or a folder to the automatic commit list.")); });
  auto* addMenu = new CommandMenu(m_addButton);
  m_addFileAction = addMenu->addAction(theme.icon(":/icons/file.svg"), QString{});
  I18n::bind(m_addFileAction, [target = m_addFileAction] { target->setText(I18n::tr("File…")); });
  m_addFileAction->setObjectName(QStringLiteral("stageFilesAction"));
  I18n::bind(m_addFileAction,
             [target = m_addFileAction] { target->setToolTip(I18n::tr("Select one or more files to commit automatically before safe shutdown or restart.")); });
  m_addDirectoryAction = addMenu->addAction(theme.icon(":/icons/folder.svg"), QString{});
  I18n::bind(m_addDirectoryAction, [target = m_addDirectoryAction] { target->setText(I18n::tr("Folder…")); });
  m_addDirectoryAction->setObjectName(QStringLiteral("stageDirectoryAction"));
  I18n::bind(m_addDirectoryAction, [target = m_addDirectoryAction] {
    target->setToolTip(I18n::tr("Select a folder whose files will be committed recursively before safe shutdown or restart."));
  });
  addMenu->setToolTipsVisible(true);
  m_addButton->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(m_addButton, &QWidget::customContextMenuRequested, addMenu,
          [this, addMenu](const QPoint& position) { addMenu->popup(m_addButton->mapToGlobal(position)); });
  connect(m_addButton, &QPushButton::clicked, this, [this] {
    if (!m_viewModel->writable()) return;
    const auto items = m_fileDialogs.openItems(
        this,
        {I18n::tr("Add files or folders"), dialogs::dialogBasePath(m_viewModel->driveLetter()), {}, {dialogs::dialogBasePath(m_viewModel->driveLetter())}});
    for (const auto& item : items) m_viewModel->addEntries({item.path}, item.directory ? app::FileStagingKind::Directory : app::FileStagingKind::File);
  });
  header->addWidget(m_addButton);

  m_removeButton = new QPushButton(theme.iconWithColor(":/icons/remove.svg", buttonIconColor), I18n::tr("Remove selected"), this);
  m_removeButton->setObjectName(QStringLiteral("dangerBtn"));
  I18n::bind(m_removeButton, [target = m_removeButton] { target->setToolTip(I18n::tr("Mark selected paths for removal. Takes effect after Apply.")); });
  header->addWidget(m_removeButton);
  layout->addWidget(m_commands);

  m_list = new EntryListView(I18n::tr("Staged path"), true, this);
  // 复用文件排除列表的 QSS 选择器，保证行高、圆角、hover 与选中态完全一致。
  m_list->setObjectName(QStringLiteral("exclusionList"));
  m_list->setSelectionMode(QAbstractItemView::ExtendedSelection);
  m_list->setAlternatingRowColors(false);
  m_list->setUniformItemSizes(true);
  m_list->setIconSize({16, 16});
  m_list->setModel(m_viewModel.get());
  m_list->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_list->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Expanding);
  m_list->setMinimumHeight(80);
  m_list->setMinimumWidth(0);
  auto* listStack = new QVBoxLayout();
  listStack->setSpacing(design::listPaginationGap);
  listStack->addWidget(m_list, 1);
  m_pagination = new PaginationBar(static_cast<EntryListView*>(m_list)->pages(), this);
  m_pagination->setObjectName(QStringLiteral("entryPagination"));
  listStack->addWidget(m_pagination);
  layout->addLayout(listStack, 1);
  m_removeButton->setProperty("accessibleName", m_removeButton->text());
  m_removeButton->setText(QString());
  m_removeButton->setFixedWidth(32);
  m_filter->setMinimumWidth(80);
  m_filter->setFixedHeight(30);
  for (const auto buttons = m_commands->findChildren<QPushButton*>(); auto* button : buttons) {
    button->setProperty("designRole", button == m_addButton ? "secondary" : "dangerQuiet");
    button->setFixedHeight(30);
    auto type = button->font();
    type.setPixelSize(12);
    button->setFont(type);
  }

  m_cornerOverlay = new RoundedCornerOverlay(
      m_list, 0, 6, [] { return ThemeManager::instance().color(Sem::Bg); }, [] { return ThemeManager::instance().color(Sem::Border); }, 1);
  m_cornerOverlay->syncToParent();
  m_list->installEventFilter(this);
  connect(m_list->verticalScrollBar(), &QAbstractSlider::valueChanged, m_cornerOverlay, QOverload<>::of(&QWidget::update));

  connect(m_addFileAction, &QAction::triggered, this, &FileStagingWidget::addFiles);
  connect(m_addDirectoryAction, &QAction::triggered, this, &FileStagingWidget::addDirectory);
  connect(m_removeButton, &QPushButton::clicked, this, &FileStagingWidget::removeSelected);
  connect(m_filter, &QLineEdit::textChanged, this, &FileStagingWidget::filterChanged);
  connect(m_list->selectionModel(), &QItemSelectionModel::selectionChanged, this, [this] {
    updateModificationControls();
    const auto selected = m_list->selectionModel()->selectedRows();
    emit selectionDetailChanged(selected.isEmpty()     ? QString()
                                : selected.size() == 1 ? selected.front().data(Qt::ToolTipRole).toString().simplified()
                                                       : I18n::tr("%1 selected").arg(selected.size()));
  });
  connect(m_list, &QListView::doubleClicked, this, &FileStagingWidget::copySelectedPath);
  connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [this](app::Theme) { refreshThemedIcons(); });

  m_list->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(m_list, &QListView::customContextMenuRequested, this, [this](const QPoint& position) {
    const auto index = m_list->indexAt(position);
    if (!index.isValid()) return;
    if (!m_list->selectionModel()->isSelected(index)) m_list->selectionModel()->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
    const QString path = index.data(presentation::PathRole).toString();
    CommandMenu menu(this);
    auto* open = menu.addAction(ThemeManager::instance().icon(":/icons/folder.svg"), I18n::tr("Open containing folder"));
    connect(open, &QAction::triggered, this, [path] { windows::shell::revealInExplorer(path); });
    auto* copy = menu.addAction(ThemeManager::instance().icon(":/icons/copy.svg"), I18n::tr("Copy file path"));
    connect(copy, &QAction::triggered, this, [this] {
      QStringList paths;
      for (const auto selected = m_list->selectionModel()->selectedRows(); const auto& item : selected)
        paths.append(item.data(presentation::PathRole).toString());
      QApplication::clipboard()->setText(paths.join(u'\n'));
      emit copiedToClipboard();
    });
    menu.addSeparator();
    const bool restore = presentation::isRemovedSelection(m_list->selectionModel()->selectedRows());
    auto* remove = menu.addAction(ThemeManager::instance().icon(restore ? ":/icons/restart.svg" : ":/icons/remove.svg"),
                                  m_removeButton->property("accessibleName").toString(), this, &FileStagingWidget::removeSelected);
    remove->setEnabled(m_removeButton->isEnabled());
    if (!restore) remove->setProperty("designRole", "destructive");
    menu.exec(m_list->viewport()->mapToGlobal(position));
  });

  connect(m_viewModel.get(), &presentation::FileStagingViewModel::entriesChanged, this, &FileStagingWidget::rebuild);
  connect(m_viewModel.get(), &presentation::FileStagingViewModel::storageUnavailable, this, [this] { setStorageUnavailable(m_viewModel->storageError()); });
  connect(m_viewModel.get(), &presentation::FileStagingViewModel::warningRaised, this,
          [this](const QString& title, const QString& body) { dialogs::warning(this, title, body); });
  connect(m_viewModel.get(), &presentation::FileStagingViewModel::modificationAvailabilityChanged, this, &FileStagingWidget::updateModificationControls);
  connect(m_viewModel.get(), &presentation::FileStagingViewModel::storageAvailabilityChanged, this, [this] {
    m_filter->setEnabled(m_viewModel->storageAvailable());
    m_list->setEnabled(m_viewModel->storageAvailable());
    rebuild();
  });
  connect(m_viewModel.get(), &presentation::FileStagingViewModel::enhancedModeAvailabilityChanged, this, &FileStagingWidget::enhancedModeAvailabilityChanged);
  refreshThemedIcons();
  m_viewModel->load();
  updateModificationControls();
  I18n::bind(this, [this] {
    static_cast<EntryListView*>(m_list)->setHeading(I18n::tr("Staged path"));
    m_viewModel->retranslate();
    renderSummary();
    updateModificationControls();
    const auto selected = m_list->selectionModel()->selectedRows();
    if (!selected.isEmpty())
      emit selectionDetailChanged(selected.size() == 1 ? selected.constFirst().data(Qt::ToolTipRole).toString().simplified()
                                                       : I18n::tr("%1 selected").arg(selected.size()));
  });
}

void FileStagingWidget::setReadOnly(const bool readOnly) { m_viewModel->setReadOnly(readOnly); }

void FileStagingWidget::setEnhancedModeAvailable(const bool available) { m_viewModel->setEnhancedModeAvailable(available); }

bool FileStagingWidget::eventFilter(QObject* watched, QEvent* event) {
  if (watched == m_list && event->type() == QEvent::Resize && m_cornerOverlay) m_cornerOverlay->syncToParent();
  return QWidget::eventFilter(watched, event);
}

void FileStagingWidget::addFiles() {
  if (!m_viewModel->writable()) return;
  m_viewModel->addEntries(m_fileDialogs.openFiles(this, {I18n::tr("Select files for automatic commit"),
                                                         dialogs::dialogBasePath(m_viewModel->driveLetter()),
                                                         {},
                                                         {dialogs::dialogBasePath(m_viewModel->driveLetter())}}),
                          app::FileStagingKind::File);
}

void FileStagingWidget::addDirectory() {
  if (!m_viewModel->writable()) return;
  const QString path = m_fileDialogs.selectDirectory(this, {I18n::tr("Select a folder for automatic recursive commit"),
                                                            dialogs::dialogBasePath(m_viewModel->driveLetter()),
                                                            {},
                                                            {dialogs::dialogBasePath(m_viewModel->driveLetter())}});
  if (!path.isEmpty()) m_viewModel->addEntries({path}, app::FileStagingKind::Directory);
}

void FileStagingWidget::removeSelected() {
  if (!m_viewModel->writable()) return;
  QStringList paths;
  const auto selected = m_list->selectionModel()->selectedRows();
  for (const auto& index : selected) paths.append(index.data(presentation::PathRole).toString());
  const bool restore = presentation::isRemovedSelection(selected);
  if (restore)
    m_viewModel->restorePaths(paths);
  else
    m_viewModel->removePaths(paths);
}

void FileStagingWidget::filterChanged(const QString& text) { static_cast<EntryListView*>(m_list)->setFilterText(text.trimmed()); }

void FileStagingWidget::copySelectedPath(const QModelIndex& index) {
  if (!index.isValid()) return;
  const QString path = index.data(presentation::PathRole).toString();
  QApplication::clipboard()->setText(path);
  emit copiedToClipboard();
}

void FileStagingWidget::rebuild() {
  m_list->clearSelection();
  m_list->setCurrentIndex({});
  renderSummary();
  filterChanged(m_filter->text());
}

void FileStagingWidget::renderSummary() {
  if (!m_viewModel->storageAvailable()) {
    setStorageUnavailable(m_viewModel->storageError());
    return;
  }
  m_list->viewport()->update();
  m_pagination->setSummary(m_viewModel->summaryText());
}

void FileStagingWidget::refreshThemedIcons() {
  const QColor buttonIconColor(0xE8EAED);
  auto& theme = ThemeManager::instance();
  m_addButton->setIcon(theme.iconWithColor(":/icons/add.svg", buttonIconColor));
  m_removeButton->setIcon(theme.iconWithColor(":/icons/remove.svg", buttonIconColor));
  m_addFileAction->setIcon(theme.icon(":/icons/file.svg"));
  m_addDirectoryAction->setIcon(theme.icon(":/icons/folder.svg"));
  m_list->viewport()->update();
  updateModificationControls();
}

void FileStagingWidget::updateModificationControls() {
  const bool writable = m_viewModel->writable();
  const auto selected = m_list->selectionModel()->selectedRows();
  const bool restore = presentation::isRemovedSelection(selected);
  m_removeButton->setProperty("accessibleName", restore ? I18n::tr("Undo removal") : I18n::tr("Remove selected"));
  m_removeButton->setIcon(ThemeManager::instance().icon(restore ? ":/icons/restart.svg" : ":/icons/remove.svg"));
  m_removeButton->setProperty("designRole", restore ? "quiet" : "dangerQuiet");
  m_addButton->setEnabled(writable);
  m_removeButton->setEnabled(writable && m_list->selectionModel()->hasSelection());
  m_addFileAction->setEnabled(writable);
  m_addDirectoryAction->setEnabled(writable);
}

void FileStagingWidget::setStorageUnavailable(const QString& error) {
  updateModificationControls();
  m_filter->setEnabled(false);
  m_list->setEnabled(false);
  m_pagination->setSummary(I18n::tr("File staging is unavailable: %1").arg(error));
}

}  // namespace uwf::ui

#include "moc_FileStagingWidget.cpp"
