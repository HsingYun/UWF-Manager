// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QString>
#include <cstddef>
#include <functional>
#include <optional>
#include <utility>
#include <variant>

#include "application/staging/contracts/FileStagingTypes.h"

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
  AutomaticCommitSummary automaticCommit{};
};

struct PowerStagingNotRequired {
  AutomaticCommitSummary automaticCommit{};
  AutomaticCommitProposal proposal{};
};

struct PowerStagingCommit {
  std::size_t filesToCommit = 0;
  std::function<PowerStagingProgress()> advanceStaging;
  AutomaticCommitSummary automaticCommit{};
  AutomaticCommitProposal proposal{};
};

struct PowerStagingFailure {
  QString details;
  std::optional<FileStagingCommitResult> result = std::nullopt;
};

using PowerStagingPreparation = std::variant<PowerStagingNotRequired, PowerStagingCommit, PowerStagingFailure>;

struct PowerActionDialogRequest {
  PowerActionDialogRequest(PowerAction requestedAction, std::function<std::optional<PowerStagingPreparation>()> preparation = {},
                           PowerActionDialogMode requestedMode = PowerActionDialogMode::Safe)
      : action(requestedAction), mode(requestedMode), pollStagingPreparation(std::move(preparation)) {}

  PowerAction action;
  PowerActionDialogMode mode;
  // Poll after the dialog enters its event loop; nullopt means preparation is pending.
  // Stop polling once a result is returned.
  std::function<std::optional<PowerStagingPreparation>()> pollStagingPreparation;
  // Run again after the user confirms. A changed proposal must be reviewed
  // again before it may authorize writes or the final power action.
  std::function<std::optional<PowerStagingPreparation>()> pollStagingRevalidation{};
};

}  // namespace uwf::app
