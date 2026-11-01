// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "infrastructure/windows/ipc/EnhancedModeAgent.h"

#include <windows.h>

#include <QScopeGuard>
#include <algorithm>
#include <array>
#include <chrono>
#include <exception>
#include <limits>
#include <optional>
#include <span>
#include <string>
#include <vector>

#include "foundation/logging/Log.h"
#include "infrastructure/windows/errors/WindowsSystemError.h"
#include "infrastructure/windows/ipc/EnhancedModeProtocol.h"
#include "infrastructure/windows/ipc/NamedPipeIo.h"
#include "infrastructure/windows/resources/UniqueHandle.h"
#include "infrastructure/windows/security/ExecutableIdentity.h"

namespace uwf::windows {

namespace {

constexpr auto kReconnectInitialDelay = std::chrono::milliseconds{250};
constexpr auto kReconnectMaximumDelay = std::chrono::seconds{5};
constexpr auto kConnectionFailureGrace = std::chrono::seconds{1};

bool isLocalSystemServiceProcess(const DWORD processId) {
  if (processId == 0) return false;
  const UniqueHandle process(OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processId));
  if (!process.valid() ||
      !windows::sameFile(windows::processImagePath(process.get()), windows::processImagePath(GetCurrentProcess()), windows::FileIdentityFormat::FileId128))
    return false;

  DWORD sessionId = std::numeric_limits<DWORD>::max();
  if (!ProcessIdToSessionId(processId, &sessionId) || sessionId != 0) return false;

  HANDLE rawToken = nullptr;
  if (!OpenProcessToken(process.get(), TOKEN_QUERY, &rawToken)) return false;
  const UniqueHandle token(rawToken);
  DWORD bytes = 0;
  GetTokenInformation(token.get(), TokenUser, nullptr, 0, &bytes);
  if (bytes == 0) return false;
  std::vector<BYTE> user(bytes);
  if (!GetTokenInformation(token.get(), TokenUser, user.data(), bytes, &bytes)) return false;
  BYTE localSystem[SECURITY_MAX_SID_SIZE]{};
  DWORD sidSize = sizeof(localSystem);
  if (!CreateWellKnownSid(WinLocalSystemSid, nullptr, localSystem, &sidSize)) return false;
  return EqualSid(reinterpret_cast<TOKEN_USER*>(user.data())->User.Sid, localSystem) != FALSE;
}

bool readAgentHandshake(const HANDLE pipe, const HANDLE stopEvent, const std::stop_token stopToken) {
  std::array<char, kEnhancedAgentHandshake.size()> received{};
  const NamedPipeIoContext context{stopEvent, std::chrono::steady_clock::now() + kEnhancedAgentExchangeTimeout};
  return readNamedPipe(pipe, std::as_writable_bytes(std::span{received}), context) == NamedPipeIoResult::Completed && !stopToken.stop_requested() &&
         received == kEnhancedAgentHandshake;
}

}  // namespace

struct EnhancedModeAgent::ControlChannel {
  ControlChannel() : stopEvent(CreateEventW(nullptr, TRUE, FALSE, nullptr)) {
    if (!stopEvent.valid()) throw WindowsSystemError(GetLastError(), "create enhanced mode agent stop event");
  }

  std::mutex stateMutex;
  std::mutex requestMutex;
  std::condition_variable changed;
  UniqueHandle stopEvent;
  HANDLE pipe = nullptr;
  std::uint64_t generation = 0;
  std::uint64_t nextRequestId = 1;
  std::optional<EnhancedAgentControlMessage> lastAcknowledgement;
};

EnhancedModeAgent::EnhancedModeAgent(QObject* parent) : app::EnhancedModeAgentConnection(parent), m_control(std::make_unique<ControlChannel>()) {}

EnhancedModeAgent::~EnhancedModeAgent() { stop(); }

void EnhancedModeAgent::start() {
  if (m_thread.joinable()) {
    if (m_workerActive.load(std::memory_order_acquire)) return;
    m_thread.join();
  }
  ResetEvent(m_control->stopEvent.get());
  m_workerActive.store(true, std::memory_order_release);
  try {
    m_thread = std::jthread([this](const std::stop_token stopToken) {
      const auto markStopped = qScopeGuard([this] { m_workerActive.store(false, std::memory_order_release); });
      try {
        run(stopToken);
      } catch (const std::exception& error) {
        UWF_LOG_E("service") << "enhanced mode identity agent stopped unexpectedly: error=" << error.what();
        if (!stopToken.stop_requested()) emit connectionStateChanged(false);
      } catch (...) {
        UWF_LOG_E("service") << "enhanced mode identity agent stopped unexpectedly: error=non-standard-exception";
        if (!stopToken.stop_requested()) emit connectionStateChanged(false);
      }
    });
  } catch (...) {
    m_workerActive.store(false, std::memory_order_release);
    throw;
  }
}

void EnhancedModeAgent::stop() {
  if (!m_thread.joinable()) return;
  m_thread.request_stop();
  SetEvent(m_control->stopEvent.get());
  m_waitChanged.notify_all();
  m_control->changed.notify_all();
  m_thread.join();
  m_workerActive.store(false, std::memory_order_release);
}

bool EnhancedModeAgent::running() const { return m_workerActive.load(std::memory_order_acquire); }

bool EnhancedModeAgent::markPreshutdownCommitHandled() { return sendControl(EnhancedAgentControl::PreshutdownCommitHandled); }

bool EnhancedModeAgent::requirePreshutdownCommit() { return sendControl(EnhancedAgentControl::PreshutdownCommitRequired); }

bool EnhancedModeAgent::sendControl(const EnhancedAgentControl control) {
  std::scoped_lock requestLock(m_control->requestMutex);
  HANDLE rawDuplicate = nullptr;
  std::uint64_t generation = 0;
  EnhancedAgentControlMessage request;
  {
    std::scoped_lock stateLock(m_control->stateMutex);
    if (!m_control->pipe) return false;
    if (!DuplicateHandle(GetCurrentProcess(), m_control->pipe, GetCurrentProcess(), &rawDuplicate, 0, FALSE, DUPLICATE_SAME_ACCESS)) return false;
    generation = m_control->generation;
    request.control = control;
    request.requestId = m_control->nextRequestId;
    m_control->nextRequestId = request.requestId == std::numeric_limits<std::uint64_t>::max() ? 1 : request.requestId + 1;
  }
  const UniqueHandle pipe(rawDuplicate);
  const auto deadline = std::chrono::steady_clock::now() + kEnhancedAgentExchangeTimeout;
  const NamedPipeIoContext context{m_control->stopEvent.get(), deadline};
  const auto frame = encodeEnhancedAgentControl(request);
  if (writeNamedPipe(pipe.get(), std::as_bytes(std::span{frame}), context) != NamedPipeIoResult::Completed) return false;

  std::unique_lock stateLock(m_control->stateMutex);
  const bool acknowledged =
      m_control->changed.wait_until(stateLock, deadline, [&] { return m_control->generation != generation || m_control->lastAcknowledgement == request; });
  return acknowledged && m_control->generation == generation && m_control->lastAcknowledgement == request;
}

bool EnhancedModeAgent::waitBeforeReconnect(const std::stop_token stopToken, const std::chrono::milliseconds delay) {
  std::unique_lock lock(m_waitMutex);
  m_waitChanged.wait_for(lock, delay, [&] { return stopToken.stop_requested(); });
  return !stopToken.stop_requested();
}

void EnhancedModeAgent::run(const std::stop_token stopToken) {
  auto retryDelay = kReconnectInitialDelay;
  const auto failureDeadline = std::chrono::steady_clock::now() + kConnectionFailureGrace;
  bool disconnectedPublished = false;
  const auto publishConnectionFailure = [&] {
    if (!disconnectedPublished && std::chrono::steady_clock::now() >= failureDeadline) {
      disconnectedPublished = true;
      emit connectionStateChanged(false);
    }
  };
  const auto waitForRetry = [&]() {
    const bool keepRunning = waitBeforeReconnect(stopToken, retryDelay);
    publishConnectionFailure();
    retryDelay = std::min(retryDelay * 2, std::chrono::duration_cast<std::chrono::milliseconds>(kReconnectMaximumDelay));
    return keepRunning;
  };

  while (!stopToken.stop_requested()) {
    constexpr DWORD kSecurityFlags = SECURITY_SQOS_PRESENT | SECURITY_IDENTIFICATION | SECURITY_EFFECTIVE_ONLY;
    UniqueHandle pipe(CreateFileW(kEnhancedPipeName, GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, kSecurityFlags | FILE_FLAG_OVERLAPPED, nullptr));
    if (!pipe.valid()) {
      if (!waitForRetry()) return;
      continue;
    }

    ULONG serverProcessId = 0;
    if (!GetNamedPipeServerProcessId(pipe.get(), &serverProcessId) || !isLocalSystemServiceProcess(static_cast<DWORD>(serverProcessId))) {
      UWF_LOG_W("service") << "enhanced mode identity pipe rejected: reason=untrusted-server";
      if (!waitForRetry()) return;
      continue;
    }
    if (!readAgentHandshake(pipe.get(), m_control->stopEvent.get(), stopToken)) {
      if (!waitForRetry()) return;
      continue;
    }

    retryDelay = kReconnectInitialDelay;
    disconnectedPublished = false;
    {
      {
        std::scoped_lock lock(m_control->stateMutex);
        m_control->pipe = pipe.get();
        m_control->lastAcknowledgement.reset();
        ++m_control->generation;
      }
      m_control->changed.notify_all();
      const auto releasePublishedPipe = qScopeGuard([this, handle = pipe.get()] {
        {
          std::scoped_lock lock(m_control->stateMutex);
          if (m_control->pipe != handle) return;
          m_control->pipe = nullptr;
          m_control->lastAcknowledgement.reset();
          ++m_control->generation;
        }
        m_control->changed.notify_all();
      });
      emit connectionStateChanged(true);

      // The channel maintains identity and acknowledges correlated changes to
      // the one-shot PRESHUTDOWN skip token after UI precommit.
      for (;;) {
        EnhancedAgentControlFrame frame{};
        if (readNamedPipeFrame(pipe.get(), std::as_writable_bytes(std::span{frame}), m_control->stopEvent.get(), kEnhancedAgentExchangeTimeout).result !=
            NamedPipeIoResult::Completed)
          break;
        const auto acknowledgement = decodeEnhancedAgentControl(frame);
        if (!acknowledgement) {
          UWF_LOG_W("service") << "enhanced mode identity channel rejected invalid control acknowledgement";
          break;
        }
        {
          std::scoped_lock lock(m_control->stateMutex);
          m_control->lastAcknowledgement = *acknowledgement;
        }
        m_control->changed.notify_all();
      }
    }
    if (stopToken.stop_requested()) return;
    disconnectedPublished = true;
    emit connectionStateChanged(false);
    if (!waitForRetry()) return;
  }
}

std::unique_ptr<app::EnhancedModeAgentConnection> createEnhancedModeAgent() { return std::make_unique<EnhancedModeAgent>(); }

}  // namespace uwf::windows

#include "moc_EnhancedModeAgent.cpp"
