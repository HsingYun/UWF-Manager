// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <memory>

#include "presentation/configuration/ApplyPlanViewModel.h"
#include "presentation/configuration/ImportViewModel.h"
#include "presentation/diagnostics/LogViewModel.h"
#include "presentation/diagnostics/SystemInformationViewModel.h"
#include "presentation/enhanced/EnhancedModeViewModel.h"
#include "presentation/overlay/OverlayFilesViewModel.h"
#include "presentation/overlay/OverlayViewModel.h"
#include "presentation/registry/RegistryPickerViewModel.h"
#include "presentation/settings/UserConfigExportViewModel.h"
#include "presentation/shell/MainWindowViewModel.h"
#include "presentation/staging/FileStagingViewModel.h"
namespace uwf::app {
class FileStagingDraft;
class FileStagingConflictPolicy;
}  // namespace uwf::app
namespace uwf::presentation {
class ViewModelFactory {
 public:
  virtual ~ViewModelFactory() = default;
  virtual std::unique_ptr<MainWindowViewModel> mainWindow(app::ApplicationStateSource& source, app::UwfCapability capability,
                                                          std::shared_ptr<app::FileStagingDraft> staging) = 0;
  virtual std::shared_ptr<SystemInformationViewModel> systemInformation() = 0;
  virtual std::shared_ptr<ImportViewModel> importCommands() = 0;
  virtual std::shared_ptr<LogViewModel> log() = 0;
  virtual std::shared_ptr<UserConfigExportViewModel> userConfigExport() = 0;
  virtual std::shared_ptr<FileStagingViewModel> fileStaging(QString drive, app::FileStagingDraft& store,
                                                            app::FileStagingConflictPolicy* conflicts = nullptr) = 0;
  virtual std::shared_ptr<ApplyPlanViewModel> applyPlan(core::PendingChanges changes, const core::UwfSnapshot& snapshot,
                                                        std::shared_ptr<app::FileStagingDraft> staging = {}) = 0;
  virtual std::shared_ptr<EnhancedModeViewModel> enhancedMode(app::EnhancedModeActions& actions) = 0;
  virtual std::shared_ptr<OverlayViewModel> overlay() = 0;
  virtual std::shared_ptr<OverlayFilesViewModel> overlayFiles(const QString& drive) = 0;
  virtual std::shared_ptr<RegistryPickerViewModel> registryPicker(RegistryPickerViewModel::Mode mode) = 0;
};
}  // namespace uwf::presentation
