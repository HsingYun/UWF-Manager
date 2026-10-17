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

#include <QString>
#include <optional>

#include "../ui/I18n.h"
#include "../ui/ThemeManager.h"

namespace uwf::app {

struct UserConfig {
  // 未指定或 system 表示启动时跟随系统，不代表当前已解析出的实际值。
  std::optional<ui::Theme> theme;
  std::optional<ui::I18n::Lang> language;
};

// 只读指定应用目录中的 UWF.ini，不创建、修改文件，也不读取注册表或其他
// 位置的配置。缺失配置使用默认值；读取失败或非法配置仅记录警告并回退。
// 目录必须为绝对路径，避免意外读取快捷方式或服务的工作目录。
[[nodiscard]] UserConfig readUserConfig(const QString& applicationDirectory);

}  // namespace uwf::app
