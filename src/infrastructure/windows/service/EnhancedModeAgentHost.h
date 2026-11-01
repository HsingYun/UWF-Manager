// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <windows.h>

#include <cstdint>
#include <memory>

namespace uwf::windows {
class PreshutdownCommitGate;

// Owns agent authentication, connection and supervision as one lifecycle.
// The gate must outlive the host. Stop joins supervision before the listener;
// failureEvent remains valid until destruction and signals an infrastructure failure.
class EnhancedModeAgentHost final {
 public:
  explicit EnhancedModeAgentHost(PreshutdownCommitGate& commitGate);
  ~EnhancedModeAgentHost();
  EnhancedModeAgentHost(const EnhancedModeAgentHost&) = delete;
  EnhancedModeAgentHost& operator=(const EnhancedModeAgentHost&) = delete;

  void requestAgent();
  void stop();
  [[nodiscard]] HANDLE failureEvent() const noexcept;
  [[nodiscard]] std::uint32_t failureCode() const noexcept;

 private:
  class Impl;
  std::unique_ptr<Impl> m_impl;
};
}  // namespace uwf::windows
