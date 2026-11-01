// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "presentation/controls/tables/TablePresentation.h"

#include <QTableView>

#include "presentation/controls/tables/ItemDelegate.h"

namespace uwf::ui {
void configureReportTable(QTableView* table) {
  table->setShowGrid(false);
  table->setIconSize(QSize(14, 14));
  table->setItemDelegate(new ItemDelegate(table));
}
}  // namespace uwf::ui
