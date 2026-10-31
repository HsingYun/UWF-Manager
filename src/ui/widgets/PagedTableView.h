// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QTableView>

namespace uwf::presentation {
class PageProxyModel;
}
namespace uwf::ui {
class PagedTableView final : public QTableView {
 public:
  PagedTableView(presentation::PageProxyModel& pages, int rowHeight, QWidget* parent = nullptr);
  [[nodiscard]] QSize minimumSizeHint() const override;

 protected:
  void keyPressEvent(QKeyEvent* event) override;

 private:
  const int m_rowHeight;
};
}  // namespace uwf::ui
