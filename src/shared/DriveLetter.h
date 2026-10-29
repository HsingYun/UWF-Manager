// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <string>

namespace uwf::drive {

// split() 的结果：把路径拆成"盘符 + 卷内剩余路径"。
struct PathSplit {
  std::string letter;  // 规范盘符（"C:"）；路径没有字面盘符前缀时为空。
  std::string rest;    // 盘符之后的部分（卷内路径，如 "\\Users\\foo"）；没有盘符
                       // 前缀时是去掉扩展长度前缀后的整条路径。
};

// 校验并规范化"盘符字符串"。接受：
//   "" / "c" / "C:"                  —— 裸盘符
//   "\\?\C:" / "\\.\C:"              —— 带扩展长度前缀的裸盘符
//   "C:\\Windows" / "\\?\C:\\x"      —— 带路径残留（取盘符头，忽略其后）
// 输出统一为单个 ASCII 大写字母加冒号（"C:"）。
// 不含盘符的输入（卷 GUID 路径、UNC 路径、卷内相对路径、空串等）一律返回
// 空串。纯字符串处理，不访问系统。
std::string normalize(const std::string& raw);

// 把路径按"字面盘符前缀"拆开（纯字符串处理，不访问系统）：
//   "C:\\Users\\foo"        → {"C:", "\\Users\\foo"}
//   "\\?\C:\\Users\\foo"    → {"C:", "\\Users\\foo"}   移除扩展长度前缀
//   "C:"                     → {"C:", ""}
//   "\\Users\\foo" / "foo"   → {"",   <去扩展前缀后原样>}   无盘符——卷内相对路径
//   "\\?\Volume{GUID}\\foo"  → {"",   ...}                只做字面拆分，不查卷 GUID
// 需要把卷 GUID 路径反查成盘符，用 fromPath。
PathSplit split(const std::string& path);

}  // namespace uwf::drive
