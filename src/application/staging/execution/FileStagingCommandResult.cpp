// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "application/staging/execution/FileStagingCommandResult.h"

#include <utility>

namespace uwf::app {

FileStagingCommandResult toFileStagingCommandResult(const FileStagingCommitResult& result, QString failureDetail) {
  return {result.succeeded() ? FileStagingCommandOutcome::Succeeded : FileStagingCommandOutcome::CompletedWithFailures,
          result.discoveredFiles,
          result.committedFiles,
          result.skippedFiles,
          result.skippedEntries,
          static_cast<std::size_t>(result.failures.size()),
          result.succeeded() ? QString{} : std::move(failureDetail),
          result.pausedEntries,
          result.pausedFiles};
}

}  // namespace uwf::app
