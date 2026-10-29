// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "RecyclingListView.h"

#include <QResizeEvent>
#include <QHash>
#include <QScopedValueRollback>
#include <QScrollBar>
#include <QStyledItemDelegate>
#include <algorithm>
#include <utility>

namespace uwf::ui {
namespace {
class RowLayout final : public QStyledItemDelegate {
 public:
  RowLayout(RecyclingListView::Measure measure, QListView* view) : QStyledItemDelegate(view), m_measure(std::move(measure)), m_view(view) {}
  QSize sizeHint(const QStyleOptionViewItem&, const QModelIndex& index) const override {
    const int width = m_view->viewport()->width();
    if (width != m_width) { m_sizes.clear(); m_width = width; }
    const QPersistentModelIndex key(index);
    auto found = m_sizes.constFind(key);
    if (found != m_sizes.cend()) return found.value();
    const auto size = m_measure(index, width);
    m_sizes.insert(key, size);
    return size;
  }
  void invalidate(const QModelIndex& index = {}) {
    if (index.isValid()) m_sizes.remove(QPersistentModelIndex(index));
    else m_sizes.clear();
  }
  void paint(QPainter*, const QStyleOptionViewItem&, const QModelIndex&) const override {}
 private:
  RecyclingListView::Measure m_measure;
  QListView* m_view;
  mutable int m_width = -1;
  mutable QHash<QPersistentModelIndex, QSize> m_sizes;
};
}
RecyclingListView::RecyclingListView(const int kindRole, Factory factory, Bind bind, Measure measure, QWidget* parent)
    : QListView(parent), m_kindRole(kindRole), m_factory(std::move(factory)), m_bind(std::move(bind)) {
  setItemDelegate(new RowLayout(std::move(measure), this));
  setLayoutMode(QListView::Batched);
  setBatchSize(64);
  setResizeMode(QListView::Adjust);
  // Row heights depend on viewport width, including when a scrollbar appears
  // during layout. Let QListView schedule its native wrapped-row reflow.
  setWordWrap(true);
  setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setSelectionMode(QAbstractItemView::NoSelection);
  setEditTriggers(QAbstractItemView::NoEditTriggers);
  setFrameShape(QFrame::NoFrame);
  setStyleSheet(QStringLiteral("QListView { background: transparent; border: none; }"));
  viewport()->setAutoFillBackground(false);
  connect(verticalScrollBar(), &QScrollBar::rangeChanged, viewport(), qOverload<>(&QWidget::update));
}
void RecyclingListView::setModel(QAbstractItemModel* source) {
  for (const auto& connection : std::as_const(m_connections)) disconnect(connection);
  m_connections.clear();
  static_cast<RowLayout*>(itemDelegate())->invalidate();
  for (auto& slot : m_slots) { slot.index = QPersistentModelIndex(); slot.widget->hide(); }
  QListView::setModel(source);
  if (source) {
    const auto invalidate = [this] { static_cast<RowLayout*>(itemDelegate())->invalidate(); };
    m_connections.append(connect(source, &QAbstractItemModel::rowsAboutToBeInserted, this, invalidate));
    m_connections.append(connect(source, &QAbstractItemModel::rowsAboutToBeRemoved, this, invalidate));
    m_connections.append(connect(source, &QAbstractItemModel::layoutAboutToBeChanged, this, invalidate));
    m_connections.append(connect(source, &QAbstractItemModel::modelAboutToBeReset, this, [this] {
      static_cast<RowLayout*>(itemDelegate())->invalidate();
    }));
    m_connections.append(connect(source, &QAbstractItemModel::dataChanged, this, [this](const QModelIndex& first, const QModelIndex& last) {
      for (auto& slot : m_slots)
        if (slot.index.row() >= first.row() && slot.index.row() <= last.row()) {
          slot.index = QPersistentModelIndex();
          slot.widget->hide();
        }
      refreshRowSizes();
    }));
  }
}
void RecyclingListView::refreshRowSizes() {
  static_cast<RowLayout*>(itemDelegate())->invalidate();
  relayoutKeepingPosition();
}
void RecyclingListView::refreshRowSize(const QModelIndex& index) {
  static_cast<RowLayout*>(itemDelegate())->invalidate(index);
  relayoutKeepingPosition();
}
void RecyclingListView::relayoutKeepingPosition() {
  const QPersistentModelIndex anchor(firstVisibleIndex());
  const int offset = anchor.isValid() ? visualRect(anchor).top() : 0;
  relayoutKeepingPosition(anchor, offset, verticalScrollBar()->value());
}
void RecyclingListView::relayoutKeepingPosition(const QPersistentModelIndex& anchor, const int offset, const int fallbackPosition) {
  if (!isVisible()) { scheduleDelayedItemsLayout(); return; }
  const auto mode = layoutMode();
  // Reflow cached row sizes in one pass. Resetting to the first batch would
  // temporarily shrink the scroll range and clamp a distant current position.
  setLayoutMode(QListView::SinglePass);
  doItemsLayout();
  setLayoutMode(mode);
  const int position = anchor.isValid() ? verticalScrollBar()->value() + visualRect(anchor).top() - offset : fallbackPosition;
  verticalScrollBar()->setValue(position);
  synchronize();
  viewport()->update();
}
void RecyclingListView::resizeEvent(QResizeEvent* event) {
  const QPersistentModelIndex anchor(!m_slots.isEmpty() && state() != ExpandingState ? firstVisibleIndex() : QModelIndex{});
  const int offset = anchor.isValid() ? visualRect(anchor).top() : 0;
  const int position = verticalScrollBar()->value();
  QListView::resizeEvent(event);
  if (anchor.isValid()) relayoutKeepingPosition(anchor, offset, position);
  synchronize();
}
void RecyclingListView::scrollContentsBy(const int dx, const int dy) {
  QListView::scrollContentsBy(dx, dy);
  synchronize();
}
void RecyclingListView::paintEvent(QPaintEvent* event) {
  synchronize();
  QListView::paintEvent(event);
}
QModelIndex RecyclingListView::firstVisibleIndex() const {
  QModelIndex first;
  for (int y = 0; y < viewport()->height() && !first.isValid(); ++y) first = indexAt(QPoint(viewport()->width() / 2, y));
  return first;
}
void RecyclingListView::synchronize() {
  if (m_synchronizing || !model() || !isVisible()) return;
  const QScopedValueRollback synchronizing(m_synchronizing, true);
  const auto first = firstVisibleIndex();
  QList<QModelIndex> visible;
  if (first.isValid()) {
    for (int row = first.row(); row < model()->rowCount(rootIndex()); ++row) {
      const auto index = model()->index(row, modelColumn(), rootIndex());
      const auto bounds = visualRect(index);
      if (!bounds.isValid() || bounds.top() >= viewport()->height()) break;
      if (bounds.bottom() >= 0) visible.append(index);
    }
  }
  for (auto& slot : m_slots) {
    if (visible.contains(slot.index)) continue;
    slot.index = QPersistentModelIndex();
    slot.widget->hide();
  }
  for (const auto& index : std::as_const(visible)) {
    auto slot = std::find_if(m_slots.begin(), m_slots.end(), [&](const auto& candidate) { return candidate.index == index; });
    if (slot == m_slots.end()) {
      const int kind = index.data(m_kindRole).toInt();
      slot = std::find_if(m_slots.begin(), m_slots.end(), [&](const auto& candidate) { return !candidate.index.isValid() && candidate.kind == kind; });
      if (slot == m_slots.end()) {
        m_slots.append({m_factory(kind, viewport()), kind, {}});
        slot = std::prev(m_slots.end());
      }
      slot->index = index;
      m_bind(slot->widget, index);
    }
    slot->widget->setGeometry(visualRect(index));
    slot->widget->show();
  }
}
}  // namespace uwf::ui
