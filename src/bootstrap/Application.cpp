// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)

#include "Application.h"

#include <windows.h>

#include <QStringList>
#include <cstdlib>
#include <exception>
#include <stdexcept>
#include <system_error>

#include "DesktopApplication.h"
#include "ServiceControlCommand.h"
#include "app/startup/StartupOptions.h"
#include "service/EnhancedModeServiceRuntime.h"
#include "shared/Log.h"
#include "windows/process/CrashHandler.h"

namespace uwf::bootstrap {

namespace {

void restrictDllSearchPath() {
  // Release 构建还通过 /DEPENDENTLOADFLAG:0x800 约束进入 main 前的静态
  // 导入；这里负责之后没有显式 LOAD_LIBRARY_SEARCH 标志的动态加载。
  // 正式包是单文件程序，不需要从应用目录、当前目录或 PATH 查找 DLL。
  if (!SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_SYSTEM32)) {
    throw std::system_error(static_cast<int>(GetLastError()), std::system_category(), "restrict process DLL search path");
  }
}

QStringList startupArguments(const int argc, char* argv[]) {
  QStringList arguments;
  arguments.reserve(argc);
  for (int index = 0; index < argc; ++index) arguments.append(QString::fromLocal8Bit(argv[index]));
  return arguments;
}

}  // namespace

int runApplication(int argc, char* argv[]) {
  try {
    restrictDllSearchPath();
    const auto options = app::parseStartupOptions(startupArguments(argc, argv));
    switch (options.mode) {
      case app::StartupMode::Service:
        return uwf::service::runEnhancedModeService();
      case app::StartupMode::InstallService:
      case app::StartupMode::UninstallService:
        return runServiceControlCommand(argc, argv, options.mode);
      case app::StartupMode::Interactive:
      case app::StartupMode::Quiet:
      case app::StartupMode::CommitStage:
        windows::CrashHandler::install();
        return runDesktopApplication(argc, argv, options);
    }
    throw std::logic_error("unknown startup mode");
  } catch (const std::exception& error) {
    UWF_LOG_E("main") << "fatal application error: error=" << error.what();
  } catch (...) {
    UWF_LOG_E("main") << "fatal application error: error=non-standard-exception";
  }
  return EXIT_FAILURE;
}

}  // namespace uwf::bootstrap
