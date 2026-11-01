// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "presentation/console/ConsoleDiagnostics.h"

#include <cstdint>
#include <format>
#include <limits>

#include "foundation/logging/Log.h"

namespace uwf::console {
namespace {
std::string_view effectName(const compat::Effect effect) noexcept {
  switch (effect) {
    case compat::Effect::None:
      return "None";
    case compat::Effect::Confirmed:
      return "Confirmed";
    case compat::Effect::Uncertain:
      return "Uncertain";
  }
  return "Unknown";
}

#if defined(UWF_DEBUG_LOGGING)
void logFailure(const std::string_view operation, const compat::Failure& failure) {
  LogStream log('D', "command");
  log << "event=failure";
  if (!operation.empty()) {
    log << " operation=";
    logQuoted(log, operation);
  }
  log << " effect=" << effectName(failure.effect) << " reason=";
  logQuoted(log, failure.message);
  for (const auto& diagnostic : failure.diagnostics) {
    if (!diagnostic.backend.empty()) {
      log << " diagnostic_backend=";
      logQuoted(log, diagnostic.backend);
    }
    if (!diagnostic.operation.empty()) {
      log << " diagnostic_operation=";
      logQuoted(log, diagnostic.operation);
    }
    if (diagnostic.code) {
      if (diagnostic.code >= std::numeric_limits<std::int32_t>::min() && diagnostic.code <= std::numeric_limits<std::uint32_t>::max())
        log << " code=" << std::format("0x{:08X}", static_cast<std::uint32_t>(diagnostic.code));
      else
        log << " code=" << std::format("0x{:X}", static_cast<std::uint64_t>(diagnostic.code));
    }
    log << " detail=";
    logQuoted(log, diagnostic.detail);
  }
}
#endif
}  // namespace

void logConsoleResult(const app::ConsoleCommandResult& result) noexcept {
  try {
    if (result.failure) {
      {
        LogStream log('E', "command");
        log << "event=failed exit_code=" << result.exitCode << " effect=" << effectName(result.effect) << " failed_reads=" << result.failedReads
            << " failures=" << (result.failures.empty() ? std::size_t{1} : result.failures.size()) << " reason=";
        logQuoted(log, app::consoleFailureSummary(result.failure->kind));
      }
#if defined(UWF_DEBUG_LOGGING)
      if (result.failures.empty())
        logFailure({}, *result.failure);
      else
        for (const auto& failure : result.failures) logFailure(failure.operation, failure.failure);
#endif
    }
    for (const auto& warning : result.warnings) {
      LogStream log('W', "command");
      log << "event=warning reason=";
      logQuoted(log, warning);
    }
  } catch (...) {
    // Evidence publication must never change a command's outcome.
  }
}

void logConsoleOutputFailure(const std::string_view operation, const std::string_view detail) noexcept {
  try {
    LogStream log('E', "command");
    log << "event=output-failed operation=";
    logQuoted(log, operation);
    log << " detail=";
    logQuoted(log, detail);
  } catch (...) {
  }
}

}  // namespace uwf::console
