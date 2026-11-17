// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "presentation/views/configuration/exclusions/ExclusionListWidget.h"

#include <QAction>
#include <QItemSelectionModel>
#include <QLineEdit>
#include <QListView>
#include <QMenu>
#include <QModelIndex>
#include <QPointer>
#include <QPushButton>
#include <QVariant>
#include <utility>

#include "application/configuration/exclusions/ExclusionEditor.h"
#include "presentation/appearance/I18n.h"
#include "presentation/appearance/ThemeManager.h"
#include "presentation/controls/clipboard/ClipboardText.h"
#include "presentation/controls/navigation/CommandMenu.h"
#include "presentation/controls/pagination/PaginationBar.h"
#include "presentation/controls/tables/PathEntryListView.h"
#include "presentation/dialogs/Dialogs.h"
#include "presentation/models/ItemRoles.h"
#include "presentation/platform/DesktopPlatform.h"

namespace uwf::ui {

ExclusionListWidget::ExclusionListWidget(std::shared_ptr<presentation::ExclusionListViewModel> model, dialogs::FileDialogProvider& fileDialogs, QWidget* parent)
    : QWidget(parent), m_viewModel(std::move(model)), m_kind(m_viewModel->kind()), m_fileDialogs(fileDialogs) {
  const Kind kind = m_kind;
  auto& tm = ThemeManager::instance();
  m_entries = createPathEntryControls(*this, *m_viewModel, kind == Kind::File ? I18n::tr("File or folder") : I18n::tr("Registry key or persistence setting"),
                                      I18n::tr("Status"));
  m_entries.list->setObjectName(QStringLiteral("exclusionList"));
  I18n::bind(m_entries.filter, [target = m_entries.filter] { target->setPlaceholderText(I18n::tr("Search paths")); });
  I18n::bind(m_entries.add, [target = m_entries.add] { target->setText(I18n::tr("Add")); });

  if (kind == Kind::File) {
    auto* addMenu = new CommandMenu(m_entries.add);
    m_addFileAct = addMenu->addAction(tm.icon(":/icons/file.svg"), QString{});
    I18n::bind(m_addFileAct, [target = m_addFileAct] { target->setText(I18n::tr("File…")); });
    I18n::bind(m_addFileAct, [target = m_addFileAct] { target->setToolTip(I18n::tr("Pick a file to add to the exclusion list.")); });
    m_addDirAct = addMenu->addAction(tm.icon(":/icons/folder.svg"), QString{});
    I18n::bind(m_addDirAct, [target = m_addDirAct] { target->setText(I18n::tr("Folder…")); });
    I18n::bind(m_addDirAct, [target = m_addDirAct] { target->setToolTip(I18n::tr("Pick a folder (and all of its contents) to add to the exclusion list.")); });
    m_entries.add->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_entries.add, &QWidget::customContextMenuRequested, addMenu,
            [this, addMenu](const QPoint& position) { addMenu->popup(m_entries.add->mapToGlobal(position)); });
    connect(m_entries.add, &QPushButton::clicked, this, [this] {
      if (!m_viewModel->canAdd()) return;
      auto items = m_fileDialogs.openItems(
          this,
          {I18n::tr("Add files or folders"), dialogs::dialogBasePath(m_viewModel->driveLetter()), {}, {dialogs::dialogBasePath(m_viewModel->driveLetter())}});
      const auto batch = m_viewModel->beginBatch();
      for (const auto& item : std::as_const(items)) addSelectedPath(item.path);
    });
    connect(m_addFileAct, &QAction::triggered, this, &ExclusionListWidget::onAddFile);
    connect(m_addDirAct, &QAction::triggered, this, &ExclusionListWidget::onAddDir);
  } else {
    const QString dskName = I18n::tr("Domain Secret Key persistence");
    const QString tscalName = I18n::tr("Terminal Services license persistence");
    // Global persistence options share the exclusion list interaction as synthetic rows.
    auto* addMenu = new CommandMenu(m_entries.add);
    addMenu->setToolTipsVisible(true);
    m_addRegKeyAct = addMenu->addAction(tm.icon(":/icons/registry.svg"), QString{});
    I18n::bind(m_addRegKeyAct, [target = m_addRegKeyAct] { target->setText(I18n::tr("Registry key…")); });
    I18n::bind(m_addRegKeyAct, [target = m_addRegKeyAct] { target->setToolTip(I18n::tr("Enter a registry key path to add to the exclusion list.")); });
    m_addDomainSecretAct = addMenu->addAction(tm.icon(":/icons/key.svg"), dskName);
    I18n::bind(m_addDomainSecretAct, [target = m_addDomainSecretAct] {
      target->setToolTip(I18n::tr("Persist the domain secret key (machine account password) across UWF sessions."));
    });
    m_addTscalAct = addMenu->addAction(tm.icon(":/icons/license.svg"), tscalName);
    I18n::bind(m_addTscalAct,
               [target = m_addTscalAct] { target->setToolTip(I18n::tr("Persist Terminal Services client access licenses across UWF sessions.")); });
    m_entries.add->setMenu(addMenu);
    connect(m_addRegKeyAct, &QAction::triggered, this, &ExclusionListWidget::onAddRegistry);
    connect(m_addDomainSecretAct, &QAction::triggered, this,
            [this] { enablePersistFlag(presentation::ExclusionListViewModel::PersistenceItem::DomainSecretKey); });
    connect(m_addTscalAct, &QAction::triggered, this, [this] { enablePersistFlag(presentation::ExclusionListViewModel::PersistenceItem::Tscal); });
    connect(addMenu, &QMenu::aboutToShow, this, &ExclusionListWidget::renderState);
  }

  connect(m_viewModel.get(), &presentation::ExclusionListViewModel::stateChanged, this, &ExclusionListWidget::rebuild);
  connect(m_viewModel.get(), &presentation::ExclusionListViewModel::editingAvailabilityChanged, this, &ExclusionListWidget::renderState);
  connect(m_viewModel.get(), &presentation::ExclusionListViewModel::removalAvailabilityChanged, this, &ExclusionListWidget::renderState);

  connect(m_entries.remove, &QPushButton::clicked, this, &ExclusionListWidget::onRemove);
  connect(m_entries.filter, &QLineEdit::textChanged, this, &ExclusionListWidget::onFilterChanged);
  connect(m_entries.list->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ExclusionListWidget::renderState);
  connect(m_entries.list->selectionModel(), &QItemSelectionModel::selectionChanged, this, [this] {
    const auto selected = m_entries.list->selectionModel()->selectedRows();
    emit selectionDetailChanged(selected.isEmpty()     ? QString()
                                : selected.size() == 1 ? selected.front().data(Qt::ToolTipRole).toString().simplified()
                                                       : I18n::tr("%1 selected").arg(selected.size()));
  });
  connect(m_entries.list, &QListView::doubleClicked, this, &ExclusionListWidget::onItemDoubleClicked);
  connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [this](app::Theme) { refreshThemedIcons(); });

  m_entries.list->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(m_entries.list, &QListView::customContextMenuRequested, this, [this](const QPoint& pos) {
    const auto index = m_entries.list->indexAt(pos);
    if (!index.isValid()) return;
    // Preserve a multi-selection when opening a menu on one of its members.
    if (!m_entries.list->selectionModel()->isSelected(index)) m_entries.list->selectionModel()->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
    const QString full = entryFullPath(index);
    if (full.isEmpty()) return;
    QStringList paths;
    for (auto indexes = m_entries.list->selectionModel()->selectedRows(); const auto& selected : std::as_const(indexes)) paths.append(entryFullPath(selected));

    auto& tm = ThemeManager::instance();
    CommandMenu menu(this);
    menu.setToolTipsVisible(true);

    if (m_kind == Kind::File) {
      const QString rel = index.data(presentation::PathRole).toString();
      auto* openAct = menu.addAction(tm.icon(":/icons/folder.svg"), I18n::tr("Open containing folder"));
      openAct->setEnabled(m_viewModel->canBrowseFiles());
      connect(openAct, &QAction::triggered, this, [this, rel]() { openContainingFolder(rel); });

      auto* copyAct = menu.addAction(tm.icon(":/icons/copy.svg"), I18n::tr("Copy file path"));
      connect(copyAct, &QAction::triggered, this, [this, paths]() { copyPathToClipboard(paths.join(QStringLiteral("\r\n"))); });

      if (paths.size() == 1 && m_viewModel->fileCommitState(rel).visible) {
        menu.addSeparator();
        auto* commitAct = menu.addAction(tm.icon(":/icons/apply.svg"), QString{});
        const auto updateAvailability = [this, commitAct, rel] {
          const auto state = m_viewModel->fileCommitState(rel);
          commitAct->setVisible(state.visible);
          commitAct->setEnabled(state.enabled);
          commitAct->setText(state.kind == app::ExclusionEditor::FileEntryKind::Directory ? I18n::tr("Commit folder changes to disk…")
                                                                                          : I18n::tr("Commit file changes to disk…"));
          commitAct->setToolTip(m_viewModel->commitRestriction());
        };
        updateAvailability();
        connect(m_viewModel.get(), &presentation::ExclusionListViewModel::commitAvailabilityChanged, commitAct, updateAvailability);
        connect(m_viewModel.get(), &presentation::ExclusionListViewModel::stateChanged, commitAct, updateAvailability);
        connect(commitAct, &QAction::triggered, this, [this, full, rel]() {
          if (m_viewModel->fileCommitState(rel).enabled) emit commitFileRequested(full);
        });
      }
    } else {
      auto* copyAct = menu.addAction(tm.icon(":/icons/copy.svg"), I18n::tr("Copy registry path"));
      connect(copyAct, &QAction::triggered, this, [this, paths]() { copyPathToClipboard(paths.join(QStringLiteral("\r\n"))); });
    }

    menu.addSeparator();
    const QString removeLabel = m_entries.remove->property("accessibleName").toString();
    const bool restore = presentation::isRemovedSelection(m_entries.list->selectionModel()->selectedRows());
    auto* remove = menu.addAction(tm.icon(restore ? ":/icons/restart.svg" : ":/icons/remove.svg"), removeLabel, &menu, [this] { onRemove(); });
    remove->setEnabled(m_entries.remove->isEnabled());
    if (!restore) remove->setProperty("designRole", "destructive");
    menu.exec(m_entries.list->viewport()->mapToGlobal(pos));
  });
  I18n::bind(this, [this] {
    m_viewModel->retranslate();
    m_entries.list->setHeading(m_kind == Kind::File ? I18n::tr("File or folder") : I18n::tr("Registry key or persistence setting"));
    m_entries.list->setStatusHeading(I18n::tr("Status"));
    if (m_addDomainSecretAct) m_addDomainSecretAct->setText(I18n::tr("Domain Secret Key persistence"));
    if (m_addTscalAct) m_addTscalAct->setText(I18n::tr("Terminal Services license persistence"));
    renderState();
    const auto selected = m_entries.list->selectionModel()->selectedRows();
    if (!selected.isEmpty())
      emit selectionDetailChanged(selected.size() == 1 ? selected.constFirst().data(Qt::ToolTipRole).toString().simplified()
                                                       : I18n::tr("%1 selected").arg(selected.size()));
  });
}

void ExclusionListWidget::openContainingFolder(const QString& entry) const {
  if (!m_viewModel->canBrowseFiles()) return;
  QString abs = entry;
  if (abs.startsWith('\\')) abs = m_viewModel->driveLetter() + abs;
  platform::revealFile(abs);
}

void ExclusionListWidget::refreshThemedIcons() {
  auto& tm = ThemeManager::instance();
  if (m_addFileAct) m_addFileAct->setIcon(tm.icon(":/icons/file.svg"));
  if (m_addDirAct) m_addDirAct->setIcon(tm.icon(":/icons/folder.svg"));
  if (m_addRegKeyAct) m_addRegKeyAct->setIcon(tm.icon(":/icons/registry.svg"));
  if (m_addDomainSecretAct) m_addDomainSecretAct->setIcon(tm.icon(":/icons/key.svg"));
  if (m_addTscalAct) m_addTscalAct->setIcon(tm.icon(":/icons/license.svg"));
  m_entries.list->viewport()->update();
  renderState();
}

QString ExclusionListWidget::entryFullPath(const QModelIndex& index) const { return absoluteFilePath(index.data(presentation::PathRole).toString()); }

QString ExclusionListWidget::absoluteFilePath(const QString& path) const { return m_viewModel->absoluteFilePath(path); }

void ExclusionListWidget::copyPathToClipboard(const QString& path) {
  if (path.isEmpty()) return;
  const QPointer self(this);
  if (writeClipboardText(path) == ClipboardWriteResult::Confirmed && self) emit copiedToClipboard();
}

void ExclusionListWidget::onItemDoubleClicked(const QModelIndex& index) {
  if (!index.isValid()) return;
  copyPathToClipboard(entryFullPath(index));
}

void ExclusionListWidget::setReadOnly(bool value) { m_viewModel->setReadOnly(value); }

void ExclusionListWidget::renderState() {
  m_entries.filter->setEnabled(true);
  m_entries.list->setEnabled(true);
  m_entries.pagination->setSummary(I18n::tr("%1 items").arg(m_viewModel->rowCount()));
  m_entries.add->setEnabled(m_viewModel->canAdd());
  m_entries.add->setToolTip(m_viewModel->additionHint());
  for (auto* action : {m_addFileAct, m_addDirAct, m_addRegKeyAct})
    if (action) action->setEnabled(m_viewModel->canAdd());
  updateAddMenuState();
  updateRemoveButtonState();
}

void ExclusionListWidget::onFilterChanged(const QString& text) { m_entries.list->setFilterText(text.trimmed()); }

void ExclusionListWidget::onAddFile() {
  if (!m_viewModel->canAdd()) return;
  auto paths = m_fileDialogs.openFiles(this, {I18n::tr("Select files to add to the exclusion list (multiple selection allowed)"),
                                              dialogs::dialogBasePath(m_viewModel->driveLetter()),
                                              {},
                                              {dialogs::dialogBasePath(m_viewModel->driveLetter())}});
  const auto batch = m_viewModel->beginBatch();
  for (const auto& p : std::as_const(paths)) addSelectedPath(p);
}

void ExclusionListWidget::onAddDir() {
  if (!m_viewModel->canAdd()) return;
  const QString path = m_fileDialogs.selectDirectory(this, {I18n::tr("Select a folder to add to the exclusion list"),
                                                            dialogs::dialogBasePath(m_viewModel->driveLetter()),
                                                            {},
                                                            {dialogs::dialogBasePath(m_viewModel->driveLetter())}});
  if (path.isEmpty()) return;
  addSelectedPath(path);
}

void ExclusionListWidget::onAddRegistry() {
  if (!m_viewModel->canAdd()) return;
  emit registryPickerRequested();
}

void ExclusionListWidget::addSelectedPath(const QString& selectedPath) {
  const auto result = m_viewModel->addSelectedPath(selectedPath);
  switch (result.outcome) {
    case app::ExclusionImportOutcome::RejectedNotOnVolume:
      dialogs::warning(this, I18n::tr("Path is not on this volume"),
                       I18n::tr("The selected path %1 is not on volume %2, and therefore cannot be added as an exclusion for this volume.")
                           .arg(result.path, m_viewModel->driveLetter()));
      break;
    case app::ExclusionImportOutcome::RejectedMissingVolume:
    case app::ExclusionImportOutcome::RejectedReadOnly:
    case app::ExclusionImportOutcome::RejectedForbidden:
      dialogs::warning(this, I18n::tr("Cannot add this exclusion"), result.reason);
      break;
    case app::ExclusionImportOutcome::RejectedConflict:
      dialogs::warning(this, I18n::tr("Cannot add this exclusion"),
                       I18n::tr("This path overlaps a file staging entry. Remove the staged path before adding the exclusion:\n%1").arg(result.conflict));
      break;
    default:
      break;
  }
}

void ExclusionListWidget::onRemove() {
  const auto indexes = m_entries.list->selectionModel()->selectedRows();
  const bool restore = presentation::isRemovedSelection(indexes);
  const auto selection = selectedEntries();
  if (!(restore ? m_viewModel->restoreSelection(selection) : m_viewModel->removeSelection(selection))) {
    renderState();
    if (restore) dialogs::warning(this, I18n::tr("Undo removal"), m_viewModel->restorationRestriction(selection));
  }
}

QVector<presentation::ExclusionListViewModel::Selection> ExclusionListWidget::selectedEntries() const {
  QVector<presentation::ExclusionListViewModel::Selection> selection;
  for (auto indexes = m_entries.list->selectionModel()->selectedRows(); const auto& index : std::as_const(indexes)) {
    const QString path = index.data(presentation::PathRole).toString();
    std::optional<presentation::ExclusionListViewModel::PersistenceItem> persistence;
    const auto value = index.data(presentation::PersistenceRole);
    if (value.isValid()) persistence = static_cast<presentation::ExclusionListViewModel::PersistenceItem>(value.toInt());
    selection.append({path, persistence});
  }
  return selection;
}

void ExclusionListWidget::rebuild() {
  m_entries.list->clearSelection();
  m_entries.list->setCurrentIndex({});
  m_entries.list->viewport()->update();
  onFilterChanged(m_entries.filter->text());
  renderState();
}

void ExclusionListWidget::enablePersistFlag(presentation::ExclusionListViewModel::PersistenceItem item) { m_viewModel->enablePersistence(item); }

void ExclusionListWidget::updateAddMenuState() {
  if (m_addDomainSecretAct)
    m_addDomainSecretAct->setEnabled(m_viewModel->canEnablePersistence(presentation::ExclusionListViewModel::PersistenceItem::DomainSecretKey));
  if (m_addTscalAct) m_addTscalAct->setEnabled(m_viewModel->canEnablePersistence(presentation::ExclusionListViewModel::PersistenceItem::Tscal));
}

bool ExclusionListWidget::canRemoveSelection() const {
  QStringList paths;
  for (auto indexes = m_entries.list->selectionModel()->selectedRows(); const auto& index : std::as_const(indexes)) paths.append(entryFullPath(index));
  return m_viewModel->canRemoveSelection(paths);
}

void ExclusionListWidget::updateRemoveButtonState() {
  if (!m_entries.remove) return;
  const auto selected = m_entries.list->selectionModel()->selectedRows();
  const bool restore = presentation::isRemovedSelection(selected);
  const bool hasSelection = m_entries.list->selectionModel()->hasSelection();
  const bool readOnly = m_viewModel->readOnly();
  const bool removable = hasSelection && !readOnly && (restore ? m_viewModel->canRestoreSelection(selectedEntries()) : canRemoveSelection());
  m_entries.remove->setEnabled(removable);
  m_entries.renderRemoval(restore, restore ? I18n::tr("Undo removal") : I18n::tr("Remove selected"));
  if (readOnly) {
    m_entries.remove->setToolTip(I18n::tr("Read-only"));
  } else if (!hasSelection) {
    m_entries.remove->setToolTip(I18n::tr("Select entries to remove."));
  } else if (restore && !removable) {
    m_entries.remove->setToolTip(m_viewModel->restorationRestriction(selectedEntries()));
  } else if (!removable) {
    m_entries.remove->setToolTip(
        I18n::tr("This registry exclusion is required while File staging contains saved entries. Clear the File staging list before removing it."));
  } else {
    m_entries.remove->setToolTip(restore ? I18n::tr("Undo removal")
                                         : I18n::tr("Remove the selected entries from the exclusion list. Takes effect after Apply."));
  }
}

}  // namespace uwf::ui

#include "moc_ExclusionListWidget.cpp"
