// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "LogViewerDialog.h"

#include <QAction>
#include <QClipboard>
#include <QDialogButtonBox>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QPushButton>
#include <QShortcut>
#include <QStackedWidget>
#include <QTableView>
#include <QVBoxLayout>
#include <algorithm>
#include <array>
#include <utility>

#include "presentation/common/ContentLoadState.h"
#include "presentation/common/ItemRoles.h"
#include "presentation/common/PageProxyModel.h"
#include "ui/appearance/I18n.h"
#include "ui/appearance/ThemeManager.h"
#include "ui/design/DesignSystem.h"
#include "ui/design/TextPainting.h"
#include "ui/dialogs/DialogAppearance.h"
#include "ui/dialogs/DialogLayout.h"
#include "ui/dialogs/Dialogs.h"
#include "ui/widgets/CommandMenu.h"
#include "ui/widgets/ContentHintBox.h"
#include "ui/widgets/ContentState.h"
#include "ui/widgets/Disclosure.h"
#include "ui/widgets/FlexibleColumnLayout.h"
#include "ui/widgets/PagedTableView.h"
#include "ui/widgets/PaginationBar.h"
#include "ui/widgets/RecordInspector.h"
#include "ui/widgets/SearchField.h"
#include "ui/widgets/SelectBox.h"
#include "ui/widgets/Surface.h"
#include "ui/widgets/TablePresentation.h"
#include "ui/widgets/TableText.h"
#include "ui/widgets/TextLabel.h"

namespace uwf::ui {
namespace {
ContentHintBox* detailText(QWidget* parent, const int pixels, const qreal lineHeight) {
  auto* text = new ContentHintBox(parent);
  auto type = text->font();
  type.setFamilies(DesignSystem::monospaceFamilies());
  type.setPixelSize(pixels);
  text->setFont(type);
  text->setLineHeight(lineHeight);
  text->setStyleSheet(QStringLiteral("QTextBrowser { background: transparent; border: 0; padding: 0; }"));
  text->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
  return text;
}
}  // namespace
LogViewerDialog::LogViewerDialog(std::shared_ptr<presentation::LogViewModel> model, QWidget* parent)
    : PanelDialog(parent),
      m_viewModel(std::move(model)),
      m_pages(new presentation::PageProxyModel(this)),
      m_content(new QStackedWidget(this)),
      m_records(new QWidget(m_content)),
      m_table(nullptr),
      m_tableHost(new Surface(design::Color::Surface, m_records)),
      m_inspector(new RecordInspector(I18n::tr("Message details"), 3, m_records)),
      m_state(new ContentState(m_content, ContentState::ScrollMode::HostManaged)),
      m_empty(new ContentState(m_records, ContentState::ScrollMode::HostManaged)),
      m_errorDetails(detailText(m_state, 12, 12 * 1.95)),
      m_details(m_inspector->textView()),
      m_errorDisclosure(nullptr),
      m_level(new SelectBox(m_records)),
      m_search(new SearchField(m_records)),
      m_pagination(new PaginationBar(*m_pages, m_records, TextStyle::Caption)),
      m_copy(nullptr),
      m_refresh(nullptr),
      m_copyAll(nullptr),
      m_clear(nullptr) {
  setWindowTitle(I18n::tr("Log"));
  setDescription(I18n::tr("View application records and diagnostic information."));
  auto* frame = new DialogLayout(this, DialogContent::Scrollable, QSize(1096, 754), QSize(720, 500));
  frame->body()->addWidget(m_content, 1);
  m_content->addWidget(m_records);
  m_content->addWidget(m_state);
  auto* records = new FlexibleColumnLayout(m_records);
  records->setContentsMargins(0, 0, 0, 0);
  records->setSpacing(15);
  auto* toolbar = new QHBoxLayout();
  toolbar->setSpacing(10);
  m_level->setObjectName(QStringLiteral("logLevelFilter"));
  m_level->setProperty("accessibleName", I18n::tr("Log level"));
  const std::array levels{
      std::pair{I18n::tr("All levels"), presentation::LogLevelFilter::All}, std::pair{I18n::tr("Error"), presentation::LogLevelFilter::Error},
      std::pair{I18n::tr("Warning"), presentation::LogLevelFilter::Warning}, std::pair{I18n::tr("Information"), presentation::LogLevelFilter::Information},
      std::pair{I18n::tr("Debug"), presentation::LogLevelFilter::Debug}};
  for (const auto& [label, level] : levels) m_level->addItem(label, static_cast<int>(level));
  m_level->setMinimumWidth(112);
  m_level->setFixedHeight(34);
  m_level->setCurrentIndex(m_level->findData(static_cast<int>(m_viewModel->levelFilter())));
  toolbar->addWidget(m_level);
  auto* session = new TextLabel(I18n::tr("Current run"), TextStyle::Caption, m_records);
  session->setColorRole(design::Color::Faint);
  toolbar->addWidget(session);
  toolbar->addStretch();
  m_search->setObjectName(QStringLiteral("logSearch"));
  m_search->setPlaceholderText(I18n::tr("Search messages or modules"));
  m_search->setProperty("accessibleName", I18n::tr("Search logs"));
  m_search->setFixedSize(235, 34);
  m_search->setText(m_viewModel->query());
  toolbar->addWidget(m_search);
  records->addLayout(toolbar);

  m_pages->setSourceModel(m_viewModel->filteredModel());
  m_table = new PagedTableView(*m_pages, design::logRowHeight, m_tableHost);
  m_table->setObjectName(QStringLiteral("logTable"));
  auto* header = m_table->horizontalHeader();
  for (int column = 0; column < 3; ++column) header->setSectionResizeMode(column, QHeaderView::Interactive);
  header->setSectionResizeMode(3, QHeaderView::Stretch);
  m_table->setColumnWidth(0, 112);
  int levelWidth = 82;
  for (const auto& [label, level] : levels)
    if (level != presentation::LogLevelFilter::All) levelWidth = std::max(levelWidth, qCeil(textLineSize(label, m_table->font()).width()) + 49);
  m_table->setColumnWidth(1, levelWidth);
  m_table->setColumnWidth(2, 92);
  m_tableHost->setBorder(Qt::LeftEdge | Qt::TopEdge | Qt::RightEdge | Qt::BottomEdge, 6);
  m_tableHost->setClipContent(true);
  auto* tableLayout = new QVBoxLayout(m_tableHost);
  tableLayout->setContentsMargins(1, 1, 1, 1);
  tableLayout->addWidget(m_table);
  records->addWidget(m_tableHost, 1);
  records->addWidget(m_empty, 1);

  m_copy = m_inspector->addAction(I18n::tr("Copy"), ThemeManager::instance().icon(QStringLiteral(":/icons/copy.svg")));
  m_copy->setObjectName(QStringLiteral("copyLogEntry"));
  m_details->setObjectName(QStringLiteral("logMessageDetails"));
  records->addWidget(m_inspector);
  records->addWidget(m_pagination);

  auto* failure = new Disclosure(I18n::tr("Error details"), m_state);
  auto* errorSurface = new Surface(design::Color::Surface, failure);
  errorSurface->setBorder(Qt::LeftEdge | Qt::TopEdge | Qt::RightEdge | Qt::BottomEdge, 5);
  auto* errorLayout = new QVBoxLayout(errorSurface);
  errorLayout->setContentsMargins(19, 15, 19, 15);
  errorLayout->addWidget(m_errorDetails);
  failure->setContent(errorSurface);
  m_errorDisclosure = failure;
  m_state->setDetails(failure);

  auto* leading = new QDialogButtonBox(this);
  m_refresh = leading->addButton(I18n::tr("Refresh"), QDialogButtonBox::ActionRole);
  m_refresh->setIcon(ThemeManager::instance().icon(QStringLiteral(":/icons/refresh.svg")));
  m_copyAll = leading->addButton(I18n::tr("Copy all"), QDialogButtonBox::ActionRole);
  m_copyAll->setIcon(ThemeManager::instance().icon(QStringLiteral(":/icons/copy.svg")));
  m_clear = leading->addButton(I18n::tr("Clear…"), QDialogButtonBox::ActionRole);
  m_clear->setIcon(ThemeManager::instance().icon(QStringLiteral(":/icons/trash.svg")));
  setDialogAction(m_clear, DialogAction::DestructiveSecondary);
  m_clear->setProperty("designRole", "dangerQuiet");
  auto* trailing = new QDialogButtonBox(this);
  trailing->addButton(I18n::tr("Close"), QDialogButtonBox::RejectRole);
  frame->setActions(trailing, leading);

  connect(m_level, &QComboBox::currentIndexChanged, this, [this] {
    m_viewModel->setLevelFilter(static_cast<presentation::LogLevelFilter>(m_level->currentData().toInt()));
    m_pages->goFirst();
    render();
  });
  connect(m_search, &QLineEdit::textChanged, this, [this](const QString& text) {
    m_viewModel->setQuery(text);
    m_pages->goFirst();
    render();
  });
  connect(m_pages, &presentation::PageProxyModel::pageChanged, this, &LogViewerDialog::updatePagination);
  connect(m_table->selectionModel(), &QItemSelectionModel::currentChanged, this, [this] { updateSelection(); });
  connect(m_copy, &QPushButton::clicked, this, [this] {
    const auto index = m_table->currentIndex();
    if (!index.isValid()) return;
    QStringList cells;
    for (int column = 0; column < m_table->model()->columnCount(); ++column)
      cells.append(index.siblingAtColumn(column).data(presentation::FullTextRole).toString());
    QGuiApplication::clipboard()->setText(cells.join(u'\t'));
  });
  connect(m_refresh, &QPushButton::clicked, m_viewModel.get(), &presentation::LogViewModel::reload);
  connect(m_state, &ContentState::actionRequested, this, [this] {
    if (m_viewModel->loading())
      m_viewModel->cancelLoading();
    else
      m_viewModel->reload();
  });
  connect(m_empty, &ContentState::actionRequested, this, [this] {
    m_level->setCurrentIndex(0);
    m_search->clear();
  });
  connect(m_copyAll, &QPushButton::clicked, this, [this] { QGuiApplication::clipboard()->setText(m_viewModel->allRowsToText()); });
  connect(m_clear, &QPushButton::clicked, this, [this] {
    if (dialogs::confirm(this, {I18n::tr("Clear logs?"), I18n::tr("The current log records will be cleared and cannot be restored."), I18n::tr("Clear logs")}))
      m_viewModel->clear();
  });
  connect(trailing, &QDialogButtonBox::rejected, this, &QDialog::reject);
  connect(this, &QDialog::finished, m_viewModel.get(), &presentation::LogViewModel::cancelLoading);
  connect(m_viewModel.get(), &presentation::LogViewModel::loadingStarted, this, &LogViewerDialog::render);
  connect(m_viewModel.get(), &presentation::LogViewModel::loadFailed, this, [this] { render(); });
  connect(m_viewModel.get(), &presentation::LogViewModel::loadingCanceled, this, &LogViewerDialog::render);
  connect(m_viewModel.get(), &presentation::LogViewModel::loadingFinished, this, [this] {
    m_pages->goLast();
    render();
  });
  auto* copyShortcut = new QShortcut(QKeySequence::Copy, m_table);
  copyShortcut->setContext(Qt::WidgetShortcut);
  connect(copyShortcut, &QShortcut::activated, m_table, [this] {
    const auto text = tableSelectionToText(m_table);
    if (!text.isEmpty()) QGuiApplication::clipboard()->setText(text);
  });
  m_table->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(m_table, &QWidget::customContextMenuRequested, this, [this](const QPoint& position) {
    CommandMenu menu(m_table);
    auto& theme = ThemeManager::instance();
    auto* selected = menu.addAction(theme.icon(":/icons/copy.svg"), I18n::tr("Copy selected rows"));
    selected->setEnabled(m_table->selectionModel()->hasSelection());
    auto* page = menu.addAction(theme.icon(":/icons/file.svg"), I18n::tr("Copy current page"));
    menu.addAction(theme.icon(":/icons/log.svg"), I18n::tr("Copy all logs"), &menu,
                   [this] { QGuiApplication::clipboard()->setText(m_viewModel->allRowsToText()); });
    connect(selected, &QAction::triggered, &menu, [this] { QGuiApplication::clipboard()->setText(tableSelectionToText(m_table)); });
    connect(page, &QAction::triggered, &menu, [this] {
      QGuiApplication::clipboard()->setText(tableRowsToText(m_table, m_pages->pager().pageStart(), m_pages->pager().pageEnd(m_pages->totalRows())));
    });
    menu.exec(m_table->viewport()->mapToGlobal(position));
  });
  m_viewModel->reload();
}
void LogViewerDialog::render() {
  using presentation::ContentLoadState;
  const auto state = m_viewModel->loadState();
  const bool loading = state == ContentLoadState::Loading;
  const bool failed = state == ContentLoadState::Failed;
  m_content->setCurrentWidget(loading || failed ? static_cast<QWidget*>(m_state) : m_records);
  m_refresh->setEnabled(!loading);
  const bool hasRows = !loading && !failed && m_pages->totalRows() > 0;
  m_copyAll->setEnabled(hasRows);
  m_clear->setEnabled(hasRows);
  m_state->setLoading(loading);
  m_state->setText(loading ? I18n::tr("Reading logs") : I18n::tr("Unable to read logs"),
                   loading ? I18n::tr("Records from the current run will appear here when reading is complete.")
                           : I18n::tr("Log reading failed. Retry or check access to the log file."));
  m_state->setIcon(ThemeManager::instance().icon(QStringLiteral(":/icons/status_error.svg")), design::Color::Danger);
  m_state->setAction(loading  ? I18n::tr("Cancel reading")
                     : failed ? I18n::tr("Read again")
                              : QString{},
                     failed ? ThemeManager::instance().icon(QStringLiteral(":/icons/refresh.svg")) : QIcon{});
  m_errorDisclosure->setVisible(failed);
  m_errorDetails->setPlainText(m_viewModel->errorText());
  m_tableHost->setVisible(hasRows);
  m_inspector->setVisible(hasRows);
  m_empty->setVisible(!hasRows);
  const bool filtered = !m_viewModel->query().isEmpty() || m_viewModel->levelFilter() != presentation::LogLevelFilter::All;
  m_empty->setText(filtered ? I18n::tr("No matching logs") : I18n::tr("No logs yet"),
                   filtered ? I18n::tr("Try another keyword or log level.") : I18n::tr("New records will appear after refreshing."));
  m_empty->setIcon(ThemeManager::instance().icon(QStringLiteral(":/icons/log.svg")));
  m_empty->setAction(filtered ? I18n::tr("Clear filters") : QString{});
  updatePagination();
  updateSelection();
}
void LogViewerDialog::updatePagination() {
  QString summary = I18n::tr("%1 records").arg(m_pages->totalRows());
  if (!m_viewModel->query().isEmpty() || m_viewModel->levelFilter() != presentation::LogLevelFilter::All)
    summary = I18n::tr("%1 records · %2 total").arg(m_pages->totalRows()).arg(m_viewModel->rowCount());
  m_pagination->setSummary(summary);
}
void LogViewerDialog::updateSelection() {
  if (!m_table->currentIndex().isValid() && m_pages->rowCount() > 0) m_table->selectRow(0);
  const auto index = m_table->currentIndex();
  m_copy->setEnabled(index.isValid());
  if (!index.isValid()) {
    m_inspector->setMetadata({});
    m_details->clear();
    return;
  }
  m_inspector->setMetadata(QStringLiteral("%1 · %2").arg(index.siblingAtColumn(0).data(presentation::FullTextRole).toString(),
                                                         index.siblingAtColumn(2).data(presentation::FullTextRole).toString()));
  m_details->setPlainText(index.siblingAtColumn(3).data(presentation::FullTextRole).toString());
}
}  // namespace uwf::ui
#include "moc_LogViewerDialog.cpp"
