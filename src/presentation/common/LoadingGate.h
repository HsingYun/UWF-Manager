// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QElapsedTimer>
#include <QObject>
#include <QTimer>
#include <chrono>
#include <cstdint>
#include <functional>

namespace uwf::presentation {
class LoadingGate final : public QObject {
 public:
  using Ticket = std::uint64_t;
  explicit LoadingGate(QObject* parent);
  [[nodiscard]] Ticket begin(std::chrono::milliseconds revealDelay = {}, std::function<void()> onReveal = {});
  [[nodiscard]] bool isCurrent(Ticket ticket) const;
  [[nodiscard]] bool isActive() const { return m_active; }
  [[nodiscard]] bool progressVisible() const { return m_active && m_progressVisible; }
  void publish(Ticket ticket, std::function<void()> completion);
  void cancel();

 private:
  void finish();
  QTimer m_timer;
  QTimer m_revealTimer;
  QElapsedTimer m_elapsed;
  std::function<void()> m_completion;
  Ticket m_ticket = 0;
  bool m_active = false;
  bool m_progressVisible = false;
};
}  // namespace uwf::presentation
