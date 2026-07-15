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

#include <optional>
#include <stop_token>
#include <string>
#include <vector>

#include "../wmi/WmiClient.h"
#include "../wmi/WmiResult.h"
#include "Types.h"

namespace uwf::api {

class UwfOverlay {
 public:
  explicit UwfOverlay(WmiSession& session) : m_session(session) {}

  [[nodiscard]] std::optional<api::OverlayRow> read(std::string* error = nullptr) const;

  // 仅 NTFS 卷受支持。volume 可以是盘符（如 "C:"）或卷名。
  // errorCode 输出参数：ExecMethod 失败时返回原始 HRESULT；调用已经抵达
  // provider 时返回 UWF 方法的 UInt32 ReturnValue。让 caller 无需猜测错误来自
  // 哪条通道即可按命名常量分支（例如 RPC_E_SERVERFAULT 时建议重试）。
  std::vector<api::OverlayFileRow> getOverlayFiles(const api::OverlayRow& row, const std::string& volume, std::string* error = nullptr,
                                                   int32_t* errorCode = nullptr, std::stop_token stopToken = {}) const;

  [[nodiscard]] WmiResult setWarningThreshold(const api::OverlayRow& row, uint32_t sizeMb) const;
  [[nodiscard]] WmiResult setCriticalThreshold(const api::OverlayRow& row, uint32_t sizeMb) const;

 private:
  WmiSession& m_session;
};

}  // namespace uwf::api
