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

// UWF_Overlay —— 覆盖层运行时状态（单例）。
//
// 读字段：OverlayConsumption / AvailableSpace / CriticalOverlayThreshold /
//        WarningOverlayThreshold
// 方法：
//   - GetOverlayFiles(Volume) -> UWF_OverlayFile[]：查询某卷当前已缓存的文件
//   - SetWarningThreshold(size)
//   - SetCriticalThreshold(size)

#include <stop_token>
#include <string>
#include <vector>

#include "../wmi/WmiClient.h"
#include "Types.h"

namespace uwf::api {

class UwfOverlay {
 public:
  explicit UwfOverlay(WmiSession& session) : m_session(session) {}

  [[nodiscard]] api::OverlayRow read() const;

  // 仅 NTFS 卷受支持。volume 可以是盘符（如 "C:"）或卷名。
  std::vector<api::OverlayFileRow> getOverlayFiles(const api::OverlayRow& row, const std::string& volume, std::stop_token stopToken = {}) const;

  void setWarningThreshold(const api::OverlayRow& row, uint32_t sizeMb) const;
  void setCriticalThreshold(const api::OverlayRow& row, uint32_t sizeMb) const;

 private:
  WmiSession& m_session;
};

}  // namespace uwf::api
