// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "PowerInteractionAdapter.h"

#include <utility>

#include "PowerConfirmDialog.h"
#include "presentation/staging/FileStagingResultText.h"
#include "ui/dialogs/Dialogs.h"

namespace uwf::ui {

app::PowerInteraction powerInteraction(QWidget* dialogParent) {
  return app::PowerInteraction{[dialogParent](app::PowerActionDialogRequest request) { return runPowerActionDialog(dialogParent, std::move(request)); },
                               [dialogParent](const QString& title, const QString& message) { dialogs::warning(dialogParent, title, message); },
                               [](const app::FileStagingCommitResult& result) { return presentation::renderFileStagingResult(result); }};
}

}  // namespace uwf::ui
