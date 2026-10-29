// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

// "About UWF Manager" 对话框：logo + 标题 + 版本号 + GPL / UWF 说明，并提供
// 系统信息及导出配置入口。诊断报告和配置导出动作由调用方注入。

#include <QString>
#include <functional>

#include "presentation/diagnostics/DiagnosticReportViewModel.h"
#include "presentation/settings/UserConfigExportViewModel.h"
#include "ui/dialogs/InformationDialog.h"

class QPushButton;

namespace uwf::ui {

class AboutDialog : public InformationDialog {
  Q_OBJECT
 public:
  AboutDialog(std::function<presentation::DiagnosticReportViewModel()> systemInformationProvider, QWidget* parent = nullptr);
  void setConfigurationExporter(std::function<presentation::UserConfigExportViewModel::Result()> exporter);

 private:
  void buildUi();
  void openSystemInformation();
  void saveConfiguration();

  std::function<presentation::DiagnosticReportViewModel()> m_systemInformationProvider;
  std::function<presentation::UserConfigExportViewModel::Result()> m_configurationExporter;
  QPushButton* m_saveConfigButton = nullptr;
};

}  // namespace uwf::ui
