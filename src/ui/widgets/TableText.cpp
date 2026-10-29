// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "TableText.h"

#include <QItemSelectionModel>
#include <QStringList>
#include <QTableView>
#include <algorithm>
#include "presentation/common/ItemRoles.h"

namespace uwf::ui {
namespace {
QString cellText(const QModelIndex& index) {
  const auto full = index.data(presentation::FullTextRole);
  return (full.isValid() ? full : index.data()).toString();
}
}  // namespace

QString tableSelectionToText(const QTableView* t) {
  const auto ranges = t->selectionModel()->selection();
  if (ranges.isEmpty()) return {};
  QString txt;
  for (const auto& range : ranges) {
    for (int r = range.top(); r <= range.bottom(); ++r) {
      QStringList cells;
      for (int c = range.left(); c <= range.right(); ++c) {
        cells << cellText(t->model()->index(r, c));
      }
      txt += cells.join('\t') + '\n';
    }
  }
  return txt;
}

QString tableRowsToText(const QTableView* t, int firstRow, int endRow) {
  QString txt;
  QStringList header;
  for (int c = 0; c < t->model()->columnCount(); ++c) {
    header << t->model()->headerData(c, Qt::Horizontal).toString();
  }
  txt += header.join('\t') + '\n';
  for (int r = std::max(0, firstRow); r < std::min(endRow, t->model()->rowCount()); ++r) {
    QStringList cells;
    for (int c = 0; c < t->model()->columnCount(); ++c) {
      cells << cellText(t->model()->index(r, c));
    }
    txt += cells.join('\t') + '\n';
  }
  return txt;
}

}  // namespace uwf::ui
