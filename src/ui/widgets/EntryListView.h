// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include "SelectionListView.h"
namespace uwf::presentation {
class PageProxyModel;
}
class QSortFilterProxyModel;
namespace uwf::ui {
class PagedViewController;
class TextLabel;
class EntryListView final : public SelectionListView {
 public:
  EntryListView(const QString& heading, bool staging, QWidget* parent = nullptr);
  void setModel(QAbstractItemModel* model) override;
  void setFilterText(const QString& text);
  void setHeading(const QString& text);
  [[nodiscard]] presentation::PageProxyModel& pages() const { return *m_pages; }

 protected:
  void resizeEvent(QResizeEvent* event) override;

 private:
  QWidget* m_header;
  TextLabel* m_heading = nullptr;
  QSortFilterProxyModel* m_filter;
  presentation::PageProxyModel* m_pages;
  PagedViewController* m_scrolling;
};
}  // namespace uwf::ui
