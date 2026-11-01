// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "presentation/console/ConsoleRenderer.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <format>
#include <limits>
#include <string_view>

#include "presentation/console/ConsoleDiagnostics.h"

namespace uwf::console {
namespace {
std::string lineText(const std::string_view input) {
  std::string text;
  for (const auto character : input) {
    const auto value = static_cast<unsigned char>(character);
    if (value < 0x20 || value == 0x7f)
      text += std::format("\\x{:02X}", value);
    else
      text += character;
  }
  return text;
}
std::string_view effectName(const compat::Effect effect) {
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
std::string failureText(const compat::Failure& failure) {
  std::string text = "Error: " + std::string(app::consoleFailureSummary(failure.kind));
  const auto diagnostic = std::find_if(failure.diagnostics.rbegin(), failure.diagnostics.rend(), [](const compat::Diagnostic& item) { return item.code != 0; });
  if (diagnostic != failure.diagnostics.rend()) {
    const auto code = diagnostic->code;
    // HRESULTs may be sign-extended or unsigned; wider native codes retain their full value.
    if (code >= std::numeric_limits<std::int32_t>::min() && code <= std::numeric_limits<std::uint32_t>::max())
      text += std::format(" (0x{:08X})", static_cast<std::uint32_t>(code));
    else
      text += std::format(" (0x{:X})", static_cast<std::uint64_t>(code));
  }
  return text;
}
std::size_t fieldWidth(const app::ConsoleSection& section) {
  std::size_t width = 0;
  for (const auto& field : section.fields) width = std::max(width, lineText(field.name).size());
  for (const auto& child : section.children) width = std::max(width, fieldWidth(child));
  return width;
}
void appendSection(std::string& block, const app::ConsoleSection& section, const std::size_t depth, const std::size_t width) {
  const std::string indentation(depth * 2, ' ');
  if (!section.title.empty()) block += indentation + lineText(section.title) + '\n';
  for (const auto& field : section.fields) {
    const auto name = lineText(field.name);
    const auto value = field.failure ? failureText(*field.failure) : lineText(field.value);
    block += indentation + "  " + name + std::string(width - name.size(), ' ') + " : " + value + '\n';
  }
  for (const auto& item : section.items) block += indentation + "  " + lineText(item) + '\n';
  for (const auto& child : section.children) appendSection(block, child, depth + 1, width);
}
}  // namespace
ConsoleText renderConsoleResult(const app::ConsoleCommandResult& result) {
  ConsoleText text;
  const auto appendBlock = [&](const std::string& block) {
    if (block.empty()) return;
    if (!text.output.empty()) text.output += '\n';
    text.output += block;
  };
  std::string heading;
  if (!result.title.empty()) heading = lineText(result.title) + '\n';
  if (result.failedReads > 0)
    heading += std::format("Status: {} ({} failed observations{})\n", result.successfulReads > 0 ? "Partially read" : "Read failed", result.failedReads,
                           result.interrupted ? "; interrupted" : "");
  else if (result.interrupted && result.successfulReads > 0)
    heading += "Status: Partially read (interrupted)\n";
  appendBlock(heading);
  std::size_t width = 0;
  for (const auto& section : result.sections) width = std::max(width, fieldWidth(section));
  for (const auto& section : result.sections) {
    std::string block;
    appendSection(block, section, 0, width);
    appendBlock(block);
  }
  std::string notes;
  for (const auto& note : result.notes) notes += lineText(note) + '\n';
  appendBlock(notes);
  for (const auto& warning : result.warnings) text.error += "Warning: " + lineText(warning) + '\n';
  if (result.failure) {
    text.error += failureText(*result.failure) + '\n';
    if (!result.publicReason.empty()) text.error += "Reason: " + lineText(result.publicReason) + '\n';
    text.error += "Effect: " + std::string(effectName(result.effect)) + '\n';
    if (result.effect != compat::Effect::None) text.error += "The operation may have applied changes. Check state before retrying.\n";
  }
  return text;
}
int publishConsoleResult(const app::ConsoleCommandResult& result, const ConsoleTextWriter& output, const ConsoleTextWriter& error) {
  const auto text = renderConsoleResult(result);
  std::string outputFailure;
  try {
    if (!text.output.empty()) output(text.output);
  } catch (const std::exception& failure) {
    logConsoleOutputFailure("WriteStandardOutput", failure.what());
    app::ConsoleCommandResult failedOutput;
    failedOutput.effect = result.effect;
    failedOutput.failure = compat::Failure{compat::ErrorKind::Failed, result.effect, "Could not write standard output", {}};
    failedOutput.publicReason = "Could not write standard output";
    outputFailure = renderConsoleResult(failedOutput).error;
  }
  try {
    if (!text.error.empty() || !outputFailure.empty()) error(text.error + outputFailure);
  } catch (const std::exception& failure) {
    logConsoleOutputFailure("WriteStandardError", failure.what());
    return app::consoleFailureExitCode(compat::ErrorKind::Failed);
  }
  return outputFailure.empty() ? result.exitCode : app::consoleFailureExitCode(compat::ErrorKind::Failed);
}
}  // namespace uwf::console
