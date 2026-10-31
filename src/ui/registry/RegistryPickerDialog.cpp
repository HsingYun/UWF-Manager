// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "RegistryPickerDialog.h"

#include <QBrush>
#include <QClipboard>
#include <QDialogButtonBox>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QLabel>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPushButton>
#include <QScrollBar>
#include <QSplitter>
#include <QStackedWidget>
#include <QTableView>
#include <QTreeView>
#include <QVBoxLayout>
#include <algorithm>
#include <exception>
#include <memory>
#include <system_error>

#include "presentation/common/ItemRoles.h"
#include "ui/appearance/I18n.h"
#include "ui/appearance/ThemeManager.h"
#include "ui/design/DesignSystem.h"
#include "ui/dialogs/DialogAppearance.h"
#include "ui/dialogs/DialogLayout.h"
#include "ui/widgets/CommandMenu.h"
#include "ui/widgets/ContentHintBox.h"
#include "ui/widgets/ContentState.h"
#include "ui/widgets/FlowLayout.h"
#include "ui/widgets/IconView.h"
#include "ui/widgets/ItemDelegate.h"
#include "ui/widgets/Surface.h"
#include "ui/widgets/TablePresentation.h"
#include "ui/widgets/TextLabel.h"
#include "ui/widgets/VerticalScrollArea.h"

namespace uwf::ui {

bool RegistryPickerDialog::requestRejection() {
  m_viewModel->cancel();
  return true;
}

RegistryPickerDialog::RegistryPickerDialog(std::shared_ptr<presentation::RegistryPickerViewModel> model, const QString& title, QWidget* parent)
    : PanelDialog(parent), m_viewModel(std::move(model)) {
  setWindowTitle(title);
  buildUi();
  populateRootHives();
}

void RegistryPickerDialog::buildUi() {
  setDescription(supportsValueSelection() ? I18n::tr("Select a key to operate on its subtree, or a named value to operate on that value only.")
                                          : I18n::tr("Select the registry key to exclude from UWF protection, including its subkeys and values."));
  auto* frame = new DialogLayout(this, DialogContent::Scrollable, QSize(design::panelWidth, design::panelHeight), QSize(720, 500));
  auto* root = frame->body();
  root->setSpacing(16);
  auto* address = new QHBoxLayout();
  address->setSpacing(10);
  m_addressBar = new QLineEdit(this);
  m_addressBar->setObjectName(QStringLiteral("registryAddress"));
  m_addressBar->setLayoutDirection(Qt::LeftToRight);
  m_addressBar->setPlaceholderText(I18n::tr("Enter HKLM\\SOFTWARE\\… and press Enter to navigate"));
  m_addressBar->setFixedHeight(36);
  auto addressFont = m_addressBar->font();
  addressFont.setFamilies(DesignSystem::monospaceFamilies());
  addressFont.setPixelSize(12);
  m_addressBar->setFont(addressFont);
  auto* go = new QPushButton(ThemeManager::instance().icon(QStringLiteral(":/icons/arrow.svg")), {}, this);
  go->setProperty("designRole", "quiet");
  go->setObjectName(QStringLiteral("registryGoButton"));
  go->setProperty("accessibleName", I18n::tr("Navigate to registry path"));
  go->setToolTip(I18n::tr("Navigate to registry path"));
  go->setFixedSize(36, 36);
  auto* refresh = new QPushButton(ThemeManager::instance().icon(QStringLiteral(":/icons/refresh.svg")), {}, this);
  refresh->setObjectName(QStringLiteral("registryRefreshButton"));
  refresh->setProperty("designRole", "quiet");
  refresh->setProperty("accessibleName", I18n::tr("Refresh"));
  refresh->setToolTip(I18n::tr("Refresh"));
  refresh->setFixedSize(36, 36);
  connect(go, &QPushButton::clicked, this, &RegistryPickerDialog::onAddressBarReturnPressed);
  connect(refresh, &QPushButton::clicked, this, &RegistryPickerDialog::onRefresh);
  address->addWidget(m_addressBar, 1);
  address->addWidget(go);
  address->addWidget(refresh);
  root->addLayout(address);
  connect(m_addressBar, &QLineEdit::returnPressed, this, &RegistryPickerDialog::onAddressBarReturnPressed);
  connect(m_addressBar, &QLineEdit::textChanged, this, [this](const QString& text) {
    if (text.isEmpty()) {
      m_tree->clearSelection();
      m_valueTable->clearSelection();
      populateRootHives();
      refreshSelectionLabels();
      updateOkButton();
    }
  });

  auto* surface = new Surface(design::Color::Surface, this);
  surface->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);
  surface->setBorder(Qt::LeftEdge | Qt::TopEdge | Qt::RightEdge | Qt::BottomEdge, 6);
  surface->setClipContent(true);
  auto* surfaceLayout = new QHBoxLayout(surface);
  surfaceLayout->setContentsMargins(1, 1, 1, 1);
  surfaceLayout->setSpacing(0);
  auto* split = new QSplitter(Qt::Horizontal, surface);
  surface->setMinimumHeight(210);
  split->setChildrenCollapsible(false);
  m_tree = new QTreeView(split);
  m_tree->setObjectName(QStringLiteral("registryTree"));
  m_tree->verticalScrollBar()->setProperty("designThinScrollBar", true);
  m_tree->horizontalScrollBar()->setProperty("designThinScrollBar", true);
  m_tree->setProperty("designSoftSurface", true);
  m_tree->setHeaderHidden(true);
  m_tree->setUniformRowHeights(true);
  m_tree->setIndentation(16);
  m_tree->setIconSize(QSize(16, 16));
  m_tree->setSelectionMode(QAbstractItemView::SingleSelection);
  m_tree->setMouseTracking(true);
  m_tree->setModel(m_viewModel.get());
  auto* treeDelegate = new ItemDelegate(m_tree);
  treeDelegate->setRowHeight(32);
  m_tree->setItemDelegate(treeDelegate);
  auto treeFont = m_tree->font();
  treeFont.setPixelSize(12);
  m_tree->setFont(treeFont);
  m_tree->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_tree->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(m_tree, &QWidget::customContextMenuRequested, this, [this](const QPoint& point) {
    const auto index = m_tree->indexAt(point);
    if (!index.isValid()) return;
    CommandMenu menu(this);
    const auto path = index.data(presentation::PathRole).toString();
    menu.addAction(ThemeManager::instance().icon(QStringLiteral(":/icons/copy.svg")), I18n::tr("Copy path"), &menu,
                   [path] { QGuiApplication::clipboard()->setText(path); });
    auto* expand = menu.addAction(ThemeManager::instance().icon(QStringLiteral(":/icons/chevron.svg")),
                                  m_tree->isExpanded(index) ? I18n::tr("Collapse") : I18n::tr("Expand"), &menu,
                                  [this, index] { m_tree->setExpanded(index, !m_tree->isExpanded(index)); });
    expand->setEnabled(m_viewModel->hasChildren(index));
    menu.exec(m_tree->viewport()->mapToGlobal(point));
  });
  connect(m_tree, &QTreeView::expanded, this, [this](const QModelIndex& index) {
    if (m_tree->isExpanded(index)) m_viewModel->loadChildren(index);
  });
  connect(m_tree->selectionModel(), &QItemSelectionModel::selectionChanged, this, &RegistryPickerDialog::onTreeSelectionChanged);
  connect(m_viewModel.get(), &presentation::RegistryPickerViewModel::childrenLoadFailed, this,
          [this](const QModelIndex& index, const QString&) { m_tree->collapse(index); });
  split->addWidget(m_tree);
  m_details = new QStackedWidget(split);
  m_details->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);
  m_valueTable = new QTableView(m_details);
  m_valueTable->setObjectName(QStringLiteral("registryValues"));
  m_valueTable->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(m_valueTable, &QWidget::customContextMenuRequested, this, [this](const QPoint& point) {
    const auto index = m_valueTable->indexAt(point);
    if (!index.isValid()) return;
    CommandMenu menu(this);
    const auto name = index.siblingAtColumn(0).data().toString();
    menu.addAction(ThemeManager::instance().icon(QStringLiteral(":/icons/copy.svg")), I18n::tr("Copy name"), &menu,
                   [name] { QGuiApplication::clipboard()->setText(name); });
    const auto path = currentKeyPath();
    menu.addAction(ThemeManager::instance().icon(QStringLiteral(":/icons/copy.svg")), I18n::tr("Copy path"), &menu,
                   [path] { QGuiApplication::clipboard()->setText(path); });
    menu.exec(m_valueTable->viewport()->mapToGlobal(point));
  });
  m_valueTable->setModel(&m_viewModel->valueModel());
  configureReportTable(m_valueTable);
  auto valueFont = m_valueTable->font();
  valueFont.setPixelSize(13);
  m_valueTable->setFont(valueFont);
  m_valueTable->verticalHeader()->hide();
  m_valueTable->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
  m_valueTable->verticalHeader()->setDefaultSectionSize(38);
  m_valueTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_valueTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_valueTable->setSelectionMode(supportsValueSelection() ? QAbstractItemView::SingleSelection : QAbstractItemView::NoSelection);
  m_valueTable->setMouseTracking(true);
  m_valueTable->setWordWrap(false);
  m_valueTable->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
  auto* header = m_valueTable->horizontalHeader();
  header->setFixedHeight(design::tableHeaderHeight);
  header->setSectionResizeMode(QHeaderView::Interactive);
  header->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  header->resizeSection(0, 145);
  header->resizeSection(1, 112);
  header->setStretchLastSection(true);
  connect(m_valueTable->selectionModel(), &QItemSelectionModel::selectionChanged, this, &RegistryPickerDialog::onValueSelectionChanged);
  m_valueTable->viewport()->installEventFilter(this);
  m_valueTable->viewport()->setMouseTracking(true);
  m_details->addWidget(m_valueTable);
  m_accessError = new ContentState(m_details, ContentState::ScrollMode::SelfManaged);
  m_accessError->setObjectName(QStringLiteral("registryAccessError"));
  m_accessError->setText(I18n::tr("Unable to access this registry key"), I18n::tr("Choose another key, or check access permissions and try again."));
  m_accessError->setIcon(ThemeManager::instance().icon(QStringLiteral(":/icons/lock.svg")), design::Color::Danger);
  m_accessError->setAction(I18n::tr("Retry"), ThemeManager::instance().icon(QStringLiteral(":/icons/refresh.svg")));
  connect(m_accessError, &ContentState::actionRequested, this, &RegistryPickerDialog::onRefresh);
  auto* error = new QWidget(m_accessError);
  auto* errorLayout = new QVBoxLayout(error);
  errorLayout->setContentsMargins(0, 0, 0, 0);
  errorLayout->setSpacing(6);
  m_errorPath = new ContentHintBox(error);
  m_errorPath->setObjectName(QStringLiteral("registryAccessErrorPath"));
  m_errorPath->setLayoutDirection(Qt::LeftToRight);
  m_errorPath->setProperty("designPlainDocument", true);
  m_errorPath->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
  m_errorDetail = new ContentHintBox(error);
  m_errorDetail->setObjectName(QStringLiteral("registryAccessErrorDetail"));
  m_errorDetail->setProperty("designPlainDocument", true);
  m_errorDetail->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
  errorLayout->addWidget(m_errorPath);
  errorLayout->addWidget(m_errorDetail);
  m_accessError->setDetails(error);
  m_details->addWidget(m_accessError);
  connect(m_viewModel.get(), &presentation::RegistryPickerViewModel::accessStateChanged, this, &RegistryPickerDialog::renderAccessState);
  renderAccessState();
  split->addWidget(m_details);
  split->setStretchFactor(0, 0);
  split->setStretchFactor(1, 1);
  split->setSizes({232, 798});
  surfaceLayout->addWidget(split);
  root->addWidget(surface, 1);

  auto* selection = new Surface(design::Color::SurfaceSoft, this);
  selection->setBorder(Qt::LeftEdge | Qt::TopEdge | Qt::RightEdge | Qt::BottomEdge, 6);
  root->addWidget(selection);
  selection->setObjectName(QStringLiteral("registrySelection"));
  selection->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  auto* selected = new QHBoxLayout(selection);
  selected->setContentsMargins(14, 12, 14, 12);
  selected->setSpacing(14);
  auto* selectionCaption = new TextLabel(I18n::tr("Current selection"), TextStyle::Caption, selection);
  selectionCaption->setColorRole(design::Color::Muted);
  selected->addWidget(selectionCaption, 0, Qt::AlignTop);
  auto* description = new QVBoxLayout();
  description->setSpacing(3);
  m_keyLabel = new TextLabel({}, TextStyle::Caption, selection);
  m_keyLabel->setObjectName(QStringLiteral("registrySelectionPath"));
  m_keyLabel->setElideMode(Qt::ElideMiddle);
  m_keyLabel->setColorRole(design::Color::Muted);
  auto keyFont = m_keyLabel->font();
  keyFont.setFamilies(DesignSystem::monospaceFamilies());
  m_keyLabel->setFont(keyFont);
  m_keyLabel->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(m_keyLabel, &QWidget::customContextMenuRequested, this, [this](const QPoint& point) {
    CommandMenu menu(m_keyLabel);
    auto* copy = menu.addAction(ThemeManager::instance().icon(QStringLiteral(":/icons/copy.svg")), I18n::tr("Copy path"));
    connect(copy, &QAction::triggered, &menu, [this] { QGuiApplication::clipboard()->setText(currentKeyPath()); });
    menu.exec(m_keyLabel->mapToGlobal(point));
  });
  description->addWidget(m_keyLabel);
  auto* scopeHost = new QWidget(selection);
  auto* scope = new FlowLayout(scopeHost);
  scope->setContentsMargins(0, 0, 0, 0);
  scope->setSpacing(6);
  m_valueLabel = new TextLabel({}, TextStyle::Eyebrow, selection);
  m_valueLabel->setObjectName(QStringLiteral("registrySelectionScope"));
  m_valueLabel->setElideMode(Qt::ElideMiddle);
  m_valueLabel->setColorRole(design::Color::Faint);
  m_valueLabel->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(m_valueLabel, &QWidget::customContextMenuRequested, this, [this](const QPoint& point) {
    CommandMenu menu(m_valueLabel);
    auto* copy = menu.addAction(ThemeManager::instance().icon(QStringLiteral(":/icons/copy.svg")), I18n::tr("Copy selection"));
    connect(copy, &QAction::triggered, &menu, [this] {
      const auto value = currentValueName();
      QGuiApplication::clipboard()->setText(currentKeyPath() + (value.isEmpty() ? QString{} : u'\n' + value));
    });
    menu.exec(m_valueLabel->mapToGlobal(point));
  });
  scope->addWidget(m_valueLabel);
  m_wholeKey = new QPushButton(I18n::tr("Use the whole key, including all subkeys"), selection);
  m_wholeKey->setProperty("designRole", "link");
  m_wholeKey->setProperty("designPaddingX", 0);
  auto scopeFont = m_wholeKey->font();
  scopeFont.setPixelSize(12);
  m_wholeKey->setFont(scopeFont);
  m_wholeKey->setFixedHeight(30);
  connect(m_wholeKey, &QPushButton::clicked, m_valueTable, &QTableView::clearSelection);
  scope->addWidget(m_wholeKey);
  description->addWidget(scopeHost);
  selected->addLayout(description, 1);
  auto* buttons = new QDialogButtonBox(this);
  m_okBtn = buttons->addButton(supportsValueSelection() ? I18n::tr("Select this registry key") : I18n::tr("Add exclusion"), QDialogButtonBox::AcceptRole);
  buttons->addButton(I18n::tr("Cancel"), QDialogButtonBox::RejectRole);
  m_okBtn->setEnabled(false);
  setDialogAction(m_okBtn, DialogAction::Primary);
  connect(buttons, &QDialogButtonBox::accepted, this, [this] {
    m_viewModel->accept(currentKeyPath(), currentValueName());
    if (m_viewModel->selection()) accept();
  });
  connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
  frame->setActions(buttons);
  refreshSelectionLabels();
}

void RegistryPickerDialog::populateRootHives() {
  m_tree->clearSelection();
  m_viewModel->resetTree();
  for (int row = 0; row < m_viewModel->rowCount(); ++row) {
    const auto index = m_viewModel->index(row, 0);
    if (index.data().toString() == QLatin1String("HKEY_LOCAL_MACHINE") && m_viewModel->loadChildren(index)) m_tree->expand(index);
  }
}

RegistryPickerDialog::KeyAvailability RegistryPickerDialog::standardAvailability(const QString& key) {
  return presentation::RegistryPickerViewModel::standardAvailability(key);
}

void RegistryPickerDialog::onRefresh() {
  // 重建后按原地址恢复选择；目标已删除时停在最深可达节点。
  const QString currentPath = m_addressBar->text().trimmed();
  populateRootHives();
  if (!currentPath.isEmpty()) navigateTo(currentPath);
}

void RegistryPickerDialog::onTreeSelectionChanged() {
  const QString key = currentKeyPath();
  // 地址栏跟随选中；程序写入文本不会触发按键导航。
  if (!key.isEmpty()) m_addressBar->setText(key);
  refreshValueTable(key);
  refreshSelectionLabels();
  updateOkButton();
}

void RegistryPickerDialog::onValueSelectionChanged() {
  refreshSelectionLabels();
  updateOkButton();
}

bool RegistryPickerDialog::eventFilter(QObject* obj, QEvent* ev) {
  if (supportsValueSelection() && m_valueTable && obj == m_valueTable->viewport() && ev->type() == QEvent::MouseButtonPress) {
    const auto* me = static_cast<QMouseEvent*>(ev);
    if (!m_valueTable->indexAt(me->pos()).isValid()) {
      // 保留默认事件处理，仅清除值选择以恢复整键递归语义。
      m_valueTable->clearSelection();
    }
  }
  return PanelDialog::eventFilter(obj, ev);
}

void RegistryPickerDialog::refreshValueTable(const QString& keyPath) {
  if (!m_valueTable) return;
  m_viewModel->loadValues(keyPath);
}

void RegistryPickerDialog::renderAccessState() {
  const auto& error = m_viewModel->accessError();
  if (!error) {
    m_errorPath->clear();
    m_errorDetail->clear();
    m_details->setCurrentWidget(m_valueTable);
    return;
  }
  m_errorPath->setPlainText(error->key);
  m_errorDetail->setPlainText((error->operation == presentation::RegistryPickerViewModel::AccessError::Operation::Keys
                                   ? I18n::tr("Failed to enumerate registry keys: %1")
                                   : I18n::tr("Failed to read registry values: %1"))
                                  .arg(error->message));
  m_details->setCurrentWidget(m_accessError);
}

void RegistryPickerDialog::onAddressBarReturnPressed() {
  const QString raw = m_addressBar->text().trimmed();
  if (raw.isEmpty()) return;
  navigateTo(raw);
}

void RegistryPickerDialog::navigateTo(const QString& keyPath) {
  const auto current = m_viewModel->locateKey(keyPath);
  if (!current.isValid()) return;
  for (auto ancestor = current.parent(); ancestor.isValid(); ancestor = ancestor.parent()) m_tree->expand(ancestor);
  m_tree->setCurrentIndex(current);
  m_tree->scrollTo(current);
}

void RegistryPickerDialog::refreshSelectionLabels() {
  const QString key = currentKeyPath();
  m_keyLabel->setText(key.isEmpty() ? I18n::tr("No registry key selected") : key);
  m_keyLabel->setToolTip(key);
  const QString value = currentValueName();
  const bool selectedValue = supportsValueSelection() && !value.isEmpty();
  const QString scope = key.isEmpty()               ? I18n::tr("Select an available subkey")
                        : !supportsValueSelection() ? I18n::tr("Global registry exclusion · includes subkeys")
                        : selectedValue             ? I18n::tr("Named value: %1 · this value only").arg(value)
                                                    : I18n::tr("Whole key and all subkeys · recursive");
  m_valueLabel->setText(scope);
  m_valueLabel->setToolTip(scope);
  m_wholeKey->setVisible(selectedValue);
  if (supportsValueSelection()) m_okBtn->setText(selectedValue ? I18n::tr("Select this registry value") : I18n::tr("Select this registry key"));
}

void RegistryPickerDialog::updateOkButton() { m_okBtn->setEnabled(m_viewModel->canAccept(currentKeyPath())); }

QString RegistryPickerDialog::currentKeyPath() const {
  const auto indexes = m_tree->selectionModel()->selectedRows();
  return indexes.isEmpty() ? QString{} : indexes.front().data(presentation::PathRole).toString();
}

QString RegistryPickerDialog::currentValueName() const {
  // Exclusion 模式固定返回空值名，保持仅操作键的调用约定。
  if (!supportsValueSelection() || !m_valueTable) return {};
  const auto indexes = m_valueTable->selectionModel()->selectedRows();
  return indexes.isEmpty() ? QString{} : indexes.front().data(presentation::RegistryValueModel::NameRole).toString();
}

void RegistryPickerDialog::setAvailabilityChecker(AvailabilityChecker checker) {
  m_viewModel->setAvailabilityChecker(std::move(checker));
  // 可用性策略影响节点标志和占位项，变更策略后须重建树。
  populateRootHives();
  updateOkButton();
}

void RegistryPickerDialog::preselectKey(const QString& key) {
  if (key.isEmpty()) return;
  m_addressBar->setText(key);
  navigateTo(key);
}

std::optional<RegistryPickerDialog::Result> RegistryPickerDialog::pick(std::shared_ptr<presentation::RegistryPickerViewModel> model, const QString& title,
                                                                       QWidget* parent, AvailabilityChecker checker, const QString& preselectKey) {
  RegistryPickerDialog dlg(std::move(model), title, parent);
  if (checker) dlg.setAvailabilityChecker(std::move(checker));
  if (!preselectKey.isEmpty()) dlg.preselectKey(preselectKey);
  if (dlg.exec() != QDialog::Accepted) return std::nullopt;
  return dlg.selection();
}

}  // namespace uwf::ui

#include "moc_RegistryPickerDialog.cpp"
