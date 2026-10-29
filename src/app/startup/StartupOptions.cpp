// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "StartupOptions.h"

namespace uwf::app {

ApplicationCommandKind StartupOptions::initialCommand() const {
  switch (mode) {
    case StartupMode::Interactive:
      return ApplicationCommandKind::Activate;
    case StartupMode::Quiet:
      return ApplicationCommandKind::EnsureRunning;
    case StartupMode::CommitStage:
      return ApplicationCommandKind::CommitStage;
    case StartupMode::Service:
    case StartupMode::InstallService:
    case StartupMode::UninstallService:
      throw StartupOptionsError("service startup modes do not use the UI single-instance command channel");
  }
  throw StartupOptionsError("startup mode is invalid");
}

StartupOptions parseStartupOptions(const QStringList& arguments) {
  StartupOptions options;
  bool modeSpecified = false;
  for (qsizetype index = 1; index < arguments.size(); ++index) {
    const QString& argument = arguments[index];
    StartupMode mode;
    if (argument == QStringLiteral("--quiet"))
      mode = StartupMode::Quiet;
    else if (argument == QStringLiteral("--commit-stage"))
      mode = StartupMode::CommitStage;
    else if (argument == QStringLiteral("--service"))
      mode = StartupMode::Service;
    else if (argument == QStringLiteral("--install"))
      mode = StartupMode::InstallService;
    else if (argument == QStringLiteral("--uninstall"))
      mode = StartupMode::UninstallService;
    else
      throw StartupOptionsError(("unknown startup argument: " + argument).toStdString());

    if (modeSpecified) throw StartupOptionsError("startup modes are mutually exclusive");
    options.mode = mode;
    modeSpecified = true;
  }
  return options;
}

}  // namespace uwf::app
