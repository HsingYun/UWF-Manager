// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QObject>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>
#include <thread>

#include "application/enhanced/contracts/EnhancedModeAgentConnection.h"

namespace uwf::windows {
enum class EnhancedAgentControl : std::uint8_t;
}

namespace uwf::windows {

class EnhancedModeAgent final : public app::EnhancedModeAgentConnection {
  Q_OBJECT
 public:
  explicit EnhancedModeAgent(QObject* parent = nullptr);
  ~EnhancedModeAgent() override;

  void start() override;
  void stop() override;
  [[nodiscard]] bool running() const override;
  [[nodiscard]] bool markPreshutdownCommitHandled() override;
  [[nodiscard]] bool requirePreshutdownCommit() override;

 private:
  struct ControlChannel;
  void run(std::stop_token stopToken);
  [[nodiscard]] bool sendControl(EnhancedAgentControl control);
  [[nodiscard]] bool waitBeforeReconnect(std::stop_token stopToken, std::chrono::milliseconds delay);

  std::mutex m_waitMutex;
  std::condition_variable m_waitChanged;
  std::atomic_bool m_workerActive{false};
  std::jthread m_thread;
  std::unique_ptr<ControlChannel> m_control;
};

[[nodiscard]] std::unique_ptr<app::EnhancedModeAgentConnection> createEnhancedModeAgent();

}  // namespace uwf::windows
