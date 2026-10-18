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
#include "TablePresentation.h"

#include <QApplication>
#include <QClipboard>
#include <QDialog>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QPainter>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScreen>
#include <QShortcut>
#include <QStyleOptionViewItem>
#include <QStyledItemDelegate>
#include <QTableWidget>
#include <QVBoxLayout>

#include "I18n.h"
#include "ThemeManager.h"

namespace uwf::ui {
namespace {

class ReportItemDelegate final : public QStyledItemDelegate {
 public:
  using QStyledItemDelegate::QStyledItemDelegate;
  void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
    QStyledItemDelegate::paint(painter, option, index);
    paintTableRowSeparator(painter, option);
  }
};

}  // namespace

QColor tableStatusColor(const TableStatus status) {
  const auto& theme = ThemeManager::instance();
  switch (status) {
    case TableStatus::Error:
      return theme.isLight() ? QColor(0xB3261E) : QColor(0xF28B96);
    case TableStatus::Warning:
      return theme.isLight() ? QColor(0x945400) : QColor(0xE4A465);
    case TableStatus::Success:
      return theme.color(Sem::AddOk);
    case TableStatus::Information:
      return theme.color(Sem::Accent);
    case TableStatus::Muted:
      return theme.color(Sem::FgMuted);
  }
  return theme.color(Sem::Fg);
}

QIcon tableStatusIcon(const TableStatus status) {
  const QString resource = status == TableStatus::Error     ? QStringLiteral(":/icons/status_error.svg")
                           : status == TableStatus::Warning ? QStringLiteral(":/icons/status_warning.svg")
                           : status == TableStatus::Success ? QStringLiteral(":/icons/status_success.svg")
                                                            : QStringLiteral(":/icons/info.svg");
  return ThemeManager::iconWithColor(resource, tableStatusColor(status));
}

void configureReportTable(QTableWidget* table) {
  table->setShowGrid(false);
  table->setProperty("horizontalRowSeparators", true);
  table->horizontalHeader()->setStyleSheet(QStringLiteral("QHeaderView::section { border-right: none; }"));
  table->setIconSize(QSize(14, 14));
  table->setItemDelegate(new ReportItemDelegate(table));
}

void paintTableRowSeparator(QPainter* painter, const QStyleOptionViewItem& option) {
  if (!option.widget || !option.widget->property("horizontalRowSeparators").toBool()) return;
  QColor color = ThemeManager::instance().color(Sem::Border);
  color.setAlphaF(0.65F);
  painter->save();
  // 细分隔线不占用单元格的排版空间，高 DPI 下仍为一个物理像素。
  painter->setPen(QPen(color, 0));
  painter->drawLine(option.rect.bottomLeft(), option.rect.bottomRight());
  painter->restore();
}

void showTableRowDetails(QTableWidget* table, const int row) {
  if (row < 0 || row >= table->rowCount()) return;
  QStringList fields;
  for (int column = 0; column < table->columnCount(); ++column) {
    if (table->isColumnHidden(column)) continue;
    const auto* header = table->horizontalHeaderItem(column);
    const auto* value = table->item(row, column);
    fields.append((header ? header->text() : QString()) + QChar('\n') + (value ? value->text() : QString()));
  }
  const QString content = fields.join(QStringLiteral("\n\n"));
  QDialog dialog(table->window());
  dialog.setObjectName(QStringLiteral("tableRowDetailsDialog"));
  dialog.setWindowTitle(I18n::tr("Operation details"));
  auto* layout = new QVBoxLayout(&dialog);
  layout->setContentsMargins(20, 18, 20, 14);
  layout->setSpacing(12);
  auto* text = new QPlainTextEdit(&dialog);
  text->setObjectName(QStringLiteral("tableRowDetailsText"));
  text->setReadOnly(true);
  text->setPlainText(content);
  text->setLineWrapMode(QPlainTextEdit::WidgetWidth);
  layout->addWidget(text, 1);
  auto* buttons = new QDialogButtonBox(&dialog);
  auto* copy = buttons->addButton(I18n::tr("Copy all"), QDialogButtonBox::ActionRole);
  auto* close = buttons->addButton(I18n::tr("Close"), QDialogButtonBox::RejectRole);
  QObject::connect(copy, &QPushButton::clicked, &dialog, [content] { QApplication::clipboard()->setText(content); });
  QObject::connect(close, &QPushButton::clicked, &dialog, &QDialog::reject);
  close->setDefault(true);
  layout->addWidget(buttons);
  const QSize available = table->screen()->availableGeometry().size() - QSize(80, 80);
  dialog.resize(QSize(720, 480).boundedTo(available));
  dialog.exec();
}

void addTableDetailsButton(QTableWidget* table, QDialogButtonBox* buttons) {
  auto* details = buttons->addButton(I18n::tr("Operation details"), QDialogButtonBox::ActionRole);
  details->setObjectName(QStringLiteral("tableDetailsButton"));
  details->setAutoDefault(false);
  details->setEnabled(false);
  const auto selectedRow = [table] {
    const auto selected = table->selectedItems();
    if (selected.isEmpty()) return -1;
    const auto* current = table->currentItem();
    return current && current->isSelected() ? current->row() : selected.front()->row();
  };
  QObject::connect(table, &QTableWidget::itemSelectionChanged, details, [details, selectedRow] { details->setEnabled(selectedRow() >= 0); });
  QObject::connect(details, &QPushButton::clicked, table, [table, selectedRow] { showTableRowDetails(table, selectedRow()); });
  QObject::connect(table, &QTableWidget::cellDoubleClicked, table, [table](const int row, int) { showTableRowDetails(table, row); });
  auto* shortcut = new QShortcut(QKeySequence(Qt::Key_Return), table);
  shortcut->setKeys({QKeySequence(Qt::Key_Return), QKeySequence(Qt::Key_Enter)});
  shortcut->setContext(Qt::WidgetShortcut);
  QObject::connect(shortcut, &QShortcut::activated, table, [table, selectedRow] { showTableRowDetails(table, selectedRow()); });
}

}  // namespace uwf::ui
