// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <QObject>
#include <QPointer>
#include <QTimer>
class QAbstractItemView;
class QScrollBar;
class QWidget;
namespace uwf::presentation {
class PageProxyModel;
}
namespace uwf::ui {
class PagedWindowLayout;
class PagedViewController final : public QObject {
  Q_OBJECT
 public:
  PagedViewController(QAbstractItemView* view, presentation::PageProxyModel& pages, int rowHeight);
  ~PagedViewController() override;
  void updateGeometry();
  [[nodiscard]] QAbstractItemView* view() const { return m_view; }
  [[nodiscard]] int rowHeight() const { return m_rowHeight; }
  [[nodiscard]] int availableViewportHeight() const;

 protected:
  bool eventFilter(QObject* object, QEvent* event) override;

 private:
  void syncPage();
  void syncOverlay();
  void showOverlay();
  void useItemScrolling();
  void beginDrag();
  QAbstractItemView* m_view;
  QPointer<PagedWindowLayout> m_windowLayout;
  QPointer<presentation::PageProxyModel> m_pages;
  QScrollBar* m_overlay;
  QTimer m_idle;
  int m_rowHeight;
  bool m_changingMode = false;
  bool m_updatingGeometry = false;
  bool m_edgeHovered = false;
  bool m_overlayHovered = false;
};
}  // namespace uwf::ui
