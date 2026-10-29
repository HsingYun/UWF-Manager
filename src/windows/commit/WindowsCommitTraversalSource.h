// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include "app/api/CommitPreparation.h"
namespace uwf::windows {
[[nodiscard]] std::shared_ptr<const app::CommitTraversalSource> systemCommitTraversalSource();
}  // namespace uwf::windows
