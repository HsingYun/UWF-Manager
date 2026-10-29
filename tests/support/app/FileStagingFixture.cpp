// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "FileStagingFixture.h"

#include "windows/commit/WindowsCommitTraversalSource.h"

namespace uwf::test::application {

app::FileStagingCommitResult completeFileStaging(app::FileStagingCommitter& committer, const QList<app::FileStagingEntry>& entries) {
  auto operation = committer.beginCommit(committer.prepare(app::FileStagingCommitter::scan(committer.prepareScan(entries))));
  while (!operation.finished()) static_cast<void>(operation.advance());
  return operation.result();
}

uwf::app::wmi::WmiMethodOutput fileExclusionResult(const std::initializer_list<std::string_view> paths) {
  uwf::app::wmi::WmiMethodOutput result;
  auto& rows = result.arrays["ExcludedFiles"];
  rows.reserve(paths.size());
  for (const auto path : paths) rows.push_back({{"FileName", uwf::app::wmi::WmiValue::fromString(std::string(path))}});
  return result;
}

app::FileStagingCommitOperation beginFileStagingWithTemporaryCommitter(uwf::app::wmi::WmiOperations& wmi, const QList<app::FileStagingEntry>& entries) {
  app::FileStagingCommitter committer(wmi, uwf::windows::systemCommitTraversalSource());
  return committer.beginCommit(committer.prepare(app::FileStagingCommitter::scan(committer.prepareScan(entries))));
}

}  // namespace uwf::test::application
