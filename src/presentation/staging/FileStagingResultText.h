// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QString>

#include "app/api/FileStagingTypes.h"
#include "app/startup/ApplicationCommand.h"

namespace uwf::presentation {

struct FileStagingFailureText {
  QString target;
  QString reason;
};

struct FileStagingResultText {
  QString summary;
  QList<FileStagingFailureText> failures;
  QString remainingFailures;
};

[[nodiscard]] QString fileStagingFailureReason(const app::FileStagingCommitFailure& failure);
[[nodiscard]] FileStagingResultText fileStagingResultText(const app::FileStagingCommitResult& result);
[[nodiscard]] QString renderFileStagingResult(const app::FileStagingCommitResult& result);
[[nodiscard]] app::ApplicationCommandResult toApplicationCommandResult(const app::FileStagingCommitResult& result);

}  // namespace uwf::presentation
