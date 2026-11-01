// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "presentation/views/commit/CommitInteractionAdapter.h"

#include <QApplication>
#include <QObjectCleanupHandler>
#include <QPointer>
#include <QTimer>

#include "presentation/views/commit/CommitOperationDialog.h"
#include "presentation/viewmodels/commit/CommitOperationViewModel.h"
#include "presentation/dialogs/Dialogs.h"

namespace uwf::ui {
app::CommitInteraction commitInteraction(QWidget* parent, app::TextFileWriter writer, dialogs::FileDialogProvider& files) {
  return {
      [owner = QPointer<QWidget>(parent), writer = std::move(writer), &files](const app::CommitRequest& request, std::shared_ptr<app::CommitBackend> backend) {
        presentation::CommitOperationViewModel model(request, std::move(backend));
        QObjectCleanupHandler lifetime;
        QWidget* parent = owner;
        auto* active = QApplication::activeModalWidget();
        for (auto* ancestor = active; ancestor; ancestor = ancestor->parentWidget())
          if (ancestor == owner) {
            parent = active;
            break;
          }
        QPointer<CommitOperationDialog> dialog = new CommitOperationDialog(model, files, writer, parent);
        lifetime.add(dialog);
        // Preparation starts after the dialog enters its event loop.
        QTimer::singleShot(0, &model, &presentation::CommitOperationViewModel::start);
        dialog->exec();
      }};
}
}  // namespace uwf::ui
