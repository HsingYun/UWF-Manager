// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

// QTableView 选区 / 全表 → 制表符分隔文本。供"提交结果"对话框和日志
// 查看器共用，避免两处各写一份。

#include <QString>

class QTableView;

namespace uwf::ui {

// 把表格选中区域按 "制表符 + 换行" 拼成可直接粘贴到 Excel 的文本。
QString tableSelectionToText(const QTableView* t);

// 把指定行区间拼成可复制文本（含表头），供当前页复制使用。
QString tableRowsToText(const QTableView* table, int firstRow, int endRow);

}  // namespace uwf::ui
