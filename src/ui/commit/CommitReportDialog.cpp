// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "CommitReportDialog.h"

#include <QAction>
#include <QClipboard>
#include <QDialogButtonBox>
#include <QGuiApplication>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QPushButton>
#include <QShortcut>
#include <QVBoxLayout>
#include <algorithm>

#include "presentation/common/PageProxyModel.h"
#include "ui/appearance/I18n.h"
#include "ui/appearance/ThemeManager.h"
#include "ui/dialogs/DialogLayout.h"
#include "ui/dialogs/PanelDialog.h"
#include "ui/widgets/CommandMenu.h"
#include "ui/widgets/ContentHintBox.h"
#include "ui/widgets/IconView.h"
#include "ui/widgets/Notice.h"
#include "ui/widgets/PagedTableView.h"
#include "ui/widgets/PaginationBar.h"
#include "ui/widgets/RecordInspector.h"
#include "ui/widgets/ResultSummary.h"
#include "ui/widgets/Surface.h"
#include "ui/widgets/TablePresentation.h"
#include "ui/widgets/TableText.h"
#include "ui/widgets/TextLabel.h"

namespace uwf::ui {
void showCommitReport(QWidget* parent, const QList<presentation::CommitReportRow>& rows, const app::CommitOperation operation,
                      const presentation::CommitCompletion completion) {
  presentation::CommitReportViewModel model(rows, operation, completion);
  const auto counts = model.counts();
  PanelDialog dialog(parent);
  dialog.setObjectName(QStringLiteral("commitReportDialog"));
  dialog.setWindowTitle(model.outcome().title);
  dialog.setDescription(I18n::tr("Review the result for each target."));
  auto* frame = new DialogLayout(&dialog, DialogContent::Scrollable, QSize(design::panelWidth, design::panelHeight), QSize(720, 500));
  auto* body = frame->body();
  body->setSpacing(16);
  body->addWidget(new ResultSummary(counts.succeeded, counts.failed, counts.skipped, &dialog));
  if (completion.untouched > 0) {
    auto* notice = new Notice(&dialog);
    notice->setTone(Notice::Tone::Warning);
    notice->setText(model.summary());
    body->addWidget(notice);
  } else {
    auto* note = new QWidget(&dialog);
    auto* row = new QHBoxLayout(note);
    row->setContentsMargins(0, 0, 0, 0);
    row->setSpacing(9);
    row->addWidget(new IconView(ThemeManager::instance().icon(QStringLiteral(":/icons/info.svg")), 17, design::Color::Muted, note));
    auto* text =
        new TextLabel(counts.failed || counts.skipped ? I18n::tr("Unsuccessful items have not been saved. Select a row to view the full path and reason.")
                                                      : I18n::tr("All changes have been saved to disk and will persist without a restart."),
                      TextStyle::Caption, note);
    text->setWordWrap(true);
    text->setColorRole(design::Color::Muted);
    row->addWidget(text, 1);
    body->addWidget(note);
  }
  presentation::PageProxyModel pages(&dialog);
  pages.setSourceModel(&model);
  auto* surface = new Surface(design::Color::Surface, &dialog);
  surface->setBorder(Qt::LeftEdge | Qt::TopEdge | Qt::RightEdge | Qt::BottomEdge, 6);
  surface->setClipContent(true);
  auto* tableLayout = new QVBoxLayout(surface);
  tableLayout->setContentsMargins(1, 1, 1, 1);
  auto* table = new PagedTableView(pages, design::reportRowHeight, surface);
  table->setObjectName(QStringLiteral("commitReportTable"));
  auto* header = table->horizontalHeader();
  for (int column = 0; column < model.columnCount(); ++column) header->setSectionResizeMode(column, QHeaderView::Interactive);
  const auto cellWidth = [table](const QString& text) { return table->fontMetrics().horizontalAdvance(text) + 28; };
  table->setColumnWidth(0, std::max({100, cellWidth(I18n::tr("Succeeded")) + 21, cellWidth(I18n::tr("Skipped")) + 21, cellWidth(I18n::tr("Failed")) + 21}));
  header->setSectionResizeMode(1, QHeaderView::Stretch);
  if (model.showExistence()) {
    table->setColumnWidth(2, std::max(80, cellWidth(model.headers()[2])));
    table->setColumnWidth(3, std::max(80, cellWidth(model.headers()[3])));
  }
  table->setColumnWidth(model.columnCount() - 2, 112);
  table->setColumnWidth(model.columnCount() - 1, model.showExistence() ? 180 : 300);
  tableLayout->addWidget(table);
  body->addWidget(surface, 1);
  auto* inspector = new RecordInspector(I18n::tr("Item details"), 2, &dialog);
  inspector->textView()->setObjectName(QStringLiteral("commitReportDetails"));
  auto* copyRow = inspector->addAction(I18n::tr("Copy"), ThemeManager::instance().icon(QStringLiteral(":/icons/copy.svg")));
  copyRow->setObjectName(QStringLiteral("commitReportCopyRowButton"));
  const auto updateSelection = [&] {
    if (!table->currentIndex().isValid() && pages.rowCount() > 0) table->selectRow(0);
    const auto source = pages.mapToSource(table->currentIndex());
    inspector->setText(model.details(source.row()));
    copyRow->setEnabled(source.isValid());
  };
  QObject::connect(table->selectionModel(), &QItemSelectionModel::currentChanged, &dialog, updateSelection);
  QObject::connect(&pages, &presentation::PageProxyModel::pageChanged, &dialog, updateSelection);
  QObject::connect(copyRow, &QPushButton::clicked, &dialog,
                   [&] { QGuiApplication::clipboard()->setText(model.details(pages.mapToSource(table->currentIndex()).row())); });
  body->addWidget(inspector);
  body->setPagedDetails(surface, table, inspector, design::reportRowHeight);
  auto* pagination = new PaginationBar(pages, &dialog, TextStyle::Caption);
  pagination->setSummary(I18n::tr("%1 items").arg(model.rowCount()));
  body->addWidget(pagination);
  updateSelection();
  const auto copyAll = [&] { QGuiApplication::clipboard()->setText(model.allRowsToText()); };
  auto* shortcut = new QShortcut(QKeySequence::Copy, table);
  shortcut->setContext(Qt::WidgetShortcut);
  QObject::connect(shortcut, &QShortcut::activated, table, [table] {
    const auto text = tableSelectionToText(table);
    if (!text.isEmpty()) QGuiApplication::clipboard()->setText(text);
  });
  table->setContextMenuPolicy(Qt::CustomContextMenu);
  QObject::connect(table, &QWidget::customContextMenuRequested, &dialog, [&](const QPoint& point) {
    CommandMenu menu(table);
    auto* selected = menu.addAction(ThemeManager::instance().icon(QStringLiteral(":/icons/copy.svg")), I18n::tr("Copy selected rows"));
    selected->setEnabled(table->selectionModel()->hasSelection());
    auto* all = menu.addAction(ThemeManager::instance().icon(QStringLiteral(":/icons/file.svg")), I18n::tr("Copy all"));
    QObject::connect(selected, &QAction::triggered, &menu, [table] { QGuiApplication::clipboard()->setText(tableSelectionToText(table)); });
    QObject::connect(all, &QAction::triggered, &menu, copyAll);
    menu.exec(table->viewport()->mapToGlobal(point));
  });
  auto* leading = new QDialogButtonBox(&dialog);
  auto* copy = leading->addButton(I18n::tr("Copy all"), QDialogButtonBox::ActionRole);
  copy->setObjectName(QStringLiteral("commitReportCopyAllButton"));
  copy->setEnabled(model.rowCount() > 0);
  copy->setIcon(ThemeManager::instance().icon(QStringLiteral(":/icons/copy.svg")));
  QObject::connect(copy, &QPushButton::clicked, &dialog, copyAll);
  auto* trailing = new QDialogButtonBox(&dialog);
  auto* done = trailing->addButton(I18n::tr("Done"), QDialogButtonBox::RejectRole);
  done->setDefault(true);
  QObject::connect(done, &QPushButton::clicked, &dialog, &QDialog::reject);
  frame->setActions(trailing, leading);
  dialog.exec();
}
}  // namespace uwf::ui
