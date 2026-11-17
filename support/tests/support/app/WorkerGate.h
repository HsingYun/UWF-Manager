// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <stop_token>

namespace uwf::test {
// Capture shared ownership in background callbacks. Every wait is bounded even
// if a test assertion exits before it can release or cancel the worker.
class WorkerGate final {
 public:
  void wait(const std::stop_token stop = {}, const bool honorCancellation = true) {
    ++started;
    std::unique_lock lock(m_mutex);
    const auto deadline = std::chrono::steady_clock::now() + kWaitLimit;
    if (honorCancellation)
      m_changed.wait_until(lock, stop, deadline, [this] { return m_released; });
    else
      m_changed.wait_until(lock, deadline, [this] { return m_released; });
    if (stop.stop_requested()) ++canceled;
    ++finished;
  }
  void release() {
    {
      const std::scoped_lock lock(m_mutex);
      m_released = true;
    }
    m_changed.notify_all();
  }
  void reset() {
    const std::scoped_lock lock(m_mutex);
    m_released = false;
  }
  std::atomic_int started{0};
  std::atomic_int canceled{0};
  std::atomic_int finished{0};

 private:
  static constexpr auto kWaitLimit = std::chrono::seconds{10};
  std::mutex m_mutex;
  std::condition_variable_any m_changed;
  bool m_released = false;
};
}  // namespace uwf::test
