#include "UwfmgrCli.h"

#include <cctype>
#include <cstdint>
#include <format>
#include <sstream>
#include <utility>

#include "../../util/DriveLetter.h"

namespace uwf::api {

namespace {

// uwfmgr CLI 引号规则：路径含空格就用双引号包整段；其它字面量原样。
std::string quoteArg(const std::string& s) { return s.find(' ') != std::string::npos ? "\"" + s + "\"" : s; }

// 尝试把 token 解析成非负整数（uint64）。成功返回 true，out 拿到值。
bool parseUInt(const std::string& s, uint64_t& out) {
  if (s.empty()) return false;
  out = 0;
  for (char c : s) {
    if (c < '0' || c > '9') return false;
    const uint64_t digit = static_cast<uint64_t>(c - '0');
    // 阈值随当前位取值——按定值 9 算会把末位较小的 6 个 uint64 上界值误拒。
    if (out > (UINT64_MAX - digit) / 10) return false;  // overflow guard
    out = out * 10 + digit;
  }
  return true;
}

// shell 风格分词：双引号包起来的整段算一个 token，引号本身不进 token。
// uwfmgr 命令本身和 Windows 路径里都不会用到单引号或反斜杠转义，所以这里
// 只处理双引号；其它字符按字面量收进 token。
std::vector<std::string> tokenize(const std::string& line) {
  std::vector<std::string> out;
  std::string cur;
  bool inQuote = false;
  for (char c : line) {
    if (c == '"') {
      inQuote = !inQuote;
      continue;
    }
    if (!inQuote && std::isspace(static_cast<unsigned char>(c))) {
      if (!cur.empty()) {
        out.push_back(std::move(cur));
        cur.clear();
      }
      continue;
    }
    cur.push_back(c);
  }
  if (!cur.empty()) out.push_back(std::move(cur));
  return out;
}

std::string toLowerAscii(std::string s) {
  for (auto& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  return s;
}

void trimInPlace(std::string& s) {
  while (!s.empty() && std::isspace(static_cast<unsigned char>(s.front()))) s.erase(s.begin());
  while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back()))) s.pop_back();
}

UwfmgrCommand makeCmd(UwfmgrKind k, std::vector<std::string> args = {}) {
  UwfmgrCommand c;
  c.kind = k;
  c.args = std::move(args);
  return c;
}

UwfmgrCommand parseLine(const std::string& rawLine, int lineNo) {
  UwfmgrCommand out;
  out.sourceLineNo = lineNo;
  out.rawLine = rawLine;

  std::string line = rawLine;
  trimInPlace(line);

  if (line.empty() || line.front() == '#' || line.starts_with("::") || line.starts_with("//")) {
    out.parseError = ParseError::Comment;
    return out;
  }

  auto tokens = tokenize(line);
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

  // 一旦下面任何分支识别出 cat/verb 组合，就先把 out.kind 设到对应枚举；
  // 后续只要 parseError 非 None 就一律按"识别但参数非法"报告，而非 Unsupported。
  if (cat == "filter") {
    if (verb == "enable") {
      out.kind = UwfmgrKind::FilterEnable;
      return out;
    }
    if (verb == "disable") {
      out.kind = UwfmgrKind::FilterDisable;
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
      out.args.push_back(std::to_string(v));
    };
    if (verb == "set-type") {
      out.kind = UwfmgrKind::OverlaySetType;
      if (tokens.size() < 3) {
        out.parseError = ParseError::MissingTypeArg;
        return out;
      }
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
      out.args.push_back(tokens[2]);
      return out;
    }
  }

  // 兜底：整段命令没匹配上任何已知 cat/verb 组合（包括 uwfmgr CLI 里我们暂不
  // 支持的 commit-* / get-config 等）。报告里归 Unsupported。
  out.parseError = ParseError::Unsupported;
  return out;
}

}  // namespace

std::string renderCommand(const UwfmgrCommand& cmd) {
  // 形式："uwfmgr.exe <cat> <verb> [<args...>]"。args 里含空格的（典型场景：
  // 路径含 "Program Files"）由 quoteArg 自动加双引号。
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

std::vector<UwfmgrCommand> parseUwfmgrText(const std::string& text) {
  std::vector<UwfmgrCommand> out;
  std::stringstream ss(text);
  std::string line;
  int lineNo = 0;
  while (std::getline(ss, line)) {
    ++lineNo;
    // strip trailing \r 防 Windows CRLF 让所有行都解析失败
    if (!line.empty() && line.back() == '\r') line.pop_back();
    out.push_back(parseLine(line, lineNo));
  }
  return out;
}

std::vector<UwfmgrCommand> renderPendingChanges(const core::PendingChanges& c) {
  std::vector<UwfmgrCommand> out;

  if (c.setFilterEnabled) {
    out.push_back(makeCmd(*c.setFilterEnabled ? UwfmgrKind::FilterEnable : UwfmgrKind::FilterDisable));
  }

  if (c.setOverlay.type) {
    out.push_back(makeCmd(UwfmgrKind::OverlaySetType, {*c.setOverlay.type == core::OverlayType::RAM ? "RAM" : "Disk"}));
  }
  if (c.setOverlay.maximumSizeMb) {
    out.push_back(makeCmd(UwfmgrKind::OverlaySetSize, {std::to_string(*c.setOverlay.maximumSizeMb)}));
  }
  if (c.setOverlay.warningThresholdMb) {
    out.push_back(makeCmd(UwfmgrKind::OverlaySetWarningThreshold, {std::to_string(*c.setOverlay.warningThresholdMb)}));
  }
  if (c.setOverlay.criticalThresholdMb) {
    out.push_back(makeCmd(UwfmgrKind::OverlaySetCriticalThreshold, {std::to_string(*c.setOverlay.criticalThresholdMb)}));
  }

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
  out.push_back(makeCmd(UwfmgrKind::OverlaySetType, {s.overlay.type == core::OverlayType::RAM ? "RAM" : "Disk"}));
  out.push_back(makeCmd(UwfmgrKind::OverlaySetSize, {std::to_string(s.overlay.maximumSizeMb)}));
  out.push_back(makeCmd(UwfmgrKind::OverlaySetWarningThreshold, {std::to_string(s.overlay.warningThresholdMb)}));
  out.push_back(makeCmd(UwfmgrKind::OverlaySetCriticalThreshold, {std::to_string(s.overlay.criticalThresholdMb)}));

  for (const auto& v : s.volumes) {
    if (v.driveLetter.empty()) continue;
    out.push_back(makeCmd(v.isProtected ? UwfmgrKind::VolumeProtect : UwfmgrKind::VolumeUnprotect, {v.driveLetter}));
  }

  // fileExclusions 的 key 是 volumeName（"\\?\Volume{...}"），但 add-exclusion
  // 的 path 已带盘符（约定），盘符回查就不必做了。
  for (const auto& [vname, paths] : s.fileExclusions) {
    (void)vname;
    for (const auto& p : paths) out.push_back(makeCmd(UwfmgrKind::FileAddExclusion, {p}));
  }
  for (const auto& k : s.registryExclusions) out.push_back(makeCmd(UwfmgrKind::RegistryAddExclusion, {k}));

  return out;
}

}  // namespace uwf::api
