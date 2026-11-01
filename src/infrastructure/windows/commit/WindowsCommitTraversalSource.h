// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include "application/commit/contracts/CommitPlan.h"
#include "application/commit/contracts/CommitTraversalSource.h"
namespace uwf::windows {
[[nodiscard]] std::shared_ptr<const app::CommitTraversalSource> systemCommitTraversalSource();
}  // namespace uwf::windows
