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
#pragma once

#include <QColor>
#include <QIcon>

class QPainter;
class QDialogButtonBox;
class QStyleOptionViewItem;
class QTableWidget;

namespace uwf::ui {

enum class TableStatus { Muted, Information, Warning, Error, Success };

QColor tableStatusColor(TableStatus status);
QIcon tableStatusIcon(TableStatus status);
void configureReportTable(QTableWidget* table);
void paintTableRowSeparator(QPainter* painter, const QStyleOptionViewItem& option);
void showTableRowDetails(QTableWidget* table, int row);
void addTableDetailsButton(QTableWidget* table, QDialogButtonBox* buttons);

}  // namespace uwf::ui
