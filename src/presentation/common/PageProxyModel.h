// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <QList>
#include <QSortFilterProxyModel>

#include "Pager.h"

namespace uwf::presentation {
class PageProxyModel final : public QSortFilterProxyModel {
  Q_OBJECT
 public:
  explicit PageProxyModel(QObject* parent = nullptr);
  void setSourceModel(QAbstractItemModel* source) override;
  [[nodiscard]] const Pager& pager() const { return m_pager; }
  [[nodiscard]] int totalRows() const;
  void setPageSize(int size);
  void setContinuous(bool continuous);
  void syncScrollPosition(int firstRow, bool atEnd);
  void goFirst();
  void goPrevious();
  void goNext();
  void goLast();
 signals:
  void pageChanged();
  void pageNavigationRequested(int firstRow);

 protected:
  bool filterAcceptsRow(int row, const QModelIndex& parent) const override;

 private:
  void refresh();
  void selectPage(int page);
  Pager m_pager;
  bool m_continuous = false;
  QList<QMetaObject::Connection> m_sourceConnections;
};
}  // namespace uwf::presentation
