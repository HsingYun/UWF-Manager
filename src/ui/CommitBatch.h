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

// 多目标批处理执行 + 结果汇总 + 弹结果对话框。提交 / 删除四个 slot 共用：
// 调用方算好 target 列表 + 写入回调 + 展示串回调，剩下的"进度条 / 取消 / 三档
// 归类 / 报告"都在这里。
//
// existsFn 可选——只有删除操作传它，在 commit 前后各探一次目标是否存在，结果
// 会进 CommitReportRow.existedBefore / existsAfter，决定结果对话框是否多两列。
// 不传时整段 constexpr 跳过，提交操作无开销。
//
// 模板定义留 header：4 个 slot 各自实例化一份，避免引入 type-erase 钩子。

#include <QList>
#include <QProgressDialog>
#include <QString>
#include <memory>
#include <optional>
#include <type_traits>
#include <utility>

#include "../uwf/wmi/WmiResult.h"
#include "CommitReportDialog.h"
#include "I18n.h"

class QWidget;

namespace uwf::ui {

template <typename Target, typename DisplayFn, typename CommitFn, typename ExistsFn = decltype(nullptr)>
void runCommitBatch(QWidget* parent, const QString& progressTitle, const QList<Target>& targets, DisplayFn displayOf, CommitFn commitOne,
                    ExistsFn existsFn = {}) {
  const int total = static_cast<int>(targets.size());

  // 进度条只在多目标时弹；单目标一两次 WMI 调用，弹窗反因 show 计时 / autoClose
  // 的时序问题残留在屏上。setValue 内部 processEvents——必须 WindowModal，否则
  // commit 半途用户点别处会嵌套触发同一个 thread-local session（WMI 不可重入）。
  std::unique_ptr<QProgressDialog> progress;
  if (total > 1) {
    progress = std::make_unique<QProgressDialog>(I18n::tr("Committing…"), I18n::tr("Cancel"), 0, total, parent);
    progress->setWindowTitle(progressTitle);
    progress->setWindowModality(Qt::WindowModal);
    progress->setMinimumDuration(500);
    progress->setAutoClose(true);
    progress->setAutoReset(false);
  }

  // existsFn 为 decltype(nullptr) 即调用方（提交操作）未传——整段 constexpr 跳过；
  // 删除操作传入它，在 commit 前后各探一次目标是否存在。
  constexpr bool kHasExists = !std::is_same_v<ExistsFn, decltype(nullptr)>;

  bool canceled = false;
  QList<CommitReportRow> allRows;
  for (int i = 0; i < total; ++i) {
    if (progress) {
      progress->setValue(i);
      if (progress->wasCanceled()) {
        canceled = true;
        break;
      }
      const QString d = displayOf(targets[i]);
      progress->setLabelText(QString("[%1/%2] %3").arg(i + 1).arg(total).arg(d.size() > 80 ? ("…" + d.right(79)) : d));
    }
    std::optional<bool> existedBefore, existsAfter;
    if constexpr (kHasExists) existedBefore = existsFn(targets[i]);
    const auto res = commitOne(targets[i]);
    if constexpr (kHasExists) existsAfter = existsFn(targets[i]);

    CommitReportRow row;
    row.path = displayOf(targets[i]);
    row.existedBefore = existedBefore;
    row.existsAfter = existsAfter;
    // res 是统一的 WmiResult；commit 类的 Skipped / Failed 三档由 commitOutcome
    // 派生（WBEM_E_NOT_FOUND → Skipped；其它非 ok → Failed）。
    const auto outcome = commitOutcome(res);
    if (outcome == CommitOutcome::Ok) {
      row.category = I18n::tr("Succeeded");
      row.errorCode = QStringLiteral("-");
      row.reason = QStringLiteral("-");
    } else {
      row.category = outcome == CommitOutcome::Skipped ? I18n::tr("Skipped") : I18n::tr("Failed");
      row.errorCode = formatErrorCode(res.hresult, res.returnValue);
      // kHasExists（删除操作传了 existsFn）== 操作类型是 Deletion——HRESULT 含义
      // 在 commit / deletion 间不同，文案要分。
      row.reason = explainCommitFailure(res.hresult, res.returnValue, kHasExists);
    }
    allRows.append(std::move(row));
  }
  if (progress) {
    progress->setValue(total);
    progress->close();
  }
  const int untouched = canceled ? (total - static_cast<int>(allRows.size())) : 0;
  showCommitReport(parent, allRows, untouched);
}

}  // namespace uwf::ui
