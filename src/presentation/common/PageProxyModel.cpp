// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "PageProxyModel.h"

#include <utility>

namespace uwf::presentation {
PageProxyModel::PageProxyModel(QObject* parent) : QSortFilterProxyModel(parent) { setDynamicSortFilter(false); }
void PageProxyModel::setSourceModel(QAbstractItemModel* source) {
  for (const auto& connection : std::as_const(m_sourceConnections)) disconnect(connection);
  m_sourceConnections.clear();
  QSortFilterProxyModel::setSourceModel(source);
  if (source) {
    m_sourceConnections = {connect(source, &QAbstractItemModel::modelReset, this, &PageProxyModel::refresh),
                           connect(source, &QAbstractItemModel::rowsInserted, this, &PageProxyModel::refresh),
                           connect(source, &QAbstractItemModel::rowsRemoved, this, &PageProxyModel::refresh),
                           connect(source, &QObject::destroyed, this, [this] {
                             m_pager.clamp(0);
                             // A common parent may already be destroying its views.
                             // Notify only if the proxy survives that destruction.
                             QMetaObject::invokeMethod(this, &PageProxyModel::refresh, Qt::QueuedConnection);
                           })};
  }
  refresh();
}
int PageProxyModel::totalRows() const { return sourceModel() ? sourceModel()->rowCount() : 0; }
bool PageProxyModel::filterAcceptsRow(int row, const QModelIndex& parent) const {
  return !parent.isValid() && (m_continuous || (row >= m_pager.pageStart() && row < m_pager.pageEnd(totalRows())));
}
void PageProxyModel::setContinuous(const bool continuous) {
  if (m_continuous == continuous) return;
#if QT_VERSION >= QT_VERSION_CHECK(6, 10, 0)
  beginFilterChange();
#endif
  m_continuous = continuous;
#if QT_VERSION >= QT_VERSION_CHECK(6, 10, 0)
  endFilterChange(Direction::Rows);
#else
  invalidateRowsFilter();
#endif
  emit pageChanged();
}
void PageProxyModel::syncScrollPosition(const int firstRow, const bool atEnd) {
  const int page = atEnd ? std::max(0, m_pager.pageCount(totalRows()) - 1) : std::max(0, firstRow) / m_pager.pageSize;
  if (m_pager.currentPage == page) return;
  m_pager.currentPage = page;
  m_pager.clamp(totalRows());
  emit pageChanged();
}
void PageProxyModel::selectPage(int page) {
  if (m_continuous) {
    m_pager.currentPage = page;
    m_pager.clamp(totalRows());
    emit pageChanged();
    emit pageNavigationRequested(m_pager.pageStart());
    return;
  }
#if QT_VERSION >= QT_VERSION_CHECK(6, 10, 0)
  beginFilterChange();
#endif
  m_pager.currentPage = page;
  m_pager.clamp(totalRows());
#if QT_VERSION >= QT_VERSION_CHECK(6, 10, 0)
  endFilterChange(Direction::Rows);
#else
  invalidateRowsFilter();
#endif
  emit pageChanged();
}
void PageProxyModel::refresh() { selectPage(m_pager.currentPage); }
void PageProxyModel::setPageSize(int size) {
  if (m_pager.pageSize == std::max(1, size)) return;
  if (m_continuous) {
    m_pager.setPageSize(size);
    m_pager.clamp(totalRows());
    emit pageChanged();
    return;
  }
#if QT_VERSION >= QT_VERSION_CHECK(6, 10, 0)
  beginFilterChange();
#endif
  m_pager.setPageSize(size);
  m_pager.clamp(totalRows());
#if QT_VERSION >= QT_VERSION_CHECK(6, 10, 0)
  endFilterChange(Direction::Rows);
#else
  invalidateRowsFilter();
#endif
  emit pageChanged();
}
void PageProxyModel::goFirst() { selectPage(0); }
void PageProxyModel::goPrevious() { selectPage(std::max(0, m_pager.currentPage - 1)); }
void PageProxyModel::goNext() { selectPage(m_pager.currentPage + 1); }
void PageProxyModel::goLast() { selectPage(m_pager.pageCount(totalRows()) - 1); }
}  // namespace uwf::presentation

#include "moc_PageProxyModel.cpp"
