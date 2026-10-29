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

 protected:
  void keyPressEvent(QKeyEvent* event) override;
};
}  // namespace uwf::ui
