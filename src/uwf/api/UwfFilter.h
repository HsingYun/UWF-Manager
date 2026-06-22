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

// UWF_Filter —— 筛选器单例。读 CurrentEnabled / NextEnabled；
// 写接口：Enable / Disable / ResetSettings / ShutdownSystem / RestartSystem。
// 所有写接口都真实调用 WMI ExecMethod（非 dry-run）。

#include <optional>
#include <string>

#include "../wmi/WmiClient.h"
#include "../wmi/WmiResult.h"
#include "Types.h"

namespace uwf::api {

class UwfFilter {
 public:
  explicit UwfFilter(WmiSession& session) : m_session(session) {}

  // 查询 UWF_Filter 单例；没有行或查询失败返回 nullopt。
  [[nodiscard]] std::optional<api::FilterRow> read(std::string* error = nullptr) const;

  // 在下次重启时启用 UWF。
  [[nodiscard]] WmiResult enable(const api::FilterRow& row) const;
  // 在下次重启时禁用 UWF。
  [[nodiscard]] WmiResult disable(const api::FilterRow& row) const;
  // 将 UWF 配置重置为原始设置。
  [[nodiscard]] WmiResult resetSettings(const api::FilterRow& row) const;
  // 安全关闭受 UWF 保护的系统（即使 overlay 已满）。
  [[nodiscard]] WmiResult shutdownSystem(const api::FilterRow& row) const;
  // 安全重启受 UWF 保护的系统（即使 overlay 已满）。
  [[nodiscard]] WmiResult restartSystem(const api::FilterRow& row) const;

 private:
  WmiSession& m_session;
};

}  // namespace uwf::api
