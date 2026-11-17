// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QAbstractItemView>
#include <QHeaderView>
#include <QScreen>
#include <QTableView>
#include <QWidget>
#include <QWindow>

namespace uwf::test {
inline QSize availableClientSize(const QWidget& widget) {
  const auto* window = widget.window();
  const auto* screen = window->screen();
  if (!screen) return {};
  const auto* handle = window->windowHandle();
  const auto margins = handle ? handle->frameMargins() : QMargins{};
  const auto size = screen->availableGeometry().size() - QSize(margins.left() + margins.right(), margins.top() + margins.bottom());
  return size.isEmpty() ? QSize{} : size;
}
inline QSize boundedWindowSize(const QWidget& widget, const QSize& requested) {
  const auto available = availableClientSize(widget);
  return requested.isEmpty() || available.isEmpty() ? QSize{} : requested.boundedTo(available);
}
inline int displayedRowHeight(const QObject* object) {
  const auto* view = qobject_cast<const QAbstractItemView*>(object);
  if (!view || !view->model() || view->model()->rowCount() == 0) return 0;
  if (const auto* table = qobject_cast<const QTableView*>(view)) return table->verticalHeader()->sectionSize(0);
  return view->visualRect(view->model()->index(0, 0)).height();
}
}  // namespace uwf::test
