// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QStringList>
#include <functional>
#include <optional>
#include <stdexcept>

#include "application/startup/ApplicationCommand.h"

namespace uwf::app {

enum class StartupMode {
  Interactive,
  Quiet,
  Service,
  Console,
};

struct StartupOptions {
  StartupMode mode = StartupMode::Interactive;

  [[nodiscard]] ApplicationCommandKind initialCommand() const;
};

class StartupOptionsError final : public std::invalid_argument {
 public:
  using std::invalid_argument::invalid_argument;
};

// Arguments include the executable name. Control entries reject extra operands;
// other operands use CLI parsing. Without operands, console context selects the entry.
[[nodiscard]] StartupOptions parseStartupOptions(const QStringList& arguments, bool consoleContext = false);
// Only the first user token can select a control entry. Later values that happen
// to equal --quiet/--service remain ordinary CLI data.
[[nodiscard]] std::optional<StartupMode> startupControlMode(const QStringList& arguments);
// Context probing can attach a console, so it is never called for explicit
// arguments, including the service and tray entry points. Malformed UTF-16 is
// rejected before any context probe; error presentation is selected by the host
// from startupControlMode on the original, unmodified arguments.
[[nodiscard]] StartupOptions resolveStartupOptions(const QStringList& arguments, const std::function<bool()>& consoleContext);

}  // namespace uwf::app
