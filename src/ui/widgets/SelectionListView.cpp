// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "SelectionListView.h"

#include <QApplication>
#include <QMouseEvent>
namespace uwf::ui {
SelectionListView::SelectionListView(QWidget* parent) : QListView(parent) {
  setUniformItemSizes(true);
  setSelectionMode(QAbstractItemView::ExtendedSelection);
  setSelectionRectVisible(true);
  setMouseTracking(true);
  setEditTriggers(QAbstractItemView::NoEditTriggers);
  setDragEnabled(false);
}
void SelectionListView::mousePressEvent(QMouseEvent* event) {
  m_pressed = indexAt(event->position().toPoint());
  m_pressPosition = event->position().toPoint();
  m_dragged = false;
  m_toggleOff = event->button() == Qt::LeftButton && event->modifiers() == Qt::NoModifier && selectionModel() && m_pressed.isValid() &&
                selectionModel()->isSelected(m_pressed) && selectionModel()->selectedRows().size() == 1;
  QListView::mousePressEvent(event);
}
void SelectionListView::mouseMoveEvent(QMouseEvent* event) {
  if ((event->position().toPoint() - m_pressPosition).manhattanLength() >= QApplication::startDragDistance()) m_dragged = true;
  QListView::mouseMoveEvent(event);
}
void SelectionListView::mouseReleaseEvent(QMouseEvent* event) {
  QListView::mouseReleaseEvent(event);
  if (event->button() == Qt::LeftButton && m_toggleOff && !m_dragged && m_pressed.isValid() && indexAt(event->position().toPoint()) == m_pressed) {
    selectionModel()->clearSelection();
    selectionModel()->setCurrentIndex({}, QItemSelectionModel::NoUpdate);
  }
  m_toggleOff = false;
  m_pressed = QPersistentModelIndex();
}
}  // namespace uwf::ui
