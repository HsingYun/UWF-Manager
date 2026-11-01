// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "application/configuration/commands/UwfmgrCommand.h"
#include "domain/uwf/contracts/AllVolumeProtection.h"
#include "domain/uwf/contracts/Backend.h"

namespace uwf::app {
struct ConsoleVolumeTarget {
  std::string id;
  std::string driveLetter;
  bool bindByDriveLetter = false;
};
struct ConsoleFileTarget {
  std::string volume;
  // Original body after the explicit CLI volume root. An omitted body is
  // empty, not "\\"; each complete UWF operation validates its own target.
  std::string path;
};

// Resolves user-facing paths without introducing platform APIs into command
// semantics. Failures retain typed UWF errors and original platform diagnostics.
class ConsoleEnvironment {
 public:
  virtual ~ConsoleEnvironment() = default;
  [[nodiscard]] virtual ConsoleVolumeTarget resolveVolume(const std::string& operand) const = 0;
  [[nodiscard]] virtual ConsoleFileTarget resolveFile(const std::string& operand) const = 0;
};

struct ConsoleField {
  std::string name;
  std::string value;
  std::optional<compat::Failure> failure = {};
};
struct ConsoleSection {
  std::string title;
  std::vector<ConsoleField> fields;
  std::vector<std::string> items;
  std::vector<ConsoleSection> children = {};
};
struct ConsoleFailure {
  std::string operation;
  compat::Failure failure;
};
struct ConsoleCommandResult {
  std::vector<ConsoleSection> sections;
  std::vector<std::string> warnings;
  std::optional<compat::Failure> failure;
  std::vector<compat::Attempt> attempts;
  compat::Effect effect = compat::Effect::None;
  int exitCode = 0;
  std::string title;
  std::vector<std::string> notes;
  std::vector<ConsoleFailure> failures = {};
  std::size_t successfulReads = 0;
  std::size_t failedReads = 0;
  std::string publicReason;
  bool interrupted = false;
};

// No event loop, UI policy, automatic elevation or process-wide output belongs
// here. The supplied UWF port owns execution scheduling and provider selection.
[[nodiscard]] ConsoleCommandResult executeConsoleCommand(const UwfmgrCommand& command, const compat::Uwf& system, const ConsoleEnvironment& environment,
                                                         const compat::Context& context = {}, const compat::AllVolumeProtection* allVolumes = nullptr);
[[nodiscard]] int consoleFailureExitCode(compat::ErrorKind kind);
// Stable English presentation; original provider text remains in Failure.
[[nodiscard]] std::string_view consoleFailureSummary(compat::ErrorKind kind);
}  // namespace uwf::app
