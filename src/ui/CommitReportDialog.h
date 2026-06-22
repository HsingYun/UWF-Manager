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

// 提交 / 删除批处理结果对话框：
//   * CommitReportRow      —— 单个目标的结果行（成功 / 跳过 / 失败）。
//   * explainCommitFailure —— HRESULT / returnValue → 给普通用户的解释串。
//   * formatErrorCode      —— HRESULT / returnValue → "0x80041001" / "rv=N"。
//   * showCommitReport     —— 分页结果对话框；调用方拼好 row 列表后弹出。

#include <QList>
#include <QString>
#include <cstdint>
#include <optional>

class QWidget;

namespace uwf::ui {

struct CommitReportRow {
  QString category;   // "成功" / "跳过" / "失败"
  QString path;       // 完整路径 / 注册表键
  QString errorCode;  // "0x80041001" 之类，成功为 "-"
  QString reason;     // 面向普通用户的解释，成功为 "-"
  // 仅删除操作填充：执行前 / 后目标是否存在。提交操作留 nullopt，结果表不显示这两列。
  std::optional<bool> existedBefore;
  std::optional<bool> existsAfter;
};

// isDeletion 区分 Commit{File,Registry}（=false）vs Commit{File,Registry}Deletion
// （=true）——同一个 HRESULT 在两类操作下含义不同，文案得分别写。
[[nodiscard]] QString explainCommitFailure(int32_t hresult, uint32_t returnValue, bool isDeletion);

[[nodiscard]] QString formatErrorCode(int32_t hresult, uint32_t returnValue);

// canceledRemaining > 0 表示用户在批处理中途取消，未处理的条数会写进汇总行。
// 模态执行（dlg.exec()），返回后释放所有 widget。
void showCommitReport(QWidget* parent, const QList<CommitReportRow>& rows, int canceledRemaining = 0);

}  // namespace uwf::ui
