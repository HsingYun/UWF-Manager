// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <chrono>
#include <mutex>
#include <optional>

namespace uwf::windows {
// One expiring UI-precommit receipt. Consumption clears even an expired receipt;
// revocation forces the service to perform its own shutdown commit.
class PreshutdownCommitGate final {
 public:
  using Clock = std::chrono::steady_clock;

  void markHandled(const Clock::time_point now = Clock::now()) {
    const std::scoped_lock lock(m_mutex);
    m_deadline = now + std::chrono::minutes{5};
  }
  void requireCommit() {
    const std::scoped_lock lock(m_mutex);
    m_deadline.reset();
  }
  [[nodiscard]] bool consumeHandled(const Clock::time_point now = Clock::now()) {
    const std::scoped_lock lock(m_mutex);
    if (!m_deadline) return false;
    const bool valid = now <= *m_deadline;
    m_deadline.reset();
    return valid;
  }

 private:
  std::mutex m_mutex;
  std::optional<Clock::time_point> m_deadline;
};
}  // namespace uwf::windows
