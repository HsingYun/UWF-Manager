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
#include <cstdint>
#include <exception>
#include <memory>
#include <optional>
#include <type_traits>
#include <utility>

#include "../util/Log.h"
#include "../uwf/wmi/WmiError.h"
#include "../uwf/wmi/WmiException.h"
#include "CommitReportDialog.h"
#include "I18n.h"

class QWidget;

namespace uwf::ui {

namespace detail {

struct ExistenceObservation {
  std::optional<bool> value;
  QString failure;
};

template <typename ExistsFn, typename Target>
[[nodiscard]] ExistenceObservation observeExistence(ExistsFn& existsFn, const Target& target) {
  try {
    return {.value = existsFn(target), .failure = {}};
  } catch (const std::exception& error) {
    return {.value = std::nullopt, .failure = QString::fromUtf8(error.what())};
  } catch (...) {
    return {.value = std::nullopt, .failure = I18n::tr("Unknown error while reading the target state.")};
  }
}

}  // namespace detail

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
    const QString display = displayOf(targets[i]);
    std::optional<bool> existedBefore;
    std::optional<bool> existsAfter;
    if constexpr (kHasExists) {
      const auto observation = detail::observeExistence(existsFn, targets[i]);
      existedBefore = observation.value;
      if (!observation.value) {
        CommitReportRow row;
        row.path = display;
        row.existedBefore = std::nullopt;
        row.existsAfter = std::nullopt;
        row.category = I18n::tr("Failed");
        row.errorCode = QStringLiteral("-");
        row.reason = I18n::tr("The target state could not be read before the operation: %1").arg(observation.failure);
        UWF_LOG_W("commit") << "operation skipped: reason=target-state-unavailable target=" << display.toStdString()
                            << " error=" << observation.failure.toStdString();
        allRows.append(std::move(row));
        continue;
      }
      if (!*observation.value) {
        CommitReportRow row;
        row.path = display;
        row.existedBefore = false;
        row.existsAfter = false;
        row.category = I18n::tr("Skipped");
        row.errorCode = QStringLiteral("-");
        row.reason = I18n::tr("The target no longer exists, so there is nothing to delete.");
        allRows.append(std::move(row));
        continue;
      }
    }
    int32_t hresult = 0;
    uint32_t returnValue = 0;
    QString failureDetail;
    QString authoritativeFailure;
    bool succeeded = false;
    bool mayHaveTakenEffect = false;
    try {
      commitOne(targets[i]);
      succeeded = true;
    } catch (const WmiProviderError& error) {
      returnValue = error.returnValue();
      failureDetail = QString::fromUtf8(error.what());
    } catch (const WmiWriteOutcomeError& error) {
      if (error.code().category() == wmiErrorCategory()) hresult = static_cast<int32_t>(error.code().value());
      mayHaveTakenEffect = true;
      failureDetail = QString::fromUtf8(error.what());
    } catch (const WmiException& error) {
      if (error.code().category() == wmiErrorCategory()) hresult = static_cast<int32_t>(error.code().value());
      failureDetail = QString::fromUtf8(error.what());
    } catch (const std::exception& error) {
      failureDetail = QString::fromUtf8(error.what());
    } catch (...) {
      failureDetail = I18n::tr("The operation failed with an unknown error.");
    }
    if constexpr (kHasExists) {
      const auto observation = detail::observeExistence(existsFn, targets[i]);
      existsAfter = observation.value;
      if (!observation.value) {
        if (succeeded || mayHaveTakenEffect) {
          succeeded = false;
          authoritativeFailure = I18n::tr("The operation result could not be confirmed because the target state reread failed: %1").arg(observation.failure);
        }
      } else if (mayHaveTakenEffect && !*observation.value) {
        // 连接在调用过程中断开时，调用是否落地不可知；目标已不存在是删除
        // 业务真正关心的权威终态，因此它可以确认“不确定”，但绝不覆盖
        // provider 明确拒绝的结果。
        succeeded = true;
      } else if (succeeded && *observation.value) {
        succeeded = false;
        authoritativeFailure = I18n::tr("The provider accepted the deletion, but the target still exists after the authoritative reread.");
      }
    }

    CommitReportRow row;
    row.path = display;
    row.existedBefore = existedBefore;
    row.existsAfter = existsAfter;
    if (succeeded) {
      row.category = I18n::tr("Succeeded");
      row.errorCode = QStringLiteral("-");
      row.reason = QStringLiteral("-");
    } else {
      const int32_t code = hresult != 0 ? hresult : static_cast<int32_t>(returnValue);
      const bool skipped = WmiError(code).code() == WmiErrorCode::NotFound;
      row.category = skipped ? I18n::tr("Skipped") : I18n::tr("Failed");
      row.errorCode = code == 0 ? QStringLiteral("-") : formatErrorCode(hresult, returnValue);
      constexpr CommitOperation kOperation = kHasExists ? CommitOperation::DeleteAndPersist : CommitOperation::Persist;
      row.reason = !authoritativeFailure.isEmpty() ? authoritativeFailure
                                                   : (code == 0 ? failureDetail : explainCommitFailure(hresult, returnValue, kOperation));
      const QString diagnostic = !authoritativeFailure.isEmpty() ? authoritativeFailure : failureDetail;
      UWF_LOG_W("commit") << "operation failed: target=" << row.path.toStdString() << " error=" << diagnostic.toStdString();
    }
    allRows.append(std::move(row));
  }
  if (progress) {
    progress->setValue(total);
    progress->close();
  }
  const int untouched = canceled ? (total - static_cast<int>(allRows.size())) : 0;
  constexpr CommitOperation kOperation = kHasExists ? CommitOperation::DeleteAndPersist : CommitOperation::Persist;
  showCommitReport(parent, allRows, kOperation, untouched);
}

}  // namespace uwf::ui
