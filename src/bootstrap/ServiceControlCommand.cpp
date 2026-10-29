// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)

#include "ServiceControlCommand.h"

#include <QCoreApplication>
#include <QScopeGuard>
#include <cstdlib>
#include <stdexcept>

#include "app/enhanced/EnhancedModeManager.h"
#include "app/enhanced/EnhancedModeProtocol.h"
#include "app/startup/StartupOptions.h"
#include "shared/Log.h"
#include "ui/appearance/I18n.h"
#include "windows/service/WindowsEnhancedModeServiceControl.h"
#include "windows/uwf/SystemUwfStateReader.h"
#include "windows/wmi/WmiSession.h"

namespace uwf::bootstrap {

int runServiceControlCommand(int argc, char* argv[], const app::StartupMode mode) {
  QCoreApplication application(argc, argv);
  application.setApplicationName("UWF Manager");
  (void)uwf::ui::I18n::instance();
  windows::initializeWmiRuntime();
  const auto shutdownWmi = qScopeGuard([] { windows::shutdownWmiRuntime(); });
  const auto capability = windows::probeUwfCapability();
  auto serviceControl = std::make_shared<windows::WindowsEnhancedModeServiceControl>();
  app::EnhancedModeManager manager(serviceControl, windows::embeddedWmiSession(), capability);
  app::EnhancedModeChangeResult result;
  switch (mode) {
    case app::StartupMode::InstallService:
      result = manager.enable(uwf::ui::I18n::enhancedModeServiceDescription());
      break;
    case app::StartupMode::UninstallService:
      result = manager.disable();
      break;
    case app::StartupMode::Interactive:
    case app::StartupMode::Quiet:
    case app::StartupMode::CommitStage:
    case app::StartupMode::Service:
      throw std::logic_error("non-control startup mode reached the service-control dispatcher");
  }
  if (!result.persistenceWarning.isEmpty()) {
    UWF_LOG_W("service") << "enhanced mode changed with UWF persistence warning: error=" << result.persistenceWarning.toStdString();
  }
  const bool reachedTarget = mode == app::StartupMode::InstallService ? result.status.state == app::EnhancedModeState::Enabled
                                                                      : result.status.state == app::EnhancedModeState::Disabled;
  return reachedTarget ? EXIT_SUCCESS : EXIT_FAILURE;
}

}  // namespace uwf::bootstrap
