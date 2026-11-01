// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include "application/commit/contracts/CommitPlan.h"
#include "application/commit/contracts/CommitTraversalSource.h"
namespace uwf::app {
[[nodiscard]] CommitPlan prepareCommit(const CommitRequest& request, const CommitTraversalSource& source, CommitPlan previous, std::stop_token stop,
                                       const CommitPreparationProgress& progress);
}  // namespace uwf::app
