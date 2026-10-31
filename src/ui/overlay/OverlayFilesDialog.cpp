// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "OverlayFilesDialog.h"

#include <QClipboard>
#include <QDateTime>
#include <QDialogButtonBox>
#include <QDir>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <utility>

#include "app/overlay/OverlayFileCatalog.h"
#include "presentation/common/ItemRoles.h"
#include "presentation/common/PageProxyModel.h"
#include "shared/ByteFormat.h"
#include "ui/appearance/I18n.h"
#include "ui/appearance/ThemeManager.h"
#include "ui/design/DesignSystem.h"
#include "ui/dialogs/DialogLayout.h"
#include "ui/dialogs/Dialogs.h"
#include "ui/dialogs/InformationDialog.h"
#include "ui/widgets/CommandMenu.h"
#include "ui/widgets/ContentHintBox.h"
#include "ui/widgets/ContentState.h"
#include "ui/widgets/Disclosure.h"
#include "ui/widgets/FlexibleColumnLayout.h"
#include "ui/widgets/IconView.h"
#include "ui/widgets/PagedTableView.h"
#include "ui/widgets/PaginationBar.h"
#include "ui/widgets/RecordInspector.h"
#include "ui/widgets/StatusBadge.h"
#include "ui/widgets/Surface.h"
#include "ui/widgets/TextLabel.h"
#include "windows/system/WindowsShell.h"

namespace uwf::ui {
namespace {
QPushButton* linkButton(const QString& text, const QString& resource, QWidget* parent) {
  auto* button = new QPushButton(resource.isEmpty() ? QIcon{} : ThemeManager::instance().icon(resource), text, parent);
  button->setProperty("designRole", "link");
  button->setProperty("designPaddingX", 0);
  button->setFixedHeight(20);
  button->setIconSize({14, 14});
  auto type = button->font();
  type.setPixelSize(12);
  button->setFont(type);
  return button;
}
}  // namespace
OverlayFilesDialog::OverlayFilesDialog(std::shared_ptr<presentation::OverlayFilesViewModel> model, dialogs::FileDialogProvider& fileDialogs, QWidget* parent)
    : PanelDialog(parent),
      m_fileDialogs(fileDialogs),
      m_viewModel(std::move(model)),
      m_pages(new presentation::PageProxyModel(this)),
      m_content(new QStackedWidget(this)),
      m_records(new QWidget(m_content)),
      m_state(new ContentState(m_content, ContentState::ScrollMode::HostManaged)) {
  setWindowTitle(I18n::tr("View overlay"));
  setDescription(I18n::tr("View the file diagnostic snapshot for %1.").arg(m_viewModel->driveLetter()));
  auto* frame = new DialogLayout(this, DialogContent::Scrollable, QSize(1096, 754), QSize(720, 500));
  frame->body()->addWidget(m_content, 1);
  m_content->addWidget(m_records);
  m_content->addWidget(m_state);
  auto* body = new FlexibleColumnLayout(m_records);
  body->setContentsMargins(0, 0, 0, 0);
  body->setSpacing(15);
  auto* toolbar = new QHBoxLayout();
  toolbar->setSpacing(8);
  toolbar->addWidget(new IconView(ThemeManager::instance().icon(QStringLiteral(":/icons/disk.svg")), 17, design::Color::Muted, m_records));
  auto* disk = new TextLabel({}, TextStyle::Body, m_records);
  I18n::bind(disk, [this, disk] { disk->setText(I18n::tr("Local disk (%1)").arg(m_viewModel->driveLetter())); });
  toolbar->addWidget(disk);
  toolbar->addStretch();
  auto* badge = new StatusBadge(m_records);
  badge->setText(I18n::tr("Diagnostic snapshot"));
  I18n::bind(badge, [badge] { badge->setText(I18n::tr("Diagnostic snapshot")); });
  toolbar->addWidget(badge);
  body->addLayout(toolbar);
  auto* note = new QHBoxLayout();
  note->setSpacing(9);
  note->addWidget(new IconView(ThemeManager::instance().icon(QStringLiteral(":/icons/info.svg")), 17, design::Color::Muted, m_records), 0, Qt::AlignTop);
  auto* warning =
      new TextLabel(I18n::tr("The list may omit files or include files already committed. Do not use it as a commit list."), TextStyle::Caption, m_records);
  warning->setWordWrap(true);
  I18n::bind(warning, [warning] { warning->setText(I18n::tr("The list may omit files or include files already committed. Do not use it as a commit list.")); });
  warning->setColorRole(design::Color::Muted);
  warning->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
  note->addWidget(warning);
  auto* scope = linkButton(I18n::tr("About this snapshot"), {}, m_records);
  I18n::bind(scope, [scope] { scope->setText(I18n::tr("About this snapshot")); });
  note->addWidget(scope, 0, Qt::AlignTop);
  note->addStretch();
  connect(scope, &QPushButton::clicked, this, &OverlayFilesDialog::showSnapshotScope);
  body->addLayout(note);
  m_pages->setSourceModel(m_viewModel.get());
  auto* tableHost = new Surface(design::Color::Surface, m_records);
  tableHost->setBorder(Qt::LeftEdge | Qt::TopEdge | Qt::RightEdge | Qt::BottomEdge, 6);
  tableHost->setClipContent(true);
  auto* tableLayout = new QVBoxLayout(tableHost);
  tableLayout->setContentsMargins(1, 1, 1, 1);
  m_table = new PagedTableView(*m_pages, design::overlayRowHeight, tableHost);
  m_table->setObjectName(QStringLiteral("overlayFilesTable"));
  m_table->setIconSize({17, 17});
  m_table->setSelectionMode(QAbstractItemView::SingleSelection);
  m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
  m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);
  m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Interactive);
  m_table->setColumnWidth(1, 98);
  m_table->setColumnWidth(2, 92);
  tableLayout->addWidget(m_table);
  body->addWidget(tableHost, 1);
  m_inspector = new RecordInspector({}, 2, m_records);
  auto* reveal = m_inspector->addAction(I18n::tr("Open containing folder"), ThemeManager::instance().icon(QStringLiteral(":/icons/folder.svg")));
  I18n::bind(reveal, [reveal] { reveal->setText(I18n::tr("Open containing folder")); });
  m_commit = m_inspector->addAction(I18n::tr("Commit changes…"), ThemeManager::instance().icon(QStringLiteral(":/icons/apply.svg")));
  m_selectedPath = m_inspector->textView();
  m_selectedPath->setObjectName(QStringLiteral("overlaySelectedPath"));
  body->addWidget(m_inspector);
  body->setPagedDetails(tableHost, m_table, m_inspector, design::overlayRowHeight);
  m_pagination = new PaginationBar(*m_pages, m_records, TextStyle::Caption);
  body->addWidget(m_pagination);

  auto* details = new Disclosure(I18n::tr("Error details and suggestions"), m_state);
  details->setObjectName(QStringLiteral("overlayErrorDisclosure"));
  auto* errorContent = new QWidget(details);
  auto* errorLayout = new FlexibleColumnLayout(errorContent);
  errorLayout->setContentsMargins(0, 12, 0, 0);
  errorLayout->setSpacing(10);
  m_errorHint = new TextLabel({}, TextStyle::Caption, errorContent);
  m_errorHint->setObjectName(QStringLiteral("overlayErrorHint"));
  m_errorHint->setWordWrap(true);
  m_errorHint->setColorRole(design::Color::Muted);
  m_errorHint->setLineHeight(1.7);
  errorLayout->addWidget(m_errorHint);
  m_errorText = new ContentHintBox(errorContent);
  m_errorText->setObjectName(QStringLiteral("overlayErrorDetails"));
  m_errorText->setStyleSheet(QStringLiteral("QTextBrowser { background: transparent; border: 0; padding: 0; }"));
  m_errorText->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
  auto errorFont = m_errorText->font();
  errorFont.setPixelSize(12);
  m_errorText->setFont(errorFont);
  m_errorText->setLineHeight(20.4);
  errorLayout->addWidget(m_errorText);
  details->setContent(errorContent);
  m_state->setDetails(details);
  m_errorDisclosure = details;
  auto* leading = new QDialogButtonBox(this);
  m_exportBtn = leading->addButton(I18n::tr("Export list"), QDialogButtonBox::ActionRole);
  m_exportBtn->setIcon(ThemeManager::instance().icon(QStringLiteral(":/icons/export.svg")));
  auto* trailing = new QDialogButtonBox(this);
  auto* close = trailing->addButton(I18n::tr("Close"), QDialogButtonBox::RejectRole);
  close->setDefault(true);
  frame->setActions(trailing, leading);
  connect(trailing, &QDialogButtonBox::rejected, this, &QDialog::reject);
  connect(this, &QDialog::finished, m_viewModel.get(), &presentation::OverlayFilesViewModel::cancelLoading);
  connect(m_exportBtn, &QPushButton::clicked, this, &OverlayFilesDialog::onExportClicked);
  connect(reveal, &QPushButton::clicked, this, [this] { openContainingFolder(m_table->currentIndex().data(presentation::PathRole).toString()); });
  connect(m_commit, &QPushButton::clicked, this, [this] {
    const auto index = m_table->currentIndex();
    if (index.isValid() && !index.data(presentation::SystemMetadataRole).toBool()) emit commitFileRequested(index.data(presentation::PathRole).toString());
  });
  connect(m_table->selectionModel(), &QItemSelectionModel::currentChanged, this, [this] { renderSelection(); });
  m_table->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(m_table, &QWidget::customContextMenuRequested, this, &OverlayFilesDialog::onContextMenu);
  connect(m_viewModel.get(), &presentation::OverlayFilesViewModel::loadingStarted, this, &OverlayFilesDialog::renderPage);
  connect(m_viewModel.get(), &presentation::OverlayFilesViewModel::loadingStopped, this, &OverlayFilesDialog::renderPage);
  connect(m_viewModel.get(), &presentation::OverlayFilesViewModel::entriesLoaded, this, [this] {
    if (!m_table->currentIndex().isValid() && m_pages->rowCount() > 0) m_table->selectRow(0);
    renderPage();
  });
  connect(m_viewModel.get(), &presentation::OverlayFilesViewModel::loadFailed, this, [this] { renderPage(); });
  connect(m_state, &ContentState::actionRequested, this, [this] {
    if (m_viewModel->loading())
      m_viewModel->cancelLoading();
    else
      m_viewModel->startLoading();
  });
  m_viewModel->startLoading();
  I18n::bind(this, [this, close] {
    setWindowTitle(I18n::tr("View overlay"));
    setDescription(I18n::tr("View the file diagnostic snapshot for %1.").arg(m_viewModel->driveLetter()));
    m_commit->setText(I18n::tr("Commit changes…"));
    m_exportBtn->setText(I18n::tr("Export list"));
    close->setText(I18n::tr("Close"));
    m_errorDisclosure->toggleButton()->setText(I18n::tr("Error details and suggestions"));
    m_viewModel->retranslate();
    renderPage();
  });
  connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [this, reveal] {
    m_exportBtn->setIcon(ThemeManager::instance().icon(QStringLiteral(":/icons/export.svg")));
    m_commit->setIcon(ThemeManager::instance().icon(QStringLiteral(":/icons/apply.svg")));
    reveal->setIcon(ThemeManager::instance().icon(QStringLiteral(":/icons/folder.svg")));
  });
}
OverlayFilesDialog::~OverlayFilesDialog() = default;

void OverlayFilesDialog::showSnapshotScope() {
  InformationDialog dialog(this);
  dialog.setObjectName(QStringLiteral("overlaySnapshotScopeDialog"));
  dialog.setWindowTitle(I18n::tr("Overlay snapshot scope"));
  dialog.setHeaderStyle(DialogWindow::HeaderStyle::Message);
  auto* frame = new DialogLayout(&dialog, DialogContent::Scrollable, QSize(design::messageWidth, 0));
  auto* body = frame->body();
  body->setSpacing(16);
  auto* introduction =
      new TextLabel(I18n::tr("This list is for diagnostics only. It cannot determine which files must be committed."), TextStyle::Description, &dialog);
  introduction->setWordWrap(true);
  introduction->setColorRole(design::Color::Muted);
  body->addWidget(introduction);
  const auto addNote = [&](const QString& title, const QString& explanation) {
    auto* group = new QWidget(&dialog);
    auto* layout = new QVBoxLayout(group);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);
    auto* heading = new TextLabel(title, TextStyle::Section, group);
    heading->setWordWrap(true);
    layout->addWidget(heading);
    auto* detail = new TextLabel(explanation, TextStyle::Caption, group);
    detail->setWordWrap(true);
    detail->setColorRole(design::Color::Muted);
    layout->addWidget(detail);
    body->addWidget(group);
  };
  addNote(I18n::tr("NTFS volumes only"), I18n::tr("Large overlays may load slowly or fail because of insufficient resources."));
  addNote(I18n::tr("Small files may be missing"), I18n::tr("Files smaller than the disk cluster size (typically 4 KB) may not appear in the list."));
  addNote(I18n::tr("Does not indicate pending commits"),
          I18n::tr("Files already committed, excluded paths, and items affected by other operations may also appear."));
  body->addSpacing(16);
  auto* buttons = new QDialogButtonBox(&dialog);
  auto* close = buttons->addButton(I18n::tr("Close"), QDialogButtonBox::RejectRole);
  close->setDefault(true);
  close->setFocus();
  connect(close, &QPushButton::clicked, &dialog, &QDialog::reject);
  frame->setActions(buttons);
  dialog.exec();
}

void OverlayFilesDialog::onContextMenu(const QPoint& pos) {
  const auto index = m_table->indexAt(pos);
  if (!index.isValid()) return;
  m_table->selectRow(index.row());
  const auto path = index.data(presentation::PathRole).toString();
  CommandMenu menu(this);
  auto* reveal = menu.addAction(ThemeManager::instance().icon(QStringLiteral(":/icons/folder.svg")), I18n::tr("Open containing folder"));
  connect(reveal, &QAction::triggered, this, [path] { openContainingFolder(path); });
  menu.addAction(ThemeManager::instance().icon(QStringLiteral(":/icons/copy.svg")), I18n::tr("Copy file path"), &menu,
                 [path] { QGuiApplication::clipboard()->setText(path); });
  if (!index.data(presentation::SystemMetadataRole).toBool()) {
    menu.addSeparator();
    auto* commit = menu.addAction(ThemeManager::instance().icon(QStringLiteral(":/icons/apply.svg")), index.data(presentation::DirectoryRole).toBool()
                                                                                                          ? I18n::tr("Commit folder changes to disk…")
                                                                                                          : I18n::tr("Commit file changes to disk…"));
    connect(commit, &QAction::triggered, this, [this, path] { emit commitFileRequested(path); });
  }
  menu.exec(m_table->viewport()->mapToGlobal(pos));
}
void OverlayFilesDialog::openContainingFolder(const QString& path) {
  if (!path.isEmpty()) windows::shell::revealInExplorer(path);
}
void OverlayFilesDialog::onExportClicked() {
  if (m_viewModel->entries().isEmpty()) return;

  const QString letter = m_viewModel->driveLetter().left(1);
  const QString stamp = QDateTime::currentDateTime().toString("yyyyMMdd-HHmmss");
  const QString suggested = QString("overlay-files-%1-%2.txt").arg(letter, stamp);

  const QString path =
      m_fileDialogs.saveFile(this, {I18n::tr("Export overlay file list"), QDir::home().filePath(suggested), I18n::tr("Text files (*.txt);;All files (*)")});
  if (path.isEmpty()) return;

  const auto error = m_viewModel->exportFile(path);
  if (!error.isEmpty()) {
    dialogs::warning(this, I18n::tr("Export failed"), error);
    return;
  }

  dialogs::information(this, I18n::tr("Export finished"),
                       I18n::tr("Saved %1 entries to:\n%2").arg(m_viewModel->entries().size()).arg(QDir::toNativeSeparators(path)));
}

void OverlayFilesDialog::renderPage() {
  using presentation::ContentLoadState;
  const auto state = m_viewModel->loadState();
  const bool loading = state == ContentLoadState::Loading;
  const bool failed = state == ContentLoadState::Failed;
  const bool ready = state == ContentLoadState::Ready && !m_viewModel->entries().isEmpty();
  m_content->setCurrentWidget(ready ? m_records : static_cast<QWidget*>(m_state));
  m_exportBtn->setEnabled(m_viewModel->canExport());
  m_state->setLoading(loading);
  m_state->setText(loading                               ? I18n::tr("Reading overlay")
                   : failed                              ? I18n::tr("Unable to read the overlay right now")
                   : state == ContentLoadState::Canceled ? I18n::tr("Reading canceled")
                                                         : I18n::tr("No overlay files found"),
                   loading  ? I18n::tr("Reading a large overlay can take longer. You can cancel at any time.")
                   : failed ? I18n::tr("The UWF provider could not complete enumeration. This can occur with a large overlay or insufficient system resources.")
                   : state == ContentLoadState::Canceled ? I18n::tr("Read again to retrieve a diagnostic snapshot.")
                                                         : I18n::tr("An empty diagnostic snapshot does not mean that the overlay contains no changes."));
  m_state->setIcon(ThemeManager::instance().icon(failed ? QStringLiteral(":/icons/status_error.svg") : QStringLiteral(":/icons/disk.svg")),
                   failed ? design::Color::Danger : design::Color::Faint);
  m_state->setAction(loading ? I18n::tr("Cancel reading") : I18n::tr("Read again"),
                     loading ? QIcon{} : ThemeManager::instance().icon(QStringLiteral(":/icons/refresh.svg")));
  m_errorDisclosure->setVisible(failed);
  m_errorHint->setText(m_viewModel->errorHint());
  m_errorHint->setVisible(!m_viewModel->errorHint().isEmpty());
  if (m_errorText->toPlainText() != m_viewModel->errorDetail()) m_errorText->setPlainText(m_viewModel->errorDetail());
  m_pagination->setSummary(
      I18n::tr("%1 items · %2").arg(m_pages->totalRows()).arg(QString::fromStdString(formatBytes(app::overlayFileCatalogSize(m_viewModel->entries())))));
  renderSelection();
}
void OverlayFilesDialog::renderSelection() {
  const auto index = m_table->currentIndex();
  const bool metadata = index.data(presentation::SystemMetadataRole).toBool();
  m_inspector->setTitle(index.data(presentation::DirectoryRole).toBool() ? I18n::tr("Selected folder") : I18n::tr("Selected file"));
  const auto path = index.data(presentation::PathRole).toString();
  if (m_selectedPath->toPlainText() != path) m_selectedPath->setPlainText(path);
  m_commit->setVisible(index.isValid() && !metadata);
  m_inspector->setNote(index.isValid() && metadata ? I18n::tr("System metadata is for diagnosis only and cannot be committed.") : QString{});
}
}  // namespace uwf::ui
#include "moc_OverlayFilesDialog.cpp"
