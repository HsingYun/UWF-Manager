// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "ExclusionListWidget.h"

#include <shlobj.h>
#include <windows.h>

#include <QAction>
#include <QApplication>
#include <QBrush>
#include <QClipboard>
#include <QColor>
#include <QEvent>
#include <QIcon>
#include <QItemSelectionModel>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QMenu>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QScreen>
#include <QScrollBar>
#include <QSizePolicy>
#include <QVBoxLayout>
#include <cmath>
#include <string_view>
#include <utility>

#include "app/configuration/ExclusionRules.h"
#include "app/ports/FileStagingStore.h"
#include "app/ports/RegistryKey.h"
#include "app/ports/VolumePaths.h"
#include "app/staging/FileStagingConflictPolicy.h"
#include "core/RegistryExclusionPolicy.h"
#include "core/UwfConstraints.h"
#include "presentation/common/ItemRoles.h"
#include "ui/appearance/I18n.h"
#include "ui/appearance/ThemeManager.h"
#include "ui/dialogs/Dialogs.h"
#include "ui/registry/RegistryPickerDialog.h"
#include "ui/widgets/CommandMenu.h"
#include "ui/widgets/EntryListView.h"
#include "ui/widgets/PaginationBar.h"
#include "ui/widgets/RoundedCornerOverlay.h"
#include "ui/widgets/SearchField.h"
#include "ui/widgets/TextLabel.h"
#include "windows/system/WindowsShell.h"

namespace uwf::ui {

ExclusionListWidget::ExclusionListWidget(std::shared_ptr<presentation::ExclusionListViewModel> model, dialogs::FileDialogProvider& fileDialogs, QWidget* parent)
    : QWidget(parent), m_viewModel(std::move(model)), m_kind(m_viewModel->kind()), m_fileDialogs(fileDialogs) {
  const Kind kind = m_kind;
  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 14, 0, 0);
  layout->setSpacing(14);

  m_commands = new QWidget(this);
  auto* header = new QHBoxLayout(m_commands);
  header->setContentsMargins(0, 0, 0, 0);
  header->setSpacing(3);
  m_filter = new SearchField(this);
  I18n::bind(m_filter, [target = m_filter] { target->setPlaceholderText(I18n::tr("Search paths")); });
  m_filter->setClearButtonEnabled(true);
  m_filter->setObjectName("filterInput");
  header->addWidget(m_filter, 1);
  header->addSpacing(9);

  auto& tm = ThemeManager::instance();
  // 填充按钮固定使用浅色图标，确保蓝色和红色背景上的对比度。
  const QColor btnIconFg(0xE8EAED);

  if (kind == Kind::File) {
    // 合并文件和文件夹入口，为筛选框保留宽度。
    m_addBtn = new QPushButton(tm.iconWithColor(":/icons/add.svg", btnIconFg), QString{}, this);
    I18n::bind(m_addBtn, [target = m_addBtn] { target->setText(I18n::tr("Add")); });
    m_addBtn->setObjectName("primaryBtn");
    I18n::bind(m_addBtn,
               [target = m_addBtn] { target->setToolTip(I18n::tr("Add a file or folder to the exclusion list. Excluded entries are not protected by UWF.")); });
    auto* addMenu = new CommandMenu(m_addBtn);
    m_addFileAct = addMenu->addAction(tm.icon(":/icons/file.svg"), QString{});
    I18n::bind(m_addFileAct, [target = m_addFileAct] { target->setText(I18n::tr("File…")); });
    I18n::bind(m_addFileAct, [target = m_addFileAct] { target->setToolTip(I18n::tr("Pick a file to add to the exclusion list.")); });
    m_addDirAct = addMenu->addAction(tm.icon(":/icons/folder.svg"), QString{});
    I18n::bind(m_addDirAct, [target = m_addDirAct] { target->setText(I18n::tr("Folder…")); });
    I18n::bind(m_addDirAct, [target = m_addDirAct] { target->setToolTip(I18n::tr("Pick a folder (and all of its contents) to add to the exclusion list.")); });
    m_addBtn->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_addBtn, &QWidget::customContextMenuRequested, addMenu,
            [this, addMenu](const QPoint& position) { addMenu->popup(m_addBtn->mapToGlobal(position)); });
    connect(m_addBtn, &QPushButton::clicked, this, [this] {
      if (m_viewModel->readOnly()) return;
      const auto items = m_fileDialogs.openItems(
          this,
          {I18n::tr("Add files or folders"), dialogs::dialogBasePath(m_viewModel->driveLetter()), {}, {dialogs::dialogBasePath(m_viewModel->driveLetter())}});
      for (const auto& item : items) addPendingEntry(item.path);
    });
    header->addWidget(m_addBtn);
    connect(m_addFileAct, &QAction::triggered, this, &ExclusionListWidget::onAddFile);
    connect(m_addDirAct, &QAction::triggered, this, &ExclusionListWidget::onAddDir);
  } else {
    // 持久化伪条目的存储、比较和展示共用同一翻译键。
    const QString dskName = I18n::tr("Domain Secret Key persistence");
    const QString tscalName = I18n::tr("Terminal Services license persistence");
    // 将两个全局持久化开关作为伪条目，复用排除列表的添加和移除交互。
    m_addBtn = new QPushButton(tm.iconWithColor(":/icons/add.svg", btnIconFg), QString{}, this);
    I18n::bind(m_addBtn, [target = m_addBtn] { target->setText(I18n::tr("Add")); });
    m_addBtn->setObjectName("primaryBtn");
    I18n::bind(m_addBtn, [target = m_addBtn] { target->setToolTip(I18n::tr("Add a registry key to the exclusion list, or enable a persistence switch.")); });
    auto* addMenu = new CommandMenu(m_addBtn);
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
    m_addBtn->setMenu(addMenu);
    header->addWidget(m_addBtn);
    connect(m_addRegKeyAct, &QAction::triggered, this, &ExclusionListWidget::onAddRegistry);
    connect(m_addDomainSecretAct, &QAction::triggered, this,
            [this] { enablePersistFlag(presentation::ExclusionListViewModel::PersistenceItem::DomainSecretKey); });
    connect(m_addTscalAct, &QAction::triggered, this, [this] { enablePersistFlag(presentation::ExclusionListViewModel::PersistenceItem::Tscal); });
    connect(addMenu, &QMenu::aboutToShow, this, &ExclusionListWidget::updateAddMenuState);
  }

  m_rmBtn = new QPushButton(tm.iconWithColor(":/icons/remove.svg", btnIconFg), I18n::tr("Remove selected"), this);
  m_rmBtn->setObjectName("dangerBtn");
  header->addWidget(m_rmBtn);
  layout->addWidget(m_commands);

  m_list = new EntryListView(kind == Kind::File ? I18n::tr("File or folder") : I18n::tr("Registry key or persistence setting"), false, this);
  m_list->setAlternatingRowColors(false);
  m_list->setSelectionMode(QAbstractItemView::ExtendedSelection);
  m_list->setUniformItemSizes(true);
  m_list->setIconSize({16, 16});
  m_list->setModel(m_viewModel.get());
  m_list->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_list->setObjectName("exclusionList");
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
  m_rmBtn->setProperty("accessibleName", m_rmBtn->text());
  m_rmBtn->setText(QString());
  m_rmBtn->setFixedWidth(32);
  m_filter->setMinimumWidth(80);
  m_filter->setFixedHeight(30);
  for (const auto buttons = m_commands->findChildren<QPushButton*>(); auto* button : buttons) {
    button->setProperty("designRole", button == m_addBtn ? "secondary" : "dangerQuiet");
    button->setFixedHeight(30);
    auto type = button->font();
    type.setPixelSize(12);
    button->setFont(type);
  }

  // QSS 不会按容器圆角裁剪列表项；遮罩覆盖溢出内容并重绘边框。
  // 遮罩挂在容器而非 viewport，以覆盖边框区域。
  m_list->installEventFilter(this);
  m_cornerOverlay = new RoundedCornerOverlay(
      m_list, 0, 6, [] { return ThemeManager::instance().color(Sem::Bg); }, [] { return ThemeManager::instance().color(Sem::Border); }, 1);
  m_cornerOverlay->syncToParent();
  connect(m_list->verticalScrollBar(), &QAbstractSlider::valueChanged, m_cornerOverlay, QOverload<>::of(&QWidget::update));

  connect(m_viewModel.get(), &presentation::ExclusionListViewModel::stateChanged, this, &ExclusionListWidget::rebuild);
  connect(m_viewModel.get(), &presentation::ExclusionListViewModel::pendingChanged, this, &ExclusionListWidget::pendingChanged);
  connect(m_viewModel.get(), &presentation::ExclusionListViewModel::readOnlyChanged, this, &ExclusionListWidget::applyReadOnly);
  connect(m_viewModel.get(), &presentation::ExclusionListViewModel::removalAvailabilityChanged, this, &ExclusionListWidget::updateRemoveButtonState);

  connect(m_rmBtn, &QPushButton::clicked, this, &ExclusionListWidget::onRemove);
  connect(m_filter, &QLineEdit::textChanged, this, &ExclusionListWidget::onFilterChanged);
  connect(m_list->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ExclusionListWidget::updateRemoveButtonState);
  connect(m_list->selectionModel(), &QItemSelectionModel::selectionChanged, this, [this] {
    const auto selected = m_list->selectionModel()->selectedRows();
    emit selectionDetailChanged(selected.isEmpty()     ? QString()
                                : selected.size() == 1 ? selected.front().data(Qt::ToolTipRole).toString().simplified()
                                                       : I18n::tr("%1 selected").arg(selected.size()));
  });
  connect(m_list, &QListView::doubleClicked, this, &ExclusionListWidget::onItemDoubleClicked);
  connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [this](app::Theme) { refreshThemedIcons(); });

  m_list->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(m_list, &QListView::customContextMenuRequested, this, [this](const QPoint& pos) {
    const auto index = m_list->indexAt(pos);
    if (!index.isValid()) return;
    // Preserve a multi-selection when opening a menu on one of its members.
    if (!m_list->selectionModel()->isSelected(index)) m_list->selectionModel()->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
    const QString full = entryFullPath(index);
    if (full.isEmpty()) return;
    QStringList paths;
    for (const auto indexes = m_list->selectionModel()->selectedRows(); const auto& selected : indexes) paths.append(entryFullPath(selected));

    auto& tm = ThemeManager::instance();
    CommandMenu menu(this);

    if (m_kind == Kind::File) {
      const QString rel = index.data(presentation::PathRole).toString();
      auto* openAct = menu.addAction(tm.icon(":/icons/folder.svg"), I18n::tr("Open containing folder"));
      connect(openAct, &QAction::triggered, this, [this, rel]() { openContainingFolder(rel); });

      auto* copyAct = menu.addAction(tm.icon(":/icons/copy.svg"), I18n::tr("Copy file path"));
      connect(copyAct, &QAction::triggered, this, [this, paths]() { copyPathToClipboard(paths.join(QStringLiteral("\r\n"))); });

      // 仅活动覆盖层中的现存、未排除路径可提交；当前会话排除项已直接写入磁盘。
      if (const auto kind = m_viewModel->fileCommitKind(rel); paths.size() == 1 && kind) {
        menu.addSeparator();
        const QString label =
            *kind == app::ExclusionEditor::FileEntryKind::Directory ? I18n::tr("Commit folder changes to disk…") : I18n::tr("Commit file changes to disk…");
        auto* commitAct = menu.addAction(tm.icon(":/icons/apply.svg"), label);
        connect(commitAct, &QAction::triggered, this, [this, full]() { emit commitFileRequested(full); });
      }
    } else {
      auto* copyAct = menu.addAction(tm.icon(":/icons/copy.svg"), I18n::tr("Copy registry path"));
      connect(copyAct, &QAction::triggered, this, [this, paths]() { copyPathToClipboard(paths.join(QStringLiteral("\r\n"))); });
    }

    menu.addSeparator();
    const QString removeLabel = m_rmBtn->property("accessibleName").toString();
    const bool restore = presentation::isRemovedSelection(m_list->selectionModel()->selectedRows());
    auto* remove = menu.addAction(tm.icon(restore ? ":/icons/restart.svg" : ":/icons/remove.svg"), removeLabel, &menu, [this] { onRemove(); });
    remove->setEnabled(m_rmBtn->isEnabled());
    if (!restore) remove->setProperty("designRole", "destructive");
    menu.exec(m_list->viewport()->mapToGlobal(pos));
  });
  I18n::bind(this, [this] {
    m_viewModel->retranslate();
    static_cast<EntryListView*>(m_list)->setHeading(m_kind == Kind::File ? I18n::tr("File or folder") : I18n::tr("Registry key or persistence setting"));
    m_pagination->setSummary(I18n::tr("%1 items").arg(m_viewModel->rowCount()));
    if (m_addDomainSecretAct) m_addDomainSecretAct->setText(I18n::tr("Domain Secret Key persistence"));
    if (m_addTscalAct) m_addTscalAct->setText(I18n::tr("Terminal Services license persistence"));
    updateRemoveButtonState();
    const auto selected = m_list->selectionModel()->selectedRows();
    if (!selected.isEmpty())
      emit selectionDetailChanged(selected.size() == 1 ? selected.constFirst().data(Qt::ToolTipRole).toString().simplified()
                                                       : I18n::tr("%1 selected").arg(selected.size()));
  });
}

void ExclusionListWidget::openContainingFolder(const QString& entry) const {
  QString abs = entry;
  if (abs.startsWith('\\')) abs = m_viewModel->driveLetter() + abs;
  windows::shell::revealInExplorer(abs);
}

void ExclusionListWidget::refreshThemedIcons() {
  auto& tm = ThemeManager::instance();
  const QColor btnIconFg(0xE8EAED);
  if (m_addBtn) {
    m_addBtn->setIcon(tm.iconWithColor(":/icons/add.svg", btnIconFg));
  }
  if (m_addFileAct) m_addFileAct->setIcon(tm.icon(":/icons/file.svg"));
  if (m_addDirAct) m_addDirAct->setIcon(tm.icon(":/icons/folder.svg"));
  if (m_addRegKeyAct) m_addRegKeyAct->setIcon(tm.icon(":/icons/registry.svg"));
  if (m_addDomainSecretAct) m_addDomainSecretAct->setIcon(tm.icon(":/icons/key.svg"));
  if (m_addTscalAct) m_addTscalAct->setIcon(tm.icon(":/icons/license.svg"));
  if (m_rmBtn) {
    m_rmBtn->setIcon(tm.iconWithColor(":/icons/remove.svg", btnIconFg));
  }
  m_list->viewport()->update();
  updateRemoveButtonState();
}

bool ExclusionListWidget::eventFilter(QObject* obj, QEvent* ev) {
  if (m_cornerOverlay && obj == m_list && ev->type() == QEvent::Resize) {
    m_cornerOverlay->syncToParent();
  }
  return QWidget::eventFilter(obj, ev);
}

QString ExclusionListWidget::entryFullPath(const QModelIndex& index) const { return absoluteFilePath(index.data(presentation::PathRole).toString()); }

QString ExclusionListWidget::absoluteFilePath(const QString& path) const { return m_viewModel->absoluteFilePath(path); }

void ExclusionListWidget::copyPathToClipboard(const QString& path) {
  if (path.isEmpty()) return;
  QApplication::clipboard()->setText(path);
  emit copiedToClipboard();
}

void ExclusionListWidget::onItemDoubleClicked(const QModelIndex& index) {
  if (!index.isValid()) return;
  copyPathToClipboard(entryFullPath(index));
}

void ExclusionListWidget::setDriveLetter(const QString& value) { m_viewModel->setDriveLetter(value); }

void ExclusionListWidget::setBaseline(const QStringList& current, const QStringList& next) { m_viewModel->setBaseline(current, next); }

void ExclusionListWidget::setPersistBaseline(bool dc, bool dn, bool tc, bool tn) { m_viewModel->setPersistBaseline(dc, dn, tc, tn); }

void ExclusionListWidget::resetPending() { m_viewModel->resetPending(); }

QStringList ExclusionListWidget::pendingAdded() const { return m_viewModel->pendingAdded(); }

QStringList ExclusionListWidget::pendingRemoved() const { return m_viewModel->pendingRemoved(); }

std::optional<bool> ExclusionListWidget::pendingPersistDomainSecretKey() const { return m_viewModel->pendingPersistDomainSecretKey(); }
std::optional<bool> ExclusionListWidget::pendingPersistTSCAL() const { return m_viewModel->pendingPersistTSCAL(); }

void ExclusionListWidget::setReadOnly(bool value) { m_viewModel->setReadOnly(value); }

void ExclusionListWidget::applyReadOnly() {
  const bool ro = m_viewModel->readOnly();
  m_filter->setEnabled(true);
  m_list->setEnabled(true);
  const auto buttons = findChildren<QPushButton*>();
  for (auto* btn : buttons) btn->setEnabled(!ro);
  updateRemoveButtonState();
}

void ExclusionListWidget::setCommitEnabled(bool value) { m_viewModel->setCommitEnabled(value); }

void ExclusionListWidget::onFilterChanged(const QString& text) { static_cast<EntryListView*>(m_list)->setFilterText(text.trimmed()); }

void ExclusionListWidget::onAddFile() {
  if (m_viewModel->readOnly()) return;
  const QStringList paths = m_fileDialogs.openFiles(this, {I18n::tr("Select files to add to the exclusion list (multiple selection allowed)"),
                                                           dialogs::dialogBasePath(m_viewModel->driveLetter()),
                                                           {},
                                                           {dialogs::dialogBasePath(m_viewModel->driveLetter())}});
  for (const auto& p : paths) addPendingEntry(p);
}

void ExclusionListWidget::onAddDir() {
  if (m_viewModel->readOnly()) return;
  const QString path = m_fileDialogs.selectDirectory(this, {I18n::tr("Select a folder to add to the exclusion list"),
                                                            dialogs::dialogBasePath(m_viewModel->driveLetter()),
                                                            {},
                                                            {dialogs::dialogBasePath(m_viewModel->driveLetter())}});
  if (path.isEmpty()) return;
  addPendingEntry(path);
}

void ExclusionListWidget::onAddRegistry() {
  if (m_viewModel->readOnly()) return;
  emit registryPickerRequested();
}

void ExclusionListWidget::addPendingEntry(const QString& raw) {
  const auto result = m_viewModel->addEntry(raw);
  switch (result.outcome) {
    case app::ExclusionImportOutcome::RejectedNotOnVolume:
      dialogs::warning(this, I18n::tr("Path is not on this volume"),
                       I18n::tr("The selected path %1 is not on volume %2, and therefore cannot be added as an exclusion for this volume.")
                           .arg(result.path, m_viewModel->driveLetter()));
      break;
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
  QVector<presentation::ExclusionListViewModel::Selection> selection;
  const auto indexes = m_list->selectionModel()->selectedRows();
  const bool restore = presentation::isRemovedSelection(indexes);
  for (const auto& index : indexes) {
    const QString path = index.data(presentation::PathRole).toString();
    std::optional<presentation::ExclusionListViewModel::PersistenceItem> persistence;
    const auto value = index.data(presentation::PersistenceRole);
    if (value.isValid()) persistence = static_cast<presentation::ExclusionListViewModel::PersistenceItem>(value.toInt());
    selection.append({path, persistence});
  }
  if (!(restore ? m_viewModel->restoreSelection(selection) : m_viewModel->removeSelection(selection))) {
    updateRemoveButtonState();
    // Restore retries follow the shared selection/read-only rule, even after
    // a failed attempt. Report the failure without retaining a UI-only state.
    if (restore) dialogs::warning(this, I18n::tr("Undo removal"), I18n::tr("Selected entries cannot currently be restored."));
  }
}

void ExclusionListWidget::rebuild() {
  m_list->clearSelection();
  m_list->setCurrentIndex({});
  m_pagination->setSummary(I18n::tr("%1 items").arg(m_viewModel->rowCount()));
  m_list->viewport()->update();
  onFilterChanged(m_filter->text());
  updateRemoveButtonState();
}

void ExclusionListWidget::enablePersistFlag(presentation::ExclusionListViewModel::PersistenceItem item) { m_viewModel->enablePersistence(item); }

void ExclusionListWidget::updateAddMenuState() {
  if (m_addDomainSecretAct) m_addDomainSecretAct->setEnabled(!m_viewModel->draft().domainSecretKey.effectiveNext());
  if (m_addTscalAct) m_addTscalAct->setEnabled(!m_viewModel->draft().tscal.effectiveNext());
}

bool ExclusionListWidget::canRemoveSelection() const {
  QStringList paths;
  for (const auto indexes = m_list->selectionModel()->selectedRows(); const auto& index : indexes) paths.append(entryFullPath(index));
  return m_viewModel->canRemoveSelection(paths);
}

void ExclusionListWidget::updateRemoveButtonState() {
  if (!m_rmBtn) return;
  const auto selected = m_list->selectionModel()->selectedRows();
  const bool restore = presentation::isRemovedSelection(selected);
  const bool hasSelection = m_list->selectionModel()->hasSelection();
  const bool readOnly = m_viewModel->readOnly();
  const bool removable = hasSelection && !readOnly && (restore || canRemoveSelection());
  m_rmBtn->setEnabled(removable);
  m_rmBtn->setProperty("accessibleName", restore ? I18n::tr("Undo removal") : I18n::tr("Remove selected"));
  m_rmBtn->setIcon(ThemeManager::instance().icon(restore ? QStringLiteral(":/icons/restart.svg") : QStringLiteral(":/icons/remove.svg")));
  if (readOnly) {
    m_rmBtn->setToolTip(I18n::tr("Read-only"));
  } else if (!hasSelection) {
    m_rmBtn->setToolTip(I18n::tr("Select entries to remove."));
  } else if (!removable) {
    m_rmBtn->setToolTip(
        I18n::tr("This registry exclusion is required while File staging contains saved entries. Clear the File staging list before removing it."));
  } else {
    m_rmBtn->setToolTip(restore ? I18n::tr("Undo removal") : I18n::tr("Remove the selected entries from the exclusion list. Takes effect after Apply."));
  }
}

}  // namespace uwf::ui

#include "moc_ExclusionListWidget.cpp"
