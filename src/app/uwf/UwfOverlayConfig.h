// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

// UWF_OverlayConfig —— 按 CurrentSession 存在 2 个实例
// （true 为当前会话，false 为下次会话）。
//
// 字段：Type / MaximumSize
// 方法：SetType(UInt32) / SetMaximumSize(UInt32)

#include <vector>

#include "UwfProviderTypes.h"
#include "app/ports/WmiOperations.h"

namespace uwf::app::provider {

class UwfOverlayConfig {
 public:
  explicit UwfOverlayConfig(wmi::WmiOperations& session) : m_session(session) {}

  // 读取全部实例（通常 2 条）。
  std::vector<provider::OverlayConfigRow> readAll() const;

  [[nodiscard]] provider::OverlayConfigRow read(provider::Session session) const;

  void setType(const provider::OverlayConfigRow& row, provider::OverlayType type) const;
  void setMaximumSize(const provider::OverlayConfigRow& row, uint32_t sizeMb) const;

 private:
  wmi::WmiOperations& m_session;
};

}  // namespace uwf::app::provider
