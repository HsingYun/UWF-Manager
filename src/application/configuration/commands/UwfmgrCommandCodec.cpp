// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "application/configuration/commands/UwfmgrCommandCodec.h"

#include <algorithm>
#include <array>
#include <format>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <utility>

#include "application/configuration/commands/UwfmgrCommandParser.h"
#include "foundation/text/StringUtil.h"

namespace uwf::app {
bool isUwfmgrCommandRepresentable(const UwfmgrCommand& command) {
  return std::ranges::none_of(command.args, [](const auto& argument) {
    return std::ranges::any_of(argument, [](const char character) {
      const auto byte = static_cast<unsigned char>(character);
      return byte < 0x20 || byte == 0x7f;
    });
  });
}

namespace {
constexpr std::array<std::string_view, 5> kScriptPreamble{kUwfmgrScriptMarker, "@echo off", "setlocal DisableDelayedExpansion",
                                                          R"(for /f "tokens=2 delims=:" %%C in ('chcp') do set "_uwf_codepage=%%C")", "chcp 65001 >nul"};
constexpr std::array<std::string_view, 5> kLegacyScriptPreamble{":: UWF Manager CMD script (UTF-8)", "@echo off", "setlocal DisableDelayedExpansion",
                                                                R"(for /f "tokens=2 delims=:" %%C in ('chcp') do set "_uwf_codepage=%%C")", "chcp 65001 >nul"};
constexpr std::array<std::string_view, 3> kScriptEpilogue{"set \"_uwf_exit=%errorlevel%\"", "chcp %_uwf_codepage% >nul", "endlocal & exit /b %_uwf_exit%"};
constexpr std::string_view utf8Bom = "\xEF\xBB\xBF";
std::string_view markerDeclaration(std::string_view line) {
  const auto trimEdges = [&] {
    while (!line.empty() && isAsciiWhitespace(line.front())) line.remove_prefix(1);
    while (!line.empty() && isAsciiWhitespace(line.back())) line.remove_suffix(1);
  };
  trimEdges();
  while (line.starts_with(utf8Bom)) {
    line.remove_prefix(utf8Bom.size());
    trimEdges();
  }
  return line;
}
std::string chainLabel(const std::size_t chain) { return "_uwf_chain_" + std::to_string(chain) + "_end"; }
std::string positiveGuard(const std::size_t chain) { return "if errorlevel 1 goto " + chainLabel(chain); }
std::string negativeGuard(const std::size_t chain) { return "if not errorlevel 0 goto " + chainLabel(chain); }

std::optional<std::string> decodeBatchPercents(const std::string& line) {
  std::string decoded;
  decoded.reserve(line.size());
  for (std::size_t index = 0; index < line.size(); ++index) {
    if (line[index] == '%') {
      if (index + 1 == line.size() || line[index + 1] != '%') return std::nullopt;
      ++index;
    }
    decoded += line[index];
  }
  return decoded;
}

std::string quoteArg(const std::string& value) {
  constexpr std::string_view special = R"(&|<>()^[]=;!'+,`~{}")";
  const bool quoted =
      value.empty() || std::ranges::any_of(value, [&](const char c) { return isAsciiWhitespace(c) || special.find(c) != std::string_view::npos; });
  if (!quoted) return value;

  std::string result(1, '"');
  std::size_t backslashes = 0;
  bool shellQuoted = true;
  for (const char c : value) {
    if (c == '\\') {
      ++backslashes;
      continue;
    }
    result.append(backslashes * (c == '"' ? 2U : 1U), '\\');
    backslashes = 0;
    if (c == '"') {
      result += "\\\"";
      shellQuoted = !shellQuoted;
    } else {
      if (!shellQuoted && (isAsciiWhitespace(c) || std::string_view("&|<>()^").find(c) != std::string_view::npos)) result += '^';
      result += c;
    }
  }
  result.append(backslashes * 2U, '\\');
  result += '"';
  return result;
}

struct TokenizedLine {
  std::vector<std::string> tokens;
  bool quotesBalanced = true;
};

TokenizedLine tokenize(const std::string& line) {
  // Remove unquoted CMD caret escapes before applying Windows argv quoting rules.
  std::string argumentLine;
  bool shellQuoted = false;
  for (std::size_t index = 0; index < line.size(); ++index) {
    const char c = line[index];
    if (c == '"') shellQuoted = !shellQuoted;
    if (!shellQuoted && c == '^' && index + 1 < line.size()) {
      const char next = line[index + 1];
      if (isAsciiWhitespace(next) || std::string_view("&|<>()^").find(next) != std::string_view::npos) {
        argumentLine += next;
        ++index;
        continue;
      }
    }
    argumentLine += c;
  }
  TokenizedLine result;
  std::string cur;
  bool inQuote = false;
  bool tokenStarted = false;
  for (std::size_t index = 0; index < argumentLine.size(); ++index) {
    char c = argumentLine[index];
    if (c == '\\') {
      const std::size_t start = index;
      while (index < argumentLine.size() && argumentLine[index] == '\\') ++index;
      const std::size_t count = index - start;
      tokenStarted = true;
      if (index == argumentLine.size() || argumentLine[index] != '"') {
        cur.append(count, '\\');
        --index;
        continue;
      }
      cur.append(count / 2, '\\');
      if (count % 2 != 0) {
        cur += '"';
        continue;
      }
      c = '"';
    }
    if (c == '"') {
      if (inQuote && index + 1 < argumentLine.size() && argumentLine[index + 1] == '"') {
        cur += '"';
        ++index;
      } else {
        inQuote = !inQuote;
      }
      tokenStarted = true;
      continue;
    }
    if (!inQuote && isAsciiWhitespace(c)) {
      if (tokenStarted) {
        result.tokens.push_back(std::move(cur));
        cur.clear();
        tokenStarted = false;
      }
      continue;
    }
    cur.push_back(c);
    tokenStarted = true;
  }
  if (tokenStarted) result.tokens.push_back(std::move(cur));
  result.quotesBalanced = !inQuote;
  return result;
}

UwfmgrCommand parseLine(const std::string& rawLine, int lineNo) {
  UwfmgrCommand out;
  out.sourceLineNo = lineNo;
  out.rawLine = rawLine;

  const std::string line = trim(rawLine);

  if (line.empty() || line.front() == '#' || line.starts_with("::") || line.starts_with("//")) {
    out.parseError = ParseError::Comment;
    return out;
  }

  auto tokenized = tokenize(line);
  auto& tokens = tokenized.tokens;
  if (!tokenized.quotesBalanced) {
    out.parseError = ParseError::MalformedQuoting;
    return out;
  }
  if (tokens.empty()) {
    out.parseError = ParseError::Comment;
    return out;
  }

  // Accept commands with or without the uwfmgr executable prefix.
  {
    const std::string first = toLowerAscii(tokens[0]);
    if (first == "uwfmgr" || first == "uwfmgr.exe") tokens.erase(tokens.begin());
  }

  if (tokens.size() < 2) {
    out.parseError = ParseError::Incomplete;
    return out;
  }

  auto parsed = parseUwfmgrArguments(tokens);
  parsed.sourceLineNo = lineNo;
  parsed.rawLine = rawLine;
  // Reuse CLI grammar while limiting import to draft configuration commands.
  if (parsed.parseError == ParseError::None && !isUwfmgrConfigurationCommand(parsed.kind)) {
    parsed.parseError = ParseError::Unsupported;
  }
  return parsed;
}

}  // namespace

bool isUwfmgrScriptMarker(const std::string_view line) {
  const auto declaration = markerDeclaration(line);
  return declaration == kUwfmgrScriptMarker || declaration == kLegacyScriptPreamble.front();
}
std::string renderUwfmgrCommand(const UwfmgrCommand& cmd) {
  if (!isUwfmgrCommandRepresentable(cmd)) throw std::invalid_argument("uwfmgr command arguments cannot contain ASCII control characters");

  const std::string a0 = cmd.args.empty() ? std::string{} : cmd.args[0];
  switch (cmd.kind) {
    case UwfmgrKind::FilterEnable:
      return "uwfmgr.exe filter enable";
    case UwfmgrKind::FilterDisable:
      return "uwfmgr.exe filter disable";
    case UwfmgrKind::FilterEnableHorm:
      return "uwfmgr.exe filter enable-HORM";
    case UwfmgrKind::FilterDisableHorm:
      return "uwfmgr.exe filter disable-HORM";
    case UwfmgrKind::OverlaySetType:
      return std::format("uwfmgr.exe overlay set-type {}", a0);
    case UwfmgrKind::OverlaySetSize:
      return std::format("uwfmgr.exe overlay set-size {}", a0);
    case UwfmgrKind::OverlaySetWarningThreshold:
      return std::format("uwfmgr.exe overlay set-warningthreshold {}", a0);
    case UwfmgrKind::OverlaySetCriticalThreshold:
      return std::format("uwfmgr.exe overlay set-criticalthreshold {}", a0);
    case UwfmgrKind::OverlaySetPassthrough:
      return std::format("uwfmgr.exe overlay set-passthrough {}", a0);
    case UwfmgrKind::OverlaySetPersistent:
      return std::format("uwfmgr.exe overlay set-persistent {}", a0);
    case UwfmgrKind::OverlaySetReadOnlyMedia:
      return std::format("uwfmgr.exe overlay set-rom-mode {}", a0);
    case UwfmgrKind::OverlayResetPersistentState:
      return std::format("uwfmgr.exe overlay reset-persistentstate {}", a0);
    case UwfmgrKind::OverlayCommit:
      return "uwfmgr.exe overlay commit";
    case UwfmgrKind::VolumeProtect:
      return std::format("uwfmgr.exe volume protect {}", quoteArg(a0));
    case UwfmgrKind::VolumeUnprotect:
      return std::format("uwfmgr.exe volume unprotect {}", quoteArg(a0));
    case UwfmgrKind::VolumeProtectAll:
      return "uwfmgr.exe volume protect all";
    case UwfmgrKind::VolumeCreateSwapfile:
      return std::format("uwfmgr.exe volume create-swapfile {}", quoteArg(a0));
    case UwfmgrKind::FileAddExclusion:
      return std::format("uwfmgr.exe file add-exclusion {}", quoteArg(a0));
    case UwfmgrKind::FileRemoveExclusion:
      return std::format("uwfmgr.exe file remove-exclusion {}", quoteArg(a0));
    case UwfmgrKind::RegistryAddExclusion:
      return std::format("uwfmgr.exe registry add-exclusion {}", quoteArg(a0));
    case UwfmgrKind::RegistryRemoveExclusion:
      return std::format("uwfmgr.exe registry remove-exclusion {}", quoteArg(a0));
    default:
      return {};
  }
  return {};
}

namespace {
std::string commandScript(const std::vector<UwfmgrCommand>& commands, const std::string_view scope) {
  std::string script;
  const auto append = [&](const std::string_view line) { script.append(line).append("\r\n"); };
  for (const auto line : kScriptPreamble) append(line);
  std::istringstream descriptions{std::string(scope)};
  for (std::string line; std::getline(descriptions, line);) {
    std::string comment = ":: ";
    for (const char character : line) {
      if (character == '\r') continue;
      if (std::string_view("&|<>()^\"").find(character) != std::string_view::npos) comment += '^';
      comment += character;
      if (character == '%') comment += '%';
    }
    append(comment);
  }
  std::size_t chain = 0;
  for (std::size_t index = 0; index < commands.size(); ++index) {
    const auto& command = commands[index];
    if (index == 0 && command.requiresPrevious) throw std::invalid_argument("dependent command has no predecessor");
    if (!command.requiresPrevious) ++chain;
    const auto line = renderUwfmgrCommand(command);
    if (line.empty()) throw std::invalid_argument("an empty uwfmgr command cannot be exported");
    std::string escaped;
    escaped.reserve(line.size());
    for (const char character : line) {
      escaped += character;
      if (character == '%') escaped += '%';
    }
    append(escaped);
    if (index + 1 < commands.size() && commands[index + 1].requiresPrevious) {
      // ERRORLEVEL is a numeric >= comparison, independent of a shadowing
      // environment variable. The second guard also covers negative HRESULTs.
      append(positiveGuard(chain));
      append(negativeGuard(chain));
    } else {
      append(":" + chainLabel(chain));
    }
  }
  for (const auto line : kScriptEpilogue) append(line);
  return script;
}
}  // namespace

std::string renderUwfmgrCommandScript(const std::vector<UwfmgrCommand>& commands, const std::string_view scope) { return commandScript(commands, scope); }

std::vector<UwfmgrCommand> parseUwfmgrText(const std::string& text) {
  std::vector<UwfmgrCommand> out;
  std::stringstream ss(text);
  std::string line;
  std::vector<std::string> lines;
  while (std::getline(ss, line)) {
    if (!line.empty() && line.back() == '\r') line.pop_back();
    // Accept a BOM before a file marker, including blocks preceded by loader comments.
    // Other framing changes remain noncanonical.
    if (line.starts_with(utf8Bom)) {
      const std::string_view unmarked(line.data() + utf8Bom.size(), line.size() - utf8Bom.size());
      if (unmarked == kUwfmgrScriptMarker || unmarked == kLegacyScriptPreamble.front()) line.erase(0, utf8Bom.size());
    }
    lines.push_back(std::move(line));
  }
  const auto appendLine = [&](const std::string& line, const int lineNo, const bool batch) {
    const auto trimmed = trim(line);
    if (trimmed.empty() || trimmed.starts_with('#') || trimmed.starts_with("::") || trimmed.starts_with("//")) {
      out.push_back(parseLine(line, lineNo));
      return;
    }
    std::vector<std::string> parts;
    std::size_t start = 0;
    bool quoted = false;
    for (std::size_t index = 0; index < line.size(); ++index) {
      const char c = line[index];
      if (!quoted && c == '^' && index + 1 < line.size()) {
        ++index;
      } else if (c == '"') {
        quoted = !quoted;
      } else if (!quoted && c == '&' && index + 1 < line.size() && line[index + 1] == '&') {
        parts.push_back(line.substr(start, index - start));
        start = index + 2;
        ++index;
      }
    }
    parts.push_back(line.substr(start));
    if (!tokenize(line).quotesBalanced) {
      auto invalid = parseLine(line, lineNo);
      invalid.parseError = ParseError::MalformedQuoting;
      out.push_back(std::move(invalid));
      return;
    }
    if (std::ranges::any_of(parts, [](const auto& part) { return trim(part).empty(); })) {
      auto invalid = parseLine(line, lineNo);
      invalid.parseError = ParseError::UnexpectedArgument;
      invalid.parseErrorContext = "&&";
      out.push_back(std::move(invalid));
      return;
    }
    for (std::size_t index = 0; index < parts.size(); ++index) {
      const auto decoded = batch ? decodeBatchPercents(parts[index]) : std::optional<std::string>(parts[index]);
      auto command = parseLine(decoded.value_or(parts[index]), lineNo);
      command.rawLine = parts[index];
      // Single percent signs are unsupported in canonical scripts; no environment expansion is performed.
      if (!decoded) command.parseError = ParseError::Unsupported;
      // A comment inside a chain must not bypass its dependency.
      if (parts.size() > 1 && command.parseError == ParseError::Comment) command.parseError = ParseError::Unsupported;
      command.requiresPrevious = index != 0;
      out.push_back(std::move(command));
    }
  };
  const auto matches = [&](const std::size_t start, const auto& envelope) {
    return start + envelope.size() <= lines.size() &&
           std::equal(envelope.begin(), envelope.end(), std::next(lines.begin(), static_cast<std::vector<std::string>::difference_type>(start)));
  };
  const auto parseGuardedBody = [&](const std::size_t begin, const std::size_t end) -> std::optional<std::vector<UwfmgrCommand>> {
    std::vector<UwfmgrCommand> commands;
    std::size_t chain = 1, index = begin;
    while (index < end) {
      auto comment = parseLine(lines[index], static_cast<int>(index + 1));
      if (comment.parseError == ParseError::Comment) {
        commands.push_back(std::move(comment));
        ++index;
        continue;
      }
      bool dependent = false;
      for (;;) {
        if (index >= end) return {};
        const auto decoded = decodeBatchPercents(lines[index]);
        if (!decoded) return {};
        auto command = parseLine(*decoded, static_cast<int>(index + 1));
        if (command.parseError != ParseError::None) return {};
        command.rawLine = lines[index];
        command.requiresPrevious = dependent;
        commands.push_back(std::move(command));
        ++index;
        if (index < end && lines[index] == ":" + chainLabel(chain)) {
          ++index;
          ++chain;
          break;
        }
        if (index + 1 >= end || lines[index] != positiveGuard(chain) || lines[index + 1] != negativeGuard(chain)) return {};
        index += 2;
        dependent = true;
      }
    }
    return commands;
  };
  for (std::size_t index = 0; index < lines.size(); ++index) {
    if (isUwfmgrScriptMarker(lines[index])) {
      const bool guarded = markerDeclaration(lines[index]) == kUwfmgrScriptMarker;
      const bool canonical = guarded ? matches(index, kScriptPreamble) : matches(index, kLegacyScriptPreamble);
      const auto start = index;
      auto end = index + 1;
      while (end < lines.size() && !matches(end, kScriptEpilogue)) ++end;
      if (!guarded && canonical && end < lines.size()) {
        for (auto command = index + kLegacyScriptPreamble.size(); command < end; ++command) appendLine(lines[command], static_cast<int>(command + 1), true);
        index = end + kScriptEpilogue.size() - 1;
        continue;
      }
      const auto parsed =
          guarded && canonical && end < lines.size() ? parseGuardedBody(index + kScriptPreamble.size(), end) : std::optional<std::vector<UwfmgrCommand>>{};
      if (parsed) {
        out.insert(out.end(), parsed->begin(), parsed->end());
      } else {
        // Reject broken generated framing as a unit so dependency guards cannot be bypassed.
        auto invalid = parseLine(lines[start], static_cast<int>(start + 1));
        invalid.parseError = ParseError::Unsupported;
        out.push_back(std::move(invalid));
      }
      if (end == lines.size()) break;
      index = end + kScriptEpilogue.size() - 1;
      continue;
    }
    appendLine(lines[index], static_cast<int>(index + 1), false);
  }
  return out;
}

}  // namespace uwf::app
