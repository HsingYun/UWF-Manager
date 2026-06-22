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

// SystemCheck：启动时的前置检查。
// 返回的结果是"结构化"的——具体的中文描述由 UI 层（main.cpp）负责组合，
// 这样核心层不引入任何显示相关的文本。

#include <string>

namespace uwf {

enum class CheckStatus {
  Ok,                  // 一切就绪
  UnsupportedEdition,  // 不是 Enterprise / Education / IoT Enterprise
};

struct SystemCheckResult {
  CheckStatus status = CheckStatus::Ok;
  std::string editionId;    // 读自注册表 EditionID，比如 "Enterprise"
  std::string productName;  // 读自注册表 ProductName，比如 "Windows 11 Enterprise"
};

SystemCheckResult runSystemChecks();
bool isElevated();
std::string uwfmgrPath();

}  // namespace uwf
