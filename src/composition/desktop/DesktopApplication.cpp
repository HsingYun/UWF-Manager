// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)

#include "composition/desktop/DesktopApplication.h"

#include <QApplication>
#include <QCoreApplication>
#include <QIcon>
#include <QScopeGuard>
#include <cstdlib>
#include <exception>
#include <optional>
#include <stdexcept>
#include <utility>

#include "application/settings/UserConfig.h"
#include "application/startup/StartupOptions.h"
#include "composition/desktop/WindowsDesktopComposition.h"
#include "composition/uwf/SystemCapability.h"
#include "foundation/logging/Log.h"
#include "infrastructure/windows/compatibility/SystemCheck.h"
#include "infrastructure/windows/management/wmi/WmiRuntime.h"
#include "infrastructure/windows/process/SecureSingleInstance.h"
#include "infrastructure/windows/settings/IniUserConfig.h"
#include "presentation/appearance/I18n.h"
#include "presentation/appearance/ThemeManager.h"
#include "presentation/views/shell/MainWindow.h"

namespace uwf::composition {

namespace {

std::optional<int> acquireSingleInstance(windows::SecureSingleInstance& singleInstance, const app::StartupOptions& options) {
  // Forward activation before system checks and UI initialization.
  const auto acquireResult = singleInstance.acquire(options.initialCommand());
  if (acquireResult == windows::SecureSingleInstance::AcquireResult::ForwardedExisting) {
    UWF_LOG_I("main") << "startup command forwarded to existing instance: command=" << static_cast<int>(options.initialCommand());
    return EXIT_SUCCESS;
  }
  if (acquireResult == windows::SecureSingleInstance::AcquireResult::Unprotected) {
    UWF_LOG_W("main") << "single-instance server unavailable: error=" << singleInstance.errorString().toStdString();
  }
  return std::nullopt;
}

void initializeUserInterface(const app::UserConfig& config) {
  uwf::ui::I18n::instance().applyStartupLanguage(config.language);

  // Apply preferences before window creation to avoid a theme/language flash.
  // Runtime menu changes do not rewrite the user-maintained configuration.
  uwf::ui::ThemeManager::instance().applyStartupTheme(config.theme);
}

windows::SystemCheckResult checkRuntimeEnvironment() {
  // Unsupported Windows editions enter compatibility mode rather than blocking startup.
  const auto check = windows::runSystemChecks();
  switch (check.status) {
    case windows::CheckStatus::UnsupportedSystem:
      UWF_LOG_W("main") << "unsupported Windows edition: mode=compatibility product=" << check.productName << " edition=" << check.editionId;
      break;
    case windows::CheckStatus::Ok:
      UWF_LOG_I("main") << "system check completed: product=" << check.productName << " edition=" << check.editionId;
      break;
  }

  if (!windows::isElevated()) {
    UWF_LOG_I("main") << "process is not elevated: mode=read-only";
  }
  return check;
}

int runMainWindow(QApplication& application, windows::SecureSingleInstance& singleInstance, const app::StartupOptions& options,
                  uwf::ui::MainWindowStartup startup) {
  const auto services = uwf::composition::systemMainWindowServices(startup.uwfCapability);
  uwf::ui::MainWindow window(services, std::move(startup));

  QObject::connect(&singleInstance, &windows::SecureSingleInstance::activationRequested, &window, &uwf::ui::MainWindow::raiseToFront);
  singleInstance.enableCommandNotifications();

  switch (options.mode) {
    case app::StartupMode::Interactive:
      window.show();
      break;
    case app::StartupMode::Quiet:
      UWF_LOG_I("main") << "startup mode selected: mode=quiet presentation=system-tray";
      window.startInTray();
      break;
    case app::StartupMode::Service:
    case app::StartupMode::Console:
      throw std::logic_error("non-UI startup mode reached the desktop dispatcher");
  }
  return application.exec();
}

}  // namespace

int runDesktopApplication(int argc, char* argv[], const app::StartupOptions& options) {
  QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
  QApplication application(argc, argv);
  application.setApplicationName("UWF Manager");
  application.setOrganizationName("UWF");
  application.setWindowIcon(QIcon(":/icons/app.svg"));
  windows::SecureSingleInstance singleInstance;
  if (const auto forwardedExitCode = acquireSingleInstance(singleInstance, options)) return *forwardedExitCode;

  const auto config = windows::readIniUserConfig(QCoreApplication::applicationDirPath());
  initializeUserInterface(config);
  UWF_LOG_I("main") << "application started: pid=" << QCoreApplication::applicationPid();
  const auto check = checkRuntimeEnvironment();
  // Release this thread's lazy WMI context after the window is destroyed.
  const auto shutdownWmi = qScopeGuard([] { windows::shutdownWmiRuntime(); });
  // Edition classification does not replace backend capability probing.
  const auto uwfCapability = composition::probeUwfCapability();
  if (uwfCapability == app::UwfCapability::Unavailable) {
    UWF_LOG_W("main") << "UWF unavailable: no backend provides filter state";
  }
  // Reconcile staging durability once at startup, outside UI refresh.
  // Failures leave the UI available for inspection and repair.
  if (windows::isElevated()) {
    try {
      systemFileStagingStore(uwfCapability)->reconcileDurability();
    } catch (const std::exception& error) {
      UWF_LOG_W("staging") << "file staging durability maintenance failed: error=" << error.what();
    } catch (...) {
      UWF_LOG_W("staging") << "file staging durability maintenance failed: error=non-standard-exception";
    }
  }
  return runMainWindow(
      application, singleInstance, options,
      {.uwfCapability = uwfCapability, .compatibilityMode = check.status == windows::CheckStatus::UnsupportedSystem, .hubVisible = config.hubVisible});
}

}  // namespace uwf::composition
