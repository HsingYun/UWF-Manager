// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include "application/staging/contracts/FileStagingTypes.h"

namespace uwf::app {
class CommitTraversalSource;
// Deduplicate selected objects, retaining the first spelling unless a later
// selection changes its kind. Selected directories cover descendants. Normalize
// staging paths, not exclusion patterns; raw storage is retained separately.
[[nodiscard]] QList<FileStagingEntry> normalizeFileStagingEntries(const QList<FileStagingEntry>& entries, const CommitTraversalSource* source = nullptr);
}  // namespace uwf::app
