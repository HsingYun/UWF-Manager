// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "FileStagingCommandResult.h"

#include <utility>

namespace uwf::app {

ApplicationCommandResult toApplicationCommandResult(const FileStagingCommitResult& result, QString failureDetail) {
  return {result.succeeded() ? ApplicationCommandOutcome::Succeeded : ApplicationCommandOutcome::CompletedWithFailures,
          result.discoveredFiles,
          result.committedFiles,
          result.skippedFiles,
          result.skippedEntries,
          static_cast<std::size_t>(result.failures.size()),
          result.succeeded() ? QString{} : std::move(failureDetail)};
}

}  // namespace uwf::app
