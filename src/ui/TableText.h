/*
 * Copyright (c) 2026 HsingYun (iakext@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

// QTableWidget 选区 / 全表 → 制表符分隔文本。供"提交结果"对话框和日志
// 查看器共用，避免两处各写一份。

#include <QString>

class QTableWidget;

namespace uwf::ui {

// 把表格选中区域按 "制表符 + 换行" 拼成可直接粘贴到 Excel 的文本。
QString tableSelectionToText(const QTableWidget* t);

// 把整张表拼成可复制文本（含表头）。
QString tableAllToText(const QTableWidget* t);

}  // namespace uwf::ui
