// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QListView>
#include <QPersistentModelIndex>
#include <functional>

namespace uwf::ui {
// Native batched item layout, with a bounded pool of interactive row widgets.
// Data stays in the source model; only visible rows own live controls.
class RecyclingListView final : public QListView {
 public:
  using Factory = std::function<QWidget*(int, QWidget*)>;
  using Bind = std::function<void(QWidget*, const QModelIndex&)>;
  using Measure = std::function<QSize(const QModelIndex&, int)>;
  RecyclingListView(int kindRole, Factory factory, Bind bind, Measure measure, QWidget* parent = nullptr);
  void setModel(QAbstractItemModel* model) override;
  void refreshRowSizes();
  void refreshRowSize(const QModelIndex& index);

 protected:
  void paintEvent(QPaintEvent* event) override;
  void scrollContentsBy(int dx, int dy) override;
  void resizeEvent(QResizeEvent* event) override;

 private:
  void synchronize();
  void relayoutKeepingPosition();
  void relayoutKeepingPosition(const QPersistentModelIndex& anchor, int offset, int fallbackPosition);
  QModelIndex firstVisibleIndex() const;
  struct Slot { QWidget* widget; int kind; QPersistentModelIndex index; };
  int m_kindRole;
  Factory m_factory;
  Bind m_bind;
  QList<Slot> m_slots;
  QList<QMetaObject::Connection> m_connections;
  bool m_synchronizing = false;
};
}  // namespace uwf::ui
