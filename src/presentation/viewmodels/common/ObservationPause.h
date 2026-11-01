// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <functional>
#include <utility>

namespace uwf::presentation {
// A receiver-thread lease. Releasing the final lease lets its owner resume
// observation; the callback must tolerate the owner being destroyed first.
class ObservationPause final {
 public:
  ObservationPause() = default;
  explicit ObservationPause(std::function<void()> resume) : m_resume(std::move(resume)) {}
  ~ObservationPause() { release(); }
  ObservationPause(const ObservationPause&) = delete;
  ObservationPause& operator=(const ObservationPause&) = delete;
  ObservationPause(ObservationPause&& other) noexcept : m_resume(std::exchange(other.m_resume, {})) {}
  ObservationPause& operator=(ObservationPause&& other) noexcept {
    if (this != &other) {
      release();
      m_resume = std::exchange(other.m_resume, {});
    }
    return *this;
  }

 private:
  void release() {
    if (auto resume = std::exchange(m_resume, {})) resume();
  }
  std::function<void()> m_resume;
};
}  // namespace uwf::presentation
