// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "presentation/views/staging/FileStagingWidget.h"

#include <QAction>
#include <QItemSelectionModel>
#include <QLineEdit>
#include <QListView>
#include <QMenu>
#include <QPointer>
#include <QPushButton>
#include <utility>

#include "infrastructure/windows/shell/WindowsShell.h"
#include "presentation/appearance/I18n.h"
#include "presentation/appearance/ThemeManager.h"
#include "presentation/controls/clipboard/ClipboardText.h"
#include "presentation/controls/navigation/CommandMenu.h"
#include "presentation/controls/pagination/PaginationBar.h"
#include "presentation/controls/tables/PathEntryListView.h"
#include "presentation/dialogs/Dialogs.h"
#include "presentation/models/ItemRoles.h"

namespace uwf::ui {

FileStagingWidget::FileStagingWidget(std::shared_ptr<presentation::FileStagingViewModel> viewModel, dialogs::FileDialogProvider& fileDialogs, QWidget* parent)
    : QWidget(parent), m_viewModel(std::move(viewModel)), m_fileDialogs(fileDialogs) {
  setObjectName(QStringLiteral("fileStagingWidget"));
  auto& theme = ThemeManager::instance();
  m_entries = createPathEntryControls(*this, *m_viewModel, I18n::tr("Staged path"), I18n::tr("Status"));
  m_entries.list->setObjectName(QStringLiteral("stagingEntries"));
  I18n::bind(m_entries.filter, [target = m_entries.filter] { target->setPlaceholderText(I18n::tr("Search paths")); });
  I18n::bind(m_entries.add, [target = m_entries.add] { target->setText(I18n::tr("Add")); });
  auto* addMenu = new CommandMenu(m_entries.add);
  m_addFileAction = addMenu->addAction(theme.icon(":/icons/file.svg"), QString{});
  I18n::bind(m_addFileAction, [target = m_addFileAction] { target->setText(I18n::tr("File…")); });
  m_addFileAction->setObjectName(QStringLiteral("stageFilesAction"));
  m_addDirectoryAction = addMenu->addAction(theme.icon(":/icons/folder.svg"), QString{});
  I18n::bind(m_addDirectoryAction, [target = m_addDirectoryAction] { target->setText(I18n::tr("Folder…")); });
  m_addDirectoryAction->setObjectName(QStringLiteral("stageDirectoryAction"));
  addMenu->setToolTipsVisible(true);
  m_entries.add->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(m_entries.add, &QWidget::customContextMenuRequested, addMenu,
          [this, addMenu](const QPoint& position) { addMenu->popup(m_entries.add->mapToGlobal(position)); });
  connect(m_entries.add, &QPushButton::clicked, this, [this] {
    if (!m_viewModel->canAddEntries()) return;
    auto items = m_fileDialogs.openItems(
        this,
        {I18n::tr("Add files or folders"), dialogs::dialogBasePath(m_viewModel->driveLetter()), {}, {dialogs::dialogBasePath(m_viewModel->driveLetter())}});
    QList<app::FileStagingEntry> entries;
    entries.reserve(items.size());
    for (const auto& item : std::as_const(items)) entries.append({item.directory ? app::FileStagingKind::Directory : app::FileStagingKind::File, item.path});
    m_viewModel->addEntries(entries);
  });
  connect(m_addFileAction, &QAction::triggered, this, &FileStagingWidget::addFiles);
  connect(m_addDirectoryAction, &QAction::triggered, this, &FileStagingWidget::addDirectory);
  connect(m_entries.remove, &QPushButton::clicked, this, &FileStagingWidget::removeSelected);
  connect(m_entries.filter, &QLineEdit::textChanged, this, &FileStagingWidget::filterChanged);
  connect(m_entries.list->selectionModel(), &QItemSelectionModel::selectionChanged, this, [this] {
    renderState();
    const auto selected = m_entries.list->selectionModel()->selectedRows();
    emit selectionDetailChanged(selected.isEmpty()     ? QString()
                                : selected.size() == 1 ? selected.front().data(Qt::ToolTipRole).toString().simplified()
                                                       : I18n::tr("%1 selected").arg(selected.size()));
  });
  connect(m_entries.list, &QListView::doubleClicked, this, &FileStagingWidget::copySelectedPath);
  connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [this](app::Theme) { refreshThemedIcons(); });

  m_entries.list->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(m_entries.list, &QListView::customContextMenuRequested, this, [this](const QPoint& position) {
    const auto index = m_entries.list->indexAt(position);
    if (!index.isValid()) return;
    if (!m_entries.list->selectionModel()->isSelected(index)) m_entries.list->selectionModel()->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
    const QString path = index.data(presentation::PathRole).toString();
    CommandMenu menu(this);
    auto* open = menu.addAction(ThemeManager::instance().icon(":/icons/folder.svg"), I18n::tr("Open containing folder"));
    connect(open, &QAction::triggered, this, [path] { windows::shell::revealInExplorer(path); });
    auto* copy = menu.addAction(ThemeManager::instance().icon(":/icons/copy.svg"), I18n::tr("Copy file path"));
    connect(copy, &QAction::triggered, this, [this] {
      QStringList paths;
      for (auto selected = m_entries.list->selectionModel()->selectedRows(); const auto& item : std::as_const(selected))
        paths.append(item.data(presentation::PathRole).toString());
      const QPointer self(this);
      if (writeClipboardText(paths.join(u'\n')) == ClipboardWriteResult::Confirmed && self) emit copiedToClipboard();
    });
    menu.addSeparator();
    const bool restore = presentation::isRemovedSelection(m_entries.list->selectionModel()->selectedRows());
    auto* remove = menu.addAction(ThemeManager::instance().icon(restore ? ":/icons/restart.svg" : ":/icons/remove.svg"),
                                  m_entries.remove->property("accessibleName").toString(), this, &FileStagingWidget::removeSelected);
    remove->setEnabled(m_entries.remove->isEnabled());
    if (!restore) remove->setProperty("designRole", "destructive");
    menu.exec(m_entries.list->viewport()->mapToGlobal(position));
  });

  connect(m_viewModel.get(), &presentation::FileStagingViewModel::entriesChanged, this, &FileStagingWidget::rebuild);
  connect(m_viewModel.get(), &presentation::FileStagingViewModel::storageUnavailable, this, &FileStagingWidget::renderState);
  connect(m_viewModel.get(), &presentation::FileStagingViewModel::warningRaised, this,
          [this](const QString& title, const QString& body) { dialogs::warning(this, title, body); });
  connect(m_viewModel.get(), &presentation::FileStagingViewModel::modificationAvailabilityChanged, this, &FileStagingWidget::renderState);
  connect(m_viewModel.get(), &presentation::FileStagingViewModel::storageAvailabilityChanged, this, &FileStagingWidget::renderState);
  connect(m_viewModel.get(), &presentation::FileStagingViewModel::enhancedModeAvailabilityChanged, this, &FileStagingWidget::enhancedModeAvailabilityChanged);
  connect(m_viewModel.get(), &presentation::FileStagingViewModel::automaticCommitStateChanged, this, &FileStagingWidget::renderState);
  refreshThemedIcons();
  m_viewModel->load();
  I18n::bind(this, [this] {
    m_entries.list->setHeading(I18n::tr("Staged path"));
    m_entries.list->setStatusHeading(I18n::tr("Status"));
    m_viewModel->retranslate();
    renderState();
    const auto selected = m_entries.list->selectionModel()->selectedRows();
    if (!selected.isEmpty())
      emit selectionDetailChanged(selected.size() == 1 ? selected.constFirst().data(Qt::ToolTipRole).toString().simplified()
                                                       : I18n::tr("%1 selected").arg(selected.size()));
  });
}

void FileStagingWidget::addFiles() {
  if (!m_viewModel->canAddEntries()) return;
  auto paths = m_fileDialogs.openFiles(this, {I18n::tr("Select files for automatic commit"),
                                              dialogs::dialogBasePath(m_viewModel->driveLetter()),
                                              {},
                                              {dialogs::dialogBasePath(m_viewModel->driveLetter())}});
  QList<app::FileStagingEntry> entries;
  entries.reserve(paths.size());
  for (const auto& path : std::as_const(paths)) entries.append({app::FileStagingKind::File, path});
  m_viewModel->addEntries(entries);
}

void FileStagingWidget::addDirectory() {
  if (!m_viewModel->canAddEntries()) return;
  const QString path = m_fileDialogs.selectDirectory(this, {I18n::tr("Select a folder for automatic recursive commit"),
                                                            dialogs::dialogBasePath(m_viewModel->driveLetter()),
                                                            {},
                                                            {dialogs::dialogBasePath(m_viewModel->driveLetter())}});
  if (!path.isEmpty()) m_viewModel->addEntries({{app::FileStagingKind::Directory, path}});
}

void FileStagingWidget::removeSelected() {
  if (!m_viewModel->writable()) return;
  QStringList paths;
  auto selected = m_entries.list->selectionModel()->selectedRows();
  for (const auto& index : std::as_const(selected)) paths.append(index.data(presentation::PathRole).toString());
  const bool restore = presentation::isRemovedSelection(selected);
  if (restore) {
    if (m_viewModel->canAddEntries()) m_viewModel->restorePaths(paths);
  } else
    m_viewModel->removePaths(paths);
}

void FileStagingWidget::filterChanged(const QString& text) { m_entries.list->setFilterText(text.trimmed()); }

void FileStagingWidget::copySelectedPath(const QModelIndex& index) {
  if (!index.isValid()) return;
  const QString path = index.data(presentation::PathRole).toString();
  const QPointer self(this);
  if (writeClipboardText(path) == ClipboardWriteResult::Confirmed && self) emit copiedToClipboard();
}

void FileStagingWidget::rebuild() {
  m_entries.list->clearSelection();
  m_entries.list->setCurrentIndex({});
  renderState();
  filterChanged(m_entries.filter->text());
}

void FileStagingWidget::refreshThemedIcons() {
  auto& theme = ThemeManager::instance();
  m_addFileAction->setIcon(theme.icon(":/icons/file.svg"));
  m_addDirectoryAction->setIcon(theme.icon(":/icons/folder.svg"));
  m_entries.list->viewport()->update();
  renderState();
}

void FileStagingWidget::renderState() {
  const bool available = m_viewModel->storageAvailable();
  const bool writable = m_viewModel->writable();
  const bool canAdd = m_viewModel->canAddEntries();
  const auto selected = m_entries.list->selectionModel()->selectedRows();
  const bool restore = presentation::isRemovedSelection(selected);
  m_entries.renderRemoval(restore, restore ? I18n::tr("Undo removal") : I18n::tr("Remove selected"));
  m_entries.add->setEnabled(canAdd);
  m_entries.remove->setEnabled((restore ? canAdd : writable) && m_entries.list->selectionModel()->hasSelection());
  m_addFileAction->setEnabled(canAdd);
  m_addDirectoryAction->setEnabled(canAdd);
  m_entries.filter->setEnabled(available);
  m_entries.list->setEnabled(available);
  const auto unavailable = I18n::tr("File staging is unavailable: %1").arg(m_viewModel->storageError());
  const auto readonlyReason = m_viewModel->busy()                                                       ? I18n::tr("Please wait for the operation to finish.")
                              : m_viewModel->editingRestriction() == app::RuntimeWriteRestriction::None ? I18n::tr("Read-only")
                                                                                                        : m_viewModel->editingRestrictionText();
  m_entries.pagination->setSummary(available ? m_viewModel->summaryText() : unavailable);
  const auto additionRestriction = !available ? unavailable : readonlyReason;
  m_entries.add->setToolTip(canAdd ? I18n::tr("Add files or a folder to the automatic commit list.") : additionRestriction);
  m_addFileAction->setToolTip(canAdd ? I18n::tr("Select one or more files to commit automatically before safe shutdown or restart.") : additionRestriction);
  m_addDirectoryAction->setToolTip(canAdd ? I18n::tr("Select a folder whose files will be committed recursively before safe shutdown or restart.")
                                          : additionRestriction);
  m_entries.remove->setToolTip(!available                          ? unavailable
                               : !writable || (restore && !canAdd) ? readonlyReason
                               : selected.isEmpty()                ? I18n::tr("Select entries to remove.")
                               : restore                           ? I18n::tr("Undo removal")
                                                                   : I18n::tr("Mark selected paths for removal. Takes effect after Apply."));
  m_entries.list->viewport()->update();
}

}  // namespace uwf::ui

#include "moc_FileStagingWidget.cpp"
