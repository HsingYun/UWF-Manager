// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QAbstractItemView>
#include <QHeaderView>
#include <QScrollBar>
#include <QStyle>
#include <QTableView>
#include <algorithm>

namespace uwf::ui {
inline int availableItemViewportHeight(const QAbstractItemView& view) {
  int available = view.maximumViewportSize().height();
  const auto* horizontal = view.horizontalScrollBar();
  // maximumViewportSize assumes an AsNeeded bar has no range. A visible,
  // non-overlay bar consumes space from the rows' actual height budget.
  if (view.horizontalScrollBarPolicy() == Qt::ScrollBarAsNeeded && horizontal->isVisibleTo(&view) &&
      horizontal->style()->pixelMetric(QStyle::PM_ScrollView_ScrollBarOverlap, nullptr, horizontal) == 0)
    available -= horizontal->height();
  return std::max(0, available);
}

inline int minimumItemViewHeight(const QAbstractItemView& view, const int rowHeight, const int minimumHeight) {
  int chrome = std::max(0, view.height() - availableItemViewportHeight(view));
  if (const auto* table = qobject_cast<const QTableView*>(&view)) {
    // Before the first layout, the view can be shorter than its header. Use
    // the header's actual requirement rather than that transient geometry.
    int tableChrome = 2 * table->frameWidth();
    if (!table->horizontalHeader()->isHidden()) tableChrome += table->horizontalHeader()->height();
    const auto* horizontal = table->horizontalScrollBar();
    const bool visible = table->horizontalScrollBarPolicy() == Qt::ScrollBarAlwaysOn ||
                         (table->horizontalScrollBarPolicy() == Qt::ScrollBarAsNeeded && horizontal->isVisibleTo(table));
    if (visible && horizontal->style()->pixelMetric(QStyle::PM_ScrollView_ScrollBarOverlap, nullptr, horizontal) == 0)
      tableChrome += horizontal->sizeHint().height();
    chrome = std::max(chrome, tableChrome);
  }
  const int rows = std::max(1, (std::max(0, minimumHeight - chrome) + rowHeight - 1) / rowHeight);
  return chrome + rows * rowHeight;
}
}  // namespace uwf::ui
