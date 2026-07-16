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

// 字符串小工具：ASCII 大小写折叠 / 裁剪，以及 UTF-8 ↔ UTF-16 转换。
//
// 这些操作原先在 core / uwf / wmi / util 各处各写一份（toLowerAscii /
// toUpperAscii / trimmed / utf8ToWide / wideToUtf8）——集中到此单一处。
//
// ASCII 部分（toLowerAscii / toUpperAscii / trim）是纯 C++、可移植；
// UTF-8 ↔ UTF-16 转换在 .cpp 里用 Windows API 实现（头文件仍只是声明，
// 不引入任何平台头）。

#include <string>
#include <string_view>

namespace uwf {

// ── ASCII 大小写折叠与裁剪（纯 C++） ────────────────────────────────────────

// ASCII 大小写折叠：仅 A-Z / a-z 受影响，非 ASCII 字节原样保留。
[[nodiscard]] std::string toLowerAscii(std::string s);
[[nodiscard]] std::string toUpperAscii(std::string s);

// 去掉首尾的 ASCII 空白字符，返回裁剪后的副本。
[[nodiscard]] std::string trim(std::string s);

// ── UTF-8 ↔ UTF-16 转换 ─────────────────────────────────────────────────────
// Windows API 几乎都收 / 发 wchar_t，而项目内字符串一律以 UTF-8 std::string
// 流转，转换在每个调用 Win32 的边界处反复需要。

// UTF-8 → UTF-16。输入为空返回空串；非法编码、长度溢出或 Win32 转换失败抛
// std::exception 的具体子类。
[[nodiscard]] std::wstring utf8ToWide(std::string_view utf8);

// UTF-16 → UTF-8。输入为空返回空串；非法编码、长度溢出或 Win32 转换失败抛
// std::exception 的具体子类。
[[nodiscard]] std::string wideToUtf8(std::wstring_view wide);

// UTF-16 → UTF-8 的便捷重载：接受 NUL 结尾的宽串（BSTR / Win32 输出等），
// 对 nullptr 安全（返回空串）。
[[nodiscard]] std::string wideToUtf8(const wchar_t* wide);

}  // namespace uwf
