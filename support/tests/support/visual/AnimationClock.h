// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QAbstractAnimation>
#include <QCoreApplication>
#include <algorithm>

namespace uwf::test {
class AnimationClock final : public QAnimationDriver {
 public:
  AnimationClock() { install(); }
  ~AnimationClock() override {
    QCoreApplication::sendPostedEvents();
    uninstall();
  }
  void advanceBy(const int milliseconds) {
    // Qt queues animation registration and timer registration separately.
    QCoreApplication::sendPostedEvents();
    QCoreApplication::sendPostedEvents();
    m_total += milliseconds;
    int remaining = milliseconds;
    while (remaining > 0 && isRunning()) {
      const int step = std::min(16, remaining);
      m_elapsed += step;
      advanceAnimation();
      remaining -= step;
      QCoreApplication::sendPostedEvents();
      QCoreApplication::sendPostedEvents();
    }
    QCoreApplication::sendPostedEvents();
  }
  [[nodiscard]] qint64 elapsed() const override { return m_elapsed; }
  [[nodiscard]] qint64 totalTime() const { return m_total; }

 protected:
  void start() override {
    m_elapsed = 0;
    QAnimationDriver::start();
  }

 private:
  qint64 m_elapsed = 0;
  qint64 m_total = 0;
};
}  // namespace uwf::test
