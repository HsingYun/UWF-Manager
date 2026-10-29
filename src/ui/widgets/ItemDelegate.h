// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <QHash>
#include <QIcon>
#include <QStyledItemDelegate>

namespace uwf::ui {
class ItemDelegate : public QStyledItemDelegate {
 public:
  explicit ItemDelegate(QObject* parent = nullptr);
  void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
  void setRowHeight(int height) { m_rowHeight = height; }
  QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;

 protected:
  void initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const override;

 private:
  mutable QHash<QString, QIcon> m_badgedIcons;
  int m_rowHeight = 0;
};
}  // namespace uwf::ui
