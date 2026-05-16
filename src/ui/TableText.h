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
