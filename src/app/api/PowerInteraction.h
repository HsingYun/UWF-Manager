// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QString>
#include <cstddef>
#include <functional>
#include <optional>
#include <utility>
#include <variant>

#include "app/api/FileStagingTypes.h"

namespace uwf::app {

enum class PowerAction { Shutdown, Restart };

enum class PowerActionDialogMode { Safe, Direct };

enum class PowerStagingState { InProgress, Completed, Failed };
enum class PowerActionDialogOutcome { Canceled, Confirmed, DirectConfirmed, CanceledAfterStagingFailure, ContinuedAfterStagingFailure };

struct PowerStagingProgress {
  PowerStagingState state = PowerStagingState::Completed;
  std::size_t processedFiles = 0;
  std::size_t totalFiles = 0;
  QString currentPath;
  QString failureDetails;
  std::optional<FileStagingCommitResult> failureResult = std::nullopt;
};

struct PowerStagingNotRequired {};

struct PowerStagingCommit {
  std::size_t filesToCommit = 0;
  std::function<PowerStagingProgress()> advanceStaging;
};

struct PowerStagingFailure {
  QString details;
  std::optional<FileStagingCommitResult> result = std::nullopt;
};

using PowerStagingPreparation = std::variant<PowerStagingNotRequired, PowerStagingCommit, PowerStagingFailure>;

struct PowerActionDialogRequest {
  PowerActionDialogRequest(PowerAction requestedAction, std::function<std::optional<PowerStagingPreparation>()> preparation = {},
                           std::optional<PowerStagingFailure> completedFailure = std::nullopt,
                           PowerActionDialogMode requestedMode = PowerActionDialogMode::Safe)
      : action(requestedAction), mode(requestedMode), pollStagingPreparation(std::move(preparation)), completedStagingFailure(std::move(completedFailure)) {}

  PowerAction action;
  PowerActionDialogMode mode;
  // 返回 nullopt 表示仍在计算。回调首次在对话框进入事件循环后执行，确保
  // 文件系统扫描不会阻塞对话框的首次显示；返回结果后不再调用。
  std::function<std::optional<PowerStagingPreparation>()> pollStagingPreparation;
  // 另一个入口已经完成同一批暂存提交时，电源流程直接复用其最终失败结果，
  // 不再伪装成一次新的“计算”或额外等待最短进度展示时间。
  std::optional<PowerStagingFailure> completedStagingFailure;
};

struct PowerInteraction {
  std::function<PowerActionDialogOutcome(PowerActionDialogRequest)> confirm;
  std::function<void(const QString&, const QString&)> warning;
  std::function<QString(const FileStagingCommitResult&)> describeStagingResult;
};

}  // namespace uwf::app
