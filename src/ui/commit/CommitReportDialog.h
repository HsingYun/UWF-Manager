// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

// 提交 / 删除批处理结果对话框：
//   * presentation::CommitReportRow      —— 单个目标的结果行（成功 / 跳过 / 失败）。
//   * presentation::explainCommitFailure —— HRESULT / returnValue → 给普通用户的解释串。
//   * presentation::formatErrorCode      —— HRESULT / returnValue → "0x80041001" / "rv=N"。
//   * showCommitReport     —— 分页结果对话框；调用方拼好 row 列表后弹出。

#include <QList>
#include <QString>
#include <cstdint>
#include <optional>

#include "presentation/commit/CommitReportViewModel.h"

class QWidget;

namespace uwf::ui {

// 完成原因与未处理数量独立传入；跳过、取消和执行中断不冒充写入失败。
// 模态执行（dlg.exec()），返回后释放所有 widget。
void showCommitReport(QWidget* parent, const QList<presentation::CommitReportRow>& rows, app::CommitOperation operation,
                      presentation::CommitCompletion completion = {});

}  // namespace uwf::ui
