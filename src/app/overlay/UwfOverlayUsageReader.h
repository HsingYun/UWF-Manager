// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include "app/api/OverlayUsageReader.h"
#include "app/uwf/UwfFilter.h"
#include "app/uwf/UwfOverlay.h"
#include "app/uwf/UwfOverlayConfig.h"
namespace uwf::app {
class UwfOverlayUsageReader final : public OverlayUsageReader {
 public:
  explicit UwfOverlayUsageReader(wmi::WmiOperations& session) : m_session(session.threadAccessor()) {}
  [[nodiscard]] OverlayUsageState read() const override;

 private:
  std::function<wmi::WmiOperations&()> m_session;
};
}  // namespace uwf::app
