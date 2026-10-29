// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

// uwfmgr CLI ↔ UWF API state 互转。
//
//   render*       —— core::PendingChanges / core::SessionSnapshot → uwfmgr 命令序列
//   parseUwfmgrText —— 文本 → 结构化 UwfmgrCommand（带 cat/verb/args + 错误码）
//   renderCommand   —— 单条 UwfmgrCommand → "uwfmgr.exe ..." 字符串
//
// 接口使用标准库类型；ParseError 保留错误分类，由调用方生成本地化说明。
// args 里的字符串就是字面量（路径含空格也不带引号）。renderCommand/Lines
// 用于本程序可逆的文本表示；只有 renderCommandScript 的完整输出可作为 CMD 批处理执行。

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "core/UwfModel.h"

namespace uwf::app {

inline constexpr std::string_view kUwfmgrScriptMarker = ":: UWF Manager CMD script (UTF-8)";

// uwfmgr 一条命令的语义类别。Unknown 留给 parser 在解析失败时使用。
enum class UwfmgrKind : uint8_t {
  Unknown,
  FilterEnable,
  FilterDisable,
  OverlaySetType,               // arg[0] = "RAM" / "Disk"
  OverlaySetSize,               // arg[0] = MB（十进制字符串）
  OverlaySetWarningThreshold,   // arg[0] = MB
  OverlaySetCriticalThreshold,  // arg[0] = MB
  VolumeProtect,                // arg[0] = "C:" 形式盘符
  VolumeUnprotect,              // arg[0] = "C:" 形式盘符
  FileAddExclusion,             // arg[0] = 带盘符的绝对路径
  FileRemoveExclusion,          // arg[0] = 带盘符的绝对路径
  RegistryAddExclusion,         // arg[0] = 注册表键
  RegistryRemoveExclusion,      // arg[0] = 注册表键
};

// parser 给出的"为什么没识别成功"。i18n 翻译留给 UI 层。
enum class ParseError : uint8_t {
  None,                   // 成功识别
  Comment,                // 注释行 / 空行——caller 应静默跳过
  Incomplete,             // 命令缺 verb / 子命令
  MissingSizeArg,         // overlay set-* 缺 MB 参数
  InvalidSize,            // overlay set-* 的参数不是非负整数
  MissingTypeArg,         // overlay set-type 缺类型参数
  UnknownType,            // overlay set-type 既非 RAM 也非 Disk；errorContext = 用户写的值
  MissingVolumeArg,       // volume protect/unprotect 缺盘符
  InvalidVolume,          // volume protect/unprotect 的参数不是合法盘符；errorContext = 用户写的值
  MissingPathArg,         // file add/remove-exclusion 缺路径
  MissingRegistryKeyArg,  // registry add/remove-exclusion 缺键
  MalformedQuoting,       // 双引号没有成对闭合
  UnexpectedArgument,     // 已识别命令后还有多余参数；errorContext = 第一项多余参数
  Unsupported,            // 整体命令模式没识别（commit-* / get-config 这种 UI 不映射的命令）
};

// 一条命令——既用于 render（API → CLI），也用于 parse（CLI → API）。
//   - render 路径：填 kind + args；调 renderCommand 拼成字符串
//   - parse 路径：parser 还会填 sourceLineNo / rawLine / parseError / parseErrorContext，
//     方便 UI 把错误回放到原始行号 + 上下文
struct UwfmgrCommand {
  UwfmgrKind kind = UwfmgrKind::Unknown;
  std::vector<std::string> args;
  bool requiresPrevious = false;

  // 仅 parse 路径有意义。render 路径默认 0/空。
  int sourceLineNo = 0;  // 1-based
  std::string rawLine;   // 原始输入行，保留首尾空格
  ParseError parseError = ParseError::None;
  std::string parseErrorContext;  // 错误模板里要插的字符串（如未知类型的实际值）
};

// 把一条 UwfmgrCommand 拼成可逆的 "uwfmgr.exe ..." 字面文本；不是可执行的
// shell 合同（CMD 交互命令和批处理的百分号展开规则不同）。
[[nodiscard]] std::string renderCommand(const UwfmgrCommand& cmd);

// 依赖命令通过 && 合并在同一行，保留前置失败时的跳过语义。
[[nodiscard]] std::vector<std::string> renderCommandLines(const std::vector<UwfmgrCommand>& commands);

// 生成可执行、可重新导入的完整 UTF-8 .cmd 脚本。封装禁用延迟变量展开，
// 临时切换 UTF-8 代码页并在结束时恢复，保留最后一条命令的退出码。
[[nodiscard]] std::string renderCommandScript(const std::vector<UwfmgrCommand>& commands);

// 逐行解析命令及 && 依赖链；引号与 caret 转义内的 && 保留为参数。
// 注释行 (#, ::, //) 和空行的 parseError = Comment，由调用方跳过。
// 仅在 renderCommandScript 的完整规范封装内按批处理规则还原 %%。
[[nodiscard]] std::vector<UwfmgrCommand> parseUwfmgrText(const std::string& text);

// 将可由 CLI 表达的变更按筛选器、覆盖层、卷、文件排除和注册表排除顺序生成。
// 覆盖层顺序依据写入前基线规划；绑定方式及注册表持久化开关无对应命令。
[[nodiscard]] std::vector<UwfmgrCommand> renderPendingChanges(const core::PendingChanges& changes, const core::OverlayConfig& baseline);

// 生成重现指定会话状态的命令。无盘符卷无法通过 CLI 定位，将被跳过。
[[nodiscard]] std::vector<UwfmgrCommand> renderSession(const core::SessionSnapshot& session);

// 以下次会话为基线，合并待应用变更；同一配置目标只输出最终命令，移除命令
// 仍保留，以便在已有配置上回放。无法用 CLI 表达的字段沿用 renderPendingChanges 的约定。
[[nodiscard]] std::vector<UwfmgrCommand> renderPlannedSession(const core::SessionSnapshot& baseline, const core::PendingChanges& changes);

}  // namespace uwf::app
