// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <type_traits>

#include "app/api/CommitInteraction.h"
#include "app/commit/CommitTarget.h"
namespace uwf::app {
// The request borrows the list and callbacks; execute it synchronously within their scope.
template <typename Target, typename DisplayFn, typename CommitFn, typename ExistsFn>
[[nodiscard]] CommitBatchRequest makeCommitBatchRequest(const QList<Target>& targets, DisplayFn& displayOf, CommitFn& commitOne, ExistsFn& existsFn) {
  constexpr auto operation = std::is_same_v<ExistsFn, decltype(nullptr)> ? CommitOperation::Persist : CommitOperation::DeleteAndPersist;
  return {
      static_cast<int>(targets.size()), operation, [&targets, &displayOf](const int index) { return displayOf(targets[index]); },
      [&targets, &commitOne, &existsFn](const int index, const QString& display) { return executeCommitTarget(targets[index], display, commitOne, existsFn); }};
}
}  // namespace uwf::app
