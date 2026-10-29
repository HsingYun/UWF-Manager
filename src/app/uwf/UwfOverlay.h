// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
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

#include "UwfProviderTypes.h"
#include "app/ports/WmiOperations.h"

namespace uwf::app::provider {

class UwfOverlay {
 public:
  explicit UwfOverlay(wmi::WmiOperations& session) : m_session(session) {}

  [[nodiscard]] provider::OverlayRow read() const;

  // 仅 NTFS 卷受支持。volume 可以是盘符（如 "C:"）或卷名。
  std::vector<provider::OverlayFileRow> getOverlayFiles(const provider::OverlayRow& row, const std::string& volume, std::stop_token stopToken = {}) const;

  void setWarningThreshold(const provider::OverlayRow& row, uint32_t sizeMb) const;
  void setCriticalThreshold(const provider::OverlayRow& row, uint32_t sizeMb) const;

 private:
  wmi::WmiOperations& m_session;
};

}  // namespace uwf::app::provider
