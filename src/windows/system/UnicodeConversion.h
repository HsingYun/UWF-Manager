// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <string>
#include <string_view>

namespace uwf::windows {

// 项目内部使用 UTF-8；在 Win32 边界转换为 UTF-16。

// UTF-8 → UTF-16。输入为空返回空串；非法编码、长度溢出或 Win32 转换失败抛
// std::exception 的具体子类。
[[nodiscard]] std::wstring utf8ToWide(std::string_view utf8);

// UTF-16 → UTF-8。输入为空返回空串；非法编码、长度溢出或 Win32 转换失败抛
// std::exception 的具体子类。
[[nodiscard]] std::string wideToUtf8(std::wstring_view wide);

// UTF-16 → UTF-8 的便捷重载：接受 NUL 结尾的宽串（BSTR / Win32 输出等），
// 对 nullptr 安全（返回空串）。
[[nodiscard]] std::string wideToUtf8(const wchar_t* wide);

}  // namespace uwf::windows
