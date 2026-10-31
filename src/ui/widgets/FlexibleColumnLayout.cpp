// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "FlexibleColumnLayout.h"

#include <QAbstractItemView>
#include <QEvent>
#include <QScopedValueRollback>
#include <QScrollBar>

#include "ui/design/ItemViewGeometry.h"

namespace uwf::ui {
int FlexibleColumnLayout::pagedRecordsMinimumHeight() const {
  const int index = m_records ? indexOf(m_records) : -1;
  if (index < 0) return 0;
  const int minimum = itemAt(index)->minimumSize().height();
  if (!m_view || m_rowHeight <= 0) return minimum;
  const auto* inner = m_records->layout();
  if (!inner || inner->count() != 1 || inner->itemAt(0)->widget() != m_view) return minimum;
  // These record surfaces contain the view and its frame margins. Round the
  // whole minimum upward, so an explicit view/host minimum cannot force a
  // partial row after the scrollbar and header have taken their space.
  const auto frame = m_records->contentsMargins();
  const auto margins = inner->contentsMargins();
  const int outside = frame.top() + frame.bottom() + margins.top() + margins.bottom();
  return outside + minimumItemViewHeight(*m_view, m_rowHeight, std::max(0, minimum - outside));
}

QSize FlexibleColumnLayout::minimumSize() const {
  auto size = QVBoxLayout::minimumSize();
  const int index = m_records ? indexOf(m_records) : -1;
  if (index >= 0 && !itemAt(index)->isEmpty()) size.rheight() += pagedRecordsMinimumHeight() - itemAt(index)->minimumSize().height();
  return size;
}

void FlexibleColumnLayout::setPagedDetails(QWidget* records, QAbstractItemView* view, QWidget* details, const int rowHeight) {
  m_records = records;
  m_details = details;
  m_view = view;
  m_rowHeight = rowHeight;
  view->installEventFilter(this);
  view->viewport()->installEventFilter(this);
  view->horizontalScrollBar()->installEventFilter(this);
  invalidate();
}

void FlexibleColumnLayout::setGeometry(const QRect& rect) {
  if (m_distributingHeight) return;
  const QScopedValueRollback<bool> distributing(m_distributingHeight, true);
  QVBoxLayout::setGeometry(rect);
  if (!m_records || !m_details || !m_view || m_rowHeight <= 0 || m_records->isHidden() || m_details->isHidden()) return;
  const int recordsIndex = indexOf(m_records);
  const int detailsIndex = indexOf(m_details);
  if (recordsIndex < 0 || detailsIndex <= recordsIndex) return;
  if (auto* layout = m_records->layout()) layout->activate();
  const int remainder = availableItemViewportHeight(*m_view) % m_rowHeight;
  if (!remainder) return;

  // Allocate from the original layout budget on each pass. The whole records
  // surface loses the partial row; the details gain that same space without
  // moving the window, pagination or the gap between the two surfaces.
  auto* records = itemAt(recordsIndex);
  auto* details = itemAt(detailsIndex);
  const auto recordsRect = records->geometry();
  const auto detailsRect = details->geometry();
  if (recordsRect.height() - remainder < pagedRecordsMinimumHeight()) return;
  records->setGeometry(recordsRect.adjusted(0, 0, 0, -remainder));
  details->setGeometry(detailsRect.adjusted(0, -remainder, 0, 0));
}

bool FlexibleColumnLayout::eventFilter(QObject* object, QEvent* event) {
  if (!m_distributingHeight && (event->type() == QEvent::Resize || event->type() == QEvent::LayoutRequest || event->type() == QEvent::Show ||
                               event->type() == QEvent::Hide))
    invalidate();
  return QVBoxLayout::eventFilter(object, event);
}
}  // namespace uwf::ui
