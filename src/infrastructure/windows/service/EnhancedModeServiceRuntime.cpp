// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "infrastructure/windows/service/EnhancedModeServiceRuntime.h"

#include <wtsapi32.h>

#include <chrono>
#include <future>
#include <memory>
#include <stdexcept>
#include <system_error>
#include <thread>
#include <utility>

#include "foundation/logging/Log.h"
#include "infrastructure/windows/ipc/EnhancedModeProtocol.h"
#include "infrastructure/windows/resources/UniqueHandle.h"
#include "infrastructure/windows/service/EnhancedModeAgentHost.h"
#include "infrastructure/windows/service/PreshutdownCommitGate.h"

namespace uwf::windows {
namespace {
constexpr DWORD kServiceWaitHintMs = 60'000;
constexpr auto kServiceHeartbeatInterval = std::chrono::seconds{15};

[[noreturn]] void throwSystemError(const char* operation, const DWORD error = GetLastError()) {
  throw std::system_error(static_cast<int>(error), std::system_category(), operation);
}

bool sessionChangeRequiresAgent(const DWORD eventType) {
  // Lock/unlock must not relaunch a UI that the user explicitly closed.
  return eventType == WTS_SESSION_LOGON || eventType == WTS_CONSOLE_CONNECT || eventType == WTS_REMOTE_CONNECT;
}

class ScmServiceHost final {
 public:
  static ScmServiceHost& instance() {
    static ScmServiceHost host;
    return host;
  }
  void run(PreshutdownCommit commit) {
    m_runtime = std::make_unique<EnhancedModeServiceRuntime>(std::move(commit), m_gate, environment());
    SERVICE_TABLE_ENTRYW table[] = {{const_cast<LPWSTR>(kEnhancedServiceName), &ScmServiceHost::serviceMainThunk}, {nullptr, nullptr}};
    if (!StartServiceCtrlDispatcherW(table)) throwSystemError("start service control dispatcher");
  }

 private:
  static void WINAPI serviceMainThunk(DWORD, LPWSTR*) {
    try {
      auto& host = instance();
      host.m_statusHandle = RegisterServiceCtrlHandlerExW(kEnhancedServiceName, &ScmServiceHost::handlerThunk, &host);
      if (!host.m_statusHandle) throwSystemError("register enhanced mode service handler");
      host.m_runtime->run();
    } catch (...) {
      // Runtime failures publish SERVICE_STOPPED. Registration failures have no status handle.
    }
  }
  static DWORD WINAPI handlerThunk(const DWORD control, const DWORD eventType, void*, void* context) {
    try {
      return static_cast<ScmServiceHost*>(context)->m_runtime->handleControl(control, eventType);
    } catch (...) {
      return ERROR_EXCEPTION_IN_SERVICE;
    }
  }
  EnhancedModeServiceEnvironment environment() {
    return {.startAgent =
                [this] {
                  m_stopEvent.reset(CreateEventW(nullptr, TRUE, FALSE, nullptr));
                  m_preshutdownEvent.reset(CreateEventW(nullptr, TRUE, FALSE, nullptr));
                  if (!m_stopEvent.valid() || !m_preshutdownEvent.valid()) throwSystemError("create enhanced mode service events");
                  m_agentHost = std::make_unique<EnhancedModeAgentHost>(m_gate);
                },
            .requestAgent =
                [this] {
                  if (m_agentHost) m_agentHost->requestAgent();
                },
            .stopAgent =
                [this] {
                  if (m_agentHost) m_agentHost->stop();
                },
            .agentFailureCode = [this] { return m_agentHost ? m_agentHost->failureCode() : NO_ERROR; },
            .waitForShutdown = [this] { return waitForServiceShutdown(m_stopEvent.get(), m_agentHost->failureEvent(), m_preshutdownEvent.get()); },
            .signalStop =
                [this] {
                  if (m_stopEvent.valid()) SetEvent(m_stopEvent.get());
                },
            .signalPreshutdown =
                [this] {
                  if (!SetEvent(m_preshutdownEvent.get())) throwSystemError("request enhanced mode preshutdown");
                },
            .publishStatus =
                [this](const SERVICE_STATUS& status) {
                  auto nativeStatus = status;
                  if (!SetServiceStatus(m_statusHandle, &nativeStatus)) throwSystemError("publish enhanced mode service status");
                }};
  }
  SERVICE_STATUS_HANDLE m_statusHandle = nullptr;
  UniqueHandle m_stopEvent;
  UniqueHandle m_preshutdownEvent;
  PreshutdownCommitGate m_gate;
  std::unique_ptr<EnhancedModeAgentHost> m_agentHost;
  std::unique_ptr<EnhancedModeServiceRuntime> m_runtime;
};
}  // namespace

ServiceShutdownEvent waitForServiceShutdown(const HANDLE stop, const HANDLE agentFailure, const HANDLE preshutdown) {
  const HANDLE events[] = {stop, agentFailure, preshutdown};
  const DWORD wait = WaitForMultipleObjects(3, events, FALSE, INFINITE);
  if (wait == WAIT_OBJECT_0) return ServiceShutdownEvent::Stop;
  if (wait == WAIT_OBJECT_0 + 1) return ServiceShutdownEvent::AgentFailure;
  if (wait == WAIT_OBJECT_0 + 2) return ServiceShutdownEvent::Preshutdown;
  throwSystemError("wait for enhanced mode service shutdown");
}

EnhancedModeServiceRuntime::EnhancedModeServiceRuntime(PreshutdownCommit commit, PreshutdownCommitGate& gate, EnhancedModeServiceEnvironment environment)
    : m_commit(std::move(commit)), m_gate(gate), m_environment(std::move(environment)) {
  if (!m_commit) throw std::invalid_argument("Preshutdown commit callback is required");
  if (!m_environment.startAgent || !m_environment.requestAgent || !m_environment.stopAgent || !m_environment.agentFailureCode ||
      !m_environment.waitForShutdown || !m_environment.signalStop || !m_environment.signalPreshutdown || !m_environment.publishStatus)
    throw std::invalid_argument("Enhanced mode service environment is incomplete");
}

void EnhancedModeServiceRuntime::run() {
  bool agentStopped = false;
  const auto stopAgent = [&] {
    if (!agentStopped) {
      m_environment.stopAgent();
      agentStopped = true;
    }
  };
  try {
    report({.state = SERVICE_START_PENDING, .checkpoint = 1, .waitHint = kServiceWaitHintMs});
    m_environment.startAgent();
    report({.state = SERVICE_RUNNING, .controls = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PRESHUTDOWN | SERVICE_ACCEPT_SESSIONCHANGE});
    const auto shutdown = m_environment.waitForShutdown();
    // The native stop callback joins authentication and supervision before commit.
    stopAgent();
    if (shutdown == ServiceShutdownEvent::Preshutdown) runPreshutdownCommit();
    reportStopped(m_environment.agentFailureCode());
  } catch (...) {
    const auto failure = std::current_exception();
    try {
      m_environment.signalStop();
    } catch (...) {
    }
    try {
      stopAgent();
    } catch (...) {
    }
    // ERROR_SERVICE_SPECIFIC_ERROR requires a private code; this service defines none.
    reportStopped(ERROR_GEN_FAILURE);
    std::rethrow_exception(failure);
  }
}

DWORD EnhancedModeServiceRuntime::handleControl(const DWORD control, const DWORD eventType) {
  switch (control) {
    case SERVICE_CONTROL_STOP:
      if (!m_preshutdownStarted.load(std::memory_order_acquire)) {
        report({.state = SERVICE_STOP_PENDING, .checkpoint = 1, .waitHint = kServiceWaitHintMs});
        m_environment.signalStop();
      }
      return NO_ERROR;
    case SERVICE_CONTROL_PRESHUTDOWN: {
      bool expected = false;
      if (!m_preshutdownStarted.compare_exchange_strong(expected, true, std::memory_order_acq_rel)) return NO_ERROR;
      try {
        report({.state = SERVICE_STOP_PENDING, .checkpoint = nextCheckpoint(), .waitHint = kServiceWaitHintMs});
        m_environment.signalPreshutdown();
      } catch (...) {
        m_preshutdownStarted.store(false, std::memory_order_release);
        throw;
      }
      return NO_ERROR;
    }
    case SERVICE_CONTROL_SESSIONCHANGE:
      if (sessionChangeRequiresAgent(eventType)) m_environment.requestAgent();
      return NO_ERROR;
    case SERVICE_CONTROL_INTERROGATE:
      publishCurrentStatus();
      return NO_ERROR;
    default:
      return ERROR_CALL_NOT_IMPLEMENTED;
  }
}

void EnhancedModeServiceRuntime::report(const StatusUpdate& update) {
  std::scoped_lock lock(m_statusMutex);
  m_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
  m_status.dwCurrentState = update.state;
  m_status.dwControlsAccepted = update.controls;
  m_status.dwWin32ExitCode = NO_ERROR;
  m_status.dwServiceSpecificExitCode = 0;
  m_status.dwCheckPoint = update.checkpoint;
  m_status.dwWaitHint = update.waitHint;
  m_environment.publishStatus(m_status);
}
void EnhancedModeServiceRuntime::publishCurrentStatus() {
  std::scoped_lock lock(m_statusMutex);
  m_environment.publishStatus(m_status);
}
void EnhancedModeServiceRuntime::reportStopped(const DWORD exitCode) noexcept {
  std::scoped_lock lock(m_statusMutex);
  m_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
  m_status.dwCurrentState = SERVICE_STOPPED;
  m_status.dwControlsAccepted = 0;
  m_status.dwWin32ExitCode = exitCode;
  m_status.dwServiceSpecificExitCode = 0;
  m_status.dwCheckPoint = 0;
  m_status.dwWaitHint = 0;
  try {
    m_environment.publishStatus(m_status);
  } catch (...) {
  }
}
DWORD EnhancedModeServiceRuntime::nextCheckpoint() { return m_checkpoint.fetch_add(1, std::memory_order_relaxed) + 1; }
void EnhancedModeServiceRuntime::reportRealProgress() {
  report({.state = SERVICE_STOP_PENDING, .checkpoint = nextCheckpoint(), .waitHint = kServiceWaitHintMs});
}
void EnhancedModeServiceRuntime::runPreshutdownCommit() {
  if (m_gate.consumeHandled()) {
    UWF_LOG_I("service") << "enhanced mode staged commit skipped: reason=recent-ui-precommit";
    return;
  }
  std::promise<app::FileStagingCommitResult> resultPromise;
  auto resultFuture = resultPromise.get_future();
  std::jthread worker([promise = std::move(resultPromise), commit = m_commit]() mutable {
    try {
      promise.set_value(commit());
    } catch (...) {
      promise.set_exception(std::current_exception());
    }
  });
  bool heartbeatAvailable = true;
  while (resultFuture.wait_for(kServiceHeartbeatInterval) != std::future_status::ready) {
    if (!heartbeatAvailable) continue;
    try {
      reportRealProgress();
    } catch (const std::exception& error) {
      heartbeatAvailable = false;
      UWF_LOG_W("service") << "enhanced mode SCM heartbeat failed while staged commit continues: error=" << error.what();
    } catch (...) {
      heartbeatAvailable = false;
      UWF_LOG_W("service") << "enhanced mode SCM heartbeat failed while staged commit continues: error=non-standard-exception";
    }
  }
  worker.join();
  try {
    const auto result = resultFuture.get();
    UWF_LOG_I("service") << "enhanced mode staged commit completed: discovered=" << result.discoveredFiles << " committed=" << result.committedFiles
                         << " skippedFiles=" << result.skippedFiles << " skippedEntries=" << result.skippedEntries << " pausedFiles=" << result.pausedFiles
                         << " pausedEntries=" << result.pausedEntries << " failures=" << result.failures.size();
  } catch (...) {
    // The injected application operation already recorded its actual failure.
  }
}

int runEnhancedModeService(PreshutdownCommit commit) {
  ScmServiceHost::instance().run(std::move(commit));
  return 0;
}
}  // namespace uwf::windows
