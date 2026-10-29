// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <QListView>
#include <QPersistentModelIndex>
namespace uwf::ui {
class SelectionListView : public QListView {
 public:
  explicit SelectionListView(QWidget* parent = nullptr);

 protected:
  void mousePressEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;

 private:
  QPersistentModelIndex m_pressed;
  QPoint m_pressPosition;
  bool m_toggleOff = false;
  bool m_dragged = false;
};
}  // namespace uwf::ui
