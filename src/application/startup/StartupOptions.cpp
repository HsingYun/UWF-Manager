// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "application/startup/StartupOptions.h"

#include <QStringView>

#include "foundation/text/Utf8.h"

namespace uwf::app {
namespace {
void validateUnicode(const QStringList& arguments) {
  for (const auto& argument : arguments)
    if (!isValidUtf16(QStringView(argument))) throw StartupOptionsError("The native command line contains invalid UTF-16");
}
}  // namespace

ApplicationCommandKind StartupOptions::initialCommand() const {
  switch (mode) {
    case StartupMode::Interactive:
      return ApplicationCommandKind::Activate;
    case StartupMode::Quiet:
      return ApplicationCommandKind::EnsureRunning;
    case StartupMode::Service:
    case StartupMode::Console:
      throw StartupOptionsError("non-UI startup modes do not use the UI single-instance command channel");
  }
  throw StartupOptionsError("startup mode is invalid");
}

std::optional<StartupMode> startupControlMode(const QStringList& arguments) {
  if (arguments.size() <= 1) return {};
  if (arguments[1] == QStringLiteral("--quiet")) return StartupMode::Quiet;
  if (arguments[1] == QStringLiteral("--service")) return StartupMode::Service;
  return {};
}

StartupOptions parseStartupOptions(const QStringList& arguments, const bool consoleContext) {
  if (const auto mode = startupControlMode(arguments)) {
    if (arguments.size() != 2) throw StartupOptionsError("The --quiet and --service entries do not accept additional arguments");
    return {*mode};
  }
  if (arguments.size() <= 1) return {consoleContext ? StartupMode::Console : StartupMode::Interactive};
  return {StartupMode::Console};
}

StartupOptions resolveStartupOptions(const QStringList& arguments, const std::function<bool()>& consoleContext) {
  validateUnicode(arguments);
  if (arguments.size() > 1) return parseStartupOptions(arguments);
  return parseStartupOptions(arguments, consoleContext && consoleContext());
}

}  // namespace uwf::app
