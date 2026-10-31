// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "FilePickerDialog.h"

#include <QAbstractButton>
#include <QClipboard>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileInfo>
#include <QGridLayout>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QLineEdit>
#include <QListView>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QShortcut>
#include <QSignalBlocker>
#include <QStackedWidget>
#include <QStyledItemDelegate>
#include <QTableView>
#include <QVBoxLayout>
#include <utility>

#include "presentation/files/FilePickerViewModel.h"
#include "presentation/common/ItemRoles.h"
#include "ui/appearance/I18n.h"
#include "ui/appearance/ThemeManager.h"
#include "ui/design/ControlAnimation.h"
#include "ui/design/ControlShape.h"
#include "ui/design/DesignSystem.h"
#include "ui/design/IconPainting.h"
#include "ui/design/ItemHighlight.h"
#include "ui/design/TextPainting.h"
#include "ui/dialogs/DialogLayout.h"
#include "ui/dialogs/Dialogs.h"
#include "ui/widgets/CommandMenu.h"
#include "ui/widgets/BusyIndicator.h"
#include "ui/widgets/IconView.h"
#include "ui/widgets/ItemDelegate.h"
#include "ui/widgets/SearchField.h"
#include "ui/widgets/SelectBox.h"
#include "ui/widgets/Surface.h"
#include "ui/widgets/TablePresentation.h"
#include "ui/widgets/TextLabel.h"
#include "ui/widgets/SelectionListView.h"

namespace uwf::ui {
class FilePickerState final : public QWidget {
 public:
  explicit FilePickerState(QWidget* parent)
      : QWidget(parent),
        m_symbols(new QStackedWidget(this)),
        m_icon(new IconView({}, 20, design::Color::Muted, m_symbols)),
        m_spinner(new BusyIndicator(m_symbols)),
        m_title(new TextLabel({}, TextStyle::Caption, this)),
        m_description(new TextLabel({}, TextStyle::Caption, this)),
        m_actions(new QWidget(this)),
        m_retry(new QPushButton(I18n::tr("Retry"), m_actions)),
        m_details(new QPushButton(I18n::tr("Error details"), m_actions)) {
    setObjectName(QStringLiteral("filePickerPlaceholder"));
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 8, 20, 8);
    layout->setSpacing(6);
    layout->addStretch();
    auto* heading = new QWidget(this);
    heading->setMaximumWidth(420);
    auto* row = new QHBoxLayout(heading);
    row->setContentsMargins(0, 0, 0, 0);
    row->setSpacing(8);
    m_symbols->setFixedSize(20, 20);
    m_spinner->setStrokeWidth(1.3);
    m_spinner->setColorRole(design::Color::Muted);
    m_symbols->addWidget(m_icon);
    m_symbols->addWidget(m_spinner);
    row->addWidget(m_symbols);
    auto titleFont = m_title->font();
    titleFont.setPixelSize(14);
    titleFont.setWeight(QFont::DemiBold);
    m_title->setFont(titleFont);
    m_title->setLineHeight(1.4);
    m_title->setWordWrap(true);
    m_title->setObjectName(QStringLiteral("filePickerStateTitle"));
    row->addWidget(m_title, 1);
    layout->addWidget(heading, 0, Qt::AlignHCenter);
    m_description->setObjectName(QStringLiteral("filePickerStateDescription"));
    m_description->setColorRole(design::Color::Muted);
    m_description->setWordWrap(true);
    m_description->setLineHeight(1.5);
    m_description->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    m_description->setMaximumWidth(420);
    layout->addWidget(m_description, 0, Qt::AlignHCenter);
    auto* actions = new QHBoxLayout(m_actions);
    actions->setContentsMargins(0, 0, 0, 0);
    actions->setSpacing(8);
    m_retry->setObjectName(QStringLiteral("filePickerRetry"));
    m_retry->setIcon(ThemeManager::instance().icon(QStringLiteral(":/icons/refresh.svg")));
    m_details->setObjectName(QStringLiteral("filePickerErrorDetails"));
    m_details->setProperty("designRole", "quiet");
    for (auto* button : {m_retry, m_details}) {
      button->setFixedHeight(30);
      actions->addWidget(button);
    }
    layout->addWidget(m_actions, 0, Qt::AlignHCenter);
    layout->addStretch();
  }
  void render(const bool loading, const bool failed, const QIcon& icon, const QString& title, const QString& description) {
    m_symbols->setCurrentWidget(loading ? static_cast<QWidget*>(m_spinner) : m_icon);
    m_icon->setIcon(icon);
    m_icon->setColorRole(failed ? design::Color::Danger : design::Color::Muted);
    m_title->setText(title);
    m_description->setText(description);
    m_actions->setVisible(failed);
  }
  [[nodiscard]] QPushButton* retryButton() const { return m_retry; }
  [[nodiscard]] QPushButton* detailsButton() const { return m_details; }

 private:
  QStackedWidget* m_symbols;
  IconView* m_icon;
  BusyIndicator* m_spinner;
  TextLabel* m_title;
  TextLabel* m_description;
  QWidget* m_actions;
  QPushButton* m_retry;
  QPushButton* m_details;
};

namespace {
using Mode = presentation::FilePickerViewModel::Mode;
using State = presentation::ContentLoadState;
class PickerTable final : public QTableView {
 public:
  explicit PickerTable(QWidget* parent) : QTableView(parent) {}

 protected:
  QItemSelectionModel::SelectionFlags selectionCommand(const QModelIndex& index, const QEvent* event) const override {
    if (selectionMode() == ExtendedSelection && index.isValid() && event && event->type() == QEvent::MouseButtonPress) {
      const auto* mouse = static_cast<const QMouseEvent*>(event);
      if (mouse->button() == Qt::LeftButton && mouse->modifiers() == Qt::NoModifier) return QItemSelectionModel::Toggle | QItemSelectionModel::Rows;
    }
    return QTableView::selectionCommand(index, event);
  }
};
class PlaceDelegate final : public QStyledItemDelegate {
 public:
  explicit PlaceDelegate(QListView* view) : QStyledItemDelegate(view), m_view(view) {}
  QSize sizeHint(const QStyleOptionViewItem&, const QModelIndex& index) const override {
    Q_UNUSED(index);
    return {150, 37};
  }
  void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
    const auto& system = DesignSystem::instance();
    const bool heading = index.data(presentation::SeparatorRole).toBool();
    auto font = option.font;
    font.setPixelSize(heading ? 11 : 13);
    painter->save();
    painter->setFont(font);
    if (heading) {
      painter->setPen(system.color(design::Color::Faint));
      paintTextLine(*painter, option.rect.adjusted(11, 11, -11, -7), index.data().toString(), font);
    } else {
      const bool selected = index.data(Qt::CheckStateRole).toInt() == Qt::Checked;
      const qreal hover = ItemHighlight::opacity(m_view->viewport(), option.rect.topLeft(), option.state.testFlag(QStyle::State_MouseOver));
      const auto fill = DesignSystem::mix(selected ? system.color(design::Color::Selected) : QColor(Qt::transparent), system.color(design::Color::Hover), hover);
      paintControlShape(*painter, option.rect, 4, fill);
      const auto visual = [&](const QRect& bounds) { return QStyle::visualRect(option.direction, option.rect, bounds); };
      const auto icon = ThemeManager::instance().icon(index.data(presentation::IconResourceRole).toString());
      paintIcon(*painter, icon, visual(QRect(option.rect.left() + 12, option.rect.center().y() - 9, 18, 18)),
                system.color(index.data(presentation::SystemMetadataRole).toBool() ? design::Color::Accent : design::Color::Muted));
      painter->setPen(system.color(design::Color::Text));
      paintTextLine(*painter, visual(option.rect.adjusted(40, 0, -12, 0)), QFontMetrics(font).elidedText(index.data().toString(), Qt::ElideRight, option.rect.width() - 52), font);
    }
    painter->restore();
  }
 private:
  QListView* m_view;
};
QListView* createPlacesView(presentation::FilePickerViewModel& model, QWidget* parent) {
  auto* view = new SelectionListView(parent);
  view->setObjectName(QStringLiteral("filePickerPlaces"));
  view->setModel(model.placesModel());
  view->setItemDelegate(new PlaceDelegate(view));
  view->setEditTriggers(QAbstractItemView::NoEditTriggers);
  view->setSelectionMode(QAbstractItemView::SingleSelection);
  view->setMouseTracking(true);
  view->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
  view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  view->setFrameShape(QFrame::NoFrame);
  view->setStyleSheet(QStringLiteral("QListView { background: transparent; border: none; }"));
  const auto navigate = [&model](const QModelIndex& index) {
    const auto path = index.data(presentation::PathRole).toString();
    if (!path.isEmpty()) model.navigate(path);
  };
  QObject::connect(view, &QListView::clicked, &model, navigate);
  QObject::connect(view, &QListView::activated, &model, navigate);
  auto* activate = new QShortcut(QKeySequence(Qt::Key_Space), view);
  activate->setContext(Qt::WidgetShortcut);
  QObject::connect(activate, &QShortcut::activated, &model, [view, navigate] { navigate(view->currentIndex()); });
  return view;
}
QPushButton* navigationButton(const QString& resource, const QString& label, QWidget* parent) {
  auto* button = new QPushButton(ThemeManager::instance().icon(resource), {}, parent);
  button->setFixedSize(32, 32);
  button->setProperty("designRole", "quiet");
  button->setProperty("accessibleName", label);
  button->setToolTip(label);
  return button;
}
}  // namespace
FilePickerDialog::FilePickerDialog(std::shared_ptr<presentation::FilePickerViewModel> model, const QString& title, QWidget* parent)
    : PanelDialog(parent),
      m_model(std::move(model)),
      m_address(new QLineEdit(this)),
      m_search(new SearchField(this)),
      m_name(new QLineEdit(this)),
      m_filter(new SelectBox(this)),
      m_table(new PickerTable(this)),
      m_pages(new QStackedWidget(this)),
      m_state(new FilePickerState(this)),
      m_count(new TextLabel({}, TextStyle::Eyebrow, this)),
      m_accept(nullptr),
      m_back(nullptr),
      m_up(nullptr),
      m_newFolder(nullptr) {
  setObjectName(QStringLiteral("filePickerDialog"));
  setWindowTitle(title);
  const auto mode = m_model->mode();
  const bool folder = mode == Mode::Directory;
  const bool saving = mode == Mode::SaveFile;
  const bool mixed = mode == Mode::Mixed;
  auto* frame = new DialogLayout(this, DialogContent::Form, QSize(design::panelWidth, design::panelHeight), QSize(720, 550));
  auto* body = frame->body();
  body->setSpacing(17);
  auto* toolbar = new QHBoxLayout();
  toolbar->setSpacing(12);
  auto* navigation = new QHBoxLayout();
  navigation->setSpacing(2);
  m_back = navigationButton(QStringLiteral(":/icons/page_previous.svg"), I18n::tr("Back"), this);
  m_up = navigationButton(QStringLiteral(":/icons/arrow_up.svg"), I18n::tr("Up one folder"), this);
  m_up->setObjectName(QStringLiteral("filePickerUp"));
  auto* refresh = navigationButton(QStringLiteral(":/icons/refresh.svg"), I18n::tr("Refresh folder"), this);
  navigation->addWidget(m_back);
  navigation->addWidget(m_up);
  navigation->addWidget(refresh);
  toolbar->addLayout(navigation);
  m_address->setObjectName(QStringLiteral("pickerAddress"));
  m_address->setProperty("accessibleName", I18n::tr("Folder address"));
  m_address->setFixedHeight(35);
  m_address->addAction(ThemeManager::instance().icon(QStringLiteral(":/icons/folder.svg")), QLineEdit::LeadingPosition);
  auto addressFont = m_address->font();
  addressFont.setPixelSize(13);
  addressFont.setFamilies(DesignSystem::monospaceFamilies());
  m_address->setFont(addressFont);
  toolbar->addWidget(m_address, 1);
  m_search->setObjectName(QStringLiteral("pickerSearch"));
  m_search->setPlaceholderText(I18n::tr("Search this folder"));
  m_search->setProperty("accessibleName", m_search->placeholderText());
  m_search->setFixedHeight(35);
  m_search->setFixedWidth(207);
  toolbar->addWidget(m_search, 0);
  body->addLayout(toolbar);

  auto* browser = new Surface(design::Color::Surface, this);
  browser->setBorder(Qt::LeftEdge | Qt::TopEdge | Qt::RightEdge | Qt::BottomEdge, 6);
  browser->setClipContent(true);
  browser->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);
  auto* browserRow = new QHBoxLayout(browser);
  browserRow->setContentsMargins(1, 1, 1, 1);
  browserRow->setSpacing(0);
  auto* places = new Surface(design::Color::SurfaceSoft, browser);
  places->setBorder(Qt::RightEdge);
  places->setFixedWidth(177);
  auto* placesLayout = new QVBoxLayout(places);
  placesLayout->setContentsMargins(7, 9, 7, 9);
  placesLayout->setSpacing(0);
  auto* placeList = createPlacesView(*m_model, places);
  placesLayout->addWidget(placeList, 1);
  browserRow->addWidget(places);
  auto* content = new QWidget(browser);
  content->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);
  auto* contentLayout = new QVBoxLayout(content);
  contentLayout->setContentsMargins(0, 0, 0, 0);
  contentLayout->setSpacing(0);
  auto* listToolbar = new Surface(design::Color::Surface, content);
  listToolbar->setBorder(Qt::BottomEdge);
  listToolbar->setFixedHeight(43);
  auto* listCommands = new QHBoxLayout(listToolbar);
  listCommands->setContentsMargins(14, 0, 14, 0);
  auto* heading = new TextLabel(folder ? I18n::tr("Folders") : I18n::tr("Files and folders"), TextStyle::Caption, listToolbar);
  heading->setColorRole(design::Color::Muted);
  listCommands->addWidget(heading, 1);
  auto* create = new QPushButton(ThemeManager::instance().icon(QStringLiteral(":/icons/add.svg")), I18n::tr("New folder"), listToolbar);
  m_newFolder = create;
  create->setProperty("designRole", "quiet");
  create->setFixedHeight(30);
  auto smallFont = create->font();
  smallFont.setPixelSize(12);
  create->setFont(smallFont);
  listCommands->addWidget(create);
  contentLayout->addWidget(listToolbar);
  configureReportTable(m_table);
  m_table->setObjectName(QStringLiteral("pickerEntries"));
  m_table->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(m_table, &QWidget::customContextMenuRequested, this, [this](const QPoint& point) {
    const auto index = m_table->indexAt(point);
    CommandMenu menu(this);
    auto& theme = ThemeManager::instance();
    if (index.isValid()) {
      if (!m_table->selectionModel()->isRowSelected(index.row(), {})) m_table->selectRow(index.row());
      const int row = index.row();
      menu.addAction(theme.icon(m_model->isDirectory(row) ? ":/icons/folder.svg" : ":/icons/file.svg"),
                     m_model->isDirectory(row) ? I18n::tr("Open folder") : I18n::tr("Select this file"), &menu, [this, row] {
                       if (m_model->isDirectory(row))
                         m_model->navigate(m_model->entryPath(row));
                       else
                         acceptSelection();
                     });
      menu.addAction(theme.icon(":/icons/copy.svg"), I18n::tr("Copy path"), &menu,
                     [this] { QGuiApplication::clipboard()->setText(m_model->selectedEntries().join(u'\n')); });
      menu.addAction(theme.icon(":/icons/file.svg"), I18n::tr("Rename…"), &menu, [this, row] { renameEntry(row); });
      menu.addSeparator();
    }
    menu.addAction(theme.icon(":/icons/refresh.svg"), I18n::tr("Refresh"), m_model.get(), &presentation::FilePickerViewModel::refresh);
    auto* create = menu.addAction(theme.icon(":/icons/folder.svg"), I18n::tr("New folder"), this, &FilePickerDialog::createDirectory);
    create->setEnabled(m_newFolder && m_newFolder->isEnabled());
    menu.exec(m_table->viewport()->mapToGlobal(point));
  });
  m_table->setFrameShape(QFrame::NoFrame);
  m_table->setModel(m_model.get());
  m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_table->setSelectionMode(m_model->multiple() ? QAbstractItemView::ExtendedSelection : QAbstractItemView::SingleSelection);
  m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_table->setShowGrid(false);
  m_table->setWordWrap(false);
  m_table->setIconSize(QSize(18, 18));
  auto tableFont = m_table->font();
  tableFont.setPixelSize(13);
  m_table->setFont(tableFont);
  m_table->verticalHeader()->hide();
  m_table->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
  m_table->verticalHeader()->setDefaultSectionSize(design::tableRowHeight);
  m_table->horizontalHeader()->setFixedHeight(design::tableHeaderHeight);
  m_table->horizontalHeader()->setStretchLastSection(false);
  m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
  m_table->setColumnWidth(1, 150);
  m_table->setColumnWidth(2, 140);
  m_table->setColumnWidth(3, 75);
  m_table->sortByColumn(0, Qt::AscendingOrder);
  m_table->setSortingEnabled(true);
  m_pages->addWidget(m_table);
  m_pages->addWidget(m_state);
  m_pages->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);
  contentLayout->addWidget(m_pages, 1);
  auto* count = new Surface(design::Color::Surface, content);
  count->setBorder(Qt::TopEdge);
  auto* countRow = new QHBoxLayout(count);
  countRow->setContentsMargins(15, 10, 15, 10);
  m_count->setColorRole(design::Color::Faint);
  countRow->addWidget(m_count);
  contentLayout->addWidget(count);
  browserRow->addWidget(content, 1);
  body->addWidget(browser, 1);

  auto* fields = new QGridLayout();
  fields->setHorizontalSpacing(12);
  fields->setVerticalSpacing(10);
  auto* nameLabel = new TextLabel(mixed ? I18n::tr("Selection") : folder ? I18n::tr("Folder") : I18n::tr("File name"), TextStyle::Caption, this);
  nameLabel->setColorRole(design::Color::Muted);
  nameLabel->setBuddy(m_name);
  fields->addWidget(nameLabel, 0, 0);
  m_name->setObjectName(QStringLiteral("pickerFilename"));
  m_name->setProperty("accessibleName", nameLabel->text());
  m_name->setReadOnly(folder || mixed);
  m_name->setPlaceholderText(folder   ? I18n::tr("Select the current folder")
                             : mixed  ? I18n::tr("Select files or folders")
                             : saving ? I18n::tr("Enter a file name")
                                      : I18n::tr("Select a file or enter its name"));
  auto nameFont = m_name->font();
  nameFont.setPixelSize(13);
  m_name->setFont(nameFont);
  m_name->setFixedHeight(36);
  fields->addWidget(m_name, 0, 1);
  fields->setColumnStretch(1, 1);
  if (!folder) {
    auto* type = new TextLabel(I18n::tr("File type"), TextStyle::Caption, this);
    type->setColorRole(design::Color::Muted);
    type->setBuddy(m_filter);
    fields->addWidget(type, 0, 2);
    m_filter->setObjectName(QStringLiteral("pickerFileType"));
    for (const auto filters = m_model->filters(); const auto& filter : filters) {
      const int pattern = static_cast<int>(filter.lastIndexOf(QStringLiteral(" (")));
      m_filter->addItem(pattern > 0 ? filter.left(pattern) : filter);
      m_filter->setItemData(m_filter->count() - 1, filter, Qt::ToolTipRole);
    }
    m_filter->setCurrentIndex(m_model->filterIndex());
    m_filter->setFixedWidth(182);
    m_filter->setFixedHeight(36);
    fields->addWidget(m_filter, 0, 3);
  } else
    m_filter->hide();
  body->addLayout(fields);
  auto* note = new QWidget(this);
  note->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  auto* noteRow = new QHBoxLayout(note);
  noteRow->setContentsMargins(0, 0, 0, 0);
  noteRow->setSpacing(8);
  noteRow->addWidget(new IconView(ThemeManager::instance().icon(QStringLiteral(":/icons/folder.svg")), 16, design::Color::Muted, note));
  auto* hint = new TextLabel(mixed                 ? I18n::tr("Files and folders can be selected together.")
                             : folder              ? I18n::tr("Select folders only.")
                             : saving              ? I18n::tr("Choose a location to save the file.")
                             : m_model->multiple() ? I18n::tr("Multiple files can be selected.")
                                                   : I18n::tr("Select a file to open."),
                             TextStyle::Caption, note);
  hint->setColorRole(design::Color::Muted);
  hint->setWordWrap(true);
  noteRow->addWidget(hint, 1);
  auto* buttons = new QDialogButtonBox(this);
  m_accept = buttons->addButton(saving   ? I18n::tr("Save")
                                : mixed  ? I18n::tr("Add selected items")
                                : folder ? I18n::tr("Select folder")
                                         : I18n::tr("Open"),
                                QDialogButtonBox::AcceptRole);
  m_accept->setObjectName(QStringLiteral("pickerAccept"));
  setDialogAction(m_accept, DialogAction::Primary);
  m_accept->setAutoDefault(false);
  auto* cancel = buttons->addButton(I18n::tr("Cancel"), QDialogButtonBox::RejectRole);
  cancel->setDefault(true);
  cancel->setFocus(Qt::OtherFocusReason);
  frame->setActions(buttons, note);
  connect(buttons, &QDialogButtonBox::accepted, this, &FilePickerDialog::acceptSelection);
  connect(buttons, &QDialogButtonBox::rejected, this, &FilePickerDialog::reject);
  connect(m_back, &QAbstractButton::clicked, m_model.get(), &presentation::FilePickerViewModel::goBack);
  connect(m_up, &QAbstractButton::clicked, m_model.get(), &presentation::FilePickerViewModel::goUp);
  connect(refresh, &QPushButton::clicked, m_model.get(), &presentation::FilePickerViewModel::refresh);
  connect(create, &QPushButton::clicked, this, &FilePickerDialog::createDirectory);
  connect(m_address, &QLineEdit::returnPressed, this, [this] { m_model->navigate(m_address->text()); });
  connect(m_search, &QLineEdit::textChanged, m_model.get(), &presentation::FilePickerViewModel::setQuery);
  connect(m_name, &QLineEdit::textEdited, this, [this](const QString& value) {
    m_table->clearSelection();
    m_model->setFilename(value);
  });
  connect(m_name, &QLineEdit::returnPressed, this, &FilePickerDialog::acceptSelection);
  connect(m_filter, &QComboBox::currentIndexChanged, m_model.get(), &presentation::FilePickerViewModel::setFilterIndex);
  connect(m_table->selectionModel(), &QItemSelectionModel::selectionChanged, this, [this] {
    QList<int> rows;
    if (m_resetting) return;
    const auto selected = m_table->selectionModel()->selectedRows();
    for (const auto& index : selected) rows.append(index.row());
    m_model->selectRows(rows);
  });
  connect(m_model.get(), &QAbstractItemModel::modelAboutToBeReset, this, [this] { m_resetting = true; });
  connect(m_model.get(), &QAbstractItemModel::modelReset, this, [this] {
    const QSignalBlocker blocker(m_table->selectionModel());
    for (int row = 0; row < m_model->rowCount(); ++row)
      if (m_model->selectedEntries().contains(m_model->entryPath(row)))
        m_table->selectionModel()->select(m_model->index(row, 0), QItemSelectionModel::Select | QItemSelectionModel::Rows);
    m_resetting = false;
  });
  connect(m_table, &QTableView::doubleClicked, this, [this](const QModelIndex& index) {
    if (index.isValid()) m_model->selectRows({index.row()});
    if (m_model->isDirectory(index.row()))
      m_model->navigate(m_model->entryPath(index.row()));
    else
      acceptSelection();
  });
  connect(m_state->retryButton(), &QPushButton::clicked, m_model.get(), &presentation::FilePickerViewModel::refresh);
  connect(m_state->detailsButton(), &QPushButton::clicked, this,
          [this] { dialogs::information(this, I18n::tr("Error details"), m_model->error()); });
  connect(&I18n::events(), &LanguageEvents::changed, m_model.get(), &presentation::FilePickerViewModel::retranslate);
  connect(m_model.get(), &presentation::FilePickerViewModel::stateChanged, this, &FilePickerDialog::renderState);
  connect(m_model.get(), &presentation::FilePickerViewModel::selectionChanged, this, &FilePickerDialog::renderSelection);
  connect(m_model.get(), &presentation::FilePickerViewModel::validationFailed, this,
          [this](const QString& error) { dialogs::warning(this, I18n::tr("Unable to select this item"), error); });
  for (const auto& [key, action] : {std::pair{QKeySequence(Qt::AltModifier | Qt::Key_Left), &presentation::FilePickerViewModel::goBack},
                                    std::pair{QKeySequence(Qt::AltModifier | Qt::Key_Right), &presentation::FilePickerViewModel::goForward},
                                    std::pair{QKeySequence(Qt::AltModifier | Qt::Key_Up), &presentation::FilePickerViewModel::goUp},
                                    std::pair{QKeySequence(Qt::Key_F5), &presentation::FilePickerViewModel::refresh}}) {
    auto* shortcut = new QShortcut(key, this);
    connect(shortcut, &QShortcut::activated, m_model.get(), action);
  }
  auto* addressShortcut = new QShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_L), this);
  connect(addressShortcut, &QShortcut::activated, this, [this] {
    m_address->setFocus();
    m_address->selectAll();
  });
  auto* activate = new QShortcut(QKeySequence(Qt::Key_Return), m_table);
  activate->setContext(Qt::WidgetShortcut);
  connect(activate, &QShortcut::activated, this, &FilePickerDialog::acceptSelection);
  renderState();
}
void FilePickerDialog::renderState() {
  const bool busy = m_model->loadState() == State::Loading;
  const bool loading = busy && m_model->progressVisible();
  const bool failed = m_model->loadState() == State::Failed;
  const QSignalBlocker block(m_address);
  m_address->setText(QDir::toNativeSeparators(m_model->path()));
  if (busy) {
    const QSignalBlocker query(m_search);
    m_search->clear();
  }
  m_back->setEnabled(m_model->canGoBack());
  m_up->setEnabled(m_model->canGoUp());
  m_newFolder->setEnabled(!busy && !failed);
  m_table->setEnabled(!busy && !failed);
  const bool empty = m_model->rowCount() == 0;
  m_pages->setCurrentWidget(loading || failed || (!busy && empty) ? static_cast<QWidget*>(m_state) : m_table);
  m_state->render(loading, failed,
                   ThemeManager::instance().icon(failed                       ? QStringLiteral(":/icons/lock.svg")
                                                  : m_search->text().isEmpty() ? QStringLiteral(":/icons/folder.svg")
                                                                               : QStringLiteral(":/icons/search.svg")),
                   loading                      ? I18n::tr("Reading folder")
                   : failed                     ? I18n::tr("Unable to access this folder")
                   : m_search->text().isEmpty() ? I18n::tr("This folder is empty")
                                                : I18n::tr("No matching items"),
                   loading                      ? I18n::tr("Reading files and subfolders in this location…")
                   : failed                     ? I18n::tr("Choose another folder, or check access permissions and retry.")
                   : m_search->text().isEmpty() ? I18n::tr("Go up one folder or choose another location.")
                                                : I18n::tr("Try another search term."));
  renderSelection();
}
void FilePickerDialog::renderSelection() {
  const QSignalBlocker blocker(m_name);
  const auto folderName = QFileInfo(m_model->path()).fileName();
  m_name->setText(m_model->mode() == Mode::Directory && m_model->filename().isEmpty()
                      ? folderName.isEmpty() ? QDir::toNativeSeparators(m_model->path()) : folderName
                      : m_model->filename());
  m_accept->setEnabled(m_model->canAccept());
  const auto selected = m_table->selectionModel()->selectedRows().count();
  m_count->setText(I18n::tr("%1 items").arg(m_model->rowCount()) + (selected ? I18n::tr(" · %1 selected").arg(selected) : QString{}));
}
void FilePickerDialog::acceptSelection() {
  const auto status = m_model->prepareAcceptance();
  if (status == presentation::FilePickerViewModel::Acceptance::Replace) {
    if (!dialogs::confirm(this,
                          {I18n::tr("Replace the existing file?"),
                           I18n::tr("A file already exists at this location:\n%1\n\nSaving will replace its contents.").arg(m_model->selectedPaths().value(0)),
                           I18n::tr("Replace file"), DialogAction::Destructive}))
      return;
  } else if (status != presentation::FilePickerViewModel::Acceptance::Ready)
    return;
  accept();
}
void FilePickerDialog::createDirectory() {
  PanelDialog dialog(this);
  dialog.setWindowTitle(I18n::tr("New folder"));
  auto* frame = new DialogLayout(&dialog, DialogContent::Form, QSize(440, 0));
  auto* name = new QLineEdit(&dialog);
  name->setProperty("accessibleName", I18n::tr("Folder name"));
  name->setText(I18n::tr("New folder"));
  name->selectAll();
  frame->body()->addWidget(name);
  auto* buttons = new QDialogButtonBox(&dialog);
  auto* create = buttons->addButton(I18n::tr("Create"), QDialogButtonBox::AcceptRole);
  setDialogAction(create, DialogAction::Primary);
  buttons->addButton(I18n::tr("Cancel"), QDialogButtonBox::RejectRole);
  frame->setActions(buttons);
  connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
  connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
  connect(name, &QLineEdit::returnPressed, &dialog, &QDialog::accept);
  if (dialog.exec() == QDialog::Accepted) m_model->createDirectory(name->text());
}
void FilePickerDialog::renameEntry(const int row) {
  PanelDialog dialog(this);
  dialog.setWindowTitle(I18n::tr("Rename"));
  auto* frame = new DialogLayout(&dialog, DialogContent::Form, QSize(440, 0));
  auto* name = new QLineEdit(QFileInfo(m_model->entryPath(row)).fileName(), &dialog);
  name->setProperty("accessibleName", I18n::tr("File name"));
  name->selectAll();
  frame->body()->addWidget(name);
  auto* buttons = new QDialogButtonBox(&dialog);
  auto* accept = buttons->addButton(I18n::tr("Rename"), QDialogButtonBox::AcceptRole);
  setDialogAction(accept, DialogAction::Primary);
  buttons->addButton(I18n::tr("Cancel"), QDialogButtonBox::RejectRole);
  frame->setActions(buttons);
  connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
  connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
  connect(name, &QLineEdit::returnPressed, &dialog, &QDialog::accept);
  if (dialog.exec() == QDialog::Accepted) m_model->renameEntry(row, name->text());
}
QStringList FilePickerDialog::selectedPaths() const { return m_model->selectedPaths(); }
QVector<app::FileBrowserEntry> FilePickerDialog::selectedItems() const { return m_model->selectedItems(); }
bool FilePickerDialog::requestRejection() {
  m_model->cancel();
  return true;
}
}  // namespace uwf::ui
#include "moc_FilePickerDialog.cpp"
