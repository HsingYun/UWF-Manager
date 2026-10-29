// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <QTableView>
#include <QVector>

namespace uwf::ui {
// Fixed leading columns, proportional content and a final flexible column share
// one header geometry. Cell text may elide; the model keeps the complete value.
class ResponsiveTableView final : public QTableView {
 public:
  struct Column {
    int minimum;
    qreal fraction = 0;
  };
  explicit ResponsiveTableView(QVector<Column> columns, QWidget* parent = nullptr);
 protected:
  void resizeEvent(QResizeEvent* event) override;
  void showEvent(QShowEvent* event) override;
 private:
  void updateColumns();
  QVector<Column> m_columns;
};
}  // namespace uwf::ui
