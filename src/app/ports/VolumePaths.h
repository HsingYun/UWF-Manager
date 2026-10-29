// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

// 不依赖 Qt 的盘符与卷路径工具；纯字符串操作与系统挂载查询分别提供。

#include <functional>
#include <stdexcept>
#include <string>
#include <vector>

#include "shared/DriveLetter.h"

namespace uwf::drive {

using MountLookup = std::function<std::vector<std::string>(const std::string&)>;

class DriveLetterResolutionError final : public std::runtime_error {
 public:
  using std::runtime_error::runtime_error;
};

// 查询卷 GUID 路径（"\\?\Volume{GUID}\"）当前挂载的全部盘符。
// 只返回盘符根挂载点，不把目录挂载点误当成其宿主盘符。卷已不存在或没有
// 盘符时返回空列表；权限、I/O 和协议错误抛出异常。
[[nodiscard]] std::vector<std::string> mountedLetters(const std::string& volumeGuidPath);

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
