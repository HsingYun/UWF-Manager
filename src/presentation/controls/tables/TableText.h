// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QString>

class QTableView;

namespace uwf::ui {

// Exports selected cells as tab-separated rows.
QString tableSelectionToText(const QTableView* t);

// Includes headers for the specified row range.
QString tableRowsToText(const QTableView* table, int firstRow, int endRow);

}  // namespace uwf::ui
