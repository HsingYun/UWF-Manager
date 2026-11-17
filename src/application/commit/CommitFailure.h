// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include "application/operations/contracts/OperationError.h"

namespace uwf::app {
enum class CommitFailureKind { None, GeneralFailure, NotFound, InvalidParameter, Other };
[[nodiscard]] CommitFailureKind classifyCommitFailure(OperationFailureKind kind);
}  // namespace uwf::app
