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

// 盘符工具：规范化盘符字符串、按盘符拆分路径、从路径解析盘符、取系统盘——
// 相关逻辑全部集中在此一处，避免散落各层各写一份、且处理口径不一致。
//
// 不依赖 Qt，core / uwf / ui 各层均可使用（UI 层在边界处做 QString 转换）。

#include <stdexcept>
#include <string>

namespace uwf::drive {

class DriveLetterResolutionError final : public std::runtime_error {
 public:
  using std::runtime_error::runtime_error;
};

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
//   "\\?\C:\\Users\\foo"    → {"C:", "\\Users\\foo"}   扩展长度前缀一并吃掉
//   "C:"                     → {"C:", ""}
//   "\\Users\\foo" / "foo"   → {"",   <去扩展前缀后原样>}   无盘符——卷内相对路径
//   "\\?\Volume{GUID}\\foo"  → {"",   ...}                只做字面拆分，不查卷 GUID
// 需要把卷 GUID 路径反查成盘符，用 fromPath。
PathSplit split(const std::string& path);

// 从一个文件路径解析出所在卷的盘符（如 "C:"）。
//   "C:\\x" / "\\?\C:\\x"            —— 字面拆分即得 "C:"
//   "\\?\Volume{GUID}\\x"            —— 字符串里没有盘符，经 Win32 API 反查
//                                       该卷挂载的盘符
// 返回规范盘符（"C:"）。路径本就没有盘符（UNC 路径、卷内相对路径）时
// 返回空串，这是正常业务结果。字面上是卷 GUID 路径却畸形或无法解析时，
// 抛出 DriveLetterResolutionError；底层 Win32 查询失败抛 std::system_error。
// 不再用空串 + error out-parameter 表达失败。
std::string fromPath(const std::string& path);

// 当前系统盘（Windows 所在卷）的盘符，如 "C:"。
// 取不到时返回空串——不臆测 "C:"，由调用方自行处理"系统盘未知"。
std::string systemLetter();

}  // namespace uwf::drive
