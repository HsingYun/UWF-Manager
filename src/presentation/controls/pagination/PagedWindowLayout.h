// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QObject>
#include <QPointer>
#include <QSize>
#include <QTimer>
#include <vector>

class QWidget;

namespace uwf::ui {
class PagedViewController;
// One owner for the user's requested size and the whole-row constraint.
// Native sizing, responsive layout and item views share this typed contract.
class PagedWindowLayout final : public QObject {
  Q_OBJECT
 public:
  static PagedWindowLayout& forWindow(QWidget* window);
  [[nodiscard]] QSize requestedSize() const;
  void observeResize(QSize size);
  [[nodiscard]] int alignedHeight(int requestedHeight) const;
  [[nodiscard]] bool hasPageHeightConstraint() const;
  [[nodiscard]] QSize constrainNativeResize(QSize requested, bool anchorBottom);
  void endNativeResize();
  void fit(bool keepCenter = false);
  void setPageVisible(PagedViewController& page, bool visible);

 protected:
  bool eventFilter(QObject* object, QEvent* event) override;

 private:
  explicit PagedWindowLayout(QWidget* window);
  void scheduleFit();
  [[nodiscard]] const PagedViewController* activePage() const;
  std::vector<QPointer<PagedViewController>> m_visiblePages;
  QWidget* m_window;
  QSize m_requested;
  QSize m_fitted;
  QTimer m_fitTimer;
  bool m_fitting = false;
  bool m_nativeSizing = false;
  bool m_anchorBottom = false;
};
}  // namespace uwf::ui
