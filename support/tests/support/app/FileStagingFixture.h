// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include "application/staging/execution/FileStagingCommitter.h"

namespace uwf::test::application {
app::FileStagingCommitResult completeFileStaging(app::FileStagingCommitter& committer, const QList<app::FileStagingEntry>& entries);

}  // namespace uwf::test::application
