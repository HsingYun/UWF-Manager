// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QString>

#include "presentation/viewmodels/diagnostics/DiagnosticReportViewModel.h"
#include "presentation/dialogs/InformationDialog.h"

namespace uwf::ui {

// Report keys remain in English; window controls use the interface language.
class SystemInformationDialog final : public InformationDialog {
  Q_OBJECT
 public:
  explicit SystemInformationDialog(presentation::DiagnosticReportViewModel model, QWidget* parent = nullptr);
};

}  // namespace uwf::ui
