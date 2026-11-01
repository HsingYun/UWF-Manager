// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "application/commit/CommitFailure.h"
namespace uwf::app {
CommitFailureKind classifyCommitFailure(const OperationFailureKind kind) {
  switch (kind) {
    case OperationFailureKind::Failed:
      return CommitFailureKind::GeneralFailure;
    case OperationFailureKind::NotFound:
      return CommitFailureKind::NotFound;
    case OperationFailureKind::InvalidArgument:
      return CommitFailureKind::InvalidParameter;
    default:
      return CommitFailureKind::Other;
  }
}
}  // namespace uwf::app
