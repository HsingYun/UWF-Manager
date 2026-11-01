// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include "presentation/controls/tables/SelectionListView.h"
namespace uwf::presentation {
class PageProxyModel;
}
class QSortFilterProxyModel;
namespace uwf::ui {
class PagedViewController;
class RoundedCornerOverlay;
class TextLabel;
class PathEntryListView final : public SelectionListView {
 public:
  PathEntryListView(const QString& heading, const QString& statusHeading, QWidget* parent = nullptr);
  void setModel(QAbstractItemModel* model) override;
  void setFilterText(const QString& text);
  void setHeading(const QString& text);
  void setStatusHeading(const QString& text);
  [[nodiscard]] presentation::PageProxyModel& pages() const { return *m_pages; }

 protected:
  void resizeEvent(QResizeEvent* event) override;
  void changeEvent(QEvent* event) override;

 private:
  QWidget* m_header;
  TextLabel* m_heading = nullptr;
  TextLabel* m_statusHeading = nullptr;
  QSortFilterProxyModel* m_filter;
  presentation::PageProxyModel* m_pages;
  PagedViewController* m_scrolling;
  RoundedCornerOverlay* m_cornerOverlay = nullptr;
};
}  // namespace uwf::ui
