// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "LoadingGate.h"

#include <chrono>
#include <utility>

#include "PresentationTiming.h"

namespace uwf::presentation {
LoadingGate::LoadingGate(QObject* parent) : QObject(parent), m_timer(this), m_revealTimer(this) {
  m_timer.setSingleShot(true);
  m_timer.setTimerType(Qt::PreciseTimer);
  connect(&m_timer, &QTimer::timeout, this, &LoadingGate::finish);
  m_revealTimer.setSingleShot(true);
  m_revealTimer.setTimerType(Qt::PreciseTimer);
}
LoadingGate::Ticket LoadingGate::begin(const std::chrono::milliseconds revealDelay, std::function<void()> onReveal) {
  cancel();
  m_active = true;
  m_elapsed.start();
  m_progressVisible = revealDelay <= std::chrono::milliseconds::zero();
  disconnect(&m_revealTimer, nullptr, this, nullptr);
  if (!m_progressVisible) {
    connect(&m_revealTimer, &QTimer::timeout, this, [this, onReveal = std::move(onReveal)] {
      m_progressVisible = true;
      m_elapsed.restart();
      if (onReveal) onReveal();
    });
    m_revealTimer.start(revealDelay);
  }
  return m_ticket;
}
bool LoadingGate::isCurrent(const Ticket ticket) const { return ticket == m_ticket; }
void LoadingGate::publish(const Ticket ticket, std::function<void()> completion) {
  if (!m_active || !isCurrent(ticket) || m_completion) return;
  m_completion = std::move(completion);
  if (!m_progressVisible) {
    m_revealTimer.stop();
    finish();
    return;
  }
  const auto remaining = timing::kMinimumProgressDisplayDuration - std::chrono::milliseconds{m_elapsed.elapsed()};
  if (remaining > std::chrono::milliseconds::zero())
    m_timer.start(remaining);
  else
    finish();
}
void LoadingGate::cancel() {
  ++m_ticket;
  m_active = false;
  m_timer.stop();
  m_revealTimer.stop();
  m_completion = {};
  m_elapsed.invalidate();
}
void LoadingGate::finish() {
  if (!m_active) return;
  auto completion = std::move(m_completion);
  m_active = false;
  m_elapsed.invalidate();
  if (completion) completion();
}
}  // namespace uwf::presentation
