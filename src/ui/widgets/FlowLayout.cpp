// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "FlowLayout.h"

#include <QStyle>
#include <QWidget>
#include <algorithm>

namespace uwf::ui {

FlowLayout::FlowLayout(QWidget* parent) : QLayout(parent) {
  setContentsMargins(0, 0, 0, 0);
  setSpacing(12);
}

FlowLayout::~FlowLayout() = default;

void FlowLayout::addItem(QLayoutItem* item) {
  m_items.emplace_back(item);
  invalidate();
}

int FlowLayout::count() const { return static_cast<int>(m_items.size()); }

QLayoutItem* FlowLayout::itemAt(const int index) const { return index >= 0 && index < count() ? m_items[static_cast<size_t>(index)].get() : nullptr; }

QLayoutItem* FlowLayout::takeAt(const int index) {
  if (index < 0 || index >= count()) return nullptr;
  auto item = std::move(m_items[static_cast<size_t>(index)]);
  m_items.erase(m_items.begin() + index);
  invalidate();
  return item.release();
}

Qt::Orientations FlowLayout::expandingDirections() const { return Qt::Horizontal; }
bool FlowLayout::hasHeightForWidth() const { return true; }
int FlowLayout::heightForWidth(const int width) const { return arrange(QRect(0, 0, width, 0)).height; }

QSize FlowLayout::minimumSize() const {
  QSize size(0, 0);
  for (const auto& item : m_items)
    if (!item->isEmpty()) size = size.expandedTo(item->minimumSize());
  const auto margins = contentsMargins();
  return size + QSize(margins.left() + margins.right(), margins.top() + margins.bottom());
}

QSize FlowLayout::sizeHint() const { return minimumSize(); }

FlowLayout::Placement FlowLayout::arrange(const QRect& rect) const {
  const auto margins = contentsMargins();
  const QRect available = rect.marginsRemoved(margins);
  Placement placement;
  placement.items.reserve(m_items.size());
  int x = available.x();
  int y = available.y();
  int lineHeight = 0;
  size_t lineStart = 0;
  const auto alignLine = [&] {
    for (size_t i = lineStart; i < placement.items.size(); ++i) {
      auto& item = placement.items[i];
      if (!item.isEmpty()) item.translate(0, (lineHeight - item.height()) / 2);
    }
    lineStart = placement.items.size();
  };
  const int gap = std::max(0, spacing());
  for (const auto& item : m_items) {
    if (item->isEmpty()) {
      placement.items.emplace_back();
      continue;
    }
    QSize size = item->sizeHint().expandedTo(item->minimumSize());
    if (x > available.x() && x + size.width() > available.x() + available.width()) {
      alignLine();
      x = available.x();
      y += lineHeight + gap;
      lineHeight = 0;
    }
    if (item->hasHeightForWidth()) size.setHeight(item->heightForWidth(size.width()));
    if (item->alignment() & Qt::AlignRight) x = std::max(x, available.x() + available.width() - size.width());
    placement.items.emplace_back(QPoint(x, y), size);
    x += size.width() + gap;
    lineHeight = std::max(lineHeight, size.height());
  }
  alignLine();
  placement.height = y - rect.y() + lineHeight + margins.bottom();
  return placement;
}

void FlowLayout::setGeometry(const QRect& rect) {
  QLayout::setGeometry(rect);
  const auto placement = arrange(rect);
  const auto direction = parentWidget() ? parentWidget()->layoutDirection() : Qt::LeftToRight;
  for (size_t i = 0; i < m_items.size(); ++i)
    if (!m_items[i]->isEmpty()) m_items[i]->setGeometry(QStyle::visualRect(direction, rect, placement.items[i]));
}

}  // namespace uwf::ui
