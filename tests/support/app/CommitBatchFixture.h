// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <QList>

#include "application/commit/CommitTarget.h"
#include "application/commit/contracts/CommitBatchRequest.h"
namespace uwf::test {
// The request borrows the list and callbacks; execute it synchronously within their scope.
template <typename Target, typename DisplayFn, typename CommitFn, typename ExistsFn>
[[nodiscard]] app::CommitBatchRequest makeCommitBatchRequest(const app::CommitOperation operation, const QList<Target>& targets, DisplayFn& displayOf,
                                                             CommitFn& commitOne, ExistsFn& existsFn) {
  return {static_cast<int>(targets.size()), operation, [&targets, &displayOf](const int index) { return displayOf(targets[index]); },
          [&targets, &commitOne, &existsFn, operation](const int index, const QString& display) {
            return app::executeCommitTarget(targets[index], display, operation, commitOne, existsFn);
          }};
}
}  // namespace uwf::test
