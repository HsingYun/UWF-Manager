// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

// UWF_Filter 的配置与安全电源接口；写操作调用真实 WMI 方法。

#include "UwfProviderTypes.h"
#include "app/ports/WmiOperations.h"

namespace uwf::app::provider {

class UwfFilter {
 public:
  explicit UwfFilter(wmi::WmiOperations& session) : m_session(session) {}

  [[nodiscard]] provider::FilterRow read() const;

  // 在下次重启时启用 UWF。
  void enable(const provider::FilterRow& row) const;
  // 在下次重启时禁用 UWF。
  void disable(const provider::FilterRow& row) const;
  // 安全关闭受 UWF 保护的系统（即使 overlay 已满）。
  void shutdownSystem(const provider::FilterRow& row) const;
  // 安全重启受 UWF 保护的系统（即使 overlay 已满）。
  void restartSystem(const provider::FilterRow& row) const;

 private:
  wmi::WmiOperations& m_session;
};

}  // namespace uwf::app::provider
