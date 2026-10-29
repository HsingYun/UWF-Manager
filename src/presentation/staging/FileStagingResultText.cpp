// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "FileStagingResultText.h"

#include <QStringList>
#include <algorithm>

#include "app/staging/FileStagingCommandResult.h"
#include "presentation/common/PresentationText.h"

namespace uwf::presentation {

QString fileStagingFailureReason(const app::FileStagingCommitFailure& failure) {
  switch (failure.kind) {
    case app::FileStagingCommitFailureKind::InvalidPath:
      return PresentationText::tr("Only absolute paths on local volumes can be staged.");
    case app::FileStagingCommitFailureKind::ReparsePoint:
      return PresentationText::tr("Reparse points are not supported.");
    case app::FileStagingCommitFailureKind::PathInspectionFailed:
      return failure.detail.isEmpty() ? PresentationText::tr("The staged path could not be inspected.") : failure.detail;
    case app::FileStagingCommitFailureKind::FileTypeChanged:
      return PresentationText::tr("The staged file now refers to a different path type.");
    case app::FileStagingCommitFailureKind::DirectoryTypeChanged:
      return PresentationText::tr("The staged folder now refers to a different path type.");
    case app::FileStagingCommitFailureKind::DirectoryEnumerationFailed:
      return failure.detail.isEmpty() ? PresentationText::tr("The staged folder could not be enumerated completely.") : failure.detail;
    case app::FileStagingCommitFailureKind::MissingDriveLetter:
      return failure.detail.isEmpty() ? PresentationText::tr("The path has no drive letter.") : failure.detail;
    case app::FileStagingCommitFailureKind::VolumeRoot:
      return PresentationText::tr("A volume root cannot be processed recursively.");
    case app::FileStagingCommitFailureKind::ProviderFailure:
      return failure.detail.isEmpty() ? PresentationText::tr("The UWF provider rejected the commit.") : failure.detail;
    case app::FileStagingCommitFailureKind::UnknownFailure:
      return PresentationText::tr("The commit failed with an unknown error.");
  }
  return PresentationText::tr("The commit failed with an unknown error.");
}

FileStagingResultText fileStagingResultText(const app::FileStagingCommitResult& result) {
  constexpr qsizetype kVisibleFailures = 8;
  FileStagingResultText text;
  text.summary = PresentationText::tr("Discovered files: %1 · Committed: %2 · Skipped files: %3 · Skipped entries: %4 · Failed: %5")
                     .arg(static_cast<qulonglong>(result.discoveredFiles))
                     .arg(static_cast<qulonglong>(result.committedFiles))
                     .arg(static_cast<qulonglong>(result.skippedFiles))
                     .arg(static_cast<qulonglong>(result.skippedEntries))
                     .arg(static_cast<qlonglong>(result.failures.size()));
  const qsizetype visible = std::min(result.failures.size(), kVisibleFailures);
  for (qsizetype index = 0; index < visible; ++index) {
    const auto& failure = result.failures[index];
    const QString target = failure.path.isEmpty() ? PresentationText::tr("File staging preparation") : failure.path;
    text.failures.append({target, fileStagingFailureReason(failure)});
  }
  if (result.failures.size() > visible) {
    text.remainingFailures =
        PresentationText::tr("%1 additional failure(s) were written to the log.").arg(static_cast<qlonglong>(result.failures.size() - visible));
  }
  return text;
}

QString renderFileStagingResult(const app::FileStagingCommitResult& result) {
  const auto view = fileStagingResultText(result);
  QStringList lines{view.summary};
  for (const auto& failure : view.failures) lines << failure.target << failure.reason;
  if (!view.remainingFailures.isEmpty()) lines << view.remainingFailures;
  return lines.join(u'\n');
}

app::ApplicationCommandResult toApplicationCommandResult(const app::FileStagingCommitResult& result) {
  return app::toApplicationCommandResult(result, result.succeeded() ? QString{} : renderFileStagingResult(result));
}

}  // namespace uwf::presentation
