// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

// Diagnostic collection and configuration export are supplied by the caller.

#include <QString>
#include <functional>

#include "presentation/viewmodels/diagnostics/DiagnosticReportViewModel.h"
#include "presentation/viewmodels/settings/UserConfigExportViewModel.h"
#include "presentation/dialogs/InformationDialog.h"

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
