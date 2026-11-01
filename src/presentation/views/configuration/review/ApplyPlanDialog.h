// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

// Displays a frozen configuration plan and delegates execution to ApplyPlanViewModel.
// Reconciliation prevents replay until the host has read a complete new snapshot.

#include <QPointer>
#include <memory>

#include "presentation/dialogs/PanelDialog.h"
#include "presentation/viewmodels/configuration/ApplyPlanViewModel.h"

namespace uwf::ui {

namespace dialogs {
class FileDialogProvider;
}

class ApplyPlanDialog : public PanelDialog {
  Q_OBJECT
 public:
  ApplyPlanDialog(std::shared_ptr<presentation::ApplyPlanViewModel> model, dialogs::FileDialogProvider& fileDialogs, QWidget* parent = nullptr);

 signals:
  // The host blocks editing and replay until a complete replacement snapshot is read.
  void reconciliationRequired();
  // The host owns restart confirmation and execution.
  void safeRestartRequested();
  void directRestartRequested();

 private:
  dialogs::FileDialogProvider& m_fileDialogs;
  std::shared_ptr<presentation::ApplyPlanViewModel> m_viewModel;
  QPointer<QDialog> m_applyConfirmation;
};

}  // namespace uwf::ui
