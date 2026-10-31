// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "PagedTableView.h"

#include <QHeaderView>
#include <QKeyEvent>

#include "PagedViewController.h"
#include "TablePresentation.h"
#include "design/DesignTokens.h"
#include "presentation/common/PageProxyModel.h"
#include "ui/design/ItemViewGeometry.h"

namespace uwf::ui {
PagedTableView::PagedTableView(presentation::PageProxyModel& pages, const int rowHeight, QWidget* parent) : QTableView(parent), m_rowHeight(rowHeight) {
  setModel(&pages);
  configureReportTable(this);
  setFrameShape(QFrame::NoFrame);
  setEditTriggers(QAbstractItemView::NoEditTriggers);
  setSelectionBehavior(QAbstractItemView::SelectRows);
  setSelectionMode(QAbstractItemView::ExtendedSelection);
  setWordWrap(false);
  setTextElideMode(Qt::ElideRight);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
  auto type = font();
  type.setPixelSize(13);
  setFont(type);
  verticalHeader()->hide();
  verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
  verticalHeader()->setDefaultSectionSize(rowHeight);
  horizontalHeader()->setFixedHeight(design::tableHeaderHeight);
  new PagedViewController(this, pages, rowHeight);
}
QSize PagedTableView::minimumSizeHint() const {
  auto minimum = QTableView::minimumSizeHint();
  minimum.setHeight(minimumItemViewHeight(*this, m_rowHeight, design::tableHeaderHeight + m_rowHeight));
  return minimum;
}
void PagedTableView::keyPressEvent(QKeyEvent* event) {
  QTableView::keyPressEvent(event);
  // Qt emits activated(), then ignores Enter for a read-only table. Keep the
  // key in this view so a parent dialog cannot also trigger its default action.
  if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) event->accept();
}
}  // namespace uwf::ui
