// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "UwfmgrCommands.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <format>
#include <iterator>
#include <map>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <utility>

#include "app/paths/PathPolicy.h"
#include "core/OverlayWritePlan.h"
#include "shared/DriveLetter.h"
#include "shared/RegistryPath.h"
#include "shared/StringUtil.h"

namespace uwf::app {

namespace {

constexpr std::array<std::string_view, 5> kScriptPreamble{
    kUwfmgrScriptMarker, "@echo off", "setlocal DisableDelayedExpansion",
    "for /f \"tokens=2 delims=:\" %%C in ('chcp') do set \"_uwf_codepage=%%C\"", "chcp 65001 >nul"};
constexpr std::array<std::string_view, 3> kScriptEpilogue{
    "set \"_uwf_exit=%errorlevel%\"", "chcp %_uwf_codepage% >nul", "endlocal & exit /b %_uwf_exit%"};

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

// Windows argv 转义与 CMD 引用状态分别处理；字面引号会切换 CMD 引用状态。
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

bool parseUInt(const std::string& s, uint64_t& out) {
  if (s.empty()) return false;
  out = 0;
  for (char c : s) {
    if (c < '0' || c > '9') return false;
    const uint64_t digit = static_cast<uint64_t>(c - '0');
    // 使用当前位判断溢出，保留 UINT64_MAX 附近的合法值。
    if (out > (UINT64_MAX - digit) / 10) return false;
    out = out * 10 + digit;
  }
  return true;
}

struct TokenizedLine {
  std::vector<std::string> tokens;
  bool quotesBalanced = true;
};

TokenizedLine tokenize(const std::string& line) {
  // 先移除 CMD 引用外的 caret 转义，再按 Windows argv 规则处理引号和反斜杠。
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

UwfmgrCommand makeCmd(UwfmgrKind k, std::vector<std::string> args = {}) {
  UwfmgrCommand c;
  c.kind = k;
  c.args = std::move(args);
  return c;
}

std::vector<UwfmgrCommand> renderOverlayChanges(const core::OverlayConfig& baseline, const core::OverlayConfigDelta& changes) {
  std::vector<UwfmgrCommand> commands;
  for (const auto& step : core::planOverlayWrites(baseline, changes)) {
    switch (step.field) {
      case core::OverlayField::Type:
        commands.push_back(makeCmd(UwfmgrKind::OverlaySetType, {*changes.type == core::OverlayType::RAM ? "RAM" : "Disk"}));
        break;
      case core::OverlayField::MaximumSize:
        commands.push_back(makeCmd(UwfmgrKind::OverlaySetSize, {std::to_string(*changes.maximumSizeMb)}));
        break;
      case core::OverlayField::WarningThreshold:
        commands.push_back(makeCmd(UwfmgrKind::OverlaySetWarningThreshold, {std::to_string(*changes.warningThresholdMb)}));
        break;
      case core::OverlayField::CriticalThreshold:
        commands.push_back(makeCmd(UwfmgrKind::OverlaySetCriticalThreshold, {std::to_string(*changes.criticalThresholdMb)}));
        break;
    }
    commands.back().requiresPrevious = step.requiresPrevious;
  }
  return commands;
}

// 按配置目标合并基线与待应用命令，保留每个目标的最终值。
std::pair<UwfmgrKind, std::string> commandTarget(const UwfmgrCommand& command) {
  auto kind = command.kind;
  if (kind == UwfmgrKind::FilterDisable) kind = UwfmgrKind::FilterEnable;
  if (kind == UwfmgrKind::VolumeUnprotect) kind = UwfmgrKind::VolumeProtect;
  if (kind == UwfmgrKind::FileRemoveExclusion) kind = UwfmgrKind::FileAddExclusion;
  if (kind == UwfmgrKind::RegistryRemoveExclusion) kind = UwfmgrKind::RegistryAddExclusion;

  std::string argument = command.args.empty() ? std::string{} : command.args.front();
  switch (kind) {
    case UwfmgrKind::VolumeProtect:
      argument = drive::normalize(argument);
      break;
    case UwfmgrKind::FileAddExclusion:
      std::replace(argument.begin(), argument.end(), '/', '\\');
      argument = toLowerAscii(app::paths::stripTrailingSeparators(argument));
      break;
    case UwfmgrKind::RegistryAddExclusion:
      argument = toLowerAscii(regkey::normalize(argument));
      break;
    default:
      argument.clear();
      break;
  }
  return {kind, std::move(argument)};
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

  // 首 token 是 uwfmgr / uwfmgr.exe（大小写不敏感）就剥掉；用户也可以省略
  // 前缀直接写 "filter enable" 这种短形式。
  {
    const std::string first = toLowerAscii(tokens[0]);
    if (first == "uwfmgr" || first == "uwfmgr.exe") tokens.erase(tokens.begin());
  }

  if (tokens.size() < 2) {
    out.parseError = ParseError::Incomplete;
    return out;
  }

  const std::string cat = toLowerAscii(tokens[0]);
  const std::string verb = toLowerAscii(tokens[1]);
  const auto rejectUnexpectedArgument = [&](const std::size_t expectedCount) {
    if (tokens.size() <= expectedCount) return false;
    out.parseError = ParseError::UnexpectedArgument;
    out.parseErrorContext = tokens[expectedCount];
    return true;
  };

  // 一旦下面任何分支识别出 cat/verb 组合，就先把 out.kind 设到对应枚举；
  // 后续只要 parseError 非 None 就一律按"识别但参数非法"报告，而非 Unsupported。
  if (cat == "filter") {
    if (verb == "enable") {
      out.kind = UwfmgrKind::FilterEnable;
      rejectUnexpectedArgument(2);
      return out;
    }
    if (verb == "disable") {
      out.kind = UwfmgrKind::FilterDisable;
      rejectUnexpectedArgument(2);
      return out;
    }
  } else if (cat == "overlay") {
    auto needSize = [&](UwfmgrKind k) {
      out.kind = k;
      if (tokens.size() < 3) {
        out.parseError = ParseError::MissingSizeArg;
        return;
      }
      uint64_t v = 0;
      if (!parseUInt(tokens[2], v)) {
        out.parseError = ParseError::InvalidSize;
        return;
      }
      if (rejectUnexpectedArgument(3)) return;
      out.args.push_back(std::to_string(v));
    };
    if (verb == "set-type") {
      out.kind = UwfmgrKind::OverlaySetType;
      if (tokens.size() < 3) {
        out.parseError = ParseError::MissingTypeArg;
        return out;
      }
      if (rejectUnexpectedArgument(3)) return out;
      const auto t = toLowerAscii(tokens[2]);
      if (t == "ram") {
        out.args.emplace_back("RAM");
      } else if (t == "disk") {
        out.args.emplace_back("Disk");
      } else {
        out.parseError = ParseError::UnknownType;
        out.parseErrorContext = tokens[2];
      }
      return out;
    }
    if (verb == "set-size") {
      needSize(UwfmgrKind::OverlaySetSize);
      return out;
    }
    if (verb == "set-warningthreshold") {
      needSize(UwfmgrKind::OverlaySetWarningThreshold);
      return out;
    }
    if (verb == "set-criticalthreshold") {
      needSize(UwfmgrKind::OverlaySetCriticalThreshold);
      return out;
    }
  } else if (cat == "volume") {
    if (verb == "protect" || verb == "unprotect") {
      out.kind = (verb == "protect") ? UwfmgrKind::VolumeProtect : UwfmgrKind::VolumeUnprotect;
      if (tokens.size() < 3) {
        out.parseError = ParseError::MissingVolumeArg;
        return out;
      }
      if (rejectUnexpectedArgument(3)) return out;
      // 文档里 protect/unprotect 接受 "all"（一次作用于所有卷），但本项目按
      // 单卷映射到 UI，不支持批量形式；归为 Unsupported，而不是按"盘符非法"
      // 误报。kind 复位成 Unknown，与其它 Unsupported 命令保持一致。
      if (toLowerAscii(tokens[2]) == "all") {
        out.kind = UwfmgrKind::Unknown;
        out.parseError = ParseError::Unsupported;
        return out;
      }
      // 规范化盘符——接受 "c" / "C:" 等形式；非法输入（含卷名形式
      // "\\?\Volume{...}"）时 normalize 返回空串。
      std::string dl = drive::normalize(tokens[2]);
      if (dl.empty()) {
        out.parseError = ParseError::InvalidVolume;
        out.parseErrorContext = tokens[2];
        return out;
      }
      out.args.push_back(std::move(dl));
      return out;
    }
  } else if (cat == "file") {
    if (verb == "add-exclusion" || verb == "remove-exclusion") {
      out.kind = (verb == "add-exclusion") ? UwfmgrKind::FileAddExclusion : UwfmgrKind::FileRemoveExclusion;
      if (tokens.size() < 3) {
        out.parseError = ParseError::MissingPathArg;
        return out;
      }
      if (rejectUnexpectedArgument(3)) return out;
      if (tokens[2].empty()) {
        out.parseError = ParseError::MissingPathArg;
        return out;
      }
      out.args.push_back(tokens[2]);
      return out;
    }
  } else if (cat == "registry") {
    if (verb == "add-exclusion" || verb == "remove-exclusion") {
      out.kind = (verb == "add-exclusion") ? UwfmgrKind::RegistryAddExclusion : UwfmgrKind::RegistryRemoveExclusion;
      if (tokens.size() < 3) {
        out.parseError = ParseError::MissingRegistryKeyArg;
        return out;
      }
      if (rejectUnexpectedArgument(3)) return out;
      if (tokens[2].empty()) {
        out.parseError = ParseError::MissingRegistryKeyArg;
        return out;
      }
      out.args.push_back(tokens[2]);
      return out;
    }
  }

  out.parseError = ParseError::Unsupported;
  return out;
}

}  // namespace

std::string renderCommand(const UwfmgrCommand& cmd) {
  // 形式："uwfmgr.exe <cat> <verb> [<args...>]"。路径与注册表键由 quoteArg
  // 统一处理空白、CMD 元字符、字面引号及末尾反斜杠。
  const std::string a0 = cmd.args.empty() ? std::string{} : cmd.args[0];
  switch (cmd.kind) {
    case UwfmgrKind::FilterEnable:
      return "uwfmgr.exe filter enable";
    case UwfmgrKind::FilterDisable:
      return "uwfmgr.exe filter disable";
    case UwfmgrKind::OverlaySetType:
      return std::format("uwfmgr.exe overlay set-type {}", a0);
    case UwfmgrKind::OverlaySetSize:
      return std::format("uwfmgr.exe overlay set-size {}", a0);
    case UwfmgrKind::OverlaySetWarningThreshold:
      return std::format("uwfmgr.exe overlay set-warningthreshold {}", a0);
    case UwfmgrKind::OverlaySetCriticalThreshold:
      return std::format("uwfmgr.exe overlay set-criticalthreshold {}", a0);
    case UwfmgrKind::VolumeProtect:
      return std::format("uwfmgr.exe volume protect {}", a0);
    case UwfmgrKind::VolumeUnprotect:
      return std::format("uwfmgr.exe volume unprotect {}", a0);
    case UwfmgrKind::FileAddExclusion:
      return std::format("uwfmgr.exe file add-exclusion {}", quoteArg(a0));
    case UwfmgrKind::FileRemoveExclusion:
      return std::format("uwfmgr.exe file remove-exclusion {}", quoteArg(a0));
    case UwfmgrKind::RegistryAddExclusion:
      return std::format("uwfmgr.exe registry add-exclusion {}", quoteArg(a0));
    case UwfmgrKind::RegistryRemoveExclusion:
      return std::format("uwfmgr.exe registry remove-exclusion {}", quoteArg(a0));
    case UwfmgrKind::Unknown:
      return {};
  }
  return {};
}

std::vector<std::string> renderCommandLines(const std::vector<UwfmgrCommand>& commands) {
  std::vector<std::string> lines;
  for (const auto& command : commands) {
    auto line = renderCommand(command);
    if (command.requiresPrevious) {
      if (lines.empty()) throw std::invalid_argument("dependent command has no predecessor");
      lines.back() += " && " + line;
    } else {
      lines.push_back(std::move(line));
    }
  }
  return lines;
}

std::string renderCommandScript(const std::vector<UwfmgrCommand>& commands) {
  std::string script;
  const auto append = [&](const std::string_view line) { script.append(line).append("\r\n"); };
  for (const auto line : kScriptPreamble) append(line);
  for (const auto& line : renderCommandLines(commands)) {
    std::string escaped;
    escaped.reserve(line.size());
    for (const char character : line) {
      escaped += character;
      if (character == '%') escaped += '%';
    }
    append(escaped);
  }
  for (const auto line : kScriptEpilogue) append(line);
  return script;
}

std::vector<UwfmgrCommand> parseUwfmgrText(const std::string& text) {
  std::vector<UwfmgrCommand> out;
  std::stringstream ss(text);
  std::string line;
  std::vector<std::string> lines;
  while (std::getline(ss, line)) {
    if (!line.empty() && line.back() == '\r') line.pop_back();
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
      // 本程序不求值环境变量；规范脚本中的单个 % 不是受支持的字面命令。
      if (!decoded) command.parseError = ParseError::Unsupported;
      // 行首注释已在上方处理；链中的注释标记不能被过滤掉并绕过依赖。
      if (parts.size() > 1 && command.parseError == ParseError::Comment) command.parseError = ParseError::Unsupported;
      command.requiresPrevious = index != 0;
      out.push_back(std::move(command));
    }
  };
  const auto matches = [&](const std::size_t start, const auto& envelope) {
    return start + envelope.size() <= lines.size() &&
           std::equal(envelope.begin(), envelope.end(), std::next(lines.begin(), static_cast<std::vector<std::string>::difference_type>(start)));
  };
  for (std::size_t index = 0; index < lines.size(); ++index) {
    if (matches(index, kScriptPreamble)) {
      const auto body = index + kScriptPreamble.size();
      auto end = body;
      while (end < lines.size() && !matches(end, kScriptEpilogue)) ++end;
      if (end < lines.size()) {
        for (auto command = body; command < end; ++command) appendLine(lines[command], static_cast<int>(command + 1), true);
        index = end + kScriptEpilogue.size() - 1;
        continue;
      }
    }
    appendLine(lines[index], static_cast<int>(index + 1), false);
  }
  return out;
}

std::vector<UwfmgrCommand> renderPendingChanges(const core::PendingChanges& c, const core::OverlayConfig& baseline) {
  std::vector<UwfmgrCommand> out;

  if (c.setFilterEnabled) {
    out.push_back(makeCmd(*c.setFilterEnabled ? UwfmgrKind::FilterEnable : UwfmgrKind::FilterDisable));
  }

  const auto overlay = renderOverlayChanges(baseline, c.setOverlay);
  out.insert(out.end(), overlay.begin(), overlay.end());

  for (const auto& [dl, want] : c.volumeProtect) {
    out.push_back(makeCmd(want ? UwfmgrKind::VolumeProtect : UwfmgrKind::VolumeUnprotect, {dl}));
  }
  // volumeBindByVolumeName 没有 CLI 对应，不输出。

  for (const auto& [dl, paths] : c.addFileExclusions) {
    (void)dl;  // CLI add-exclusion 不带盘符——路径自带盘符就足够定位了
    for (const auto& p : paths) out.push_back(makeCmd(UwfmgrKind::FileAddExclusion, {p}));
  }
  for (const auto& [dl, paths] : c.removeFileExclusions) {
    (void)dl;
    for (const auto& p : paths) out.push_back(makeCmd(UwfmgrKind::FileRemoveExclusion, {p}));
  }
  for (const auto& k : c.addRegistryExclusions) out.push_back(makeCmd(UwfmgrKind::RegistryAddExclusion, {k}));
  for (const auto& k : c.removeRegistryExclusions) out.push_back(makeCmd(UwfmgrKind::RegistryRemoveExclusion, {k}));

  return out;
}

std::vector<UwfmgrCommand> renderSession(const core::SessionSnapshot& s) {
  std::vector<UwfmgrCommand> out;

  out.push_back(makeCmd(s.filter.enabled ? UwfmgrKind::FilterEnable : UwfmgrKind::FilterDisable));
  const core::OverlayConfigDelta allOverlay{.type = s.overlay.type,
                                            .maximumSizeMb = s.overlay.maximumSizeMb,
                                            .warningThresholdMb = s.overlay.warningThresholdMb,
                                            .criticalThresholdMb = s.overlay.criticalThresholdMb};
  const auto overlay = renderOverlayChanges(s.overlay, allOverlay);
  out.insert(out.end(), overlay.begin(), overlay.end());

  for (const auto& v : s.volumes) {
    if (v.driveLetter.empty()) continue;
    out.push_back(makeCmd(v.isProtected ? UwfmgrKind::VolumeProtect : UwfmgrKind::VolumeUnprotect, {v.driveLetter}));
  }

  // fileExclusions 的 key 是 UWF 裸 volumeName（"Volume{GUID}"），但 add-exclusion
  // 的 path 已带盘符（约定），盘符回查就不必做了。
  for (const auto& [vname, paths] : s.fileExclusions) {
    (void)vname;
    for (const auto& p : paths) out.push_back(makeCmd(UwfmgrKind::FileAddExclusion, {p}));
  }
  for (const auto& k : s.registryExclusions) out.push_back(makeCmd(UwfmgrKind::RegistryAddExclusion, {k}));

  return out;
}

std::vector<UwfmgrCommand> renderPlannedSession(const core::SessionSnapshot& baseline, const core::PendingChanges& changes) {
  auto commands = renderSession(baseline);
  std::map<std::pair<UwfmgrKind, std::string>, std::size_t> targets;
  for (std::size_t index = 0; index < commands.size(); ++index) targets.emplace(commandTarget(commands[index]), index);

  for (auto& command : renderPendingChanges(changes, baseline.overlay)) {
    const auto [target, inserted] = targets.emplace(commandTarget(command), commands.size());
    if (inserted)
      commands.push_back(std::move(command));
    else
      commands[target->second] = std::move(command);
  }
  // 合并替换保留了原位置；按实际基线重新安排覆盖层命令的依赖顺序。
  const core::OverlayConfigDelta allOverlay{.type = changes.setOverlay.type.value_or(baseline.overlay.type),
                                            .maximumSizeMb = changes.setOverlay.maximumSizeMb.value_or(baseline.overlay.maximumSizeMb),
                                            .warningThresholdMb = changes.setOverlay.warningThresholdMb.value_or(baseline.overlay.warningThresholdMb),
                                            .criticalThresholdMb = changes.setOverlay.criticalThresholdMb.value_or(baseline.overlay.criticalThresholdMb)};
  const auto overlay = renderOverlayChanges(baseline.overlay, allOverlay);
  std::copy(overlay.begin(), overlay.end(), commands.begin() + 1);
  return commands;
}

}  // namespace uwf::app
