// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <memory>

#include "application/overlay/contracts/OverlayPolicyActions.h"
#include "application/overlay/contracts/OverlayPolicyOperations.h"
#include "domain/uwf/contracts/Backend.h"

namespace uwf::app {
// Adapt overlay-policy operations to UWF requests; routing remains behind the supplied port.
class UwfOverlayOperations final : public OverlayPolicyOperations {
 public:
  explicit UwfOverlayOperations(std::shared_ptr<compat::Uwf> system);
  [[nodiscard]] OverlayPolicySnapshot snapshot() const override;
  [[nodiscard]] static OverlayPolicySnapshot snapshot(const compat::UwfStateFacts& state);
  [[nodiscard]] WriteEffect setOption(OverlayOption option, bool enabled) override;
  [[nodiscard]] WriteEffect replace(OverlayProperty node, std::int64_t value) override;

 private:
  [[nodiscard]] WriteEffect write(const compat::Request& request, OverlayProperty node);
  std::shared_ptr<compat::Uwf> m_system;
};
[[nodiscard]] std::shared_ptr<OverlayPolicyActions> uwfOverlayPolicy(std::shared_ptr<compat::Uwf> system);
}  // namespace uwf::app
