// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <windows.h>

#include <atomic>
#include <cstdint>
#include <functional>
#include <mutex>

#include "application/staging/contracts/FileStagingTypes.h"

namespace uwf::windows {
using PreshutdownCommit = std::function<app::FileStagingCommitResult()>;
class PreshutdownCommitGate;

enum class ServiceShutdownEvent { Stop, AgentFailure, Preshutdown };

// Preserve stop/failure precedence when multiple native events are pending.
ServiceShutdownEvent waitForServiceShutdown(HANDLE stop, HANDLE agentFailure, HANDLE preshutdown);

// The native adapter owns handles and joins its agent threads in stopAgent.
struct EnhancedModeServiceEnvironment {
  std::function<void()> startAgent;
  std::function<void()> requestAgent;
  std::function<void()> stopAgent;
  std::function<std::uint32_t()> agentFailureCode;
  std::function<ServiceShutdownEvent()> waitForShutdown;
  std::function<void()> signalStop;
  std::function<void()> signalPreshutdown;
  std::function<void(const SERVICE_STATUS&)> publishStatus;
};

// Owns control admission, SCM status and shutdown sequencing. The gate and
// environment dependencies must outlive run() and any concurrent control call.
class EnhancedModeServiceRuntime final {
 public:
  EnhancedModeServiceRuntime(PreshutdownCommit commit, PreshutdownCommitGate& gate, EnhancedModeServiceEnvironment environment);
  void run();
  DWORD handleControl(DWORD control, DWORD eventType = 0);

 private:
  struct StatusUpdate {
    DWORD state;
    DWORD controls = 0;
    DWORD checkpoint = 0;
    DWORD waitHint = 0;
  };
  void report(const StatusUpdate& update);
  void publishCurrentStatus();
  void reportStopped(DWORD exitCode) noexcept;
  DWORD nextCheckpoint();
  void reportRealProgress();
  void runPreshutdownCommit();

  PreshutdownCommit m_commit;
  PreshutdownCommitGate& m_gate;
  EnhancedModeServiceEnvironment m_environment;
  SERVICE_STATUS m_status{};
  std::mutex m_statusMutex;
  std::atomic_uint32_t m_checkpoint{1};
  std::atomic_bool m_preshutdownStarted{false};
};

// Enter the Windows SCM dispatcher before creating the desktop application.
// The host owns SCM, IPC and worker lifetimes; application work is injected.
int runEnhancedModeService(PreshutdownCommit commit);
}  // namespace uwf::windows
