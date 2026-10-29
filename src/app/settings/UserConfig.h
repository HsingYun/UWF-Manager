// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QString>
#include <optional>

#include "app/settings/UiPreferences.h"

namespace uwf::app {

struct UserConfig {
  // 未指定或 system 表示启动时跟随系统，不代表当前已解析出的实际值。
  std::optional<Theme> theme;
  std::optional<LanguageId> language;
  bool hubVisible = true;
};

// 目录必须为绝对路径，避免意外读取快捷方式或服务的工作目录。
[[nodiscard]] QString userConfigPath(const QString& applicationDirectory);
// 只读指定应用目录中的 UWF.ini，不创建、修改文件，也不读取注册表或其他
// 位置的配置。缺失配置使用默认值；读取失败或非法配置仅记录警告并回退。
[[nodiscard]] UserConfig readUserConfig(const QString& applicationDirectory);
// Produce the existing INI contract without creating storage. Unspecified overrides remain "system".
[[nodiscard]] QString serializeUserConfig(const UserConfig& config);

}  // namespace uwf::app
