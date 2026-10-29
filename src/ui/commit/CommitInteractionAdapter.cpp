// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "CommitInteractionAdapter.h"

#include <QApplication>
#include <QObjectCleanupHandler>
#include <QPointer>
#include <QTimer>

#include "CommitOperationDialog.h"
#include "presentation/commit/CommitOperationViewModel.h"
#include "ui/dialogs/Dialogs.h"

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
        // Show first. No file system, registry, WMI or traversal call is performed
        // in the original input handler before the window has been presented.
        QTimer::singleShot(0, &model, &presentation::CommitOperationViewModel::start);
        dialog->exec();
      }};
}
}  // namespace uwf::ui
