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

// 字节 / MB 数值 → 人类可读字符串的统一格式化。
//   formatBytes  接 uint64_t 原始字节数（用于 DiskTab / OverlayFilesDialog 等
//                按 B 计的场景）。
//   formatMb     接 uint32_t MB 数（用于 GlobalStatusPanel 显示 RAM / overlay
//                上限——常见 8/16/32/64 GB 这类整除值会落到整数分支输出更短）。
// 不依赖 Qt，core / uwf / ui 各层均可用。

#include <cstdint>
#include <string>

namespace uwf {

// bytes → "X B" / "X.X KB" / "X.X MB" / "X.XX GB" / "X.XX TB"。
// 选最高 ≤ bytes 的单位；KB / MB 保留 1 位小数，GB / TB 保留 2 位（GB 数量级
// 起常见尾数较细，多一位更准）。
[[nodiscard]] std::string formatBytes(uint64_t bytes);

// MB → "X MB" / "X GB" / "X TB" / "X PB"。挑最紧凑的单位：能整除就整数显示，
// 否则保留 2 位小数。常见 RAM 容量（8/16/32/64 GB）总是落到整数分支输出最短。
[[nodiscard]] std::string formatMb(uint32_t mb);

}  // namespace uwf
