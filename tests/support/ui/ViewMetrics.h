// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <QAbstractItemView>
#include <QHeaderView>
#include <QTableView>
namespace uwf::test {
inline int displayedRowHeight(const QObject* object) {
  const auto* view = qobject_cast<const QAbstractItemView*>(object);
  if (!view || !view->model() || view->model()->rowCount() == 0) return 0;
  if (const auto* table = qobject_cast<const QTableView*>(view)) return table->verticalHeader()->sectionSize(0);
  return view->visualRect(view->model()->index(0, 0)).height();
}
}  // namespace uwf::test
