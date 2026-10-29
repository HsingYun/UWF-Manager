// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

// Displays a frozen configuration plan and delegates execution to ApplyPlanViewModel.
// Reconciliation prevents replay until the host has read a complete new snapshot.

#include <memory>

#include "presentation/configuration/ApplyPlanViewModel.h"
#include "ui/dialogs/PanelDialog.h"

namespace uwf::ui {

namespace dialogs {
class FileDialogProvider;
}

class ApplyPlanDialog : public PanelDialog {
  Q_OBJECT
 public:
  ApplyPlanDialog(std::shared_ptr<presentation::ApplyPlanViewModel> model, dialogs::FileDialogProvider& fileDialogs, QWidget* parent = nullptr);

 signals:
  // 当对话框内的基线可能已过期时发出。宿主必须在新快照完整读取
  // 成功前阻止继续编辑或重放这批变更。
  void reconciliationRequired();
  // 应用开始后出现的重启按钮只负责发出请求，确认与执行统一交给宿主。
  void safeRestartRequested();
  void directRestartRequested();

 private:
  dialogs::FileDialogProvider& m_fileDialogs;
  std::shared_ptr<presentation::ApplyPlanViewModel> m_viewModel;
};

}  // namespace uwf::ui
