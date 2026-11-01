// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QStringList>
#include <array>
#include <cstddef>
#include <optional>
#include <utility>

#include "application/staging/contracts/AutomaticCommitDisposition.h"
#include "presentation/viewmodels/common/PresentationText.h"
#include "presentation/viewmodels/common/RuntimeWriteText.h"

namespace uwf::presentation {
[[nodiscard]] inline QString automaticCommitDescription(const app::AutomaticCommitDisposition disposition) {
  switch (disposition) {
    case app::AutomaticCommitDisposition::SkipReadOnlyMedia:
      return runtimeWriteRestrictionText(app::RuntimeWriteRestriction::ReadOnlyMediaActive);
    case app::AutomaticCommitDisposition::SkipRetainedOverlay:
      return PresentationText::tr("Persistent overlay data will be retained at the next startup. Automatic commits are paused.");
    case app::AutomaticCommitDisposition::SkipHormActive:
      return PresentationText::tr("Automatic file commits are unavailable while HORM is active.");
    case app::AutomaticCommitDisposition::SkipHormTransition:
      return PresentationText::tr("Automatic file commits are paused until restart after a HORM mode change.");
    case app::AutomaticCommitDisposition::SkipFullCommit:
      return PresentationText::tr("Automatic file commits are unavailable after FullCommit.");
    case app::AutomaticCommitDisposition::Unavailable:
      return runtimeWriteRestrictionText(app::RuntimeWriteRestriction::StateUnavailable);
    case app::AutomaticCommitDisposition::CheckDurability:
    case app::AutomaticCommitDisposition::AlreadyPersistent:
      return {};
  }
  return {};
}

// A presentation projection of the shared result, with no independent mode
// state. Keep all reasons: an aggregate can contain paused, eligible and unknown
// targets together. A known execution count refines promises, not eligibility.
struct AutomaticCommitPresentation {
  QString status;
  QString description;
  bool unavailable = false;
  bool paused = false;
  bool partiallyPaused = false;
  bool alreadyPersistent = false;
};

[[nodiscard]] inline AutomaticCommitPresentation automaticCommitPresentation(const app::AutomaticCommitSummary& summary,
                                                                             const std::optional<std::size_t> pendingFiles = {}) {
  using D = app::AutomaticCommitDisposition;
  using S = app::AutomaticCommitSummary;
  AutomaticCommitPresentation result;
  result.unavailable = summary.unavailable;
  result.paused = summary.paused();
  result.partiallyPaused = summary.partiallyPaused();
  result.alreadyPersistent = summary.alreadyPersistent && !summary.commitRequired && !result.paused && !result.unavailable;
  result.status = result.unavailable         ? PresentationText::tr("Unavailable")
                  : result.partiallyPaused   ? PresentationText::tr("Partially paused")
                  : result.paused            ? PresentationText::tr("Paused")
                  : result.alreadyPersistent ? PresentationText::tr("No files to commit")
                                             : PresentationText::tr("Ready");
  QStringList reasons;
  if (result.unavailable) reasons.append(automaticCommitDescription(D::Unavailable));
  if (result.partiallyPaused && !result.unavailable && (!pendingFiles || *pendingFiles > 0))
    reasons.append(PresentationText::tr("Only eligible staged files will be committed."));
  constexpr std::array pauses{std::pair{&S::hormPaused, D::SkipHormActive}, std::pair{&S::hormTransitionPaused, D::SkipHormTransition},
                              std::pair{&S::fullCommitPaused, D::SkipFullCommit}, std::pair{&S::readOnlyMediaPaused, D::SkipReadOnlyMedia},
                              std::pair{&S::retainedOverlayPaused, D::SkipRetainedOverlay}};
  for (const auto& [present, disposition] : pauses)
    if (summary.*present) reasons.append(automaticCommitDescription(disposition));
  if (summary.unsupported() || summary.hormTransitionPaused) reasons.append(PresentationText::tr("Skipped staged files are not committed by this operation."));
  if (reasons.isEmpty())
    reasons.append(result.alreadyPersistent ? PresentationText::tr("Staged files are already on unprotected storage. No automatic commit is needed.")
                                            : PresentationText::tr("Commits staged files before normal shutdown or restart."));
  result.description = reasons.join(u'\n');
  return result;
}
}  // namespace uwf::presentation
