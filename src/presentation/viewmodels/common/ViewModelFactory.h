// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <memory>

#include "application/state/contracts/ApplicationStateSource.h"
#include "presentation/viewmodels/configuration/ApplyPlanViewModel.h"
#include "presentation/viewmodels/configuration/ImportViewModel.h"
#include "presentation/viewmodels/configuration/OverlayMaintenanceViewModel.h"
#include "presentation/viewmodels/diagnostics/LogViewModel.h"
#include "presentation/viewmodels/diagnostics/SystemInformationViewModel.h"
#include "presentation/viewmodels/enhanced/EnhancedModeViewModel.h"
#include "presentation/viewmodels/overlay/OverlayFilesViewModel.h"
#include "presentation/viewmodels/overlay/OverlayViewModel.h"
#include "presentation/viewmodels/registry/RegistryPickerViewModel.h"
#include "presentation/viewmodels/settings/UserConfigExportViewModel.h"
#include "presentation/viewmodels/shell/MainWindowViewModel.h"
#include "presentation/viewmodels/staging/FileStagingViewModel.h"

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
  virtual std::shared_ptr<FileStagingViewModel> fileStaging(QString driveLetter, app::FileStagingDraft& draft,
                                                            app::FileStagingConflictPolicy* conflicts = nullptr, const app::FileStagingScope& scope = {}) = 0;
  std::shared_ptr<ApplyPlanViewModel> applyPlan(core::PendingChanges changes, const core::UwfSnapshot& snapshot,
                                                std::shared_ptr<app::FileStagingDraft> staging = {}) {
    return applyPlan({std::move(changes), {}}, snapshot, {}, std::move(staging));
  }
  virtual std::shared_ptr<ApplyPlanViewModel> applyPlan(app::ConfigurationChanges changes, const core::UwfSnapshot& snapshot,
                                                        const app::OverlayPolicySnapshot& policy, std::shared_ptr<app::FileStagingDraft> staging = {},
                                                        std::optional<app::OverlayMaintenanceSnapshot> maintenance = {}) = 0;
  virtual std::shared_ptr<EnhancedModeViewModel> enhancedMode(app::EnhancedModeActions& actions) = 0;
  virtual std::shared_ptr<OverlayMaintenanceViewModel> overlayMaintenance(std::shared_ptr<app::OverlayMaintenanceEditor> = {}) { return {}; }
  virtual std::shared_ptr<OverlayViewModel> overlay() = 0;
  virtual std::shared_ptr<OverlayFilesViewModel> overlayFiles(std::shared_ptr<VolumeConfigurationViewModel> volume, MainWindowViewModel* window = nullptr) = 0;
  virtual std::shared_ptr<RegistryPickerViewModel> registryPicker(RegistryPickerViewModel::Mode mode) = 0;
};
}  // namespace uwf::presentation
