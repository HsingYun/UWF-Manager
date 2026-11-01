// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <memory>
#include <mutex>

#include "application/overlay/contracts/OverlayPolicy.h"
#include "application/overlay/contracts/OverlayPolicyActions.h"
#include "application/overlay/contracts/OverlayPolicyOperations.h"

namespace uwf::app {
// Owns draft validation and command ordering using the published global state.
// Apply records command receipts without querying prerequisites or reading a command back.
// The operations port hides every platform/backend detail from this service.
class OverlayPolicyService final : public OverlayPolicyActions {
 public:
  explicit OverlayPolicyService(std::shared_ptr<OverlayPolicyOperations> operations);
  [[nodiscard]] OverlayPolicySnapshot snapshot() const override;
  [[nodiscard]] OverlayPolicyResult apply(const OverlayPolicyUpdate& update) override;

 private:
  std::shared_ptr<OverlayPolicyOperations> m_operations;
  mutable std::mutex m_mutex;
};
}  // namespace uwf::app
