// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QString>

#include "presentation/diagnostics/DiagnosticReportViewModel.h"
#include "ui/dialogs/InformationDialog.h"

namespace uwf::ui {

// 只读诊断报告窗口。报告采用稳定的英文 key，便于用户直接复制到 issue，
// 而窗口标题、提示与按钮仍跟随界面语言。
class SystemInformationDialog final : public InformationDialog {
  Q_OBJECT
 public:
  explicit SystemInformationDialog(presentation::DiagnosticReportViewModel model, QWidget* parent = nullptr);
};

}  // namespace uwf::ui
