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
// 写接口：Enable / Disable / ShutdownSystem / RestartSystem。
// 所有写接口都真实调用 WMI ExecMethod（非 dry-run）。

#include "../wmi/WmiClient.h"
#include "Types.h"

namespace uwf::api {

class UwfFilter {
 public:
  explicit UwfFilter(WmiOperations& session) : m_session(session) {}

  [[nodiscard]] api::FilterRow read() const;

  // 在下次重启时启用 UWF。
  void enable(const api::FilterRow& row) const;
  // 在下次重启时禁用 UWF。
  void disable(const api::FilterRow& row) const;
  // 安全关闭受 UWF 保护的系统（即使 overlay 已满）。
  void shutdownSystem(const api::FilterRow& row) const;
  // 安全重启受 UWF 保护的系统（即使 overlay 已满）。
  void restartSystem(const api::FilterRow& row) const;

 private:
  WmiOperations& m_session;
};

}  // namespace uwf::api
