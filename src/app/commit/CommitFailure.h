// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <cstdint>
namespace uwf::app {
enum class CommitFailureKind { None, GeneralFailure, NotFound, InvalidParameter, Other };
[[nodiscard]] CommitFailureKind classifyCommitFailure(int32_t hresult, uint32_t returnValue);
}  // namespace uwf::app
