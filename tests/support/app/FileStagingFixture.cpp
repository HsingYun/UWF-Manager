// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "FileStagingFixture.h"

namespace uwf::test::application {

app::FileStagingCommitResult completeFileStaging(app::FileStagingCommitter& committer, const QList<app::FileStagingEntry>& entries) {
  auto operation = committer.beginCommit(committer.prepare(app::FileStagingCommitter::scan(committer.prepareScan(entries))));
  while (!operation.finished()) static_cast<void>(operation.advance());
  return operation.result();
}

}  // namespace uwf::test::application
