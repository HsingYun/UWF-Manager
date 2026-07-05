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
#include "TableText.h"

#include <QStringList>
#include <QTableWidget>
#include <QTableWidgetItem>

namespace uwf::ui {

QString tableSelectionToText(const QTableWidget* t) {
  const auto ranges = t->selectedRanges();
  if (ranges.isEmpty()) return {};
  QString txt;
  for (const auto& range : ranges) {
    for (int r = range.topRow(); r <= range.bottomRow(); ++r) {
      QStringList cells;
      for (int c = range.leftColumn(); c <= range.rightColumn(); ++c) {
        const auto* it = t->item(r, c);
        cells << (it ? it->text() : QString());
      }
      txt += cells.join('\t') + '\n';
    }
  }
  return txt;
}

QString tableAllToText(const QTableWidget* t) {
  QString txt;
  QStringList header;
  for (int c = 0; c < t->columnCount(); ++c) {
    // horizontalHeaderItem 对未设过 header 的列返回 nullptr；调用方虽然都
    // 设了表头，这里加保险防止以后扩展时悄悄 crash。
    const auto* it = t->horizontalHeaderItem(c);
    header << (it ? it->text() : QString());
  }
  txt += header.join('\t') + '\n';
  for (int r = 0; r < t->rowCount(); ++r) {
    QStringList cells;
    for (int c = 0; c < t->columnCount(); ++c) {
      const auto* it = t->item(r, c);
      cells << (it ? it->text() : QString());
    }
    txt += cells.join('\t') + '\n';
  }
  return txt;
}

}  // namespace uwf::ui
