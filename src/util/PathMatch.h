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

// 路径前缀匹配工具：用于"目标 path 是否落在排除列表里"这类判定。
// 文件路径与注册表键路径都用同一套规则——大小写不敏感、分隔符兼容 `\`/`/`、
// 头尾分隔符容忍。不依赖 Qt / Windows，core / uwf / ui 各层都能用。

#include <string>
#include <vector>

namespace uwf {

// 去掉末尾的 `\` 与 `/`。排除项可能写成 "C:\Foo\" 也可能写成 "C:\Foo"——
// 统一后才能做稳定的前缀 / 相等比较。
[[nodiscard]] std::string stripTrailingSep(std::string s);

// 判断 target 是否等于 prefix 或者是它的后代。大小写不敏感（ASCII），
// 分隔符兼容 `\` 与 `/`（注册表键固定 `\`，文件路径在 Windows 下也是 `\`，
// 但用户输入可能含 `/`）。两个参数都假定已 stripTrailingSep。
[[nodiscard]] bool pathIsExcludedBy(const std::string& target, const std::string& prefix);

// 在 excls 里找第一条"覆盖"了 target 的排除项；找不到返回空串。
// 命中时返回**原始**排除字符串（未 stripTrailingSep），方便在错误提示里
// 原样展示给用户（"我加进去的是 C:\Foo\，你给我去掉斜杠不对劲"）。
[[nodiscard]] std::string findCoveringExclusion(const std::vector<std::string>& excls, const std::string& target);

}  // namespace uwf
