/*
 * Copyright (c) 2026 HsingYun (iakext@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

// uwfmgr CLI ↔ UWF API state 互转。
//
//   render*       —— core::PendingChanges / core::SessionSnapshot → uwfmgr 命令序列
//   parseUwfmgrText —— 文本 → 结构化 UwfmgrCommand（带 cat/verb/args + 错误码）
//   renderCommand   —— 单条 UwfmgrCommand → "uwfmgr.exe ..." 字符串
//
// 不依赖 Qt——错误用 ParseError 枚举返回，调用方（UI）按 enum 自己 i18n 翻译。
// args 里的字符串就是 CLI 看到的字面量（路径含空格也不带引号；renderCommand
// 时按 CLI 引用规则自动加引号）。

#include <cstdint>
#include <string>
#include <vector>

#include "../../core/UwfModel.h"

namespace uwf::api {

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

  // 仅 parse 路径有意义。render 路径默认 0/空。
  int sourceLineNo = 0;  // 1-based
  std::string rawLine;   // 原始行（trim 过左右空格）
  ParseError parseError = ParseError::None;
  std::string parseErrorContext;  // 错误模板里要插的字符串（如未知类型的实际值）
};

// 把一条 UwfmgrCommand 拼成可执行的 "uwfmgr.exe ..." 字符串。args 里含空格的
// （如带空格的路径）会自动加双引号；其它字面量原样拼接。
[[nodiscard]] std::string renderCommand(const UwfmgrCommand& cmd);

// 按行解析整段文本：每一非空非注释行产生一条 UwfmgrCommand；注释行
// (#, ::, //) 和空行的 parseError = Comment 由 caller 静默跳过。CRLF 安全。
[[nodiscard]] std::vector<UwfmgrCommand> parseUwfmgrText(const std::string& text);

// 把 PendingChanges 里所有 .has_value() 的字段渲染成对应的 uwfmgr 命令；
// 顺序：filter → overlay (type/size/warning/critical) → volume protect →
// file add → file remove → registry add → registry remove。
//
// volumeBindByVolumeName 没有 CLI 对应，**不会**产生命令。UI 想给"no CLI
// equivalent"提示得自己处理（因为 PendingChanges 里这个字段在 / 不在它都
// 知道，不需要靠这个返回值传达）。
[[nodiscard]] std::vector<UwfmgrCommand> renderPendingChanges(const core::PendingChanges& changes);

// 把一份 SessionSnapshot 渲染成"用 uwfmgr 命令重现该状态"的命令序列。
// 用 current session 通常更直观（"现在 UWF 实际在跑什么"），用 next 更精确。
// vol 列表里 driveLetter 为空的会被跳过（uwfmgr 命令需要盘符定位）。
[[nodiscard]] std::vector<UwfmgrCommand> renderSession(const core::SessionSnapshot& session);

}  // namespace uwf::api
