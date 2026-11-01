// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)

#include "composition/entry/Application.h"

// Shell API declarations require the Win32 types first.
// clang-format off
#include <windows.h>
#include <shellapi.h>
// clang-format on

#include <QStringList>
#include <cstdlib>
#include <exception>
#include <memory>
#include <system_error>
#include <utility>

#include "application/startup/StartupDispatch.h"
#include "application/startup/StartupOptions.h"
#include "composition/console/ConsoleApplication.h"
#include "composition/desktop/DesktopApplication.h"
#include "composition/service/EnhancedModeServiceComposition.h"
#include "foundation/logging/Log.h"
#include "infrastructure/windows/console/ConsoleSession.h"
#include "infrastructure/windows/process/CrashHandler.h"
#include "infrastructure/windows/process/UiElevation.h"

namespace uwf::composition {

namespace {

void restrictDllSearchPath() {
  // Release builds restrict static imports with /DEPENDENTLOADFLAG:0x800.
  // Restrict subsequent default DLL searches to System32; packaged builds are self-contained.
  if (!SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_SYSTEM32)) {
    throw std::system_error(static_cast<int>(GetLastError()), std::system_category(), "restrict process DLL search path");
  }
}

QStringList nativeStartupArguments() {
  int count = 0;
  const auto release = [](wchar_t** arguments) { LocalFree(arguments); };
  const std::unique_ptr<wchar_t*, decltype(release)> native(CommandLineToArgvW(GetCommandLineW(), &count), release);
  if (!native) throw std::system_error(static_cast<int>(GetLastError()), std::system_category(), "Read native command-line arguments");
  QStringList arguments;
  arguments.reserve(count);
  for (int index = 0; index < count; ++index) {
    auto value = QString::fromWCharArray(native.get()[index]);
    arguments.append(std::move(value));
  }
  return arguments;
}

}  // namespace

int runApplication(int argc, char* argv[]) {
  // Report classification failures through the console without constructing a window.
  bool consoleErrors = true;
  try {
    const auto arguments = nativeStartupArguments();
    consoleErrors = !app::startupControlMode(arguments).has_value();
    const auto options = app::resolveStartupOptions(arguments, &windows::hasConsoleContext);
    consoleErrors = options.mode == app::StartupMode::Console;
    restrictDllSearchPath();
    return app::dispatchStartup(options, {.isElevated = &windows::currentProcessElevatedForUi,
                                          .elevateUi = [&] { return windows::relaunchUiElevated(options.mode == app::StartupMode::Quiet); },
                                          .desktop =
                                              [&] {
                                                windows::CrashHandler::install();
                                                return runDesktopApplication(argc, argv, options);
                                              },
                                          .service = &runEnhancedModeService,
                                          .console = [&] { return runConsoleApplication(argc, argv, arguments); }});
  } catch (const std::exception& error) {
    UWF_LOG_E("main") << "fatal application error: error=" << error.what();
    if (consoleErrors) return reportConsoleStartupFailure(QString::fromUtf8(error.what()));
  } catch (...) {
    UWF_LOG_E("main") << "fatal application error: error=non-standard-exception";
    if (consoleErrors) return reportConsoleStartupFailure(QStringLiteral("Application startup failed with an unknown error"));
  }
  return EXIT_FAILURE;
}

}  // namespace uwf::composition
