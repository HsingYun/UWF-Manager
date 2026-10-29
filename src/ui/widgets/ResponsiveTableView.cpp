// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "ResponsiveTableView.h"

#include <QHeaderView>
#include <QResizeEvent>
#include <QShowEvent>
#include <QTimer>
#include <algorithm>
#include <utility>

#include "TablePresentation.h"
#include "design/DesignTokens.h"

namespace uwf::ui {
ResponsiveTableView::ResponsiveTableView(QVector<Column> columns, QWidget* parent) : QTableView(parent), m_columns(std::move(columns)) {
  configureReportTable(this);
  setFrameShape(QFrame::NoFrame);
  setEditTriggers(QAbstractItemView::NoEditTriggers);
  setSelectionBehavior(QAbstractItemView::SelectRows);
  setSelectionMode(QAbstractItemView::SingleSelection);
  setWordWrap(false);
  setTextElideMode(Qt::ElideRight);
  setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
  verticalHeader()->hide();
  verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
  horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
  horizontalHeader()->setFixedHeight(design::tableHeaderHeight);
  auto type = font();
  type.setPixelSize(13);
  setFont(type);
}
void ResponsiveTableView::resizeEvent(QResizeEvent* event) {
  QTableView::resizeEvent(event);
  updateColumns();
}
void ResponsiveTableView::showEvent(QShowEvent* event) {
  QTableView::showEvent(event);
  updateColumns();
  QTimer::singleShot(0, this, [this] { updateColumns(); });
}
void ResponsiveTableView::updateColumns() {
  if (!model() || m_columns.isEmpty()) return;
  const int count = std::min(static_cast<int>(m_columns.size()), model()->columnCount());
  int available = viewport()->width();
  int reserved = 0;
  for (int column = 0; column < count; ++column) reserved += m_columns[column].minimum;
  available = std::max(available, reserved);
  int used = 0;
  for (int column = 0; column < count; ++column) {
    int remaining = 0;
    for (int next = column + 1; next < count; ++next) remaining += m_columns[next].minimum;
    const auto& spec = m_columns[column];
    const int preferred = column == count - 1 ? available - used : spec.fraction > 0 ? qRound(available * spec.fraction) : spec.minimum;
    const int width = std::clamp(preferred, spec.minimum, available - used - remaining);
    setColumnWidth(column, width);
    used += width;
  }
}
}  // namespace uwf::ui
